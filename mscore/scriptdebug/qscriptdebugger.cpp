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

#include "qscriptdebugger.h"

#include "qscriptdebuggerclient.h"
#include "qscriptdebuggerevent.h"
#include "qscriptdebuggerfrontend.h"
#include "qscriptdebuggerresponse.h"
#include "qscriptdebuggervalue.h"
#include "qscriptdebuggerproperty.h"
#include "qscriptbreakpointinfo.h"
#include "qscriptdebuggerstackwidget_p.h"
#include "qscriptdebuggercodewidget_p.h"
#include "qscriptdebuggerscriptswidget_p.h"
#include "qscriptdebuggerlocalswidget_p.h"
#include "qscriptdebuggerbreakpointswidget_p.h"
#include "qscriptdebuggerconsolewidget_p.h"
#include <QtCore/qfileinfo.h>
#include <QtCore/qvariant.h>
#include <QtCore/qdebug.h>
#include <QtCore/qdatetime.h>
#include <QtGui/qaction.h>
#include <QtGui/qstandarditemmodel.h>
#include <QtScript/qscriptcontextinfo.h>

#include <QtGui/qmainwindow.h>
#include <QtGui/qdockwidget.h>
#include <QtGui/qtoolbar.h>

//#include <private/qobject_p.h>

Q_DECLARE_METATYPE(QList<qint64>)
Q_DECLARE_METATYPE(QScriptContextInfo)
Q_DECLARE_METATYPE(QList<QScriptContextInfo>)
Q_DECLARE_METATYPE(QScriptDebuggerValue)
Q_DECLARE_METATYPE(QScriptDebuggerValueList)
Q_DECLARE_METATYPE(QScriptDebuggerPropertyList)

/*!
  \class QScriptDebugger

  \brief The QScriptDebugger class provides a graphical debugger.

  This class gives access to individual components of the
  debugger. This makes it possible to customize how you want to
  integrate the debugger into your application. The initMainWindow()
  function provides a default main window setup. See also
  QScriptEmbeddedDebugger.

  To use this class, you must first provide the debugger with a
  QScriptDebuggerFrontend; see for example
  QScriptEngineDebuggerFrontend.  Call setFrontend() to set the
  front-end that the debugger should use.

  The debugger user interface consists of several widgets.

  \table
  \header
    \o Name
    \o Accessor function
    \o Description
  \row
    \o Scripts widget
    \o scriptsWidget()
    \o Lists the scripts that are currently loaded.
  \row
    \o Code widget
    \o codeWidget()
    \o Displays the contents of a script.
  \row
    \o Stack widget
    \o stackWidget()
    \o Shows the backtrace.
  \row
    \o Locals widget
    \o localsWidget()
    \o Shows the local variables of a stack frame.
  \row
    \o Breakpoints widget
    \o breakpointsWidget()
    \o Shows breakpoints.
  \row
    \o Console widget
    \o consoleWidget()
    \o Displays status messages and debug output.
  \endtable

  \sa QScriptEmbeddedDebugger
*/

//
// The model declarations
//

class ScriptItem : public QStandardItem
{
public:
    ScriptItem(qint64 id, const QString &fileName, int lineNumber)
        : m_id(id), m_fileName(fileName), m_lineNumber(lineNumber)
        { }

    qint64 id() const { return m_id; }
    QString fileName() const { return m_fileName; }
    int lineNumber() const { return m_lineNumber; }

    QVariant data(int role) const
    {
        if (role == Qt::DisplayRole) {
            if (m_fileName.isEmpty())
                return QObject::tr("<unnamed script>");
            return QFileInfo(m_fileName).fileName();
        } else if (role == Qt::ToolTipRole) {
            if (QFileInfo(m_fileName).fileName() != m_fileName)
                return m_fileName;
        }
        return QVariant();
    }

private:
    qint64 m_id;
    QString m_fileName;
    int m_lineNumber;
};

class ScriptsModel : public QStandardItemModel
{
public:
    ScriptsModel(QObject *parent = 0)
        : QStandardItemModel(parent) {}

    ScriptItem *itemFromId(qint64 id) const;
};


class StackModel : public QAbstractTableModel
{
public:
    StackModel(QObject *parent = 0)
        : QAbstractTableModel(parent) {}

    void update(const QList<QScriptContextInfo> &infos);
    QScriptContextInfo info(int frameIndex) const
        { return m_infos.at(frameIndex); }

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation, int role) const;

private:
    QList<QScriptContextInfo> m_infos;
};


class LocalNode
{
public:
    LocalNode(const QScriptDebuggerProperty &prop, LocalNode *parent)
        : m_prop(prop), m_parent(parent), m_populated(false) {}
    ~LocalNode() { qDeleteAll(m_children); }

    QScriptDebuggerProperty m_prop;
    LocalNode *m_parent;
    QList<LocalNode*> m_children;
    bool m_populated;
};

class DebuggerClient;

class LocalsModel : public QAbstractItemModel
{
public:
    LocalsModel(DebuggerClient *client, QObject *parent = 0);

    void clear();
    void add(const QScriptDebuggerPropertyList &props);
    void populateNode(LocalNode *node, const QScriptDebuggerPropertyList &props);

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    QModelIndex index(int row, int column, const QModelIndex &parent) const;
    QModelIndex parent(const QModelIndex &index) const;
    bool hasChildren(const QModelIndex &parent) const;
    bool canFetchMore(const QModelIndex &parent) const;
    void fetchMore(const QModelIndex &parent);

    LocalNode *nodeFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromNode(LocalNode *node) const;

private:
    DebuggerClient *m_client;
    LocalNode *m_activationNode;
};


class BreakpointsModel : public QAbstractTableModel
{
public:
    BreakpointsModel(ScriptsModel *scripts, QObject *parent = 0)
        : QAbstractTableModel(parent), m_scripts(scripts) {}

