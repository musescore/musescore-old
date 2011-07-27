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
#include "page.h"
#include "style.h"
#include "score.h"
#include "preview.h"

#define MM(x) ((x)/INCH)

const PaperSize paperSizes[] = {
      PaperSize(QPrinter::A4,      "A4",        MM(210),  MM(297)),
      PaperSize(QPrinter::B5,      "B5",        MM(176),  MM(250)),
      PaperSize(QPrinter::Letter,  "Letter",    8.5,      11),
      PaperSize(QPrinter::Legal,   "Legal",     8.5,      14),
      PaperSize(QPrinter::Executive,"Executive",7.5,      10),
      PaperSize(QPrinter::A0,      "A0",        MM(841),  MM(1189)),
      PaperSize(QPrinter::A1,      "A1",        MM(594),  MM(841)),
      PaperSize(QPrinter::A2,      "A2",        MM(420),  MM(594)),
      PaperSize(QPrinter::A3,      "A3",        MM(297),  MM(420)),
      PaperSize(QPrinter::A5,      "A5",        MM(148),  MM(210)),
      PaperSize(QPrinter::A6,      "A6",        MM(105),  MM(148)),
      PaperSize(QPrinter::A7,      "A7",        MM(74),   MM(105)),
      PaperSize(QPrinter::A8,      "A8",        MM(52),   MM(74)),
      PaperSize(QPrinter::A9,      "A9",        MM(37),   MM(52)),
      PaperSize(QPrinter::B0,      "B0",        MM(1000), MM(1414)),
      PaperSize(QPrinter::B1,      "B1",        MM(707),  MM(1000)),
      PaperSize(QPrinter::B10,     "B10",       MM(31),   MM(44)),
      PaperSize(QPrinter::B2,      "B2",        MM(500),  MM(707)),
      PaperSize(QPrinter::B3,      "B3",        MM(353),  MM(500)),
      PaperSize(QPrinter::B4,      "B4",        MM(250),  MM(353)),
      PaperSize(QPrinter::B5,      "B5",        MM(125),  MM(176)),
      PaperSize(QPrinter::B6,      "B6",        MM(88),   MM(125)),
      PaperSize(QPrinter::B7,      "B7",        MM(62),   MM(88)),
      PaperSize(QPrinter::B8,      "B8",        MM(44),   MM(62)),
      PaperSize(QPrinter::B9,      "B9",        MM(163),  MM(229)),
      PaperSize(QPrinter::Comm10E, "Comm10E",   MM(105),  MM(241)),
      PaperSize(QPrinter::DLE,     "DLE",       MM(110),  MM(220)),
      PaperSize(QPrinter::Folio,   "Folio",     MM(210),  MM(330)),
      PaperSize(QPrinter::Ledger,  "Ledger",    MM(432),  MM(279)),
      PaperSize(QPrinter::Tabloid, "Tabloid",   MM(279),  MM(432)),
      PaperSize(QPrinter::Custom,  "Custom",    MM(210),  MM(297)),
      PaperSize(QPrinter::A4, 0, 0, 0  )
      };

//---------------------------------------------------------
//   paperSizeNameToIndex
//---------------------------------------------------------

int paperSizeNameToIndex(const QString& name)
      {
      int i;
      for (i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (name == paperSizes[i].name)
                  return i;
            }
      printf("unknown paper size\n");
      return 0;
      }

//---------------------------------------------------------
//   paperSizeSizeToIndex
//---------------------------------------------------------

static const double minSize = 0.1;      // minimum paper size for sanity check
static const double maxError = 0.01;    // max allowed error when matching sizes

static double sizeError(const double si, const double sref)
      {
      double relErr = (si - sref) / sref;
      return relErr > 0 ? relErr : -relErr;
      }

int paperSizeSizeToIndex(const double wi, const double hi)
      {
      if (wi < minSize || hi < minSize) return -1;
      int i;
      for (i = 0;;++i) {
            if (paperSizes[i].name == 0)
                  break;
            if (sizeError(wi, paperSizes[i].w) < maxError && sizeError(hi, paperSizes[i].h) < maxError)
                  return i;
            }
      printf("unknown paper size\n");
      return -1;
      }

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
      delete preview;
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
      
      pageOffsetEntry->setValue(pf->_pageOffset + 1);

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
      pf._pageOffset      = pageOffsetEntry->value() - 1;

      double sp = spatiumEntry->value() * f1;

      cs->startCmd();
      cs->undoChangePageFormat(&pf, sp);
      cs->setLayoutAll(true);
      cs->endCmd();
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
      PageFormat* f = preview->score()->pageFormat();
      f->_pageOffset = val - 1;
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
