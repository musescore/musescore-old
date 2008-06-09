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

#include "qscriptdebuggercontrol_p.h"

#include "qscriptdebuggerfrontend.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptbreakpointinfo.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"
#include <QtCore/qfile.h>
#include <QtNetwork/qtcpserver.h>
#include <QtNetwork/qtcpsocket.h>
#include <QtScript/qscriptcontextinfo.h>

class QScriptDebuggerControlPrivate;

void qScriptDebugRegisterMetaTypes();

class QScriptRemoteTargetFrontend : public QScriptDebuggerFrontend
{
    friend class QScriptDebuggerControl;
public:
    QScriptRemoteTargetFrontend(QScriptDebuggerControlPrivate *control);
    ~QScriptRemoteTargetFrontend() {}

protected:
    void processCommand(int id, const QScriptDebuggerCommand &command);

private:
    QScriptDebuggerControlPrivate *control;
};

class QScriptDebuggerControlPrivate
//    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerControl)
public:
    enum TargetType {
        NoTarget,
        Remote,
        Process
    };

    QScriptDebuggerControlPrivate();
    ~QScriptDebuggerControlPrivate();

    void processCommand(int id, const QScriptDebuggerCommand &command);
    void write(const QByteArray &data);
    void initiateHandshake();

    /*
    void _q_emitAttached();
    void _q_emitDetached();
    void _q_engineDestroyed();
    void _q_newConnection();
    void _q_stateChanged(QAbstractSocket::SocketState state);
    void _q_socketError(QAbstractSocket::SocketError error);
    void _q_readyRead();
    */

    QScriptDebuggerControl::State state;

    union {
        QScriptRemoteTargetFrontend *remote;
        QScriptDebuggerFrontend *frontend;
    };
    TargetType type;

    QTcpServer *server;
    QTcpSocket *socket;
    int blockSize;

    QIODevice *input;
    QIODevice *output;

    QScriptDebuggerControl::Error error;
    QString errorString;

    QScriptDebuggerControl *q_ptr; // ###
};

QScriptRemoteTargetFrontend::QScriptRemoteTargetFrontend(QScriptDebuggerControlPrivate *ctrl)
    : QScriptDebuggerFrontend()
{
    qScriptDebugRegisterMetaTypes();
    control = ctrl;
}

void QScriptRemoteTargetFrontend::processCommand(int id, const QScriptDebuggerCommand &command)
{
    control->processCommand(id, command);
}

QScriptDebuggerControlPrivate::QScriptDebuggerControlPrivate()
{
    state = QScriptDebuggerControl::UnattachedState;
    type = NoTarget;
    frontend = 0;
    blockSize = 0;
    server = 0;
    socket = 0;
    input = 0;
    output = 0;
}

QScriptDebuggerControlPrivate::~QScriptDebuggerControlPrivate()
{
    switch (type) {
    case NoTarget:
        break;
    case Remote:
        delete remote;
        delete server;
        break;
    case Process:
        delete remote;
        break;
    }
}

void QScriptDebuggerControlPrivate::processCommand(int id, const QScriptDebuggerCommand &command)
{
//    qDebug() << "processCommand(" << id << command.type() << ")";
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_4);
    out << (quint32)0; // reserve 4 bytes for block size
    out << (qint32)id;
    out << command;
    out.device()->seek(0);
    out << (quint32)(block.size() - sizeof(quint32));
    write(block);
}

void QScriptDebuggerControlPrivate::write(const QByteArray &data)
{
//    qDebug() << "  writing" << block.size() << "bytes";
    QIODevice *dev;
    if (type == Remote)
        dev = socket;
    else {
        Q_ASSERT(type == Process);
        dev = output;
    }
    if (dev->write(data) != data.size())
        Q_ASSERT(0);
    if (qobject_cast<QFile*>(dev))
        ((QFile*)dev)->flush(); // ### NB
}

void QScriptDebuggerControlPrivate::initiateHandshake()
{
//    qDebug("starting handshake");
    state = QScriptDebuggerControl::HandshakingState;
    QByteArray handshakeData("QtScriptDebug-Handshake");
    write(handshakeData);
}

