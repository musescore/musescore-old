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

#ifndef QSCRIPTDEBUGGERBACKEND_P_H
#define QSCRIPTDEBUGGERBACKEND_P_H

//#include <private/qscriptengineagent_p.h>

#include <QtCore/qlist.h>
#include <QtCore/qmap.h>
#include <QtCore/qvariant.h>
#include <QtScript/qscriptvalue.h>
#include "qscriptbreakpointinfo.h"

class QScriptContext;
class QScriptContextInfo;
class QScriptInfo;
class QScriptDebuggerCommand;
class QScriptDebuggerResponse;
class QScriptValueIterator;
class QVariant;

class QScriptInfoPrivate;
class QScriptInfo
{
public:
    QScriptInfo();
    QScriptInfo(const QString &contents, const QString &fileName,
                int baseLineNumber);
    QScriptInfo(const QScriptInfo &other);
    ~QScriptInfo();

    QString contents() const;
    QString fileName() const;
    int baseLineNumber() const;

    bool isValid() const;

    QScriptInfo &operator=(const QScriptInfo &other);

private:
    QScriptInfoPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptInfo)
};

class QScriptDebuggerBackend;
class QScriptDebuggerBackendPrivate
// : public QScriptEngineAgentPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebuggerBackend)
public:
    QScriptDebuggerBackendPrivate();
    ~QScriptDebuggerBackendPrivate();

    int contextIndex(const QScriptDebuggerCommand &command) const;
    int argumentIndex(const QScriptDebuggerCommand &command) const;
    static QVariant debuggerValue(const QScriptValue &value);
    static QVariant debuggerValueList(const QScriptValueList &lst);
    QScriptValue scriptValue(const QVariant &value) const;
    QScriptValueList scriptValueList(const QVariant &lst) const;
    bool isValidBreakpointId(int id) const;

    static QScriptValue trace(QScriptContext *context, QScriptEngine *engine);
    static QScriptValue pause(QScriptContext *context, QScriptEngine *engine);

    int nextBreakpointId;
    QList<int> breakpointIds;
    QScriptBreakpointInfoList breakpointInfos;

    QMap<qint64, QScriptInfo> loadedScripts;
    QMap<qint64, QScriptInfo> checkpointScripts;
    QMap<qint64, QScriptInfo> previousCheckpointScripts;
    QList<QList<qint64> > scriptIdStack;

    int state;
    int stepDepth;
    QScriptValue stepOutValue;
    QScriptValue exceptionValue;
    qint64 targetLocationScriptId;
    QString targetLocationFileName;
    int targetLocationLineNumber;

    int nextIteratorId;
    QMap<int, QScriptValueIterator*> iterators;

    QScriptDebuggerBackend *q_ptr; // ###
};

#endif
