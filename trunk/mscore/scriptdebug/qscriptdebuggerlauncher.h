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

#ifndef QSCRIPTDEBUGGERLAUNCHER_H
#define QSCRIPTDEBUGGERLAUNCHER_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobject.h>

#include <QtCore/qprocess.h>

class QScriptEngine;

class QScriptDebuggerLauncherPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerLauncher : public QObject
{
    Q_OBJECT
public:
    enum Error {
        HandshakeError,
        UnknownError
    };

    enum LaunchPolicy {
        LaunchOnNextStatement,
        LaunchOnUncaughtException
    };

    QScriptDebuggerLauncher(QObject *parent = 0);
    ~QScriptDebuggerLauncher();

    void setSource(QScriptEngine *engine);
    QScriptEngine *source() const;

    LaunchPolicy launchPolicy() const;
    void setLaunchPolicy(LaunchPolicy policy);

    QString debugger() const;
    void setDebugger(const QString &path);

signals:
    void launched();
    void error(QScriptDebuggerLauncher::Error error);

private:
    QScriptDebuggerLauncherPrivate *d_ptr; // ###

    Q_DECLARE_PRIVATE(QScriptDebuggerLauncher)
    Q_DISABLE_COPY(QScriptDebuggerLauncher)

    Q_PRIVATE_SLOT(d_func(), void _q_stateChanged(QProcess::ProcessState))
    Q_PRIVATE_SLOT(d_func(), void _q_error(QProcess::ProcessError))
    Q_PRIVATE_SLOT(d_func(), void _q_readyRead())
};

#endif
