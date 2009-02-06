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

#include "qscriptdebuggerbackend.h"

#include "qscriptdebuggerbackend_p.h"
#include "qscriptdebuggercommand.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"
#include "qscriptbreakpointinfo.h"
#include <QtScript/qscriptengine.h>
#include <QtScript/qscriptvalue.h>
#include <QtScript/qscriptvalueiterator.h>
#include <QtScript/qscriptcontextinfo.h>
#include <QtCore/qatomic.h>
#include <QtCore/qdatetime.h>
#include <QtCore/qdebug.h>
#include <QtCore/qset.h>

Q_DECLARE_METATYPE(QScriptContextInfo)
Q_DECLARE_METATYPE(QScriptContextInfoList)
Q_DECLARE_METATYPE(QScriptDebuggerValue)
Q_DECLARE_METATYPE(QScriptDebuggerValueList)
Q_DECLARE_METATYPE(QScriptDebuggerProperty)
Q_DECLARE_METATYPE(QScriptDebuggerPropertyList)
Q_DECLARE_METATYPE(QScriptValue::PropertyFlags)
Q_DECLARE_METATYPE(QScriptBreakpointInfo)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<qint64>)

Q_DECLARE_METATYPE(QScriptDebuggerBackendPrivate*)

/*!
  \class QScriptDebuggerBackend

  \brief The QScriptDebuggerBackend class is the base class of debugger back-ends.

  QScriptDebuggerBackend builds on the low-level events reported by
  the QScriptEngineAgent class in order to provide debugging-specific
  functionality, such as stepping and breakpoints.

  This class is usually used together with the QScriptDebuggerFrontend
  class, in order to form a (front-end, back-end) pair.

  Call contextCount() to obtain the number of active contexts
  (frames). Call context() to obtain a pointer to a QScriptContext.

  Call scriptContents() to get the contents of a loaded script.

  Call stepInto() to step into the next script statement; call stepOver()
  to step over the next script statement; and call stepOut() to step out
  of the currently executing script function. An event() will be triggered
  when the stepping is completed.

  Call runToLocation() to execute script statements until a certain
  location has been reached. An event() will be triggered when the location
  has been reached.

  Call pauseEvaluation() to request that evaluation should be
  paused. An event() will be triggered upon the next script statement
  that is reached.

  Call continueEvalution() to allow script evaluation to continue.

  Call setBreakpoint() to set a breakpoint. A breakpoint event() will
  be triggered when a breakpoint is hit. Call deleteBreakpoint() to
  delete a breakpoint. Call modifyBreakpoint() to change the state of
  an existing breakpoint.

  Two functions are provided that allow you to instrument scripts for
  debugging purposes: traceFunction() returns a script function that,
  when called, will generate a Trace event. breakFunction() returns a
  script function that, when called, will generate a Break event.

  \section1 Subclassing

  When subclassing QScriptDebuggerBackend, you must implement the pure
  virtual event() function. This function typically forwards the event
  to a QScriptDebuggerFrontend object. For most type of events,
  event() should block until the back-end is instructed to resume
  execution (e.g. until continueEvalution() is called). You must
  implement resume(), which is responsible for making event() return.

  \sa QScriptDebuggerFrontend, QScriptDebuggerEvent
*/


// small helper class to store information about a script
class QScriptInfoPrivate
{
public:
    QScriptInfoPrivate();
    ~QScriptInfoPrivate();

    QString contents;
    QString fileName;
    int lineNumber;

    QBasicAtomicInt ref;
};

QScriptInfoPrivate::QScriptInfoPrivate()
{
    ref = 0;
}

QScriptInfoPrivate::~QScriptInfoPrivate()
{
}

QScriptInfo::QScriptInfo()
    : d_ptr(0)
{
}

QScriptInfo::QScriptInfo(const QString &code, const QString &fn, int ln)
    : d_ptr(new QScriptInfoPrivate)
{
    d_ptr->contents = code;
    d_ptr->fileName = fn;
    d_ptr->lineNumber = ln;
    d_ptr->ref.ref();
}

QScriptInfo::QScriptInfo(const QScriptInfo &other)
    : d_ptr(other.d_ptr)
{
    if (d_ptr)
        d_ptr->ref.ref();
}

QScriptInfo::~QScriptInfo()
{
    if (d_ptr && !d_ptr->ref.deref()) {
        delete d_ptr;
        d_ptr = 0;
    }
}

QScriptInfo &QScriptInfo::operator=(const QScriptInfo &other)
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

QString QScriptInfo::contents() const
{
    Q_D(const QScriptInfo);
    if (!d)
        return QString();
    return d->contents;
}

QString QScriptInfo::fileName() const
{
    Q_D(const QScriptInfo);
    if (!d)
        return QString();
    return d->fileName;
}

int QScriptInfo::baseLineNumber() const
{
    Q_D(const QScriptInfo);
    if (!d)
        return -1;
    return d->lineNumber;
}

bool QScriptInfo::isValid() const
{
    Q_D(const QScriptInfo);
    return (d != 0);
}



QScriptDebuggerBackendPrivate::QScriptDebuggerBackendPrivate()
{
    nextBreakpointId = 0;
    nextIteratorId = 0;
    state = QScriptDebuggerBackend::NoState;
    scriptIdStack.append(QList<qint64>());
}

QScriptDebuggerBackendPrivate::~QScriptDebuggerBackendPrivate()
{
    scriptIdStack.clear();
    qDeleteAll(iterators);
    iterators.clear();
}

bool QScriptDebuggerBackendPrivate::isValidBreakpointId(int id) const
{
    return (breakpointIds.indexOf(id) != -1);
}

int QScriptDebuggerBackendPrivate::contextIndex(const QScriptDebuggerCommand &command) const
{
    return command.attribute(QScriptDebuggerCommand::ContextIndex).toInt();
}

int QScriptDebuggerBackendPrivate::argumentIndex(const QScriptDebuggerCommand &command) const
{
    return command.attribute(QScriptDebuggerCommand::ArgumentIndex).toInt();
}

QVariant QScriptDebuggerBackendPrivate::debuggerValue(const QScriptValue &value)
{
    return qVariantFromValue(QScriptDebuggerValue::fromScriptValue(value));
}