    int columnCount(const QModelIndex &parent) const;
    int rowCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation, int role) const;

    void addBreakpoint(int bpId, qint64 scriptId, int lineNumber);
    void removeBreakpoint(int bpId);

    int findBreakpoint(qint64 scriptId, int lineNumber) const;

private:
    ScriptsModel *m_scripts;
    QList<int> m_ids;
    QList<qint64> m_scriptIds;
    QList<int> m_lineNumbers;
};

//
//
//

class DebuggerClient;

class DebuggerClientJob
{
public:
    enum Status {
        NotDone,
        Done
    };

    DebuggerClientJob(DebuggerClient *client)
        : m_client(client) {}
    virtual ~DebuggerClientJob() {}

    virtual void start() = 0;
    virtual Status handleResponse(int commandId, const QScriptDebuggerResponse &response) = 0;

    DebuggerClient *client() const;
    QScriptDebuggerFrontend *frontend() const;
    QScriptDebuggerPrivate *debugger() const;

private:
    DebuggerClient *m_client;
};



class QScriptDebuggerPrivate
//    : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(QScriptDebugger)
public:
    QScriptDebuggerPrivate(QScriptDebugger *q);
    ~QScriptDebuggerPrivate();

    void emitExecutionHalted();
    void prepareExecute();
    void initActions();
    void initWidgets();
    void enableOrDisableActions(bool running);

    void maybeShowScript(qint64 scriptId, int executionLineNumber);

    DebuggerClient *client;

    QAction *breakAction;
    QAction *continueAction;
    QAction *stepIntoAction;
    QAction *stepOverAction;
    QAction *stepOutAction;
    QAction *runToCursorAction;

    QScriptDebuggerStackWidget *stackWidget;
    QScriptDebuggerCodeWidget *codeWidget;
    QScriptDebuggerScriptsWidget *scriptsWidget;
    QScriptDebuggerLocalsWidget *localsWidget;
    QScriptDebuggerBreakpointsWidget *breakpointsWidget;
    QScriptDebuggerConsoleWidget *consoleWidget;

    QScriptDebugger *q_ptr; // ###
};



class DebuggerClient : public QScriptDebuggerClient
{
public:
    DebuggerClient(QScriptDebuggerFrontend *frontend,
                   QScriptDebuggerPrivate *debugger);
    ~DebuggerClient();

    void commandFinished(int id, const QScriptDebuggerResponse &response);
    void event(const QScriptDebuggerEvent &event);

    QScriptDebuggerPrivate *debugger() const;
    QAbstractItemModel *scriptsModel() const;
    qint64 scriptIdFromIndex(const QModelIndex &index) const;
    ScriptItem *scriptItemFromId(qint64 id) const;
    QAbstractItemModel *stackModel() const;
    QAbstractItemModel *localsModel() const;
    QAbstractItemModel *breakpointsModel() const;

    void addToScriptsModel(const QVariantList &info);
    void removeFromScriptsModel(const QList<qint64> &ids);
    void updateStackModel(const QList<QScriptContextInfo> &infos);
    void addToLocalsModel(const QScriptDebuggerPropertyList &props);
    void populateLocal(LocalNode *node, const QScriptDebuggerPropertyList &props);
    void addToBreakpointModel(int bpId, qint64 scriptId, int lineNumber);
    void removeFromBreakpointModel(int bpId);

    void syncScripts();
    void syncStack();
    void syncAll();
    void loadScript(qint64 scriptId, int executionLineNumber);
    void selectStackFrame(int index);
    void setBreakpoint(qint64 scriptId, int lineNumber);
    void deleteBreakpoint(qint64 scriptId, int lineNumber);

    void scheduleJob(DebuggerClientJob *job);

private:
    QScriptDebuggerPrivate *m_debugger;
    ScriptsModel *m_scriptsModel;
    StackModel *m_stackModel;
    LocalsModel *m_localsModel;
    BreakpointsModel *m_breakpointsModel;
    QList<DebuggerClientJob*> m_jobs;
};



QScriptDebuggerPrivate::QScriptDebuggerPrivate(QScriptDebugger *q)
{
    q_ptr = q;
    client = 0;
    initWidgets();
    initActions();
}

QScriptDebuggerPrivate::~QScriptDebuggerPrivate()
{
    delete client;
}

void QScriptDebuggerPrivate::prepareExecute()
{
    if (codeWidget)
        codeWidget->invalidateExecutionLineNumbers();
}

void QScriptDebuggerPrivate::emitExecutionHalted()
{
    Q_Q(QScriptDebugger);
    emit q->executionHalted();
}

void QScriptDebuggerPrivate::initWidgets()
{
    // widgets are created lazily
    stackWidget = 0;
    codeWidget = 0;
    scriptsWidget = 0;
    localsWidget = 0;
    breakpointsWidget = 0;
    consoleWidget = 0;
}

