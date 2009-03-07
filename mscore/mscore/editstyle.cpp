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
#include "articulation.h"

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

EditStyle::EditStyle(Score* s, QWidget* parent)
   : QDialog(parent), cs(s), lstyle(s->style())
      {
      setModal(true);
      setupUi(this);

      stemGroups[0] = new QButtonGroup(this);
      stemGroups[0]->addButton(voice1Up);
      stemGroups[0]->addButton(voice1Down);

      stemGroups[1] = new QButtonGroup(this);
      stemGroups[1]->addButton(voice2Up);
      stemGroups[1]->addButton(voice2Down);

      stemGroups[2] = new QButtonGroup(this);
      stemGroups[2]->addButton(voice3Up);
      stemGroups[2]->addButton(voice3Down);

      stemGroups[3] = new QButtonGroup(this);
      stemGroups[3]->addButton(voice4Up);
      stemGroups[3]->addButton(voice4Down);

      pageList->setCurrentRow(0);
      connect(buttonOk, SIGNAL(clicked()), this, SLOT(ok()));
      connect(buttonApply, SIGNAL(clicked()), this, SLOT(apply()));

      articulationTable->verticalHeader()->setVisible(false);
      QStringList headers;
      headers << tr("Articulation") << tr("Anchor");
      articulationTable->setHorizontalHeaderLabels(headers);
      QStringList ci;
      ci << tr("TopStaff") << tr("BottomStaff") << tr("Chord") << tr("TopChord")
         << tr("BottomChord");

      for (int i = 0; i < ARTICULATIONS; ++i) {
            ArticulationInfo* ai = &Articulation::articulationList[i];
            QTableWidgetItem* item = new QTableWidgetItem(ai->name);
            articulationTable->setItem(i, 0, item);
            QComboBox* cb = new QComboBox();
            cb->addItems(ci);
            cb->setCurrentIndex(int(ai->anchor));
            articulationTable->setCellWidget(i, 1, cb);
            }
      setValues();
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
      if (cs->styleB(ST_concertPitch) != lstyle[ST_concertPitch].toBool()) {
            cs->startCmd();
            cs->cmdConcertPitchChanged(lstyle[ST_concertPitch].toBool());
            cs->endCmd();
            }
      cs->setStyle(lstyle);
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
      lstyle[ST_staffUpperBorder]        = StyleVal(Spatium(staffUpperBorder->value()));
      lstyle[ST_staffLowerBorder]        = StyleVal(Spatium(staffLowerBorder->value()));
      lstyle[ST_staffDistance]           = StyleVal(Spatium(staffDistance->value()));
      lstyle[ST_akkoladeDistance]        = StyleVal(Spatium(akkoladeDistance->value()));
      lstyle[ST_systemDistance]          = StyleVal(Spatium(systemDistance->value()));
      lstyle[ST_lyricsDistance]          = StyleVal(Spatium(lyricsDistance->value()));
      lstyle[ST_lyricsMinBottomDistance] = StyleVal(Spatium(lyricsMinBottomDistance->value()));
      lstyle[ST_systemFrameDistance]     = StyleVal(Spatium(systemFrameDistance->value()));
      lstyle[ST_frameSystemDistance]     = StyleVal(Spatium(frameSystemDistance->value()));
      lstyle[ST_bracketDistance]         = StyleVal(Spatium(bracketDistance->value()));
      lstyle[ST_measureSpacing]          = StyleVal(measureSpacing->value());
      lstyle[ST_minNoteDistance]         = StyleVal(Spatium(minNoteDistance->value()));
      lstyle[ST_barNoteDistance]         = StyleVal(Spatium(barNoteDistance->value()));
      lstyle[ST_noteBarDistance]         = StyleVal(Spatium(noteBarDistance->value()));
      lstyle[ST_showPageNumber]          = StyleVal(showPageNumber->isChecked());
      lstyle[ST_showPageNumberOne]       = StyleVal(showFirstPageNumber->isChecked());
      lstyle[ST_pageNumberOddEven]       = StyleVal(showOddEvenPageNumber->isChecked());
      lstyle[ST_showMeasureNumber]       = StyleVal(showMeasureNumber->isChecked());
      lstyle[ST_showMeasureNumberOne]    = StyleVal(showFirstMeasureNumber->isChecked());
      lstyle[ST_measureNumberInterval]   = StyleVal(intervalMeasureNumber->value());
      lstyle[ST_measureNumberSystem]     = StyleVal(showEverySystemMeasureNumber->isChecked());
      lstyle[ST_measureNumberAllStaffs]  = StyleVal(showAllStaffsMeasureNumber->isChecked());
      lstyle[ST_clefLeftMargin]          = StyleVal(Spatium(clefLeftMargin->value()));
      lstyle[ST_keysigLeftMargin]        = StyleVal(Spatium(keysigLeftMargin->value()));
      lstyle[ST_timesigLeftMargin]       = StyleVal(Spatium(timesigLeftMargin->value()));
      lstyle[ST_clefKeyRightMargin]      = StyleVal(Spatium(clefKeyRightMargin->value()));
      lstyle[ST_beginRepeatLeftMargin]   = StyleVal(Spatium(beginRepeatLeftMargin->value()));
      lstyle[ST_beamWidth]               = StyleVal(Spatium(beamWidth->value()));
      lstyle[ST_beamDistance]            = StyleVal(beamDistance->value());
      lstyle[ST_beamMinLen]              = StyleVal(Spatium(beamMinLen->value()));
      lstyle[ST_beamMinSlope]            = StyleVal(beamMinSlope->value());
      lstyle[ST_beamMaxSlope]            = StyleVal(beamMaxSlope->value());
      lstyle[ST_graceNoteMag]            = StyleVal(graceNoteSize->value() * 0.01);
      lstyle[ST_smallStaffMag]           = StyleVal(smallStaffSize->value() * 0.01);
      lstyle[ST_smallNoteMag]            = StyleVal(smallNoteSize->value() * 0.01);
      lstyle[ST_smallClefMag]            = StyleVal(smallClefSize->value() * 0.01);
      lstyle[ST_pageFillLimit]           = StyleVal(pageFillThreshold->value() * 0.01);
      lstyle[ST_lastSystemFillLimit]     = StyleVal(lastSystemFillThreshold->value() * 0.01);
      lstyle[ST_hairpinWidth]            = StyleVal(Spatium(hairpinLineWidth->value()));
      lstyle[ST_hairpinHeight]           = StyleVal(Spatium(hairpinHeight->value()));
      lstyle[ST_hairpinContHeight]       = StyleVal(Spatium(hairpinContinueHeight->value()));
      lstyle[ST_genClef]                 = StyleVal(genClef->isChecked());
      lstyle[ST_genKeysig]               = StyleVal(genKeysig->isChecked());
      lstyle[ST_genTimesig]              = StyleVal(genTimesig->isChecked());
      lstyle[ST_genCourtesyTimesig]      = StyleVal(genCourtesyTimesig->isChecked());
      lstyle[ST_useGermanNoteNames]      = StyleVal(useGermanNoteNames->isChecked());
      lstyle[ST_chordNamesUseSymbols]    = StyleVal(chordNamesUseSymbols->isChecked());
      lstyle[ST_concertPitch]            = StyleVal(concertPitch->isChecked());
      lstyle[ST_createMultiMeasureRests] = StyleVal(multiMeasureRests->isChecked());
      lstyle[ST_minEmptyMeasures]        = StyleVal(minEmptyMeasures->value());
      lstyle[ST_minMMRestWidth]          = StyleVal(Spatium(minMeasureWidth->value()));
      lstyle[ST_hideEmptyStaves]         = StyleVal(hideEmptyStaves->isChecked());
      lstyle[ST_dotNoteDistance]         = StyleVal(Spatium(noteDotDistance->value()));
      lstyle[ST_dotDotDistance]          = StyleVal(Spatium(dotDotDistance->value()));
      lstyle[ST_ledgerLineWidth]         = StyleVal(Spatium(ledgerLineWidth->value()));
      lstyle[ST_propertyDistanceHead]    = StyleVal(Spatium(propertyDistanceHead->value()));
      lstyle[ST_propertyDistanceStem]    = StyleVal(Spatium(propertyDistanceStem->value()));
      lstyle[ST_propertyDistance]        = StyleVal(Spatium(propertyDistance->value()));
      lstyle[ST_stemDir1]                = StyleVal(voice1Up->isChecked() ? UP : DOWN);
      lstyle[ST_stemDir2]                = StyleVal(voice2Up->isChecked() ? UP : DOWN);
      lstyle[ST_stemDir3]                = StyleVal(voice3Up->isChecked() ? UP : DOWN);
      lstyle[ST_stemDir4]                = StyleVal(voice4Up->isChecked() ? UP : DOWN);
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void EditStyle::setValues()
      {
      staffUpperBorder->setValue(lstyle[ST_staffUpperBorder].toSpatium().val());
      staffLowerBorder->setValue(lstyle[ST_staffLowerBorder].toSpatium().val());
      staffDistance->setValue(lstyle[ST_staffDistance].toSpatium().val());
      akkoladeDistance->setValue(lstyle[ST_akkoladeDistance].toSpatium().val());
      systemDistance->setValue(lstyle[ST_systemDistance].toSpatium().val());
      lyricsDistance->setValue(lstyle[ST_lyricsDistance].toSpatium().val());
      lyricsMinBottomDistance->setValue(lstyle[ST_lyricsMinBottomDistance].toSpatium().val());
      systemFrameDistance->setValue(lstyle[ST_systemFrameDistance].toSpatium().val());
      frameSystemDistance->setValue(lstyle[ST_frameSystemDistance].toSpatium().val());
      bracketDistance->setValue(lstyle[ST_bracketDistance].toSpatium().val());

      measureSpacing->setValue(lstyle[ST_measureSpacing].toDouble());
      minNoteDistance->setValue(lstyle[ST_minNoteDistance].toSpatium().val());
      barNoteDistance->setValue(lstyle[ST_barNoteDistance].toSpatium().val());
      noteBarDistance->setValue(lstyle[ST_noteBarDistance].toSpatium().val());

      showPageNumber->setChecked(lstyle[ST_showPageNumber].toBool());
      showFirstPageNumber->setChecked(lstyle[ST_showPageNumberOne].toBool());
      showOddEvenPageNumber->setChecked(lstyle[ST_pageNumberOddEven].toBool());
      showMeasureNumber->setChecked(lstyle[ST_showMeasureNumber].toBool());
      showFirstMeasureNumber->setChecked(lstyle[ST_showMeasureNumberOne].toBool());
      intervalMeasureNumber->setValue(lstyle[ST_measureNumberInterval].toInt());
      showIntervalMeasureNumber->setChecked(!lstyle[ST_measureNumberSystem].toBool());
      showAllStaffsMeasureNumber->setChecked(lstyle[ST_measureNumberAllStaffs].toBool());
      showEverySystemMeasureNumber->setChecked(lstyle[ST_measureNumberSystem].toBool());


      clefLeftMargin->setValue(lstyle[ST_clefLeftMargin].toSpatium().val());
      keysigLeftMargin->setValue(lstyle[ST_keysigLeftMargin].toSpatium().val());
      timesigLeftMargin->setValue(lstyle[ST_timesigLeftMargin].toSpatium().val());
      clefKeyRightMargin->setValue(lstyle[ST_clefKeyRightMargin].toSpatium().val());
      beginRepeatLeftMargin->setValue(lstyle[ST_beginRepeatLeftMargin].toSpatium().val());

      beamWidth->setValue(lstyle[ST_beamWidth].toSpatium().val());
      beamDistance->setValue(lstyle[ST_beamDistance].toDouble());
      beamMinLen->setValue(lstyle[ST_beamMinLen].toSpatium().val());
      beamMinSlope->setValue(lstyle[ST_beamMinSlope].toDouble());
      beamMaxSlope->setValue(lstyle[ST_beamMaxSlope].toDouble());

      graceNoteSize->setValue(lstyle[ST_graceNoteMag].toDouble() * 100.0);
      smallStaffSize->setValue(lstyle[ST_smallStaffMag].toDouble() * 100.0);
      smallNoteSize->setValue(lstyle[ST_smallNoteMag].toDouble() * 100.0);
      smallClefSize->setValue(lstyle[ST_smallClefMag].toDouble() * 100.0);
      pageFillThreshold->setValue(lstyle[ST_pageFillLimit].toDouble() * 100.0);
      lastSystemFillThreshold->setValue(lstyle[ST_lastSystemFillLimit].toDouble() * 100.0);

      hairpinLineWidth->setValue(lstyle[ST_hairpinWidth].toSpatium().val());
      hairpinHeight->setValue(lstyle[ST_hairpinHeight].toSpatium().val());
      hairpinContinueHeight->setValue(lstyle[ST_hairpinContHeight].toSpatium().val());

      genClef->setChecked(lstyle[ST_genClef].toBool());
      genKeysig->setChecked(lstyle[ST_genKeysig].toBool());
      genTimesig->setChecked(lstyle[ST_genTimesig].toBool());
      genCourtesyTimesig->setChecked(lstyle[ST_genCourtesyTimesig].toBool());

      useGermanNoteNames->setChecked(lstyle[ST_useGermanNoteNames].toBool());
      chordNamesUseSymbols->setChecked(lstyle[ST_chordNamesUseSymbols].toBool());
      concertPitch->setChecked(lstyle[ST_concertPitch].toBool());

      multiMeasureRests->setChecked(lstyle[ST_createMultiMeasureRests].toBool());
      minEmptyMeasures->setValue(lstyle[ST_minEmptyMeasures].toInt());
      minMeasureWidth->setValue(lstyle[ST_minMMRestWidth].toSpatium().val());
      hideEmptyStaves->setChecked(lstyle[ST_hideEmptyStaves].toBool());

      noteDotDistance->setValue(lstyle[ST_dotNoteDistance].toSpatium().val());
      dotDotDistance->setValue(lstyle[ST_dotDotDistance].toSpatium().val());
      ledgerLineWidth->setValue(lstyle[ST_ledgerLineWidth].toSpatium().val());

      propertyDistanceHead->setValue(lstyle[ST_propertyDistanceHead].toSpatium().val());
      propertyDistanceStem->setValue(lstyle[ST_propertyDistanceStem].toSpatium().val());
      propertyDistance->setValue(lstyle[ST_propertyDistance].toSpatium().val());

      voice1Up->setChecked(lstyle[ST_stemDir1].toBool() == UP);
      voice2Up->setChecked(lstyle[ST_stemDir2].toBool() == UP);
      voice3Up->setChecked(lstyle[ST_stemDir3].toBool() == UP);
      voice4Up->setChecked(lstyle[ST_stemDir4].toBool() == UP);

      voice1Down->setChecked(lstyle[ST_stemDir1].toBool() != UP);
      voice2Down->setChecked(lstyle[ST_stemDir2].toBool() != UP);
      voice3Down->setChecked(lstyle[ST_stemDir3].toBool() != UP);
      voice4Down->setChecked(lstyle[ST_stemDir4].toBool() != UP);
      }

