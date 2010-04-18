//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      setupUi(this);

      Part* part = staff->part();
      instrument = *part;

      lines->setValue(staff->lines());
      small->setChecked(staff->small());

      useDrumset->setChecked(instrument.useDrumset());
      editDrumset->setEnabled(instrument.useDrumset());

      setInterval(instrument.transpose());

      shortName->setHtml(part->shortNameHtml());
      longName->setHtml(part->longNameHtml());
      slashStyle->setChecked(staff->slashStyle());
      invisible->setChecked(staff->invisible());

      aPitchMin->setValue(instrument.minPitchA());
      aPitchMax->setValue(instrument.maxPitchA());
      pPitchMin->setValue(instrument.minPitchP());
      pPitchMax->setValue(instrument.maxPitchP());

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(editDrumset, SIGNAL(clicked()), SLOT(editDrumsetClicked()));
      connect(changeInstrument, SIGNAL(clicked()), SLOT(showInstrumentDialog()));
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

      instrument.setUseDrumset(useDrumset->isChecked());
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

      bool snd = sn->toHtml() != part->shortName()->doc()->toHtml();
      bool lnd = ln->toHtml() != part->longName()->doc()->toHtml();

      instrument.setMinPitchA(aPitchMin->value());
      instrument.setMaxPitchA(aPitchMax->value());
      instrument.setMinPitchP(pPitchMin->value());
      instrument.setMaxPitchP(pPitchMax->value());

      if (snd || lnd || !(instrument == *part)) {
            score->undo()->push(new ChangePart(part, ln, sn, instrument));
            score->rebuildMidiMapping();
            seq->initInstruments();
            score->setPlaylistDirty(true);
            }

      int l        = lines->value();
      bool s       = small->isChecked();
      bool noStems = slashStyle->isChecked();
      bool inv     = invisible->isChecked();
      if (l != staff->lines() || s != staff->small() || noStems != staff->slashStyle()
         || inv != staff->invisible())
            score->undo()->push(new ChangeStaff(staff, l, s, noStems, inv));
      score->setUpdateAll(true);
      score->end();
      }

//---------------------------------------------------------
//   editDrumsetClicked
//---------------------------------------------------------

void EditStaff::editDrumsetClicked()
      {
      EditDrumset dse(staff->part()->drumset(), this);
      dse.exec();
      }

//---------------------------------------------------------
//   showInstrumentDialog
//---------------------------------------------------------

void EditStaff::showInstrumentDialog()
      {
      SelectInstrument si(this);
      if (si.exec()) {
            InstrumentTemplate* t = si.instrTemplate();
            // setMidiProgram(t->midiProgram);

            aPitchMin->setValue(t->minPitchA);
            aPitchMax->setValue(t->maxPitchA);
            pPitchMin->setValue(t->minPitchP);
            pPitchMax->setValue(t->maxPitchP);

            shortName->setHtml(t->shortName);
            longName->setHtml(t->longName);
            instrument.setTrackName(t->trackName);

            setInterval(t->transpose);

            if (t->useDrumset) {
                  instrument.setDrumset(new Drumset(*((t->drumset) ? t->drumset : smDrumset)));
                  instrument.setUseDrumset(true);
                  }
            else
                  instrument.setUseDrumset(false);
            useDrumset->setChecked(instrument.useDrumset());
            instrument.setMidiActions(t->midiActions);
            instrument.setArticulation(t->articulation);
            instrument.setChannel(t->channel);
            }
      }

//---------------------------------------------------------
//   SelectInstrument
//---------------------------------------------------------

SelectInstrument::SelectInstrument(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      buildTemplateList();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
      connect(showMore, SIGNAL(clicked()), SLOT(buildTemplateList()));
      }

//---------------------------------------------------------
//   buildTemplateList
//---------------------------------------------------------

void SelectInstrument::buildTemplateList()
      {
      populateInstrumentList(instrumentList, showMore->isChecked());
      }

//---------------------------------------------------------
//   on_instrumentList_itemSelectionChanged
//---------------------------------------------------------

void SelectInstrument::on_instrumentList_itemSelectionChanged()
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      bool flag = !wi.isEmpty();
      buttonBox->button(QDialogButtonBox::Ok)->setEnabled(flag);
      }

//---------------------------------------------------------
//   on_instrumentList
//---------------------------------------------------------

void SelectInstrument::on_instrumentList_itemDoubleClicked(QTreeWidgetItem*, int)
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if(!wi.isEmpty())
          done(true);
      }

//---------------------------------------------------------
//   instrTemplate
//---------------------------------------------------------

InstrumentTemplate* SelectInstrument::instrTemplate() const
      {
      QList<QTreeWidgetItem*> wi = instrumentList->selectedItems();
      if (wi.isEmpty())
            return 0;
      InstrumentTemplateListItem* item = (InstrumentTemplateListItem*)wi.front();
      return item->instrumentTemplate();
      }

