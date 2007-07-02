//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editinstrument.cpp,v 1.4 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "editinstrument.h"
#include "instrdialog.h"

//---------------------------------------------------------
//   EditInstrument
//---------------------------------------------------------

EditInstrument::EditInstrument(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void EditInstrument::update()
      {
      foreach(InstrumentTemplate* t, templates)
            delete t;
      templates.clear();

      QString curGroup;
      InstrumentTemplateListItem* group = 0;
      foreach(InstrumentTemplate* t, instrumentTemplates) {
            InstrumentTemplate* tt = new InstrumentTemplate(*t);
            templates.append(tt);
            if (curGroup != tt->group) {
                  curGroup = tt->group;
                  group    = new InstrumentTemplateListItem(curGroup, instrumentList);
                  group->setFlags(Qt::ItemIsEnabled);
                  }
            new InstrumentTemplateListItem(tt, group);
            }
      }

//---------------------------------------------------------
//   setCurrentInstrument
//---------------------------------------------------------

void EditInstrument::setCurrentInstrument(const InstrumentTemplate* t)
      {
      bool found = false;
      for (int i = 0; i < instrumentList->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = instrumentList->topLevelItem(i);
            int n = item->childCount();
            for (int k = 0; k < n; ++k) {
                  QTreeWidgetItem* item2 = item->child(k);
                  InstrumentTemplate* it = ((InstrumentTemplateListItem*)item2)->instrumentTemplate();
                  if (t == it) {
                        item2->setExpanded(true);
                        item2->setSelected(true);
                        found = true;
                        break;
                        }
                  }
            if (found) {
                  item->setExpanded(true);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   ~EditInstrument
//---------------------------------------------------------

EditInstrument::~EditInstrument()
      {
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void EditInstrument::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      InstrumentTemplate* t = 0;
      if (!wi.isEmpty()) {
            QTreeWidgetItem* item = wi.front();
            t = ((InstrumentTemplateListItem*)item)->instrumentTemplate();
            }

//      int clefIdx[MAX_STAVES];
//      int bracket;            // bracket type (NO_BRACKET)

      if (t == 0) {
            nameEdit->setText("");
            shortNameEdit->setText("");
            minPitch->setValue(0);
            maxPitch->setValue(0);
            transpose->setValue(0);
            midiProgram->setValue(0);
            staves->setValue(1);
            }
      else {
            nameEdit->setText(t->name);
            shortNameEdit->setText(t->shortName);
            minPitch->setValue(t->minPitch);
            maxPitch->setValue(t->maxPitch);
            transpose->setValue(t->transpose);
            midiProgram->setValue(t->midiProgram);
            staves->setValue(t->staves);
            }
      }

//---------------------------------------------------------
//   on_buttonCancel_pressed
//---------------------------------------------------------

void EditInstrument::on_buttonCancel_pressed()
      {
//      printf("cancel\n");
      }

//---------------------------------------------------------
//   on_buttonOk_pressed
//---------------------------------------------------------

void EditInstrument::on_buttonOk_pressed()
      {
      foreach(InstrumentTemplate* t, instrumentTemplates)
            delete t;
      instrumentTemplates.clear();

      for (int i = 0; i < instrumentList->topLevelItemCount(); ++i) {
            QTreeWidgetItem* item = instrumentList->topLevelItem(i);
            int n = item->childCount();
            for (int k = 0; k < n; ++k) {
                  QTreeWidgetItem* item2 = item->child(k);
                  InstrumentTemplate* it = ((InstrumentTemplateListItem*)item2)->instrumentTemplate();
                  instrumentTemplates.append(it);
                  }
            }
      }

//---------------------------------------------------------
//   on_buttonNew_pressed
//---------------------------------------------------------

void EditInstrument::on_buttonNew_pressed()
      {
      printf("New\n");
      }

