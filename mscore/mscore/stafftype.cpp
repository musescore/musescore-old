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

#include "stafftype.h"
#include "staff.h"
#include "score.h"

QList<StaffType*> staffTypes;

//---------------------------------------------------------
//   initStaffTypes
//---------------------------------------------------------

void initStaffTypes()
      {
      StaffType* st = new StaffType("Pitched 5 lines");
      st->setGroup(PITCHED_STAFF);
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(true);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);

      st = new StaffType("Tab");
      st->setGroup(TAB_STAFF);
      st->setLines(6);
      st->setLineDistance(Spatium(1.5));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(false);
      staffTypes.append(st);

      st = new StaffType("Percussion 5 lines");
      st->setGroup(PERCUSSION_STAFF);
      st->setLines(5);
      st->setLineDistance(Spatium(1.0));
      st->setGenClef(true);
      st->setGenKeysig(false);
      st->setSlashStyle(false);
      st->setShowBarlines(true);
      st->setShowLedgerLines(true);
      staffTypes.append(st);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffType::write(Xml& xml, int idx) const
      {
      xml.stag(QString("StaffType idx=\"%1\"").arg(idx));
      xml.tag("name", name());
      if (lines() != 5)
            xml.tag("lines", lines());
      if (lineDistance().val() != 1.0)
            xml.tag("lineDistance", lineDistance().val());
      if (!genClef())
            xml.tag("clef", genClef());
      if (!genKeysig())
            xml.tag("keysig", genKeysig());
      if (slashStyle())
            xml.tag("slashStyle", slashStyle());
      if (!showBarlines())
            xml.tag("barlines", showBarlines());
      if (!showLedgerLines())
            xml.tag("ledgerlines", showLedgerLines());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffType::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            if (tag == "name")
                  setName(e.text());
            else if (tag == "lines")
                  setLines(v);
            else if (tag == "lineDistance")
                  setLineDistance(Spatium(e.text().toDouble()));
            else if (tag == "clef")
                  setGenClef(v);
            else if (tag == "keysig")
                  setGenKeysig(v);
            else if (tag == "slashStyle")
                  setSlashStyle(v);
            else if (tag == "barlines")
                  setShowBarlines(v);
            else if (tag == "ledgerlines")
                  setShowLedgerLines(v);
            else
                  domError(e);
            }
      }

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
//   typeChanged
//---------------------------------------------------------

void EditStaffType::typeChanged(QListWidgetItem* n, QListWidgetItem* o)
      {
      printf("%p -> %p\n", o, n);
      if (n == 0)
            return;
      if (o) {
            //
            // save modified StaffType
            //
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
                  if (!st->modified()) {
                        StaffType* nst = new StaffType(*st);
                        nst->setModified(true);
                        staffTypes[idx] = nst;
                        st = nst;
                        }
                  st->setName(o->text());
                  st->setGroup(sg);
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
      StaffType* ns = new StaffType(*staffTypes[idx]);
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
                  staffTypes.append(ns);
                  QListWidgetItem* item = new QListWidgetItem(ns->name());
                  item->setData(Qt::UserRole, n);
                  staffTypeList->addItem(item);
                  staffTypeList->setCurrentItem(item);
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   nameEdited
//---------------------------------------------------------

void EditStaffType::nameEdited(const QString& s)
      {
      staffTypeList->currentItem()->setText(s);
      }

