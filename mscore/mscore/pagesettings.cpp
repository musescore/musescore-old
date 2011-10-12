//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "globals.h"
#include "pagesettings.h"
#include "libmscore/page.h"
#include "libmscore/style.h"
#include "libmscore/score.h"
#include "navigator.h"
#include "libmscore/mscore.h"
#include "musescore.h"

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

PageSettings::PageSettings(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      setModal(true);

      NScrollArea* sa = new NScrollArea;
      preview = new Navigator(sa, this);

      static_cast<QVBoxLayout*>(previewGroup->layout())->insertWidget(0, sa);

      mmUnit = true;      // should be made a global configuration item

      if (mmUnit)
            mmButton->setChecked(true);
      else
            inchButton->setChecked(true);
      connect(mmButton,             SIGNAL(clicked()),            SLOT(mmClicked()));
      connect(inchButton,           SIGNAL(clicked()),            SLOT(inchClicked()));
      connect(buttonApply,          SIGNAL(clicked()),            SLOT(apply()));
      connect(buttonOk,             SIGNAL(clicked()),            SLOT(ok()));
      connect(landscape,            SIGNAL(toggled(bool)),        SLOT(landscapeToggled(bool)));
      connect(twosided,             SIGNAL(toggled(bool)),        SLOT(twosidedToggled(bool)));
      connect(pageHeight,           SIGNAL(valueChanged(double)), SLOT(pageHeightChanged(double)));
      connect(pageWidth,            SIGNAL(valueChanged(double)), SLOT(pageWidthChanged(double)));
      connect(oddPageTopMargin,     SIGNAL(valueChanged(double)), SLOT(otmChanged(double)));
      connect(oddPageBottomMargin,  SIGNAL(valueChanged(double)), SLOT(obmChanged(double)));
      connect(oddPageLeftMargin,    SIGNAL(valueChanged(double)), SLOT(olmChanged(double)));
      connect(oddPageRightMargin,   SIGNAL(valueChanged(double)), SLOT(ormChanged(double)));
      connect(evenPageTopMargin,    SIGNAL(valueChanged(double)), SLOT(etmChanged(double)));
      connect(evenPageBottomMargin, SIGNAL(valueChanged(double)), SLOT(ebmChanged(double)));
      connect(pageGroup,            SIGNAL(activated(int)),       SLOT(pageFormatSelected(int)));
      connect(spatiumEntry,         SIGNAL(valueChanged(double)), SLOT(spatiumChanged(double)));
      connect(pageOffsetEntry,      SIGNAL(valueChanged(int)),    SLOT(pageOffsetChanged(int)));
      }

//---------------------------------------------------------
//   PageSettings
//---------------------------------------------------------

