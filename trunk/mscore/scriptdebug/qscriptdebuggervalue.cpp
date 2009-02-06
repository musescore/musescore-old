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

#include "qscriptdebuggervalue.h"

#include <QtCore/qatomic.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qnumeric.h>
#include <QtCore/qstring.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptengine.h>

/*!
  \class QScriptDebuggerValue

  \brief The QScriptDebuggerValue class acts as a container for Qt Script debugger types.

  QScriptDebuggerValue is used to represent a value passed to a
  debugger as part of a debugger command (QScriptDebuggerCommand) or
  received as part of a debugger event (QScriptDebuggerEvent) or
  command response (QScriptDebuggerResponse).

  A QScriptDebuggerValue is similar to a QScriptValue in the types of
  values it can hold; the main difference is that QScriptDebuggerValue
  uses identifiers to represent script objects. This is because the
  script engine containing the object can be located in a different
  process or on a remote machine. The QScriptDebuggerValue class
  doesn't have the methods for operating on script objects that
  QScriptValue has (e.g.  QScriptValue::setProperty()); to operate on
  the object that a QScriptDebuggerValue refers to, you use one of the
  command functions in the QScriptDebuggerFrontend class
  (e.g. QScriptDebuggerFrontend::scheduleSetProperty()).

  \sa QScriptDebuggerFrontend
*/

/*!
  \enum QScriptDebuggerValue::ValueType

  This enum specifies the possible value types.

  \value NoValue Not a value.
  \value UndefinedValue An undefined value.
  \value NullValue A null value.
  \value BooleanValue A boolean value.
  \value StringValue A string value.
  \value NumberValue A number value.
  \value ObjectValue An object.
*/

class QScriptDebuggerValuePrivate
{
public:
    QScriptDebuggerValuePrivate();
    ~QScriptDebuggerValuePrivate();

    QScriptDebuggerValue::ValueType type;
    union {
        bool booleanValue;
        QString *stringValue;
        double numberValue;
        qint64 objectId;
    };

    QBasicAtomicInt ref;
};

QScriptDebuggerValuePrivate::QScriptDebuggerValuePrivate()
{
    ref = 0;
}

QScriptDebuggerValuePrivate::~QScriptDebuggerValuePrivate()
{
    if (type == QScriptDebuggerValue::StringValue)
        delete stringValue;
}

/*!
  Constructs a QScriptDebuggerValue with no value.
*/
QScriptDebuggerValue::QScriptDebuggerValue()
    : d_ptr(0)
{
}

/*!
  \internal
*/
QScriptDebuggerValue::QScriptDebuggerValue(double value)
    : d_ptr(new QScriptDebuggerValuePrivate)
{
    d_ptr->type = NumberValue;
    d_ptr->numberValue = value;
    d_ptr->ref.ref();
}

/*!
  \internal
*/
QScriptDebuggerValue::QScriptDebuggerValue(bool value)
    : d_ptr(new QScriptDebuggerValuePrivate)
{
    d_ptr->type = BooleanValue;
    d_ptr->booleanValue = value;
    d_ptr->ref.ref();
}

/*!
  \internal
*/
QScriptDebuggerValue::QScriptDebuggerValue(const QString &value)
    : d_ptr(new QScriptDebuggerValuePrivate)
{
    d_ptr->type = StringValue;
    d_ptr->stringValue = new QString(value);
    d_ptr->ref.ref();
}

/*!
  \internal
*/
QScriptDebuggerValue::QScriptDebuggerValue(qint64 value)
    : d_ptr(new QScriptDebuggerValuePrivate)
{
    d_ptr->type = ObjectValue;
    d_ptr->objectId = value;
    d_ptr->ref.ref();
}

/*!
  \internal
*/
QScriptDebuggerValue::QScriptDebuggerValue(ValueType type)
    : d_ptr(new QScriptDebuggerValuePrivate)
{
    d_ptr->type = type;
    d_ptr->ref.ref();
}

