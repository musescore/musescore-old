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

#include "qscriptdebuggercodewidget_p.h"

//#include <private/qwidget_p.h>
#include <QtGui/qstackedwidget.h>
#include <QtGui/qboxlayout.h>
#include "editor/scriptedit.h"

QScriptDebuggerCodeWidget::QScriptDebuggerCodeWidget(QWidget *parent)
    : QWidget(parent)
{
    stack = new QStackedWidget();
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(stack);
}

QScriptDebuggerCodeWidget::~QScriptDebuggerCodeWidget()
{
}

void QScriptDebuggerCodeWidget::addScript(qint64 scriptId, const QString &fileName,
                                          int baseLineNumber, const QString &contents)
{
    ScriptEdit *edit = new ScriptEdit();
    QObject::connect(edit, SIGNAL(breakpointToggled(int,bool)),
                     this, SLOT(onBreakpointToggled(int,bool)));
    edit->setReadOnly(true);
    edit->setBackgroundVisible(false);
    edit->setPlainText(contents);
    edit->setProperty("_q_fileName", fileName); // ### hacky
    edit->setBaseLineNumber(baseLineNumber);
    editorHash.insert(scriptId, edit);
    stack->addWidget(edit);
}

void QScriptDebuggerCodeWidget::removeScript(qint64 scriptId)
{
    QWidget *edit = editorHash.take(scriptId);
    stack->removeWidget(edit);
}

qint64 QScriptDebuggerCodeWidget::currentScript() const
{
    return editorHash.key(currentEditor());
}

bool QScriptDebuggerCodeWidget::setCurrentScript(qint64 scriptId)
{
    ScriptEdit *edit = editorHash.value(scriptId);
    if (!edit)
        return false;
    stack->setCurrentWidget(edit);
    return true;
}

void QScriptDebuggerCodeWidget::setExecutionLineNumber(qint64 scriptId, int lineNumber)
{
    ScriptEdit *edit = editorHash.value(scriptId);
    if (edit)
        edit->setExecutionBlockNumber(lineNumber-1);
}

void QScriptDebuggerCodeWidget::invalidateExecutionLineNumbers()
{
    QHash<qint64, ScriptEdit*>::const_iterator it;
    for (it = editorHash.constBegin(); it != editorHash.constEnd(); ++it)
        it.value()->setExecutionBlockNumber(-1);
}

int QScriptDebuggerCodeWidget::cursorLineNumber() const
{
    ScriptEdit *edit = currentEditor();
    if (!edit)
        return -1;
    return edit->cursorLineNumber();
}

void QScriptDebuggerCodeWidget::clear()
{
    editorHash.clear();
    while (stack->count() != 0)
        delete stack->widget(0);
}

void QScriptDebuggerCodeWidget::onBreakpointToggled(int lineNumber, bool set)
{
    ScriptEdit *edit = qobject_cast<ScriptEdit*>(sender());
    qint64 scriptId = editorHash.key(edit);
    emit breakpointToggled(scriptId, lineNumber, set);
}

ScriptEdit *QScriptDebuggerCodeWidget::currentEditor() const
{
    return qobject_cast<ScriptEdit*>(stack->currentWidget());
}
