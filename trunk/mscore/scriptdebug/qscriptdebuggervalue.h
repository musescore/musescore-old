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

#ifndef QSCRIPTDEBUGGERVALUE_H
#define QSCRIPTDEBUGGERVALUE_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobjectdefs.h>

#include <QtCore/qlist.h>

class QScriptEngine;
class QScriptValue;
class QString;
class QDataStream;

class QScriptDebuggerValuePrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerValue
{
public:
    enum ValueType {
        NoValue,
        UndefinedValue,
        NullValue,
        BooleanValue,
        StringValue,
        NumberValue,
        ObjectValue
    };

    QScriptDebuggerValue();
    QScriptDebuggerValue(const QScriptDebuggerValue &other);
    ~QScriptDebuggerValue();

    QScriptDebuggerValue &operator=(const QScriptDebuggerValue &other);

    ValueType type() const;
    double numberValue() const;
    bool booleanValue() const;
    QString stringValue() const;
    qint64 objectId() const;

    bool operator==(const QScriptDebuggerValue &other) const;
    bool operator!=(const QScriptDebuggerValue &other) const;

    QScriptValue toScriptValue(QScriptEngine *engine) const;

    QString toString() const;

    static QScriptDebuggerValue fromScriptValue(const QScriptValue &value);
    static QScriptDebuggerValue fromNumber(double value);
    static QScriptDebuggerValue fromBoolean(bool value);
    static QScriptDebuggerValue fromString(const QString &value);
    static QScriptDebuggerValue fromObjectId(qint64 id);
    static QScriptDebuggerValue undefined();
    static QScriptDebuggerValue null();

private:
    QScriptDebuggerValue(double value);
    QScriptDebuggerValue(bool value);
    QScriptDebuggerValue(const QString &value);
    QScriptDebuggerValue(qint64 value);
    QScriptDebuggerValue(ValueType type);

    QScriptDebuggerValuePrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptDebuggerValue)
};

typedef QList<QScriptDebuggerValue> QScriptDebuggerValueList;

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerValue &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerValue &);

#endif
