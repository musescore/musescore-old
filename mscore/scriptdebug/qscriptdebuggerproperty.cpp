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

#include "qscriptdebuggerproperty.h"

#include "qscriptdebuggervalue.h"
#include <QtCore/qatomic.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qnumeric.h>
#include <QtCore/qstring.h>

/*!
  \class QScriptDebuggerProperty

  \brief The QScriptDebuggerProperty class is used to describe a property of an object.
*/

class QScriptDebuggerPropertyPrivate
{
public:
    QScriptDebuggerPropertyPrivate();
    ~QScriptDebuggerPropertyPrivate();

    QString name;
    QScriptDebuggerValue value;
    QScriptValue::PropertyFlags flags;

    QBasicAtomicInt ref;
};

QScriptDebuggerPropertyPrivate::QScriptDebuggerPropertyPrivate()
{
    ref = 0;
}

QScriptDebuggerPropertyPrivate::~QScriptDebuggerPropertyPrivate()
{
}

/*!
  Constructs an invalid QScriptDebuggerProperty.
*/
QScriptDebuggerProperty::QScriptDebuggerProperty()
    : d_ptr(0)
{
}

/*!
  Constructs a QScriptDebuggerProperty with the given \a name,
  \a value and \a flags.
*/
QScriptDebuggerProperty::QScriptDebuggerProperty(const QString &name,
                                                 const QScriptDebuggerValue &value,
                                                 QScriptValue::PropertyFlags flags)
    : d_ptr(new QScriptDebuggerPropertyPrivate)
{
    d_ptr->name = name;
    d_ptr->value = value;
    d_ptr->flags = flags;
    d_ptr->ref.ref();
}

/*!
  Constructs a QScriptDebuggerProperty that is a copy of the \a other property.
*/
QScriptDebuggerProperty::QScriptDebuggerProperty(const QScriptDebuggerProperty &other)
    : d_ptr(other.d_ptr)
{
    if (d_ptr)
        d_ptr->ref.ref();
}

/*!
  Destroys this QScriptDebuggerProperty.
*/
QScriptDebuggerProperty::~QScriptDebuggerProperty()
{
    if (d_ptr && !d_ptr->ref.deref()) {
        delete d_ptr;
        d_ptr = 0;
    }
}

/*!
  Assigns the \a other property to this QScriptDebuggerProperty.
*/
QScriptDebuggerProperty &QScriptDebuggerProperty::operator=(const QScriptDebuggerProperty &other)
{
    if (d_ptr == other.d_ptr)
        return *this;
    if (d_ptr && !d_ptr->ref.deref())
        delete d_ptr;
    d_ptr = other.d_ptr;
    if (d_ptr)
        d_ptr->ref.ref();
    return *this;
}

/*!
  Returns the name of this QScriptDebuggerProperty.
*/
QString QScriptDebuggerProperty::name() const
{
    Q_D(const QScriptDebuggerProperty);
    if (!d)
        return QString();
    return d->name;
}

/*!
  Returns the value of this QScriptDebuggerProperty.
*/
QScriptDebuggerValue QScriptDebuggerProperty::value() const
{
    Q_D(const QScriptDebuggerProperty);
    if (!d)
        return QScriptDebuggerValue();
    return d->value;
}

/*!
  Returns the flags of this QScriptDebuggerProperty.
*/
QScriptValue::PropertyFlags QScriptDebuggerProperty::flags() const
{
    Q_D(const QScriptDebuggerProperty);
    if (!d)
        return 0;
    return d->flags;
}

/*!
  Returns true if this QScriptDebuggerProperty is valid, otherwise
  returns false.
*/
bool QScriptDebuggerProperty::isValid() const
{
    Q_D(const QScriptDebuggerProperty);
    return (d != 0);
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerProperty &property)
  \relates QScriptDebuggerProperty

  Writes the given \a property to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerProperty &property)
{
    out << property.name();
    out << property.value();
    out << (quint32)property.flags();
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerProperty &property)
  \relates QScriptDebuggerProperty

  Reads a QScriptDebuggerProperty from the specified \a stream into the
  given \a property.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerProperty &property)
{
    QString name;
    QScriptDebuggerValue value;
    quint32 flags;
    in >> name;
    in >> value;
    in >> flags;
    property = QScriptDebuggerProperty(name, value, QScriptValue::PropertyFlags(flags));
    return in;
}
