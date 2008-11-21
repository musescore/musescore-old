//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: symbols.cpp,v 1.34 2006/04/06 13:03:11 wschweer Exp $
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
#include "style.h"
#include "sym.h"
#include "spatium.h"
#include "utils.h"
#include "mscore.h"
#include "score.h"

QVector<Sym> symbols(lastSym);

QMap<const char*, SymCode*> charReplaceMap;

SymCode pSymbols[] = {
      SymCode(0xe10e, TEXT_STYLE_DYNAMICS1),    // sharp
      SymCode(0xe112, TEXT_STYLE_DYNAMICS1),    // flat
      SymCode(0xe102, TEXT_STYLE_DYNAMICS1),    // note2_Sym
      SymCode(0xe0fc, TEXT_STYLE_DYNAMICS1),    // note4_Sym
      SymCode(0xe0f8, TEXT_STYLE_DYNAMICS1),    // note8_Sym
      SymCode(0xe0f9, TEXT_STYLE_DYNAMICS1),    // note16_Sym
      SymCode(0xe0fa, TEXT_STYLE_DYNAMICS1),    // note32_Sym
      SymCode(0xe0fb, TEXT_STYLE_DYNAMICS1),    // note64_Sym
      SymCode(0xe168, TEXT_STYLE_DYNAMICS1),    // coda
      SymCode(0xe169, TEXT_STYLE_DYNAMICS1),    // varcoda
      SymCode(0xe167, TEXT_STYLE_DYNAMICS1),    // segno
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0xa9,   -1, "(C)", SYMBOL_COPYRIGHT),
      SymCode(0x00c0, -1),
      SymCode(0x00c1, -1),
      SymCode(0x00c2, -1),
      SymCode(0x00c3, -1),
      SymCode(0x00c4, -1),
      SymCode(0x00c5, -1),
      SymCode(0x00c6, -1),
      SymCode(0x00c7, -1),
      SymCode(0x00c8, -1),
      SymCode(0x00c9, -1),
      SymCode(0x00ca, -1),
      SymCode(0x00cb, -1),
      SymCode(0x00cc, -1),
      SymCode(0x00cd, -1),
      SymCode(0x00ce, -1),
      SymCode(0x00cf, -1),

      SymCode(0x00d0, -1),
      SymCode(0x00d1, -1),
      SymCode(0x00d2, -1),
      SymCode(0x00d3, -1),
      SymCode(0x00d4, -1),
      SymCode(0x00d5, -1),
      SymCode(0x00d6, -1),
      SymCode(0x00d7, -1),
      SymCode(0x00d8, -1),
      SymCode(0x00d9, -1),
      SymCode(0x00da, -1),
      SymCode(0x00db, -1),
      SymCode(0x00dc, -1),
      SymCode(0x00dd, -1),
      SymCode(0x00de, -1),
      SymCode(0x00df, -1),

      SymCode(0x00e0, -1),
      SymCode(0x00e1, -1),
      SymCode(0x00e2, -1),
      SymCode(0x00e3, -1),
      SymCode(0x00e4, -1),
      SymCode(0x00e5, -1),
      SymCode(0x00e6, -1),
      SymCode(0x00e7, -1),
      SymCode(0x00e8, -1),
      SymCode(0x00e9, -1),
      SymCode(0x00ea, -1),
      SymCode(0x00eb, -1),
      SymCode(0x00ec, -1),
      SymCode(0x00ed, -1),
      SymCode(0x00ee, -1),
      SymCode(0x00ef, -1),

      SymCode(0x00f0, -1),
      SymCode(0x00f1, -1),
      SymCode(0x00f2, -1),
      SymCode(0x00f3, -1),
      SymCode(0x00f4, -1),
      SymCode(0x00f5, -1),
      SymCode(0x00f6, -1),
      SymCode(0x00f7, -1),
      SymCode(0x00f8, -1),
      SymCode(0x00f9, -1),
      SymCode(0x00fa, -1),
      SymCode(0x00fb, -1),
      SymCode(0x00fc, -1),
      SymCode(0x00fd, -1),
      SymCode(0x00fe, -1),
      SymCode(0x00ff, -1),

      SymCode(0x00BC, -1, "1/4", SYMBOL_FRACTION),
      SymCode(0x00BD, -1, "1/2", SYMBOL_FRACTION),
      SymCode(0x00BE, -1, "3/4", SYMBOL_FRACTION),
      SymCode(0x2153, -1, "1/3", SYMBOL_FRACTION),
      SymCode(0x2154, -1, "2/3", SYMBOL_FRACTION),
      SymCode(0x2155, -1, "1/5", SYMBOL_FRACTION),
      SymCode(0x2156, -1, "2/5", SYMBOL_FRACTION),
      SymCode(0x2157, -1, "3/5", SYMBOL_FRACTION),
      SymCode(0x2158, -1, "4/5", SYMBOL_FRACTION),
      SymCode(0x2159, -1, "1/6", SYMBOL_FRACTION),
      SymCode(0x215A, -1, "5/6", SYMBOL_FRACTION),
      SymCode(0x215B, -1, "1/8", SYMBOL_FRACTION),
      SymCode(0x215C, -1, "3/8", SYMBOL_FRACTION),
      SymCode(0x215D, -1, "5/8", SYMBOL_FRACTION),
      SymCode(0x215E, -1, "7/8", SYMBOL_FRACTION),

      SymCode(-1, -1)    // indicates end
      };

