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

#include "qscriptdebuggerlauncher.h"

#include "qscriptdebuggerbackend.h"
#include "qscriptdebuggerbackend_p.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include <QtCore/qeventloop.h>
#include <QtCore/qdebug.h>

class QScriptDebuggerLauncherPrivate;

class QScriptProcessBackendPrivate;
class QScriptProcessBackend : public QScriptDebuggerBackend
{
public:
    QScriptProcessBackend(QScriptEngine *engine,
                          QScriptDebuggerLauncherPrivate *launcher);
    ~QScriptProcessBackend();

    void event(const QScriptDebuggerEvent &event);

protected:
    void resume();

private:
    Q_DECLARE_PRIVATE(QScriptProcessBackend)
    Q_DISABLE_COPY(QScriptProcessBackend)
};

class QScriptProcessBackendPrivate : public QScriptDebuggerBackendPrivate
{
    Q_DECLARE_PUBLIC(QScriptProcessBackend)
public:
    QScriptProcessBackendPrivate() {}
    ~QScriptProcessBackendPrivate() {}

    QScriptDebuggerLauncherPrivate *launcher;
};

class QScriptDebuggerLauncherPrivate
//    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerLauncher)
public:
    enum State {
        UnconnectedState,
        HandshakingState,
        ConnectedState
    };

    QScriptDebuggerLauncherPrivate();
    ~QScriptDebuggerLauncherPrivate();

    void event(const QScriptDebuggerEvent &event);
    void resume();

    void _q_stateChanged(QProcess::ProcessState state);
    void _q_error(QProcess::ProcessError error);
    void _q_readyRead();

    int blockSize;
    QScriptEngine *source;
    QScriptDebuggerLauncher::LaunchPolicy policy;
    QEventLoop *eventLoop;
    QProcess *process;
    QScriptProcessBackend *backend;
    State state;
    QScriptDebuggerLauncher::Error error;
    QString errorString;
    QString debugger;
    const QScriptDebuggerEvent *launchReasonEvent;

    QScriptDebuggerLauncher *q_ptr; // ###
};

void qScriptDebugRegisterMetaTypes();

QScriptProcessBackend::QScriptProcessBackend(QScriptEngine *engine,
                                             QScriptDebuggerLauncherPrivate *launcher)
    : QScriptDebuggerBackend(*new QScriptProcessBackendPrivate, engine)
{
    Q_D(QScriptProcessBackend);
    d->launcher = launcher;
    qScriptDebugRegisterMetaTypes();
}

QScriptProcessBackend::~QScriptProcessBackend()
{
}

void QScriptProcessBackend::event(const QScriptDebuggerEvent &event)
{
    Q_D(QScriptProcessBackend);
    d->launcher->event(event);
}

void QScriptProcessBackend::resume()
{
    Q_D(QScriptProcessBackend);
    d->launcher->resume();
}

QScriptDebuggerLauncherPrivate::QScriptDebuggerLauncherPrivate()
{
    blockSize = 0;
    source = 0;
//    policy = QScriptDebuggerLauncher::LaunchOnUncaughtException;
    policy = QScriptDebuggerLauncher::LaunchOnNextStatement;
    process = 0;
    eventLoop = 0;
    backend = 0;
    state = UnconnectedState;
    error = QScriptDebuggerLauncher::UnknownError;
    launchReasonEvent = 0;
}

QScriptDebuggerLauncherPrivate::~QScriptDebuggerLauncherPrivate()
{
    if (process) {
        // ### graceful way to quit the debugger (send an event/command)
    }
}

// ### generalize

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
void QScriptDebuggerLauncherPrivate::event(const QScriptDebuggerEvent &event)
{
    Q_Q(QScriptDebuggerLauncher);
    if (!process) {
        // see if we should launch debugger
        switch (policy) {
            break;
        case QScriptDebuggerLauncher::LaunchOnUncaughtException:
            if (event.type() != QScriptDebuggerEvent::Exception)
                return;
            // fallthrough
        case QScriptDebuggerLauncher::LaunchOnNextStatement:
            // launch it!
            launchReasonEvent = &event;
            process = new QProcess(q);
            QObject::connect(process, SIGNAL(stateChanged(QProcess::ProcessState)),
                             q, SLOT(_q_stateChanged(QProcess::ProcessState)));
            QObject::connect(process, SIGNAL(error(QProcess::ProcessError)),
                             q, SLOT(_q_error(QProcess::ProcessError)));
            QObject::connect(process, SIGNAL(readyRead()), q, SLOT(_q_readyRead()));
            process->start(debugger, QStringList() << "--stdinout");
        }
    } else {
        Q_ASSERT(state == ConnectedState);
//        qDebug() << "serializing event of type" << event.type();
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_4);
        out << (quint32)0; // reserve 4 bytes for block size
        out << (quint8)0;  // type = event
        out << event;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));
        qDebug() << "writing event (" << block.size() << " bytes )";
        process->write(block);
    }
    // run an event loop until the debugger triggers a resume
    if (!eventLoop)
        eventLoop = new QEventLoop();
    else if (eventLoop->isRunning())
        return;
    qDebug("entering event loop");
    eventLoop->exec();
    qDebug("returned from event loop");
}

