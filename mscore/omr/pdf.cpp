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
      QImage* image;
      int ny;

   public:
      QImageOutputDev()           { image = 0;   }
      void setImage(QImage* img)  { image = img; }
      virtual ~QImageOutputDev()  {}

      virtual GBool interpretType3Chars() { return gFalse; }
      virtual GBool needNonText()         { return gTrue; }

      // 0,0 is top left corner
      virtual GBool upsideDown()          { return gFalse; }

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
            double* ctm = state->getCTM();
            double xmag = width/ctm[0];
            double ymag = height/ctm[3];
            double xoff = (ctm[2]+ctm[4]) * xmag;
            int ph      = state->getPageHeight() * ymag;
            int yoff    = int(ph - (ctm[3]+ctm[5]) * ymag);

printf("Image %4d %4d   at %5f %4d   mag %f %f\n",
   width, height, xoff, yoff, xmag, ymag);

            bool invertBits = colorMap->getDecodeLow(0) == 0.0;

            if (image->isNull()) {
                  int pw = state->getPageWidth() * xmag;
                  pw     = ((pw+31)/32) * 32;
                  int ph = state->getPageHeight() * ymag;
                  // *image = QImage(pw, ph+32, QImage::Format_MonoLSB);
                  *image = QImage(pw, ph, QImage::Format_MonoLSB);
                  QVector<QRgb> ct(2);
                  ct[0] = qRgb(255, 255, 255);
                  ct[1] = qRgb(0, 0, 0);
                  image->setColorTable(ct);
                  image->fill(0);
                  ny = yoff;
                  }

            // copy the stream
            str->reset();     // initialize stream

            if (yoff != ny)
                  printf("  ***next image does not fit, gap %d\n", yoff - ny);
            yoff = ny;

            int stride  = (width + 7) / 8;
            uchar mask  = 0xff << (stride * 8 - width);
            int qstride = image->bytesPerLine();
            uchar* p    = image->bits();
            for (int y = 0; y < height; ++y) {
                  p = image->scanLine(y + yoff);
                  for (int x = 0; x < stride; ++x) {
                        static unsigned char invert[16] = {
                              0, 8, 4, 12, 2, 10, 6, 14, 1, 9, 5, 13, 3, 11, 7, 15
                              };
                        uchar c = str->getChar();
                        if (invertBits)
                              c = ~c;
                        *p++ = (invert[c & 15] << 4) | invert[(c >> 4) & 15];
                        }
                  p[-1] &= ~mask;
                  }
            str->close();
            ny += height;
            }
      else {
printf("Color Image ================%d %d\n", width, height);
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
      _doc->displayPage(imgOut, i+1, 1200.0, 1200.0, 0, gTrue, gFalse, gFalse);
      return image;
      }

