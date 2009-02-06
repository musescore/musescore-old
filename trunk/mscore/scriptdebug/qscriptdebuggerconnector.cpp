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

#include "qscriptdebuggerconnector.h"

#include "qscriptdebuggerbackend.h"
#include "qscriptdebuggerbackend_p.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtCore/qeventloop.h>
#include <QtScript/qscriptcontext.h>
#include <QtScript/qscriptengine.h>
//#include <private/qobject_p.h>

/*!
  \class QScriptDebuggerConnector

  \brief The QScriptDebuggerConnector class provides a way to establish a connection with a debugger.

  Call the setEngine() function to set the QScriptEngine object that
  the connection will apply to.

  Call connectToDebugger() to attempt to establish a connection to a debugger.
  The connected() signal is emitted when the connection has been established;
  error() is emitted if the connection attempt failed.

  Call listen() to listen for an incoming connection from a debugger.
  The connected() signal is emitted when a connection has been established.

  Once a connection has been established, you can use the script
  engine like normal, e.g. start evaluating scripts. All further
  communication with the debugger happens behind the scenes (for
  example, the debugger is notified when an uncaught script exception
  occurs).

  Two functions are provided that allow you to instrument scripts for
  debugging purposes: traceFunction() returns a script function that,
  when called, will generate a Trace event. breakFunction() returns a
  script function that, when called, will generate a Break event.
*/

//#define DEBUGGERCONNECTOR_DEBUG

class QScriptDebuggerConnectorPrivate;

class QScriptRemoteTargetBackendPrivate;
class QScriptRemoteTargetBackend : public QScriptDebuggerBackend
{
public:
    QScriptRemoteTargetBackend(QScriptEngine *engine,
                               QScriptDebuggerConnectorPrivate *connector);
    ~QScriptRemoteTargetBackend();

    void event(const QScriptDebuggerEvent &event);

protected:
    void resume();

private:
    Q_DECLARE_PRIVATE(QScriptRemoteTargetBackend)
    Q_DISABLE_COPY(QScriptRemoteTargetBackend)
};

class QScriptRemoteTargetBackendPrivate : public QScriptDebuggerBackendPrivate
{
    Q_DECLARE_PUBLIC(QScriptRemoteTargetBackend)
public:
    QScriptRemoteTargetBackendPrivate();
    ~QScriptRemoteTargetBackendPrivate();

    QScriptDebuggerConnectorPrivate *connector;
};

class QScriptDebuggerConnectorPrivate
//    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerConnector)
public:
    enum State {
        UnconnectedState,
        HandshakingState,
        ConnectedState
    };

    QScriptDebuggerConnectorPrivate();
    ~QScriptDebuggerConnectorPrivate();

    void event(const QScriptDebuggerEvent &event);
    void resume();

    QScriptEngine *engine;
    QTcpServer *server;
    QTcpSocket *socket;
    State state;
    int blockSize;
    QScriptRemoteTargetBackend *backend;
    QEventLoop *eventLoop;
    QScriptDebuggerConnector::Error error;
    QString errorString;

    QScriptDebuggerConnector *q_ptr; // ###
};

QScriptRemoteTargetBackendPrivate::QScriptRemoteTargetBackendPrivate()
{
}

QScriptRemoteTargetBackendPrivate::~QScriptRemoteTargetBackendPrivate()
{
}

void qScriptDebugRegisterMetaTypes();

QScriptRemoteTargetBackend::QScriptRemoteTargetBackend(QScriptEngine *engine,
                                                       QScriptDebuggerConnectorPrivate *connector)
    : QScriptDebuggerBackend(*new QScriptRemoteTargetBackendPrivate, engine)
{
    Q_D(QScriptRemoteTargetBackend);
    d->connector = connector;
    qScriptDebugRegisterMetaTypes();
}

QScriptRemoteTargetBackend::~QScriptRemoteTargetBackend()
{
}

/*!
  \reimp
*/
void QScriptRemoteTargetBackend::event(const QScriptDebuggerEvent &event)
{
    Q_D(QScriptRemoteTargetBackend);
    // forward to connector
    d->connector->event(event);
}

