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

#ifndef QSCRIPTCONSOLEDEBUGGER_H
#define QSCRIPTCONSOLEDEBUGGER_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobject.h>

class QScriptDebuggerFrontend;

class QScriptConsoleDebuggerPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptConsoleDebugger : public QObject
{
    Q_OBJECT
public:
    QScriptConsoleDebugger(QObject *parent = 0);
    ~QScriptConsoleDebugger();

    QScriptDebuggerFrontend *frontend() const;
    void setFrontend(QScriptDebuggerFrontend *frontend);

private slots:
    void _q_prompt();

private:
    QScriptConsoleDebuggerPrivate *d_ptr; // ###

    Q_DECLARE_PRIVATE(QScriptConsoleDebugger)
    Q_DISABLE_COPY(QScriptConsoleDebugger)
};

#endif
