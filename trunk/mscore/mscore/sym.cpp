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

static const double FMAG = 1.0;

QPrinter* printer = 0;
static double printerDpi;

#ifdef __MINGW32__
#define CTABLE_HACK
#endif

// #define CTABLE_HACK

#ifdef CTABLE_HACK
      struct CTable {
            int code;
            int font;
            double x, y, w, h;
            double width;
            };

      CTable ctable[] = {
            { 56, 2, 6, -93, 57, 98, 67 },
            { 49, 2, 13, -93, 41, 97, 67 },
            { 53, 2, 5, -91, 56, 96, 67 },
            { 57600, 0, -2, -2, 131, 58, 125 },
            { 57601, 0, -2, -54, 131, 58, 125 },
            { 57602, 0, -54, -11, 235, 67, 125 },
            { 57603, 0, -54, -54, 235, 66, 125 },
            { 57605, 0, -2, -86, 56, 174, 50 },
            { 57606, 0, -2, -85, 56, 89, 50 },
            { 57607, 0, -10, -134, 94, 245, 79 },
            { 57609, 0, -3, -71, 91, 162, 83 },
            { 57608, 0, -3, -71, 91, 162, 83 },
            { 57610, 0, -11, -71, 115, 245, 100 },
            { 57611, 0, -20, -154, 133, 328, 109 },
            { 57612, 0, -24, -154, 145, 412, 117 },
            { 57613, 0, -22, -238, 151, 496, 125 },
            { 57604, 0, -2, -86, 156, 174, 150 },
            { 57705, 1, -8, -203, 187, 244, 168 },
            { 57773, 0, -2, -113, 162, 136, 159 },
            { 57774, 0, -2, -20, 162, 136, 159 },
            { 61649, 0, 6, -257, 323, 323, 333 },
            { 48, 0, -2, -169, 128, 173, 123 },
            { 49, 0, -3, -170, 114, 174, 106 },
            { 50, 0, -2, -169, 128, 173, 123 },
            { 51, 0, -2, -169, 117, 173, 111 },
            { 52, 0, -3, -169, 140, 173, 134 },
            { 53, 0, -2, -169, 123, 173, 113 },
            { 54, 0, -2, -169, 119, 173, 114 },
            { 55, 0, -2, -169, 127, 174, 113 },
            { 56, 0, -2, -169, 130, 173, 123 },
            { 57, 0, -2, -169, 119, 173, 114 },
            { 57614, 0, -2, -127, 98, 256, 92 },
            { 57619, 0, -5, -130, 67, 262, 56 },
            { 57620, 0, -11, -158, 82, 216, 67 },
            { 57626, 0, -11, -158, 136, 216, 121 },
            { 57628, 0, -6, -47, 98, 96, 83 },
            { 57629, 0, 10, -90, 44, 182, 50 },
            { 57630, 0, -52, -90, 44, 182, 0 },
            { 57639, 0, -2, -20, 43, 43, 37 },
            { 57642, 0, -15, -48, 198, 98, 165 },
            { 57643, 0, -2, -48, 171, 98, 165 },
            { 57644, 0, -2, -47, 122, 97, 116 },
            { 57645, 0, -2, -48, 116, 98, 110 },
            { 57646, 0, -2, -48, 171, 98, 165 },
            { 57647, 0, -2, -48, 127, 98, 122 },
            { 57632, 0, -9, -43, 96, 89, 83 },
            { 57649, 0, -3, -63, 200, 140, 193 },
            { 57650, 0, -2, -57, 146, 127, 140 },
            { 57652, 0, -2, -57, 122, 127, 116 },
            { 57654, 0, -2, -89, 258, 181, 252 },
            { 57655, 0, -2, -89, 202, 181, 196 },
            { 57656, 0, -2, -90, 149, 182, 143 },
            { 57657, 0, -2, -52, 149, 106, 143 },
            { 57658, 0, -2, -50, 132, 103, 127 },
            { 57659, 0, -2, -48, 116, 98, 110 },
            { 57660, 0, -2, -59, 136, 121, 131 },
            { 57688, 0, -113, -122, 229, 132, 111 },
            { 57689, 0, -113, -9, 229, 134, 111 },
            { 57696, 0, -35, -44, 72, 115, 33 },
            { 57697, 0, -77, -44, 156, 90, 75 },
            { 57698, 0, -161, -45, 324, 91, 159 },
            { 57699, 0, -19, -18, 39, 39, 17 },
            { 57700, 0, -19, -86, 39, 94, 17 },
            { 57701, 0, -19, -7, 40, 94, 17 },
            { 57702, 0, -52, -8, 106, 19, 50 },
            { 57703, 0, -52, -71, 107, 81, 50 },
            { 57704, 0, -52, -8, 107, 80, 50 },
            { 57705, 0, -44, -94, 89, 98, 42 },
            { 57706, 0, -43, -2, 89, 98, 42 },
            { 57707, 0, -35, -44, 72, 90, 33 },
            { 57708, 0, -48, -49, 99, 99, 46 },
            { 57709, 0, -56, -176, 115, 180, 54 },
            { 57710, 0, -65, -113, 131, 117, 63 },
            { 57711, 0, -93, -46, 188, 94, 91 },
            { 57712, 0, -93, -46, 188, 94, 91 },
            { 57713, 0, -109, -183, 205, 191, 71 },
            { 57714, 0, -44, -58, 90, 104, 42 },
            { 57715, 0, -44, -44, 90, 104, 42 },
            { 57716, 0, -44, -127, 90, 131, 42 },
            { 57717, 0, -44, -2, 90, 131, 42 },
            { 57718, 0, -46, -47, 95, 95, 44 },
            { 57703, 1, -2, -254, 173, 259, 167 },
            { 57704, 1, -2, -202, 185, 242, 145 },
            { 57722, 0, -3, -54, 45, 109, 42 },
            { 57723, 0, -39, -53, 45, 109, 0 },
            { 57726, 0, -2, -104, 73, 127, 67 },
            { 57727, 0, -21, -69, 127, 73, 83 },
            { 57728, 0, -22, -104, 113, 109, 67 },
            { 57729, 0, -22, -86, 113, 109, 67 },
            { 57730, 0, -42, -43, 87, 89, 35 },
            { 57731, 0, -86, -43, 174, 89, 70 },
            { 57732, 0, -86, -57, 173, 117, 70 },
            { 57733, 0, -121, -43, 244, 89, 104 },
            { 57734, 0, -121, -57, 244, 117, 104 },
            { 57735, 0, -133, -44, 256, 150, 104 },
            { 57738, 0, -133, -80, 256, 126, 104 },
            { 57736, 0, -133, -58, 256, 164, 104 },
            { 57739, 0, -133, -81, 256, 141, 104 },
            { 57741, 0, -121, -164, 243, 210, 104 },
            { 57737, 0, -121, -44, 256, 150, 104 },
            { 57740, 0, -121, -80, 256, 126, 104 },
            { 57744, 0, -7, -2, 81, 257, 75 },
            { 57745, 0, -7, -2, 80, 299, 75 },
            { 57746, 0, -7, -2, 81, 365, 70 },
            { 57747, 0, -7, -2, 81, 448, 70 },
            { 57748, 0, -7, -244, 101, 248, 90 },
            { 57749, 0, -60, 73, 142, 119, 70 },
            { 57750, 0, -75, -182, 177, 112, 90 },
            { 57751, 0, -7, -253, 100, 257, 90 },
            { 57752, 0, -7, -327, 101, 331, 90 },
            { 57753, 0, -7, -369, 101, 373, 90 },
            { 61583, 0, 26, -243, 247, 247, 297 },
            { 61584, 0, 26, -243, 247, 247, 297 },
            { 57754, 0, -2, -169, 233, 340, 227 },
            { 57755, 0, -2, -135, 189, 272, 183 },
            { 57756, 0, -7, -90, 230, 266, 224 },
            { 57757, 0, -7, -73, 187, 215, 179 },
            { 57758, 0, -2, -418, 219, 640, 214 },
            { 57759, 0, -2, -335, 177, 515, 171 },
            { 57760, 0, 54, -86, 116, 174, 167 },
            { 57761, 0, 43, -68, 94, 139, 134 },
            { 57762, 0, 13, -243, 224, 488, 234 },
            { 57763, 0, 9, -195, 182, 391, 187 },
            { 57764, 0, -2, -89, 151, 181, 142 },
            { 57765, 0, -2, -119, 151, 240, 142 },
            { 57766, 0, -2, -169, 136, 136, 130 },
            { 57767, 0, -2, -81, 76, 34, 70 },
            { 57768, 0, -2, -25, 29, 29, 23 },
            { 57769, 0, -2, -169, 146, 177, 139 },
            { 57770, 0, -4, -149, 120, 153, 111 },
            { 57771, 0, -2, -98, 75, 102, 67 },
            { 57772, 0, -2, -169, 273, 177, 266 },
            { 57775, 0, -132, -260, 267, 264, 131 },
            { 57776, 0, -23, -22, 47, 47, 21 },
            { 57777, 0, -91, -177, 184, 181, 89 },
            { 57778, 0, -174, -343, 350, 347, 173 },
            { 57779, 0, -91, -263, 184, 267, 89 },
            { 61626, 0, 26, -243, 247, 247, 297 },
            { 61627, 0, 26, -243, 247, 247, 297 },
            { 57780, 0, -91, -177, 184, 181, 89 },
            { 61629, 0, 26, -243, 247, 247, 297 },
            { 61630, 0, 26, -243, 247, 247, 297 },
            { 61631, 0, 26, -243, 247, 247, 297 },
            { 61632, 0, 26, -243, 247, 247, 297 },
            { 61633, 0, 26, -243, 247, 247, 297 },
            { 61634, 0, 26, -243, 247, 247, 297 },
            { 61635, 0, 26, -265, 246, 299, 297 },
            { 61636, 0, 26, -265, 246, 299, 297 },
            { 61637, 0, 27, -265, 246, 299, 297 },
            { 61638, 0, 27, -265, 246, 299, 297 },
            { 61639, 0, 26, -243, 299, 246, 349 },
            { 61640, 0, 26, -242, 299, 246, 349 },
            { 61641, 0, 26, -243, 299, 246, 349 },
            { 61642, 0, 26, -242, 299, 246, 349 },
            { 61643, 0, 6, -257, 323, 325, 333 },
            { 61644, 0, -2, -266, 340, 340, 333 },
            { 61645, 0, 6, -257, 323, 323, 333 },
            { 61646, 0, 6, -257, 323, 323, 333 },
            { 61647, 0, 6, -257, 323, 323, 333 },
            { 61648, 0, 6, -257, 323, 323, 333 },
            { 102, 1, -36, -160, 185, 223, 107 },
            { 109, 1, -17, -104, 174, 114, 146 },
            { 112, 1, -47, -100, 170, 153, 121 },
            { 114, 1, -13, -109, 113, 113, 73 },
            { 115, 1, 4, -100, 81, 104, 69 },
            { 122, 1, -2, -91, 102, 106, 95 },
            { 43, 0, -3, -128, 91, 91, 83 },
            { 57597, 0, -2, -269, 116, 407, 109 },
            { 57599, 0, -2, -336, 121, 385, 115 },
            { 57596, 0, -2, -335, 116, 386, 109 },
            { 57592, 0, -2, -335, 186, 386, 179 },
            { 57593, 0, -2, -336, 184, 387, 179 },
            { 57594, 0, -2, -336, 185, 387, 179 },
            { 57595, 0, -3, -350, 185, 448, 179 },
            { 57598, 0, -2, -20, 104, 43, 37 },
            { 57672, 0, -2, -40, 116, 81, 110 },
            { 57673, 0, -2, -40, 116, 82, 110 },
            { 57674, 0, -3, -39, 172, 81, 165 },
            };
