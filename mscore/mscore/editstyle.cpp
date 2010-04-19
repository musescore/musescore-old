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

#include "score.h"
#include "scoreview.h"
#include "style.h"
#include "editstyle.h"
#include "articulation.h"
#include "sym.h"
#include "icons.h"
#include "mscore.h"
#include "undo.h"

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

      articulationTable->verticalHeader()->setVisible(false);
      articulationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
      QStringList headers;
      headers << tr("Symbol") << tr("Anchor");
      articulationTable->setHorizontalHeaderLabels(headers);
      articulationTable->setColumnWidth(0, 200);
      articulationTable->setColumnWidth(1, 180);
      articulationTable->setRowCount(ARTICULATIONS);

      for (int i = 0; i < ARTICULATIONS; ++i) {
            ArticulationInfo* ai = &Articulation::articulationList[i];

            QTableWidgetItem* item = new QTableWidgetItem(*symIcon(symbols[ai->sym], 50, 25, 25), qApp->translate("articulation", qPrintable(ai->name)));
            item->setFlags(item->flags() & ~Qt::ItemIsEditable);
            articulationTable->setItem(i, 0, item);

            QComboBox* cb = new QComboBox();
            cb->addItem(tr("TopStaff"), A_TOP_STAFF);
            cb->addItem(tr("BottomStaff"), A_BOTTOM_STAFF);
            cb->addItem(tr("Chord"), A_CHORD);
            articulationTable->setCellWidget(i, 1, cb);
            }
      setValues();
      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(chordDescriptionFileButton, SIGNAL(clicked()), SLOT(selectChordDescriptionFile()));
      }

//---------------------------------------------------------
//   buttonClicked
//---------------------------------------------------------

