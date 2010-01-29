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

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      setupUi(this);

      Part* part = staff->part();

      useDrumset->setChecked(part->useDrumset());
      editDrumset->setEnabled(part->useDrumset());
      lines->setValue(staff->lines());
      small->setChecked(staff->small());
      int diatonic  = part->transposeDiatonic();
      int chromatic = part->transposeChromatic();
      bool upFlag = true;
      if (chromatic < 0 || diatonic < 0) {
            upFlag = false;
            chromatic = -chromatic;
            diatonic  = -diatonic;
            }
      int interval = searchInterval(diatonic, chromatic);
      if (interval == -1) {
            printf("unknown interval %d %d\n", diatonic, chromatic);
            }
      else {
            iList->setCurrentIndex(interval);
            up->setChecked(upFlag);
            }

      shortName->setHtml(part->shortNameHtml());
      longName->setHtml(part->longNameHtml());
      slashStyle->setChecked(staff->slashStyle());
      invisible->setChecked(staff->invisible());

      aPitchMin->setValue(part->minPitchA());
      aPitchMax->setValue(part->maxPitchA());
      pPitchMin->setValue(part->minPitchP());
      pPitchMax->setValue(part->maxPitchP());

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(editDrumset, SIGNAL(clicked()), SLOT(editDrumsetClicked()));
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

      bool ud       = useDrumset->isChecked();
      int interval  = iList->currentIndex();
      bool upFlag   = up->isChecked();
      int diatonic  = intervalList[interval].steps;
      int chromatic = intervalList[interval].semitones;

      if (!upFlag) {
            diatonic  = -diatonic;
            chromatic = -chromatic;
            }
      const QTextDocument* ln = longName->document();
      const QTextDocument* sn = shortName->document();

      bool snd = sn->toHtml() != part->shortName()->doc()->toHtml();
      bool lnd = ln->toHtml() != part->longName()->doc()->toHtml();
      int aMin = aPitchMin->value();
      int aMax = aPitchMax->value();
      int pMin = pPitchMin->value();
      int pMax = pPitchMax->value();

      if ((ud != part->useDrumset())
         || (diatonic != part->transposeDiatonic())
         || (chromatic != part->transposeChromatic())
         || snd || lnd
         || aMin != part->minPitchA()
         || aMax != part->maxPitchA()
         || pMin != part->minPitchP()
         || pMax != part->maxPitchP()
         ) {
            score->undo()->push(new ChangePart(part, ud, diatonic,
               chromatic, ln, sn, aMin, aMax, pMin, pMax));
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

