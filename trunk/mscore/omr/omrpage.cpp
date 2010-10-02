//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "omrpage.h"
#include "image.h"
#include "utils.h"
#include "omr.h"
#include "ocr.h"
#include "score.h"
#include "text.h"
#include "measurebase.h"
#include "box.h"

struct Lv {
      int line;
      double val;
      Lv(int a, double b) : line(a), val(b) {}
      bool operator< (const Lv& a) const {
            return a.val < val;
            }
      };

//---------------------------------------------------------
//   OmrPage
//---------------------------------------------------------

OmrPage::OmrPage(Omr* parent)
      {
      _omr = parent;
      cropL = cropR = cropT = cropB = 0;
      }

//---------------------------------------------------------
//   dot
//---------------------------------------------------------

bool OmrPage::dot(int x, int y) const
      {
      const uint* p = scanLine(y) + (x / 32);
      return (*p) & (0x1 << (x % 32));
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void OmrPage::read(int pageNo)
      {
      crop();
      slice();
      deSkew();
      crop();
      slice();
      getStaffLines();

      int numStaves = staves.size();
      printf("===numStaves: %d\n", numStaves);

      //--------------------------------------------------
      //    search bar lines
      //--------------------------------------------------

      int stavesSystem = 2;
      int systems = numStaves / stavesSystem;

      for (int system = 0; system < systems; ++system) {
            int idx = system * stavesSystem;
            int x1  = staves[idx].x();
            int x2  = x1 + staves[idx].width();
            int y1  = staves[idx].y();
            int y2  = staves[idx+1].y() + staves[idx+1].height();
            int h   = y2 - y1 + 1;
            int th  = h * 4 / 5;     // 4/5 threshold
            int xx  = -1;
            int w   = 0;
            bool firstBarLine = true;
            for (int x = x1; x < x2; ++x) {
                  int dots = 0;
                  for (int y = y1; y < y2; ++y) {
                        if (dot(x, y) || dot(x-1, y) || dot(x+1, y))
                              ++dots;
                        }
                  if (dots >= th) {
                        if (x > xx+1) {
                              if (w) {
                                    double dx = double(xx) - (w * .5);
                                    barlines.append(QLineF(dx, y1, dx, y1 + h));
                                    if (firstBarLine) {
                                          firstBarLine = false;
                                          staves[idx].setX(dx);
                                          staves[idx + 1].setX(dx);
                                          }
                                    else {
                                          staves[idx].setWidth(dx - staves[idx].x());
                                          staves[idx+1].setWidth(dx - staves[idx+1].x());
                                          }
                                    w = 1;
                                    }
                              else
                                    ++w;
                              }
                        xx = x;
                        }
                  }
            if (w) {
                  double dx = double(xx) - (w * .5);
                  barlines.append(QLineF(dx, y1, dx, y1 + h));
                  staves[idx].setWidth(dx - staves[idx].x());
                  staves[idx+1].setWidth(dx - staves[idx+1].x());
                  }
            }
#if 0
      foreach(QRectF r, staves) {
            int x1 = r.x();
            int x2 = x1 + r.width();
            int y = r.y();
            for (int i = -1; i < 10; ++i) {
                  if (i & 0x1)
                        searchNotes(i, x1, x2, y + i * _spatium*.5);
                  }
            }
#endif
      }

//---------------------------------------------------------
//   addText
//---------------------------------------------------------

static void addText(Score* score, int subtype, const QString& s)
      {
      MeasureBase* measure = score->first();
      if (measure == 0 || measure->type() != VBOX) {
            measure = new VBox(score);
            measure->setNext(score->first());
            measure->setTick(0);
      	score->add(measure);
            }
      Text* text = new Text(score);
      switch(subtype) {
            case TEXT_TITLE:    text->setTextStyle(TEXT_STYLE_TITLE);    break;
            case TEXT_SUBTITLE: text->setTextStyle(TEXT_STYLE_SUBTITLE); break;
            case TEXT_COMPOSER: text->setTextStyle(TEXT_STYLE_COMPOSER); break;
            case TEXT_POET:     text->setTextStyle(TEXT_STYLE_POET);     break;
            }
      text->setSubtype(subtype);
      text->setText(s);
      measure->add(text);
      }

//---------------------------------------------------------
//   readHeader
//---------------------------------------------------------

void OmrPage::readHeader(Score* score)
      {
      OcrImage img = OcrImage(_image.bits(), _slices[0], (_image.width() + 31)/32);
      QString s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_TITLE, s);

      img = OcrImage(_image.bits(), _slices[1], (_image.width() + 31)/32);
      s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_SUBTITLE, s);

      img = OcrImage(_image.bits(), _slices[2], (_image.width() + 31)/32);
      s = _omr->ocr()->readLine(img).trimmed();
      if (!s.isEmpty())
            addText(score, TEXT_COMPOSER, s);
      }