QVariant QScriptDebuggerBackendPrivate::debuggerValueList(const QScriptValueList &lst)
{
    QScriptDebuggerValueList result;
    for (int i = 0; i < lst.size(); ++i)
        result.append(QScriptDebuggerValue::fromScriptValue(lst.at(i)));
    return qVariantFromValue(result);
}

QScriptValue QScriptDebuggerBackendPrivate::scriptValue(const QVariant &arg) const
{
    Q_Q(const QScriptDebuggerBackend);
    return qvariant_cast<QScriptDebuggerValue>(arg).toScriptValue(q->engine());
}

QScriptValueList QScriptDebuggerBackendPrivate::scriptValueList(const QVariant &lst) const
{
    Q_Q(const QScriptDebuggerBackend);
    QScriptValueList result;
    QScriptDebuggerValueList srcList = qvariant_cast<QScriptDebuggerValueList>(lst);
    for (int i = 0; i < srcList.size(); ++i)
        result.append(srcList.at(i).toScriptValue(q->engine()));
    return result;
}

QScriptValue QScriptDebuggerBackendPrivate::trace(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue data = context->callee().data();
    QScriptDebuggerBackendPrivate *self = qscriptvalue_cast<QScriptDebuggerBackendPrivate*>(data);
    if (!self)
        return engine->undefinedValue();
    QString str;
    for (int i = 0; i < context->argumentCount(); ++i) {
        if (i > 0)
            str.append(QLatin1String(" "));
        str.append(context->argument(i).toString());
    }
    QScriptDebuggerEvent e(QScriptDebuggerEvent::Trace);
    e.setAttribute(QScriptDebuggerEvent::Text, str);
    self->q_func()->event(e);
    return engine->undefinedValue();
}

QScriptValue QScriptDebuggerBackendPrivate::pause(QScriptContext *context, QScriptEngine *engine)
{
    QScriptValue data = context->callee().data();
    QScriptDebuggerBackendPrivate *self = qscriptvalue_cast<QScriptDebuggerBackendPrivate*>(data);
    if (!self)
        return engine->undefinedValue();
    self->q_func()->pauseEvaluation();
    return engine->undefinedValue();
}

/*!
  Constructs a QScriptDebuggerBackend object for the given \a engine.
  The new object is not automatically set as the agent of the engine.

  \sa QScriptEngine::setAgent()
*/
QScriptDebuggerBackend::QScriptDebuggerBackend(QScriptEngine *engine)
//    : QScriptEngineAgent(*new QScriptDebuggerBackendPrivate, engine)
    : QScriptEngineAgent(engine),
        d_ptr(new QScriptDebuggerBackendPrivate)
{
    d_ptr->q_ptr = this;
}

/*!
  \internal
*/
QScriptDebuggerBackend::QScriptDebuggerBackend(QScriptDebuggerBackendPrivate &dd, QScriptEngine *engine)
//    : QScriptEngineAgent(dd, engine)
    : QScriptEngineAgent(engine),
        d_ptr(&dd)
{
    d_ptr->q_ptr = this;
    engine->setAgent(this);
}

/*!
  Destroys this QScriptDebuggerBackend.
*/
QScriptDebuggerBackend::~QScriptDebuggerBackend()
{
    delete d_ptr; // ###
}

/*!
  \fn void event(const QScriptDebuggerEvent &event)

  This function is called when the given \a event has occurred in the
  back-end.

  Reimplement this function to handle events.

  For events that should stall execution (e.g. breakpoint events),
  this function should block. Script execution will resume when this
  function returns.

  \sa resume()
*/

/*!
  Returns the execution state of this back-end.
*/
QScriptDebuggerBackend::State QScriptDebuggerBackend::state() const
{
    Q_D(const QScriptDebuggerBackend);
    return QScriptDebuggerBackend::State(d->state);
}

/*!
  \internal

  Sets the execution state of this back-end.

  \sa event()
*/
void QScriptDebuggerBackend::setState(State state)
{
    Q_D(QScriptDebuggerBackend);
    d->state = state;
    if ((state == SteppingOverState) || (state == SteppingOutState)) {
        if (engine()->isEvaluating())
            d->stepDepth = 0;
        else
            d->stepDepth = -1;
    }
}

/*!
  Sets the location that is the target when the back-end is in the
  RunningToLocationState state. The location is defined by the given
  \a fileName and \a lineNumber.
*/
void QScriptDebuggerBackend::setTargetLocation(const QString &fileName, int lineNumber)
{
    Q_D(QScriptDebuggerBackend);
    d->targetLocationFileName = fileName;
    d->targetLocationLineNumber = lineNumber;
    d->targetLocationScriptId = -1;
}

/*!
  Sets the location that is the target when the back-end is in the
  RunningToLocationState state. The location is defined by the given
  \a scriptId and \a lineNumber.
*/
void QScriptDebuggerBackend::setTargetLocation(qint64 scriptId, int lineNumber)
{
    Q_D(QScriptDebuggerBackend);
    d->targetLocationScriptId = scriptId;
    d->targetLocationLineNumber = lineNumber;
    d->targetLocationFileName = QString();
}

/*!
  Sets a breakpoint at the location specified by the given \a fileName
  and \a lineNumber, and returns a unique identifier associated with
  the breakpoint.

  A Breakpoint event will be generated when the breakpoint is triggered.

  \sa deleteBreakpoint(), event()
*/
int QScriptDebuggerBackend::setBreakpoint(const QString &fileName, int lineNumber)
{
    Q_D(QScriptDebuggerBackend);
    d->breakpointInfos.append(QScriptBreakpointInfo(fileName, lineNumber));
    int id = ++d->nextBreakpointId;
    d->breakpointIds.append(id);
    return id;
}

/*!
  Sets a breakpoint at the location specified by the given \a scriptId
  and \a lineNumber, and returns a unique identifier associated with
  the breakpoint.

  A Breakpoint event will be generated when the breakpoint is triggered.

  \sa deleteBreakpoint()
*/
int QScriptDebuggerBackend::setBreakpoint(qint64 scriptId, int lineNumber)
{
    Q_D(QScriptDebuggerBackend);
    d->breakpointInfos.append(QScriptBreakpointInfo(scriptId, lineNumber));
    int id = ++d->nextBreakpointId;
    d->breakpointIds.append(id);
    return id;
}

