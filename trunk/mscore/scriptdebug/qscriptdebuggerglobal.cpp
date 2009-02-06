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

#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"
#include "qscriptbreakpointinfo.h"
#include <QtScript/qscriptcontextinfo.h>

Q_DECLARE_METATYPE(QScriptDebuggerCommand)
Q_DECLARE_METATYPE(QScriptDebuggerResponse)
Q_DECLARE_METATYPE(QScriptDebuggerEvent)
Q_DECLARE_METATYPE(QScriptContextInfo)
Q_DECLARE_METATYPE(QScriptContextInfoList)
Q_DECLARE_METATYPE(QScriptDebuggerValue)
Q_DECLARE_METATYPE(QScriptDebuggerValueList)
Q_DECLARE_METATYPE(QScriptDebuggerProperty)
Q_DECLARE_METATYPE(QScriptDebuggerPropertyList)
Q_DECLARE_METATYPE(QScriptValue::PropertyFlags)
Q_DECLARE_METATYPE(QScriptBreakpointInfo)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<qint64>)

void qScriptDebugRegisterMetaTypes()
{
    qMetaTypeId<QScriptDebuggerCommand>();
    qMetaTypeId<QScriptDebuggerResponse>();
    qMetaTypeId<QScriptDebuggerEvent>();
    qMetaTypeId<QScriptContextInfo>();
    qMetaTypeId<QScriptContextInfoList>();
    qMetaTypeId<QScriptDebuggerValue>();
    qMetaTypeId<QScriptDebuggerValueList>();
    qMetaTypeId<QScriptDebuggerProperty>();
    qMetaTypeId<QScriptDebuggerPropertyList>();
    qMetaTypeId<QScriptValue::PropertyFlags>();
    qMetaTypeId<QScriptBreakpointInfo>();
    qMetaTypeId<QList<int> >();
    qMetaTypeId<QList<qint64> >();

    qRegisterMetaTypeStreamOperators<QScriptDebuggerCommand>("QScriptDebuggerCommand");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerResponse>("QScriptDebuggerResponse");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerEvent>("QScriptDebuggerEvent");
    qRegisterMetaTypeStreamOperators<QScriptContextInfo>("QScriptContextInfo");
    qRegisterMetaTypeStreamOperators<QScriptContextInfoList>("QScriptContextInfoList");
    qRegisterMetaTypeStreamOperators<QScriptBreakpointInfo>("QScriptBreakpointInfo");
    qRegisterMetaTypeStreamOperators<QScriptBreakpointInfoList>("QScriptBreakpointInfoList");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerValue>("QScriptDebuggerValue");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerValueList>("QScriptDebuggerValueList");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerProperty>("QScriptDebuggerProperty");
    qRegisterMetaTypeStreamOperators<QScriptDebuggerPropertyList>("QScriptDebuggerPropertyList");
    qRegisterMetaTypeStreamOperators<QList<int> >("QList<int>");
    qRegisterMetaTypeStreamOperators<QList<qint64> >("QList<qint64>");
}
