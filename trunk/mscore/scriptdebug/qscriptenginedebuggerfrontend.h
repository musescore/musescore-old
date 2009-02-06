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

#ifndef QSCRIPTENGINEDEBUGGERFRONTEND_H
#define QSCRIPTENGINEDEBUGGERFRONTEND_H

#include "qscriptdebuggerfrontend.h"

class QScriptDebuggerCommand;
class QScriptEngine;
class QScriptValue;

class QScriptEngineDebuggerFrontendPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptEngineDebuggerFrontend : public QScriptDebuggerFrontend
{
public:
    QScriptEngineDebuggerFrontend();
    ~QScriptEngineDebuggerFrontend();

    void attachTo(QScriptEngine *engine);
    void detach();

    QScriptEngine *engine() const;

    void breakAtFirstStatement();

    QScriptValue traceFunction() const;
    QScriptValue breakFunction() const;

protected:
    void processCommand(int id, const QScriptDebuggerCommand &command);

    QScriptEngineDebuggerFrontend(QScriptEngineDebuggerFrontendPrivate &dd);

private:
    Q_DECLARE_PRIVATE(QScriptEngineDebuggerFrontend)
    Q_DISABLE_COPY(QScriptEngineDebuggerFrontend)
};

#endif