/*!
  Returns information for the breakpoint with the given \a id, or an
  invalid QScriptBreakInfo there is no breakpoint with that id.

  \sa breakpointIdentifiers()
*/
QScriptBreakpointInfo QScriptDebuggerBackend::breakpointInfo(int id) const
{
    Q_D(const QScriptDebuggerBackend);
    for (int i = 0; i < d->breakpointIds.size(); ++i) {
        if (d->breakpointIds.at(i) == id)
            return d->breakpointInfos.at(i);
    }
    return QScriptBreakpointInfo();
}

/*!
  Modifies the existing breakpoint with the given \a id to have the
  given \a info.
*/
bool QScriptDebuggerBackend::modifyBreakpoint(int id, const QScriptBreakpointInfo &info)
{
    Q_D(QScriptDebuggerBackend);
    for (int i = 0; i < d->breakpointIds.size(); ++i) {
        if (d->breakpointIds.at(i) == id) {
            d->breakpointInfos[i] = info;
            return true;
        }
    }
    return false;
}

/*!
  Returns the identifiers of all breakpoints.

  \sa breakpointInfo()
*/
QList<int> QScriptDebuggerBackend::breakpointIdentifiers() const
{
    Q_D(const QScriptDebuggerBackend);
    return d->breakpointIds;
}

/*!
  Deletes the breakpoint with the given \a id.
*/
bool QScriptDebuggerBackend::deleteBreakpoint(int id)
{
    Q_D(QScriptDebuggerBackend);
    for (int i = 0; i < d->breakpointIds.size(); ++i) {
        if (d->breakpointIds.at(i) == id) {
            d->breakpointIds.removeAt(i);
            d->breakpointInfos.removeAt(i);
            return true;
        }
    }
    return false;
}

/*!
  Deletes all breakpoints.
*/
void QScriptDebuggerBackend::deleteAllBreakpoints()
{
    Q_D(QScriptDebuggerBackend);
    d->breakpointIds.clear();
    d->breakpointInfos.clear();
}

/*!
  Returns the number of active contexts (frames).

  \sa context()
*/
int QScriptDebuggerBackend::contextCount() const
{
    Q_D(const QScriptDebuggerBackend);
    return d->scriptIdStack.size();
}

/*!
  Returns the context at the given \a index, or 0 if the index is not
  valid.

  index=0 represents the bottom (innermost) context;
  index=contextCount()-1 represents the initial (global) context.

  \sa contextCount()
*/
QScriptContext *QScriptDebuggerBackend::context(int index) const
{
    if (index < 0)
        return 0;
    QScriptContext *ctx = engine()->currentContext();
    while (ctx) {
        if (index == 0)
            return ctx;
        ctx = ctx->parentContext();
        --index;
    }
    return 0;
}

/*!
  Returns the original source code associated with the \a scriptId.
*/
QString QScriptDebuggerBackend::scriptContents(qint64 scriptId) const
{
    Q_D(const QScriptDebuggerBackend);
    QScriptInfo info = d->loadedScripts.value(scriptId);
    return info.contents();
}

/*!
  Returns \a count lines of the original source associated with the \a
  scriptId, starting at line \a lineNumber.
*/
QStringList QScriptDebuggerBackend::scriptLines(qint64 scriptId, int lineNumber, int count) const
{
    Q_D(const QScriptDebuggerBackend);
    QScriptInfo info = d->loadedScripts.value(scriptId);
    QString contents = info.contents();
    if (contents.isEmpty())
        return QStringList();
    QStringList allLines = contents.split(QLatin1Char('\n'));
    return allLines.mid(qMax(0, lineNumber - info.baseLineNumber()), count);
}

/*!
  Returns the filename associated with the \a scriptId (i.e. the
  string that was passed as argument to QScriptEngine::evaluate()).
*/
QString QScriptDebuggerBackend::scriptFileName(qint64 scriptId) const
{
    Q_D(const QScriptDebuggerBackend);
    QScriptInfo info = d->loadedScripts.value(scriptId);
    return info.fileName();
}

/*!
  Returns the base line number associated with the \a scriptId
  (i.e. the number that was passed as argument to
  QScriptEngine::evaluate()).
*/
int QScriptDebuggerBackend::scriptBaseLineNumber(qint64 scriptId) const
{
    Q_D(const QScriptDebuggerBackend);
    QScriptInfo info = d->loadedScripts.value(scriptId);
    return info.baseLineNumber();
}

/*!
  Registers the iterator \a it and returns a unique identifier
  associated with the iterator.

  The backend takes ownership of \a it.

  \sa findIterator(), unregisterIterator()
*/
int QScriptDebuggerBackend::registerIterator(QScriptValueIterator *it)
{
    Q_D(QScriptDebuggerBackend);
    int id = d->iterators.key(it);
    if (id != 0)
        return id;
    id = ++d->nextIteratorId;
    d->iterators.insert(id, it);
    return id;
}

/*!
  Returns the iterator identified by the given \a id, or 0 if no
  such iterator exists.

  \sa registerIterator()
*/
QScriptValueIterator *QScriptDebuggerBackend::findIterator(int id) const
{
    Q_D(const QScriptDebuggerBackend);
    return d->iterators.value(id);
}

/*!
  Unregisters the iterator identified by the given \a id and deletes
  it.

  \sa registerIterator()
*/
void QScriptDebuggerBackend::unregisterIterator(int id)
{
    Q_D(QScriptDebuggerBackend);
    QScriptValueIterator *it = d->iterators.take(id);
    delete it;
}

/*!
  Checkpoints the loaded scripts.

  \sa scriptsDelta(), listScripts()
*/
void QScriptDebuggerBackend::scriptsCheckpoint()
{
    Q_D(QScriptDebuggerBackend);
    d->previousCheckpointScripts = d->checkpointScripts;
    d->checkpointScripts = d->loadedScripts;
}