/*!
  \reimp
*/
void QScriptRemoteTargetBackend::resume()
{
    Q_D(QScriptRemoteTargetBackend);
    // forward to connector
    d->connector->resume();
}



QScriptDebuggerConnectorPrivate::QScriptDebuggerConnectorPrivate()
{
    state = UnconnectedState;
    blockSize = 0;
    engine = 0;
    server = 0;
    socket = 0;
    backend = 0;
    error = QScriptDebuggerConnector::NoError;
}

QScriptDebuggerConnectorPrivate::~QScriptDebuggerConnectorPrivate()
{
    delete server;
    if (eventLoop) {
        eventLoop->quit();
        eventLoop->deleteLater();
    }
}

void QScriptDebuggerConnector::_q_stateChanged(QAbstractSocket::SocketState s)
{
    Q_D(QScriptDebuggerConnector);
    if (s == QAbstractSocket::ConnectedState) {
        d->state = QScriptDebuggerConnectorPrivate::HandshakingState;
    } else if (s == QAbstractSocket::UnconnectedState) {
        d->engine->setAgent(0);
        // ### do we really want to kill the back-end?
        d->backend = 0; // owned by engine
        d->state = QScriptDebuggerConnectorPrivate::UnconnectedState;
        emit disconnected();
    }
}

void QScriptDebuggerConnector::_q_error(QAbstractSocket::SocketError err)
{
    Q_D(QScriptDebuggerConnector);
    if (err == QAbstractSocket::HostNotFoundError)
        d->error = QScriptDebuggerConnector::HostNotFoundError;
    else if (err == QAbstractSocket::ConnectionRefusedError)
        d->error = QScriptDebuggerConnector::ConnectionRefusedError;
    else
        d->error = QScriptDebuggerConnector::SocketError;
    d->errorString = d->socket->errorString();
    emit error(QScriptDebuggerConnector::SocketError);
}

void QScriptDebuggerConnector::_q_newConnection()
{
    Q_D(QScriptDebuggerConnector);
    d->socket = d->server->nextPendingConnection();
    d->server->close();
    QObject::connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                     this, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
    QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(_q_error(QAbstractSocket::SocketError)));
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
    // the handshake is initiated by the debugger side, so wait for it
    d->state = QScriptDebuggerConnectorPrivate::HandshakingState;
}

/*!
  \internal

  This slot handles data that is received from the debugger side.
  When we are in the normal (Connected) state, the following steps are
  performed:
  1. Deserialize a command (when we have a complete one)
  2. Execute the command in the back-end
  3. Serialize the response
  4. Write the serialized response back to the debugger
*/
void QScriptDebuggerConnector::_q_readyRead()
{
    Q_D(QScriptDebuggerConnector);
    switch (d->state) {
    case QScriptDebuggerConnectorPrivate::UnconnectedState:
        Q_ASSERT(0);
        break;

    case QScriptDebuggerConnectorPrivate::HandshakingState: {
        QByteArray handshakeData("QtScriptDebug-Handshake");
        if (d->socket->bytesAvailable() == handshakeData.size()) {
            QByteArray ba = d->socket->read(handshakeData.size());
            if (ba == handshakeData) {
#ifdef DEBUGGERCONNECTOR_DEBUG
                qDebug() << "sending handshake reply (" << handshakeData.size() << "bytes )";
#endif
                d->socket->write(handshakeData);
                // handshaking complete, initialize back-end
                if (!d->backend)
                    d->backend = new QScriptRemoteTargetBackend(d->engine, d);
                d->engine->setAgent(d->backend);
                // ### a way to specify if a break should be triggered immediately,
                // or only if an uncaught exception is triggered
                d->backend->pauseEvaluation();
                d->state = QScriptDebuggerConnectorPrivate::ConnectedState;
                emit connected();
            } else {
                d->error = QScriptDebuggerConnector::HandshakeError;
                d->errorString = QString::fromLatin1("Incorrect handshake data received");
                d->state = QScriptDebuggerConnectorPrivate::UnconnectedState;
                emit error(d->error);
                d->socket->close(); // ### hmm
            }
        }
    }   break;

    case QScriptDebuggerConnectorPrivate::ConnectedState: {
#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "received data. bytesAvailable:" << d->socket->bytesAvailable();
#endif
        QDataStream in(d->socket);
        in.setVersion(QDataStream::Qt_4_4);
        if (d->blockSize == 0) {
            if (d->socket->bytesAvailable() < (int)sizeof(quint32))
                return;
            in >> d->blockSize;
#ifdef DEBUGGERCONNECTOR_DEBUG
            qDebug() << "  blockSize:" << d->blockSize;
#endif
        }
        if (d->socket->bytesAvailable() < d->blockSize)
            return;

#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "deserializing command";
#endif
        qint32 id;
        in >> id;
        QScriptDebuggerCommand command(QScriptDebuggerCommand::None);
        in >> command;

#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "executing command of type" << command.type();
#endif
        QScriptDebuggerResponse response = d->backend->applyCommand(command);

#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "serializing response";
#endif
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_4);
        out << (quint32)0; // reserve 4 bytes for block size
        out << (quint8)1;  // type = command response
        out << id;
        out << response;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));