void QScriptDebuggerPrivate::initActions()
{
    Q_Q(QScriptDebugger);
    QIcon continueIcon;
    continueIcon.addPixmap(QPixmap(":images/play.png"), QIcon::Normal);
    continueIcon.addPixmap(QPixmap(":images/d_play.png"), QIcon::Disabled);
    continueAction = new QAction(continueIcon, QObject::tr("Continue"), q);
    continueAction->setShortcut(QObject::tr("F5"));
    QObject::connect(continueAction, SIGNAL(triggered()), q, SLOT(_q_continue()));

    QIcon breakIcon;
    breakIcon.addPixmap(QPixmap(":images/pause.png"), QIcon::Normal);
    breakIcon.addPixmap(QPixmap(":images/d_pause.png"), QIcon::Disabled);
    breakAction = new QAction(breakIcon, QObject::tr("Break"), q);
    breakAction->setShortcut(QObject::tr("Shift+F5"));
    QObject::connect(breakAction, SIGNAL(triggered()), q, SLOT(_q_break()));

    QIcon stepIntoIcon;
    stepIntoIcon.addPixmap(QPixmap(":images/stepinto.png"), QIcon::Normal);
    stepIntoIcon.addPixmap(QPixmap(":images/d_stepinto.png"), QIcon::Disabled);
    stepIntoAction = new QAction(stepIntoIcon, QObject::tr("Step Into"), q);
    stepIntoAction->setShortcut(QObject::tr("F11"));
    QObject::connect(stepIntoAction, SIGNAL(triggered()), q, SLOT(_q_stepInto()));

    QIcon stepOverIcon;
    stepOverIcon.addPixmap(QPixmap(":images/stepover.png"), QIcon::Normal);
    stepOverIcon.addPixmap(QPixmap(":images/d_stepover.png"), QIcon::Disabled);
    stepOverAction = new QAction(stepOverIcon, QObject::tr("Step Over"), q);
    stepOverAction->setShortcut(QObject::tr("F10"));
    QObject::connect(stepOverAction, SIGNAL(triggered()), q, SLOT(_q_stepOver()));

    QIcon stepOutIcon;
    stepOutIcon.addPixmap(QPixmap(":images/stepout.png"), QIcon::Normal);
    stepOutIcon.addPixmap(QPixmap(":images/d_stepout.png"), QIcon::Disabled);
    stepOutAction = new QAction(stepOutIcon, QObject::tr("Step Out"), q);
    stepOutAction->setShortcut(QObject::tr("Shift+F11"));
    QObject::connect(stepOutAction, SIGNAL(triggered()), q, SLOT(_q_stepOut()));

    QIcon runToCursorIcon;
    runToCursorIcon.addPixmap(QPixmap(":images/runtocursor.png"), QIcon::Normal);
    runToCursorIcon.addPixmap(QPixmap(":images/d_runtocursor.png"), QIcon::Disabled);
    runToCursorAction = new QAction(runToCursorIcon, QObject::tr("Run to Cursor"), q);
    runToCursorAction->setShortcut(QObject::tr("Ctrl+F10"));
    QObject::connect(runToCursorAction, SIGNAL(triggered()), q, SLOT(_q_runToCursor()));
}

void QScriptDebuggerPrivate::enableOrDisableActions(bool running)
{
    continueAction->setEnabled(!running);
    breakAction->setEnabled(running);
    stepIntoAction->setEnabled(!running);
    stepOverAction->setEnabled(!running);
    stepOutAction->setEnabled(!running);
    runToCursorAction->setEnabled(!running);
}

void QScriptDebuggerPrivate::maybeShowScript(qint64 scriptId, int executionLineNumber)
{
    if (scriptId == -1)
        return;
    if (!codeWidget || !codeWidget->isVisible()) {
        // ### emit a signal so the code widget can be shown on demand?
        return;
    }
    if (codeWidget->setCurrentScript(scriptId))
        codeWidget->setExecutionLineNumber(scriptId, executionLineNumber);
    else
        client->loadScript(scriptId, executionLineNumber);
}

//
// The job classes
//

DebuggerClient *DebuggerClientJob::client() const
{
    return m_client;
}

QScriptDebuggerFrontend *DebuggerClientJob::frontend() const
{
    return m_client->frontend();
}

QScriptDebuggerPrivate *DebuggerClientJob::debugger() const
{
    return m_client->debugger();
}

class SyncScriptsJob : public DebuggerClientJob
{
public:
    SyncScriptsJob(DebuggerClient *client)
        : DebuggerClientJob(client) {}

    void start()
    {
        frontend()->scheduleScriptsCheckpointAndDelta(/*includeContents=*/false);
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        QVariantList delta = response.result().toList();
        QVariantList added = delta.at(0).toList();
        QList<qint64> removed = qvariant_cast<QList<qint64> >(delta.at(1));
        client()->removeFromScriptsModel(removed);
        client()->addToScriptsModel(added);
        return Done;
    }
};

class SyncStackJob : public DebuggerClientJob
{
public:
    SyncStackJob(DebuggerClient *client)
        : DebuggerClientJob(client) {}

    void start()
    {
        frontend()->scheduleGetContextInfos();
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        QList<QScriptContextInfo> infos = qvariant_cast<QList<QScriptContextInfo> >(response.result());
        Q_ASSERT(!infos.isEmpty());
        client()->updateStackModel(infos);

        client()->selectStackFrame(0);

        return Done;
    }
};

class LoadScriptJob : public DebuggerClientJob
{
public:
    LoadScriptJob(qint64 scriptId, int executionLineNumber, DebuggerClient *client)
        : DebuggerClientJob(client), m_scriptId(scriptId),
          m_executionLineNumber(executionLineNumber) {}

    void start()
    {
        frontend()->scheduleGetScriptContents(m_scriptId);
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        ScriptItem *item = client()->scriptItemFromId(m_scriptId);
        Q_ASSERT(item);
        QString contents = response.result().toString();
        debugger()->codeWidget->addScript(m_scriptId, item->fileName(),
                                          item->lineNumber(), contents);
        debugger()->codeWidget->setCurrentScript(m_scriptId);
        debugger()->codeWidget->setExecutionLineNumber(m_scriptId, m_executionLineNumber);
        return Done;
    }

private:
    qint64 m_scriptId;
    int m_executionLineNumber;
};

class LoadLocalsJob : public DebuggerClientJob
{
public:
    enum State {
        NoState,
        GetActivationState,
        NewActivationIteratorState,
        ActivationIteratorNextPropertiesState,
        DeleteActivationIteratorState,
        FinishedState
    };

    LoadLocalsJob(int frameIndex, DebuggerClient *client)
        : DebuggerClientJob(client), m_frameIndex(frameIndex), m_state(NoState) {}

