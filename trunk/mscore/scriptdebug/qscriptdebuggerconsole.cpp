/****************************************************************************
**
** Copyright (C) 2007-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Script Debug project on Trolltech Labs.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qscriptdebuggerconsole.h"
#include "qscriptdebuggerfrontend.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptbreakpointinfo.h"
#include "qscriptcontextinfo.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"
#include <QtCore/qdebug.h>
#include <QtCore/qstringlist.h>

Q_DECLARE_METATYPE(QScriptDebuggerValue)
Q_DECLARE_METATYPE(QScriptDebuggerValueList)
Q_DECLARE_METATYPE(QScriptDebuggerProperty)
Q_DECLARE_METATYPE(QScriptDebuggerPropertyList)
Q_DECLARE_METATYPE(QScriptContextInfo)
Q_DECLARE_METATYPE(QScriptBreakpointInfo)
Q_DECLARE_METATYPE(QList<int>)

// ### separate response handler class for each command
// - each response handler is a state machine
class QScriptDebuggerConsoleCommand
{
public:
    QScriptDebuggerConsoleCommand(QScriptDebuggerConsole *console);
    virtual ~QScriptDebuggerConsoleCommand();

    virtual QString name() const = 0;
    virtual QString description() const;
    virtual QString help() const;

    virtual void execute(const QStringList &args) = 0;

    QScriptDebuggerConsole *console() const;

private:
    Q_DISABLE_COPY(QScriptDebuggerConsoleCommand)
};



QScriptDebuggerConsole::QScriptDebuggerConsole(QScriptDebuggerFrontend *frontend)
    : QScriptDebuggerClient(frontend)
{
    m_commandPrefix = QLatin1String(".");
    m_hasBeenWelcomed = false;
    m_breakpointId = -1;
    m_state = NoState;
    m_outputHandler = 0;
    frontend->setClient(this);
}

QScriptDebuggerConsole::~QScriptDebuggerConsole()
{
}

QScriptConsoleOutputHandler *QScriptDebuggerConsole::outputHandler() const
{
    return m_outputHandler;
}

void QScriptDebuggerConsole::setOutputHandler(QScriptConsoleOutputHandler *handler)
{
    m_outputHandler = handler;
}

void QScriptDebuggerConsole::showWelcomeMessage()
{
    message("\n*** Welcome to qsdbg. Debugger commands start with a . (period)\n"
            "*** Any other input will be evaluated by the script interpreter.\n"
            "*** Type .help for help.\n");
}

void QScriptDebuggerConsole::message(const QString &text)
{
    if (m_outputHandler)
        m_outputHandler->output(QScriptConsoleOutputHandler::Normal, text + QLatin1String("\n"));
}

void QScriptDebuggerConsole::errorMessage(const QString &text)
{
    if (m_outputHandler)
        m_outputHandler->output(QScriptConsoleOutputHandler::Error, text + QLatin1String("\n"));
}

int QScriptDebuggerConsole::currentFrameIndex() const
{
    return m_currentFrameIndex;
}

void QScriptDebuggerConsole::setCurrentFrameIndex(int index)
{
    m_currentFrameIndex = index;
    m_listLineNumber = -1;
}

void QScriptDebuggerConsole::executeCommand(const QString &command, const QStringList &args)
{
    Q_ASSERT(m_state == NoState);
    if (command == QLatin1String("c")
        || command == QLatin1String("continue")) {
        frontend()->scheduleContinue();
    } else if (command == QLatin1String("s")
               || command == QLatin1String("step")) {
        frontend()->scheduleStepInto();
    } else if (command == QLatin1String("n")
               || command == QLatin1String("next")) {
        frontend()->scheduleStepOver();
    } else if (command == QLatin1String("finish")) {
        frontend()->scheduleStepOut();
    } else if (command == QLatin1String("backtrace") || command == QLatin1String("bt")) {
        m_state = BacktraceState;
        frontend()->scheduleGetBacktrace( 0);
    } else if (command == QLatin1String("up")) {
        m_state = UpState;
        frontend()->scheduleGetContextString( currentFrameIndex()+1);
    } else if (command == QLatin1String("down")) {
        m_state = DownState;
        frontend()->scheduleGetContextString( currentFrameIndex()-1);
    } else if (command == QLatin1String("frame") || command == QLatin1String("f")) {
        bool ok = false;
        int index = args.value(0).toInt(&ok);
        if (ok) {
            m_desiredFrameIndex = index;
            m_state = FrameState;
            frontend()->scheduleGetContextString(index);
        } else {
            ready();
        }
    } else if (command == QLatin1String("list")) {
        bool ok;
        int line = args.value(0).toInt(&ok);
        if (ok)
            m_listLineNumber = qMax(1, line - 5);
        m_state = List1State;
        frontend()->scheduleGetContextInfo(currentFrameIndex());
    } else if (command == QLatin1String("info")) {
        if (args.size() < 1) {
            ready();
        } else {
            QString what = args.value(0);
            if (what == QLatin1String("locals")) {
                m_state = InfoLocals1State;
                frontend()->scheduleGetActivationObject(currentFrameIndex());
            } else if (what == QLatin1String("breakpoints")) {
                m_state = InfoBreakpoints1State;
                frontend()->scheduleGetBreakpointIdentifiers();
            } else {
                errorMessage(QString::fromLatin1("No info about %0.").arg(what));
                ready();
            }
        }
    } else if (command == QLatin1String("break") || command == QLatin1String("b")) {
        QString str = args.value(0);
        int colonIndex = str.indexOf(QLatin1Char(':'));
        if (colonIndex != -1) {
            m_breakpointFileName = str.left(colonIndex);
            m_breakpointLineNumber = str.mid(colonIndex+1).toInt();
            m_state = SetBreakpointState;
            frontend()->scheduleSetBreakpoint(m_breakpointFileName, m_breakpointLineNumber);
        } else {
            errorMessage(QString::fromLatin1("Breakpoints must be of the form <file>:<line>."));
            ready();
        }
    } else if (command == QLatin1String("enable") || command == QLatin1String("disable")) {
        bool enable = (command == QLatin1String("enable"));
        if (args.size() > 0)
            m_breakpointId = args.value(0).toInt();
        if (m_breakpointId < 0) {
            ready();
        } else {
            m_state = EnableOrDisableState;
            frontend()->scheduleSetBreakpointEnabled(m_breakpointId, enable);
        }
    } else if (command == QLatin1String("help")) {
        message("continue - continue execution\n"
                "step     - step into statement\n"
                "next     - step over statement\n"
                "list     - show where you are\n"
                "\n"
                "break    - set breakpoint\n"
                "delete   - remove breakpoint\n"
                "disable  - disable breakpoint\n"
                "enable   - enable breakpoint\n"
                "\n"
                "backtrace - show backtrace\n"
                "up       - one frame up\n"
                "down     - one frame down\n"
                "frame    - set frame\n"
                "\n"
                "info locals - show local variables");
        ready();
    } else {
        errorMessage(QString::fromLatin1("Undefined command \"%0\". Try \"help\".")
                     .arg(command));
        ready();
    }
}

bool QScriptDebuggerConsole::hasIncompleteInput() const
{
    return !m_input.isEmpty();
}

void QScriptDebuggerConsole::commandFinished(int id, const QScriptDebuggerResponse &response)
{
//    qDebug() << "commandFinished(" << id << ") state:" << m_state;
    switch (m_state) {
    case NoState:
        break;

    case CanEvaluateState: {
        bool ok = response.result().toBool();
        if (ok) {
            m_state = EvaluateState;
            frontend()->scheduleEvaluate(m_input, "typein");
        } else {
            ready();
        }
    }   break;

    case EvaluateState: {
        QScriptDebuggerValue result = qvariant_cast<QScriptDebuggerValue>(response.result());
        m_input.clear();
        m_state = ToStringState;
        m_toStringCommandId = frontend()->scheduleToString(result);
    }   break;

    case ToStringState: {
        if (m_toStringCommandId == id) {
            QString result = response.result().toString();
            message(result);
            ready();
        }
    }   break;

    case BreakState: {
        QStringList lines = response.result().toStringList();
        QString text;
        if (!lines.isEmpty())
            text = lines.at(0);
        else
            text = QString::fromLatin1("(no source text available)");
        message(QString::fromLatin1("%0\t%1").arg(m_lineNumber).arg(text));
        ready();
    }   break;

    case BacktraceState: {
        QStringList bt = response.result().toStringList();
        for (int i = 0; i < bt.size(); ++i)
            message(QString::fromLatin1("#%0  %1").arg(i).arg(bt.at(i)));
        ready();
    }   break;

    case UpState: {
        QString str = response.result().toString();
        if (str.isEmpty()) {
            errorMessage(QString::fromLatin1("Initial frame selected; you cannot go up."));
        } else {
            int newIndex = currentFrameIndex() + 1;
            setCurrentFrameIndex(newIndex);
            message(QString::fromLatin1("#%0  %1").arg(newIndex).arg(str));
        }
        ready();
    }   break;

    case DownState: {
        QString str = response.result().toString();
        if (str.isEmpty()) {
            errorMessage(QString::fromLatin1("Bottom (innermost) frame selected; you cannot go down."));
        } else {
            int newIndex = currentFrameIndex() - 1;
            setCurrentFrameIndex(newIndex);
            message(QString::fromLatin1("#%0  %1").arg(newIndex).arg(str));
        }
        ready();
    }   break;

    case FrameState: {
        QString str = response.result().toString();
        if (str.isEmpty()) {
            errorMessage(QString::fromLatin1("No such frame."));
        } else {
            setCurrentFrameIndex(m_desiredFrameIndex);
            message(QString::fromLatin1("#%0  %1").arg(m_desiredFrameIndex).arg(str));
        }
        ready();
    }   break;

    case List1State: {
        QScriptContextInfo info = qvariant_cast<QScriptContextInfo>(response.result());
        if (m_listLineNumber == -1)
            m_listLineNumber = qMax(1, info.lineNumber() - 5);
        m_state = List2State;
        frontend()->scheduleGetScriptLines(info.scriptId(), m_listLineNumber, 10);
    }   break;
    case List2State: {
        QStringList lines = response.result().toStringList();
        for (int i = 0; i < 10; ++i)
            message(QString::fromLatin1("%0\t%1").arg(i+m_listLineNumber).arg(lines.value(i)));
        m_listLineNumber += 10;
        ready();
    }   break;

    case InfoLocals1State: {
        m_activation = qvariant_cast<QScriptDebuggerValue>(response.result());
        m_state = InfoLocals2State;
        frontend()->scheduleNewIterator(m_activation);
    }   break;
    case InfoLocals2State: {
        m_activationIteratorId = response.result().toInt();
        m_state = InfoLocals3State;
        frontend()->scheduleIteratorNextProperties(m_activationIteratorId, 64);
    }   break;
    case InfoLocals3State: {
        QScriptDebuggerPropertyList props = qvariant_cast<QScriptDebuggerPropertyList>(response.result());
        if (props.isEmpty()) {
            ready();
        } else {
            for (int i = 0; i < props.size(); ++i) {
                QScriptDebuggerProperty p = props.at(i);
                message(QString::fromLatin1("%0 = %1").arg(p.name()).arg(p.value().toString()));
            }
            frontend()->scheduleIteratorNextProperties(m_activationIteratorId, 64);
        }
    }   break;

    case InfoBreakpoints1State: {
        m_breakpointIds = qvariant_cast<QList<int> >(response.result());
        if (m_breakpointIds.isEmpty()) {
            message(QString::fromLatin1("No breakpoints set."));
            ready();
        } else {
            m_breakpointIndex = 0;
            m_state = InfoBreakpoints2State;
            frontend()->scheduleGetBreakpointInfo(m_breakpointIds.at(m_breakpointIndex));
        }
    }   break;
    case InfoBreakpoints2State: {
        QScriptBreakpointInfo info = qvariant_cast<QScriptBreakpointInfo>(response.result());
        if (m_breakpointIndex == 0)
            message(QString::fromLatin1("ID\tEnabled\tWhere"));
        QString line;
        line += QString::number(m_breakpointIds.at(m_breakpointIndex));
        line += QString::fromLatin1("\t");
        if (info.isEnabled())
            line += QString::fromLatin1("Yes");
        else
            line += QString::fromLatin1("No");
        line += QString::fromLatin1("\t%0:%1").arg(info.fileName()).arg(info.lineNumber());
        message(line);
        ++m_breakpointIndex;
        if (m_breakpointIndex == m_breakpointIds.size()) {
            ready();
        } else {
            frontend()->scheduleGetBreakpointInfo(m_breakpointIds.at(m_breakpointIndex));
        }
    }   break;

    case SetBreakpointState: {
        m_breakpointId = response.result().toInt();
        message(QString::fromLatin1("Breakpoint %0 at %1, line %2.")
                .arg(m_breakpointId).arg(m_breakpointFileName).arg(m_breakpointLineNumber));
        ready();
    }   break;

    case Breakpoint1State: {
        QScriptBreakpointInfo info = qvariant_cast<QScriptBreakpointInfo>(response.result());
        m_breakpointFileName = info.fileName();
        m_breakpointLineNumber = info.lineNumber();
        message(QString::fromLatin1("Breakpoint %0 at %1, line %2.")
                .arg(m_breakpointId).arg(m_breakpointFileName).arg(m_breakpointLineNumber));
        m_state = Breakpoint2State;
        frontend()->scheduleGetScriptId(0);
    }   break;
    case Breakpoint2State: {
        qint64 scriptId = response.result().toLongLong();
        m_state = BreakState;
        frontend()->scheduleGetScriptLines(scriptId, m_breakpointLineNumber, 1);
    }   break;

    case EnableOrDisableState: {
        if (response.error() == QScriptDebuggerResponse::InvalidBreakpointID) {
            errorMessage(QString::fromLatin1("No breakpoint with ID %0.").arg(m_breakpointId));
        }
        ready();
    }   break;

    }
}

void QScriptDebuggerConsole::event(const QScriptDebuggerEvent &event)
{
//    qDebug() << "event(" << event.type() << ")";
    if (!m_hasBeenWelcomed) {
        showWelcomeMessage();
        m_hasBeenWelcomed = true;
    }

    setCurrentFrameIndex(0);
    m_lineNumber = event.lineNumber();
    switch (event.type()) {
    case QScriptDebuggerEvent::Break:
    case QScriptDebuggerEvent::SteppingFinished:
        m_state = BreakState;
        frontend()->scheduleGetScriptLines(event.scriptId(), m_lineNumber, 1);
        break;

    case QScriptDebuggerEvent::Breakpoint:
        m_breakpointId = event.attribute(QScriptDebuggerEvent::BreakpointID).toInt();
        m_state = Breakpoint1State;
        frontend()->scheduleGetBreakpointInfo(m_breakpointId);
        break;

    case QScriptDebuggerEvent::Exception: {
        m_input.clear();
        if (m_state == EvaluateState) {
            frontend()->scheduleClearExceptions();
        }
        QScriptDebuggerValue val = qvariant_cast<QScriptDebuggerValue>(event.attribute(QScriptDebuggerEvent::ExceptionValue));
        m_state = ToStringState;
        m_toStringCommandId = frontend()->scheduleToString(val);
        }   break;

    default: {
        errorMessage(QString::fromLatin1("Unknown debugger event (%0)").arg(event.type()));
        ready();
      }
    }
}

void QScriptDebuggerConsole::ready()
{
    m_state = NoState;
    emit commandExecuted();
}

void QScriptDebuggerConsole::input(const QString &text)
{
    if (m_input.isEmpty() && (text.isEmpty() || text.startsWith(m_commandPrefix))) {
        QString cmd;
        if (text.isEmpty()) {
            cmd = m_lastInteractiveCommand;
        } else {
            cmd = text;
            m_lastInteractiveCommand = cmd;
        }

        QStringList parts = cmd.split(QLatin1Char(' '), QString::SkipEmptyParts);
        if (!parts.isEmpty()) {
            QString command = parts.takeFirst().mid(1);
            executeCommand(command, parts);
        }
    } else {
        m_input += text;
        m_input += QLatin1Char('\n');

        m_state = CanEvaluateState;
        frontend()->scheduleCanEvaluate(m_input);
    }
}
