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
      bool        modif = false;                            // assume no modifications
      int         idx   = o->data(Qt::UserRole).toInt();
      StaffType*  st    = staffTypes[idx];
      StaffTypeTablature*  stt;

      // if any of the common properties is modified
      if (name->text()              != st->name()
         || st->lines()             != lines->value()
         || st->lineDistance().val()!= lineDistance->value()
         || st->genKeysig()         != genKeysig->isChecked()
         || st->showLedgerLines()   != showLedgerLines->isChecked()
         )
            modif = true;
      // or if any of the props specific to each group is modified
      switch(st->group()) {
      case PITCHED_STAFF:
            if(st->genClef() != genClefPitched->isChecked()
               || st->showBarlines() != showBarlinesPitched->isChecked()
               || st->slashStyle()   != stemlessPitched->isChecked()
               )
                  modif = true;
            break;
      case TAB_STAFF:
            if(st->genClef()              != genClefTablature->isChecked()
               || st->showBarlines()      != showBarlinesTablature->isChecked()
               )
                  modif = true;
            stt = (StaffTypeTablature*)st;
            if(stt->durationFontName()    != durFontName->currentFont().family()
               || stt->durationFontSize() != durFontSize->value()
               || stt->durationFontUserY()!= durY->value()
               || stt->fretFontName()     != fretFontName->currentFont().family()
               || stt->fretFontSize()     != fretFontSize->value()
               || stt->fretFontUserY()    != fretY->value()
               || stt->genDurations()     != genDurations->isChecked()
               || stt->genTimesig()       != genTimesigTablature->isChecked()
               || stt->linesThrough()     != linesThroughRadio->isChecked()
               || stt->onLines()          != onLinesRadio->isChecked()
               || stt->useNumbers()       != numbersRadio->isChecked()
               )
                  modif = true;
            break;
      case PERCUSSION_STAFF:
            if(st->genClef() != genClefPercussion->isChecked())
                  modif = true;
            break;
            }

#if 0
            if (!st->modified()) {
                  StaffType* nst = new StaffType(*st);
                  nst->setModified(true);
                  staffTypes[idx] = nst;
                  st = nst;
                  }
#endif
      if(modif) {
            // save common properties
            st->setName(o->text());
            st->setLines(lines->value());
            st->setLineDistance(Spatium(lineDistance->value()));
            st->setGenKeysig(genKeysig->isChecked());
            st->setShowLedgerLines(showLedgerLines->isChecked());
            // save-group specific properties
            switch(st->group()) {
            case PITCHED_STAFF:
                  st->setGenClef(genClefPitched->isChecked());
                  st->setSlashStyle(stemlessPitched->isChecked());
                  st->setShowBarlines(showBarlinesPitched->isChecked());
                  break;
            case TAB_STAFF:
                  st->setGenClef(genClefTablature->isChecked());
                  st->setSlashStyle(stemlessTablature->isChecked());
                  st->setShowBarlines(showBarlinesTablature->isChecked());
                  stt = (StaffTypeTablature*)st;
                  stt->setDurationFontName(durFontName->currentText());
                  stt->setDurationFontSize(durFontSize->value());
                  stt->setDurationFontUserY(durY->value());
                  stt->setFretFontName(fretFontName->currentText());
                  stt->setFretFontSize(fretFontSize->value());
                  stt->setFretFontUserY(fretY->value());
                  stt->setGenDurations(genDurations->isChecked());
                  stt->setGenTimesig(genTimesigTablature->isChecked());
                  stt->setLinesThrough(linesThroughRadio->isChecked());
                  stt->setOnLines(onLinesRadio->isChecked());
                  stt->setUseNumbers(numbersRadio->isChecked());
                  break;
            case PERCUSSION_STAFF:
                  st->setGenClef(genClefPercussion->isChecked());
                  break;
                  }
            modified = true;
            }
      }

//---------------------------------------------------------
//   typeChanged
//---------------------------------------------------------

void EditStaffType::typeChanged(QListWidgetItem* n, QListWidgetItem* o)
      {
      StaffTypeTablature * tab;

      if (n == 0)
            return;
      if (o)
            saveCurrent(o);
      // retrieve staff type corresponding to new current item in type list
      int idx = n->data(Qt::UserRole).toInt();
      StaffType* st = staffTypes[idx];
      // set properties common to all groups (some props appears in multiple group pages)
      name->setText(st->name());
      lines->setValue(st->lines());
      lineDistance->setValue(st->lineDistance().val());
      genClefPitched->setChecked(st->genClef());
      genClefTablature->setChecked(st->genClef());
      genClefPercussion->setChecked(st->genClef());
      genKeysig->setChecked(st->genKeysig());
      stemlessPitched->setChecked(st->slashStyle());
      stemlessTablature->setChecked(st->slashStyle());
      showBarlinesPitched->setChecked(st->showBarlines());
      showBarlinesTablature->setChecked(st->showBarlines());
      showLedgerLines->setChecked(st->showLedgerLines());
      // switch to stack page and set props specific to each staff group
      QFont f = QFont();
      switch(st->group()) {
            case PITCHED_STAFF:
                  stack->setCurrentIndex(0);
//                  staffGroup->setText(tr("Pitched"));
                  break;
            case TAB_STAFF:
                  stack->setCurrentIndex(1);
//                  staffGroup->setText(tr("Tablature"));
                  tab = (StaffTypeTablature*)st;
                  genTimesigTablature->setChecked(tab->genTimesig());
                  genDurations->setChecked(tab->genDurations());
                  f.setFamily(tab->fretFontName());
                  f.setPointSizeF(tab->fretFontSize());
                  fretFontName->setCurrentFont(f);
                  fretFontSize->setValue(tab->fretFontSize());
                  fretY->setValue(tab->fretFontUserY());
                  numbersRadio->setChecked(tab->useNumbers());
                  lettersRadio->setChecked(!tab->useNumbers());
                  onLinesRadio->setChecked(tab->onLines());
                  aboveLinesRadio->setChecked(!tab->onLines());
                  linesThroughRadio->setChecked(tab->linesThrough());
                  linesBrokenRadio->setChecked(!tab->linesThrough());
                  f.setFamily(tab->durationFontName());
                  f.setPointSizeF(tab->durationFontSize());
                  durFontName->setCurrentFont(f);
                  durFontSize->setValue(tab->durationFontSize());
                  durY->setValue(tab->durationFontUserY());
                  break;
            case PERCUSSION_STAFF:
                  stack->setCurrentIndex(2);
//                  staffGroup->setText(tr("Percussion"));
                  break;
            }
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


