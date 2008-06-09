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

#include "qscriptenginedebuggerfrontend.h"

#include "qscriptdebuggerbackend.h"
#include "qscriptdebuggerbackend_p.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerfrontend_p.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptengine.h"
#include <QtCore/qeventloop.h>
#include <QtCore/qdebug.h>

/*!
  \class QScriptEngineDebuggerFrontend

  \brief The QScriptEngineDebuggerFrontend class provides a debugger interface to a QScriptEngine.

  This front-end can be used when the debugger is running in the same
  process as the script engine being debugged. For out-of-process
  debugging, a front-end should be obtained from e.g. a
  QScriptDebuggerConnector instead.

  Call attachTo() to attach the debugger to a QScriptEngine. Once this
  is done, the scheduleXXX() functions inherited from
  QScriptDebuggerFrontend can be used to schedule debugger commands.

  Call detach() to detach from the current engine.
*/

class QScriptEngineDebuggerFrontendPrivate;

class QScriptEngineDebuggerBackendPrivate;
class QScriptEngineDebuggerBackend : public QScriptDebuggerBackend
{
public:
    QScriptEngineDebuggerBackend(QScriptEngine *engine,
                                 QScriptEngineDebuggerFrontendPrivate *frontend);
    ~QScriptEngineDebuggerBackend();

protected:
    void event(const QScriptDebuggerEvent &event);
    void resume();

private:
    Q_DECLARE_PRIVATE(QScriptEngineDebuggerBackend)
    Q_DISABLE_COPY(QScriptEngineDebuggerBackend)
};

class QScriptEngineDebuggerBackendPrivate : public QScriptDebuggerBackendPrivate
{
    Q_DECLARE_PUBLIC(QScriptEngineDebuggerBackend)
public:
    QScriptEngineDebuggerBackendPrivate();
    ~QScriptEngineDebuggerBackendPrivate();

    QScriptEngineDebuggerFrontendPrivate *frontend;
};

class QScriptEngineDebuggerFrontendPrivate : public QScriptDebuggerFrontendPrivate
{
    Q_DECLARE_PUBLIC(QScriptEngineDebuggerFrontend)
public:
    QScriptEngineDebuggerFrontendPrivate();
    ~QScriptEngineDebuggerFrontendPrivate();

    void event(const QScriptDebuggerEvent &event);
    void resume();

    QScriptEngineDebuggerBackend *backend;
    QEventLoop *eventLoop;
};

QScriptEngineDebuggerBackendPrivate::QScriptEngineDebuggerBackendPrivate()
{
    frontend = 0;
}

QScriptEngineDebuggerBackendPrivate::~QScriptEngineDebuggerBackendPrivate()
{
}

/*!
  \internal

  Creates a new QScriptEngineDebuggerBackend object for the given \a
  engine.  The back-end will forward events to the given \a frontend.
*/
QScriptEngineDebuggerBackend::QScriptEngineDebuggerBackend(QScriptEngine *engine,
                                                           QScriptEngineDebuggerFrontendPrivate *frontend)
    : QScriptDebuggerBackend(*new QScriptEngineDebuggerBackendPrivate, engine)
{
    Q_D(QScriptEngineDebuggerBackend);
    d->frontend = frontend;
}

QScriptEngineDebuggerBackend::~QScriptEngineDebuggerBackend()
{
}

/*!
  \reimp
*/
void QScriptEngineDebuggerBackend::event(const QScriptDebuggerEvent &event)
{
    Q_D(QScriptEngineDebuggerBackend);
    // forward to front-end
    d->frontend->event(event);
}

/*!
  \reimp
*/
void QScriptEngineDebuggerBackend::resume()
{
    Q_D(QScriptEngineDebuggerBackend);
    // forward to front-end
    d->frontend->resume();
}



QScriptEngineDebuggerFrontendPrivate::QScriptEngineDebuggerFrontendPrivate()
{
    backend = 0;
    eventLoop = 0;
}

QScriptEngineDebuggerFrontendPrivate::~QScriptEngineDebuggerFrontendPrivate()
{
}