/*!
  Returns the difference between the scripts of the last checkpoint
  and the previous checkpoint.

  The result consists of two lists. The first list describes the
  scripts that have been loaded since the previous checkpoint. Each
  list entry is a list, containing the ID, filename and line number of
  the script; additionally, if \a includeContents is true, the
  contents is included. The second list contains the identifiers of
  the scripts that have been unloaded since the previous checkpoint.

  \sa scriptsCheckpoint()
*/
QVariantList QScriptDebuggerBackend::scriptsDelta(bool includeContents) const
{
    Q_D(const QScriptDebuggerBackend);
    QVariantList result;
    QSet<qint64> prevSet = d->previousCheckpointScripts.keys().toSet();
    QSet<qint64> currSet = d->checkpointScripts.keys().toSet();
    QSet<qint64> addedScriptIds = currSet - prevSet;
    QSet<qint64> removedScriptIds = prevSet - currSet;
    {
        // loaded
        QVariantList subresult;
        QSet<qint64>::const_iterator it;
        for (it = addedScriptIds.constBegin(); it != addedScriptIds.constEnd(); ++it) {
            QVariantList item;
            qint64 id = *it;
            item.append(id);
            QScriptInfo info = d->checkpointScripts.value(id);
            Q_ASSERT(info.isValid());
            item.append(info.fileName());
            item.append(info.baseLineNumber());
            if (includeContents)
                item.append(info.contents());
            subresult.append(item);
        }
        result.append(subresult);
    }
    {
        // unloaded
        QList<qint64> subresult;
        QSet<qint64>::const_iterator it;
        for (it = removedScriptIds.constBegin(); it != removedScriptIds.constEnd(); ++it)
            subresult.append(*it);
        result.append(qVariantFromValue(subresult));
    }
    return result;
}

/*!
  Returns a list of all the currently loaded scripts. Each list entry
  is a list, containing the ID, file name and line number of a script;
  if \a includeContents is true, the contents of the script is
  included as well.
*/
QVariantList QScriptDebuggerBackend::listScripts(bool includeContents) const
{
    Q_D(const QScriptDebuggerBackend);
    QVariantList result;
    QMap<qint64, QScriptInfo>::const_iterator it;
    for (it = d->loadedScripts.constBegin(); it != d->loadedScripts.constEnd(); ++it) {
        QVariantList item;
        item.append(it.key());
        const QScriptInfo &info = it.value();
        item.append(info.fileName());
        item.append(info.baseLineNumber());
        if (includeContents)
            item.append(info.contents());
        result.append(item);
    }
    return result;
}

/*!
  Steps into the next script statement. A step event() will be
  generated when stepping is completed.

  \sa stepOver(), stepOut()
*/
void QScriptDebuggerBackend::stepInto()
{
    setState(SteppingIntoState);
    resume();
}

/*!
  Steps over the next script statement. A step event() will be
  generated when stepping is completed.

  \sa stepInto(), stepOut()
*/
void QScriptDebuggerBackend::stepOver()
{
    setState(SteppingOverState);
    resume();
}

/*!
  Steps out of the current script function. A step event() will be
  generated when stepping is completed.

  \sa stepInto(), stepOver()
*/
void QScriptDebuggerBackend::stepOut()
{
    setState(SteppingOutState);
    resume();
}

/*!
  Runs to the location described by the given \a fileName and \a
  lineNumber.  An event() will be generated once the location has been
  reached.

  \sa setBreakpoint()
*/
void QScriptDebuggerBackend::runToLocation(const QString &fileName, int lineNumber)
{
    setTargetLocation(fileName, lineNumber);
    setState(RunningToLocationState);
    resume();
}

/*!
  Runs to the location described by the given \a scriptId and \a
  lineNumber.  An event() will be generated once the location has been
  reached.

  \sa setBreakpoint()
*/
void QScriptDebuggerBackend::runToLocation(qint64 scriptId, int lineNumber)
{
    setTargetLocation(scriptId, lineNumber);
    setState(RunningToLocationState);
    resume();
}

/*!
  Continues evaluation. Execution will continue until a pause is
  requested, or until a breakpoint is hit.

  \sa pauseEvaluation()
*/
void QScriptDebuggerBackend::continueEvalution()
{
    setState(NoState);
    resume();
}

/*!
  Requests that evaluation should be paused. When the next script
  statement is reached, an event() will be generated.

  \sa continueEvaluation()
*/
void QScriptDebuggerBackend::pauseEvaluation()
{
    setState(PausingState);
}

/*!
  Returns a trace function. The trace function has similar semantics
  to the built-in print() function; however, instead of writing text
  to standard output, it generates a trace event containing the text.
*/
QScriptValue QScriptDebuggerBackend::traceFunction() const
{
    Q_D(const QScriptDebuggerBackend);
    if (!engine())
        return QScriptValue();
    QScriptValue fun = engine()->newFunction(QScriptDebuggerBackendPrivate::trace);
    fun.setData(qScriptValueFromValue(engine(), const_cast<QScriptDebuggerBackendPrivate*>(d)));
    return fun;
}

/*!
  Returns a break function. When the break function is called, it will
  generate a Break event (i.e. it can be used to explicitly trigger
  the debugger from a script).
*/
QScriptValue QScriptDebuggerBackend::breakFunction() const
{
    Q_D(const QScriptDebuggerBackend);
    if (!engine())
        return QScriptValue();
    QScriptValue fun = engine()->newFunction(QScriptDebuggerBackendPrivate::pause);
    fun.setData(qScriptValueFromValue(engine(), const_cast<QScriptDebuggerBackendPrivate*>(d)));
    return fun;
}

static QScriptValueList scopeChain(QScriptContext *ctx)
{
    QScriptValueList result;
#if QT_VERSION >= 0x040500
    result = ctx->scopeChain();
#else
    result.append(ctx->activationObject());
    QScriptValue scope = ctx->callee().scope();
    while (scope.isObject()) {
        result.append(scope);
        scope = scope.scope();
    }
#endif
    return result;
}

