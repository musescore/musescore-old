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

extern QString iconPath;

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

            QIcon icon(iconPath + ai->name + ".svg");
            QTableWidgetItem* item = new QTableWidgetItem(icon, qApp->translate("articulation", qPrintable(ai->name)));
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
      connect(editEvenHeader, SIGNAL(clicked()), SLOT(editEvenHeaderClicked()));
      connect(editOddHeader,  SIGNAL(clicked()), SLOT(editOddHeaderClicked()));
      connect(editEvenFooter, SIGNAL(clicked()), SLOT(editEvenFooterClicked()));
      connect(editOddFooter,  SIGNAL(clicked()), SLOT(editOddFooterClicked()));
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
      lstyle.set(ST_staffLineWidth,          Spatium(staffLineWidth->value()));
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
      lstyle.set(ST_genCourtesyClef,         genCourtesyClef->isChecked());

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

      lstyle.set(ST_SlurEndWidth,            Spatium(slurEndLineWidth->value()));
      lstyle.set(ST_SlurMidWidth,            Spatium(slurMidLineWidth->value()));
      lstyle.set(ST_SlurDottedWidth,         Spatium(slurDottedLineWidth->value()));
      lstyle.set(ST_SlurBow,                 Spatium(slurBow->value()));

      lstyle.set(ST_MusicalSymbolFont,       musicalSymbolFont->currentText());

      lstyle.set(ST_showHeader,              showHeader->isChecked());
      lstyle.set(ST_headerFirstPage,         showHeaderFirstPage->isChecked());
      lstyle.set(ST_headerOddEven,           headerOddEven->isChecked());
      lstyle.set(ST_evenHeader,              evenHeader->toHtml());
      lstyle.set(ST_oddHeader,               oddHeader->toHtml());

      lstyle.set(ST_showFooter,              showFooter->isChecked());
      lstyle.set(ST_footerFirstPage,         showFooterFirstPage->isChecked());
      lstyle.set(ST_footerOddEven,           footerOddEven->isChecked());
      lstyle.set(ST_evenFooter,              evenFooter->toHtml());
      lstyle.set(ST_oddFooter,               oddFooter->toHtml());

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
      staffLineWidth->setValue(lstyle[ST_staffLineWidth].toSpatium().val());

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
      genCourtesyClef->setChecked(lstyle[ST_genCourtesyClef].toBool());

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

      slurEndLineWidth->setValue(lstyle[ST_SlurEndWidth].toSpatium().val());
      slurMidLineWidth->setValue(lstyle[ST_SlurMidWidth].toSpatium().val());
      slurDottedLineWidth->setValue(lstyle[ST_SlurDottedWidth].toSpatium().val());
      slurBow->setValue(lstyle[ST_SlurBow].toSpatium().val());
//      ST_SectionPause,
      musicalSymbolFont->setCurrentIndex(lstyle[ST_MusicalSymbolFont].toString() == "Emmentaler" ? 0 : 1);

      showHeader->setChecked(lstyle[ST_showHeader].toBool());
      showHeaderFirstPage->setChecked(lstyle[ST_headerFirstPage].toBool());
      headerOddEven->setChecked(lstyle[ST_headerOddEven].toBool());
      evenHeader->setHtml(lstyle[ST_evenHeader].toString());
      oddHeader->setHtml(lstyle[ST_oddHeader].toString());

      showFooter->setChecked(lstyle[ST_showFooter].toBool());
      showFooterFirstPage->setChecked(lstyle[ST_footerFirstPage].toBool());
      footerOddEven->setChecked(lstyle[ST_footerOddEven].toBool());
      evenFooter->setHtml(lstyle[ST_evenFooter].toString());
      oddFooter->setHtml(lstyle[ST_oddFooter].toString());
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

//---------------------------------------------------------
//   editEvenHeaderClicked
//---------------------------------------------------------

void EditStyle::editEvenHeaderClicked()
      {
      QString s = editText(evenHeader->toHtml());
      evenHeader->setHtml(s);
      }

//---------------------------------------------------------
//   editOddHeaderClicked
//---------------------------------------------------------

void EditStyle::editOddHeaderClicked()
      {
      QString s = editText(oddHeader->toHtml());
      oddHeader->setHtml(s);
      }

