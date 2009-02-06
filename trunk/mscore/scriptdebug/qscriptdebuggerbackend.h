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

#ifndef QSCRIPTDEBUGGERBACKEND_H
#define QSCRIPTDEBUGGERBACKEND_H

#include "qscriptdebugglobal.h"

#include <QtScript/qscriptengineagent.h>

#include <QtCore/qlist.h>

class QScriptBreakpointInfo;
class QScriptContext;
class QScriptDebuggerCommand;
class QScriptDebuggerEvent;
class QScriptDebuggerResponse;
class QScriptValueIterator;
class QStringList;

class QScriptDebuggerBackendPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebuggerBackend : public QScriptEngineAgent
{
public:
    enum State {
        NoState,
        SteppingIntoState,
        SteppedIntoState,
        SteppingOverState,
        SteppedOverState,
        SteppingOutState,
        SteppedOutState,
        RunningToLocationState,
        ReachedLocationState,
        PausingState,
        PausedState
    };

    QScriptDebuggerBackend(QScriptEngine *engine);
    ~QScriptDebuggerBackend();

    int contextCount() const;
    QScriptContext *context(int index) const;

    QString scriptContents(qint64 scriptId) const;
    QStringList scriptLines(qint64 scriptId, int lineNumber, int count) const;
    QString scriptFileName(qint64 scriptId) const;
    int scriptBaseLineNumber(qint64 scriptId) const;

    void stepInto();
    void stepOver();
    void stepOut();
    void continueEvalution();
    void pauseEvaluation();
    void runToLocation(const QString &fileName, int lineNumber);
    void runToLocation(qint64 scriptId, int lineNumber);

    State state() const;
    void setState(State state);

    void setTargetLocation(const QString &fileName, int lineNumber);
    void setTargetLocation(qint64 scriptId, int lineNumber);

    int setBreakpoint(const QString &fileName, int lineNumber);
    int setBreakpoint(qint64 scriptId, int lineNumber);

    QScriptBreakpointInfo breakpointInfo(int id) const;
    QList<int> breakpointIdentifiers() const;

    bool modifyBreakpoint(int id, const QScriptBreakpointInfo &info);
    bool deleteBreakpoint(int id);
    void deleteAllBreakpoints();

    int registerIterator(QScriptValueIterator *it);
    QScriptValueIterator *findIterator(int id) const;
    void unregisterIterator(int id);

    void scriptsCheckpoint();
    QVariantList scriptsDelta(bool includeContents) const;
    QVariantList listScripts(bool includeContents) const;

    QScriptValue traceFunction() const;
    QScriptValue breakFunction() const;

    virtual void event(const QScriptDebuggerEvent &event) = 0;

    virtual QScriptDebuggerResponse applyCommand(const QScriptDebuggerCommand &command);

    void scriptLoad(qint64 id, const QString &program,
                    const QString &fileName, int baseLineNumber);
    void scriptUnload(qint64 id);

    void contextPush();
    void contextPop();

    void functionEntry(qint64 scriptId);
    void functionExit(qint64 scriptId,
                      const QScriptValue &returnValue);

    void positionChange(qint64 scriptId,
                        int lineNumber, int columnNumber);

    void exceptionThrow(qint64 scriptId,
                        const QScriptValue &exception,
                        bool hasHandler);
    void exceptionCatch(qint64 scriptId,
                        const QScriptValue &exception);

    bool supportsExtension(Extension extension) const;
    QVariant extension(Extension extension,
                       const QVariant &argument = QVariant());

protected:
    virtual void resume() = 0;

protected:
    QScriptDebuggerBackend(QScriptDebuggerBackendPrivate &dd, QScriptEngine *engine);
    QScriptDebuggerBackendPrivate *d_ptr; // ###

private:
    Q_DECLARE_PRIVATE(QScriptDebuggerBackend)
    Q_DISABLE_COPY(QScriptDebuggerBackend)
};

#endif