//---------------------------------------------------------
//   crop
//---------------------------------------------------------

void OmrPage::crop()
      {
      int wl  = wordsPerLine();
      int cropT = cropB = cropL = cropR = 0;
      for (int y = 0; y < height(); ++y) {
            const uint* p = scanLine(y);
            for (int k = 0; k < wl; ++k) {
                  if (*p++) {
                        cropT = y;
                        break;
                        }
                  }
            if (cropT)
                  break;
            }
      for (int y = height()-1; y >= cropT; --y) {
            const uint* p = scanLine(y);
            for (int k = 0; k < wl; ++k) {
                  if (*p++) {
                        cropB = height() - y - 1;
                        break;
                        }
                  }
            if (cropB)
                  break;
            }
      int y1 = cropT;
      int y2 = height() - cropT - cropB;
      for (int x = 0; x < wl; ++x) {
            for (int y = y1; y < y2; ++y) {
                  if (*(scanLine(y) + x)) {
                        cropL = x;
                        break;
                        }
                  }
            if (cropL)
                  break;
            }
      for (int x = wl-1; x >= cropL; --x) {
            for (int y = y1; y < y2; ++y) {
                  if (*(scanLine(y) + x)) {
                        cropR = wl - x - 1;
                        break;
                        }
                  }
            if (cropR)
                  break;
            }
      printf("*** crop: T%d B%d L%d R:%d\n", cropT, cropB, cropL, cropR);
      }

//---------------------------------------------------------
//   slice
//---------------------------------------------------------

void OmrPage::slice()
      {
      _slices.clear();
      int y1    = cropT;
      int y2    = height() - cropB;
      int x1    = cropL;
      int x2    = wordsPerLine() - cropR;
      int xbits = (x2 - x1) * 32;

      for (int y = y1; y < y2;) {
            //
            // skip contents
            //
            int ys = y;
            for (; y < y2; ++y) {
                  const uint* p = scanLine(y) + cropL;
                  bool bits = false;
                  for (int x = cropL; x < x2; ++x) {
                        if (*p) {
                              bits = true;
                              break;
                              }
                        ++p;
                        }
                  if (!bits)
                        break;
                  }
            if (y - ys)
                  _slices.append(QRect(cropL*32, ys, xbits, y - ys));
            //
            // skip space
            //
            for (; y < y2; ++y) {
                  const uint* p = scanLine(y) + cropL;
                  bool bits = false;
                  for (int x = cropL; x < x2; ++x) {
                        if (*p) {
                              bits = true;
                              break;
                              }
                        ++p;
                        }
                  if (bits)
                        break;
                  }
            }
      }

//---------------------------------------------------------
//    deSkew
//---------------------------------------------------------

void OmrPage::deSkew()
      {
      int wl    = wordsPerLine();
      int h     = height();
      uint* db  = new uint[wl * h];
      memset(db, 0, wl * h * sizeof(uint));

      foreach(const QRect& r, _slices) {
            double rot = skew(r);

            // rot = 0.0;
            printf("rot %f\n", rot);

            if (rot == 0.0) {
                  memcpy(db + wl * r.y(), scanLine(r.y()), wl * r.height() * sizeof(uint));
                  continue;
                  }

            QTransform t;
            t.rotate(rot);
            QTransform tt = QImage::trueMatrix(t, width(), r.height());

            double m11 = tt.m11();
            double m12 = tt.m12();
            double m21 = tt.m21();
            double m22 = tt.m22();
            double dx  = tt.m31();
            double dy  = tt.m32();

            double m21y = r.y() * m21;
            double m22y = r.y() * m22;
            int y2 = r.y() + r.height();
            for (int y = r.y(); y < y2; ++y) {
                  const uint* s = scanLine(y);
                  m21y += m21;
                  m22y += m22;
                  for (int x = 0; x < wl; ++x) {
                        uint c = *s++;
                        if (c == 0)
                              continue;
                        uint mask = 1;
                        for (int xx = 0; xx < 32; ++xx) {
                              if (c & mask) {
                                    int xs  = x * 32 + xx;
                                    int xd  = lrint(m11 * xs + m21y + dx);
                                    int yd  = lrint(m22y + m12 * xs + dy);
                                    uint* d = db + wl * yd + (xd / 32);
                                    *d |= (0x1 << (xd % 32));
                                    }
                              mask <<= 1;
                              }
                        }
                  }
            }
      memcpy(_image.bits(), db, wl * h * sizeof(uint));
//??crash      delete[] db;
      }

