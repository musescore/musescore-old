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

struct LilypondNames {
      int msIndex;
      const char* name;
      } lilypondNames[] = {
            { wholerestSym,         "rests.0" },
            { halfrestSym,          "rests.1" },
            { outsidewholerestSym,  "rests.0o" },
            { outsidehalfrestSym,   "rests.1o" },
            { rest_M3,              "rests.M3" },
            { breverestSym,         "rests.M2" },
            { longarestSym,         "rests.M1" },
            { rest4Sym,             "rests.2" },
            { clasquartrestSym,     "rests.2classical" },
            { rest8Sym,             "rests.3" },
            { rest16Sym,            "rests.4" },
            { rest32Sym,            "rests.5" },
            { rest64Sym,            "rests.6" },
            { rest128Sym,           "rests.7" },

            { sharpSym,             "accidentals.sharp" },
            { sharpArrowUpSym,      "accidentals.sharp.arrowup" },
            { sharpArrowDownSym,    "accidentals.sharp.arrowdown" },
            { sharpArrowBothSym,    "accidentals.sharp.arrowboth" },
            { sharpslashSym,        "accidentals.sharp.slashslash.stem" },
            { sharpslash2Sym,       "accidentals.sharp.slashslashslash.stemstem" },
            { sharpslash3Sym,       "accidentals.sharp.slashslashslash.stem" },
            { sharpslash4Sym,       "accidentals.sharp.slashslash.stemstemstem" },
            { naturalSym,           "accidentals.natural" },
            { naturalArrowUpSym,    "accidentals.natural.arrowup" },
            { naturalArrowDownSym,  "accidentals.natural.arrowdown" },
            { naturalArrowBothSym,  "accidentals.natural.arrowboth" },
            { flatSym,              "accidentals.flat" },
            { flatArrowUpSym,       "accidentals.flat.arrowup" },
            { flatArrowDownSym,     "accidentals.flat.arrowdown" },
            { flatArrowBothSym,     "accidentals.flat.arrowboth" },
            { flatslashSym,         "accidentals.flat.slash" },
            { flatslash2Sym,        "accidentals.flat.slashslash" },
            { mirroredflat2Sym,     "accidentals.mirroredflat.flat" },
            { mirroredflatSym,      "accidentals.mirroredflat" },
            { mirroredflatslashSym, "accidentals.mirroredflat.backslash" },
            { flatflatSym,          "accidentals.flatflat" },
            { flatflatslashSym,     "accidentals.flatflat.slash" },
            { sharpsharpSym,        "accidentals.doublesharp" },
            { rightparenSym,        "accidentals.rightparen" },
            { leftparenSym,         "accidentals.leftparen" },
            { -1, "arrowheads.open.01" },
            { -1, "arrowheads.open.0M1" },
            { -1, "arrowheads.open.11" },
            { -1, "arrowheads.open.1M1" },
            { -1, "arrowheads.close.01" },
            { -1, "arrowheads.close.0M1" },
            { -1, "arrowheads.close.11" },
            { -1, "arrowheads.close.1M1" },
            { dotSym,               "dots.dot" },
            { longaupSym,           "noteheads.uM2" },
            { longadownSym,         "noteheads.dM2" },
            { brevisheadSym,        "noteheads.sM1" },
            { wholeheadSym,         "noteheads.s0" },
            { halfheadSym,          "noteheads.s1" },
            { quartheadSym,         "noteheads.s2" },
            { wholediamondheadSym,  "noteheads.s0diamond" },
            { halfdiamondheadSym,   "noteheads.s1diamond" },
            { diamondheadSym,       "noteheads.s2diamond" },
            { s0triangleHeadSym,    "noteheads.s0triangle" },
            { d1triangleHeadSym,    "noteheads.d1triangle" },
            { u1triangleHeadSym,    "noteheads.u1triangle" },
            { u2triangleHeadSym,    "noteheads.u2triangle" },
            { d2triangleHeadSym,    "noteheads.d2triangle" },
            { wholeslashheadSym,    "noteheads.s0slash" },
            { halfslashheadSym,     "noteheads.s1slash" },
            { quartslashheadSym,    "noteheads.s2slash" },
            { wholecrossedheadSym,  "noteheads.s0cross" },
            { halfcrossedheadSym,   "noteheads.s1cross" },
            { crossedheadSym,       "noteheads.s2cross" },
            { xcircledheadSym,      "noteheads.s2xcircle" },
            { s0doHeadSym,          "noteheads.s0do" },
            { d1doHeadSym,          "noteheads.d1do" },
            { u1doHeadSym,          "noteheads.u1do" },
            { d2doHeadSym,          "noteheads.d2do" },
            { u2doHeadSym,          "noteheads.u2do" },
            { s0reHeadSym,          "noteheads.s0re" },
            { u1reHeadSym,          "noteheads.u1re" },
            { d1reHeadSym,          "noteheads.d1re" },
            { u2reHeadSym,          "noteheads.u2re" },
            { d2reHeadSym,          "noteheads.d2re" },
            { s0miHeadSym,          "noteheads.s0mi" },
            { s1miHeadSym,          "noteheads.s1mi" },
            { s2miHeadSym,          "noteheads.s2mi" },
            { u0faHeadSym,          "noteheads.u0fa" },
            { d0faHeadSym,          "noteheads.d0fa" },
            { u1faHeadSym,          "noteheads.u1fa" },
            { d1faHeadSym,          "noteheads.d1fa" },
            { u2faHeadSym,          "noteheads.u2fa" },
            { d2faHeadSym,          "noteheads.d2fa" },
            { s0laHeadSym,          "noteheads.s0la" },
            { s1laHeadSym,          "noteheads.s1la" },
            { s2laHeadSym,          "noteheads.s2la" },
            { s0tiHeadSym,          "noteheads.s0ti" },
            { u1tiHeadSym,          "noteheads.u1ti" },
            { d1tiHeadSym,          "noteheads.d1ti" },
            { u2tiHeadSym,          "noteheads.u2ti" },
            { d2tiHeadSym,          "noteheads.d2ti" },
            { ufermataSym,          "scripts.ufermata" },
            { dfermataSym,          "scripts.dfermata" },

            { -1, "scripts.ushortfermata" },
            { -1, "scripts.dshortfermata" },
            { -1, "scripts.ulongfermata" },
            { -1, "scripts.dlongfermata" },
            { -1, "scripts.uverylongfermata" },
            { -1, "scripts.dverylongfermata" },

            { thumbSym,             "scripts.thumb" },
            { sforzatoaccentSym,    "scripts.sforzato" },
            { esprSym,              "scripts.espr" },
            { staccatoSym,          "scripts.staccato" },
            { ustaccatissimoSym,    "scripts.ustaccatissimo" },
            { dstaccatissimoSym,    "scripts.dstaccatissimo" },
            { tenutoSym,            "scripts.tenuto" },
            { uportatoSym,          "scripts.uportato" },
            { dportatoSym,          "scripts.dportato" },
            { umarcatoSym,          "scripts.umarcato" },
            { dmarcatoSym,          "scripts.dmarcato" },
            { ouvertSym,            "scripts.open" },
            { plusstopSym,          "scripts.stopped" },
            { upbowSym,             "scripts.upbow" },
            { downbowSym,           "scripts.downbow" },
            { reverseturnSym,       "scripts.reverseturn" },
            { turnSym,              "scripts.turn" },
            { trillSym,             "scripts.trill" },
            { upedalheelSym,        "scripts.upedalheel" },
            { dpedalheelSym,        "scripts.dpedalheel" },

            { upedaltoeSym,         "scripts.upedaltoe" },
            { dpedaltoeSym,         "scripts.dpedaltoe" },
            { flageoletSym,         "scripts.flageolet" },
            { segnoSym,             "scripts.segno" },
            { codaSym,              "scripts.coda" },
            { varcodaSym,           "scripts.varcoda" },

            { rcommaSym,            "scripts.rcomma" },
            { lcommaSym,            "scripts.lcomma" },

            { -1, "scripts.rvarcomma" },
            { -1, "scripts.lvarcomma" },

            { arpeggioSym,          "scripts.arpeggio" },
            { trillelementSym,      "scripts.trill_element" },
            { arpeggioarrowdownSym, "scripts.arpeggio.arrow.M1" },
            { arpeggioarrowupSym,   "scripts.arpeggio.arrow.1" },
            { trilelementSym,       "scripts.trilelement" },
            { prallSym,             "scripts.prall" },
            { mordentSym,           "scripts.mordent" },
            { prallprallSym,        "scripts.prallprall" },
            { prallmordentSym,      "scripts.prallmordent" },
            { upprallSym,           "scripts.upprall" },

            { upmordentSym,         "scripts.upmordent" },
            { pralldownSym,         "scripts.pralldown" },
            { downprallSym,         "scripts.downprall" },
            { downmordentSym,       "scripts.downmordent" },
            { prallupSym,           "scripts.prallup" },
            { lineprallSym,         "scripts.lineprall" },
            { -1, "scripts.caesura.curved" },
            { -1, "scripts.caesura.straight" },

            { eighthflagSym,        "flags.u3" },
            { sixteenthflagSym,     "flags.u4" },
            { thirtysecondflagSym,  "flags.u5" },
            { sixtyfourthflagSym,   "flags.u6" },
            { flag128Sym,           "flags.u7" },
            { deighthflagSym,       "flags.d3" },
            { gracedashSym,         "flags.ugrace" },
            { dgracedashSym,        "flags.dgrace" },
            { dsixteenthflagSym,    "flags.d4" },
            { dthirtysecondflagSym, "flags.d5" },
            { dsixtyfourthflagSym,  "flags.d6" },
            { dflag128Sym,          "flags.d7" },
            { altoclefSym,          "clefs.C" },
            { caltoclefSym,         "clefs.C_change" },
            { bassclefSym,          "clefs.F" },
            { cbassclefSym,         "clefs.F_change" },
            { trebleclefSym,        "clefs.G" },
            { ctrebleclefSym,       "clefs.G_change" },
            { percussionclefSym,    "clefs.percussion" },
            { cpercussionclefSym,   "clefs.percussion_change" },
            { tabclefSym,           "clefs.tab" },
            { ctabclefSym,          "clefs.tab_change" },
            { fourfourmeterSym,     "timesig.C44" },
            { allabreveSym,         "timesig.C22" },
            { pedalasteriskSym,     "pedal.*" },
            { pedaldashSym,         "pedal.M" },
            { pedaldotSym,          "pedal.." },
            { pedalPSym,            "pedal.P" },
            { pedaldSym,            "pedal.d" },
            { pedaleSym,            "pedal.e" },
            { pedalPedSym,          "pedal.Ped" },

            { brackettipsRightUp,   "brackettips.uright"     },
            { brackettipsRightDown, "brackettips.dright"     },
            { brackettipsLeftUp,    "brackettips.uleft"      },
            { brackettipsLeftDown,  "brackettips.dleft"      },
            { accDotSym,            "accordion.accDot"       },
            { accFreebaseSym,       "accordion.accFreebase"  },
            { accStdbaseSym,        "accordion.accStdbase"   },
            { accBayanbaseSym,      "accordion.accBayanbase" },
            { accOldEESym,          "accordion.accOldEE"     },
            { accDiscantSym,        "accordion.accDiscant"   },

            { -1, "left up" },
            { -1, "left down" },
            { -1, "plus" },
            { -1, "comma" },
            { -1, "hyphen" },
            { -1, "period" },

            { zeroSym,        "zero" },
            { oneSym,         "one" },
            { twoSym,         "two" },
            { threeSym,       "three" },
            { fourSym,        "four" },
            { fiveSym,        "five" },
            { sixSym,         "six" },
            { sevenSym,       "seven" },
            { eightSym,       "eight" },
            { nineSym,        "nine" },

            { -1, "space" },

            { letterzSym,     "z" },
            { letterfSym,     "f" },
            { lettersSym,     "s" },
            { letterpSym,     "p" },
            { lettermSym,     "m" },
            { letterzSym,     "r" },
      };

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

