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

#include "qscriptconsoledebugger.h"

#include "qscriptdebuggerfrontend.h"
#include "qscriptdebuggerconsole.h"
#include <QtCore/qtextstream.h>

//#include <private/qobject_p.h>

/*!
  \class QScriptConsoleDebugger

  \brief The QScriptConsoleDebugger class provides a console debugger.

  Call setFrontend() to set the front-end that the console debugger
  should use.
*/

class QScriptTextStreamOutputHandler : public QScriptConsoleOutputHandler
{
public:
    QScriptTextStreamOutputHandler();
    ~QScriptTextStreamOutputHandler();

    QTextStream *outputStream() const;
    void setOutputStream(QTextStream *outputStream);

    QTextStream *errorStream() const;
    void setErrorStream(QTextStream *errorStream);

    void output(Mode mode, const QString &text);

private:
    QTextStream *m_defaultOutputStream;
    QTextStream *m_defaultErrorStream;
    QTextStream *m_outputStream;
    QTextStream *m_errorStream;
};

QScriptTextStreamOutputHandler::QScriptTextStreamOutputHandler()
{
    m_defaultOutputStream = new QTextStream(stdout);
    m_defaultErrorStream = new QTextStream(stderr);
    m_outputStream = m_defaultOutputStream;
    m_errorStream = m_defaultErrorStream;
}

QScriptTextStreamOutputHandler::~QScriptTextStreamOutputHandler()
{
    delete m_defaultOutputStream;
    delete m_defaultErrorStream;
}

QTextStream *QScriptTextStreamOutputHandler::outputStream() const
{
    return m_outputStream;
}

void QScriptTextStreamOutputHandler::setOutputStream(QTextStream *outputStream)
{
    m_outputStream = outputStream;
}

QTextStream *QScriptTextStreamOutputHandler::errorStream() const
{
    return m_errorStream;
}

void QScriptTextStreamOutputHandler::setErrorStream(QTextStream *errorStream)
{
    m_errorStream = errorStream;
}

void QScriptTextStreamOutputHandler::output(Mode mode, const QString &text)
{
    switch (mode) {
    case Normal:
        *m_outputStream << text;
        m_outputStream->flush();
        break;
    case Warning:
    case Error:
        *m_errorStream << text;
        m_errorStream->flush();
        break;
    }
}



class QScriptConsoleDebuggerPrivate
//    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QScriptConsoleDebugger)
public:
    QScriptConsoleDebuggerPrivate();
    ~QScriptConsoleDebuggerPrivate();

    QScriptDebuggerConsole *console;
    QScriptTextStreamOutputHandler *outputHandler;

    QScriptConsoleDebugger *q_ptr; // ###
};

QScriptConsoleDebuggerPrivate::QScriptConsoleDebuggerPrivate()
{
    console = 0;
    outputHandler = 0;
}

QScriptConsoleDebuggerPrivate::~QScriptConsoleDebuggerPrivate()
{
    delete console;
    delete outputHandler;
}

void QScriptConsoleDebugger::_q_prompt()
{
    Q_D(QScriptConsoleDebugger);
    QString prompt;
    if (d->console->hasIncompleteInput()) {
        prompt = QString::fromLatin1(".... ");
    } else {
        prompt = QString::fromLatin1("(qsdbg) ");
    }
    d->outputHandler->output(QScriptConsoleOutputHandler::Normal, prompt);

    QTextStream in(stdin);
    QString line = in.readLine();
    d->console->input(line);
}



/*!
  Constructs a new QScriptConsoleDebugger object with the given \a
  parent.
*/
QScriptConsoleDebugger::QScriptConsoleDebugger(QObject *parent)
//    : QObject(*new QScriptConsoleDebuggerPrivate, parent)
    : QObject(parent),
        d_ptr(new QScriptConsoleDebuggerPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys this QScriptConsoleDebugger.
*/
QScriptConsoleDebugger::~QScriptConsoleDebugger()
{
    delete d_ptr; // ###
}

/*!
  Sets the front-end that this debugger should use.
*/
void QScriptConsoleDebugger::setFrontend(QScriptDebuggerFrontend *frontend)
{
    Q_D(QScriptConsoleDebugger);
    d->console = new QScriptDebuggerConsole(frontend);
    d->outputHandler = new QScriptTextStreamOutputHandler();
    d->console->setOutputHandler(d->outputHandler);
    QObject::connect(d->console, SIGNAL(commandExecuted()),
                     this, SLOT(_q_prompt()));
}

/*!
  Returns the front-end that this debugger is using.
*/
QScriptDebuggerFrontend *QScriptConsoleDebugger::frontend() const
{
    Q_D(const QScriptConsoleDebugger);
    if (!d->console)
        return 0;
    return d->console->frontend();
}
