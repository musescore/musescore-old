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

#include "qscriptembeddeddebugger.h"
#include "qscriptdebugger.h"
#include "qscriptenginedebuggerfrontend.h"

#include <QtGui/qmainwindow.h>
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptvalue.h>

/*!
  \class QScriptEmbeddedDebugger

  \brief The QScriptEmbeddedDebugger class provides a graphical debugger.

  Typically you would initialize the debugger as follows:
  \code
    QScriptEngine engine;

    QScriptEmbeddedDebugger debugger;
    debugger.attachTo(&engine);
  \endcode

  Once this is done, you can use the script engine like normal (e.g.
  start evaluating scripts).  The debugger will pop up in a separate
  window as soon as execution is halted (e.g.  when an uncaught
  exception occurs). You can call the breakAtFirstStatement() function
  to cause the debugger to be invoked as soon as script evaluation
  begins.

  Scripts can use the print() function to print messages to the
  debugger's Console view. Scripts can call the qs_break() function to
  trigger the debugger at a specific point of a script.
*/

/*!
  Constructs a new QScriptEmbeddedDebugger object with the given \a
  parent.
*/
QScriptEmbeddedDebugger::QScriptEmbeddedDebugger(QObject *parent)
    : QObject(parent), m_frontend(0), m_debugger(0), m_window(0)
{
}

/*!
  Destroys this QScriptEmbeddedDebugger.
*/
QScriptEmbeddedDebugger::~QScriptEmbeddedDebugger()
{
    delete m_debugger;
    delete m_frontend;
    delete m_window;
}

/*!
  Attaches to the given \a engine.

  \sa detach()
*/
void QScriptEmbeddedDebugger::attachTo(QScriptEngine *engine)
{
    if (!m_frontend)
        m_frontend = new QScriptEngineDebuggerFrontend;
    m_frontend->attachTo(engine);

    QScriptValue global = engine->globalObject();
    global.setProperty("print", m_frontend->traceFunction());
    global.setProperty("qs_break", m_frontend->breakFunction());
    
    if (!m_debugger) {
        m_debugger = new QScriptDebugger;
        QObject::connect(m_debugger, SIGNAL(executionHalted()),
                         this, SLOT(_q_onExecutionHalted()));
        m_window = new QMainWindow();
        m_debugger->initMainWindow(m_window);
        m_window->resize(1024, 672);
    }
    m_debugger->setFrontend(m_frontend);
}

/*!
  Detaches from the current engine.

  \sa attachTo()
*/
void QScriptEmbeddedDebugger::detach()
{
    if (m_frontend)
        m_frontend->detach();
    if (m_debugger)
        m_debugger->setFrontend(0);
}

/*!
  Causes the executionHalted() signal to be emitted as soon as the
  next script statement is reached.
*/
void QScriptEmbeddedDebugger::breakAtFirstStatement()
{
    if (m_frontend)
        m_frontend->breakAtFirstStatement();
}

void QScriptEmbeddedDebugger::_q_onExecutionHalted()
{
    if (m_window) {
        m_window->show();
        m_window->activateWindow();
    }
}
