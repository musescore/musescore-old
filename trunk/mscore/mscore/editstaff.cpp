//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "editstaff.h"
#include "staff.h"
#include "part.h"
#include "editdrumset.h"
#include "score.h"
#include "measure.h"
#include "undo.h"
#include "text.h"
#include "utils.h"
#include "instrtemplate.h"
#include "seq.h"
#include "stafftype.h"
#include "selinstrument.h"
#include "texteditor.h"

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      setupUi(this);

      Part* part = staff->part();
      instrument = *part->instr();

      Score* score = part->score();
      int idx      = 0;
      int curIdx   = 0;
      foreach(StaffType* st, score->staffTypes()) {
            staffType->addItem(st->name(), idx);
            if (st == s->staffType())
                  curIdx = idx;
            ++idx;
            }
      staffType->setCurrentIndex(curIdx);

      small->setChecked(staff->small());
      setInterval(instrument.transpose());
      shortName->setHtml(part->instr(0)->shortName().toHtml());
      longName->setHtml(part->instr(0)->longName().toHtml());
      invisible->setChecked(staff->invisible());

      aPitchMin->setValue(instrument.minPitchA());
      aPitchMax->setValue(instrument.maxPitchA());
      pPitchMin->setValue(instrument.minPitchP());
      pPitchMax->setValue(instrument.maxPitchP());

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument, SIGNAL(clicked()), SLOT(showInstrumentDialog()));
      connect(editStaffType,    SIGNAL(clicked()), SLOT(showEditStaffType()));
      connect(editShortName,    SIGNAL(clicked()), SLOT(editShortNameClicked()));
      connect(editLongName,     SIGNAL(clicked()), SLOT(editLongNameClicked()));
      }

//---------------------------------------------------------
//   setInterval
//---------------------------------------------------------

void EditStaff::setInterval(const Interval& iv)
      {
      int diatonic  = iv.diatonic;
      int chromatic = iv.chromatic;

      int oct = chromatic / 12;
      if (oct < 0)
            oct = -oct;

      bool upFlag = true;
      if (chromatic < 0 || diatonic < 0) {
            upFlag    = false;
            chromatic = -chromatic;
            diatonic  = -diatonic;
            }
      chromatic %= 12;
      diatonic  %= 7;

      int interval = searchInterval(diatonic, chromatic);
      if (interval == -1) {
            printf("EditStaff: unknown interval %d %d\n", diatonic, chromatic);
            interval = 0;
            }
      iList->setCurrentIndex(interval);
      up->setChecked(upFlag);
      down->setChecked(!upFlag);
      octave->setValue(oct);
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void EditStaff::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  printf("EditStaff: unknown button %d\n", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStaff::apply()
      {
      Score* score  = staff->score();
      Part* part    = staff->part();

      int intervalIdx = iList->currentIndex();
      bool upFlag     = up->isChecked();

      Interval interval = intervalList[intervalIdx];
      interval.diatonic  += octave->value() * 7;
      interval.chromatic += octave->value() * 12;

      if (!upFlag)
            interval.flip();
      instrument.setTranspose(interval);

      const QTextDocument* ln = longName->document();
      const QTextDocument* sn = shortName->document();

      bool snd = sn->toHtml() != part->shortName().toHtml();
      bool lnd = ln->toHtml() != part->longName().toHtml();

      instrument.setMinPitchA(aPitchMin->value());
      instrument.setMaxPitchA(aPitchMax->value());
      instrument.setMinPitchP(pPitchMin->value());
      instrument.setMaxPitchP(pPitchMax->value());

      if (snd || lnd || !(instrument == *part->instr())) {
            score->undo()->push(new ChangePart(part, ln, sn, instrument));
            }

      bool s   = small->isChecked();
      bool inv = invisible->isChecked();
      StaffType* st = score->staffTypes()[staffType->currentIndex()];

      if (
         s != staff->small()
         || inv != staff->invisible()
         || st  != staff->staffType()
           )
            score->undo()->push(new ChangeStaff(staff, s, inv, staff->show(), st));
      score->setLayoutAll(true);
      score->end();
      }

//---------------------------------------------------------
//   editDrumsetClicked
//---------------------------------------------------------

void EditStaff::editDrumsetClicked()
      {
      EditDrumset dse(staff->part()->instr()->drumset(), this);
      dse.exec();
      }

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(instrument, this);
      if (si.exec()) {
            const InstrumentTemplate* t = si.instrTemplate();
            // setMidiProgram(t->midiProgram);

            aPitchMin->setValue(t->minPitchA);
            aPitchMax->setValue(t->maxPitchA);
            pPitchMin->setValue(t->minPitchP);
            pPitchMax->setValue(t->maxPitchP);

            shortName->setHtml(t->shortName);
            longName->setHtml(t->longName);
//TODOxx            trackName = t->trackName;

            setInterval(t->transpose);

            if (t->useDrumset) {
                  instrument.setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
                  instrument.setUseDrumset(true);
                  }
            else
                  instrument.setUseDrumset(false);
            instrument.setMidiActions(t->midiActions);
            instrument.setArticulation(t->articulation);
            instrument.setChannel(t->channel);
            }
      }

//---------------------------------------------------------
//   editTablatureClicked
//---------------------------------------------------------

void EditStaff::editTablatureClicked()
      {
      printf("Edit Tablature\n");
      }

//---------------------------------------------------------
//   showEditStaffType
//---------------------------------------------------------

void EditStaff::showEditStaffType()
      {
      EditStaffType* est = new EditStaffType(this, staff);
      if (est->exec() && est->isModified()) {
            QList<StaffType*> tl = est->getStaffTypes();
            Score* score = staff->score();
            score->setStaffTypeList(tl);
            int curIdx   = 0;
            staffType->clear();
            int idx = 0;
            foreach(StaffType* st, tl) {
                  staffType->addItem(st->name(), idx);
                  if (st == staff->staffType())
                        curIdx = idx;
                  ++idx;
                  }
            staffType->setCurrentIndex(curIdx);
            }
      }

//---------------------------------------------------------
//   editShortNameClicked
//---------------------------------------------------------

void EditStaff::editShortNameClicked()
      {
      QString s = editText(shortName->toHtml());
      shortName->setHtml(s);
      }

//---------------------------------------------------------
//   editLongNameClicked
//---------------------------------------------------------

void EditStaff::editLongNameClicked()
      {
      QString s = editText(longName->toHtml());
      longName->setHtml(s);
      }