void QScriptDebuggerControl::_q_stateChanged(QAbstractSocket::SocketState s)
{
    Q_D(QScriptDebuggerControl);
//    qDebug() << "stateChanged(" << s << ")";
    if (s == QAbstractSocket::ConnectedState) {
        d->initiateHandshake();
    } else if (s == QAbstractSocket::UnconnectedState) {
        d->state = QScriptDebuggerControl::UnattachedState;
        emit detached();
    } else {
//        qDebug() << "stateChanged(" << s << ")";
    }
}

void QScriptDebuggerControl::_q_socketError(QAbstractSocket::SocketError err)
{
    Q_D(QScriptDebuggerControl);
//    qDebug() << "error(" << err << ")";
    if (err == QAbstractSocket::HostNotFoundError)
        d->error = QScriptDebuggerControl::HostNotFoundError;
    else if (err == QAbstractSocket::ConnectionRefusedError)
        d->error = QScriptDebuggerControl::ConnectionRefusedError;
    else
        d->error = QScriptDebuggerControl::SocketError;
    d->errorString = d->socket->errorString();
    emit error(d->error);
}

/*!
  \internal

  This slot handles data that is received from the debuggee side.
  When we are in the normal (Connected) state, the following steps are
  performed:
  1. Deserialize an event or command response (when we have a complete one)
  2. Notify the front-end (notifyEvent() or notifyCommandFinished())
*/
void QScriptDebuggerControl::_q_readyRead()
{
    Q_D(QScriptDebuggerControl);
    QIODevice *dev = 0;
    if (d->type == QScriptDebuggerControlPrivate::Remote)
        dev = d->socket;
    else {
        Q_ASSERT(d->type == QScriptDebuggerControlPrivate::Process);
        dev = d->input;
    }
    Q_ASSERT(dev);
    switch (d->state) {
    case QScriptDebuggerControl::UnattachedState:
        Q_ASSERT(0);
        break;

    case QScriptDebuggerControl::HandshakingState: {
        QByteArray handshakeData("QtScriptDebug-Handshake");
        if (dev->bytesAvailable() >= handshakeData.size()) {
            QByteArray ba = dev->read(handshakeData.size());
            if (ba == handshakeData) {
//                qDebug("handshake ok!");
                d->remote = new QScriptRemoteTargetFrontend(d);
                d->state = QScriptDebuggerControl::AttachedState;
                QMetaObject::invokeMethod(this, "_q_emitAttached", Qt::QueuedConnection);
                if (dev->bytesAvailable() > 0)
                    QMetaObject::invokeMethod(this, "_q_readyRead", Qt::QueuedConnection);
            } else {
                d->error = QScriptDebuggerControl::HandshakeError;
                d->errorString = QString::fromLatin1("Incorrect handshake data received");
                d->state = QScriptDebuggerControl::UnattachedState;
                emit error(d->error);
                dev->close(); // ###
            }
        }
    }   break;

    case QScriptDebuggerControl::AttachedState: {
//        qDebug() << "got something! bytes available:" << dev->bytesAvailable();
        QDataStream in(dev);
        in.setVersion(QDataStream::Qt_4_4);
        if (d->blockSize == 0) {
            if (dev->bytesAvailable() < (int)sizeof(quint32))
                return;
            in >> d->blockSize;
//            qDebug() << "blockSize:" << d->blockSize;
        }
        if (dev->bytesAvailable() < d->blockSize)
            return;

        quint8 type;
        in >> type;
        if (type == 0) {
            // event
//            qDebug() << "deserializing event";
            QScriptDebuggerEvent event(QScriptDebuggerEvent::None);
            in >> event;
//            qDebug() << "notifying event of type" << event.type();
            d->remote->notifyEvent(event);
        } else {
            // command response
//            qDebug() << "deserializing command response";
            qint32 id;
            in >> id;
            QScriptDebuggerResponse response;
            in >> response;
//            qDebug() << "notifying command" << id << "finished";
            d->remote->notifyCommandFinished((int)id, response);
        }
        d->blockSize = 0;
        if (dev->bytesAvailable() != 0)
            QMetaObject::invokeMethod(this, "_q_readyRead", Qt::QueuedConnection);
    }   break;
    }
}