/*!
  \internal

  Handles an event that has happened in the back-end. We notify the
  client, then enter an event loop until the front-end sends a command
  telling us to resume.
*/
void QScriptEngineDebuggerFrontendPrivate::event(const QScriptDebuggerEvent &event)
{
    Q_Q(QScriptEngineDebuggerFrontend);
    q->notifyEvent(event);
    if (event.type() == QScriptDebuggerEvent::Trace)
        return;
    if (!eventLoop)
        eventLoop = new QEventLoop();
    else if (eventLoop->isRunning()) {
        qWarning("oops, recursive event notification detected");
        resume();
        return;
    }
    eventLoop->exec();
    eventLoop->processEvents();
}

void QScriptEngineDebuggerFrontendPrivate::resume()
{
    Q_ASSERT(eventLoop);
    // quitting the event loop will cause event() to return (see above)
    eventLoop->quit();
}


/*!
  Constructs a new QScriptEngineDebuggerFrontend object.

  \sa attachTo()
*/
QScriptEngineDebuggerFrontend::QScriptEngineDebuggerFrontend()
    : QScriptDebuggerFrontend(*new QScriptEngineDebuggerFrontendPrivate)
{
}

/*!
  \internal
*/
QScriptEngineDebuggerFrontend::QScriptEngineDebuggerFrontend(QScriptEngineDebuggerFrontendPrivate &dd)
    : QScriptDebuggerFrontend(dd)
{
}

/*!
  Destroys this QScriptEngineDebuggerFrontend.
*/
QScriptEngineDebuggerFrontend::~QScriptEngineDebuggerFrontend()
{
    Q_D(QScriptEngineDebuggerFrontend);
    if (d->eventLoop) {
        d->eventLoop->quit();
        delete d->eventLoop;
    }
    // backend is owned by engine
}

/*!
  Attaches this debugger front-end to the given \a engine.

  The front-end automatically detaches from the old engine, if any.

  \sa detach()
*/
void QScriptEngineDebuggerFrontend::attachTo(QScriptEngine *engine)
{
    Q_D(QScriptEngineDebuggerFrontend);
    detach();
    d->backend = new QScriptEngineDebuggerBackend(engine, d);
    engine->setAgent(d->backend);
}

/*!
  Detaches this debugger front-end from the current engine.

  \sa attachTo(), engine()
*/
void QScriptEngineDebuggerFrontend::detach()
{
    Q_D(QScriptEngineDebuggerFrontend);
    if (d->backend)
        d->backend->engine()->setAgent(0);
    // don't delete the backend, it's owned by the engine
    d->backend = 0;
}

/*!
  Returns the engine that this debugger front-end is attached to, or 0
  if the front-end is not attached to an engine.

  \sa attachTo()
*/
QScriptEngine *QScriptEngineDebuggerFrontend::engine() const
{
    Q_D(const QScriptEngineDebuggerFrontend);
    if (!d->backend)
        return 0;
    return d->backend->engine();
}

/*!
  Causes the front-end to generate a break event as soon as the next
  script statement is reached.

  \sa QScriptDebuggerClient::event()
*/
void QScriptEngineDebuggerFrontend::breakAtFirstStatement()
{
    Q_D(const QScriptEngineDebuggerFrontend);
    if (d->backend)
        d->backend->pauseEvaluation();
}

/*!
  Returns a trace function. The trace function has similar semantics
  to the built-in print() function; however, instead of writing text
  to standard output, it generates a trace event containing the text.
*/
QScriptValue QScriptEngineDebuggerFrontend::traceFunction() const
{
    Q_D(const QScriptEngineDebuggerFrontend);
    if (!d->backend)
        return QScriptValue();
    return d->backend->traceFunction();
}

/*!
  Returns a break function. The break function, when called,
  will generate a Break event.
*/
QScriptValue QScriptEngineDebuggerFrontend::breakFunction() const
{
    Q_D(const QScriptEngineDebuggerFrontend);
    if (!d->backend)
        return QScriptValue();
    return d->backend->breakFunction();
}

/*!
  \reimp

  The command is simply forwarded to the back-end.
*/
void QScriptEngineDebuggerFrontend::processCommand(int id, const QScriptDebuggerCommand &command)
{
    Q_D(QScriptEngineDebuggerFrontend);
    if (d->eventLoop && d->eventLoop->isRunning()) {
        // ### post the command?
    }
    QScriptDebuggerResponse response = d->backend->applyCommand(command);
    notifyCommandFinished(id, response);
}
