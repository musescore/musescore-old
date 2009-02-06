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

#include "qscriptdebuggerstackwidget_p.h"
#include <QtGui/qboxlayout.h>
#include <QtGui/qheaderview.h>
#include <QtGui/qtreeview.h>

QScriptDebuggerStackWidget::QScriptDebuggerStackWidget(QWidget *parent)
    : QWidget(parent)
{
    view = new QTreeView();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAlternatingRowColors(true);
    view->setRootIsDecorated(false);
    view->header()->setDefaultAlignment(Qt::AlignLeft);
//    view->header()->setResizeMode(QHeaderView::ResizeToContents);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(view);
}

QScriptDebuggerStackWidget::~QScriptDebuggerStackWidget()
{
}

void QScriptDebuggerStackWidget::setModel(QAbstractItemModel *model)
{
    view->setModel(model);
    QObject::connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
                     this, SLOT(onCurrentChanged(QModelIndex)));
}

void QScriptDebuggerStackWidget::setFrameIndex(int index)
{
    view->setCurrentIndex(view->model()->index(index, 0));
}

int QScriptDebuggerStackWidget::frameIndex() const
{
    return view->currentIndex().row();
}

void QScriptDebuggerStackWidget::onCurrentChanged(const QModelIndex &index)
{
    emit frameSelected(index.row());
}

QAbstractItemModel *QScriptDebuggerStackWidget::model() const
{
    return view->model();
}
