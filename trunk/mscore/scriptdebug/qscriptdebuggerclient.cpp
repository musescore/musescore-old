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

#include "qscriptdebuggerclient.h"

#include "qscriptdebuggerclient_p.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include <QtCore/qcoreapplication.h>
#include <QtCore/qcoreevent.h>

/*!
  \class QScriptDebuggerClient

  \brief The QScriptDebuggerClient class provides an interface for handling debugger events and command responses.

  Call QScriptDebuggerFrontend::setClient() to set a debugger
  front-end's active client.

  \section1 Subclassing

  When subclassing QScriptDebuggerClient, there are two pure virtual
  functions you must implement: commandFinished() and
  event(). commandFinished() will be invoked every time the debugger
  has finished executing a command. Typically, the client will want to
  convey to the user of the debugger that the command was finished
  (e.g. by outputting some text or updating the GUI), and/or issue
  more commands. event() will be invoked to let the client know that a
  debugging event has occured; for example, when a script statement
  has been stepped over. Similarly, the client will at this point
  want to let the user know what happened and give him a way to
  issue new commands (e.g. to resume execution).

  \sa QScriptDebuggerFrontend
*/

QScriptDebuggerClientPrivate::QScriptDebuggerClientPrivate()
{
}

QScriptDebuggerClientPrivate::~QScriptDebuggerClientPrivate()
{
}



class QScriptDebuggerEventEvent : public QEvent
{
public:
    QScriptDebuggerEventEvent(const QScriptDebuggerEvent &event);
    ~QScriptDebuggerEventEvent();

    const QScriptDebuggerEvent &event() const;
    QVariant argument() const;

private:
    QScriptDebuggerEvent m_event;
};

QScriptDebuggerEventEvent::QScriptDebuggerEventEvent(const QScriptDebuggerEvent &event)
    : QEvent(QEvent::Type(QEvent::User+1)), m_event(event)
{
}

QScriptDebuggerEventEvent::~QScriptDebuggerEventEvent()
{
}

const QScriptDebuggerEvent &QScriptDebuggerEventEvent::event() const
{
    return m_event;
}



class QScriptDebuggerCommandFinishedEvent : public QEvent
{
public:
    QScriptDebuggerCommandFinishedEvent(int id, const QScriptDebuggerResponse &response);
    ~QScriptDebuggerCommandFinishedEvent();

    int id() const;
    QScriptDebuggerResponse response() const;

private:
    int m_id;
    QScriptDebuggerResponse m_response;
};

QScriptDebuggerCommandFinishedEvent::QScriptDebuggerCommandFinishedEvent(
    int id, const QScriptDebuggerResponse &response)
    : QEvent(QEvent::Type(QEvent::User+2)), m_id(id), m_response(response)
{
}

QScriptDebuggerCommandFinishedEvent::~QScriptDebuggerCommandFinishedEvent()
{
}

int QScriptDebuggerCommandFinishedEvent::id() const
{
    return m_id;
}

QScriptDebuggerResponse QScriptDebuggerCommandFinishedEvent::response() const
{
    return m_response;
}



bool QScriptDebuggerClientPrivate::event(QEvent *e)
{
    Q_Q(QScriptDebuggerClient);
    if (e->type() == QEvent::User+1) {
        // event notification
        QScriptDebuggerEventEvent *de = static_cast<QScriptDebuggerEventEvent*>(e);
        q->event(de->event());
        return true;
    } else if (e->type() == QEvent::User+2) {
        // command finished notification
        QScriptDebuggerCommandFinishedEvent *fe;
        fe = static_cast<QScriptDebuggerCommandFinishedEvent*>(e);
        q->commandFinished(fe->id(), fe->response());
        return true;
    }
    return false;
}

void QScriptDebuggerClientPrivate::postEvent(const QScriptDebuggerEvent &event)
{
    QScriptDebuggerEventEvent *e = new QScriptDebuggerEventEvent(event);
    QCoreApplication::postEvent(this, e);
}

void QScriptDebuggerClientPrivate::postCommandFinished(
    int id, const QScriptDebuggerResponse &response)
{
    QScriptDebuggerCommandFinishedEvent *e = new QScriptDebuggerCommandFinishedEvent(id, response);
    QCoreApplication::postEvent(this, e);
}

QScriptDebuggerClientPrivate *QScriptDebuggerClientPrivate::get(QScriptDebuggerClient *q)
{
    if (!q)
        return 0;
    return q->d_func();
}

/*!
  Constructs a QScriptDebuggerClient object associated with the given
  \a frontend.
*/
QScriptDebuggerClient::QScriptDebuggerClient(QScriptDebuggerFrontend *frontend)
    : d_ptr(new QScriptDebuggerClientPrivate)
{
    d_ptr->q_ptr = this;
    d_ptr->frontend = frontend;
}

/*!
  \internal
*/
QScriptDebuggerClient::QScriptDebuggerClient(QScriptDebuggerFrontend *frontend,
                                             QScriptDebuggerClientPrivate &dd)
    : d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    d_ptr->frontend = frontend;
}

/*!
  Destroys this QScriptDebuggerClient.
*/
QScriptDebuggerClient::~QScriptDebuggerClient()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns the front-end that this client is associated with.
*/
QScriptDebuggerFrontend *QScriptDebuggerClient::frontend() const
{
    Q_D(const QScriptDebuggerClient);
    return d->frontend;
}

/*!
  \fn void QScriptDebuggerClient::commandFinished(int id, const QScriptDebuggerResponse &response)

  This function is called when the debugger has finished executing the
  command identified by the given \a id, and has produced the given \a
  response.

  Reimplement this function to handle command completion.
*/

/*!
  \fn void QScriptDebuggerClient::event(const QScriptDebuggerEvent &event)

  This function is called when the given debugger \a event has occurred.

  Reimplement this function to handle events.
*/
