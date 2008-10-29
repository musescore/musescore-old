//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __EDITDRUMSET_H__
#define __EDITDRUMSET_H__

#include "ui_editdrumset.h"
#include "drumset.h"

//---------------------------------------------------------
//   EditDrumset
//---------------------------------------------------------

class EditDrumset : public QDialog, private Ui::EditDrumsetBase {
      Q_OBJECT

      Drumset* oDrumset;
      Drumset  nDrumset;

      QPushButton* loadButton;
      QPushButton* saveButton;
      void apply();
      void load();
      void save();
      void updateList();
      void updateExample();

   private slots:
      void bboxClicked(QAbstractButton* button);
      void itemChanged(QTreeWidgetItem* current, QTreeWidgetItem* previous);
      void nameChanged(const QString&);
      void valueChanged();

   public:
      EditDrumset(Drumset* ds, QWidget* parent = 0);
      };


#endif