    void start()
    {
        m_state = GetActivationState;
        frontend()->scheduleGetScopeChain(m_frameIndex);
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        static const int PropertyBatchSize = 1000;
        switch (m_state) {
        case GetActivationState: {
            QScriptDebuggerValueList scopeChain = qvariant_cast<QScriptDebuggerValueList>(response.result());
            QScriptDebuggerValue activation = scopeChain.value(0);
            m_state = NewActivationIteratorState;
            frontend()->scheduleNewIterator(activation);
        }   break;
        case NewActivationIteratorState: {
            m_activationIteratorId = response.result().toInt();
            m_state = ActivationIteratorNextPropertiesState;
            frontend()->scheduleIteratorNextProperties(m_activationIteratorId, PropertyBatchSize);
        }   break;
        case ActivationIteratorNextPropertiesState: {
            QScriptDebuggerPropertyList props = qvariant_cast<QScriptDebuggerPropertyList>(response.result());
            client()->addToLocalsModel(props);
            m_state = DeleteActivationIteratorState;
            frontend()->scheduleDeleteIterator(m_activationIteratorId);
        }   break;
        case DeleteActivationIteratorState: {
            // ### load the this-object too
            m_state = FinishedState;
        }   break;
        default:
            ;
        }
        return (m_state == FinishedState) ? Done : NotDone;
    }

private:
    int m_frameIndex;
    int m_activationIteratorId;
    State m_state;
};

class PopulateLocalJob : public DebuggerClientJob
{
public:
    enum State {
        NoState,
        NewIteratorState,
        IteratorNextPropertiesState,
        DeleteIteratorState,
        FinishedState
    };

    PopulateLocalJob(LocalNode *node, DebuggerClient *client)
        : DebuggerClientJob(client), m_node(node),
          m_iteratorId(-1), m_state(NoState)
        {}

    void start()
    {
        m_state = NewIteratorState;
        frontend()->scheduleNewIterator(m_node->m_prop.value());
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        static const int PropertyBatchSize = 1000;
        switch (m_state) {
        case NewIteratorState: {
            m_iteratorId = response.result().toInt();
            m_state = IteratorNextPropertiesState;
            frontend()->scheduleIteratorNextProperties(m_iteratorId, PropertyBatchSize);
        }   break;
        case IteratorNextPropertiesState: {
            QScriptDebuggerPropertyList props = qvariant_cast<QScriptDebuggerPropertyList>(response.result());
            client()->populateLocal(m_node, props);
            m_state = DeleteIteratorState;
            frontend()->scheduleDeleteIterator(m_iteratorId);
        }   break;
        case DeleteIteratorState: {
            m_state = FinishedState;
        }   break;
        default:
            ;
        }
        return (m_state == FinishedState) ? Done : NotDone;
    }

private:
    LocalNode *m_node;
    int m_iteratorId;
    State m_state;
};

class SetBreakpointJob : public DebuggerClientJob
{
public:
    SetBreakpointJob(qint64 scriptId, int lineNumber, DebuggerClient *client)
        : DebuggerClientJob(client), m_scriptId(scriptId), m_lineNumber(lineNumber)
        {}

    void start()
    {
        frontend()->scheduleSetBreakpoint(m_scriptId, m_lineNumber);
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &response)
    {
        int id = response.result().toInt();
        client()->addToBreakpointModel(id, m_scriptId, m_lineNumber);
        return Done;
    }

private:
    qint64 m_scriptId;
    int m_lineNumber;
};

class DeleteBreakpointJob : public DebuggerClientJob
{
public:
    DeleteBreakpointJob(int id, DebuggerClient *client)
        : DebuggerClientJob(client), m_id(id)
        {}

    void start()
    {
        frontend()->scheduleDeleteBreakpoint(m_id);
    }

    Status handleResponse(int /*commandId*/, const QScriptDebuggerResponse &/*response*/)
    {
        client()->removeFromBreakpointModel(m_id);
        return Done;
    }

private:
    int m_id;
};

//
// The client
//

DebuggerClient::DebuggerClient(QScriptDebuggerFrontend *frontend,
                               QScriptDebuggerPrivate *debugger)
    : QScriptDebuggerClient(frontend), m_debugger(debugger)
{
    m_scriptsModel = new ScriptsModel;
    m_stackModel = new StackModel;
    m_localsModel = new LocalsModel(this);
    m_breakpointsModel = new BreakpointsModel(m_scriptsModel);
}

DebuggerClient::~DebuggerClient()
{
    delete m_scriptsModel;
    delete m_stackModel;
    delete m_localsModel;
    delete m_breakpointsModel;
}

QScriptDebuggerPrivate *DebuggerClient::debugger() const
{
    return m_debugger;
}

QAbstractItemModel *DebuggerClient::scriptsModel() const
{
    return m_scriptsModel;
}

QAbstractItemModel *DebuggerClient::stackModel() const
{
    return m_stackModel;
}

QAbstractItemModel *DebuggerClient::localsModel() const
{
    return m_localsModel;
}

QAbstractItemModel *DebuggerClient::breakpointsModel() const
{
    return m_breakpointsModel;
}

ScriptItem *DebuggerClient::scriptItemFromId(qint64 id) const
{
    return m_scriptsModel->itemFromId(id);
}

qint64 DebuggerClient::scriptIdFromIndex(const QModelIndex &index) const
{
    ScriptItem *item = (ScriptItem*)m_scriptsModel->itemFromIndex(index);
    if (!item)
        return -1;
    return item->id();
}

void DebuggerClient::addToScriptsModel(const QVariantList &info)
{
    for (int i = 0; i < info.size(); ++i) {
        QVariantList item = info.at(i).toList();
        qint64 scriptId = item.at(0).toLongLong();
        QString fileName = item.at(1).toString();
        int lineNumber = item.at(2).toInt();
        ScriptItem *modelItem = new ScriptItem(scriptId, fileName, lineNumber);
        m_scriptsModel->appendRow(modelItem);
    }
}