#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "writing response (" << block.size() << "bytes )";
#endif
        d->socket->write(block);
        d->blockSize = 0;

#ifdef DEBUGGERCONNECTOR_DEBUG
        qDebug() << "bytes available is now" << d->socket->bytesAvailable();
#endif
        if (d->socket->bytesAvailable() != 0)
            QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
    }   break;

    }
}

/*!
  \internal

  This callback handles a debugging event that has happened in the
  back-end. The following steps are performed:
  1. Serialize the event
  2. Write the serialized event to the debugger
  3. Run event loop

  The event loop will quit when the debugger sends a command telling
  the back-end to resume.
*/
void QScriptDebuggerConnectorPrivate::event(const QScriptDebuggerEvent &event)
{
    if (state != ConnectedState)
        return;
    if (eventLoop && eventLoop->isRunning()) {
        qWarning("QScriptDebuggerConnector: oops, recursive event notification detected");
        resume();
        return;
    }
#ifdef DEBUGGERCONNECTOR_DEBUG
    qDebug() << "serializing event of type" << event.type();
#endif
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);
    out << (quint32)0; // reserve 4 bytes for block size
    out << (quint8)0;  // type = event
    out << event;
    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));

#ifdef DEBUGGERCONNECTOR_DEBUG
    qDebug() << "writing event (" << block.size() << " bytes )";
#endif
    socket->write(block);

    if (event.type() == QScriptDebuggerEvent::Trace)
        return;

    // run an event loop until the debugger triggers a resume
    if (!eventLoop)
        eventLoop = new QEventLoop();
#ifdef DEBUGGERCONNECTOR_DEBUG
    qDebug("entering event loop");
#endif
    eventLoop->exec();
#ifdef DEBUGGERCONNECTOR_DEBUG
    qDebug("returned from event loop");
#endif
}

/*!
  \internal
*/
void QScriptDebuggerConnectorPrivate::resume()
{
    // quit the event loop so the engine will continue running
    if (eventLoop)
        eventLoop->quit();
}

/*!
  Constructs a new QScriptDebuggerConnector object with the given \a
  parent.
*/
QScriptDebuggerConnector::QScriptDebuggerConnector(QObject *parent)
//    : QObject(*new QScriptDebuggerConnectorPrivate, parent)
    : QObject(parent),
        d_ptr(new QScriptDebuggerConnectorPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
  Destroys this QScriptDebuggerConnector.
*/
QScriptDebuggerConnector::~QScriptDebuggerConnector()
{
    delete d_ptr; // ###
}

/*!
  Sets the \a engine that this connector will manage a connection to.

  You should call this function before calling connectToDebugger() or
  listen(). Calling this function when a connection has already been
  established has no effect.
*/
void QScriptDebuggerConnector::setEngine(QScriptEngine *engine)
{
    Q_D(QScriptDebuggerConnector);
    if (d->state == QScriptDebuggerConnectorPrivate::UnconnectedState)
        d->engine = engine;
}

/*!
  Returns the \a engine that this connector manages a connection to,
  or 0 if no engine has been set.
*/
QScriptEngine *QScriptDebuggerConnector::engine() const
{
    Q_D(const QScriptDebuggerConnector);
    return d->engine;
}

/*!
  Attempts to make a connection to the given \a address on the given
  \a port.

  The connected() signal is emitted when the connection has been
  established.

  \sa disconnectFromDebugger(), listen()
*/
void QScriptDebuggerConnector::connectToDebugger(const QHostAddress &address, quint16 port)
{
    Q_D(QScriptDebuggerConnector);
    if (d->state != QScriptDebuggerConnectorPrivate::UnconnectedState)
        return;
    if (!d->engine) {
        qWarning("QScriptDebuggerConnector::connectToDebugger(): no engine has been set (call setEngine() first)");
        return;
    }
    if (!d->socket) {
        d->socket = new QTcpSocket(this);
        QObject::connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                         this, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
        QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                         this, SLOT(_q_error(QAbstractSocket::SocketError)));
        QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
    }
    d->socket->connectToHost(address, port);
}