PageSettings::~PageSettings()
      {
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void PageSettings::setScore(Score* s)
      {
      cs  = s;
      preview->setScore(s->clone());

      Score* sl = preview->score();
      const PageFormat* pf = s->pageFormat();

      sl->setPageFormat(*pf);
      sl->setSpatium(s->spatium());

      pageGroup->clear();
      for (int i = 0; true; ++i) {
            if (paperSizes[i].name == 0)
                  break;
            pageGroup->addItem(QString(paperSizes[i].name));
            }

      pageGroup->setCurrentIndex(pf->size());
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   updateValues
//---------------------------------------------------------

void PageSettings::updateValues()
      {
      Score* sc = preview->score();
      bool mm = mmButton->isChecked();

      oddPageTopMargin->blockSignals(true);
      oddPageBottomMargin->blockSignals(true);
      oddPageLeftMargin->blockSignals(true);
      oddPageRightMargin->blockSignals(true);
      evenPageTopMargin->blockSignals(true);
      evenPageBottomMargin->blockSignals(true);
      evenPageLeftMargin->blockSignals(true);
      evenPageRightMargin->blockSignals(true);
      spatiumEntry->blockSignals(true);
      pageWidth->blockSignals(true);
      pageHeight->blockSignals(true);
      pageOffsetEntry->blockSignals(true);

      const char* suffix = mm ? "mm" : "in";
      oddPageTopMargin->setSuffix(suffix);
      oddPageBottomMargin->setSuffix(suffix);
      oddPageLeftMargin->setSuffix(suffix);
      oddPageRightMargin->setSuffix(suffix);
      evenPageTopMargin->setSuffix(suffix);
      evenPageBottomMargin->setSuffix(suffix);
      evenPageLeftMargin->setSuffix(suffix);
      evenPageRightMargin->setSuffix(suffix);
      spatiumEntry->setSuffix(suffix);
      pageWidth->setSuffix(suffix);
      pageHeight->setSuffix(suffix);

      const PageFormat* pf = sc->pageFormat();

      QString s;

      //qreal printableWidthValue = pf->printableWidth();
      qreal widthValue = pf->width();
      if (mm) {
            oddPageTopMargin->setValue(pf->oddTopMargin() * INCH);
            oddPageBottomMargin->setValue(pf->oddBottomMargin() * INCH);
            oddPageLeftMargin->setValue(pf->oddLeftMargin() * INCH);
            oddPageRightMargin->setValue(pf->oddRightMargin() * INCH);

            evenPageTopMargin->setValue(pf->evenTopMargin() * INCH);
            evenPageBottomMargin->setValue(pf->evenBottomMargin() * INCH);
            evenPageLeftMargin->setValue(pf->evenLeftMargin() * INCH);
            evenPageRightMargin->setValue(pf->evenRightMargin() * INCH);

            spatiumEntry->setValue(cs->spatium()/DPMM);
            pageHeight->setValue(pf->height() * INCH);
            widthValue          *= INCH;
            }
      else {
            oddPageTopMargin->setValue(pf->oddTopMargin());
            oddPageBottomMargin->setValue(pf->oddBottomMargin());
            oddPageLeftMargin->setValue(pf->oddLeftMargin());
            oddPageRightMargin->setValue(pf->oddRightMargin());

            evenPageTopMargin->setValue(pf->evenTopMargin());
            evenPageBottomMargin->setValue(pf->evenBottomMargin());
            evenPageLeftMargin->setValue(pf->evenLeftMargin());
            evenPageRightMargin->setValue(pf->evenRightMargin());

            spatiumEntry->setValue(cs->spatium()/DPI);
            pageHeight->setValue(pf->height());
            }
      pageWidth->setValue(widthValue);

      oddPageLeftMargin->setMaximum(widthValue);
      oddPageRightMargin->setMaximum(widthValue);
      evenPageLeftMargin->setMaximum(widthValue);
      evenPageRightMargin->setMaximum(widthValue);

      evenPageTopMargin->setEnabled(pf->twosided());
      evenPageBottomMargin->setEnabled(pf->twosided());
      evenPageLeftMargin->setEnabled(false);
      evenPageRightMargin->setEnabled(false);

      if(twosided->isChecked()) {
            evenPageRightMargin->setValue(oddPageLeftMargin->value());
            evenPageLeftMargin->setValue(oddPageRightMargin->value());
            }
      else {
            evenPageRightMargin->setValue(oddPageRightMargin->value());
            evenPageLeftMargin->setValue(oddPageLeftMargin->value());
      }

      landscape->setChecked(pf->landscape());
      twosided->setChecked(pf->twosided());

      pageOffsetEntry->setValue(sc->pageNumberOffset() + 1);

      pageWidth->blockSignals(false);
      pageHeight->blockSignals(false);
      oddPageTopMargin->blockSignals(false);
      oddPageBottomMargin->blockSignals(false);
      oddPageLeftMargin->blockSignals(false);
      oddPageRightMargin->blockSignals(false);
      evenPageTopMargin->blockSignals(false);
      evenPageBottomMargin->blockSignals(false);
      evenPageLeftMargin->blockSignals(false);
      evenPageRightMargin->blockSignals(false);
      spatiumEntry->blockSignals(false);
      pageOffsetEntry->blockSignals(false);
	}

//---------------------------------------------------------
//   inchClicked
//---------------------------------------------------------

void PageSettings::inchClicked()
      {
      mmUnit = false;
      updateValues();
      }

//---------------------------------------------------------
//   mmClicked
//---------------------------------------------------------

void PageSettings::mmClicked()
      {
      mmUnit = true;
      updateValues();
      }

//---------------------------------------------------------
//   landscapeToggled
//---------------------------------------------------------

void PageSettings::landscapeToggled(bool flag)
      {
      PageFormat pf(*preview->score()->pageFormat());
      pf.setLandscape(flag);
      preview->score()->setPageFormat(pf);
      updateValues();
      updatePreview(0);
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      PageFormat pf(*preview->score()->pageFormat());
      pf.setTwosided(flag);
      preview->score()->setPageFormat(pf);
      updateValues();
      updatePreview(1);
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PageSettings::apply()
      {
      double f  = mmUnit ? 1.0/INCH : 1.0;
      double f1 = mmUnit ? DPMM : DPI;

      PageFormat pf;

      pf.setSize(pageGroup->currentIndex());
      pf.setWidth(pageWidth->value() * f);
      pf.setHeight(pageHeight->value() * f);
      pf.setPrintableWidth((pageWidth->value() - oddPageLeftMargin->value() - oddPageRightMargin->value())  * f);
      pf.setEvenTopMargin(evenPageTopMargin->value() * f);
      pf.setEvenBottomMargin(evenPageBottomMargin->value() * f);
      pf.setEvenLeftMargin(evenPageLeftMargin->value() * f);
      pf.setOddTopMargin(oddPageTopMargin->value() * f);
      pf.setOddBottomMargin(oddPageBottomMargin->value() * f);
      pf.setOddLeftMargin(oddPageLeftMargin->value() * f);
      pf.setLandscape(landscape->isChecked());
      pf.setTwosided(twosided->isChecked());

      double sp = spatiumEntry->value() * f1;

      cs->startCmd();
      cs->undoChangePageFormat(&pf, sp, pageOffsetEntry->value()-1);
      cs->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void PageSettings::ok()
      {
      apply();
      done(0);
      }

//---------------------------------------------------------
//   done
//---------------------------------------------------------

void PageSettings::done(int val)
      {
      cs->setLayoutAll(true);     // HACK
      QDialog::done(val);
      }

//---------------------------------------------------------
//   pageFormatSelected
//---------------------------------------------------------

void PageSettings::pageFormatSelected(int size)
      {
      PageFormat pf(*preview->score()->pageFormat());
      pf.setSize(size);
      preview->score()->setPageFormat(pf);
      preview->score()->doLayout();
      updatePreview(0);
      }

//---------------------------------------------------------
//   otmChanged
//---------------------------------------------------------

void PageSettings::otmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf(*preview->score()->pageFormat());
      pf.setOddTopMargin(val);
      preview->score()->setPageFormat(pf);
      updatePreview(1);
      }

//---------------------------------------------------------
//   olmChanged
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      if(twosided->isChecked()) {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }
      else{
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageLeftMargin->blockSignals(false);
            }
      PageFormat pf(*preview->score()->pageFormat());
      pf.setPrintableWidth(pf.width() - pf.oddRightMargin() - val);
      pf.setOddLeftMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(0);
      }

//---------------------------------------------------------
//   ormChanged
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat pf(*preview->score()->pageFormat());

      if (twosided->isChecked()) {
            evenPageLeftMargin->blockSignals(true);
            evenPageLeftMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageLeftMargin->blockSignals(false);
            }
      else {
            evenPageRightMargin->blockSignals(true);
            evenPageRightMargin->setValue(val * (mmUnit ? INCH : 1.0));
            evenPageRightMargin->blockSignals(false);
            }

      pf.setPrintableWidth(pf.width() - pf.oddLeftMargin() - val);
      preview->score()->setPageFormat(pf);
      updatePreview(0);
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf(*preview->score()->pageFormat());
      pf.setOddBottomMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(1);
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf(*preview->score()->pageFormat());
      pf.setEvenTopMargin(val);
      preview->score()->setPageFormat(pf);

      updatePreview(1);
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      PageFormat pf(*preview->score()->pageFormat());
      pf.setEvenBottomMargin(val);
      preview->score()->setPageFormat(pf);
      updatePreview(1);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void PageSettings::spatiumChanged(double val)
      {
      val *= mmUnit ? DPMM : DPI;
      preview->score()->setSpatium(val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   pageOffsetChanged
//---------------------------------------------------------

void PageSettings::pageOffsetChanged(int val)
      {
      preview->score()->setPageNumberOffset(val);
      updatePreview(0);
      }

//---------------------------------------------------------
//   pageHeightChanged
//---------------------------------------------------------

void PageSettings::pageHeightChanged(double val)
      {
      double val2 = pageWidth->value();
      if (mmUnit) {
            val /= INCH;
            val2 /= INCH;
            }
      int pf = paperSizeNameToIndex("Custom");
      pageGroup->setCurrentIndex(pf);

      PageFormat f(*preview->score()->pageFormat());
      f.setSize(pf);
      f.setHeight(val);
      f.setWidth(val2);
      preview->score()->setPageFormat(f);

      updatePreview(1);
      }

//---------------------------------------------------------
//   pageWidthChanged
//---------------------------------------------------------

void PageSettings::pageWidthChanged(double val)
      {
      double val2 = pageHeight->value();
      if (mmUnit) {
            val /= INCH;
            val2 /= INCH;
            }
      int pf = paperSizeNameToIndex("Custom");
      pageGroup->setCurrentIndex(pf);

      PageFormat f(*preview->score()->pageFormat());
      f.setSize(pf);
      f.setWidth(val);
      f.setHeight(val2);
      preview->score()->setPageFormat(f);

      updatePreview(0);
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview(int val)
      {
      switch(val) {
            case 0:
                  preview->score()->doLayout();
                  break;
            case 1:
                  preview->score()->doLayoutPages();
                  break;
            }
      preview->layoutChanged();
      }