void DebuggerClient::removeFromScriptsModel(const QList<qint64> &ids)
{
    for (int i = 0; i < ids.size(); ++i) {
        qint64 scriptId = ids.at(i);
        for (int j = 0; j < m_scriptsModel->rowCount(); ++j) {
            ScriptItem *item = (ScriptItem*)m_scriptsModel->item(j);
            if (item->id() == scriptId) {
                m_scriptsModel->removeRow(j);
                break;
            }
        }
    }
}

void DebuggerClient::updateStackModel(const QList<QScriptContextInfo> &infos)
{
    m_stackModel->update(infos);
}

void DebuggerClient::addToLocalsModel(const QScriptDebuggerPropertyList &props)
{
    m_localsModel->clear(); // ### fixme
    m_localsModel->add(props);
}

void DebuggerClient::addToBreakpointModel(int bpId, qint64 scriptId, int lineNumber)
{
    m_breakpointsModel->addBreakpoint(bpId, scriptId, lineNumber);
}

void DebuggerClient::removeFromBreakpointModel(int bpId)
{
    m_breakpointsModel->removeBreakpoint(bpId);
}

void DebuggerClient::populateLocal(LocalNode *node, const QScriptDebuggerPropertyList &props)
{
    m_localsModel->populateNode(node, props);
}

void DebuggerClient::commandFinished(int id, const QScriptDebuggerResponse &response)
{
//    qDebug() << "commandFinished(" << id << ")";
    if (!m_jobs.isEmpty()) {
        DebuggerClientJob *job = m_jobs[0];
        if (job->handleResponse(id, response) == DebuggerClientJob::Done) {
            m_jobs.removeFirst();
            delete job;
            if (!m_jobs.isEmpty())
                m_jobs[0]->start();
        }
    }
}

void DebuggerClient::event(const QScriptDebuggerEvent &event)
{
//    qDebug() << "event(" << event.type() << ")";
    if (m_debugger->consoleWidget) {
        QString timeStr = QTime::currentTime().toString("[hh:mm:ss]");
        QString posStr;
        if (event.lineNumber() == -1)
            posStr = "???";
        else {
            posStr = QString::fromLatin1("%0:%1")
                     .arg(event.fileName()).arg(event.lineNumber());
        }
        switch (event.type()) {
        case QScriptDebuggerEvent::None:
            break;
        case QScriptDebuggerEvent::Break:
            m_debugger->consoleWidget->log(QString::fromLatin1("%0 Evaluation paused at %1")
                                           .arg(timeStr).arg(posStr));
            break;
        case QScriptDebuggerEvent::SteppingFinished:
            m_debugger->consoleWidget->log(QString::fromLatin1("%0 Stepped to %1")
                                           .arg(timeStr).arg(posStr));
            break;
        case QScriptDebuggerEvent::LocationReached:
            m_debugger->consoleWidget->log(QString::fromLatin1("%0 Ran to %1")
                                           .arg(timeStr).arg(posStr));
            break;
        case QScriptDebuggerEvent::Breakpoint:
            m_debugger->consoleWidget->log(QString::fromLatin1("%0 Breakpoint hit at %1")
                                           .arg(timeStr).arg(posStr));
            break;
        case QScriptDebuggerEvent::Exception: {
            if (event.attribute(QScriptDebuggerEvent::HasExceptionHandler).toBool()) {
                // ignore exceptions that are caught
                frontend()->scheduleResume();
                return;
            }
            QString exceptStr = event.attribute(QScriptDebuggerEvent::ExceptionString).toString();
            m_debugger->consoleWidget->log(QString::fromLatin1("<font color=red>%0 Uncaught exception at %1: %2</font>")
                                           .arg(timeStr).arg(posStr).arg(exceptStr));
        }   break;
        case QScriptDebuggerEvent::Trace: {
            QString text = event.attribute(QScriptDebuggerEvent::Text).toString();
            m_debugger->consoleWidget->log(text);
        }   return; // NB: trace events don't stall execution
        default:
            Q_ASSERT_X(0, "QScriptDebugger", "unknown event");
        }
    }
    m_debugger->emitExecutionHalted();
    syncAll();
}

void DebuggerClient::syncScripts()
{
    scheduleJob(new SyncScriptsJob(this));
}

void DebuggerClient::syncStack()
{
    scheduleJob(new SyncStackJob(this));
}

void DebuggerClient::syncAll()
{
    syncScripts();
    syncStack();
}

void DebuggerClient::loadScript(qint64 scriptId, int executionLineNumber)
{
    scheduleJob(new LoadScriptJob(scriptId, executionLineNumber, this));
}

void DebuggerClient::selectStackFrame(int index)
{
    if (index == -1)
        return;

    QScriptContextInfo info = m_stackModel->info(index);
    Q_ASSERT(!info.isNull());
    qint64 id = info.scriptId();
    int lineNumber = info.lineNumber();

    m_debugger->maybeShowScript(id, lineNumber);

    if (m_debugger->stackWidget)
        m_debugger->stackWidget->setFrameIndex(index);

    if (m_debugger->localsWidget) {
        scheduleJob(new LoadLocalsJob(index, this));
    }
}

void DebuggerClient::setBreakpoint(qint64 scriptId, int lineNumber)
{
    scheduleJob(new SetBreakpointJob(scriptId, lineNumber, this));
}

void DebuggerClient::deleteBreakpoint(qint64 scriptId, int lineNumber)
{
    int id = m_breakpointsModel->findBreakpoint(scriptId, lineNumber);
    if (id != -1)
        scheduleJob(new DeleteBreakpointJob(id, this));
}

void DebuggerClient::scheduleJob(DebuggerClientJob *job)
{
    m_jobs.append(job);
    if (m_jobs.size() == 1)
        job->start();
}

//
//
//

