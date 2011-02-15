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
#include "editstafftype.h"
#include "editpitch.h"
#include "editstringdata.h"
#include "tablature.h"

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

//      aPitchMin->setValue(instrument.minPitchA());
//      aPitchMax->setValue(instrument.maxPitchA());
//      pPitchMin->setValue(instrument.minPitchP());
//      pPitchMax->setValue(instrument.maxPitchP());
      _minPitchA = instrument.minPitchA();
      _maxPitchA = instrument.maxPitchA();
      _minPitchP = instrument.minPitchP();
      _maxPitchP = instrument.maxPitchP();
      minPitchA->setText(midiCodeToStr(_minPitchA));
      maxPitchA->setText(midiCodeToStr(_maxPitchA));
      minPitchP->setText(midiCodeToStr(_minPitchP));
      maxPitchP->setText(midiCodeToStr(_maxPitchP));

      int numStr = instrument.tablature() ? instrument.tablature()->strings() : 0;
      numOfStrings->setText(QString::number(numStr));

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(bboxClicked(QAbstractButton*)));
      connect(changeInstrument, SIGNAL(clicked()), SLOT(showInstrumentDialog()));
      connect(editStaffType,    SIGNAL(clicked()), SLOT(showEditStaffType()));
      connect(editShortName,    SIGNAL(clicked()), SLOT(editShortNameClicked()));
      connect(editLongName,     SIGNAL(clicked()), SLOT(editLongNameClicked()));
      connect(minPitchASelect,  SIGNAL(clicked()), SLOT(minPitchAClicked()));
      connect(maxPitchASelect,  SIGNAL(clicked()), SLOT(maxPitchAClicked()));
      connect(minPitchPSelect,  SIGNAL(clicked()), SLOT(minPitchPClicked()));
      connect(maxPitchPSelect,  SIGNAL(clicked()), SLOT(maxPitchPClicked()));
      connect(editStringData,   SIGNAL(clicked()), SLOT(editStringDataClicked()));
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

      instrument.setMinPitchA(_minPitchA);
      instrument.setMaxPitchA(_maxPitchA);
      instrument.setMinPitchP(_minPitchP);
      instrument.setMaxPitchP(_maxPitchP);
      instrument.setShortName(QTextDocumentFragment(shortName->document()));
      instrument.setLongName(QTextDocumentFragment(longName->document()));

      bool s   = small->isChecked();
      bool inv = invisible->isChecked();
      StaffType* st = score->staffTypes()[staffType->currentIndex()];
      bool updateNeeded = false;
      // before changing instrument, check if notes need to be updated
      // true if changing into or away from TAB or from one TAB type to another
      if(   (st->group() == TAB_STAFF && staff->staffType()->group() != TAB_STAFF) ||
            (st->group() != TAB_STAFF && staff->staffType()->group() == TAB_STAFF) ||
            (st->group() == TAB_STAFF && staff->staffType()->group() == TAB_STAFF &&
                        instrument.tablature() != part->instr()->tablature()) )
            updateNeeded = true;

      if (!(instrument == *part->instr())) {
printf("instrument changed <%s>\n", qPrintable(instrument.longName().toPlainText()));
            score->undo()->push(new ChangePart(part, instrument));
            }

      if (
         s != staff->small()
         || inv != staff->invisible()
         || st  != staff->staffType()
           )
            score->undo()->push(new ChangeStaff(staff, s, inv, staff->show(), st));

      if(updateNeeded)
            score->cmdUpdateNotes();

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

            _minPitchA = t->minPitchA;
            _maxPitchA = t->maxPitchA;
            _minPitchP = t->minPitchP;
            _maxPitchP = t->maxPitchP;
            minPitchA->setText(midiCodeToStr(_minPitchA));
            maxPitchA->setText(midiCodeToStr(_maxPitchA));
            minPitchP->setText(midiCodeToStr(_minPitchP));
            maxPitchP->setText(midiCodeToStr(_maxPitchP));

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

//---------------------------------------------------------
//   <Pitch>Clicked
//---------------------------------------------------------

void EditStaff::minPitchAClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.minPitchA() );
      if ( (newCode=ep->exec()) != -1) {
            minPitchA->setText(midiCodeToStr(newCode));
            _minPitchA = newCode;
            }
      }

void EditStaff::maxPitchAClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.maxPitchA() );
      if ( (newCode=ep->exec()) != -1) {
            maxPitchA->setText(midiCodeToStr(newCode));
            _maxPitchA = newCode;
            }
      }

void EditStaff::minPitchPClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.minPitchP() );
      if ( (newCode=ep->exec()) != -1) {
            minPitchP->setText(midiCodeToStr(newCode));
            _minPitchP = newCode;
            }
      }

void EditStaff::maxPitchPClicked()
      {
      int         newCode;

      EditPitch* ep = new EditPitch(this, instrument.maxPitchP() );
      if ( (newCode=ep->exec()) != -1) {
            maxPitchP->setText(midiCodeToStr(newCode));
            _maxPitchP = newCode;
            }
      }

//---------------------------------------------------------
//   editStringDataClicked
//---------------------------------------------------------

void EditStaff::editStringDataClicked()
      {
      int         frets = instrument.tablature()->frets();
      QList<int>  stringList = instrument.tablature()->stringList();

      EditStringData* esd = new EditStringData(this, &stringList, &frets);
      if (esd->exec()) {
            Tablature * tab = new Tablature(frets, stringList);
            instrument.setTablature(tab);
            }
      }

//---------------------------------------------------------
//   midiCodeToStr
//    Converts a MIDI numeric pitch code to human-readable note name
//---------------------------------------------------------

static char g_cNoteName[12][4] =
{"C", "C#", "D", "Eb", "E", "F", "F#", "G", "Ab", "A", "Bb", "B" };

QString EditStaff::midiCodeToStr(int midiCode)
      {
      return QString("%1 %2").arg(g_cNoteName[midiCode % 12]).arg(midiCode / 12 - 1);
      }
