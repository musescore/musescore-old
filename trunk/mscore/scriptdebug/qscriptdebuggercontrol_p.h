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

#ifndef QSCRIPTDEBUGGERCONTROL_P_H
#define QSCRIPTDEBUGGERCONTROL_P_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobject.h>

#include <QtNetwork/qabstractsocket.h>

class QHostAddress;
class QIODevice;
class QScriptDebuggerFrontend;

// ### kill / refactor

class QScriptDebuggerControlPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerControl : public QObject
{
    Q_OBJECT
public:
    enum State {
        UnattachedState,
        HandshakingState,
        AttachedState
    };

    enum Error {
        NoError,
        HostNotFoundError,
        ConnectionRefusedError,
        HandshakeError,
        SocketError
    };

    QScriptDebuggerControl(QObject *parent = 0);
    ~QScriptDebuggerControl();

    void attachTo(const QHostAddress &address, quint16 port);
    void attachTo(QIODevice *in, QIODevice *out);
    void detach();

    bool listen(const QHostAddress &address, quint16 port);
    bool isListening() const;

    State state() const;

    Error error() const;
    QString errorString() const;

    QScriptDebuggerFrontend *frontend() const;

signals:
    void attached();
    void detached();
    void error(QScriptDebuggerControl::Error error);

private slots:
    void _q_emitAttached();
    void _q_emitDetached();
    void _q_newConnection();
    void _q_stateChanged(QAbstractSocket::SocketState);
    void _q_socketError(QAbstractSocket::SocketError);
    void _q_readyRead();

private:
    QScriptDebuggerControlPrivate *d_ptr; // ###

    Q_DECLARE_PRIVATE(QScriptDebuggerControl)
    Q_DISABLE_COPY(QScriptDebuggerControl)
};

#endif