/*!
  \internal
*/
void QScriptDebuggerLauncherPrivate::resume()
{
    // quit the event loop so the engine will continue running
    if (eventLoop)
        eventLoop->quit();
}

void QScriptDebuggerLauncherPrivate::_q_stateChanged(QProcess::ProcessState s)
{
    qDebug() << "stateChanged(" << s << ")";
    if (s == QProcess::Running) {
        state = HandshakingState;
    } else if (s == QProcess::NotRunning) {
        state = UnconnectedState;
        resume();
        // ### emit finished()?
    }
}

void QScriptDebuggerLauncherPrivate::_q_error(QProcess::ProcessError err)
{
    Q_Q(QScriptDebuggerLauncher);
    qDebug() << "processError(" << err << ")";
    // ### fixme
    error = QScriptDebuggerLauncher::UnknownError;
    errorString = process->errorString();
    emit q->error(error);
}

/*!
  \internal

  This slot handles data that is received from the debugger process.
*/
void QScriptDebuggerLauncherPrivate::_q_readyRead()
{
//    qDebug("readyRead()");
    // ### generalize
    Q_Q(QScriptDebuggerLauncher);
    switch (state) {
    case UnconnectedState:
        Q_ASSERT(0);
        break;

    case HandshakingState: {
        QByteArray handshakeData("QtScriptDebug-Handshake");
        if (process->bytesAvailable() == handshakeData.size()) {
            QByteArray ba = process->read(handshakeData.size());
            if (ba == handshakeData) {
                qDebug("sending handshake reply");
                process->write(handshakeData);
                state = ConnectedState;
//                emit q->connected();
                // now we can send the initial event to the debugger
                event(*launchReasonEvent);
                Q_ASSERT(process->bytesAvailable() == 0);
            } else {
                error = QScriptDebuggerLauncher::HandshakeError;
                errorString = QString::fromLatin1("Incorrect handshake data received");
                // ### what state do we enter?
                state = UnconnectedState;
//                emit q->error(error);
            }
        }
    }   break;

    case ConnectedState: {
        QDataStream in(process);
        in.setVersion(QDataStream::Qt_4_4);
        if (blockSize == 0) {
            if (process->bytesAvailable() < (int)sizeof(quint32))
                return;
            in >> blockSize;
        }
        if (process->bytesAvailable() < blockSize)
            return;

        qint32 id;
        in >> id;
        QScriptDebuggerCommand command(QScriptDebuggerCommand::None);
        in >> command;

        QScriptDebuggerResponse response = backend->applyCommand(command);

        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_4);
        out << (quint32)0; // reserve 4 bytes for block size
        out << (quint8)1;  // type = command response
        out << id;
        out << response;
        out.device()->seek(0);
        out << (quint32)(block.size() - sizeof(quint32));
        process->write(block);
        blockSize = 0;

        if (process->bytesAvailable() != 0)
            QMetaObject::invokeMethod(q, "readyRead", Qt::QueuedConnection);
    }   break;

    }
}

QScriptDebuggerLauncher::QScriptDebuggerLauncher(QObject *parent)
//    : QObject(*new QScriptDebuggerLauncherPrivate, parent)
    : QObject(parent),
        d_ptr(new QScriptDebuggerLauncherPrivate)
{
    d_ptr->q_ptr = this;
}

QScriptDebuggerLauncher::~QScriptDebuggerLauncher()
{
    delete d_ptr; // ###
}

void QScriptDebuggerLauncher::setSource(QScriptEngine *engine)
{
    Q_D(QScriptDebuggerLauncher);
    d->source = engine;
    d->backend = new QScriptProcessBackend(engine, d);
    if (d->policy == LaunchOnNextStatement)
        d->backend->setState(QScriptDebuggerBackend::PausingState);
    else
        d->backend->setState(QScriptDebuggerBackend::NoState);
}

QScriptEngine *QScriptDebuggerLauncher::source() const
{
    Q_D(const QScriptDebuggerLauncher);
    return d->source;
}

QScriptDebuggerLauncher::LaunchPolicy QScriptDebuggerLauncher::launchPolicy() const
{
    Q_D(const QScriptDebuggerLauncher);
    return d->policy;
}

void QScriptDebuggerLauncher::setLaunchPolicy(LaunchPolicy policy)
{
    Q_D(QScriptDebuggerLauncher);
    d->policy = policy;
    if (d->policy == LaunchOnNextStatement)
        d->backend->setState(QScriptDebuggerBackend::PausingState);
    else
        d->backend->setState(QScriptDebuggerBackend::NoState);
}

QString QScriptDebuggerLauncher::debugger() const
{
    Q_D(const QScriptDebuggerLauncher);
    return d->debugger;
}

void QScriptDebuggerLauncher::setDebugger(const QString &path)
{
    Q_D(QScriptDebuggerLauncher);
    d->debugger = path;
}

#include "moc_qscriptdebuggerlauncher.h"

