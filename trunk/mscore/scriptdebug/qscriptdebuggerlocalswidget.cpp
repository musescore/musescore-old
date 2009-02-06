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

#include "qscriptdebuggerlocalswidget_p.h"
#include <QtGui/qheaderview.h>
#include <QtGui/qtreeview.h>
#include <QtGui/qboxlayout.h>

QScriptDebuggerLocalsWidget::QScriptDebuggerLocalsWidget(QWidget *parent)
    : QWidget(parent)
{
    view = new QTreeView();
    view->setEditTriggers(QAbstractItemView::NoEditTriggers);
    view->setAlternatingRowColors(true);
    view->header()->setDefaultAlignment(Qt::AlignLeft);
    view->header()->setResizeMode(QHeaderView::ResizeToContents);

    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox->addWidget(view);
}

QScriptDebuggerLocalsWidget::~QScriptDebuggerLocalsWidget()
{
}

void QScriptDebuggerLocalsWidget::setModel(QAbstractItemModel *model)
{
    view->setModel(model);
}

QAbstractItemModel *QScriptDebuggerLocalsWidget::model() const
{
    return view->model();
}