/*!
  Applies the given \a command to this back-end.

  This function decodes the \a command by examining its type and
  attributes. It is typically called to apply commands created by
  a QScriptDebuggerFrontend.

  Reimplement this function to perform custom command execution.
*/
// ### this could be done by a separate class...
QScriptDebuggerResponse QScriptDebuggerBackend::applyCommand(
    const QScriptDebuggerCommand &command)
{
    Q_D(QScriptDebuggerBackend);
    QScriptDebuggerResponse response;
    switch (command.type()) {
    case QScriptDebuggerCommand::None:
        break;

    case QScriptDebuggerCommand::GetContextCount:
        response.setAttribute(QScriptDebuggerResponse::Result, contextCount());
        break;

    case QScriptDebuggerCommand::GetContextInfo: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptContextInfo result(ctx);
            response.setResult(qVariantFromValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetContextInfos: {
        QScriptContextInfoList result;
        QScriptContext *ctx = engine()->currentContext();
        while (ctx) {
            result.append(QScriptContextInfo(ctx));
            ctx = ctx->parentContext();
        }
        response.setResult(qVariantFromValue(result));
    }   break;

    case QScriptDebuggerCommand::GetContextString: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            response.setResult(ctx->toString());
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetScriptID: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptContextInfo info(ctx);
            response.setResult(info.scriptId());
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetBacktrace: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            response.setResult(ctx->backtrace());
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetThisObject: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptValue result = ctx->thisObject();
            response.setResult(d->debuggerValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::SetThisObject: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
            QScriptValue obj = d->scriptValue(arg);
            ctx->setThisObject(obj);
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetActivationObject: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptValue result = ctx->activationObject();
            response.setResult(d->debuggerValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::SetActivationObject: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
            QScriptValue obj = d->scriptValue(arg);
            ctx->setActivationObject(obj);
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetArgumentCount: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            response.setResult(ctx->argumentCount());
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetArgument: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            int argIndex = d->argumentIndex(command);
            if ((argIndex >= 0) && (argIndex < ctx->argumentCount())) {
                QScriptValue result = ctx->argument(argIndex);
                response.setResult(d->debuggerValue(result));
            } else {
                response.setError(QScriptDebuggerResponse::InvalidArgumentIndex);
            }
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetArguments: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptDebuggerValueList result;
            for (int i = 0; i < ctx->argumentCount(); ++i)
                result.append(QScriptDebuggerValue::fromScriptValue(ctx->argument(i)));
            response.setResult(qVariantFromValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetArgumentsObject: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptValue result = ctx->argumentsObject();
            response.setResult(d->debuggerValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetScopeChain: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptValueList result = scopeChain(ctx);
            response.setResult(d->debuggerValueList(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetCallee: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            QScriptValue result = ctx->callee();
            response.setResult(d->debuggerValue(result));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::IsCalledAsConstructor: {
        int contextIndex = d->contextIndex(command);
        QScriptContext *ctx = context(contextIndex);
        if (ctx) {
            bool result = ctx->isCalledAsConstructor();
            response.setResult(result);
        } else {
            response.setError(QScriptDebuggerResponse::InvalidContextIndex);
        }
    }   break;

    case QScriptDebuggerCommand::GetScriptContents: {
        qint64 scriptId = command.attribute(QScriptDebuggerCommand::ScriptID).toLongLong();
        QScriptInfo info = d->loadedScripts.value(scriptId);
        if (info.isValid()) {
            response.setResult(info.contents());
        } else {
            response.setError(QScriptDebuggerResponse::InvalidScriptID);
        }
    }   break;

    case QScriptDebuggerCommand::GetScriptLines: {
        qint64 scriptId = command.attribute(QScriptDebuggerCommand::ScriptID).toLongLong();
        QScriptInfo info = d->loadedScripts.value(scriptId);
        if (info.isValid()) {
            int lineNumber = command.attribute(QScriptDebuggerCommand::LineNumber).toInt();
            int count = command.attribute(QScriptDebuggerCommand::Count).toInt();
            response.setResult(scriptLines(scriptId, lineNumber, count));
        } else {
            response.setError(QScriptDebuggerResponse::InvalidScriptID);
        }
    }   break;

    case QScriptDebuggerCommand::AbortEvaluation: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        engine()->abortEvaluation(d->scriptValue(arg));
    }   break;

    case QScriptDebuggerCommand::CanEvaluate: {
        QString program = command.attribute(QScriptDebuggerCommand::Program).toString();
        bool result = engine()->canEvaluate(program);
        response.setAttribute(QScriptDebuggerResponse::Result, result);
    }   break;

    case QScriptDebuggerCommand::ClearExceptions:
        engine()->clearExceptions();
        break;

    case QScriptDebuggerCommand::CollectGarbage:
        engine()->collectGarbage();
        break;

    case QScriptDebuggerCommand::Evaluate: {
        QString program = command.attribute(QScriptDebuggerCommand::Program).toString();
        QString fileName = command.attribute(QScriptDebuggerCommand::FileName).toString();
        int lineNumber = command.attribute(QScriptDebuggerCommand::LineNumber).toInt();
        State oldState = state();
        setState(NoState);
        QScriptValue result = engine()->evaluate(program, fileName, lineNumber);
        setState(oldState);
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::GetGlobalObject: {
        QScriptValue result = engine()->globalObject();
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::HasUncaughtException: {
        response.setResult(engine()->hasUncaughtException());
    }   break;

    case QScriptDebuggerCommand::GetUncaughtException: {
        QScriptValue result = engine()->uncaughtException();
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::GetUncaughtExceptionLineNumber: {
        response.setResult(engine()->uncaughtExceptionLineNumber());
    }   break;

    case QScriptDebuggerCommand::GetUncaughtExceptionBacktrace: {
        response.setResult(engine()->uncaughtExceptionBacktrace());
    }   break;

    case QScriptDebuggerCommand::NewArray: {
        uint length = command.attribute(QScriptDebuggerCommand::Length).toUInt();
        QScriptValue result = engine()->newArray(length);
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::NewObject: {
        QScriptValue result = engine()->newObject();
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::PushContext:
        engine()->pushContext();
        break;

    case QScriptDebuggerCommand::PopContext:
        engine()->popContext();
        break;

    case QScriptDebuggerCommand::Call: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::FunctionObject);
        QScriptValue fun = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::ThisObject);
        QScriptValue thisObj = d->scriptValue(arg2);
        QVariant arg3 = command.attribute(QScriptDebuggerCommand::FunctionArguments);
        QScriptValueList funArgs = d->scriptValueList(arg3);
        QScriptValue result = fun.call(thisObj, funArgs);
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::Construct: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::FunctionObject);
        QScriptValue fun = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::FunctionArguments);
        QScriptValueList funArgs = d->scriptValueList(arg2);
        QScriptValue result = fun.construct(funArgs);
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::GetProperty: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::PropertyName);
        QString name = arg2.toString();
        QScriptValue result = obj.property(name);
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::SetProperty: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::PropertyName);
        QString name = arg2.toString();
        QVariant arg3 = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg3);
        obj.setProperty(name, val);
    }   break;

    case QScriptDebuggerCommand::GetPropertyFlags: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::PropertyName);
        QString name = arg2.toString();
        QScriptValue::PropertyFlags result = obj.propertyFlags(name);
        response.setResult(qVariantFromValue(result));
    }   break;

     case QScriptDebuggerCommand::GetPropertyNames: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QStringList result;
        QScriptValueIterator it(obj);
        while (it.hasNext()) {
            it.next();
            result.append(it.name());
        }
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::GetProperties: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::PropertyNames);
        QStringList names = arg2.toStringList();
        QScriptDebuggerValueList result;
        for (int i = 0; i < names.size(); ++i)
            result.append(QScriptDebuggerValue::fromScriptValue(obj.property(names.at(i))));
        response.setResult(qVariantFromValue(result));
    }   break;

    case QScriptDebuggerCommand::GetPrototype: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QScriptValue result = obj.prototype();
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::SetPrototype: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg1);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg2);
        obj.setPrototype(val);
    }   break;

    case QScriptDebuggerCommand::ToBoolean: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        bool result = val.toBoolean();
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ToDateTime: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        QDateTime result = val.toDateTime();
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ToNumber: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        double result = val.toNumber();
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ToObject: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        QScriptValue result = val.toObject();
        response.setResult(d->debuggerValue(result));
    }   break;

    case QScriptDebuggerCommand::ToRegExp: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        QRegExp result = val.toRegExp();
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ToString: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Value);
        QScriptValue val = d->scriptValue(arg);
        QString result = val.toString();
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::NewIterator: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::Object);
        QScriptValue obj = d->scriptValue(arg);
        QScriptValueIterator *it = new QScriptValueIterator(obj);
        int id = registerIterator(it);
        response.setResult(id);
    }   break;

    case QScriptDebuggerCommand::IteratorFlags: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            response.setResult(qVariantFromValue(it->flags()));
    }   break;

    case QScriptDebuggerCommand::IteratorHasNext: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            response.setResult(it->hasNext());
    }   break;

    case QScriptDebuggerCommand::IteratorHasPrevious: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            response.setResult(it->hasPrevious());
    }   break;

    case QScriptDebuggerCommand::IteratorName: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            response.setResult(it->name());
    }   break;

    case QScriptDebuggerCommand::IteratorNext: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            it->next();
    }   break;

    case QScriptDebuggerCommand::IteratorPrevious: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            it->previous();
    }   break;

    case QScriptDebuggerCommand::IteratorRemove: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            it->remove();
    }   break;

    case QScriptDebuggerCommand::IteratorSetValue: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it) {
            QVariant arg2 = command.attribute(QScriptDebuggerCommand::Value);
            QScriptValue val = d->scriptValue(arg2);
            it->setValue(val);
        }
    }   break;

    case QScriptDebuggerCommand::IteratorToBack: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            it->toBack();
    }   break;

    case QScriptDebuggerCommand::IteratorToFront: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            it->toFront();
    }   break;

    case QScriptDebuggerCommand::IteratorValue: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it)
            response.setResult(d->debuggerValue(it->value()));
    }   break;

    case QScriptDebuggerCommand::IteratorAssign: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it) {
            QVariant arg2 = command.attribute(QScriptDebuggerCommand::Object);
            QScriptValue obj = d->scriptValue(arg2);
            it->operator=(obj);
        }
    }   break;

    case QScriptDebuggerCommand::IteratorProperty: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it) {
            QScriptDebuggerValue value = QScriptDebuggerValue::fromScriptValue(it->value());
            QScriptDebuggerProperty prop(it->name(), value, it->flags());
            response.setResult(qVariantFromValue(prop));
        }
    }   break;

    case QScriptDebuggerCommand::IteratorNextProperty: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it && it->hasNext()) {
            it->next();
            QScriptDebuggerValue value = QScriptDebuggerValue::fromScriptValue(it->value());
            QScriptDebuggerProperty prop(it->name(), value, it->flags());
            response.setResult(qVariantFromValue(prop));
        }
    }   break;

    case QScriptDebuggerCommand::IteratorNextProperties: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg1.toInt();
        QScriptValueIterator *it = findIterator(id);
        if (it) {
            QVariant arg2 = command.attribute(QScriptDebuggerCommand::Count);
            int count = arg2.toInt();
            int n = -1;
            QScriptDebuggerPropertyList result;
            while (it->hasNext() && ((++n < count) || (count == 0))) {
                it->next();
                QScriptDebuggerValue value = QScriptDebuggerValue::fromScriptValue(it->value());
                QScriptDebuggerProperty prop(it->name(), value, it->flags());
                result.append(prop);
            }
            response.setResult(qVariantFromValue(result));
        }
    }   break;

    case QScriptDebuggerCommand::DeleteIterator: {
        QVariant arg = command.attribute(QScriptDebuggerCommand::IteratorID);
        int id = arg.toInt();
        unregisterIterator(id);
    }   break;

    case QScriptDebuggerCommand::Break:
        pauseEvaluation();
        break;

    case QScriptDebuggerCommand::Continue:
        continueEvalution();
        break;

    case QScriptDebuggerCommand::StepInto:
        stepInto();
        break;

    case QScriptDebuggerCommand::StepOver:
        stepOver();
        break;

    case QScriptDebuggerCommand::StepOut:
        stepOut();
        break;

    case QScriptDebuggerCommand::Resume:
        resume();
        break;

    case QScriptDebuggerCommand::RunToLocation: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::FileName);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::LineNumber);
        QString fileName = arg1.toString();
        int lineNumber = arg2.toInt();
        runToLocation(fileName, lineNumber);
    }   break;

    case QScriptDebuggerCommand::RunToLocationByID: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::ScriptID);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::LineNumber);
        qint64 scriptId = arg1.toLongLong();
        int lineNumber = arg2.toInt();
        runToLocation(scriptId, lineNumber);
    }   break;

    case QScriptDebuggerCommand::SetBreakpoint: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::FileName);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::LineNumber);
        QString fileName = arg1.toString();
        int lineNumber = arg2.toInt();
        int id = setBreakpoint(fileName, lineNumber);
        response.setResult(id);
    }   break;

    case QScriptDebuggerCommand::SetBreakpointByScriptID: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::ScriptID);
        QVariant arg2 = command.attribute(QScriptDebuggerCommand::LineNumber);
        qint64 scriptId = arg1.toLongLong();
        int lineNumber = arg2.toInt();
        int id = setBreakpoint(scriptId, lineNumber);
        response.setResult(id);
    }   break;

    case QScriptDebuggerCommand::DeleteBreakpoint: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::BreakpointID);
        int id = arg1.toInt();
        if (!d->isValidBreakpointId(id))
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
        else
            deleteBreakpoint(id);
    }   break;

    case QScriptDebuggerCommand::DeleteAllBreakpoints:
        deleteAllBreakpoints();
        break;

    case QScriptDebuggerCommand::ModifyBreakpoint: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::BreakpointID);
        int id = arg1.toInt();
        if (!d->isValidBreakpointId(id)) {
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
        } else {
            QVariant arg2 = command.attribute(QScriptDebuggerCommand::BreakpointInfo);
            QScriptBreakpointInfo info = qvariant_cast<QScriptBreakpointInfo>(arg2);
            modifyBreakpoint(id, info);
        }
    }   break;

    case QScriptDebuggerCommand::GetBreakpointInfo: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::BreakpointID);
        int id = arg1.toInt();
        if (!d->isValidBreakpointId(id))
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
        else
            response.setResult(qVariantFromValue(breakpointInfo(id)));
    }   break;

    case QScriptDebuggerCommand::GetBreakpointIdentifiers:
        response.setResult(qVariantFromValue(breakpointIdentifiers()));
        break;

    case QScriptDebuggerCommand::SetBreakpointEnabled: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::BreakpointID);
        int id = arg1.toInt();
        if (!d->isValidBreakpointId(id)) {
            response.setError(QScriptDebuggerResponse::InvalidBreakpointID);
        } else {
            QVariant arg2 = command.attribute(QScriptDebuggerCommand::Enabled);
            bool enabled = arg2.toBool();
            QScriptBreakpointInfo info = breakpointInfo(id);
            info.setEnabled(enabled);
            modifyBreakpoint(id, info);
        }
    }   break;

    case QScriptDebuggerCommand::ListScripts: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::IncludeScriptContents);
        bool includeContents = arg1.toBool();
        QVariantList result = listScripts(includeContents);
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ScriptsCheckpoint: {
        scriptsCheckpoint();
    }   break;

    case QScriptDebuggerCommand::GetScriptsDelta: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::IncludeScriptContents);
        bool includeContents = arg1.toBool();
        QVariantList result = scriptsDelta(includeContents);
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::ScriptsCheckpointAndDelta: {
        QVariant arg1 = command.attribute(QScriptDebuggerCommand::IncludeScriptContents);
        bool includeContents = arg1.toBool();
        scriptsCheckpoint();
        QVariantList result = scriptsDelta(includeContents);
        response.setResult(result);
    }   break;

    case QScriptDebuggerCommand::UserCommand:
    case QScriptDebuggerCommand::MaxUserCommand:
        break;
    }
    return response;
}

