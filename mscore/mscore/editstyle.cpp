//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstyle.cpp,v 1.10 2006/03/02 17:08:33 wschweer Exp $
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

#include "score.h"
#include "canvas.h"
#include "style.h"
#include "editstyle.h"

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(QWidget* parent)
   : QDialog(parent)
      {
      lstyle = new Style;
      setModal(true);
      setupUi(this);
      pageList->setCurrentRow(0);
      connect(buttonOk, SIGNAL(clicked()), this, SLOT(ok()));
      connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
      }

EditStyle::~EditStyle()
      {
      delete lstyle;
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void EditStyle::ok()
      {
      apply();
      close();
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStyle::apply()
      {
      getValues();
      *(cs->style()) = *lstyle;
      cs->startCmd();
      cs->setLayoutAll(true);
      cs->endCmd();
      cs->setDirty(true);
      }

//---------------------------------------------------------
//   getValues
//---------------------------------------------------------

void EditStyle::getValues()
      {
      lstyle->staffUpperBorder       = Spatium(staffUpperBorder->value());
      lstyle->staffLowerBorder       = Spatium(staffLowerBorder->value());
      lstyle->staffDistance          = Spatium(staffDistance->value());
      lstyle->accoladeDistance       = Spatium(akkoladeDistance->value());
      lstyle->systemDistance         = Spatium(systemDistance->value());
      lstyle->lyricsDistance         = Spatium(lyricsDistance->value());
      lstyle->lyricsMinBottomDistance = Spatium(lyricsMinBottomDistance->value());
      lstyle->systemBoxDistance       = Spatium(systemFrameDistance->value());
      lstyle->boxSystemDistance       = Spatium(frameSystemDistance->value());

      lstyle->measureSpacing         = measureSpacing->value();
      lstyle->minNoteDistance        = Spatium(minNoteDistance->value());
      lstyle->barNoteDistance        = Spatium(barNoteDistance->value());
      lstyle->noteBarDistance        = Spatium(noteBarDistance->value());

      lstyle->showPageNumber         = showPageNumber->isChecked();
      lstyle->showPageNumberOne      = showFirstPageNumber->isChecked();
      lstyle->pageNumberOddEven      = showOddEvenPageNumber->isChecked();
      lstyle->showMeasureNumber      = showMeasureNumber->isChecked();
      lstyle->showMeasureNumberOne   = showFirstMeasureNumber->isChecked();
      lstyle->measureNumberInterval  = intervalMeasureNumber->value();
      lstyle->measureNumberSystem    = showEverySystemMeasureNumber->isChecked();
      lstyle->measureNumberAllStaffs = showAllStaffsMeasureNumber->isChecked();

      lstyle->clefLeftMargin         = Spatium(clefLeftMargin->value());
      lstyle->keysigLeftMargin       = Spatium(keysigLeftMargin->value());
      lstyle->timesigLeftMargin      = Spatium(timesigLeftMargin->value());
      lstyle->clefKeyRightMargin     = Spatium(clefKeyRightMargin->value());

      lstyle->beamWidth              = Spatium(beamWidth->value());
      lstyle->beamDistance           = beamDistance->value();
      lstyle->beamMinLen             = Spatium(beamMinLen->value());
      lstyle->beamMinSlope           = beamMinSlope->value();
      lstyle->beamMaxSlope           = beamMaxSlope->value();
      lstyle->graceNoteMag           = graceNoteSize->value() * 0.01;
      lstyle->smallStaffMag          = smallStaffSize->value() * 0.01;
      lstyle->smallNoteMag           = smallNoteSize->value() * 0.01;
      lstyle->smallClefMag           = smallClefSize->value() * 0.01;

      lstyle->pageFillLimit          = pageFillThreshold->value() * 0.01;
      lstyle->lastSystemFillLimit    = lastSystemFillThreshold->value() * 0.01;

      lstyle->hairpinWidth           = Spatium(hairpinLineWidth->value());
      lstyle->hairpinHeight          = Spatium(hairpinHeight->value());
      lstyle->hairpinContHeight      = Spatium(hairpinContinueHeight->value());
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void EditStyle::setScore(Score* s)
      {
      cs = s;
      *lstyle = *(cs->style());
      setValues();
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      staffUpperBorder->setValue(lstyle->staffUpperBorder.val());
      staffLowerBorder->setValue(lstyle->staffLowerBorder.val());
      staffDistance->setValue(lstyle->staffDistance.val());
      akkoladeDistance->setValue(lstyle->accoladeDistance.val());
      systemDistance->setValue(lstyle->systemDistance.val());
      lyricsDistance->setValue(lstyle->lyricsDistance.val());
      lyricsMinBottomDistance->setValue(lstyle->lyricsMinBottomDistance.val());
      systemFrameDistance->setValue(lstyle->systemBoxDistance.val());
      frameSystemDistance->setValue(lstyle->boxSystemDistance.val());

      measureSpacing->setValue(lstyle->measureSpacing);
      minNoteDistance->setValue(lstyle->minNoteDistance.val());
      barNoteDistance->setValue(lstyle->barNoteDistance.val());
      noteBarDistance->setValue(lstyle->noteBarDistance.val());

      showPageNumber->setChecked(lstyle->showPageNumber);
      showFirstPageNumber->setChecked(lstyle->showPageNumberOne);
      showOddEvenPageNumber->setChecked(lstyle->pageNumberOddEven);
      showMeasureNumber->setChecked(lstyle->showMeasureNumber);
      showFirstMeasureNumber->setChecked(lstyle->showMeasureNumberOne);
      intervalMeasureNumber->setValue(lstyle->measureNumberInterval);
      showIntervalMeasureNumber->setChecked(!lstyle->measureNumberSystem);
      showAllStaffsMeasureNumber->setChecked(lstyle->measureNumberAllStaffs);
      showEverySystemMeasureNumber->setChecked(lstyle->measureNumberSystem);

      clefLeftMargin->setValue(lstyle->clefLeftMargin.val());
      keysigLeftMargin->setValue(lstyle->keysigLeftMargin.val());
      timesigLeftMargin->setValue(lstyle->timesigLeftMargin.val());
      clefKeyRightMargin->setValue(lstyle->clefKeyRightMargin.val());

      beamWidth->setValue(lstyle->beamWidth.val());
      beamDistance->setValue(lstyle->beamDistance);
      beamMinLen->setValue(lstyle->beamMinLen.val());
      beamMinSlope->setValue(lstyle->beamMinSlope);
      beamMaxSlope->setValue(lstyle->beamMaxSlope);

      graceNoteSize->setValue(lstyle->graceNoteMag * 100.0);
      smallStaffSize->setValue(lstyle->smallStaffMag * 100.0);
      smallNoteSize->setValue(lstyle->smallNoteMag * 100.0);
      smallClefSize->setValue(lstyle->smallClefMag * 100.0);
      pageFillThreshold->setValue(lstyle->pageFillLimit * 100.0);
      lastSystemFillThreshold->setValue(lstyle->lastSystemFillLimit * 100.0);

      hairpinLineWidth->setValue(lstyle->hairpinWidth.val());
      hairpinHeight->setValue(lstyle->hairpinHeight.val());
      hairpinContinueHeight->setValue(lstyle->hairpinContHeight.val());
      }

