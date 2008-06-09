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

#ifndef QSCRIPTDEBUGGERFRONTEND_H
#define QSCRIPTDEBUGGERFRONTEND_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobject.h>

#include <QtCore/qstring.h>
#include <QtCore/qstringlist.h>
#include "qscriptdebuggervalue.h"

class QScriptDebuggerClient;
class QScriptDebuggerCommand;
class QScriptDebuggerEvent;
class QScriptDebuggerResponse;
class QScriptBreakpointInfo;

// ### make QObject?
// ### how to do response timeout notification?
class QScriptDebuggerFrontendPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerFrontend
{
public:
    QScriptDebuggerFrontend();
    virtual ~QScriptDebuggerFrontend();

    QScriptDebuggerClient *client() const;
    void setClient(QScriptDebuggerClient *client);

    int scheduleGetContextCount();
    int scheduleGetContextInfo(int index);
    int scheduleGetContextInfos();
    int scheduleGetContextString(int index);
    int scheduleGetScriptId(int contextIndex);
    int scheduleGetBacktrace(int contextIndex);
    int scheduleGetThisObject(int contextIndex);
    int scheduleSetThisObject(int contextIndex,
                              const QScriptDebuggerValue &object);
    int scheduleGetActivationObject(int contextIndex);
    int scheduleSetActivationObject(int contextIndex,
                                    const QScriptDebuggerValue &object);
    int scheduleGetScopeChain(int contextIndex);
    int scheduleGetArgumentsObject(int contextIndex);
    int scheduleGetArgumentCount(int contextIndex);
    int scheduleGetArgument(int contextIndex, int argIndex);
    int scheduleGetArguments(int contextIndex);
    int scheduleGetCallee(int contextIndex);
    int scheduleIsCalledAsConstructor(int contextIndex);
    // ### a way to evaluate an expression in a given frame

    int scheduleGetScriptContents(qint64 scriptId);
    int scheduleGetScriptLines(qint64 scriptId, int lineNumber, int count);
    int scheduleListScripts(bool includeScriptContents);
    int scheduleScriptsCheckpoint();
    int scheduleGetScriptsDelta(bool includeScriptContents);
    int scheduleScriptsCheckpointAndDelta(bool includeScriptContents);

    int scheduleAbortEvaluation(const QScriptDebuggerValue &result);
    int scheduleCanEvaluate(const QString &program);
    int scheduleClearExceptions();
    int scheduleCollectGarbage();
    int scheduleEvaluate(const QString &program,
                         const QString &fileName = QString(),
                         int lineNumber = 1);
    int scheduleGetGlobalObject();
    int scheduleNewArray(uint length = 0);
    int scheduleNewObject();
    int schedulePushContext();
    int schedulePopContext();
    int scheduleHasUncaughtException();
    int scheduleGetUncaughtException();
    int scheduleGetUncaughtExceptionLineNumber();
    int scheduleGetUncaughtExceptionBacktrace();

    int scheduleCall(const QScriptDebuggerValue &function,
                     const QScriptDebuggerValue &thisObject,
                     const QScriptDebuggerValueList &arguments);
    int scheduleConstruct(const QScriptDebuggerValue &function,
                          const QScriptDebuggerValueList &arguments);
    // ### rename to scheduleGetValue()?
    int scheduleGetProperty(const QScriptDebuggerValue &object,
                            const QString &propertyName);
    int scheduleSetProperty(const QScriptDebuggerValue &object,
                            const QString &propertyName,
                            const QScriptDebuggerValue &value);
    int scheduleGetPropertyFlags(const QScriptDebuggerValue &object,
                                 const QString &propertyName);
    int scheduleGetPropertyNames(const QScriptDebuggerValue &object);
    int scheduleGetProperties(const QScriptDebuggerValue &object,
                              const QStringList &propertyNames);
    int scheduleGetPrototype(const QScriptDebuggerValue &object);
    int scheduleSetPrototype(const QScriptDebuggerValue &object,
                             const QScriptDebuggerValue &prototype);
    int scheduleToBoolean(const QScriptDebuggerValue &value);
    int scheduleToDateTime(const QScriptDebuggerValue &value);
    int scheduleToNumber(const QScriptDebuggerValue &value);
    int scheduleToObject(const QScriptDebuggerValue &value);
    int scheduleToRegExp(const QScriptDebuggerValue &value);
    int scheduleToString(const QScriptDebuggerValue &value);

    int scheduleNewIterator(const QScriptDebuggerValue &object);
    int scheduleIteratorFlags(int id);
    int scheduleIteratorHasNext(int id);
    int scheduleIteratorHasPrevious(int id);
    int scheduleIteratorName(int id);
    int scheduleIteratorNext(int id);
    int scheduleIteratorPrevious(int id);
    int scheduleIteratorRemove(int id);
    int scheduleIteratorSetValue(int id,
                                 const QScriptDebuggerValue &value);
    int scheduleIteratorToBack(int id);
    int scheduleIteratorToFront(int id);
    int scheduleIteratorValue(int id);
    int scheduleIteratorAssign(int id,
                               const QScriptDebuggerValue &object);
    int scheduleIteratorProperty(int id);
    int scheduleIteratorNextProperty(int id);
    int scheduleIteratorNextProperties(int id, int maxCount);
    int scheduleDeleteIterator(int id);

    int scheduleBreak();
    int scheduleContinue();
    int scheduleStepInto();
    int scheduleStepOver();
    int scheduleStepOut();
    int scheduleRunToLocation(const QString &fileName, int lineNumber);
    int scheduleRunToLocation(qint64 scriptId, int lineNumber);
    int scheduleResume();

    int scheduleSetBreakpoint(const QString &fileName, int lineNumber);
    int scheduleSetBreakpoint(qint64 scriptId, int lineNumber);
    int scheduleDeleteBreakpoint(int id);
    int scheduleDeleteAllBreakpoints();
    int scheduleGetBreakpointIdentifiers();
    int scheduleGetBreakpointInfo(int id);
    int scheduleSetBreakpointEnabled(int id, bool enabled);
    int scheduleModifyBreakpoint(int id, const QScriptBreakpointInfo &info);

protected:
    int scheduleCommand(const QScriptDebuggerCommand &command);

    void notifyCommandFinished(int id, const QScriptDebuggerResponse &response);
    void notifyEvent(const QScriptDebuggerEvent &event);

    // ### make a QScriptDebuggerDelegate class?
    virtual void processCommand(int id, const QScriptDebuggerCommand &command) = 0;

protected:
    QScriptDebuggerFrontend(QScriptDebuggerFrontendPrivate &dd);
    QScriptDebuggerFrontendPrivate *d_ptr;

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerFrontend)
    Q_DISABLE_COPY(QScriptDebuggerFrontend)
};

#endif