/*!
  Attempts to close the connection.

  The disconnected() signal is emitted when the connection has been
  closed.

  \sa connectToDebugger()
*/
void QScriptDebuggerConnector::disconnectFromDebugger()
{
    Q_D(QScriptDebuggerConnector);
    if (!d->socket)
        return;
    d->socket->disconnectFromHost();
}

/*!
  Listens for an incoming connection on the given \a address and \a
  port.

  Returns true on success; otherwise returns false.

  The connected() signal is emitted when a connection has been
  established.

  \sa isListening(), connectToDebugger()
*/
bool QScriptDebuggerConnector::listen(const QHostAddress &address, quint16 port)
{
    Q_D(QScriptDebuggerConnector);
    if (d->state != QScriptDebuggerConnectorPrivate::UnconnectedState)
        return false;
    if (!d->engine) {
        qWarning("QScriptDebuggerConnector::listen(): no engine has been set (call setEngine() first)");
        return false;
    }
    if (d->socket)
        return false;
    if (!d->server) {
        d->server = new QTcpServer();
        QObject::connect(d->server, SIGNAL(newConnection()),
                         this, SLOT(_q_newConnection()));
    }
    return d->server->listen(address, port);
}

/*!
  Returns true if the connector is currently listening for an incoming
  connection; otherwise returns false.

  \sa listen()
*/
bool QScriptDebuggerConnector::isListening() const
{
    Q_D(const QScriptDebuggerConnector);
    if (!d->server)
        return false;
    return d->server->isListening();
}

/*!
  Returns the type of error that last occurred.

  \sa errorString()
*/
QScriptDebuggerConnector::Error QScriptDebuggerConnector::error() const
{
    Q_D(const QScriptDebuggerConnector);
    return d->error;
}

/*!
  Returns a human-readable description of the last error that occurred.

  \sa error()
*/
QString QScriptDebuggerConnector::errorString() const
{
    Q_D(const QScriptDebuggerConnector);
    return d->errorString;
}

/*!
  Returns a function that can be used to send text output to the debugger.
  The function has the same semantics as the built-in print() function,
  except that the resulting string is sent to the debugger rather than
  to standard output.
*/
QScriptValue QScriptDebuggerConnector::traceFunction() const
{
    Q_D(const QScriptDebuggerConnector);
    if (!d->engine)
        return QScriptValue();
    if (!d->backend) {
        QScriptDebuggerConnectorPrivate *dd = const_cast<QScriptDebuggerConnectorPrivate*>(d);
        dd->backend = new QScriptRemoteTargetBackend(d->engine, dd);
    }
    return d->backend->traceFunction();
}

/*!
  Returns a break function.

  The break function can be used to trigger the debugger from a
  script.
*/
QScriptValue QScriptDebuggerConnector::breakFunction() const
{
    Q_D(const QScriptDebuggerConnector);
    if (!d->engine)
        return QScriptValue();
    if (!d->backend) {
        QScriptDebuggerConnectorPrivate *dd = const_cast<QScriptDebuggerConnectorPrivate*>(d);
        dd->backend = new QScriptRemoteTargetBackend(d->engine, dd);
    }
    return d->backend->breakFunction();
}