struct ScanLine {
      int run;
      int x1, x2;
      ScanLine() { run = 0; x1 = 100000; x2 = 0; }
      };

struct H {
      int y;
      int bits;

      H(int a, int b) : y(a), bits(b) {}
      };

//---------------------------------------------------------
//   xproject
//---------------------------------------------------------

int OmrPage::xproject(const uint* p, int wl)
      {
      int run = 0;
      int w   = wl - cropL - cropR;
      int x1 = cropL + w/4;         // only look at part of page
      int x2 = x1 + w/2;
      for (int x = cropL; x < x2; ++x) {
            uint v = *p++;
            run += bitsSetTable[v & 0xff]
               + bitsSetTable[(v >> 8) & 0xff]
               + bitsSetTable[(v >> 16) & 0xff]
               + bitsSetTable[v >> 24];
            }
      return run;
      }

//---------------------------------------------------------
//   xproject2
//---------------------------------------------------------

double OmrPage::xproject2(int y1)
      {
      int wl          = wordsPerLine();
      const uint* db  = bits();
      double val      = 0.0;

      int w  = wl - cropL - cropR;
      int x1 = (cropL + w/4)*32;         // only look at part of page
      int x2 = x1 + (w/2 * 32);

      int ddx = x2 - x1;
      for (int dy = -12; dy < 12; ++dy) {
            int onRun   = 0;
            int offRun  = 0;
            int on      = 0;
            int off     = 0;
            bool onFlag = false;
            int incy    = (dy > 0) ? 1 : (dy < 0) ? -1 : 0;
            int ddy     = dy < 0 ? -dy : dy;
            int y       = y1;
            if (y < 1)
                  y = 0;
            int err     = ddx / 2;
            for (int x = x1; x < x2;) {
                  const uint* d  = db + wl * y + (x / 32);
                  bool bit = ((*d) & (0x1 << (x % 32)));
                  bit = bit || ((*(d+wl)) & (0x1 << (x % 32)));
                  bit = bit || ((*(d-wl)) & (0x1 << (x % 32)));
                  if (bit != onFlag) {
                        if (!onFlag) {
                              //
                              // end of offrun:
                              //
                              if (offRun > 20) {
                                    off += offRun * offRun;
                                    on  += onRun * onRun;
                                    onRun  = 0;
                                    offRun = 0;
                                    }
                              else {
                                    onRun += offRun;
                                    offRun = 0;
                                    }
                              }
                        onFlag = bit;
                        }
                  (bit ? onRun : offRun)++;
                  if (offRun > 100) {
                        offRun = 0;
                        off   = 1;
                        on    = 0;
                        onRun = 0;
                        break;
                        }
                  err -= ddy;
                  if (err < 0) {
                        err += ddx;
                        y   += incy;
                        if (y < 1)
                              y = 1;
                        else if (y >= height())
                              y = height()-1;
                        }
                  ++x;
                  }
            if (offRun > 20)
                  off += offRun * offRun;
            else
                  onRun += offRun;
            on  += onRun * onRun;
            if (off == 0)
                  off = 1;
            double nval = double(on) / double(off);
            if (nval > val)
                  val = nval;
            }
      return val;
      }

static bool sortLvStaves(const Lv& a, const Lv& b)
      {
      return a.line < b.line;
      }

//---------------------------------------------------------
//   getStaffLines
//---------------------------------------------------------

