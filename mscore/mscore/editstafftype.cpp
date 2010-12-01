//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "editstafftype.h"
#include "stafftype.h"
#include "score.h"
#include "staff.h"

//---------------------------------------------------------
//   EditStaffType
//---------------------------------------------------------

EditStaffType::EditStaffType(QWidget* parent, Staff* st)
   : QDialog(parent)
      {
      setupUi(this);
      staff = st;
      Score* score = staff->score();
      staffTypes   = score->staffTypes();
      int idx = 0;
      QListWidgetItem* ci = 0;
      foreach(StaffType* st, staffTypes) {
            QListWidgetItem* item = new QListWidgetItem(st->name());
            item->setData(Qt::UserRole, idx);
            if (st == staff->staffType())
                  ci = item;
            staffTypeList->addItem(item);
            ++idx;
            }
      connect(staffTypeList, SIGNAL(currentItemChanged(QListWidgetItem*, QListWidgetItem*)),
         SLOT(typeChanged(QListWidgetItem*, QListWidgetItem*)));
      connect(newType, SIGNAL(clicked()), SLOT(createNewType()));
      connect(name, SIGNAL(textEdited(const QString&)), SLOT(nameEdited(const QString&)));
      if (ci)
            staffTypeList->setCurrentItem(ci);
      modified = false;
      }

//---------------------------------------------------------
//   saveCurrent
//---------------------------------------------------------

void EditStaffType::saveCurrent(QListWidgetItem* o)
      {
      int idx       = o->data(Qt::UserRole).toInt();
      StaffType* st = staffTypes[idx];
      StaffGroup sg;
      if (typePitched->isChecked())
            sg = PITCHED_STAFF;
      else if (typeTab->isChecked())
            sg = TAB_STAFF;
      else if (typePercussion->isChecked())
            sg = PERCUSSION_STAFF;

      if (name->text() != st->name()
         || st->group() != sg
         || st->lines() != lines->value()
         || st->lineDistance().val() != lineDistance->value()
         || st->genClef() != useClef->isChecked()
         || st->genKeysig() != useKeysig->isChecked()
         || st->slashStyle() != stemless->isChecked()
         || st->showBarlines() != useBarlines->isChecked()
         || st->showLedgerLines() != useLedgerLines->isChecked()
         ) {
#if 0
            if (!st->modified()) {
                  StaffType* nst = new StaffType(*st);
                  nst->setModified(true);
                  staffTypes[idx] = nst;
                  st = nst;
                  }
#endif
            st->setName(o->text());
//TODO: cannot morph            st->setGroup(sg);
            st->setLines(lines->value());
            st->setLineDistance(Spatium(lineDistance->value()));
            st->setGenClef(useClef->isChecked());
            st->setGenKeysig(useKeysig->isChecked());
            st->setSlashStyle(stemless->isChecked());
            st->setShowBarlines(useBarlines->isChecked());
            st->setShowLedgerLines(useLedgerLines->isChecked());
            modified = true;
            }
      }

//---------------------------------------------------------
//   typeChanged
//---------------------------------------------------------

void EditStaffType::typeChanged(QListWidgetItem* n, QListWidgetItem* o)
      {
      if (n == 0)
            return;
      if (o)
            saveCurrent(o);
      int idx = n->data(Qt::UserRole).toInt();
      StaffType* st = staffTypes[idx];
      name->setText(st->name());
      typePitched->setChecked(st->group() == PITCHED_STAFF);
      typeTab->setChecked(st->group() == TAB_STAFF);
      typePercussion->setChecked(st->group() == PERCUSSION_STAFF);
      lines->setValue(st->lines());
      lineDistance->setValue(st->lineDistance().val());
      useClef->setChecked(st->genClef());
      useKeysig->setChecked(st->genKeysig());
      stemless->setChecked(st->slashStyle());
      useBarlines->setChecked(st->showBarlines());
      useLedgerLines->setChecked(st->showLedgerLines());
      }

//---------------------------------------------------------
//   createNewType
//---------------------------------------------------------

void EditStaffType::createNewType()
      {
      //
      // initialize new StaffType with current selected one
      //
      int idx       = staffTypeList->currentItem()->data(Qt::UserRole).toInt();
      StaffType* ns = staffTypes[idx]->clone();
      ns->setModified(true);

      //
      // create unique new name for StaffType
      //
      for (int i = 1;;++i) {
            QString name = QString("type-%1").arg(i);
            int n = staffTypes.size();
            int k;
            for (k = 0; k < n; ++k) {
                  if (staffTypes[k]->name() == name)
                        break;
                  }
            if (k == n) {
                  ns->setName(name);
                  break;
                  }
            }
      staffTypes.append(ns);
      QListWidgetItem* item = new QListWidgetItem(ns->name());
      item->setData(Qt::UserRole, staffTypes.size() - 1);
      staffTypeList->addItem(item);
      staffTypeList->setCurrentItem(item);
      modified = true;
      }

//---------------------------------------------------------
//   nameEdited
//---------------------------------------------------------

void EditStaffType::nameEdited(const QString& s)
      {
      staffTypeList->currentItem()->setText(s);
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void EditStaffType::accept()
      {
      QListWidgetItem* item = staffTypeList->currentItem();
      if (item)
            saveCurrent(item);
      QDialog::accept();
      }