#ifdef __MINGW32__
// #define CTABLE_HACK
#endif

#ifdef CTABLE_HACK
struct CTable {
      int code;
      int font;
      double x, y, w, h;
      double width;
      };
CTable ctable[] = {
      { 56, 2, 8, -91, 51, 92, 67 },
      { 49, 2, 15, -91, 35, 91, 67 },
      { 53, 2, 7, -89, 50, 90, 67 },
      { 57600, 0, 0, 0, 125, 52, 125 },
      { 57601, 0, 0, -52, 125, 52, 125 },
      { 57602, 0, -52, -9, 229, 61, 125 },
      { 57603, 0, -52, -52, 229, 60, 125 },
      { 57605, 0, 0, -84, 50, 168, 50 },
      { 57606, 0, 0, -83, 50, 83, 50 },
      { 57607, 0, -8, -132, 88, 239, 79 },
      { 57609, 0, -1, -69, 85, 156, 83 },
      { 57608, 0, -1, -69, 85, 156, 83 },
      { 57610, 0, -9, -69, 109, 239, 100 },
      { 57611, 0, -18, -152, 127, 322, 109 },
      { 57612, 0, -22, -152, 139, 406, 117 },
      { 57613, 0, -20, -236, 145, 490, 125 },
      { 57604, 0, 0, -84, 150, 168, 150 },
      { 57705, 1, -6, -201, 181, 238, 168 },
      { 57773, 0, 0, -111, 156, 130, 159 },
      { 57774, 0, 0, -18, 156, 130, 159 },
      { 61649, 0, 8, -255, 317, 317, 333 },
      { 48, 0, 0, -167, 122, 167, 123 },
      { 49, 0, -1, -168, 108, 168, 106 },
      { 50, 0, 0, -167, 122, 167, 123 },
      { 51, 0, 0, -167, 111, 167, 111 },
      { 52, 0, -1, -167, 134, 167, 134 },
      { 53, 0, 0, -167, 117, 167, 113 },
      { 54, 0, 0, -167, 113, 167, 114 },
      { 55, 0, 0, -167, 121, 168, 113 },
      { 56, 0, 0, -167, 124, 167, 123 },
      { 57, 0, 0, -167, 113, 167, 114 },
      { 57614, 0, 0, -125, 92, 250, 92 },
      { 57619, 0, -3, -128, 61, 256, 56 },
      { 57620, 0, -9, -156, 76, 210, 67 },
      { 57626, 0, -9, -156, 130, 210, 121 },
      { 57628, 0, -4, -45, 92, 90, 83 },
      { 57629, 0, 12, -88, 38, 176, 50 },
      { 57630, 0, -50, -88, 38, 176, 0 },
      { 57639, 0, 0, -18, 37, 37, 37 },
      { 57642, 0, -13, -46, 192, 92, 165 },
      { 57643, 0, 0, -46, 165, 92, 165 },
      { 57644, 0, 0, -45, 116, 91, 116 },
      { 57645, 0, 0, -46, 110, 92, 110 },
      { 57646, 0, 0, -46, 165, 92, 165 },
      { 57647, 0, 0, -46, 121, 92, 122 },
      { 57632, 0, -7, -41, 90, 83, 83 },
      { 57649, 0, -1, -61, 194, 134, 193 },
      { 57650, 0, 0, -55, 140, 121, 140 },
      { 57652, 0, 0, -55, 116, 121, 116 },
      { 57654, 0, 0, -87, 252, 175, 252 },
      { 57655, 0, 0, -87, 196, 175, 196 },
      { 57656, 0, 0, -88, 143, 176, 143 },
      { 57657, 0, 0, -50, 143, 100, 143 },
      { 57658, 0, 0, -48, 126, 97, 127 },
      { 57659, 0, 0, -46, 110, 92, 110 },
      { 57660, 0, 0, -57, 130, 115, 131 },
      { 57688, 0, -111, -120, 223, 126, 111 },
      { 57689, 0, -111, -7, 223, 128, 111 },
      { 57696, 0, -33, -42, 66, 109, 33 },
      { 57697, 0, -75, -42, 150, 84, 75 },
      { 57698, 0, -159, -43, 318, 85, 159 },
      { 57699, 0, -17, -16, 33, 33, 17 },
      { 57700, 0, -17, -84, 33, 88, 17 },
      { 57701, 0, -17, -5, 34, 88, 17 },
      { 57702, 0, -50, -6, 100, 13, 50 },
      { 57703, 0, -50, -69, 101, 75, 50 },
      { 57704, 0, -50, -6, 101, 74, 50 },
      { 57705, 0, -42, -92, 83, 92, 42 },
      { 57706, 0, -41, 0, 83, 92, 42 },
      { 57707, 0, -33, -42, 66, 84, 33 },
      { 57708, 0, -46, -47, 93, 93, 46 },
      { 57709, 0, -54, -174, 109, 174, 54 },
      { 57710, 0, -63, -111, 125, 111, 63 },
      { 57711, 0, -91, -44, 182, 88, 91 },
      { 57712, 0, -91, -44, 182, 88, 91 },
      { 57713, 0, -107, -181, 199, 185, 71 },
      { 57714, 0, -42, -56, 84, 98, 42 },
      { 57715, 0, -42, -42, 84, 98, 42 },
      { 57716, 0, -42, -125, 84, 125, 42 },
      { 57717, 0, -42, 0, 84, 125, 42 },
      { 57718, 0, -44, -45, 89, 89, 44 },
      { 57703, 1, 0, -252, 167, 253, 167 },
      { 57704, 1, 0, -200, 179, 236, 145 },
      { 57722, 0, -1, -52, 39, 103, 42 },
      { 57723, 0, -37, -51, 39, 103, 0 },
      { 57726, 0, 0, -102, 67, 121, 67 },
      { 57727, 0, -19, -67, 121, 67, 83 },
      { 57728, 0, -20, -102, 107, 103, 67 },
      { 57729, 0, -20, -84, 107, 103, 67 },
      { 57730, 0, -40, -41, 81, 83, 35 },
      { 57731, 0, -84, -41, 168, 83, 70 },
      { 57732, 0, -84, -55, 167, 111, 70 },
      { 57733, 0, -119, -41, 238, 83, 104 },
      { 57734, 0, -119, -55, 238, 111, 104 },
      { 57735, 0, -131, -42, 250, 144, 104 },
      { 57738, 0, -131, -78, 250, 120, 104 },
      { 57736, 0, -131, -56, 250, 158, 104 },
      { 57739, 0, -131, -79, 250, 135, 104 },
      { 57741, 0, -119, -162, 237, 204, 104 },
      { 57737, 0, -119, -42, 250, 144, 104 },
      { 57740, 0, -119, -78, 250, 120, 104 },
      { 57744, 0, -5, 0, 75, 251, 75 },
      { 57745, 0, -5, 0, 74, 293, 75 },
      { 57746, 0, -5, 0, 75, 359, 70 },
      { 57747, 0, -5, 0, 75, 442, 70 },
      { 57748, 0, -5, -242, 95, 242, 90 },
      { 57749, 0, -58, 75, 136, 113, 70 },
      { 57750, 0, -73, -180, 171, 106, 90 },
      { 57751, 0, -5, -251, 94, 251, 90 },
      { 57752, 0, -5, -325, 95, 325, 90 },
      { 57753, 0, -5, -367, 95, 367, 90 },
      { 61583, 0, 28, -241, 241, 241, 297 },
      { 61584, 0, 28, -241, 241, 241, 297 },
      { 57754, 0, 0, -167, 227, 334, 227 },
      { 57755, 0, 0, -133, 183, 266, 183 },
      { 57756, 0, -5, -88, 224, 260, 224 },
      { 57757, 0, -5, -71, 181, 209, 179 },
      { 57758, 0, 0, -416, 213, 634, 214 },
      { 57759, 0, 0, -333, 171, 509, 171 },
      { 57760, 0, 56, -84, 110, 168, 167 },
      { 57761, 0, 45, -66, 88, 133, 134 },
      { 57762, 0, 15, -241, 218, 482, 234 },
      { 57763, 0, 11, -193, 176, 385, 187 },
      { 57764, 0, 0, -87, 145, 175, 142 },
      { 57765, 0, 0, -117, 145, 234, 142 },
      { 57766, 0, 0, -167, 130, 130, 130 },
      { 57767, 0, 0, -79, 70, 28, 70 },
      { 57768, 0, 0, -23, 23, 23, 23 },
      { 57769, 0, 0, -167, 140, 171, 139 },
      { 57770, 0, -2, -147, 114, 147, 111 },
      { 57771, 0, 0, -96, 69, 96, 67 },
      { 57772, 0, 0, -167, 267, 171, 266 },
      { 57775, 0, -130, -258, 261, 258, 131 },
      { 57776, 0, -21, -20, 41, 41, 21 },
      { 57777, 0, -89, -175, 178, 175, 89 },
      { 57778, 0, -172, -341, 344, 341, 173 },
      { 57779, 0, -89, -261, 178, 261, 89 },
      { 61626, 0, 28, -241, 241, 241, 297 },
      { 61627, 0, 28, -241, 241, 241, 297 },
      { 57780, 0, -89, -175, 178, 175, 89 },
      { 61629, 0, 28, -241, 241, 241, 297 },
      { 61630, 0, 28, -241, 241, 241, 297 },
      { 61631, 0, 28, -241, 241, 241, 297 },
      { 61632, 0, 28, -241, 241, 241, 297 },
      { 61633, 0, 28, -241, 241, 241, 297 },
      { 61634, 0, 28, -241, 241, 241, 297 },
      { 61635, 0, 28, -263, 240, 293, 297 },
      { 61636, 0, 28, -263, 240, 293, 297 },
      { 61637, 0, 29, -263, 240, 293, 297 },
      { 61638, 0, 29, -263, 240, 293, 297 },
      { 61639, 0, 28, -241, 293, 240, 349 },
      { 61640, 0, 28, -240, 293, 240, 349 },
      { 61641, 0, 28, -241, 293, 240, 349 },
      { 61642, 0, 28, -240, 293, 240, 349 },
      { 61643, 0, 8, -255, 317, 319, 333 },
      { 61644, 0, 0, -264, 334, 334, 333 },
      { 61645, 0, 8, -255, 317, 317, 333 },
      { 61646, 0, 8, -255, 317, 317, 333 },
      { 61647, 0, 8, -255, 317, 317, 333 },
      { 61648, 0, 8, -255, 317, 317, 333 },
      { 102, 1, -34, -158, 179, 217, 107 },
      { 109, 1, -15, -102, 168, 108, 146 },
      { 112, 1, -45, -98, 164, 147, 121 },
      { 114, 1, -11, -107, 107, 107, 73 },
      { 115, 1, 6, -98, 75, 98, 69 },
      { 122, 1, 0, -89, 96, 100, 95 },
      { 43, 0, -1, -126, 85, 85, 83 },
      { 57597, 0, 0, -267, 110, 401, 109 },
      { 57599, 0, 0, -334, 115, 379, 115 },
      { 57596, 0, 0, -333, 110, 380, 109 },
      { 57592, 0, 0, -333, 180, 380, 179 },
      { 57593, 0, 0, -334, 178, 381, 179 },
      { 57594, 0, 0, -334, 179, 381, 179 },
      { 57595, 0, -1, -348, 179, 442, 179 },
      { 57598, 0, 0, -18, 98, 37, 37 },
      { 57672, 0, 0, -38, 110, 75, 110 },
      { 57673, 0, 0, -38, 110, 76, 110 },
      { 57674, 0, -1, -37, 166, 75, 165 },
      };