void QScriptDebugger::_q_onScriptSelected(const QModelIndex &index)
{
    Q_D(QScriptDebugger);
    qint64 scriptId = d->client->scriptIdFromIndex(index);
    d->maybeShowScript(scriptId, /*executionLineNumber=*/-1);
}

void QScriptDebugger::_q_onStackFrameSelected(int index)
{
    Q_D(QScriptDebugger);
    d->client->selectStackFrame(index);
}

void QScriptDebugger::_q_onCommandEntered(const QString &command)
{
//    Q_D(QScriptDebugger);
    qDebug() << "IMPLEMENT ME:" << command;
}

void QScriptDebugger::_q_onBreakpointToggled(qint64 scriptId, int lineNumber, bool set)
{
    Q_D(QScriptDebugger);
    if (set)
        d->client->setBreakpoint(scriptId, lineNumber);
    else
        d->client->deleteBreakpoint(scriptId, lineNumber);
}

void QScriptDebugger::_q_break()
{
    Q_D(QScriptDebugger);
    d->prepareExecute();
    d->client->frontend()->scheduleBreak();
}

void QScriptDebugger::_q_continue()
{
    Q_D(QScriptDebugger);
    d->prepareExecute();
    d->client->frontend()->scheduleContinue();
}

void QScriptDebugger::_q_stepInto()
{
    Q_D(QScriptDebugger);
    d->prepareExecute();
    d->client->frontend()->scheduleStepInto();
}

void QScriptDebugger::_q_stepOver()
{
    Q_D(QScriptDebugger);
    d->prepareExecute();
    d->client->frontend()->scheduleStepOver();
}

void QScriptDebugger::_q_stepOut()
{
    Q_D(QScriptDebugger);
    d->prepareExecute();
    d->client->frontend()->scheduleStepOut();
}

void QScriptDebugger::_q_runToCursor()
{
    Q_D(QScriptDebugger);
    if (!d->codeWidget)
        return;
    int lineNumber = d->codeWidget->cursorLineNumber();
    if (lineNumber == -1)
        return;
    qint64 scriptId = d->codeWidget->currentScript();
    d->prepareExecute();
    d->client->frontend()->scheduleRunToLocation(scriptId, lineNumber);
}

//
//
//

/*!
  Constructs a new QScriptDebugger object with the given \a
  parent.
*/
QScriptDebugger::QScriptDebugger(QObject *parent)
//    : QObject(*new QScriptDebuggerPrivate, parent)
    : QObject(parent),
        d_ptr(new QScriptDebuggerPrivate(this))
{
}

/*!
  Destroys this QScriptDebugger.
*/
QScriptDebugger::~QScriptDebugger()
{
    delete d_ptr; // ###
}

/*!
  Returns the front-end that this debugger uses.
*/
QScriptDebuggerFrontend *QScriptDebugger::frontend() const
{
    Q_D(const QScriptDebugger);
    if (!d->client)
        return 0;
    return d->client->frontend();
}

/*!
  Sets the front-end that this debugger should use.
*/
void QScriptDebugger::setFrontend(QScriptDebuggerFrontend *frontend)
{
    Q_D(QScriptDebugger);
    // ### possible to keep the same client throughout?
    if (d->client) {
        delete d->client;
        d->client = 0;
    }
    if (frontend) {
        d->client = new DebuggerClient(frontend, d);
        frontend->setClient(d->client);
        if (d->codeWidget)
            d->codeWidget->clear();
        if (d->stackWidget)
            d->stackWidget->setModel(d->client->stackModel());
        if (d->localsWidget)
            d->localsWidget->setModel(d->client->localsModel());
        if (d->breakpointsWidget)
            d->breakpointsWidget->setModel(d->client->breakpointsModel());
        if (d->scriptsWidget)
            d->scriptsWidget->setModel(d->client->scriptsModel());
        d->continueAction->setEnabled(true);
        d->breakAction->setEnabled(true);
        d->stepIntoAction->setEnabled(true);
        d->stepOverAction->setEnabled(true);
        d->stepOutAction->setEnabled(true);
        d->runToCursorAction->setEnabled(true);
    } else {
        if (d->codeWidget)
            d->codeWidget->clear();
        if (d->stackWidget)
            d->stackWidget->setModel(0);
        if (d->localsWidget)
            d->localsWidget->setModel(0);
        if (d->breakpointsWidget)
            d->breakpointsWidget->setModel(0);
        if (d->scriptsWidget)
            d->scriptsWidget->setModel(0);
        d->continueAction->setEnabled(false);
        d->breakAction->setEnabled(false);
        d->stepIntoAction->setEnabled(false);
        d->stepOverAction->setEnabled(false);
        d->stepOutAction->setEnabled(false);
        d->runToCursorAction->setEnabled(false);
    }
}

/*!
  Returns the "Break" action.
*/
QAction *QScriptDebugger::breakAction() const
{
    Q_D(const QScriptDebugger);
    return d->breakAction;
}

/*!
  Returns the "Continue" action.
*/
QAction *QScriptDebugger::continueAction() const
{
    Q_D(const QScriptDebugger);
    return d->continueAction;
}

/*!
  Returns the "Step Into" action.
*/
QAction *QScriptDebugger::stepIntoAction() const
{
    Q_D(const QScriptDebugger);
    return d->stepIntoAction;
}

/*!
  Returns the "Step Over" action.
*/
QAction *QScriptDebugger::stepOverAction() const
{
    Q_D(const QScriptDebugger);
    return d->stepOverAction;
}

/*!
  Returns the "Step Out" action.
*/
QAction *QScriptDebugger::stepOutAction() const
{
    Q_D(const QScriptDebugger);
    return d->stepOutAction;
}

/*!
  Returns the "Run To Cursor" action.
*/
QAction *QScriptDebugger::runToCursorAction() const
{
    Q_D(const QScriptDebugger);
    return d->runToCursorAction;
}

