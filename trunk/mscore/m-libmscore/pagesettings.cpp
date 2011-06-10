//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pagesettings.cpp 3549 2010-10-04 10:51:28Z wschweer $
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
#include "page.h"
#include "style.h"
#include "score.h"

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
      PaperSize(int(QPrinter::Custom) + 1, "iPad",   MM(148),  MM(197)),
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

static const qreal minSize = 0.1;      // minimum paper size for sanity check
static const qreal maxError = 0.01;    // max allowed error when matching sizes

static qreal sizeError(const qreal si, const qreal sref)
      {
      qreal relErr = (si - sref) / sref;
      return relErr > 0 ? relErr : -relErr;
      }

int paperSizeSizeToIndex(const qreal wi, const qreal hi)
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

