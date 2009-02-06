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

#ifndef QSCRIPTDEBUGGERCODEWIDGET_H
#define QSCRIPTDEBUGGERCODEWIDGET_H

#include <QtGui/qwidget.h>

#include <QtCore/qhash.h>

class ScriptEdit;
class QStackedWidget;

class QScriptDebuggerCodeWidget : public QWidget
{
    Q_OBJECT
public:
    QScriptDebuggerCodeWidget(QWidget *parent = 0);
    ~QScriptDebuggerCodeWidget();

    void addScript(qint64 scriptId, const QString &fileName, int baseLineNumber, const QString &contents);
    void removeScript(qint64 scriptId);

    qint64 currentScript() const;
    bool setCurrentScript(qint64 scriptId);

    void setExecutionLineNumber(qint64 scriptId, int lineNumber);
    void invalidateExecutionLineNumbers();

    int cursorLineNumber() const;

    void clear();

signals:
    void breakpointToggled(qint64 scriptId, int lineNumber, bool set);

private slots:
    void onBreakpointToggled(int, bool);

private:
    ScriptEdit *currentEditor() const;

private:
    QStackedWidget *stack;
    QHash<qint64, ScriptEdit*> editorHash;
    Q_DISABLE_COPY(QScriptDebuggerCodeWidget)
};

#endif