Sym::Sym(const char* name, int c, int fid, double ax, double ay)
   : _code(c), fontId(fid), _name(name), _font(fontId2font(fontId)), _attach(ax * DPI/PPI, ay * DPI/PPI)
      {
      QFontMetricsF fm(_font);
      if (!fm.inFont(_code))
            printf("Sym: character 0x%x(%d) <%s> are not in font <%s>\n", _code.unicode(),c, _name, qPrintable(_font.family()));
      w     = fm.width(_code);
      _bbox = fm.boundingRect(_code);
      }

Sym::Sym(const char* name, int c, int fid, const QPointF& a, const QRectF& b)
   : _code(c), fontId(fid), _name(name), _font(fontId2font(fontId))
      {
      double s = DPI/PPI;
      _bbox.setRect(b.x() * s, b.y() * s, b.width() * s, b.height() * s);
      _attach = a * s;
      w = _bbox.width();
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
      painter.drawText(x * imag, y * imag, QString(_code));
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
      painter.drawText(x * imag, y * imag, QString(n, _code));
      painter.scale(imag, imag);
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
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      symbols[clefEightSym]               = Sym(QT_TRANSLATE_NOOP("symbol", "clef eight"),                 0x38, 2);
      symbols[clefOneSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "clef one"),                   0x31, 2);
      symbols[clefFiveSym]                = Sym(QT_TRANSLATE_NOOP("symbol", "clef five"),                  0x35, 2);
      symbols[letterfSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "f"),                          0x66, 1);
      symbols[lettermSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "m"),                          0x6d, 1);
      symbols[letterpSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "p"),                          0x70, 1);
      symbols[letterrSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "r"),                          0x72, 1);
      symbols[lettersSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "s"),                          0x73, 1);
      symbols[letterzSym]                 = Sym(QT_TRANSLATE_NOOP("symbol", "z"),                          0x7a, 1);
      // used for GUI:
      symbols[note2Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/2"),   0xe104, 1);
      symbols[note4Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/4"),   0xe105, 1);
      symbols[note8Sym]                   = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/8"),   0xe106, 1);
      symbols[note16Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/16"),  0xe107, 1);
      symbols[note32Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/32"),  0xe108, 1);
      symbols[note64Sym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "note 1/64"),  0xe109, 1);
      symbols[dotdotSym]                  = Sym(QT_TRANSLATE_NOOP("symbol", "dot dot"),    0xe10b, 1);


      QHash<QString, int> lnhash;
      for (unsigned int i = 0; i < sizeof(lilypondNames)/sizeof(*lilypondNames); ++i)
            lnhash[QString(lilypondNames[i].name)] = lilypondNames[i].msIndex;

      QFile f(":/data/symbols.xml");
      if (!f.open(QFile::ReadOnly)) {
            printf("cannot open symbols file\n");
            exit(-1);
            }
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(&f, false, &err, &line, &column)) {
            QString error;
            error.sprintf("error reading style file %s at line %d column %d: %s\n",
               f.fileName().toLatin1().data(), line, column, err.toLatin1().data());
            QMessageBox::warning(0,
               QWidget::tr("MuseScore: Load font symbols failed:"),
               error,
               QString::null, QWidget::tr("Quit"), QString::null, 0, 1);
            return;
            }
      f.close();
      docName = f.fileName();
      for (QDomElement e = doc.documentElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "museScore") {
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull();  ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Glyph") {
                              QString name;
                              int code = -1;
                              QPointF p;
                              QRectF b;
                              for (QDomElement eee = ee.firstChildElement(); !eee.isNull();  eee = eee.nextSiblingElement()) {
                                    QString tag(eee.tagName());
                                    QString val(eee.text());
                                    if (tag == "name")
                                          name = val;
                                    else if (tag == "code") {
                                          bool ok;
                                          code = val.mid(2).toInt(&ok, 16);
                                          if (!ok)
                                                printf("cannot read code\n");
                                          }
                                    else if (tag == "attach")
                                          p = readPoint(eee);
                                    else if (tag == "bbox")
                                          b = readRectF(eee);
                                    else
                                          domError(eee);
                                    }
                              if (code == -1)
                                    printf("no code for glyph <%s>\n", qPrintable(name));
                              int idx = lnhash[name];
                              if (idx > 0)
                                    symbols[idx] = Sym(qPrintable(name), code, 0, p, b);
                              else if (idx == 0)
                                    printf("symbol <%s> not found\n", qPrintable(name));
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }

      if (charReplaceMap.isEmpty()) {
            for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
                  if (pSymbols[i].code == 0 || pSymbols[i].text == 0)
                        continue;
                  charReplaceMap.insert(pSymbols[i].text, &pSymbols[i]);
                  }
            }
      }

