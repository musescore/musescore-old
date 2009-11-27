//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
#include "utils.h"
#include "mscore.h"
#include "score.h"

QVector<Sym> symbols(lastSym);
QMap<const char*, SymCode*> charReplaceMap;

SymCode pSymbols[] = {
      SymCode(0xe10e, 1),    //natural
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
            _font.setFamily("MScore");
            _font.setStyleStrategy(QFont::NoFontMerging);
            }
      else if (fontId == 1) {
            _font.setFamily("MScore1");
            }
      else if (fontId == 2) {
            _font.setFamily("Times New Roman");
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
   : _code(c), fontId(fid), _name(name), _font(fontId2font(fontId))
      {
      QFontMetricsF fm(_font);
      if (!fm.inFont(_code))
            printf("Sym: character 0x%x <%s> are not in font <%s>\n", _code.unicode(), _name, qPrintable(_font.family()));
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
      return QRectF(_bbox.x() * mag, _bbox.y() * mag, _bbox.width() * mag, _bbox.height() * mag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y) const
      {
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(_font);
      painter.drawText(QPointF(x, y) * imag, QString(_code));
      painter.scale(imag, imag);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(QPainter& painter, double mag, qreal x, qreal y, int n) const
      {
      double imag = 1.0 / mag;
      painter.scale(mag, mag);
      painter.setFont(_font);
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

QString symToHtml(const Sym& s, int leftMargin)
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
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s.code().unicode());
      }

QString symToHtml(const Sym& s1, const Sym& s2, int leftMargin)
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
            "<p style=\" margin-top:0px; margin-bottom:0px; margin-left:%3px; margin-right:0px; -qt-block-indent:0; text-indent:0px;\">"
                "&#%4;&#%5;"
              "</p>"
            "</body>"
          "</html>"
      "</data>").arg(family).arg(size).arg(leftMargin).arg(s1.code().unicode()).arg(s2.code().unicode());
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
      symbols[clefEightSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "clef eight"),                 0x38, 2);
      symbols[clefOneSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "clef one"),                   0x31, 2);
      symbols[clefFiveSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "clef five"),                  0x35, 2);

      symbols[plusSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "plus"),                       0x2b, 0);
      symbols[zeroSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "zero"),                       0x30, 0);
      symbols[oneSym]                     = Sym(QT_TRANSLATE_NOOP("symbol", "one"),                        0x31, 0);
      symbols[twoSym]                     = Sym(QT_TRANSLATE_NOOP("symbol", "two"),                        0x32, 0);
      symbols[threeSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "three"),                      0x33, 0);
      symbols[fourSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "four"),                       0x34, 0);
      symbols[fiveSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "five"),                       0x35, 0);
      symbols[sixSym]                     = Sym(QT_TRANSLATE_NOOP("symbol", "six"),                        0x36, 0);
      symbols[sevenSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "seven"),                      0x37, 0);
      symbols[eightSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "eight"),                      0x38, 0);
      symbols[nineSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "nine"),                       0x39, 0);

      symbols[letterfSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "f"),                          0x66, 1);
      symbols[lettermSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "m"),                          0x6d, 1);
      symbols[letterpSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "p"),                          0x70, 1);
      symbols[letterrSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "r"),                          0x72, 1);
      symbols[lettersSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "s"),                          0x73, 1);
      symbols[letterzSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "z"),                          0x7a, 1);

      symbols[wholerestSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "whole rest"),               0xe100, 0);
      symbols[halfrestSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "half rest"),                0xe101, 0);
      symbols[outsidewholerestSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "outside whole rest"),       0xe102, 0);
      symbols[outsidehalfrestSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "outside half rest"),        0xe103, 0);
      symbols[rest_M3]                    = Sym(QT_TRANSLATE_NOOP("symbol", "rest M3"),                  0xe104, 0);
      symbols[longarestSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "longa rest"),               0xe105, 0);
      symbols[breverestSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "breve rest"),               0xe106, 0);
      symbols[quartrestSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "quart rest"),               0xe107, 0);

      symbols[clasquartrestSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "clas quart rest"),          0xe108, 0);
      symbols[eighthrestSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "eight rest"),               0xe109, 0);
      symbols[sixteenthrestSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "16' rest"),                 0xe10a, 0);
      symbols[thirtysecondrestSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "32'  rest"),                0xe10b, 0);
      symbols[sixtyfourthrestSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "64' rest"),                 0xe10c, 0);
      symbols[hundredtwentyeighthrestSym] = Sym(QT_TRANSLATE_NOOP("symbol", "128' rest"),                0xe10d, 0);

      symbols[sharpSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "sharp"),                    0xe10e, 0);
      symbols[sharpArrowUpSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "sharp arrow up"),           0xe10f, 0);
      symbols[sharpArrowDownSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "sharp arrow down"),         0xe110, 0);
      symbols[sharpArrowBothSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "sharp arrow both"),         0xe111, 0);
      symbols[sharpslashSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "sharp slash"),              0xe112, 0);
      symbols[sharpslash2Sym]             = Sym(QT_TRANSLATE_NOOP("symbol", "sharp slash2"),             0xe113, 0);
      symbols[sharpslash3Sym]             = Sym(QT_TRANSLATE_NOOP("symbol", "sharp slash3"),             0xe114, 0);
      symbols[sharpslash4Sym]             = Sym(QT_TRANSLATE_NOOP("symbol", "sharp slash4"),             0xe115, 0);
      symbols[naturalSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "natural"),                  0xe116, 0);
      symbols[naturalArrowUpSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "natural arrow up"),         0xe117, 0);
      symbols[naturalArrowDownSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "natural arrow down"),       0xe118, 0);
      symbols[naturalArrowBothSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "natural arrow both"),       0xe119, 0);
      symbols[flatSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "flat"),                     0xe11a, 0);
      symbols[flatArrowUpSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "flat arrow up"),            0xe11b, 0);
      symbols[flatArrowDownSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "flat arrow down"),          0xe11c, 0);
      symbols[flatArrowBothSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "flat arrow both"),          0xe11d, 0);
      symbols[flatslashSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "flat slash"),               0xe11e, 0);
      symbols[flatslash2Sym]              = Sym(QT_TRANSLATE_NOOP("symbol", "flat slash2"),              0xe11f, 0);
      symbols[mirroredflat2Sym]           = Sym(QT_TRANSLATE_NOOP("symbol", "mirrored flat2"),           0xe120, 0);
      symbols[mirroredflatSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "mirrored flat"),            0xe121, 0);
      symbols[mirroredflatslashSym]       = Sym(QT_TRANSLATE_NOOP("symbol", "mirrored flat slash"),      0xe122, 0);
      symbols[flatflatSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "flat flat"),                0xe123, 0);
      symbols[flatflatslashSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "flat flat slash"),          0xe124, 0);
      symbols[sharpsharpSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "sharp sharp"),              0xe125, 0);

      symbols[rightparenSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "right parenthesis"),        0xe126, 0);
      symbols[leftparenSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "left parenthesis"),         0xe127, 0);
                                                                                                        //
      symbols[dotSym]                     = Sym(QT_TRANSLATE_NOOP("symbol", "dot"),                      0xe130, 0);
      symbols[longaupSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "longa up"),                 0xe131, 0);
      symbols[longadownSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "longa down"),               0xe132, 0);
      symbols[brevisheadSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "brevis head"),              0xe133, 0);
      symbols[wholeheadSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "whole head"),               0xe134, 0);
      symbols[halfheadSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "half head"),                0xe135, 0);
      symbols[quartheadSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "quart head"),               0xe136, 0);
      symbols[wholediamondheadSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "whole diamond head"),       0xe137, 0);
      symbols[halfdiamondheadSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "half diamond head"),        0xe138, 0);
      symbols[diamondheadSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "diamond head"),             0xe139, 0);
      symbols[wholetriangleheadSym]       = Sym(QT_TRANSLATE_NOOP("symbol", "whole triangle head"),      0xe13a, 0);
      symbols[halftriangleheadSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "half triangle head"),       0xe13b, 0);

      symbols[triangleheadSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "triangle head"),            0xe13d, 0);

      symbols[wholeslashheadSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "whole slash head"),         0xe13f, 0);
      symbols[halfslashheadSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "half slash head"),          0xe140, 0);
      symbols[quartslashheadSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "quart slash head"),         0xe141, 0);
      symbols[wholecrossedheadSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "whole cross head"),         0xe142, 0);
      symbols[halfcrossedheadSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "half cross head"),          0xe143, 0);
      symbols[crossedheadSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "cross head"),               0xe144, 0);
      symbols[xcircledheadSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "x circle head"),            0xe145, 0);

      symbols[wholediamond2headSym]       = Sym(QT_TRANSLATE_NOOP("symbol", "whole diamond2 head"),      0xe150, 0);
      symbols[halfdiamond2headSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "half diamond2 head"),       0xe151, 0);
      symbols[diamond2headSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "diamond2 head"),            0xe152, 0);

      symbols[ufermataSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "ufermata"),                 0xe161, 0);
      symbols[dfermataSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "dfermata"),                 0xe162, 0);

      symbols[thumbSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "thumb"),                    0xe169, 0);
      symbols[sforzatoaccentSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "sforza to accent"),         0xe16a, 0);
      symbols[esprSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "espressivo"),               0xe16b, 0);
      symbols[staccatoSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "staccato"),                 0xe16c, 0);
      symbols[ustaccatissimoSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "ustaccatissimo"),           0xe16d, 0);
      symbols[dstaccatissimoSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "dstacattissimo"),           0xe16e, 0);
      symbols[tenutoSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "tenuto"),                   0xe16f, 0);
      symbols[uportatoSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "uportato"),                 0xe170, 0);
      symbols[dportatoSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "dportato"),                 0xe171, 0);
      symbols[umarcatoSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "umarcato"),                 0xe172, 0);
      symbols[dmarcatoSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "dmarcato"),                 0xe173, 0);
      symbols[ouvertSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "ouvert"),                   0xe174, 0);
      symbols[plusstopSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "plus stop"),                0xe175, 0);
      symbols[upbowSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "up bow"),                   0xe176, 0);
      symbols[downbowSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "down bow"),                 0xe177, 0);
      symbols[reverseturnSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "reverse turn"),             0xe178, 0);
      symbols[turnSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "turn"),                     0xe179, 0);
      symbols[trillSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "trill"),                    0xe17a, 0);
      symbols[upedalheelSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "upedal heel"),              0xe17b, 0);
      symbols[dpedalheelSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "dpedalheel"),               0xe17c, 0);
      symbols[upedaltoeSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "upedal toe"),               0xe17d, 0);
      symbols[dpedaltoeSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "dpedal toe"),               0xe17e, 0);
      symbols[flageoletSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "flageolet"),                0xe17f, 0);
      symbols[segnoSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "segno"),                    0xe180, 0);
      symbols[codaSym]                    = Sym(QT_TRANSLATE_NOOP("symbol", "coda"),                     0xe181, 0);
      symbols[varcodaSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "varied coda"),              0xe182, 0);
      symbols[rcommaSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "rcomma"),                   0xe183, 0);
      symbols[lcommaSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "lcomma"),                   0xe184, 0);

      symbols[arpeggioSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "arpeggio"),                 0xe187, 0);
      symbols[trillelementSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "trillelement"),             0xe188, 0);
      symbols[arpeggioarrowdownSym]       = Sym(QT_TRANSLATE_NOOP("symbol", "arpeggio arrow down"),      0xe189, 0);
      symbols[arpeggioarrowupSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "arpeggio arrow up"),        0xe18a, 0);
      symbols[trilelementSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "trill element"),            0xe18b, 0);
      symbols[prallSym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "prall"),                    0xe18c, 0);
      symbols[mordentSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "mordent"),                  0xe18d, 0);
      symbols[prallprallSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "prall prall"),              0xe18e, 0);
      symbols[prallmordentSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "prall mordent"),            0xe18f, 0);
      symbols[upprallSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "up prall"),                 0xe190, 0);
      symbols[upmordentSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "up mordent"),               0xe191, 0);
      symbols[pralldownSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "prall down"),               0xe192, 0);
      symbols[downprallSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "down prall"),               0xe193, 0);
      symbols[downmordentSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "down mordent"),             0xe194, 0);
      symbols[prallupSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "prall up"),                 0xe195, 0);
      symbols[lineprallSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "line prall"),               0xe196, 0);

      symbols[eighthflagSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "eight flag"),               0xe199, 0);
      symbols[sixteenthflagSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "sixteenth flag"),           0xe19a, 0);
      symbols[thirtysecondflagSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "thirtysecond flag"),        0xe19b, 0);
      symbols[sixtyfourthflagSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "sixtyfour flag"),           0xe19c, 0);
      symbols[deighthflagSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "128 flag"),                 0xe19d, 0);
      symbols[deighthflagSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "deight flag"),              0xe19e, 0);
      symbols[gracedashSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "grace dash"),               0xe19f, 0);
      symbols[dgracedashSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "dgrace dash"),              0xe1a0, 0);
      symbols[dsixteenthflagSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "dsixteenth flag"),          0xe1a1, 0);
      symbols[dthirtysecondflagSym]       = Sym(QT_TRANSLATE_NOOP("symbol", "dthirtysecond flag"),       0xe1a2, 0);
      symbols[dsixtyfourthflagSym]        = Sym(QT_TRANSLATE_NOOP("symbol", "dsixtyfourth flag"),        0xe1a3, 0);
      symbols[altoclefSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "alto clef"),                0xe1a4, 0);
      symbols[altoclefSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "alto clef"),                0xe1a5, 0);
      symbols[caltoclefSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "calto clef"),               0xe1a6, 0);
      symbols[bassclefSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "bass clef"),                0xe1a7, 0);
      symbols[cbassclefSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "cbass clef"),               0xe1a8, 0);
      symbols[trebleclefSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "trebleclef"),               0xe1a9, 0);   //G-Clef
      symbols[ctrebleclefSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "ctrebleclef"),              0xe1aa, 0);
      symbols[percussionclefSym]          = Sym(QT_TRANSLATE_NOOP("symbol", "percussion clef"),          0xe1ab, 0);
      symbols[cpercussionclefSym]         = Sym(QT_TRANSLATE_NOOP("symbol", "cpercussion clef"),         0xe1ac, 0);
      symbols[tabclefSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "tab clef"),                 0xe1ad, 0);
      symbols[ctabclefSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "ctab clef"),                0xe1ae, 0);
      symbols[fourfourmeterSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "four four meter"),          0xe1af, 0);
      symbols[allabreveSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "allabreve"),                0xe1b0, 0);
      symbols[pedalasteriskSym]           = Sym(QT_TRANSLATE_NOOP("symbol", "pedalasterisk"),            0xe1b1, 0);
      symbols[pedaldashSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "pedaldash"),                0xe1b2, 0);
      symbols[pedaldotSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "pedaldot"),                 0xe1b3, 0);
      symbols[pedalPSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "pedalP"),                   0xe1b4, 0);
      symbols[pedaldSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "pedald"),                   0xe1b5, 0);
      symbols[pedaleSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "pedale"),                   0xe1b6, 0);
      symbols[pedalPedSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "pedal ped"),                0xe1b7, 0);
      symbols[brackettipsRightUp]         = Sym(QT_TRANSLATE_NOOP("symbol", "bracket tips right up"),    0xe1b8, 0);
      symbols[brackettipsRightDown]       = Sym(QT_TRANSLATE_NOOP("symbol", "bracket tips right down"),  0xe1b9, 0);
      symbols[accDiscantSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "acc discant"),              0xe1ba, 0);
      symbols[accDotSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "acc dot"),                  0xe1bb, 0);
      symbols[accFreebaseSym]             = Sym(QT_TRANSLATE_NOOP("symbol", "acc freebase"),             0xe1bc, 0);
      symbols[accStdbaseSym]              = Sym(QT_TRANSLATE_NOOP("symbol", "acc stdbase"),              0xe1bd, 0);
      symbols[accBayanbaseSym]            = Sym(QT_TRANSLATE_NOOP("symbol", "acc bayanbase"),            0xe1be, 0);
      symbols[accOldEESym]                = Sym(QT_TRANSLATE_NOOP("symbol", "acc old ee"),               0xe1bf, 0);
      symbols[brackettipsLeftUp]          = Sym(QT_TRANSLATE_NOOP("symbol", "bracket tips left up"),     0xe1c0, 0);
      symbols[brackettipsLeftDown]        = Sym(QT_TRANSLATE_NOOP("symbol", "bracket tips left down"),   0xe1c1, 0);

      // used for GUI:
      symbols[note2Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/2"),   0xe104, 1);
      symbols[note4Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/4"),   0xe105, 1);
      symbols[note8Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/8"),   0xe106, 1);
      symbols[note16Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/16"),  0xe107, 1);
      symbols[note32Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/32"),  0xe108, 1);
      symbols[note64Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/64"),  0xe109, 1);
      symbols[dotdotSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "dot dot"),    0xe10b, 1);

//      Sym::writeCtable();


      if (charReplaceMap.isEmpty()) {
            for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
                  if (pSymbols[i].code == 0 || pSymbols[i].text == 0)
                        continue;
                  charReplaceMap.insert(pSymbols[i].text, &pSymbols[i]);
                  }
            }
      }