#endif

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, const QChar& c, int f)
   : _code(c), fontId(f), _name(name)
      {
      QFont font;
      if (fontId == 0) {
            font = QFont("MScore");
            font.setPointSizeF(20.0 * FMAG);
            }
      else if (fontId == 1) {
            font = QFont("MScore1");
            font.setPointSizeF(20.0 * FMAG);
            }
      else if (fontId == 2) {
            font = QFont("Times New Roman");
            font.setPointSizeF(8.0 * FMAG);
            }
#ifdef CTABLE_HACK
      unsigned i;
      for (i = 0; i < sizeof(ctable)/sizeof(*ctable); ++i) {
            if ((c.unicode() == ctable[i].code) && (f == ctable[i].font)) {
                  _bbox = QRectF(
                        ctable[i].x,
                        ctable[i].y,
                        ctable[i].w,
                        ctable[i].h);
                  _width = ctable[i].width;
//                  _baseline = ctable[i].baseline;
                  break;
                  }
            }
      if (i == sizeof(ctable)/sizeof(*ctable)) {
printf("sym not found\n");
            _bbox   = QFontMetricsF(font, printer).boundingRect(_code);
            _width  = QFontMetricsF(font, printer).width(_code);
            }
#else
      QFontMetricsF fm(font, printer);
//      _bbox   = QFontMetricsF(font, printer).tightBoundingRect(QString(_code));
      _bbox   = fm.boundingRect(_code);
      _bbox.adjust(-2, -2, 4, 4);   // HACK
      _width  = fm.width(_code);
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
//    BUG:
//      For unknown reasons i cannot get the right font
//      metrics from Qt.
//      The problem is that the floating point metrics
//      of Qt are the same as the integer metrics (rounded
//      down to screen resolution). This makes no sense.
//      Painting with float accuracy does not work well.
//
//      Qt4.3 delivers slightly different font metrics than
//      Qt4.2.2
//---------------------------------------------------------

const QRectF Sym::bbox(double mag) const
      {
      double m = _spatium * mag / (spatiumBase20 * FMAG * printerDpi);
      return QRectF(_bbox.topLeft() * m, _bbox.size() * m);
      }

double Sym::width(double mag) const
      {
      double m = _spatium * mag / (spatiumBase20 * FMAG * printerDpi);
      return _width * m;
      }

double Sym::height(double mag) const
      {
      double m = _spatium * mag / (spatiumBase20 * FMAG * printerDpi);
      return _bbox.height() * m;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, qreal x, qreal y) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y) const
      {
      painter.setFont(font(mag));
      painter.drawText(QPointF(x, y), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, qreal x, qreal y, int n) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(n, _code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(0,0), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag) const
      {
      painter.setFont(font(mag));
      painter.drawText(QPointF(0,0), QString(_code));
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Sym::font(double extraMag) const
      {
      double mag = _spatium * extraMag / (spatiumBase20 * DPI);
      if (fontId == 0) {
            QFont f("MScore");
            f.setPointSizeF(20.0 * mag);
            return f;
            }
      else if (fontId == 1) {
            QFont f("MScore1");
            f.setPointSizeF(20.0 * mag);
            return f;
            }
      else if (fontId == 2) {
            QFont f("Times New Roman");
            f.setPointSizeF(8.0 * mag);
            return f;
            }
      else {
            printf("illegal font id %d\n", fontId);
            return QFont();
            }
      }

//---------------------------------------------------------
//   symToHtml
//    transform symbol into html code suitable
//    for QDocument->setHtml()
//---------------------------------------------------------

QString symToHtml(const Sym& s)
      {
      QFont f        = s.font();
      double size    = f.pointSizeF();
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
                "&#%3;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(s.code().unicode());
      }

QString symToHtml(const Sym& s1, const Sym& s2)
      {
      QFont f        = s1.font();
      double size    = f.pointSizeF();
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
              << ", " << sym._width
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
      if (printer == 0) {
            printer = new QPrinter(QPrinter::HighResolution);
#ifdef CTABLE_HACK
            printerDpi = 1200.0;
#else
            printerDpi = double(printer->logicalDpiX());
#endif
            }

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

      symbols[letterfSym]                 = Sym("f",                  0x66, 1);
      symbols[lettermSym]                 = Sym("m",                  0x6d, 1);
      symbols[letterpSym]                 = Sym("p",                  0x70, 1);
      symbols[letterrSym]                 = Sym("r",                  0x72, 1);
      symbols[lettersSym]                 = Sym("s",                  0x73, 1);
      symbols[letterzSym]                 = Sym("z",                  0x7a, 1);

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
      symbols[diamondheadSym]             = Sym("diamond head",             0xe120, 0);

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

      symbols[segnoSym]                   = Sym("segno",                    0xe167, 1);
      symbols[codaSym]                    = Sym("coda",                     0xe168, 1);
      symbols[varcodaSym]                 = Sym("varcoda",                  0xe169, 1);

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

      symbols[wholediamond2headSym]        = Sym("whole diamond2 head",       0xe148, 0);
      symbols[halfdiamond2headSym]         = Sym("half diamond2 head",        0xe149, 0);
      symbols[diamond2headSym]             = Sym("diamond2 head",             0xe14a, 0);


      // used for GUI:
      symbols[note2Sym]                   = Sym("note 1/2",   0xe0ff, 0);
      symbols[note4Sym]                   = Sym("note 1/4",   0xe0fc, 0);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe0f8, 0);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe0f9, 0);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe0fa, 0);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe0fb, 0);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe0fe, 0);

      Sym::writeCtable();
      }