#endif

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, const QChar& c, int fid)
   : _code(c), fontId(fid), _name(name)
      {
      //
      // font is rendered with a physical resolution of PDPI
      //    and logical resolution of DPI
      //
      // rastral size is 20pt = 20/72 inch
      //
      int size = lrint(20.0 * DPI / PPI);
      if (fontId == 0) {
            _font = new QFont("MScore");
            }
      else if (fontId == 1) {
            _font = new QFont("MScore1");
            }
      else if (fontId == 2) {
            _font = new QFont("Times New Roman");
            size = lrint(8 * DPI / PPI);
            }
      else {
            printf("illegal font id %d\n", fontId);
            abort();
            }
      _font->setPixelSize(size);
      QFontMetricsF fm(*_font);
#ifdef CTABLE_HACK
      double m = (DPI/PPI) / 16.66667;
      unsigned i;
      for (i = 0; i < sizeof(ctable)/sizeof(*ctable); ++i) {
            if ((c.unicode() == ctable[i].code) && (fontId == ctable[i].font)) {
                  _bbox = QRectF(ctable[i].x * m,ctable[i].y * m,ctable[i].w * m,ctable[i].h * m);
                  w     = ctable[i].width * m;
                  break;
                  }
            }
      if (i == sizeof(ctable)/sizeof(*ctable)) {
            printf("sym <%s> 0x%x not found\n", name, c.unicode());
            _bbox = QFontMetricsF(*_font).boundingRect(_code);
            w     = QFontMetricsF(*_font).width(_code);
            }
#else
      w     = fm.width(_code);
      _bbox = fm.boundingRect(_code);
#endif
      }