/*!
  Constructs a QScriptDebuggerValue that is a copy of the \a other value.
*/
QScriptDebuggerValue::QScriptDebuggerValue(const QScriptDebuggerValue &other)
    : d_ptr(other.d_ptr)
{
    if (d_ptr)
        d_ptr->ref.ref();
}

/*!
  Destroys this QScriptDebuggerValue.
*/
QScriptDebuggerValue::~QScriptDebuggerValue()
{
    if (d_ptr && !d_ptr->ref.deref()) {
        delete d_ptr;
        d_ptr = 0;
    }
}

/*!
  Assigns the \a other value to this QScriptDebuggerValue.
*/
QScriptDebuggerValue &QScriptDebuggerValue::operator=(const QScriptDebuggerValue &other)
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
  Returns the type of this value.
*/
QScriptDebuggerValue::ValueType QScriptDebuggerValue::type() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return NoValue;
    return d->type;
}

/*!
  Returns this value as a number.
*/
double QScriptDebuggerValue::numberValue() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return 0;
    return d->numberValue;
}

/*!
  Returns this value as a boolean.
*/
bool QScriptDebuggerValue::booleanValue() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return false;
    return d->booleanValue;
}

/*!
  Returns this value as a string.
*/
QString QScriptDebuggerValue::stringValue() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return QString();
    return *d->stringValue;
}

/*!
  Returns this value as an object ID.
*/
qint64 QScriptDebuggerValue::objectId() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return -1;
    return d->objectId;
}

/*!
  Constructs a QScriptDebuggerValue from the given \a value.
*/
QScriptDebuggerValue QScriptDebuggerValue::fromScriptValue(const QScriptValue &value)
{
    if (!value.isValid())
        return QScriptDebuggerValue();
    else if (value.isUndefined())
        return QScriptDebuggerValue::undefined();
    else if (value.isNull())
        return QScriptDebuggerValue::null();
    else if (value.isNumber())
        return QScriptDebuggerValue::fromNumber(value.toNumber());
    else if (value.isBoolean())
        return QScriptDebuggerValue::fromBoolean(value.toBoolean());
    else if (value.isString())
        return QScriptDebuggerValue::fromString(value.toString());
    else {
        Q_ASSERT(value.isObject());
        return QScriptDebuggerValue::fromObjectId(value.objectId());
    }
}

/*!
  Constructs a QScriptDebuggerValue from the given \a value.
*/
QScriptDebuggerValue QScriptDebuggerValue::fromNumber(double value)
{
    return QScriptDebuggerValue(value);
}

/*!
  Constructs a QScriptDebuggerValue from the given \a value.
*/
QScriptDebuggerValue QScriptDebuggerValue::fromBoolean(bool value)
{
    return QScriptDebuggerValue(value);
}

/*!
  Constructs a QScriptDebuggerValue from the given \a value.
*/
QScriptDebuggerValue QScriptDebuggerValue::fromString(const QString &value)
{
    return QScriptDebuggerValue(value);
}

/*!
  Constructs a QScriptDebuggerValue from the given object \a id.
*/
QScriptDebuggerValue QScriptDebuggerValue::fromObjectId(qint64 id)
{
    return QScriptDebuggerValue(id);
}

/*!
  Constructs an undefined value.
*/
QScriptDebuggerValue QScriptDebuggerValue::undefined()
{
    return QScriptDebuggerValue(UndefinedValue);
}

/*!
  Constructs a null value.
*/
QScriptDebuggerValue QScriptDebuggerValue::null()
{
    return QScriptDebuggerValue(NullValue);
}

/*!
  Converts this QScriptDebuggerValue to a QScriptValue in the
  given \a engine and returns the resulting value.
*/
QScriptValue QScriptDebuggerValue::toScriptValue(QScriptEngine *engine) const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return QScriptValue();
    switch (d->type) {
    case NoValue:
        return QScriptValue();
    case UndefinedValue:
        return engine->undefinedValue();
    case NullValue:
        return engine->nullValue();
    case BooleanValue:
        return QScriptValue(engine, d->booleanValue);
    case StringValue:
        return QScriptValue(engine, *d->stringValue);
    case NumberValue:
        return QScriptValue(engine, d->numberValue);
    case ObjectValue:
        return engine->objectById(d->objectId);
    }
    return QScriptValue();
}

