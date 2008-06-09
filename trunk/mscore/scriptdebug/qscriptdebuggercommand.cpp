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

#include <QtCore/qdatastream.h>
#include <QtCore/qmap.h>

class QScriptDebuggerCommandPrivate
{
public:
    QScriptDebuggerCommandPrivate();
    ~QScriptDebuggerCommandPrivate();

    QScriptDebuggerCommand::Type type;
    QMap<QScriptDebuggerCommand::Attribute, QVariant> attributes;
};

QScriptDebuggerCommandPrivate::QScriptDebuggerCommandPrivate()
{
}

QScriptDebuggerCommandPrivate::~QScriptDebuggerCommandPrivate()
{
}

/*!
  \class QScriptDebuggerCommand

  \brief The QScriptDebuggerCommand class represents a command issued to a QScriptDebuggerFrontend.

  A debugger command is described by a command type and zero or more attributes.

  This class is only relevant if you have subclassed QScriptDebuggerFrontend
  and reimplemented QScriptDebuggerFrontend::processCommand(), or if you have
  subclassed QScriptDebuggerBackend and reimplemented
  QScriptDebuggerBackend::applyCommand(); otherwise, use the
  scheduleXXX functions in QScriptDebuggerFrontend (those functions generate
  proper QScriptDebuggerCommand objects behind the scenes).

  \sa QScriptDebuggerFrontend, QScriptDebuggerBackend
*/

/*!
  \enum QScriptDebuggerCommand::Type

  This enum specifies the possible command types.
  \value None No operation.

  \value GetContextCount Get the number of contexts (frames).
  \value GetContextInfo Get the QScriptContextInfo of a context.
  \value GetContextInfos Get a list of QScriptContextInfo objects.
  \value GetContextString Get a string representation of a context.
  \value GetScriptID Get the ID of the script associated with a context.
  \value GetBacktrace Get the backtrace of a context.
  \value GetThisObject Get the this-object of a context.
  \value SetThisObject Set the this-object of a context.
  \value GetActivationObject Get the activation object of a context.
  \value SetActivationObject Set the activation object of a context.
  \value GetArgumentCount Get the argument count of a context.
  \value GetArgument Get an argument of a context.
  \value GetArguments Get a list of the arguments of a context.
  \value GetArgumentsObject Get the arguments object of a context.
  \value GetScopeChain Get the scope chain of a context.
  \value GetCallee Get the callee of a context.
  \value IsCalledAsConstructor Get whether a context is called as a constructor.

  \value AbortEvaluation Abort evaluation.
  \value CanEvaluate Check if a script can be evaluated.
  \value ClearExceptions Clear exceptions.
  \value CollectGarbage Collect garbage.
  \value Evaluate Evaluate a script.
  \value GetGlobalObject Get the Global Object.
  \value HasUncaughtException Check whether the engine has an uncaught exception.
  \value GetUncaughtException Get the uncaught exception.
  \value GetUncaughtExceptionLineNumber Get the line number associated with the uncaught exception.
  \value GetUncaughtExceptionBacktrace Get a backtrace of the uncaught exception.
  \value NewArray Create a new Array object.
  \value NewObject Create a new Object object.
  \value PushContext Push a context.
  \value PopContext Pop the current context.

  \value Call Call a value as a function.
  \value Construct Call a value as a constructor.
  \value GetProperty Get a property of an object.
  \value SetProperty Set a property of an object.
  \value GetPropertyFlags Get the flags of a property.
  \value GetPropertyNames Get the names of properties of an object.
  \value GetProperties Get one or more properties of an object.
  \value GetPrototype Get the internal prototype of an object.
  \value SetPrototype Set the internal prototype of an object.
  \value ToBoolean Convert a value to a bool.
  \value ToDateTime Convert a value to a QDateTime.
  \value ToNumber Convert a value to a number.
  \value ToObject Convert a value to an object.
  \value ToRegExp Convert a value to a QRegExp.
  \value ToString Convert a value to a string.

  \value NewIterator Create a new iterator for an object.
  \value IteratorFlags
  \value IteratorHasNext
  \value IteratorHasPrevious
  \value IteratorName
  \value IteratorNext
  \value IteratorPrevious
  \value IteratorRemove
  \value IteratorSetValue
  \value IteratorToBack
  \value IteratorToFront
  \value IteratorValue
  \value IteratorAssign
  \value IteratorProperty
  \value IteratorNextProperty
  \value IteratorNextProperties
  \value DeleteIterator

  \value Break Request a break.
  \value Continue Continue evaluation.
  \value StepInto Step into the next script statement.
  \value StepOver Step over the next script statement.
  \value StepOut Step out of the current script function.
  \value RunToLocation Run until a specific location is reached.
  \value Resume Resume evaluation in the current state.

  \value SetBreakpoint Set a breakpoint.
  \value SetBreakpointByScriptID
  \value DeleteBreakpoint Delete a breakpoint.
  \value DeleteAllBreakpoints Delete all breakpoints.
  \value ModifyBreakpoint Modify an existing breakpoint.
  \value GetBreakpointIdentifiers Get a list of breakpoint identifiers.
  \value GetBreakpointInfo Get information about a breakpoint.
  \value SetBreakpointEnabled

  \value GetScriptContents Get the contents of a script.
  \value GetScriptLines Get lines of a script.
  \value ListScripts List the loaded scripts.
  \value ScriptsCheckpoint Do a checkpoint of the loaded scripts.
  \value GetScriptsDelta Get the difference between the current and previous checkpoint.
  \value ScriptsCheckpointAndDelta Do a checkpoint and get the difference.

  \value UserCommand
  \value MaxUserCommand
*/

