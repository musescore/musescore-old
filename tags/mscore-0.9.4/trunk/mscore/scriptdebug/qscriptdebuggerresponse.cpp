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

#include "qscriptdebuggerresponse.h"

#include <QtCore/qdatastream.h>

/*!
  \class QScriptDebuggerResponse

  \brief The QScriptDebuggerResponse class represents a front-end's response to a QScriptDebuggerCommand.

  A response contains an error code and zero or more attributes
  (e.g. the result of the command).

  \sa QScriptDebuggerClient::commandFinished()
*/

/*!
  \enum QScriptDebuggerResponse::Error

  This enum specifies the possible error codes for a response.

  \value NoError
  \value InvalidContextIndex
  \value InvalidArgumentIndex
  \value InvalidScriptID
  \value InvalidBreakpointID
  \value UserError
  \value MaxUserError
*/

/*!
  \enum QScriptDebuggerResponse::Attribute

  This enum specifies the possible attributes a response can have.

  \value Result
  \value UserAttribute
  \value MaxUserAttribute
*/

class QScriptDebuggerResponsePrivate
{
public:
    QScriptDebuggerResponsePrivate();
    ~QScriptDebuggerResponsePrivate();

    QScriptDebuggerResponse::Error error;
    QMap<QScriptDebuggerResponse::Attribute, QVariant> attributes;
};

QScriptDebuggerResponsePrivate::QScriptDebuggerResponsePrivate()
{
}

QScriptDebuggerResponsePrivate::~QScriptDebuggerResponsePrivate()
{
}

/*!
  Constructs a QScriptDebuggerResponse object.
*/
QScriptDebuggerResponse::QScriptDebuggerResponse()
    : d_ptr(new QScriptDebuggerResponsePrivate)
{
}

/*!
  Constructs a QScriptDebuggerResponse that is a copy of the \a other response.
*/
QScriptDebuggerResponse::QScriptDebuggerResponse(const QScriptDebuggerResponse &other)
    : d_ptr(new QScriptDebuggerResponsePrivate)
{
    d_ptr->error = other.d_ptr->error;
    d_ptr->attributes = other.d_ptr->attributes;
}

/*!
  Destroys this QScriptDebuggerResponse.
*/
QScriptDebuggerResponse::~QScriptDebuggerResponse()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Assigns the \a other response to this QScriptDebuggerResponse.
*/
QScriptDebuggerResponse &QScriptDebuggerResponse::operator=(const QScriptDebuggerResponse &other)
{
    d_ptr->error = other.d_ptr->error;
    d_ptr->attributes = other.d_ptr->attributes;
    return *this;
}

/*!
  Returns the error code of this response.
*/
QScriptDebuggerResponse::Error QScriptDebuggerResponse::error() const
{
    Q_D(const QScriptDebuggerResponse);
    return d->error;
}

/*!
  Sets the \a error code of this response.
*/
void QScriptDebuggerResponse::setError(Error error)
{
    Q_D(QScriptDebuggerResponse);
    d->error = error;
}

/*!
  Returns the value of the given \a attribute, or \a defaultValue
  if the attribute is not defined.
*/
QVariant QScriptDebuggerResponse::attribute(Attribute attribute, const QVariant &defaultValue) const
{
    Q_D(const QScriptDebuggerResponse);
    return d->attributes.value(attribute, defaultValue);
}

/*!
  Sets the \a value of the given \a attribute.
*/
void QScriptDebuggerResponse::setAttribute(Attribute attribute, const QVariant &value)
{
    Q_D(QScriptDebuggerResponse);
    d->attributes[attribute] = value;
}

/*!
  Returns the Result attribute of this response. This function is
  provided for convenience.

  \sa attribute()
*/
QVariant QScriptDebuggerResponse::result() const
{
    return attribute(Result);
}

/*!
  Sets the Result attribute of this response to the given \a
  value. This function is provided for convenience.

  \sa setAttribute()
*/
void QScriptDebuggerResponse::setResult(const QVariant &value)
{
    setAttribute(Result, value);
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerResponse &response)
  \relates QScriptDebuggerResponse

  Writes the given \a response to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerResponse &response)
{
    const QScriptDebuggerResponsePrivate *d = response.d_ptr;
    out << (quint32)d->error;
    out << (qint32)d->attributes.size();
    QMap<QScriptDebuggerResponse::Attribute, QVariant>::const_iterator it;
    for (it = d->attributes.constBegin(); it != d->attributes.constEnd(); ++it) {
        out << (quint32)it.key();
        out << it.value();
    }
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerResponse &response)
  \relates QScriptDebuggerResponse

  Reads a QScriptDebuggerResponse from the specified \a stream into the
  given \a response.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerResponse &response)
{
    QScriptDebuggerResponsePrivate *d = response.d_ptr;

    quint32 error;
    in >> error;
    d->error = QScriptDebuggerResponse::Error(error);

    qint32 attribCount;
    in >> attribCount;
    QMap<QScriptDebuggerResponse::Attribute, QVariant> attribs;
    for (qint32 i = 0; i < attribCount; ++i) {
        quint32 key;
        in >> key;
        QVariant value;
        in >> value;
        attribs[QScriptDebuggerResponse::Attribute(key)] = value;
    }
    d->attributes = attribs;

    return in;
}
