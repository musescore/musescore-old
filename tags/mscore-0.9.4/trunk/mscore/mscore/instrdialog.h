//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: instrdialog.h,v 1.14 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __INSTRDIALOG_H__
#define __INSTRDIALOG_H__

#include "ui_instrdialog.h"
#include "globals.h"

class InstrumentTemplate;
class Instrument;
class Part;
class Staff;
class Score;
class EditInstrument;

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

class InstrumentsDialog : public QDialog, public Ui::InstrumentDialogBase {
      Q_OBJECT
      Score* cs;
      EditInstrument* editInstrument;

      void buildTemplateList();

   private slots:
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemDoubleClicked(QTreeWidgetItem* item, int);
      void on_partiturList_itemSelectionChanged();
      void on_addButton_clicked();
      void on_removeButton_clicked();
      void on_upButton_clicked();
      void on_downButton_clicked();
      void on_editButton_clicked();
      void on_aboveButton_clicked();
      void on_belowButton_clicked();
      void on_saveButton_clicked();
      void on_loadButton_clicked();
      virtual void accept();

   public:
      InstrumentsDialog(QWidget* parent = 0);
      void setScore(Score* s) { cs = s; }
      void genPartList();
      };

#endif
