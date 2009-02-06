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

#ifndef QSCRIPTDEBUGGERRESPONSE_H
#define QSCRIPTDEBUGGERRESPONSE_H

#include "qscriptdebugglobal.h"

#include <QtCore/qvariant.h>

class QDataStream;

class QScriptDebuggerResponsePrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerResponse
{
public:
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerResponse &);
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerResponse &);

    enum Error {
        NoError,
        InvalidContextIndex,
        InvalidArgumentIndex,
        InvalidScriptID,
        InvalidBreakpointID,
        UserError = 1000,
        MaxUserError = 32767
    };

    enum Attribute {
        Result,
        UserAttribute = 1000,
        MaxUserAttribute = 32767
    };

    QScriptDebuggerResponse();
    QScriptDebuggerResponse(const QScriptDebuggerResponse &other);
    ~QScriptDebuggerResponse();

    Error error() const;
    void setError(Error error);

    QVariant attribute(Attribute attribute, const QVariant &defaultValue = QVariant()) const;
    void setAttribute(Attribute attribute, const QVariant &value);

    QVariant result() const;
    void setResult(const QVariant &value);

    QScriptDebuggerResponse &operator=(const QScriptDebuggerResponse &other);

private:
    QScriptDebuggerResponsePrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptDebuggerResponse)
};

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerResponse &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerResponse &);

#endif
