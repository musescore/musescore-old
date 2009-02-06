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

#include "qscriptdebuggerconsolewidget_p.h"
#include <QtGui/qboxlayout.h>
#include <QtGui/qlineedit.h>
#include <QtGui/qplaintextedit.h>

QScriptDebuggerConsoleWidget::QScriptDebuggerConsoleWidget(QWidget *parent)
    : QWidget(parent), commandHistoryIndex(0)
{
/*    edit = new QLineEdit();
    QObject::connect(edit, SIGNAL(returnPressed()),
                     this, SLOT(onReturnPressed()));
    edit->installEventFilter(this);*/

    logger = new QPlainTextEdit();
    logger->setBackgroundVisible(false);
    logger->setReadOnly(true);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(logger);
//    vbox->addWidget(edit);
}

QScriptDebuggerConsoleWidget::~QScriptDebuggerConsoleWidget()
{
}

bool QScriptDebuggerConsoleWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == edit) {
        if (event->type() == QEvent::KeyPress) {
            QKeyEvent *ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Up) {
                if (commandHistoryIndex > 0) {
                    if (commandHistoryIndex == commandHistory.size())
                        incompleteCommand = edit->text();
                    --commandHistoryIndex;
                    QString text = commandHistory.at(commandHistoryIndex);
                    edit->setText(text);
                    edit->setCursorPosition(text.length());
                }
            } else if (ke->key() == Qt::Key_Down) {
                if (commandHistoryIndex < commandHistory.size()) {
                    ++commandHistoryIndex;
                    QString text;
                    if (commandHistoryIndex < commandHistory.size())
                        text = commandHistory.at(commandHistoryIndex);
                    else
                        text = incompleteCommand;
                    edit->setText(text);
                    edit->setCursorPosition(text.length());
                }
            }
        }
    }
    return false;
}

void QScriptDebuggerConsoleWidget::onReturnPressed()
{
    QString input = edit->text().trimmed();
    if (!input.isEmpty())
        commandHistory.append(input);
    else if (!commandHistory.isEmpty())
        input = commandHistory.last();
    else
        return;
    edit->clear();
    commandHistoryIndex = commandHistory.size();
    incompleteCommand = QString();
    emit commandEntered(input);
}

void QScriptDebuggerConsoleWidget::log(const QString &htmlMessage)
{
    logger->appendHtml(htmlMessage);
}

void QScriptDebuggerConsoleWidget::clear()
{
    logger->clear();
}
