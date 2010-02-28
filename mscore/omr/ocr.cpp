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

#include "ocr.h"

//---------------------------------------------------------
//   Ocr
//---------------------------------------------------------

Ocr::Ocr()
      {
      }

//---------------------------------------------------------
//   recognize
//---------------------------------------------------------

QList<OcrResult> Ocr::recognize(const OcrImage& img) const
      {
      QList<OcrResult> rl;

      FeatureVector fv;
      createFeatureVector(&fv, img);
      foreach(OcrDictValue dv, dictionary)
            rl.append(OcrResult(dv.c, dv.f.compare(fv)));
      qSort(rl);
      printf("%d %d <%c> ", img.r.width(), img.r.height(), rl.front().c.toLatin1());
      fv.dump("");
      return rl;
      }

//---------------------------------------------------------
//   learn
//---------------------------------------------------------

void Ocr::learn(const QChar& c, const OcrImage& img)
      {
      FeatureVector fv;
      createFeatureVector(&fv, img);
      dictionary.append(OcrDictValue(c, fv));
      printf("Learn: %c ", c.toLatin1());
      fv.dump("");
      }

//---------------------------------------------------------
//   compare
//---------------------------------------------------------

int FeatureVector::compare(const FeatureVector& v) const
      {
      int diff = 0;
      for (int i = 0; i < FeatureVectorSize; ++i) {
            int val = values[i] - v.values[i];
            diff += val * val;
            }
      return diff;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void FeatureVector::dump(const char* msg) const
      {
      printf("%s:", msg);
      for (int i = 0; i < FeatureVectorSize; ++i)
            printf(" %d", values[i]);
      printf("\n");
      }

//---------------------------------------------------------
//   ratio
//---------------------------------------------------------

int ratio(int a, int b)
      {
      if (b == 0)
            return 127;
      if (a == 0)
            return -128;
      int r = (a > b) ? (a*32 / b) : (b*-8 / a);
      if (r > 127)
            return 127;
      if (r < -128)
            return -128;
      return r;
      }

//---------------------------------------------------------
//   createFeatureVector
//---------------------------------------------------------

void Ocr::createFeatureVector(FeatureVector* fv, const OcrImage& img) const
      {
      memset(fv, 0, FeatureVectorSize);

      //
      //    0     width/height ratio
      //
      const QRect r = img.r;
      int w = r.width();
      int h = r.height();
      fv->values[0] = ratio(w, h);

      //
      //    darkness4
      //
      int w2 = w/xstripes;
      int h2 = h/ystripes;
      for (int xk = 0; xk < xstripes; ++xk) {
            for (int yk = 0; yk < ystripes; ++yk) {
                  int x1 = r.x() + xk * w2;
                  int x2 = x1 + w2;
                  int y1 = r.y() + yk * h2;
                  int y2 = y1 + h2;
                  int b  = 0;
                  for (int x = x1; x < x2; ++x) {
                        for (int y = y1; y < y2; ++y) {
                              if (img.dot(x, y))
                                    ++b;
                              }
                        }
                  double g = double(b)/double(w2 * h2);
                  fv->values[1 + xk * ystripes + yk] = uchar(g * 255.0);
                  }
            }
      }

//---------------------------------------------------------
//   crop
//---------------------------------------------------------

OcrImage OcrImage::crop() const
      {
      OcrImage di;
      di.stride = stride;
      di.image  = image;

      int cb = 0;
      if (r.width() < 32) {
            uint mask = ~0;
            int x1 = r.x();
            int x2 = x1 + r.width();
            int s1 = x1 % 32;
            int s2 = 32-(x2 % 32);
            mask <<= x1 + x2;
            mask >>= x2;
            uint* p = image + (r.y() + r.height() - 1) * stride;
            for (int i = 0; i < r.height(); ++i) {
                  if (*p & mask) {
                        cb = i;
                        break;
                        }
                  p -= stride;
                  }
            }
      else if ((r.width() == 32) && ((r.x() % 32) == 0)) {
            uint* p = image + (r.y() + r.height() - 1) * stride;
            for (int i = 0; i < r.height(); ++i) {
                  if (*p) {
                        cb = i;
                        break;
                        }
                  p -= stride;
                  }
            }

      else {
            }
      di.r = r;
      di.r.setHeight(r.height() - cb);
      return di;
      }

//---------------------------------------------------------
//   learn
//---------------------------------------------------------

void Ocr::learn(const QChar& c)
      {
      QFont font("Century Schoolbook L");
      font.setPixelSize(24 * 4);
      QFontMetrics fm(font);
      int y = fm.ascent();

      int width  = fm.width(c);
      int height = y * 2;

      QImage image = QImage(width, height, QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      image.setColorTable(ct);
      image.fill(0);

      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);

      painter.setFont(font);
      painter.setPen(Qt::black);
      QRect rect;
      painter.drawText(0, y, QString(c));
      painter.end();

      OcrImage img(image.bits(), QRect(0, 0, fm.width(c), y), (width + 31)/32);
      learn(c, img);
      }

//---------------------------------------------------------
//   test
//---------------------------------------------------------

void Ocr::test(const QChar& c)
      {
      int width  = 32;
      int height = 32;
      QImage image = QImage(width, height, QImage::Format_MonoLSB);
      QVector<QRgb> ct(2);
      ct[0] = qRgb(255, 255, 255);
      ct[1] = qRgb(0, 0, 0);
      image.setColorTable(ct);
      image.fill(0);

      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      QFont font("Century Schoolbook L");
      font.setPixelSize(24);
      QFontMetrics fm(font);
      int y = fm.ascent();

      painter.setFont(font);
      painter.setPen(Qt::black);
      QRect rect;
      painter.drawText(0, y, QString(c));
      painter.end();
      OcrImage img(image.bits(), QRect(0, 0, fm.width(c), y), (width + 31)/32);


      QList<OcrResult> rl = recognize(img);
      printf("Test <%c>\n", c.toLatin1());
      int i = 0;
      foreach(OcrResult r, rl) {
            printf("  <%c> %d\n", r.c.toLatin1(), r.matchVal);
            if (++i >= 4)
                  break;
            }
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Ocr::init()
      {
      static const char testData[] = "InvetioC-DurBWV0123456789JhaSbsc";
      printf("ocr init\n");

      int n = sizeof(testData)/sizeof(*testData) - 1;
      for (int i = 0; i < n; ++i)
            learn(QChar(testData[i]));
//      for (int i = 0; i < n; ++i)
//            test(QChar(testData[i]));
      }

//---------------------------------------------------------
//   readLine
//---------------------------------------------------------

QString Ocr::readLine(const OcrImage& img, QList<QRect>* rl)
      {
      QString s;
      QRect r(img.r);
      QVector<int> vp(r.width());

      int x1 = r.x();
      int x2 = x1 + r.width();
      int y1 = r.y();
      int y2 = y1 + r.height();

      for (int x = x1; x < x2; ++x) {
            int vBits = 0;
            for (int y = y1; y < y2; ++y) {
                  if (img.dot(x, y))
                        ++vBits;
                  }
            vp[x - x1] = vBits;
            }
      bool onMode = vp[0] != 0;
      int xb1 = 0;
      QList<QRect> bl;
      for (int x = 0; x < r.width(); ++x) {
            if (vp[x] > 0) {
                  if (!onMode) {
                        onMode = true;
                        xb1 = x;
                        }
                  }
            else {
                  if (onMode) {
                        onMode = false;
                        bl.append(QRect(xb1+x1, y1, x - xb1, r.height()));
                        }
                  }
            }
      printf("%d %d %d %d -- %d character boxes found\n", r.x(), r.y(), r.width(), r.height(), bl.size());
      OcrImage i(img);
      foreach(QRect r, bl) {
            i.r = r;
            QList<OcrResult> result = recognize(i);
            printf("  ocr <%c>\n", result.front().c.toLatin1());
            s += result.front().c;
            }
      if (rl)
            *rl = bl;
      return s;
      }

