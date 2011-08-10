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
      QHBoxLayout* ppLayout = new QHBoxLayout;
      ppLayout->addWidget(preview);
      previewGroup->setLayout(ppLayout);

      mmUnit = true;      // should be made a global configuration item

      if (mmUnit)
            mmButton->setChecked(true);
      else
            inchButton->setChecked(true);
      connect(mmButton, SIGNAL(clicked()), SLOT(mmClicked()));
      connect(inchButton, SIGNAL(clicked()), SLOT(inchClicked()));
      connect(buttonApply, SIGNAL(clicked()), SLOT(apply()));
      connect(buttonOk, SIGNAL(clicked()), SLOT(ok()));
      connect(landscape, SIGNAL(toggled(bool)), SLOT(landscapeToggled(bool)));
      connect(twosided, SIGNAL(toggled(bool)), SLOT(twosidedToggled(bool)));
      connect(pageHeight, SIGNAL(valueChanged(double)), SLOT(pageHeightChanged(double)));
      connect(pageWidth,  SIGNAL(valueChanged(double)), SLOT(pageWidthChanged(double)));
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

      pageGroup->setCurrentIndex(pf->size);
      setValues(s);

      connect(oddPageTopMargin, SIGNAL(valueChanged(double)), SLOT(otmChanged(double)));
      connect(oddPageBottomMargin, SIGNAL(valueChanged(double)), SLOT(obmChanged(double)));
      connect(oddPageLeftMargin, SIGNAL(valueChanged(double)), SLOT(olmChanged(double)));
      connect(oddPageRightMargin, SIGNAL(valueChanged(double)), SLOT(ormChanged(double)));
      connect(evenPageTopMargin, SIGNAL(valueChanged(double)), SLOT(etmChanged(double)));
      connect(evenPageBottomMargin, SIGNAL(valueChanged(double)), SLOT(ebmChanged(double)));
      connect(evenPageLeftMargin, SIGNAL(valueChanged(double)), SLOT(elmChanged(double)));
      connect(evenPageRightMargin, SIGNAL(valueChanged(double)), SLOT(ermChanged(double)));
      connect(pageGroup, SIGNAL(activated(int)), SLOT(pageFormatSelected(int)));
      connect(spatiumEntry, SIGNAL(valueChanged(double)), SLOT(spatiumChanged(double)));
	connect(pageOffsetEntry, SIGNAL(valueChanged(int)), SLOT(pageOffsetChanged(int)));
      }

//---------------------------------------------------------
//   setValues
//---------------------------------------------------------

void PageSettings::setValues(Score* sc)
      {
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

      PageFormat* pf = sc->pageFormat();

      QString s;

      if (mm) {
            oddPageTopMargin->setValue(pf->oddTopMargin * INCH);
            oddPageBottomMargin->setValue(pf->oddBottomMargin * INCH);
            oddPageLeftMargin->setValue(pf->oddLeftMargin * INCH);
            oddPageRightMargin->setValue(pf->oddRightMargin * INCH);

            evenPageTopMargin->setValue(pf->evenTopMargin * INCH);
            evenPageBottomMargin->setValue(pf->evenBottomMargin * INCH);
            evenPageLeftMargin->setValue(pf->evenLeftMargin * INCH);
            evenPageRightMargin->setValue(pf->evenRightMargin * INCH);

            spatiumEntry->setValue(cs->spatium()/DPMM);
            pageHeight->setValue(pf->height() * INCH);
            pageWidth->setValue(pf->width() * INCH);
            }
      else {
            oddPageTopMargin->setValue(pf->oddTopMargin);
            oddPageBottomMargin->setValue(pf->oddBottomMargin);
            oddPageLeftMargin->setValue(pf->oddLeftMargin);
            oddPageRightMargin->setValue(pf->oddRightMargin);

            evenPageTopMargin->setValue(pf->evenTopMargin);
            evenPageBottomMargin->setValue(pf->evenBottomMargin);
            evenPageLeftMargin->setValue(pf->evenLeftMargin);
            evenPageRightMargin->setValue(pf->evenRightMargin);

            spatiumEntry->setValue(cs->spatium()/DPI);
            pageWidth->setValue(pf->width());
            pageHeight->setValue(pf->height());
            }
      evenPageTopMargin->setEnabled(pf->twosided);
      evenPageBottomMargin->setEnabled(pf->twosided);
      evenPageLeftMargin->setEnabled(pf->twosided);
      evenPageRightMargin->setEnabled(pf->twosided);

      landscape->setChecked(pf->landscape);
      twosided->setChecked(pf->twosided);

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
      setValues(preview->score());
      }