void OmrPage::getStaffLines()
      {
      int h  = height();
      int wl = wordsPerLine();
      int y1 = cropT;
      if (y1 < 1)
            y1 = 1;
      int y2 = h - cropB;
      if (y2 >= h)
            --y2;

      double projection[h];
      for (int y = 0; y < y1; ++y)
            projection[y] = 0;
      for (int y = y2; y < h; ++y)
            projection[y] = 0;
// printf("y1 %d y2 %d  h %d\n", y1, y2, h);
      for (int y = y1; y < y2; ++y)
            projection[y] = xproject2(y);

      int autoTableSize = (wl * 32) / 10;       // 1/10 page width
      if (autoTableSize > y2-y1)
            autoTableSize = y2 - y1;
      double autoTable[autoTableSize];
      memset(autoTable, 0, sizeof(autoTable));
      for (int i = 0; i < autoTableSize; ++i) {
            autoTable[i] = covariance(projection+y1, projection+i+y1, y2-y1-i);
            }
      //
      // search for first maximum in covariance starting at 10 to skip
      // line width. Staff line distance (spatium) must be at least 10 dots
      //
      double maxCorrelation = 0;
      _spatium = 0;
      for (int i = 10; i < autoTableSize; ++i) {
            if (autoTable[i] > maxCorrelation) {
                  maxCorrelation = autoTable[i];
                  _spatium = i;
                  }
            }
      if (_spatium == 0) {
            printf("*** no staff lines found\n");
            return;
            }
      printf("*** spatium = %f\n", _spatium);

      //---------------------------------------------------
      //    look for staves
      //---------------------------------------------------

      QList<Lv> lv;
      int ly = 0;
      int lval = -1000.0;
      for (int y = y1; y < (y2 - _spatium * 4); ++y) {
            double val = 0.0;
            for (int i = 0; i < 5; ++i)
                  val += projection[y + i * int(_spatium)];
            if (val < lval) {
                  lv.append(Lv(ly, lval));
//                  lines.append(HLine(0, width(), ly)); // debug
                  }
            lval = val;
            ly   = y;
            }
      qSort(lv);

//      for (int i = 0; i < lv.size(); ++i) {
//            printf("%d  %f\n", lv[i].line, lv[i].val);
//            lines.append(HLine(0, width(), lv[i].line)); // debug
//            }

      QList<Lv> staveTop;
      int staffHeight = _spatium * 6;
      foreach(Lv a, lv) {
            if (a.val < 500)   // MAGIC to avoid false positives
                  continue;
            int line = a.line;
            bool ok  = true;
            foreach(Lv b, staveTop) {
                  if ((line > (b.line - staffHeight)) && (line < (b.line + staffHeight))) {
                        ok = false;
                        break;
                        }
                  }
            if (ok) {
                  staveTop.append(a);
//                  lines.append(HLine(0, width(), a.line)); // debug
                  }
            }
      qSort(staveTop.begin(), staveTop.end(), sortLvStaves);
      foreach(Lv a, staveTop) {
            staves.append(QRectF(cropL * 32, a.line, width() - cropR*32, _spatium*4));
            }
      }

struct Hv {
      int x;
      int val;
      Hv(int a, int b) : x(a), val(b) {}
      bool operator< (const Hv& a) const {
            return a.val < val;
            }
      };

//---------------------------------------------------------
//   searchNotes
//---------------------------------------------------------

void OmrPage::searchNotes(int line, int x1, int x2, int y)
      {
      QList<Hv> val;

      int w = _spatium * 1.3;
      x1 += w/2;
      x2 -= w/2;
      bool on = false;
      int xx1;

      for (int x = x1; x < x2; ++x) {
            if (dot(x, y)) {
                  if (!on) {
                        on = true;
                        xx1 = x;
                        }
                  }
            else {
                  if (on) {
                        int w = x - xx1;
                        if (w > _spatium && w < (_spatium*1.5))
                              _notes.append(QRect(xx1, y-_spatium*.5, w, _spatium));
                        on = false;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   staffDistance
//---------------------------------------------------------

double OmrPage::staffDistance() const
      {
      if (staves.size() < 2)
            return 5;
      return ((staves[1].y() - staves[0].y()) / _spatium) - 4.0;
      }

//---------------------------------------------------------
//   systemDistance
//---------------------------------------------------------

double OmrPage::systemDistance() const
      {
      if (staves.size() < 3)
            return 6.0;
      return ((staves[2].y() - staves[1].y()) / _spatium) - 4.0;
      }

