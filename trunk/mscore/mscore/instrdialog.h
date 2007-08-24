//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: instrdialog.h,v 1.14 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __INSTRUMENTS__
#define __INSTRUMENTS__

#include "ui_instrdialog.h"
#include "globals.h"

class InstrumentTemplate;
class Instrument;
class Part;
class Staff;
class Score;
class EditInstrument;

//---------------------------------------------------------
//   InstrumentTemplate
//---------------------------------------------------------

struct InstrumentTemplate {
      QString group;
      QString name;           // also used for track name
      QString shortName;
      int staves;             // 1 <= MAX_STAVES
      int clefIdx[MAX_STAVES];
      int staffLines[MAX_STAVES];
      int smallStaff[MAX_STAVES];
      int bracket;            // bracket type (NO_BRACKET)
      int midiProgram;
      int minPitch;
      int maxPitch;
      int transpose;          // for transposing instruments
      bool useDrumset;
      };

//---------------------------------------------------------
//   InstrumentsDialog
//---------------------------------------------------------

class InstrumentsDialog : public QDialog, public Ui::InstrumentDialogBase {
      Q_OBJECT
      Score* cs;
      EditInstrument* editInstrument;

   private slots:
      void on_instrumentList_itemSelectionChanged();
      void on_instrumentList_itemActivated(QTreeWidgetItem* item, int);
      void on_partiturList_itemSelectionChanged();
      void on_addButton_clicked();
      void on_removeButton_clicked();
      void on_upButton_clicked();
      void on_downButton_clicked();
      void on_editButton_clicked();
      void on_aboveButton_clicked();
      void on_belowButton_clicked();
      virtual void accept();

   public:
      InstrumentsDialog(QWidget* parent = 0);
      void setScore(Score* s) { cs = s; }
      void genPartList();
      };

//---------------------------------------------------------
//   InstrumentTemplateListItem
//---------------------------------------------------------

class InstrumentTemplateListItem : public QTreeWidgetItem {
      InstrumentTemplate* _instrumentTemplate;
      QString _group;

   public:
      InstrumentTemplateListItem(QString group, QTreeWidget* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, InstrumentTemplateListItem* parent);
      InstrumentTemplateListItem(InstrumentTemplate* i, QTreeWidget* parent);

      InstrumentTemplate* instrumentTemplate() const { return _instrumentTemplate; }
      virtual QString text(int col) const;
      };

enum { ITEM_KEEP, ITEM_DELETE, ITEM_ADD };
enum { PART_LIST_ITEM = QTreeWidgetItem::UserType, STAFF_LIST_ITEM };

//---------------------------------------------------------
//   PartListItem
//---------------------------------------------------------

class PartListItem : public QTreeWidgetItem {

   public:
      int op;
      Part* part;
      const InstrumentTemplate* it;

      PartListItem(Part* p, QTreeWidget* lv);
      PartListItem(const InstrumentTemplate* i, QTreeWidget* lv);
      };

//---------------------------------------------------------
//   StaffListItem
//---------------------------------------------------------

class StaffListItem : public QTreeWidgetItem {
      int _clef;
      int _partIdx;

   public:
      StaffListItem();
      StaffListItem(PartListItem* li);

      int op;
      Staff* staff;
      int partIdx() const      { return _partIdx; }
      void setPartIdx(int val);
      int staffIdx;

      void setClef(int val);
      int clef() const { return _clef; }
      };

extern QList<InstrumentTemplate*> instrumentTemplates;

#endif