/*!
  \internal
*/
void QScriptDebuggerControl::_q_emitAttached()
{
    emit attached();
}

/*!
  \internal
*/
void QScriptDebuggerControl::_q_emitDetached()
{
    emit detached();
}

/*!
  \internal
*/
void QScriptDebuggerControl::_q_newConnection()
{
    Q_D(QScriptDebuggerControl);
    if (d->socket)
        delete d->socket;
    d->socket = d->server->nextPendingConnection();
    d->server->close();
    QObject::connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                     this, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
    QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                     this, SLOT(_q_socketError(QAbstractSocket::SocketError)));
    QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
    d->type = QScriptDebuggerControlPrivate::Remote;
    d->initiateHandshake();
}

QScriptDebuggerControl::QScriptDebuggerControl(QObject *parent)
//    : QObject(*new QScriptDebuggerControlPrivate, parent)
    : QObject(parent),
        d_ptr(new QScriptDebuggerControlPrivate)
{
    d_ptr->q_ptr = this;
}

QScriptDebuggerControl::~QScriptDebuggerControl()
{
    delete d_ptr; // ###
}

void QScriptDebuggerControl::attachTo(const QHostAddress &address, quint16 port)
{
    Q_D(QScriptDebuggerControl);
    Q_ASSERT(d->state == UnattachedState);
    d->type = QScriptDebuggerControlPrivate::Remote;
    if (!d->socket) {
        d->socket = new QTcpSocket(this);
        QObject::connect(d->socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                         this, SLOT(_q_stateChanged(QAbstractSocket::SocketState)));
        QObject::connect(d->socket, SIGNAL(error(QAbstractSocket::SocketError)),
                         this, SLOT(_q_socketError(QAbstractSocket::SocketError)));
        QObject::connect(d->socket, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
    }
    d->socket->connectToHost(address, port);
}

void QScriptDebuggerControl::attachTo(QIODevice *in, QIODevice *out)
{
    Q_D(QScriptDebuggerControl);
    Q_ASSERT(d->state == UnattachedState);
    d->type = QScriptDebuggerControlPrivate::Process;
    Q_ASSERT(in->isReadable());
    Q_ASSERT(out->isWritable());
    d->input = in;
    QObject::connect(d->input, SIGNAL(readyRead()), this, SLOT(_q_readyRead()));
    // ### connect to aboutToClose() signal?
    d->output = out;
    d->initiateHandshake();
}

void QScriptDebuggerControl::detach()
{
    Q_D(QScriptDebuggerControl);
    switch (d->type) {
    case QScriptDebuggerControlPrivate::NoTarget:
        break;
    case QScriptDebuggerControlPrivate::Remote:
        d->socket->disconnectFromHost();
        break;
    case QScriptDebuggerControlPrivate::Process:
        // ### todo
        break;
    }
    d->type = QScriptDebuggerControlPrivate::NoTarget;
}

bool QScriptDebuggerControl::listen(const QHostAddress &address, quint16 port)
{
    Q_D(QScriptDebuggerControl);
    if (!d->server) {
        d->server = new QTcpServer();
        QObject::connect(d->server, SIGNAL(newConnection()),
                         this, SLOT(_q_newConnection()));
    }
    return d->server->listen(address, port);
}

bool QScriptDebuggerControl::isListening() const
{
    Q_D(const QScriptDebuggerControl);
    if (!d->server)
        return false;
    return d->server->isListening();
}

QScriptDebuggerControl::Error QScriptDebuggerControl::error() const
{
    Q_D(const QScriptDebuggerControl);
    return d->error;
}

QString QScriptDebuggerControl::errorString() const
{
    Q_D(const QScriptDebuggerControl);
    return d->errorString;
}

QScriptDebuggerFrontend *QScriptDebuggerControl::frontend() const
{
    Q_D(const QScriptDebuggerControl);
    return d->frontend;
}
