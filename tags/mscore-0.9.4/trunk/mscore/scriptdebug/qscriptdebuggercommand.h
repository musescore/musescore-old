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

#ifndef QSCRIPTDEBUGGERCOMMAND_H
#define QSCRIPTDEBUGGERCOMMAND_H

#include "qscriptdebugglobal.h"

#include <QtCore/qvariant.h>

class QDataStream;

class QScriptDebuggerCommandPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerCommand
{
public:
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerCommand &);
    friend Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerCommand &);

    enum Type {
        None,

        GetContextCount,
        GetContextInfo,
        GetContextInfos,
        GetContextString,
        GetScriptID,
        GetBacktrace,
        GetThisObject,
        SetThisObject,
        GetActivationObject,
        SetActivationObject,
        GetArgumentCount,
        GetArgument,
        GetArguments,
        GetArgumentsObject,
        GetScopeChain,
        GetCallee,
        IsCalledAsConstructor,

        AbortEvaluation,
        CanEvaluate,
        ClearExceptions,
        CollectGarbage,
        Evaluate,
        GetGlobalObject,
        HasUncaughtException,
        GetUncaughtException,
        GetUncaughtExceptionLineNumber,
        GetUncaughtExceptionBacktrace,
        NewArray,
        NewObject,
        PushContext,
        PopContext,

        Call,
        Construct,
        GetProperty,
        SetProperty,
        GetPropertyFlags,
        GetPropertyNames,
        GetProperties,
        GetPrototype,
        SetPrototype,
        ToBoolean,
        ToDateTime,
        ToNumber,
        ToObject,
        ToRegExp,
        ToString,

        NewIterator,
        IteratorFlags,
        IteratorHasNext,
        IteratorHasPrevious,
        IteratorName,
        IteratorNext,
        IteratorPrevious,
        IteratorRemove,
        IteratorSetValue,
        IteratorToBack,
        IteratorToFront,
        IteratorValue,
        IteratorAssign,
        IteratorProperty,
        IteratorNextProperty,
        IteratorNextProperties,
        DeleteIterator,

        Break,
        Continue,
        StepInto,
        StepOver,
        StepOut,
        RunToLocation,
        RunToLocationByID,
        Resume,

        SetBreakpoint,
        SetBreakpointByScriptID,
        DeleteBreakpoint,
        DeleteAllBreakpoints,
        ModifyBreakpoint,
        GetBreakpointIdentifiers,
        GetBreakpointInfo,
        SetBreakpointEnabled,

        GetScriptContents,
        GetScriptLines,
        ListScripts,
        ScriptsCheckpoint,
        GetScriptsDelta,
        ScriptsCheckpointAndDelta,

        // ### Disconnect,
        // ### ForceReturn,

        UserCommand = 1000,
        MaxUserCommand = 32767
    };

    enum Attribute {
        FileName,
        LineNumber,
        ColumnNumber,
        ScriptID,
        BreakpointID,
        BreakpointInfo,
        ContextIndex,
        ArgumentIndex,
        Length,
        Count,
        Program,
        Value,
        Enabled,
        FunctionObject,
        ThisObject,
        FunctionArguments,
        Object,
        PropertyName,
        PropertyNames,
        IteratorID,
        IncludeScriptContents,
        UserAttribute = 1000,
        MaxUserAttribute = 32767
    };

    QScriptDebuggerCommand();
    QScriptDebuggerCommand(Type type);
    QScriptDebuggerCommand(const QScriptDebuggerCommand &other);
    ~QScriptDebuggerCommand();

    Type type() const;

    QVariant attribute(Attribute attribute, const QVariant &defaultValue = QVariant()) const;
    void setAttribute(Attribute attribute, const QVariant &value);

    QScriptDebuggerCommand &operator=(const QScriptDebuggerCommand &other);
private:
    QScriptDebuggerCommandPrivate *d_ptr;

    Q_DECLARE_PRIVATE(QScriptDebuggerCommand)
};

Q_SCRIPTDEBUG_EXPORT QDataStream &operator<<(QDataStream &, const QScriptDebuggerCommand &);
Q_SCRIPTDEBUG_EXPORT QDataStream &operator>>(QDataStream &, QScriptDebuggerCommand &);

#endif
