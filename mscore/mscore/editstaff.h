//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __EDITSTAFF_H__
#define __EDITSTAFF_H__

#include "ui_editstaff.h"
#include "ui_selectinstr.h"
#include "instrument.h"

class Staff;
class InstrumentTemplate;

//---------------------------------------------------------
//   EditStaff
//    edit staff and part properties
//---------------------------------------------------------

class EditStaff : public QDialog, private Ui::EditStaffBase {
      Q_OBJECT

      Staff* staff;
      Instrument instrument;

      void apply();
      void initFromInstrument(const Instrument&);
      void setInterval(const Interval&);

   private slots:
      void bboxClicked(QAbstractButton* button);
      void editDrumsetClicked();
      void showInstrumentDialog();

   public:
      EditStaff(Staff*, QWidget* parent = 0);
      };

//---------------------------------------------------------
//   SelectInstrument
//---------------------------------------------------------

class SelectInstrument : public QDialog, private Ui::SelectInstrument {
      Q_OBJECT

   private slots:
      void buildTemplateList();
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemDoubleClicked(QTreeWidgetItem* item, int);

   public:
      SelectInstrument(QWidget* parent = 0);
      InstrumentTemplate* instrTemplate() const;
      };

#endif

