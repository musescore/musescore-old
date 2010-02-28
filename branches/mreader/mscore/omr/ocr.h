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

#ifndef __OCR_H__
#define __OCR_H__

//---------------------------------------------------------
//   OcrResult
//---------------------------------------------------------

struct OcrResult {
      QChar c;
      int matchVal;
      OcrResult() {};
      OcrResult(const QChar& _c, int mv) : c(_c), matchVal(mv) {}
      bool operator<(const OcrResult& r) const { return matchVal < r.matchVal; }
      };

//---------------------------------------------------------
//   OcrImage
//---------------------------------------------------------

struct OcrImage {
      uint* image;
      QRect r;
      int stride; // uint* stride

      OcrImage() {}
      OcrImage(const uchar* p, const QRect& _r, int _s) : image((uint*)p), r(_r), stride(_s) {}
      OcrImage crop() const;
      bool dot(int x, int y) const {
            return (*(image + (y * stride) + (x / 32))) & (0x1 << (x % 32));
            }
      };

//---------------------------------------------------------
//   FeatureVector
//---------------------------------------------------------

static const int xstripes = 8;
static const int ystripes = 8;
static const int FeatureVectorSize = xstripes*ystripes+1;

struct FeatureVector {
      uchar values[FeatureVectorSize];

      int compare(const FeatureVector& v) const;
      void dump(const char*) const;
      };

//---------------------------------------------------------
//   OcrDictValue
//---------------------------------------------------------

struct OcrDictValue {
      QChar c;
      FeatureVector f;
      OcrDictValue() {}
      OcrDictValue(const QChar& _c, const FeatureVector& _f) : c(_c), f(_f) {}
      };

//---------------------------------------------------------
//   Ocr
//---------------------------------------------------------

class Ocr {
      QList<OcrDictValue> dictionary;

      void createFeatureVector(FeatureVector*, const OcrImage&) const;
      void learn(const QChar& c);
      void learn(const QChar& c, const OcrImage&);
      void test(const QChar& c);

   public:
      Ocr();
      QList<OcrResult> recognize(const OcrImage&) const;
      void init();
      QString readLine(const OcrImage&, QList<QRect>* rl = 0);
      };

#endif

