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

#ifndef QSCRIPTDEBUGGER_H
#define QSCRIPTDEBUGGER_H

#include "qscriptdebugglobal.h"

#include <QtCore/qobject.h>

class QModelIndex;
class QScriptDebuggerFrontend;
class QWidget;
class QAction;
class QMainWindow;

class QScriptDebuggerPrivate;
class Q_SCRIPTDEBUG_EXPORT QScriptDebugger : public QObject
{
    Q_OBJECT
public:
    QScriptDebugger(QObject *parent = 0);
    ~QScriptDebugger();

    QScriptDebuggerFrontend *frontend() const;
    void setFrontend(QScriptDebuggerFrontend *frontend);

    QAction *breakAction() const;
    QAction *continueAction() const;
    QAction *stepIntoAction() const;
    QAction *stepOverAction() const;
    QAction *stepOutAction() const;
    QAction *runToCursorAction() const;
    QAction *findAction() const;
    QAction *findNextAction() const;

    QWidget *stackWidget() const;
    QWidget *codeWidget() const;
    QWidget *scriptsWidget() const;
    QWidget *localsWidget() const;
    QWidget *breakpointsWidget() const;
    QWidget *consoleWidget() const;

    void initMainWindow(QMainWindow *win) const;

signals:
    void executionHalted();

private slots:
    void _q_onScriptSelected(const QModelIndex &);
    void _q_onStackFrameSelected(int index);
    void _q_onCommandEntered(const QString &);
    void _q_onBreakpointToggled(qint64, int, bool);
    void _q_continue();
    void _q_break();
    void _q_stepInto();
    void _q_stepOver();
    void _q_stepOut();
    void _q_runToCursor();

private:
    QScriptDebuggerPrivate *d_ptr; // ###

    Q_DECLARE_PRIVATE(QScriptDebugger)
    Q_DISABLE_COPY(QScriptDebugger)
};

#endif
