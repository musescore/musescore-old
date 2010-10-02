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

#include "pdf.h"
#include "poppler/PDFDoc.h"
#include "poppler/OutputDev.h"
#include "poppler/GlobalParams.h"
#include "poppler/GfxState.h"

//------------------------------------------------------------------------
// QImageOutputDev
//------------------------------------------------------------------------

class QImageOutputDev: public OutputDev {
      int iy;
      QImage* image;

   public:
      QImageOutputDev()           { image = 0;   }
      void setImage(QImage* img)  { image = img; }
      virtual ~QImageOutputDev()  {}

      virtual GBool interpretType3Chars() { return gFalse; }
      virtual GBool needNonText()         { return gTrue; }
      virtual GBool upsideDown()          { return gTrue; }
      virtual GBool useDrawChar()         { return gFalse; }

  //----- image drawing
      virtual void drawImage(GfxState *state, Object *ref, Stream *str,
	   int width, int height, GfxImageColorMap *colorMap,
	   GBool interpolate, int *maskColors, GBool inlineImg);
      virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
         int width, int height,
	   GfxImageColorMap *colorMap,
	   GBool interpolate,
	   Stream *maskStr, int maskWidth, int maskHeight,
	   GBool maskInvert, GBool maskInterpolate);
      virtual void drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
	   int width, int height,
	   GfxImageColorMap *colorMap,
	   GBool interpolate,
	   Stream *maskStr,
	   int maskWidth, int maskHeight,
	   GfxImageColorMap *maskColorMap,
	   GBool maskInterpolate);
      };

void QImageOutputDev::drawMaskedImage(GfxState *state, Object *ref, Stream *str,
         int width, int height,
	   GfxImageColorMap *colorMap,
	   GBool interpolate,
	   Stream *maskStr, int maskWidth, int maskHeight,
	   GBool maskInvert, GBool maskInterpolate)
      {
      printf("=========drawMaskedImage\n");
      abort();
      }

void QImageOutputDev::drawSoftMaskedImage(GfxState *state, Object *ref, Stream *str,
	   int width, int height,
	   GfxImageColorMap *colorMap,
	   GBool interpolate,
	   Stream *maskStr,
	   int maskWidth, int maskHeight,
	   GfxImageColorMap *maskColorMap,
	   GBool maskInterpolate)
      {
      printf("==========drawSoftMaskedImage\n");
      abort();
      }

//---------------------------------------------------------
//   drawImage
//---------------------------------------------------------

void QImageOutputDev::drawImage(GfxState* state, Object*, Stream* str,
   int width, int height, GfxImageColorMap* colorMap, GBool, int*, GBool)
      {
      if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 1) {
printf("Image %d %d\n", width, height);
            bool invertBits = colorMap->getDecodeLow(0) == 0.0;
            str->reset();     // initialize stream

            double* ctm = state->getCTM();
            QMatrix matrix;

            matrix.setMatrix(
              ctm[0] / width,   0,
              0,                 -ctm[3] / height,
              ctm[4],  ctm[3] + ctm[5]);

            int pw = state->getPageWidth() / matrix.m11();
            pw = ((pw+31)/32) * 32;
            int ph = state->getPageHeight() / matrix.m22();
            if (image->isNull()) {
                  // *image = QImage(pw, ph+32, QImage::Format_MonoLSB);
                  *image = QImage(width, height, QImage::Format_MonoLSB);
                  QVector<QRgb> ct(2);
                  ct[0] = qRgb(255, 255, 255);
                  ct[1] = qRgb(0, 0, 0);
                  image->setColorTable(ct);
                  image->fill(0);
                  iy = 0;
                  }
/*            if (image->width() < pw || image->height() < ph) {
                  printf("**********drop image %d x %d  <  %d x %d\n",
                     image->width(), image->height(), pw, ph);
                  return;
                  }
  */
            // copy the stream
            int stride  = (width + 7) / 8;
            uchar mask  = 0xff << (stride * 8 - width);
            int qstride = image->bytesPerLine();
            uchar* p    = image->bits();
            for (int y = 0; y < height; ++y) {
                  p = image->scanLine(y);
                  int x = 0;
                  for (; x < stride; ++x) {
                        static unsigned char invert[16] = {
                              0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
                              };
                        unsigned char c = str->getChar();
                        if (invertBits)
                              c = ~c;
                        *p++ = (invert[c & 15] << 4) | invert[(c >> 4) & 15];
                        }
                  p[-1] &= ~mask;

//                  for (; x < qstride; ++x)
//                        *p++ = 0;
                  }
            iy += height;

            str->close();
            }
      else {
printf("Color Image %d %d\n", width, height);
#if 0
            fprintf(f, "P6\n");
            fprintf(f, "%d %d\n", width, height);
            fprintf(f, "255\n");

            // initialize stream
            ImageStream* imgStr = new ImageStream(str, width, colorMap->getNumPixelComps(), colorMap->getBits());
            imgStr->reset();

            // for each line...
            for (int y = 0; y < height; ++y) {

                  // write the line
                  Guchar *p = imgStr->getLine();
                  for (int x = 0; x < width; ++x) {
                        GfxRGB rgb;
                        colorMap->getRGB(p, &rgb);
                        fputc(colToByte(rgb.r), f);
                        fputc(colToByte(rgb.g), f);
                        fputc(colToByte(rgb.b), f);
                        p += colorMap->getNumPixelComps();
                        }
                  }
            imgStr->close();
            delete imgStr;
#endif
            }
      }

//---------------------------------------------------------
//   numPages
//---------------------------------------------------------

int Pdf::numPages() const
      {
      return _doc->getNumPages();
      }

//---------------------------------------------------------
//   Pdf
//---------------------------------------------------------

Pdf::Pdf(const QString& path)
      {
      globalParams        = new GlobalParams();
      GooString* fileName = new GooString(qPrintable(path));
      _doc = new PDFDoc(fileName, 0, 0);
      imgOut = 0;
      }

//---------------------------------------------------------
//   page
//---------------------------------------------------------

QImage Pdf::page(int i)
      {
      if (imgOut == 0)
            imgOut = new QImageOutputDev();
      QImage image;
      imgOut->setImage(&image);
      // useMediaBox, crop, printing
      _doc->displayPage(imgOut, i+1, 72, 72, 0, gTrue, gFalse, gFalse);
      return image;
      }

