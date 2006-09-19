//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: editstyle.cpp,v 1.10 2006/03/02 17:08:33 wschweer Exp $
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
      setupUi(this);
      pageList->setCurrentRow(0);
      connect(buttonOk, SIGNAL(clicked()), this, SLOT(ok()));
      connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));
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
      ::style->staffUpperBorder = Spatium(staffUpperBorder->value());
      ::style->staffLowerBorder = Spatium(staffLowerBorder->value());
      ::style->staffDistance    = Spatium(staffDistance->value());
      ::style->accoladeDistance = Spatium(akkoladeDistance->value());
      ::style->systemDistance   = Spatium(systemDistance->value());
      ::style->measureSpacing   = measureSpacing->value();
      ::style->minNoteDistance  = Spatium(minNoteDistance->value());
      ::style->barNoteDistance  = Spatium(barNoteDistance->value());
      ::style->noteBarDistance  = Spatium(noteBarDistance->value());

      ::style->showPageNumber   = showPageNumber->isChecked();
      ::style->showPageNumberOne = showFirstPageNumber->isChecked();
      ::style->pageNumberOddEven = showOddEvenPageNumber->isChecked();
      ::style->showMeasureNumber = showMeasureNumber->isChecked();
      ::style->showMeasureNumberOne = showFirstMeasureNumber->isChecked();
      ::style->measureNumberInterval = intervalMeasureNumber->value();
      ::style->measureNumberSystem = showEverySystemMeasureNumber->isChecked();
      ::style->measureNumberAllStaffs = showAllStaffsMeasureNumber->isChecked();
      cs->layout();
      cs->endCmd(false);
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues(Score* s)
      {
      cs = s;
      staffUpperBorder->setValue(::style->staffUpperBorder.val());
      staffLowerBorder->setValue(::style->staffLowerBorder.val());
      staffDistance->setValue(::style->staffDistance.val());
      akkoladeDistance->setValue(::style->accoladeDistance.val());
      systemDistance->setValue(::style->systemDistance.val());
      measureSpacing->setValue(::style->measureSpacing);
      minNoteDistance->setValue(::style->minNoteDistance.val());
      barNoteDistance->setValue(::style->barNoteDistance.val());
      noteBarDistance->setValue(::style->noteBarDistance.val());

      showPageNumber->setChecked(::style->showPageNumber);
      showFirstPageNumber->setChecked(::style->showPageNumberOne);
      showOddEvenPageNumber->setChecked(::style->pageNumberOddEven);
      showMeasureNumber->setChecked(::style->showMeasureNumber);
      showFirstMeasureNumber->setChecked(::style->showMeasureNumberOne);
      intervalMeasureNumber->setValue(::style->measureNumberInterval);
      showIntervalMeasureNumber->setChecked(!::style->measureNumberSystem);
      showAllStaffsMeasureNumber->setChecked(::style->measureNumberAllStaffs);
      showEverySystemMeasureNumber->setChecked(::style->measureNumberSystem);
      }
