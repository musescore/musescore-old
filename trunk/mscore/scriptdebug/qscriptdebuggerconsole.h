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

#ifndef QSCRIPTDEBUGGERCONSOLE_H
#define QSCRIPTDEBUGGERCONSOLE_H

#include <QtCore/qobject.h>
#include <QtCore/qlist.h>
#include <QtCore/qstringlist.h>
#include "qscriptdebuggerclient.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"

class QScriptConsoleOutputHandler;
class QScriptDebuggerEvent;
class QScriptDebuggerResponse;
class QScriptDebuggerFrontend;

class Q_SCRIPTDEBUG_EXPORT QScriptConsoleOutputHandler
{
public:
    enum Mode {
        Normal,
        Warning,
        Error
    };

    QScriptConsoleOutputHandler() {}
    virtual ~QScriptConsoleOutputHandler() {}

    virtual void output(Mode mode, const QString &text) = 0;

private:
    // ### Q_DECLARE_PRIVATE(QScriptConsoleOutputHandler)
    Q_DISABLE_COPY(QScriptConsoleOutputHandler)
};

class QScriptDebuggerConsolePrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerConsole : public QObject,
    public QScriptDebuggerClient
{
    Q_OBJECT
public:
    enum State {
        NoState,
        CanEvaluateState,
        EvaluateState,
        ToStringState,
        BreakState,
        BacktraceState,
        UpState,
        DownState,
        FrameState,
        List1State,
        List2State,
        InfoLocals1State,
        InfoLocals2State,
        InfoLocals3State,
        InfoBreakpoints1State,
        InfoBreakpoints2State,
        SetBreakpointState,
        Breakpoint1State,
        Breakpoint2State,
        EnableOrDisableState
    };

    QScriptDebuggerConsole(QScriptDebuggerFrontend *frontend);
    ~QScriptDebuggerConsole();

    QScriptConsoleOutputHandler *outputHandler() const;
    void setOutputHandler(QScriptConsoleOutputHandler *handler);

    int listLineNumber() const;
    void setListLineNumber(int lineNumber);

    int currentFrameIndex() const;
    void setCurrentFrameIndex(int index);

    bool hasIncompleteInput() const;

    // registerCommand()
    // unregisterCommand()
    // registeredCommandNames()

    void commandFinished(int id, const QScriptDebuggerResponse &response);
    void event(const QScriptDebuggerEvent &event);

public slots:
    void input(const QString &text);

signals:
    void commandAboutToBeExecuted();
    void commandExecuted();

private:
    void message(const QString &text);
    void errorMessage(const QString &text);

    void showWelcomeMessage();

    void executeCommand(const QString &command, const QStringList &args);

    void ready();

    QScriptConsoleOutputHandler *m_outputHandler;
    State m_state;
    QString m_commandPrefix;
    int m_currentFrameIndex;
    int m_listLineNumber;
    QString m_lastInteractiveCommand;
    QString m_input;
    int m_toStringCommandId;
    int m_lineNumber;
    int m_desiredFrameIndex;
    QScriptDebuggerValue m_activation;
    QScriptDebuggerProperty m_property;
    int m_activationIteratorId;
    QString m_program;
    int m_breakpointId;
    QString m_breakpointFileName;
    int m_breakpointLineNumber;
    QList<int> m_breakpointIds;
    int m_breakpointIndex;
    bool m_hasBeenWelcomed;

    // Q_DECLARE_PRIVATE(QScriptDebuggerConsole)
    Q_DISABLE_COPY(QScriptDebuggerConsole)
};

#endif