/*!
  Returns the stack widget.
*/
QWidget *QScriptDebugger::stackWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->stackWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->stackWidget = new QScriptDebuggerStackWidget();
        QObject::connect(dd->stackWidget, SIGNAL(frameSelected(int)),
                         this, SLOT(_q_onStackFrameSelected(int)));
        if (d->client) {
            dd->stackWidget->setModel(d->client->stackModel());
            // ### refresh it?
        }
    }
    return d->stackWidget;
}

/*!
  Returns the code widget.
*/
QWidget *QScriptDebugger::codeWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->codeWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->codeWidget = new QScriptDebuggerCodeWidget();
        QObject::connect(dd->codeWidget, SIGNAL(breakpointToggled(qint64,int,bool)),
                         this, SLOT(_q_onBreakpointToggled(qint64,int,bool)));
    }
    return d->codeWidget;
}

/*!
  Returns the scripts widget.
*/
QWidget *QScriptDebugger::scriptsWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->scriptsWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->scriptsWidget = new QScriptDebuggerScriptsWidget();
        QObject::connect(dd->scriptsWidget, SIGNAL(selected(QModelIndex)),
                         this, SLOT(_q_onScriptSelected(QModelIndex)));
        if (d->client) {
            dd->scriptsWidget->setModel(d->client->scriptsModel());
// ###            d->client->listScripts();
        }
    }
    return d->scriptsWidget;
}

/*!
  Returns the locals widget.
*/
QWidget *QScriptDebugger::localsWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->localsWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->localsWidget = new QScriptDebuggerLocalsWidget();
        if (d->client) {
            dd->localsWidget->setModel(d->client->localsModel());
        }
    }
    return d->localsWidget;
}

/*!
  Returns the breakpoints widget.
*/
QWidget *QScriptDebugger::breakpointsWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->breakpointsWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->breakpointsWidget = new QScriptDebuggerBreakpointsWidget();
        if (d->client) {
            dd->breakpointsWidget->setModel(d->client->breakpointsModel());
        }
    }
    return d->breakpointsWidget;
}

/*!
  Returns the console widget.
*/
QWidget *QScriptDebugger::consoleWidget() const
{
    Q_D(const QScriptDebugger);
    if (!d->consoleWidget) {
        QScriptDebuggerPrivate *dd = const_cast<QScriptDebuggerPrivate*>(d);
        dd->consoleWidget = new QScriptDebuggerConsoleWidget();
        QObject::connect(dd->consoleWidget, SIGNAL(commandEntered(QString)),
                         this, SLOT(_q_onCommandEntered(QString)));
    }
    return d->consoleWidget;
}

/*!
  Initializes the window \a win with a default setup of the
  debugger's components.
*/
void QScriptDebugger::initMainWindow(QMainWindow *win) const
{
    QDockWidget *scriptsDock = new QDockWidget(win);
    scriptsDock->setWindowTitle(QObject::tr("Loaded scripts"));
    scriptsDock->setWidget(scriptsWidget());
    win->addDockWidget(Qt::LeftDockWidgetArea, scriptsDock);

    QDockWidget *stackDock = new QDockWidget(win);
    stackDock->setWindowTitle(QObject::tr("Stack"));
    stackDock->setWidget(stackWidget());
    win->addDockWidget(Qt::RightDockWidgetArea, stackDock);

    QDockWidget *localsDock = new QDockWidget(win);
    localsDock->setWindowTitle(QObject::tr("Locals"));
    localsDock->setWidget(localsWidget());
    win->addDockWidget(Qt::RightDockWidgetArea, localsDock);

    QDockWidget *consoleDock = new QDockWidget(win);
    consoleDock->setWindowTitle(QObject::tr("Console"));
    consoleDock->setWidget(consoleWidget());
    win->addDockWidget(Qt::BottomDockWidgetArea, consoleDock);

    QDockWidget *breakpointsDock = new QDockWidget(win);
    breakpointsDock->setWindowTitle(QObject::tr("Breakpoints"));
    breakpointsDock->setWidget(breakpointsWidget());
    win->addDockWidget(Qt::LeftDockWidgetArea, breakpointsDock);

    QToolBar *tb = new QToolBar(win);
    tb->addAction(continueAction());
    tb->addAction(breakAction());
    tb->addAction(stepIntoAction());
    tb->addAction(stepOverAction());
    tb->addAction(stepOutAction());
    tb->addAction(runToCursorAction());
    win->addToolBar(Qt::TopToolBarArea, tb);

    win->setCentralWidget(codeWidget());
}

//
// The model implementations
//

ScriptItem *ScriptsModel::itemFromId(qint64 id) const
{
    for (int i = 0; i < rowCount(); ++i) {
        ScriptItem *it = (ScriptItem*)item(i);
        if (it->id() == id)
            return it;
    }
    return 0;
}

LocalsModel::LocalsModel(DebuggerClient *client, QObject *parent)
    : QAbstractItemModel(parent), m_client(client), m_activationNode(0)
{
}

LocalNode *LocalsModel::nodeFromIndex(const QModelIndex &index) const
{
    if (!index.isValid())
        return m_activationNode;
    return static_cast<LocalNode*>(index.internalPointer());
}

QModelIndex LocalsModel::indexFromNode(LocalNode *node) const
{
    if (!node || (node == m_activationNode))
        return QModelIndex();
    LocalNode *par = node->m_parent;
    int row = par ? par->m_children.indexOf(node) : 0;
    return createIndex(row, 0, node);
}

int LocalsModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

int LocalsModel::rowCount(const QModelIndex &parent) const
{
    LocalNode *node = nodeFromIndex(parent);
    return node ? node->m_children.count() : 0;
}

