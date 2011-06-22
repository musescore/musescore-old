//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010-2011 Werner Schweer
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

#include "pattern.h"
#include "utils.h"
#include "libmscore/sym.h"

//---------------------------------------------------------
//   Pattern
//---------------------------------------------------------

Pattern::Pattern()
   : QImage()
      {
      }

Pattern::~Pattern()
      {
      }

//---------------------------------------------------------
//   patternMatch
//    compare two patterns for similarity
//    return:
//          1.0   - identical
//          0.5   - 50% of all pixel match
//          0.0   - no match
//---------------------------------------------------------

double Pattern::match(const Pattern* a) const
      {
      if (image()->byteCount() != a->image()->byteCount())
            return 0.0;
      int k = 0;
      const uchar* p1 = image()->bits();
      const uchar* p2 = a->image()->bits();
      for (int i = 0; i < image()->byteCount(); ++i) {
            uchar v = (*(p1++)) ^ (*(p2++));
            k += bitsSetTable[v];
            }
      return 1.0 - (double(k) / (h() * w()));
      }

//---------------------------------------------------------
//   Pattern
//    create a Pattern from symbol
//---------------------------------------------------------

Pattern::Pattern(Sym* symbol, double spatium)
   : QImage()
      {
      QFont f("MScore");
      f.setPixelSize(lrint(spatium * 4));
      QFontMetrics fm(f);
      QString s;
      QChar code(symbol->code());
      QRect r(fm.boundingRect(code));
      int _w = r.width();
      int _h = ((r.height() + 1) / 2) * 2;

      _image = QImage(_w, _h, QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      _image.setColorTable(ct);
      _image.fill(0);

      QPainter painter;
      painter.begin(&_image);
      painter.setFont(f);
      painter.drawText(0, _h / 2, code);
      painter.end();

      int ww = _w % 32;
      if (ww == 0)
            return;
      uint mask = 0xffffffff << ww;
      int n = ((_w + 31) / 32) - 1;
      for (int i = 0; i < _h; ++i) {
            uint* p = (uint*)_image.scanLine(i);
            p += n;
            *p = ((*p) & ~mask);
            }
      }

//---------------------------------------------------------
//   Pattern
//    create a Pattern from image
//---------------------------------------------------------

Pattern::Pattern(QImage* img, int x, int y, int w, int h)
      {
      _image = img->copy(x, y, w, h);
      int ww = w % 32;
      if (ww == 0)
            return;
      uint mask = 0xffffffff << ww;
      int n = ((w + 31) / 32) - 1;
      for (int i = 0; i < h; ++i) {
            uint* p = (uint*)_image.scanLine(i);
            p += n;
            *p = ((*p) & ~mask);
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void Pattern::dump() const
      {
      printf("pattern %d x %d\n", _image.width(), _image.height());
      for (int y = 0; y < _image.height(); ++y) {
            for (int x = 0; x < _image.bytesPerLine(); ++x) {
                  uchar c = *(_image.bits() + y * _image.bytesPerLine() + x);
                  for (int i = 0; i < 8; ++i)
                        printf("%c", (c & (0x1 << i)) ? '*' : '-');
                  }
            printf("\n");
            }
      }

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

bool Pattern::dot(int x, int y) const
      {
      const uint* p = (const uint*)_image.scanLine(y) + (x / 32);
      return (*p) & (0x1 << (x % 32));
      }