//---------------------------------------------------------
//   mmClicked
//---------------------------------------------------------

void PageSettings::mmClicked()
      {
      mmUnit = true;
      setValues(preview->score());
      }

//---------------------------------------------------------
//   landscapeToggled
//---------------------------------------------------------

void PageSettings::landscapeToggled(bool flag)
      {
      preview->score()->pageFormat()->landscape = flag;
      preview->layout();
      setValues(preview->score());
      }

//---------------------------------------------------------
//   twosidedToggled
//---------------------------------------------------------

void PageSettings::twosidedToggled(bool flag)
      {
      preview->score()->pageFormat()->twosided = flag;
      preview->layout();
      setValues(preview->score());
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void PageSettings::apply()
      {
      double f  = mmUnit ? 1.0/INCH : 1.0;
      double f1 = mmUnit ? DPMM : DPI;

      PageFormat pf;

      pf.size             = pageGroup->currentIndex();
      pf._width           = pageWidth->value() * f;
      pf._height          = pageHeight->value() * f;
      pf.evenTopMargin    = evenPageTopMargin->value() * f;
      pf.evenBottomMargin = evenPageBottomMargin->value() * f;
      pf.evenLeftMargin   = evenPageLeftMargin->value() * f;
      pf.evenRightMargin  = evenPageRightMargin->value() * f;
      pf.oddTopMargin     = oddPageTopMargin->value() * f;
      pf.oddBottomMargin  = oddPageBottomMargin->value() * f;
      pf.oddLeftMargin    = oddPageLeftMargin->value() * f;
      pf.oddRightMargin   = oddPageRightMargin->value() * f;
      pf.landscape        = landscape->isChecked();
      pf.twosided         = twosided->isChecked();

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
printf("page format %d\n", pf);
      preview->score()->pageFormat()->size = pf;
      preview->layout();
      setValues(preview->score());
      }

//---------------------------------------------------------
//   otmChanged
//---------------------------------------------------------

void PageSettings::otmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->oddTopMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   olmChanged
//---------------------------------------------------------

void PageSettings::olmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->oddLeftMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   ormChanged
//---------------------------------------------------------

void PageSettings::ormChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->oddRightMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   obmChanged
//---------------------------------------------------------

void PageSettings::obmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->oddBottomMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   etmChanged
//---------------------------------------------------------

void PageSettings::etmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->evenTopMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   elmChanged
//---------------------------------------------------------

void PageSettings::elmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->evenLeftMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   ermChanged
//---------------------------------------------------------

void PageSettings::ermChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->evenRightMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   ebmChanged
//---------------------------------------------------------

void PageSettings::ebmChanged(double val)
      {
      if (mmUnit)
            val /= INCH;
      preview->score()->pageFormat()->evenBottomMargin = val;
      preview->layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void PageSettings::spatiumChanged(double val)
      {
      val *= mmUnit ? DPMM : DPI;
      preview->score()->setSpatium(val);
      preview->layout();
      }

//---------------------------------------------------------
//   pageOffsetChanged
//---------------------------------------------------------

void PageSettings::pageOffsetChanged(int val)
      {
      preview->score()->setPageNumberOffset(val);
      preview->layout();
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
      f->size = pf;
      f->_height = val;
      f->_width = val2;

      preview->layout();
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
      f->size = pf;
      f->_width = val;
      f->_height = val2;

      preview->layout();
      }

