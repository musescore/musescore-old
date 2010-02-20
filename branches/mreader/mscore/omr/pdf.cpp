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
#include <PDFDoc.h>
#include <OutputDev.h>
#include <GlobalParams.h>
#include <GfxState.h>

//------------------------------------------------------------------------
// QImageOutputDev
//------------------------------------------------------------------------

class QImageOutputDev: public OutputDev {
      int imgNum;			// current image number
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
/*      virtual void drawMaskedImage(GfxState *state, Object *ref, Stream *str,
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
      */
      };

//---------------------------------------------------------
//   drawImage
//---------------------------------------------------------

void QImageOutputDev::drawImage(GfxState*, Object*, Stream* str,
   int width, int height,
   GfxImageColorMap *colorMap,
   GBool, int*, GBool)
      {
      if (colorMap->getNumPixelComps() == 1 && colorMap->getBits() == 1) {
            // fprintf(f, "%d %d\n", width, height);
            str->reset();     // initialize stream

            // copy the stream
            int stride  = (width + 7) / 8;
            unsigned char mask = 0xff << (stride * 8 - width);
            int qstride = ((width + 31) / 32) * 4;
            // int qsize   = height * qstride;

            *image = QImage(width, height, QImage::Format_MonoLSB);

            const uchar* data = image->bits();

            uchar* p = const_cast<uchar*>(data);
            for (int y = 0; y < height; ++y) {
                  int x = 0;
                  for (; x < stride; ++x) {
                        static unsigned char invert[16] = {
                              0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
                              };
                        unsigned char c = ~(str->getChar());
                        *p++ = (invert[c & 15] << 4) | invert[(c >> 4) & 15];
                        }
                  p[-1] &= ~mask;
                  for (; x < qstride; ++x)
                        *p++ = 0;
                  }

            str->close();
            QVector<QRgb> ct(2);
            ct[0] = qRgb(255, 255, 255);
            ct[1] = qRgb(0, 0, 0);
            image->setColorTable(ct);
            }
      else {
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
      _doc->displayPages(imgOut, i+1, i+1, 72, 72, 0, gTrue, gFalse, gFalse);
      return image;
      }