void EditStyle::buttonClicked(QAbstractButton* b)
      {
      switch (buttonBox->standardButton(b)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
                  done(1);
                  break;
            default:
                  cs->undo()->current()->unwind();
                  cs->setLayoutAll(true);
                  done(0);
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStyle::apply()
      {
      getValues();
      if (cs->style(ST_chordDescriptionFile).toString() != lstyle[ST_chordDescriptionFile].toString())
            lstyle.clearChordList();
      cs->undo()->push(new ChangeStyle(cs, lstyle));
      cs->setLayoutAll(true);
      cs->end();
      }

//---------------------------------------------------------
//   getValues
//---------------------------------------------------------

void EditStyle::getValues()
      {
      lstyle.set(ST_staffUpperBorder,        Spatium(staffUpperBorder->value()));
      lstyle.set(ST_staffLowerBorder,        Spatium(staffLowerBorder->value()));
      lstyle.set(ST_staffDistance,           Spatium(staffDistance->value()));
      lstyle.set(ST_akkoladeDistance,        Spatium(akkoladeDistance->value()));
      lstyle.set(ST_systemDistance,          Spatium(systemDistance->value()));
      lstyle.set(ST_lyricsDistance,          Spatium(lyricsDistance->value()));
      lstyle.set(ST_lyricsMinBottomDistance, Spatium(lyricsMinBottomDistance->value()));
      lstyle.set(ST_systemFrameDistance,     Spatium(systemFrameDistance->value()));
      lstyle.set(ST_frameSystemDistance,     Spatium(frameSystemDistance->value()));

      lstyle.set(ST_repeatBarTips,           showRepeatBarTips->isChecked());
      lstyle.set(ST_startBarlineSingle,      showStartBarlineSingle->isChecked());
      lstyle.set(ST_startBarlineMultiple,    showStartBarlineMultiple->isChecked());

      lstyle.set(ST_bracketDistance,         Spatium(bracketDistance->value()));
      lstyle.set(ST_measureSpacing,          measureSpacing->value());
      lstyle.set(ST_minNoteDistance,         Spatium(minNoteDistance->value()));
      lstyle.set(ST_barNoteDistance,         Spatium(barNoteDistance->value()));
      lstyle.set(ST_noteBarDistance,         Spatium(noteBarDistance->value()));
      lstyle.set(ST_showPageNumber,          showPageNumber->isChecked());
      lstyle.set(ST_showPageNumberOne,       showFirstPageNumber->isChecked());
      lstyle.set(ST_pageNumberOddEven,       showOddEvenPageNumber->isChecked());
      lstyle.set(ST_showMeasureNumber,       showMeasureNumber->isChecked());
      lstyle.set(ST_showMeasureNumberOne,    showFirstMeasureNumber->isChecked());
      lstyle.set(ST_measureNumberInterval,   intervalMeasureNumber->value());
      lstyle.set(ST_measureNumberSystem,     showEverySystemMeasureNumber->isChecked());
      lstyle.set(ST_measureNumberAllStaffs,  showAllStaffsMeasureNumber->isChecked());
      lstyle.set(ST_clefLeftMargin,          Spatium(clefLeftMargin->value()));
      lstyle.set(ST_keysigLeftMargin,        Spatium(keysigLeftMargin->value()));
      lstyle.set(ST_timesigLeftMargin,       Spatium(timesigLeftMargin->value()));
      lstyle.set(ST_clefKeyRightMargin,      Spatium(clefKeyRightMargin->value()));
      lstyle.set(ST_clefBarlineDistance,     Spatium(clefBarlineDistance->value()));
      lstyle.set(ST_beginRepeatLeftMargin,   Spatium(beginRepeatLeftMargin->value()));
      lstyle.set(ST_beamWidth,               Spatium(beamWidth->value()));
      lstyle.set(ST_beamDistance,            beamDistance->value());
      lstyle.set(ST_beamMinLen,              Spatium(beamMinLen->value()));
      lstyle.set(ST_beamMinSlope,            beamMinSlope->value());
      lstyle.set(ST_beamMaxSlope,            beamMaxSlope->value());
      lstyle.set(ST_graceNoteMag,            graceNoteSize->value() * 0.01);
      lstyle.set(ST_smallStaffMag,           smallStaffSize->value() * 0.01);
      lstyle.set(ST_smallNoteMag,            smallNoteSize->value() * 0.01);
      lstyle.set(ST_smallClefMag,            smallClefSize->value() * 0.01);
      lstyle.set(ST_pageFillLimit,           pageFillThreshold->value() * 0.01);
      lstyle.set(ST_lastSystemFillLimit,     lastSystemFillThreshold->value() * 0.01);
      lstyle.set(ST_hairpinWidth,            Spatium(hairpinLineWidth->value()));
      lstyle.set(ST_hairpinHeight,           Spatium(hairpinHeight->value()));
      lstyle.set(ST_hairpinContHeight,       Spatium(hairpinContinueHeight->value()));
      lstyle.set(ST_genClef,                 genClef->isChecked());
      lstyle.set(ST_genKeysig,               genKeysig->isChecked());
      lstyle.set(ST_genTimesig,              genTimesig->isChecked());
      lstyle.set(ST_genCourtesyTimesig,      genCourtesyTimesig->isChecked());
      lstyle.set(ST_genCourtesyKeysig,       genCourtesyKeysig->isChecked());

      lstyle.set(ST_chordDescriptionFile,   chordDescriptionFile->text());

      lstyle.set(ST_concertPitch,            concertPitch->isChecked());
      lstyle.set(ST_createMultiMeasureRests, multiMeasureRests->isChecked());
      lstyle.set(ST_minEmptyMeasures,        minEmptyMeasures->value());
      lstyle.set(ST_minMMRestWidth,          Spatium(minMeasureWidth->value()));
      lstyle.set(ST_hideEmptyStaves,         hideEmptyStaves->isChecked());

      lstyle.set(ST_accidentalNoteDistance,  Spatium(accidentalNoteDistance->value()));
      lstyle.set(ST_accidentalDistance,      Spatium(accidentalDistance->value()));
      lstyle.set(ST_dotNoteDistance,         Spatium(noteDotDistance->value()));
      lstyle.set(ST_dotDotDistance,          Spatium(dotDotDistance->value()));
      lstyle.set(ST_ledgerLineWidth,         Spatium(ledgerLineWidth->value()));

      lstyle.set(ST_propertyDistanceHead,    Spatium(propertyDistanceHead->value()));
      lstyle.set(ST_propertyDistanceStem,    Spatium(propertyDistanceStem->value()));
      lstyle.set(ST_propertyDistance,        Spatium(propertyDistance->value()));
      lstyle.set(ST_stemDir1,                voice1Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir2,                voice2Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir3,                voice3Up->isChecked() ? UP : DOWN);
      lstyle.set(ST_stemDir4,                voice4Up->isChecked() ? UP : DOWN);

      lstyle.set(ST_shortenStem,             shortenStem->isChecked());
      lstyle.set(ST_shortStemProgression,    Spatium(shortStemProgression->value()));
      lstyle.set(ST_shortestStem,            Spatium(shortestStem->value()));

      lstyle.set(ST_ArpeggioNoteDistance,    Spatium(arpeggioNoteDistance->value()));
      lstyle.set(ST_ArpeggioLineWidth,       Spatium(arpeggioLineWidth->value()));
      lstyle.set(ST_ArpeggioHookLen,         Spatium(arpeggioHookLen->value()));

      lstyle.set(ST_FixMeasureNumbers,       fixNumberMeasures->value());
      lstyle.set(ST_FixMeasureWidth,         fixMeasureWidth->isChecked());

      for (int i = 0; i < ARTICULATIONS; ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            lstyle.set(StyleIdx(ST_UfermataAnchor + i), cb->itemData(cb->currentIndex()).toInt());
            }
//      lstyle.set(ST_warnPitchRange,  warnPitchRange->isChecked());
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

      showRepeatBarTips->setChecked(lstyle[ST_repeatBarTips].toBool());
      showStartBarlineSingle->setChecked(lstyle[ST_startBarlineSingle].toBool());
      showStartBarlineMultiple->setChecked(lstyle[ST_startBarlineMultiple].toBool());

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
      clefBarlineDistance->setValue(lstyle[ST_clefBarlineDistance].toSpatium().val());
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
      genCourtesyKeysig->setChecked(lstyle[ST_genCourtesyKeysig].toBool());

      useGermanNoteNames->setChecked(lstyle[ST_useGermanNoteNames].toBool());
      QString s(lstyle[ST_chordDescriptionFile].toString());
      chordDescriptionFile->setText(s);
      concertPitch->setChecked(lstyle[ST_concertPitch].toBool());

      multiMeasureRests->setChecked(lstyle[ST_createMultiMeasureRests].toBool());
      minEmptyMeasures->setValue(lstyle[ST_minEmptyMeasures].toInt());
      minMeasureWidth->setValue(lstyle[ST_minMMRestWidth].toSpatium().val());
      hideEmptyStaves->setChecked(lstyle[ST_hideEmptyStaves].toBool());

      accidentalNoteDistance->setValue(lstyle[ST_accidentalNoteDistance].toSpatium().val());
      accidentalDistance->setValue(lstyle[ST_accidentalDistance].toSpatium().val());
      noteDotDistance->setValue(lstyle[ST_dotNoteDistance].toSpatium().val());
      dotDotDistance->setValue(lstyle[ST_dotDotDistance].toSpatium().val());
      ledgerLineWidth->setValue(lstyle[ST_ledgerLineWidth].toSpatium().val());

      propertyDistanceHead->setValue(lstyle[ST_propertyDistanceHead].toSpatium().val());
      propertyDistanceStem->setValue(lstyle[ST_propertyDistanceStem].toSpatium().val());
      propertyDistance->setValue(lstyle[ST_propertyDistance].toSpatium().val());

      voice1Up->setChecked(lstyle[ST_stemDir1].toDirection() == UP);
      voice2Up->setChecked(lstyle[ST_stemDir2].toDirection() == UP);
      voice3Up->setChecked(lstyle[ST_stemDir3].toDirection() == UP);
      voice4Up->setChecked(lstyle[ST_stemDir4].toDirection() == UP);

      voice1Down->setChecked(lstyle[ST_stemDir1].toDirection() != UP);
      voice2Down->setChecked(lstyle[ST_stemDir2].toDirection() != UP);
      voice3Down->setChecked(lstyle[ST_stemDir3].toDirection() != UP);
      voice4Down->setChecked(lstyle[ST_stemDir4].toDirection() != UP);

      shortenStem->setChecked(lstyle[ST_shortenStem].toBool());
      shortStemProgression->setValue(lstyle[ST_shortStemProgression].toSpatium().val());
      shortestStem->setValue(lstyle[ST_shortestStem].toSpatium().val());
      arpeggioNoteDistance->setValue(lstyle[ST_ArpeggioNoteDistance].toSpatium().val());
      arpeggioLineWidth->setValue(lstyle[ST_ArpeggioLineWidth].toSpatium().val());
      arpeggioHookLen->setValue(lstyle[ST_ArpeggioHookLen].toSpatium().val());

      for (int i = 0; i < ARTICULATIONS; ++i) {
            QComboBox* cb = static_cast<QComboBox*>(articulationTable->cellWidget(i, 1));
            if (cb == 0)
                  continue;
            int st  = lstyle[StyleIdx(ST_UfermataAnchor + i)].toInt();
            int idx = 0;
            if (st == A_TOP_STAFF)
                  idx = 0;
            else if (st == A_BOTTOM_STAFF)
                  idx = 1;
            else if (st == A_CHORD)
                  idx = 2;
            cb->setCurrentIndex(idx);
            }
//      warnPitchRange->setChecked(lstyle[ST_warnPitchRange].toBool());

      fixNumberMeasures->setValue(lstyle[ST_FixMeasureNumbers].toInt());
      fixMeasureWidth->setChecked(lstyle[ST_FixMeasureWidth].toBool());
      }

//---------------------------------------------------------
//   selectChordDescriptionFile
//---------------------------------------------------------

void EditStyle::selectChordDescriptionFile()
      {
      QString path = QString("%1styles/%2").arg(mscoreGlobalShare).arg(chordDescriptionFile->text());
      QString fn = QFileDialog::getOpenFileName(
         0, QWidget::tr("MuseScore: Load Chord Description"),
         path,
         QWidget::tr("MuseScore Chord Description (*.xml);;All Files (*)")
         );
      if (fn.isEmpty())
            return;
      QFileInfo fi(fn);
      chordDescriptionFile->setText(fi.fileName());
      }