//---------------------------------------------------------
//   findSymbol
//---------------------------------------------------------

const Sym* findSymbol(QChar code, int fontId)
      {
      foreach(const Sym& s, symbols) {
            if (s.code() == code && s.getFontId() == fontId)
                  return &s;
            }
      return 0;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF Sym::bbox(double mag) const
      {
      mag *= _spatiumMag;
      return QRectF(_bbox.x() * mag, _bbox.y() * mag, _bbox.width() * mag, _bbox.height() * mag);
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

double Sym::height(double mag) const
      {
      return _bbox.height() * mag * _spatiumMag;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double Sym::width(double mag) const
      {
      return w * mag * _spatiumMag;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, qreal x, qreal y) const
      {
      draw(painter, 1.0, x, y);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y) const
      {
      mag *= _spatiumMag;
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(*_font);
      painter.drawText(QPointF(x, y) * imag, QString(_code));
      painter.scale(imag, imag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y, int n) const
      {
      mag *= _spatiumMag;
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(*_font);
      painter.drawText(QPointF(x, y) * imag, QString(n, _code));
      painter.scale(imag, imag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter) const
      {
      draw(painter, 1.0, 0.0, 0.0);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag) const
      {
      draw(painter, mag, 0.0, 0.0);
      }

//---------------------------------------------------------
//   symToHtml
//    transform symbol into html code suitable
//    for QDocument->setHtml()
//---------------------------------------------------------

QString symToHtml(const Sym& s)
      {
      double size    = s.font().pixelSize() * 72.0 / DPI;
      QString family = s.font().family();
      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%3;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(s.code().unicode());
      }

QString symToHtml(const Sym& s1, const Sym& s2)
      {
      QFont f        = s1.font();
      double size    = s1.font().pixelSize() * 72.0 / DPI;
      QString family = f.family();

      return QString(
      "<data>"
        "<html>"
          "<head>"
            "<meta name=\"qrichtext\" content=\"1\" >"
            "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf8\" />"
            "<style type=\"text/css\">"
              "p, li { white-space: pre-wrap; }"
              "</style>"
            "</head>"
          "<body style=\" font-family:'%1'; font-size:%2pt;\">"
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%3;&#%4;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(s1.code().unicode()).arg(s2.code().unicode());
      }

//---------------------------------------------------------
//   writeCtable
//---------------------------------------------------------

void Sym::writeCtable()
      {
      QFile f("ctable.cpp");
      f.open(QIODevice::WriteOnly);
      QTextStream s(&f);

      s << "      CTable ctable[] = {\n";
      for (int i = 0; i < symbols.size(); ++i) {
            const Sym& sym = symbols[i];
            s << "            { " << int(sym._code.unicode())
              << ", " << sym.fontId
              << ", " << sym._bbox.x()
              << ", " << sym._bbox.y()
              << ", " << sym._bbox.width()
              << ", " << sym._bbox.height()
              << ", " << sym.w
              << " },\n";
              };
      s << "            };\n";
      f.close();
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      symbols[clefEightSym]               = Sym("clef eight", 0x38, 2);
      symbols[clefOneSym]                 = Sym("clef one",   0x31, 2);
      symbols[clefFiveSym]                = Sym("clef five",  0x35, 2);

      symbols[stemSym]                    = Sym("stem",  0xf08f, 0);
      symbols[stemSym]                    = Sym("stem",  0xf08f, 0);
      symbols[dstemSym]                   = Sym("dstem", 0xf090, 0);

      symbols[plusSym]                    = Sym("plus",               0x2b, 0);

      symbols[zeroSym]                    = Sym("zero",               0x30, 0);
      symbols[oneSym]                     = Sym("one",                0x31, 0);
      symbols[twoSym]                     = Sym("two",                0x32, 0);
      symbols[threeSym]                   = Sym("three",              0x33, 0);
      symbols[fourSym]                    = Sym("four",               0x34, 0);
      symbols[fiveSym]                    = Sym("five",               0x35, 0);
      symbols[sixSym]                     = Sym("six",                0x36, 0);
      symbols[sevenSym]                   = Sym("seven",              0x37, 0);
      symbols[eightSym]                   = Sym("eight",              0x38, 0);
      symbols[nineSym]                    = Sym("nine",               0x39, 0);

      symbols[letterfSym]                 = Sym("f",                  0x66, 0);
      symbols[lettermSym]                 = Sym("m",                  0x6d, 0);
      symbols[letterpSym]                 = Sym("p",                  0x70, 0);
      symbols[letterrSym]                 = Sym("r",                  0x72, 0);
      symbols[lettersSym]                 = Sym("s",                  0x73, 0);
      symbols[letterzSym]                 = Sym("z",                  0x7a, 0);

      symbols[wholerestSym]               = Sym("whole rest",               0xe100, 0);
      symbols[halfrestSym]                = Sym("half rest",                0xe101, 0);
      symbols[outsidewholerestSym]        = Sym("outside whole rest",       0xe102, 0);
      symbols[outsidehalfrestSym]         = Sym("outside half rest",        0xe103, 0);
      symbols[rest_M3]                    = Sym("rest M3",                  0xe104, 0);
      symbols[longarestSym]               = Sym("longa rest",               0xe105, 0);
      symbols[breverestSym]               = Sym("breve rest",               0xe106, 0);
      symbols[quartrestSym]               = Sym("quart rest",               0xe107, 0);

      symbols[clasquartrestSym]           = Sym("clas quart rest",          0xe108, 0);
      symbols[eighthrestSym]              = Sym("eight rest",               0xe109, 0);
      symbols[sixteenthrestSym]           = Sym("16' rest",                 0xe10a, 0);
      symbols[thirtysecondrestSym]        = Sym("32'  rest",                0xe10b, 0);
      symbols[sixtyfourthrestSym]         = Sym("64' rest",                 0xe10c, 0);
      symbols[hundredtwentyeighthrestSym] = Sym("128' rest",                0xe10d, 0);
      symbols[sharpSym]                   = Sym("sharp",                    0xe10e, 0);

      symbols[naturalSym]                 = Sym("natural",                  0xe113, 0);
      symbols[flatSym]                    = Sym("flat",                     0xe114, 0);
      symbols[flatflatSym]                = Sym("flat flat",                0xe11a, 0);
      symbols[sharpsharpSym]              = Sym("sharp sharp",              0xe11c, 0);

      symbols[rightparenSym]              = Sym("right parenthesis",        0xe11d, 0);
      symbols[leftparenSym]               = Sym("left parenthesis",         0xe11e, 0);
      symbols[dotSym]                     = Sym("dot",                      0xe127, 0);

      symbols[brevisheadSym]              = Sym("brevis head",              0xe12a, 0);
      symbols[wholeheadSym]               = Sym("whole head",               0xe12b, 0);
      symbols[halfheadSym]                = Sym("half head",                0xe12c, 0);
      symbols[quartheadSym]               = Sym("quart head",               0xe12d, 0);

      symbols[wholediamondheadSym]        = Sym("whole diamond head",       0xe12e, 0);
      symbols[halfdiamondheadSym]         = Sym("half diamond head",        0xe12f, 0);
      symbols[diamondheadSym]             = Sym("diamond head",             0xe130, 0);

      symbols[wholetriangleheadSym]       = Sym("whole triangle head",      0xe131, 0);
      symbols[halftriangleheadSym]        = Sym("half triangle head",       0xe132, 0);
      symbols[triangleheadSym]            = Sym("triangle head",            0xe134, 0);

      symbols[wholeslashheadSym]          = Sym("whole slash head",         0xe136, 0);
      symbols[halfslashheadSym]           = Sym("half slash head",          0xe137, 0);
      symbols[quartslashheadSym]          = Sym("quart slash head",         0xe138, 0);

      symbols[wholecrossedheadSym]        = Sym("whole cross head",         0xe139, 0);
      symbols[halfcrossedheadSym]         = Sym("half cross head",          0xe13a, 0);
      symbols[crossedheadSym]             = Sym("cross head",               0xe13b, 0);
      symbols[xcircledheadSym]            = Sym("x circle head",            0xe13c, 0);

      symbols[ufermataSym]                = Sym("ufermata",                 0xe158, 0);
      symbols[dfermataSym]                = Sym("dfermata",                 0xe159, 0);
      symbols[thumbSym]                   = Sym("thumb",                    0xe160, 0);
      symbols[sforzatoaccentSym]          = Sym("sforza to accent",         0xe161, 0);
      symbols[esprSym]                    = Sym("espressivo",               0xe162, 0);
      symbols[staccatoSym]                = Sym("staccato",                 0xe163, 0);
      symbols[ustaccatissimoSym]          = Sym("ustaccatissimo",           0xe164, 0);
      symbols[dstaccatissimoSym]          = Sym("dstacattissimo",           0xe165, 0);

      symbols[tenutoSym]                  = Sym("tenuto",                   0xe166, 0);
      symbols[uportatoSym]                = Sym("uportato",                 0xe167, 0);
      symbols[dportatoSym]                = Sym("dportato",                 0xe168, 0);
      symbols[umarcatoSym]                = Sym("umarcato",                 0xe169, 0);
      symbols[dmarcatoSym]                = Sym("dmarcato",                 0xe16a, 0);
      symbols[ouvertSym]                  = Sym("ouvert",                   0xe16b, 0);
      symbols[plusstopSym]                = Sym("plus stop",                0xe16c, 0);
      symbols[upbowSym]                   = Sym("up bow",                   0xe16d, 0);
      symbols[downbowSym]                 = Sym("down bow",                 0xe16e, 0);
      symbols[reverseturnSym]             = Sym("reverse turn",             0xe16f, 0);
      symbols[turnSym]                    = Sym("turn",                     0xe170, 0);
      symbols[trillSym]                   = Sym("trill",                    0xe171, 0);
      symbols[upedalheelSym]              = Sym("upedal heel",              0xe172, 0);
      symbols[dpedalheelSym]              = Sym("dpedalheel",               0xe173, 0);
      symbols[upedaltoeSym]               = Sym("upedal toe",               0xe174, 0);
      symbols[dpedaltoeSym]               = Sym("dpedal toe",               0xe175, 0);

      symbols[flageoletSym]               = Sym("flageolet",                0xe176, 0);

      symbols[segnoSym]                   = Sym("segno",                    0xe177, 0);
      symbols[codaSym]                    = Sym("coda",                     0xe178, 0);
      symbols[varcodaSym]                 = Sym("varcoda",                  0xe179, 0);

      symbols[rcommaSym]                  = Sym("rcomma",                   0xe17a, 0);
      symbols[lcommaSym]                  = Sym("lcomma",                   0xe17b, 0);
      symbols[arpeggioSym]                = Sym("arpeggio",                 0xe17e, 0);
      symbols[trillelementSym]            = Sym("trillelement",             0xe17f, 0);
      symbols[arpeggioarrowdownSym]       = Sym("arpeggio arrow down",      0xe180, 0);
      symbols[arpeggioarrowupSym]         = Sym("arpeggio arrow up",        0xe181, 0);
      symbols[trilelementSym]             = Sym("trill element",            0xe182, 0);
      symbols[prallSym]                   = Sym("prall",                    0xe183, 0);
      symbols[mordentSym]                 = Sym("mordent",                  0xe184, 0);
      symbols[prallprallSym]              = Sym("prall prall",              0xe185, 0);
/*100*/      symbols[prallmordentSym]            = Sym("prall mordent",     0xe186, 0);
      symbols[upprallSym]                 = Sym("up prall",                 0xe187, 0);
      symbols[upmordentSym]               = Sym("up mordent",               0xe188, 0);
      symbols[pralldownSym]               = Sym("prall down",               0xe189, 0);
      symbols[downprallSym]               = Sym("down prall",               0xe18a, 0);
      symbols[downmordentSym]             = Sym("down mordent",             0xe18b, 0);
      symbols[prallupSym]                 = Sym("prall up",                 0xe18c, 0);
      symbols[lineprallSym]               = Sym("line prall",               0xe18d, 0);
//=======
      symbols[eighthflagSym]              = Sym("eight flag",               0xe190, 0);
      symbols[sixteenthflagSym]           = Sym("sixteenth flag",           0xe191, 0);
      symbols[thirtysecondflagSym]        = Sym("thirtysecond flag",        0xe192, 0);
      symbols[sixtyfourthflagSym]         = Sym("sixtyfour flag",           0xe193, 0);
      symbols[deighthflagSym]             = Sym("deight flag",              0xe194, 0);
      symbols[gracedashSym]               = Sym("grace dash",               0xe195, 0);
      symbols[dgracedashSym]              = Sym("dgrace dash",              0xe196, 0);
      symbols[dsixteenthflagSym]          = Sym("dsixteenth flag",          0xe197, 0);
      symbols[dthirtysecondflagSym]       = Sym("dthirtysecond flag",       0xe198, 0);
      symbols[dsixtyfourthflagSym]        = Sym("dsixtyfourth flag",        0xe199, 0);

      symbols[altoclefSym]                = Sym("alto clef",                0xe19a, 0);
      symbols[caltoclefSym]               = Sym("calto clef",               0xe19b, 0);

      symbols[bassclefSym]                = Sym("bass clef",                0xe19c, 0);
      symbols[cbassclefSym]               = Sym("cbass clef",               0xe19d, 0);
/*122*/
      symbols[trebleclefSym]              = Sym("trebleclef",               0xe19e, 0);   //G-Clef
      symbols[ctrebleclefSym]             = Sym("ctrebleclef",              0xe19f, 0);
      symbols[percussionclefSym]          = Sym("percussion clef",          0xe1a0, 0);
      symbols[cpercussionclefSym]         = Sym("cpercussion clef",         0xe1a1, 0);
      symbols[tabclefSym]                 = Sym("tab clef",                 0xe1a2, 0);
      symbols[ctabclefSym]                = Sym("ctab clef",                0xe1a3, 0);

      symbols[fourfourmeterSym]           = Sym("four four meter",          0xe1a4, 0);
      symbols[allabreveSym]               = Sym("allabreve",                0xe1a5, 0);
      symbols[pedalasteriskSym]           = Sym("pedalasterisk",            0xe1a6, 0);
      symbols[pedaldashSym]               = Sym("pedaldash",                0xe1a7, 0);
      symbols[pedaldotSym]                = Sym("pedaldot",                 0xe1a8, 0);

      symbols[pedalPSym]                  = Sym("pedalP",                   0xe1a9, 0);
      symbols[pedaldSym]                  = Sym("pedald",                   0xe1aa, 0);
      symbols[pedaleSym]                  = Sym("pedale",                   0xe1ab, 0);
      symbols[pedalPedSym]                = Sym("pedal ped",                0xe1ac, 0);

      symbols[brackettipsUp]              = Sym("bracket ticks up",         0xe1ad, 0);
      symbols[brackettipsDown]            = Sym("bracket ticks down",       0xe1ae, 0);

      symbols[accDiscantSym]              = Sym("acc discant",              0xe1af, 0);
      symbols[accDotSym]                  = Sym("acc dot",                  0xe1b0, 0);
      symbols[accFreebaseSym]             = Sym("acc freebase",             0xe1b1, 0);
      symbols[accStdbaseSym]              = Sym("acc stdbase",              0xe1b2, 0);
      symbols[accBayanbaseSym]            = Sym("acc bayanbase",            0xe1b3, 0);
      symbols[accSBSym]                   = Sym("acc sb",                   0xf0ba, 0);
      symbols[accBBSym]                   = Sym("acc bb",                   0xf0bb, 0);
      symbols[accOldEESym]                = Sym("acc old ee",               0xe1b4, 0);
      symbols[accOldEESSym]               = Sym("acc old ees",              0xf0bd, 0);
      symbols[wholedoheadSym]             = Sym("whole do head",            0xf0be, 0);
      symbols[halfdoheadSym]              = Sym("half do head",             0xf0bf, 0);
      symbols[doheadSym]                  = Sym("do head",                  0xf0c0, 0);

      symbols[wholereheadSym]             = Sym("whole re head",             0xf0c1, 0);
      symbols[halfreheadSym]              = Sym("half re head",              0xf0c2, 0);
      symbols[reheadSym]                  = Sym("re head",                   0xf0c3, 0);
      symbols[wholemeheadSym]             = Sym("whole me head",             0xf0c4, 0);
      symbols[halfmeheadSym]              = Sym("half me head",              0xf0c5, 0);
      symbols[meheadSym]                  = Sym("me head",                   0xf0c6, 0);
      symbols[wholefaheadSym]             = Sym("whole fa head",             0xf0c7, 0);
      symbols[halffauheadSym]             = Sym("half fau head",             0xf0c8, 0);
      symbols[fauheadSym]                 = Sym("fau head",                  0xf0c9, 0);
      symbols[halffadheadSym]             = Sym("half fad head",             0xf0ca, 0);
      symbols[fadheadSym]                 = Sym("fad head",                  0xf0cb, 0);
      symbols[wholelaheadSym]             = Sym("whole la head",             0xf0cc, 0);
      symbols[halflaheadSym]              = Sym("half la head",              0xf0cd, 0);
      symbols[laheadSym]                  = Sym("la head",                   0xf0ce, 0);
      symbols[wholeteheadSym]             = Sym("whole te head",             0xf0cf, 0);
      symbols[halfteheadSym]              = Sym("half te head",              0xf0d0, 0);
      symbols[teheadSym]                  = Sym("te head",                   0xf0d1, 0);

      symbols[flipSym]                    = Sym("flip stem",  0xe0fd, 0);

      symbols[wholediamond2headSym]        = Sym("whole diamond2 head",       0xe147, 0);
      symbols[halfdiamond2headSym]         = Sym("half diamond2 head",        0xe148, 0);
      symbols[diamond2headSym]             = Sym("diamond2 head",             0xe149, 0);


      // used for GUI:
      symbols[note2Sym]                   = Sym("note 1/2",   0xe1b5, 0);
      symbols[note4Sym]                   = Sym("note 1/4",   0xe1b6, 0);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe1b7, 0);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe1b8, 0);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe1b9, 0);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe1ba, 0);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe1bd, 0);

//      Sym::writeCtable();


      if (charReplaceMap.isEmpty()) {
            for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
                  if (pSymbols[i].code == 0 || pSymbols[i].text == 0)
                        continue;
                  charReplaceMap.insert(pSymbols[i].text, &pSymbols[i]);
                  }
            }
      }

