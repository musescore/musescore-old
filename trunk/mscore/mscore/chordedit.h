//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#ifndef __CHORDEDIT_H__
#define __CHORDEDIT_H__

// #include <QItemDelegate>
// #include <QModelIndex>
// #include <QSize>
// #include <QStandardItemModel>

#include "harmony.h"
#include "ui_chordedit.h"

class DegreeTabDelegate;

//---------------------------------------------------------
//   class ChordEdit
//---------------------------------------------------------

class ChordEdit : public QDialog, Ui::ChordEdit {
      Q_OBJECT

      QButtonGroup* rootGroup;
      QButtonGroup* extensionGroup;
      QStandardItemModel* model;
      DegreeTabDelegate* delegate;
      void updateDegrees();
      bool isValidDegree(int r);

   private slots:
      void otherToggled(bool);
      void chordChanged();
      void addButtonClicked();
      void deleteButtonClicked();
      void modelDataChanged(const QModelIndex & topLeft, const QModelIndex & bottomRight);

   public:
      ChordEdit(QWidget* parent = 0);
      void setExtension(int val);
      void setRoot(int val);
      void setBase(int val);
      void addDegree(HDegree d);
      int numberOfDegrees();
      HDegree degree(int i);
      int extension();
      int root();
      int base();
      };

//---------------------------------------------------------
//   class DegreeTabDelegate
//---------------------------------------------------------

class DegreeTabDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    DegreeTabDelegate(QObject *parent = 0);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const;

    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const;

    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const;
};

#endif

