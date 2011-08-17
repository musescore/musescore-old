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

#include "globals.h"
#include "pagesettings.h"
#include "libmscore/page.h"
#include "libmscore/style.h"
#include "libmscore/score.h"
#include "preview.h"
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
      preview = new PagePreview;
      static_cast<QVBoxLayout*>(previewGroup->layout())->insertWidget(0, preview);

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
      connect(previewPage,          SIGNAL(valueChanged(int)),    SLOT(setPage(int)));
      connect(printableWidth,       SIGNAL(valueChanged(double)), SLOT(pwChanged(double)));
      connect(oddPageTopMargin,     SIGNAL(valueChanged(double)), SLOT(otmChanged(double)));
      connect(oddPageBottomMargin,  SIGNAL(valueChanged(double)), SLOT(obmChanged(double)));
      connect(oddPageLeftMargin,    SIGNAL(valueChanged(double)), SLOT(olmChanged(double)));
      connect(oddPageRightMargin,   SIGNAL(valueChanged(double)), SLOT(ormChanged(double)));
      connect(evenPageTopMargin,    SIGNAL(valueChanged(double)), SLOT(etmChanged(double)));
      connect(evenPageBottomMargin, SIGNAL(valueChanged(double)), SLOT(ebmChanged(double)));
      connect(evenPageLeftMargin,   SIGNAL(valueChanged(double)), SLOT(elmChanged(double)));
      connect(evenPageRightMargin,  SIGNAL(valueChanged(double)), SLOT(ermChanged(double)));
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
      preview->setScore(s);
      // previewPage->setValue(1);

      Score* sl = preview->score();
      PageFormat* pf = s->pageFormat();

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
      printableWidth->blockSignals(true);

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
      printableWidth->setSuffix(suffix);

      PageFormat* pf = sc->pageFormat();

      QString s;

      qreal printableWidthValue = pf->printableWidth();
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
            printableWidthValue *= INCH;
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
      printableWidth->setValue(printableWidthValue);
      printableWidth->setMaximum(widthValue);
      qreal maxBorder = widthValue - printableWidthValue;

      oddPageLeftMargin->setMaximum(maxBorder);
      oddPageRightMargin->setMaximum(maxBorder);
      evenPageLeftMargin->setMaximum(maxBorder);
      evenPageRightMargin->setMaximum(maxBorder);

      evenPageTopMargin->setEnabled(pf->twosided());
      evenPageBottomMargin->setEnabled(pf->twosided());
      evenPageLeftMargin->setEnabled(pf->twosided());
      evenPageRightMargin->setEnabled(pf->twosided());

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
      printableWidth->blockSignals(false);
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
      preview->score()->pageFormat()->setLandscape(flag);
      updateValues();
      updatePreview();
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      preview->score()->pageFormat()->setTwosided(flag);
      updateValues();
      updatePreview();
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
      pf.setPrintableWidth(printableWidth->value() * f);
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

void PageSettings::pageFormatSelected(int pf)
      {
      preview->score()->pageFormat()->setSize(pf);
      preview->doLayout();
      updatePreview();
      }

//---------------------------------------------------------
//   otmChanged
//---------------------------------------------------------

void PageSettings::otmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->setOddTopMargin(val);
      updatePreview();
      }

//---------------------------------------------------------
//   olmChanged
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat* pf = preview->score()->pageFormat();
      pf->setOddLeftMargin(val);

      // right margin depends, as score width is identical
      // for even and odd pages
      oddPageRightMargin->blockSignals(true);
      oddPageRightMargin->setValue(pf->oddRightMargin() * (mmUnit ? INCH : 1.0));
      evenPageRightMargin->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   ormChanged
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat* pf = preview->score()->pageFormat();

      // left margin depends, as score width is identical
      // for even and odd pages
      oddPageLeftMargin->blockSignals(true);
      oddPageLeftMargin->setValue(pf->oddLeftMargin() * (mmUnit ? INCH : 1.0));
      evenPageRightMargin->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->setOddBottomMargin(val);
      updatePreview();
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->setEvenTopMargin(val);
      updatePreview();
      }

//---------------------------------------------------------
//   elmChanged
//---------------------------------------------------------

void PageSettings::elmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat* pf = preview->score()->pageFormat();
      pf->setEvenLeftMargin(val);

      // right margin depends, as score width is identical
      // for even and odd pages
      evenPageRightMargin->blockSignals(true);
      evenPageRightMargin->setValue(pf->evenRightMargin() * (mmUnit ? INCH : 1.0));
      evenPageRightMargin->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   ermChanged
//---------------------------------------------------------

void PageSettings::ermChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat* pf = preview->score()->pageFormat();

      // left margin depends, as score width is identical
      // for even and odd pages
      evenPageLeftMargin->blockSignals(true);
      evenPageLeftMargin->setValue(pf->evenLeftMargin() * (mmUnit ? INCH : 1.0));
      evenPageLeftMargin->blockSignals(false);

      updatePreview();
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->setEvenBottomMargin(val);
      updatePreview();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void PageSettings::spatiumChanged(double val)
      {
      val *= mmUnit ? DPMM : DPI;
      preview->score()->setSpatium(val);
      updatePreview();
      }

//---------------------------------------------------------
//   pageOffsetChanged
//---------------------------------------------------------

void PageSettings::pageOffsetChanged(int val)
      {
      preview->score()->setPageNumberOffset(val);
      updatePreview();
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

      PageFormat* f = preview->score()->pageFormat();
      f->setSize(pf);
      f->setHeight(val);
      f->setWidth(val2);

      updatePreview();
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

      PageFormat* f = preview->score()->pageFormat();
      f->setSize(pf);
      f->setWidth(val);
      f->setHeight(val2);

      updatePreview();
      }

//---------------------------------------------------------
//   setPage
//---------------------------------------------------------

void PageSettings::setPage(int n)
      {
      preview->showPage(n-1);
      }

//---------------------------------------------------------
//   updatePreview
//---------------------------------------------------------

void PageSettings::updatePreview()
      {
      preview->doLayout();
      previewPage->setMaximum(preview->pages());
      setPage(previewPage->value());
      }

//---------------------------------------------------------
//   pwChanged
//---------------------------------------------------------

void PageSettings::pwChanged(double val)
      {
      if (mmUnit)
            val /= INCH;

      PageFormat* pf = preview->score()->pageFormat();
      pf->setPrintableWidth(val);

      if (val + pf->oddLeftMargin() > pf->width())
            pf->setOddLeftMargin(pf->width() - val);
      if (val + pf->evenLeftMargin() > pf->width())
            pf->setEvenLeftMargin(pf->width() - val);

      updateValues();
      }