QVariant LocalsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    LocalNode *node = nodeFromIndex(index);
    if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return node->m_prop.name();
        else if (index.column() == 1) {
            QScriptDebuggerValue value = node->m_prop.value();
            if (value.type() != QScriptDebuggerValue::ObjectValue)
                return node->m_prop.value().toString();
            return QString();
        }
    }
    return QVariant();
}

QVariant LocalsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            if (section == 0)
                return QObject::tr("Name");
            else if (section == 1)
                return QObject::tr("Value");
        }
    }
    return QVariant();
}

QModelIndex LocalsModel::index(int row, int column, const QModelIndex &parent) const
{
    LocalNode *node = nodeFromIndex(parent);
    if ((node == 0) || (row >= node->m_children.count()))
        return QModelIndex();
    return createIndex(row, column, node->m_children.at(row));
}

QModelIndex LocalsModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    LocalNode *node = nodeFromIndex(index);
    return indexFromNode(node->m_parent);
}

bool LocalsModel::hasChildren(const QModelIndex &parent) const
{
    LocalNode *node = nodeFromIndex(parent);
    if (!node)
        return false;
    return !node->m_children.isEmpty()
        || ((node->m_prop.value().type() == QScriptDebuggerValue::ObjectValue)
            && !node->m_populated);
}

bool LocalsModel::canFetchMore(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return false;
    LocalNode *node = nodeFromIndex(parent);
    return node
        && (node->m_prop.value().type() == QScriptDebuggerValue::ObjectValue)
        && !node->m_populated;
}

void LocalsModel::fetchMore(const QModelIndex &parent)
{
    LocalNode *node = nodeFromIndex(parent);
    m_client->scheduleJob(new PopulateLocalJob(node, m_client));
    node->m_populated = true;
}

void LocalsModel::clear()
{
    if (m_activationNode) {
//        emit layoutAboutToBeChanged();
        delete m_activationNode;
        m_activationNode = 0;
//        emit layoutChanged();
        emit reset();
    }
}

void LocalsModel::add(const QScriptDebuggerPropertyList &props)
{
    if (!m_activationNode)
        m_activationNode = new LocalNode(QScriptDebuggerProperty(), 0);
    LocalNode *node = m_activationNode;
    for (int i = 0; i < props.count(); ++i) {
        LocalNode *child = new LocalNode(props.at(i), node);
        node->m_children.append(child);
    }
    emit layoutAboutToBeChanged();
    emit layoutChanged();
}

void LocalsModel::populateNode(LocalNode *node, const QScriptDebuggerPropertyList &props)
{
    if (!props.isEmpty()) {
        QModelIndex index = indexFromNode(node);
        beginInsertRows(index, 0, props.count() - 1);
        for (int i = 0; i < props.count(); ++i) {
            LocalNode *child = new LocalNode(props.at(i), node);
            node->m_children.append(child);
        }
        node->m_populated = true;
        endInsertRows();
    } else {
        node->m_populated = true;
    }
}



int StackModel::columnCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return 2;
    return 0;
}

int StackModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_infos.count();
    return 0;
}

QVariant StackModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_infos.count())
        return QVariant();
    const QScriptContextInfo &info = m_infos.at(index.row());
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            return info.functionName();
        } else if (index.column() == 1) {
            if (info.lineNumber() == -1)
                return QString::fromLatin1("<unknown>");
            QString fn = QFileInfo(info.fileName()).fileName();
            return QString::fromLatin1("%0:%1").arg(fn).arg(info.lineNumber());
        }
    } else if (role == Qt::ToolTipRole) {
        if (QFileInfo(info.fileName()).fileName() != info.fileName())
            return info.fileName();
    }
    return QVariant();
}

QVariant StackModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (section == 0)
            return QObject::tr("Name");
        else if (section == 1)
            return QObject::tr("Location");
    }
    return QVariant();
}

void StackModel::update(const QList<QScriptContextInfo> &infos)
{
    emit layoutAboutToBeChanged();
    m_infos = infos;
    emit layoutChanged();
}



int BreakpointsModel::columnCount(const QModelIndex &/*parent*/) const
{
    return 2;
}

int BreakpointsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid())
        return m_ids.count();
    return 0;
}

QVariant BreakpointsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= m_ids.count())
        return QVariant();
    ScriptItem *item = m_scripts->itemFromId(m_scriptIds.at(index.row()));
    if (role == Qt::DisplayRole) {
        if (index.column() == 0) {
            if (item)
                return QFileInfo(item->fileName()).fileName();
            return QObject::tr("<ghost script>");
        } else if (index.column() == 1)
            return m_lineNumbers.at(index.row());
    } else if (role == Qt::ToolTipRole) {
        if (item && QFileInfo(item->fileName()).fileName() != item->fileName())
            return item->fileName();
    }
    return QVariant();
}

QVariant BreakpointsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation != Qt::Horizontal)
        return QVariant();
    if (role == Qt::DisplayRole) {
        if (section == 0)
            return QObject::tr("File");
        else if (section == 1)
            return QObject::tr("Line");
    }
    return QVariant();
}

int BreakpointsModel::findBreakpoint(qint64 scriptId, int lineNumber) const
{
    for (int i = 0; m_ids.count(); ++i) {
        if ((m_scriptIds.at(i) == scriptId) && (m_lineNumbers.at(i) == lineNumber))
            return m_ids.at(i);
    }
    return -1;
}

void BreakpointsModel::addBreakpoint(int bpId, qint64 scriptId, int lineNumber)
{
    m_ids.append(bpId);
    m_scriptIds.append(scriptId);
    m_lineNumbers.append(lineNumber);
    emit reset();
}

void BreakpointsModel::removeBreakpoint(int bpId)
{
    for (int i = 0; m_ids.count(); ++i) {
        if (m_ids.at(i) == bpId) {
            m_ids.removeAt(i);
            m_scriptIds.removeAt(i);
            m_lineNumbers.removeAt(i);
            emit reset();
            break;
        }
    }
}