/*!
  \enum QScriptDebuggerCommand::Attribute

  This enum specifies the possible attributes a command can
  have. Which attributes are actually used depends on the type of
  command.

  \value FileName File name.
  \value LineNumber Line number.
  \value ColumnNumber Column number.
  \value ScriptID Script ID.
  \value BreakpointID Breakpoint ID.
  \value BreakpointInfo Breakpoint info (QScriptBreakpointInfo).
  \value ContextIndex Context index.
  \value ArgumentIndex Argument index.
  \value Length
  \value Count
  \value Program Program text.
  \value Value
  \value Enabled
  \value FunctionObject
  \value ThisObject
  \value FunctionArguments
  \value Object
  \value PropertyName
  \value PropertyNames
  \value IteratorID
  \value IncludeScriptContents
  \value UserAttribute
  \value MaxUserAttribute
*/

/*!
  Constructs a QScriptDebuggerCommand of the given \a type, with no
  attributes defined.
*/
QScriptDebuggerCommand::QScriptDebuggerCommand(Type type)
    :  d_ptr(new QScriptDebuggerCommandPrivate)
{
    d_ptr->type = type;
}

/*!
  Constructs a QScriptDebuggerCommand of type None.
*/
QScriptDebuggerCommand::QScriptDebuggerCommand()
    :  d_ptr(new QScriptDebuggerCommandPrivate)
{
    d_ptr->type = None;
}

/*!
  Constructs a QScriptDebuggerCommand that is a copy of the \a other
  command.
*/
QScriptDebuggerCommand::QScriptDebuggerCommand(const QScriptDebuggerCommand &other)
    :  d_ptr(new QScriptDebuggerCommandPrivate)
{
    d_ptr->type = other.d_ptr->type;
    d_ptr->attributes = other.d_ptr->attributes;
}

/*!
  Assigns the \a other value to this QScriptDebuggerCommand.
*/
QScriptDebuggerCommand &QScriptDebuggerCommand::operator=(const QScriptDebuggerCommand &other)
{
    d_ptr->type = other.d_ptr->type;
    d_ptr->attributes = other.d_ptr->attributes;
    return *this;
}

/*!
  Destroys this QScriptDebuggerCommand.
*/
QScriptDebuggerCommand::~QScriptDebuggerCommand()
{
    delete d_ptr;
    d_ptr = 0;
}

/*!
  Returns the type of this command.
*/
QScriptDebuggerCommand::Type QScriptDebuggerCommand::type() const
{
    Q_D(const QScriptDebuggerCommand);
    return d->type;
}

/*!
  Returns the value of the given \a attribute, or \a defaultValue
  if the attribute is not defined.
*/
QVariant QScriptDebuggerCommand::attribute(Attribute attribute,
                                           const QVariant &defaultValue) const
{
    Q_D(const QScriptDebuggerCommand);
    return d->attributes.value(attribute, defaultValue);
}

/*!
  Sets the \a value of the given \a attribute.
*/
void QScriptDebuggerCommand::setAttribute(Attribute attribute,
                                          const QVariant &value)
{
    Q_D(QScriptDebuggerCommand);
    d->attributes[attribute] = value;
}

/*!
  \fn QDataStream &operator<<(QDataStream &stream, const QScriptDebuggerCommand &command)
  \relates QScriptDebuggerCommand

  Writes the given \a command to the specified \a stream.
*/
QDataStream &operator<<(QDataStream &out, const QScriptDebuggerCommand &command)
{
    const QScriptDebuggerCommandPrivate *d = command.d_ptr;
    out << (quint32)d->type;
    out << (qint32)d->attributes.size();
    QMap<QScriptDebuggerCommand::Attribute, QVariant>::const_iterator it;
    for (it = d->attributes.constBegin(); it != d->attributes.constEnd(); ++it) {
        out << (quint32)it.key();
        out << it.value();
    }
    return out;
}

/*!
  \fn QDataStream &operator>>(QDataStream &stream, QScriptDebuggerCommand &command)
  \relates QScriptDebuggerCommand

  Reads a QScriptDebuggerCommand from the specified \a stream into the
  given \a command.
*/
QDataStream &operator>>(QDataStream &in, QScriptDebuggerCommand &command)
{
    QScriptDebuggerCommandPrivate *d = command.d_ptr;

    quint32 type;
    in >> type;
    d->type = QScriptDebuggerCommand::Type(type);

    qint32 attribCount;
    in >> attribCount;
    QMap<QScriptDebuggerCommand::Attribute, QVariant> attribs;
    for (qint32 i = 0; i < attribCount; ++i) {
        quint32 key;
        in >> key;
        QVariant value;
        in >> value;
        attribs[QScriptDebuggerCommand::Attribute(key)] = value;
    }
    d->attributes = attribs;

    return in;
}
