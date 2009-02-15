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
      SymCode(0xe10c, 1),    // sharp
      SymCode(0xe10d, 1),    // flat
      SymCode(0xe104, 1),    // note2_Sym
      SymCode(0xe105, 1),    // note4_Sym
      SymCode(0xe106, 1),    // note8_Sym
      SymCode(0xe107, 1),    // note16_Sym
      SymCode(0xe108, 1),    // note32_Sym
      SymCode(0xe109, 1),    // note64_Sym
      SymCode(0xe10a, 1),    // dot
      SymCode(0xe10b, 1),    // dotdot
      SymCode(0xe167, 1),    // coda
      SymCode(0xe168, 1),    // varcoda
      SymCode(0xe169, 1),    // segno
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

      // SymCode(0x203F, -1),    // curved ligature to connect two syllables
      SymCode(0x35c, -1),    // curved ligature to connect two syllables
      SymCode(0x361, -1),    // curved ligature (top)

      SymCode(-1, -1)    // indicates end
      };

//---------------------------------------------------------
//   fontId2Font
//---------------------------------------------------------

QFont fontId2font(int fontId)
      {
      QFont _font;
      //
      // font is rendered with a physical resolution of PDPI
      //    and logical resolution of DPI
      //
      // rastral size is 20pt = 20/72 inch
      //
      int size = lrint(20.0 * DPI / PPI);
      if (fontId == 0) {
            _font = QFont("MScore");
            _font.setStyleStrategy(QFont::NoFontMerging);
            }
      else if (fontId == 1) {
            _font = QFont("MScore1");
            }
      else if (fontId == 2) {
            _font = QFont("Times New Roman");
            _font.setStyleStrategy(QFont::NoFontMerging);
            size = lrint(8 * DPI / PPI);
            }
      else {
            printf("illegal font id %d\n", fontId);
            abort();
            }
      _font.setPixelSize(size);
      return _font;
      }

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, const QChar& c, int fid)
   : _code(c), fontId(fid), _name(name)
      {
      _font = new QFont(fontId2font(fontId));
      QFontMetricsF fm(*_font);
      w     = fm.width(_code);
      _bbox = fm.boundingRect(_code);
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
      symbols[sharpslashSym]              = Sym("sharp slash",              0xe10f, 0);
      symbols[sharpslash2Sym]             = Sym("sharp slash2",             0xe110, 0);
      symbols[sharpslash3Sym]             = Sym("sharp slash3",             0xe111, 0);
      symbols[sharpslash4Sym]             = Sym("sharp slash4",             0xe112, 0);

      symbols[naturalSym]                 = Sym("natural",                  0xe113, 0);
      symbols[flatSym]                    = Sym("flat",                     0xe114, 0);
      symbols[flatslashSym]               = Sym("flat slash",               0xe115, 0);
      symbols[flatslash2Sym]              = Sym("flat slash2",              0xe116, 0);
      symbols[mirroredflat2Sym]           = Sym("mirrored flat2",           0xe117, 0);
      symbols[mirroredflatSym]            = Sym("mirrored flat",            0xe118, 0);
      symbols[mirroredflatslashSym]       = Sym("mirrored flat slash",      0xe119, 0);
      symbols[flatflatSym]                = Sym("flat flat",                0xe11a, 0);
      symbols[flatflatslashSym]           = Sym("flat flat slash",          0xe11b, 0);
      symbols[sharpsharpSym]              = Sym("sharp sharp",              0xe11c, 0);

      symbols[rightparenSym]              = Sym("right parenthesis",        0xe11d, 0);
      symbols[leftparenSym]               = Sym("left parenthesis",         0xe11e, 0);
      symbols[dotSym]                     = Sym("dot",                      0xe127, 0);

      symbols[longaupSym]                 = Sym("longa up",                 0xe128, 0);
      symbols[longadownSym]               = Sym("longa down",               0xe129, 0);

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
      symbols[varcodaSym]                 = Sym("varied coda",              0xe179, 0);

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
/*100*/
      symbols[prallmordentSym]            = Sym("prall mordent",            0xe186, 0);
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

      symbols[accDiscantSym]              = Sym(QT_TR_NOOP("acc discant"),  0xe1af, 0);
      symbols[accDotSym]                  = Sym(QT_TR_NOOP("acc dot"),      0xe1b0, 0);
      symbols[accFreebaseSym]             = Sym(QT_TR_NOOP("acc freebase"), 0xe1b1, 0);
      symbols[accStdbaseSym]              = Sym(QT_TR_NOOP("acc stdbase"),  0xe1b2, 0);
      symbols[accBayanbaseSym]            = Sym(QT_TR_NOOP("acc bayanbase"),0xe1b3, 0);

      symbols[accOldEESym]                = Sym("acc old ee",               0xe1b4, 0);

      symbols[flipSym]                    = Sym("flip stem",  0xe0fd, 0);

      symbols[wholediamond2headSym]        = Sym("whole diamond2 head",       0xe147, 0);
      symbols[halfdiamond2headSym]         = Sym("half diamond2 head",        0xe148, 0);
      symbols[diamond2headSym]             = Sym("diamond2 head",             0xe149, 0);


      // used for GUI:
      symbols[note2Sym]                   = Sym("note 1/2",   0xe104, 1);
      symbols[note4Sym]                   = Sym("note 1/4",   0xe105, 1);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe106, 1);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe107, 1);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe108, 1);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe109, 1);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe10b, 1);

//      Sym::writeCtable();


      if (charReplaceMap.isEmpty()) {
            for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
                  if (pSymbols[i].code == 0 || pSymbols[i].text == 0)
                        continue;
                  charReplaceMap.insert(pSymbols[i].text, &pSymbols[i]);
                  }
            }
      }