//---------------------------------------------------------
//   editEvenFooterClicked
//---------------------------------------------------------

void EditStyle::editEvenFooterClicked()
      {
      QString s = editText(evenFooter->toHtml());
      evenFooter->setHtml(s);
      }

//---------------------------------------------------------
//   editOddFooterClicked
//---------------------------------------------------------

void EditStyle::editOddFooterClicked()
      {
      QString s = editText(oddFooter->toHtml());
      oddFooter->setHtml(s);
      }

//---------------------------------------------------------
//   TextEditor
//---------------------------------------------------------

TextEditor::TextEditor(QWidget* parent)
   : QDialog(parent)
      {
      QVBoxLayout* vl = new QVBoxLayout;
      setLayout(vl);
      QFrame* f = new QFrame;
      QHBoxLayout* hl = new QHBoxLayout;
      hl->setSpacing(0);
      f->setLayout(hl);

      typefaceBold = new QToolButton;
      typefaceBold->setIcon(*icons[textBold_ICON]);
      typefaceBold->setToolTip(tr("bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = new QToolButton;
      typefaceItalic->setIcon(*icons[textItalic_ICON]);
      typefaceItalic->setToolTip(tr("italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = new QToolButton;
      typefaceUnderline->setIcon(*icons[textUnderline_ICON]);
      typefaceUnderline->setToolTip(tr("underline"));
      typefaceUnderline->setCheckable(true);

      leftAlign   = new QToolButton;
      leftAlign->setIcon(*icons[textLeft_ICON]);
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);

      centerAlign = new QToolButton;
      centerAlign->setIcon(*icons[textCenter_ICON]);
      centerAlign->setToolTip(tr("align center"));
      centerAlign->setCheckable(true);

      rightAlign  = new QToolButton;
      rightAlign->setIcon(*icons[textRight_ICON]);
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);

      typefaceSubscript = new QToolButton;
      typefaceSubscript->setIcon(*icons[textSub_ICON]);
      typefaceSubscript->setToolTip(tr("subscript"));
      typefaceSubscript->setCheckable(true);

      typefaceSuperscript = new QToolButton;
      typefaceSuperscript->setIcon(*icons[textSuper_ICON]);
      typefaceSuperscript->setToolTip(tr("superscript"));
      typefaceSuperscript->setCheckable(true);

      typefaceSize = new QDoubleSpinBox(this);
      typefaceFamily = new QFontComboBox(this);

      hl->addWidget(typefaceBold);
      hl->addWidget(typefaceItalic);
      hl->addWidget(typefaceUnderline);
      hl->addStretch(10);
      hl->addWidget(leftAlign);
      hl->addWidget(centerAlign);
      hl->addWidget(rightAlign);
      hl->addStretch(10);
      hl->addWidget(typefaceSubscript);
      hl->addWidget(typefaceSuperscript);
      hl->addStretch(10);
      hl->addWidget(typefaceFamily);
      hl->addWidget(typefaceSize);
      hl->addStretch(10);

      vl->addWidget(f);
      edit = new QTextEdit;
      vl->addWidget(edit);

      charFormatChanged(edit->currentCharFormat());
      cursorPositionChanged();

      connect(typefaceBold,        SIGNAL(clicked(bool)), SLOT(toggleBold(bool)));
      connect(typefaceItalic,      SIGNAL(clicked(bool)), SLOT(toggleItalic(bool)));
      connect(typefaceUnderline,   SIGNAL(clicked(bool)), SLOT(toggleUnderline(bool)));
      connect(leftAlign,           SIGNAL(clicked(bool)), SLOT(toggleLeftAlign(bool)));
      connect(centerAlign,         SIGNAL(clicked(bool)), SLOT(toggleCenterAlign(bool)));
      connect(rightAlign,          SIGNAL(clicked(bool)), SLOT(toggleRightAlign(bool)));
      connect(typefaceSubscript,   SIGNAL(clicked(bool)), SLOT(toggleTypefaceSubscript(bool)));
      connect(typefaceSuperscript, SIGNAL(clicked(bool)), SLOT(toggleTypefaceSuperscript(bool)));
      connect(edit, SIGNAL(currentCharFormatChanged(const QTextCharFormat&)), SLOT(charFormatChanged(const QTextCharFormat&)));
      connect(edit, SIGNAL(cursorPositionChanged()), SLOT(cursorPositionChanged()));
      connect(typefaceSize,        SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextEditor::setText(const QString& txt)
      {
      edit->setHtml(txt);
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString TextEditor::text() const
      {
      return edit->toHtml();
      }

//---------------------------------------------------------
//   editText
//---------------------------------------------------------

QString EditStyle::editText(const QString& s)
      {
      TextEditor editor;
      editor.setText(s);

      editor.exec();

      return editor.text();
      }

//---------------------------------------------------------
//   toggleBold
//---------------------------------------------------------

void TextEditor::toggleBold(bool val)
      {
      edit->setFontWeight(val ? 75 : 50);
      }

//---------------------------------------------------------
//   toggleUnderline
//---------------------------------------------------------

void TextEditor::toggleUnderline(bool val)
      {
      edit->setFontUnderline(val);
      }

//---------------------------------------------------------
//   toggleItalic
//---------------------------------------------------------

void TextEditor::toggleItalic(bool val)
      {
      edit->setFontItalic(val);
      }

//---------------------------------------------------------
//   toggleLeftAlign
//---------------------------------------------------------

void TextEditor::toggleLeftAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);
      if (val) {
            //leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleCenterAlign
//---------------------------------------------------------

void TextEditor::toggleCenterAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(val ? Qt::AlignHCenter : Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);

      if (val) {
            leftAlign->setChecked(false);
            // centerAlign
            rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleRightAlign
//---------------------------------------------------------

void TextEditor::toggleRightAlign(bool val)
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      f.setAlignment(val ? Qt::AlignRight : Qt::AlignLeft);
      cursor.setBlockFormat(f);
      edit->setTextCursor(cursor);
      if (val) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            // rightAlign->setChecked(false);
            }
      }

//---------------------------------------------------------
//   toggleTypefaceSubscript
//---------------------------------------------------------

void TextEditor::toggleTypefaceSubscript(bool val)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setVerticalAlignment(val ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   toggleTypefaceSuperscript
//---------------------------------------------------------

void TextEditor::toggleTypefaceSuperscript(bool val)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setVerticalAlignment(val ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextEditor::sizeChanged(double size)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setFontPointSize(size);
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextEditor::fontChanged(const QFont& f)
      {
      QTextCharFormat cf = edit->currentCharFormat();
      cf.setFontFamily(f.family());
      edit->setCurrentCharFormat(cf);
      }

//---------------------------------------------------------
//   charFormatChanged
//---------------------------------------------------------

void TextEditor::charFormatChanged(const QTextCharFormat& f)
      {
      typefaceItalic->setChecked(f.fontItalic());
      typefaceUnderline->setChecked(f.fontUnderline());
      typefaceBold->setChecked(f.fontWeight() >= 75);
      if (f.verticalAlignment() == QTextCharFormat::AlignSuperScript) {
            typefaceSuperscript->setChecked(true);
            typefaceSubscript->setChecked(false);
            }
      else if (f.verticalAlignment() == QTextCharFormat::AlignSubScript) {
            typefaceSuperscript->setChecked(false);
            typefaceSubscript->setChecked(true);
            }
      else {
            typefaceSuperscript->setChecked(false);
            typefaceSubscript->setChecked(false);
            }
      typefaceSize->setValue(f.fontPointSize());
      typefaceFamily->setCurrentFont(f.font());
      }

//---------------------------------------------------------
//   cursorPositionChanged
//---------------------------------------------------------

void TextEditor::cursorPositionChanged()
      {
      QTextCursor cursor = edit->textCursor();
      QTextBlockFormat f = cursor.blockFormat();
      if (f.alignment() & Qt::AlignLeft) {
            leftAlign->setChecked(true);
            centerAlign->setChecked(false);
            rightAlign->setChecked(false);
            }
      else if (f.alignment() & Qt::AlignHCenter) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(true);
            rightAlign->setChecked(false);
            }
      else if (f.alignment() & Qt::AlignRight) {
            leftAlign->setChecked(false);
            centerAlign->setChecked(false);
            rightAlign->setChecked(true);
            }
      }

