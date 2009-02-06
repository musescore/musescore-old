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

#ifndef QSCRIPTDEBUGGERCONNECTOR_H
#define QSCRIPTDEBUGGERCONNECTOR_H

#include <QtCore/qobject.h>

#include <QtNetwork/qhostaddress.h>
#include <QtNetwork/qabstractsocket.h>

class QScriptEngine;
class QScriptValue;

class QScriptDebuggerConnectorPrivate;
class QScriptDebuggerConnector : public QObject
{
Q_OBJECT
public:
    enum Error {
        NoError,
        HostNotFoundError,
        ConnectionRefusedError,
        HandshakeError,
        SocketError
    };

    QScriptDebuggerConnector(QObject *parent = 0);
    ~QScriptDebuggerConnector();

    void setEngine(QScriptEngine *engine);
    QScriptEngine *engine() const;

    void connectToDebugger(const QHostAddress &address, quint16 port);
    void disconnectFromDebugger();

    bool listen(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);
    bool isListening() const;

    Error error() const;
    QString errorString() const;

    QScriptValue traceFunction() const;
    QScriptValue breakFunction() const;

private slots:
    void _q_stateChanged(QAbstractSocket::SocketState);
    void _q_error(QAbstractSocket::SocketError);
    void _q_newConnection();
    void _q_readyRead();

signals:
    void connected();
    void disconnected();
    void error(Error error);

private:
    QScriptDebuggerConnectorPrivate *d_ptr; // ###

    Q_DECLARE_PRIVATE(QScriptDebuggerConnector)
    Q_DISABLE_COPY(QScriptDebuggerConnector)
/*
    Q_PRIVATE_SLOT(d_func(), void _q_stateChanged(QAbstractSocket::SocketState))
    Q_PRIVATE_SLOT(d_func(), void _q_error(QAbstractSocket::SocketError))
    Q_PRIVATE_SLOT(d_func(), void _q_newConnection())
    Q_PRIVATE_SLOT(d_func(), void _q_readyRead())*/
};

#endif