/*!
  Returns true if this QScriptDebuggerValue is equal to the \a other
  value, otherwise returns false.
*/
bool QScriptDebuggerValue::operator==(const QScriptDebuggerValue &other) const
{
    Q_D(const QScriptDebuggerValue);
    const QScriptDebuggerValuePrivate *od = other.d_func();
    if (d == od)
        return true;
    if (!d || !od)
        return false;
    if (d->type != od->type)
        return false;
    switch (d->type) {
    case NoValue:
    case UndefinedValue:
    case NullValue:
        return true;
    case BooleanValue:
        return d->booleanValue == od->booleanValue;
    case StringValue:
        return *d->stringValue == *od->stringValue;
    case NumberValue:
        return d->numberValue == od->numberValue;
    case ObjectValue:
        return d->objectId == od->objectId;
    }
    return false;
}

/*!
  Returns true if this QScriptDebuggerValue is not equal to the \a
  other value, otherwise returns false.
*/
bool QScriptDebuggerValue::operator!=(const QScriptDebuggerValue &other) const
{
    return !(*this == other);
}

/*!
  Returns a string representation of this value.
*/
QString QScriptDebuggerValue::toString() const
{
    Q_D(const QScriptDebuggerValue);
    if (!d)
        return QString();
    switch (d->type) {
    case NoValue:
        return QString();
    case UndefinedValue:
        return QString::fromLatin1("undefined");
    case NullValue:
        return QString::fromLatin1("null");
    case BooleanValue:
        if (d->booleanValue)
            return QString::fromLatin1("true");
        else
            return QString::fromLatin1("false");
    case StringValue:
        return *d->stringValue;
    case NumberValue:
        return QString::number(d->numberValue); // ### qScriptNumberToString()
    case ObjectValue:
        return QString::fromLatin1("[object Object]");
    }
    return QString();
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerValue &value)
  \relates QScriptDebuggerValue

  Writes the given \a value to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerValue &value)
{
    out << (quint32)value.type();
    switch (value.type()) {
    case QScriptDebuggerValue::NoValue:
    case QScriptDebuggerValue::UndefinedValue:
    case QScriptDebuggerValue::NullValue:
        break;
    case QScriptDebuggerValue::BooleanValue:
        out << value.booleanValue();
        break;
    case QScriptDebuggerValue::StringValue:
        out << value.stringValue();
        break;
    case QScriptDebuggerValue::NumberValue:
        out << value.numberValue();
        break;
    case QScriptDebuggerValue::ObjectValue:
        out << value.objectId();
        break;
    }
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerValue &value)
  \relates QScriptDebuggerValue

  Reads a QScriptDebuggerValue from the specified \a stream into the
  given \a value.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerValue &value)
{
    quint32 type;
    in >> type;
    switch (QScriptDebuggerValue::ValueType(type)) {
    case QScriptDebuggerValue::UndefinedValue:
        value = QScriptDebuggerValue::undefined();
        break;
    case QScriptDebuggerValue::NullValue:
        value = QScriptDebuggerValue::null();
        break;
    case QScriptDebuggerValue::BooleanValue: {
        bool b;
        in >> b;
        value = QScriptDebuggerValue::fromBoolean(b);
    }   break;
    case QScriptDebuggerValue::StringValue: {
        QString s;
        in >> s;
        value = QScriptDebuggerValue::fromString(s);
    }   break;
    case QScriptDebuggerValue::NumberValue: {
        double d;
        in >> d;
        value = QScriptDebuggerValue::fromNumber(d);
    }   break;
    case QScriptDebuggerValue::ObjectValue: {
        qint64 id;
        in >> id;
        value = QScriptDebuggerValue::fromObjectId(id);
    }   break;
    case QScriptDebuggerValue::NoValue:
    default:
        value = QScriptDebuggerValue();
        break;
    }
    return in;
}
