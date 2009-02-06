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

#ifndef QSCRIPTDEBUGGEREVENT_H
#define QSCRIPTDEBUGGEREVENT_H

#include "qscriptdebugglobal.h"

#include <QtCore/qvariant.h>

class QDataStream;

class QScriptDebuggerEventPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerEvent
{
public:
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerEvent &);
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerEvent &);

    enum Type {
        None,
        Break,
        SteppingFinished,
        LocationReached,
        Breakpoint,
        Exception,
        Trace,
        // ### EngineDestroyed
        // ### notification that the engine is about to return control to C++?
        UserEvent = 1000,
        MaxUserEvent = 32767
    };

    enum Attribute {
        FileName,
        LineNumber,
        ScriptID,
        BreakpointID,
        ReturnValue,
        ExceptionValue,
        ExceptionString,
        HasExceptionHandler,
        Text,
        UserAttribute = 1000,
        MaxUserAttribute = 32767
    };

    QScriptDebuggerEvent();
    QScriptDebuggerEvent(Type type);
    QScriptDebuggerEvent(const QScriptDebuggerEvent &other);
    ~QScriptDebuggerEvent();

    Type type() const;

    int lineNumber() const;
    qint64 scriptId() const;
    QString fileName() const;

    QScriptDebuggerEvent &operator=(const QScriptDebuggerEvent &other);

    QVariant attribute(Attribute attribute,
                       const QVariant &defaultValue = QVariant()) const;
    void setAttribute(Attribute attribute, const QVariant &value);

private:
    QScriptDebuggerEventPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptDebuggerEvent)
};

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerEvent &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerEvent &);

#endif