/*!
  \fn void QScriptDebuggerBackend::resume()

  This function is called when control should be returned back to the
  back-end, i.e. when script evaluation should be resumed after an
  event has been delivered.

  Subclasses must reimplement this function to make event() return.

  \sa event()
*/

/*!
  \fn void QScriptDebuggerBackend::event(const QScriptDebuggerEvent &event)

  This function is called when the back-end has generated the given \a event.

  Subclasses must reimplement this function to handle the
  event. Typically the event is forwarded to a
  QScriptDebuggerFrontend, which will in turn forward it to its
  QScriptDebuggerClient. The client may then query the front-end for
  information about the execution state, and call e.g.
  continueEvalution() to resume execution. This function should block
  until resume() is called.

  \sa resume()
*/

/*!
  \reimp
*/
void QScriptDebuggerBackend::scriptLoad(qint64 id, const QString &program,
                                 const QString &fileName, int baseLineNumber)
{
    Q_D(QScriptDebuggerBackend);
    QScriptInfo info = QScriptInfo(program, fileName, baseLineNumber);
    d->loadedScripts.insert(id, info);
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::scriptUnload(qint64 id)
{
    Q_D(QScriptDebuggerBackend);
    d->loadedScripts.take(id);
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::contextPush()
{
    Q_D(QScriptDebuggerBackend);
    d->scriptIdStack.append(QList<qint64>());
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::contextPop()
{
    Q_D(QScriptDebuggerBackend);
    d->scriptIdStack.removeLast();
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::functionEntry(qint64 scriptId)
{
    Q_D(QScriptDebuggerBackend);
    QList<qint64> &ids = d->scriptIdStack.last();
    ids.append(scriptId);
    if ((d->state == SteppingOverState) || (d->state == SteppingOutState))
        ++d->stepDepth;
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::functionExit(qint64 scriptId,
                                          const QScriptValue &returnValue)
{
    Q_UNUSED(scriptId);
    Q_D(QScriptDebuggerBackend);
    QList<qint64> &ids = d->scriptIdStack.last();
    ids.removeLast();
    if (d->state == SteppingOverState) {
        if (--d->stepDepth <= 0)
            d->state = SteppedOverState;
    } else if (d->state == SteppingOutState) {
        if (--d->stepDepth < 0) {
            d->stepOutValue = returnValue;
            d->state = SteppedOutState;
        }
    }
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::positionChange(qint64 scriptId,
                                            int lineNumber, int columnNumber)
{
    Q_D(QScriptDebuggerBackend);
    Q_UNUSED(columnNumber);
    if (!d->breakpointIds.isEmpty()) {
        // check if we hit a breakpoint
        for (int i = 0; i < d->breakpointInfos.size(); ++i) {
            QScriptBreakpointInfo &info = d->breakpointInfos[i];
            if (!info.isEnabled())
                continue;
            if (info.lineNumber() != lineNumber)
                continue;
            if (info.scriptId() != -1) {
                if (info.scriptId() != scriptId)
                    continue;
            } else {
                QScriptContext *ctx = engine()->currentContext();
                QScriptContextInfo ctxInfo(ctx);
                if (info.fileName() != ctxInfo.fileName())
                    continue;
            }
            if (!info.condition().isEmpty()) {
                // ### careful, evaluating the condition might cause an exception
                QScriptValue ret = engine()->evaluate(info.condition());
                if (!ret.toBoolean())
                    continue;
            }
            int ignore = info.ignoreCount();
            if (ignore != 0) {
                info.setIgnoreCount(ignore - 1);
                continue;
            }
            // trigger!
            info.bumpHitCount();
            int bpId = d->breakpointIds.at(i);
            bool singleShot = info.isSingleShot();
            QScriptDebuggerEvent e(QScriptDebuggerEvent::Breakpoint);
            e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
            e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
            e.setAttribute(QScriptDebuggerEvent::BreakpointID, bpId);
            if (!info.fileName().isEmpty()) {
                e.setAttribute(QScriptDebuggerEvent::FileName, info.fileName());
            } else {
                QScriptContext *ctx = engine()->currentContext();
                QScriptContextInfo ctxInfo(ctx);
                e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
            }
            event(e);
            if (singleShot)
                deleteBreakpoint(bpId);
            return;
        }
    }

    switch (d->state) {
    case NoState:
    case SteppingOutState:
        // Do nothing
        break;

    case SteppingIntoState: {
        d->state = SteppedIntoState;
        QScriptDebuggerEvent e(QScriptDebuggerEvent::SteppingFinished);
        e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
        e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
        QScriptContext *ctx = engine()->currentContext();
        QScriptContextInfo ctxInfo(ctx);
        e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
        event(e);
    }   break;

    case SteppingOverState:
        if (0 != d->stepDepth)
            break;
        // fallthrough
        d->state = SteppedOverState;
    case SteppedOverState: {
        QScriptDebuggerEvent e(QScriptDebuggerEvent::SteppingFinished);
        e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
        e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
        QScriptContext *ctx = engine()->currentContext();
        QScriptContextInfo ctxInfo(ctx);
        e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
        event(e);
    }   break;

    case SteppedOutState: {
        QScriptDebuggerEvent e(QScriptDebuggerEvent::SteppingFinished);
        e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
        e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
        QScriptContext *ctx = engine()->currentContext();
        QScriptContextInfo ctxInfo(ctx);
        e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
        e.setAttribute(QScriptDebuggerEvent::ReturnValue, d->debuggerValue(d->stepOutValue));
        event(e);
    }   break;

    case RunningToLocationState: {
        if (lineNumber == d->targetLocationLineNumber) {
            bool reached = false;
            if (d->targetLocationScriptId == scriptId) {
                reached = true;
            } else {
                QScriptContext *ctx = engine()->currentContext();
                QScriptContextInfo ctxInfo(ctx);
                reached = (ctxInfo.fileName() == d->targetLocationFileName);
            }
            if (reached) {
                d->state = ReachedLocationState;
                QScriptDebuggerEvent e(QScriptDebuggerEvent::LocationReached);
                e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
                e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
                if (!d->targetLocationFileName.isEmpty()) {
                    e.setAttribute(QScriptDebuggerEvent::FileName, d->targetLocationFileName);
                } else {
                    QScriptContext *ctx = engine()->currentContext();
                    QScriptContextInfo ctxInfo(ctx);
                    e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
                }
                event(e);
            }
        }
    }   break;

    case PausingState: {
        d->state = PausedState;
        QScriptDebuggerEvent e(QScriptDebuggerEvent::Break);
        e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
        e.setAttribute(QScriptDebuggerEvent::LineNumber, lineNumber);
        QScriptContext *ctx = engine()->currentContext();
        QScriptContextInfo ctxInfo(ctx);
        e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
        event(e);
    }   break;
    }
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::exceptionThrow(qint64 scriptId,
                                            const QScriptValue &exception,
                                            bool hasHandler)
{
    Q_D(QScriptDebuggerBackend);
    QScriptDebuggerEvent e(QScriptDebuggerEvent::Exception);
    e.setAttribute(QScriptDebuggerEvent::ScriptID, scriptId);
    e.setAttribute(QScriptDebuggerEvent::ExceptionValue, d->debuggerValue(exception));
    e.setAttribute(QScriptDebuggerEvent::ExceptionString, exception.toString());
    e.setAttribute(QScriptDebuggerEvent::HasExceptionHandler, hasHandler);
    QScriptContext *ctx = engine()->currentContext();
    QScriptContextInfo ctxInfo(ctx);
    e.setAttribute(QScriptDebuggerEvent::FileName, ctxInfo.fileName());
    e.setAttribute(QScriptDebuggerEvent::LineNumber, ctxInfo.lineNumber());
    event(e);
}

/*!
  \reimp
*/
void QScriptDebuggerBackend::exceptionCatch(qint64 scriptId,
                                            const QScriptValue &exception)
{
    Q_UNUSED(scriptId);
    Q_UNUSED(exception);
}

/*!
  \reimp
*/
bool QScriptDebuggerBackend::supportsExtension(Extension extension) const
{
    Q_UNUSED(extension);
    return false;
}

/*!
  \reimp
*/
QVariant QScriptDebuggerBackend::extension(Extension extension,
                                           const QVariant &argument)
{
    Q_UNUSED(extension);
    Q_UNUSED(argument);
    return QVariant();
}
