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

#ifndef QSCRIPTDEBUGGERPROPERTY_H
#define QSCRIPTDEBUGGERPROPERTY_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobjectdefs.h>

#include <QtCore/qlist.h>
#include <QtScript/qscriptvalue.h>

class QDataStream;
class QString;
class QScriptDebuggerValue;

class QScriptDebuggerPropertyPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerProperty
{
public:
    QScriptDebuggerProperty();
    QScriptDebuggerProperty(const QString &name,
                            const QScriptDebuggerValue &value,
                            QScriptValue::PropertyFlags flags);
    QScriptDebuggerProperty(const QScriptDebuggerProperty &other);
    ~QScriptDebuggerProperty();

    QScriptDebuggerProperty &operator=(const QScriptDebuggerProperty &other);

    QString name() const;
    QScriptDebuggerValue value() const;
    QScriptValue::PropertyFlags flags() const;

    bool isValid() const;

private:
    QScriptDebuggerPropertyPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptDebuggerProperty)
};

typedef QList<QScriptDebuggerProperty> QScriptDebuggerPropertyList;

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerProperty &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerProperty &);

#endif
