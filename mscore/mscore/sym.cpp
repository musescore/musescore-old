//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: symbols.cpp,v 1.34 2006/04/06 13:03:11 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "painter.h"
#include "mscore.h"
#include "score.h"

QVector<Sym> symbols(lastSym);

static const double FMAG = 1.0;

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const char* name, const QChar& c, int f)
   : _code(c), fontId(f), _name(name)
      {
      QPrinter printer(QPrinter::HighResolution);

      QFont font;
      if (fontId == 0) {
            font = QFont("MScore");
            font.setPointSizeF(20.0 * FMAG);
            }
      else if (fontId == 1) {
            font = QFont("MScore");
            font.setPointSizeF(14.0 * FMAG);
            }
      else {
            font = QFont("Times New Roman");
            font.setPointSizeF(8.0 * FMAG);
            }
      _bbox = QFontMetricsF(font, &printer).boundingRect(_code);
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

const QRectF Sym::bbox() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * 1200.0);
      return QRectF(_bbox.topLeft() * m, _bbox.size() * m);
      }

double Sym::width() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * 1200.0);
      return _bbox.width() * m;
      }

double Sym::height() const
      {
      double m = _spatium / (spatiumBase20 * FMAG * 1200.0);
      return _bbox.height() * m;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y, int n) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y), QString(n, _code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter) const
      {
      painter.setFont(font());
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
            QFont f("MScore");
            f.setPointSizeF(14.0 * mag);
            return f;
            }
      else {
            QFont f("Times New Roman");
            f.setPointSizeF(8.0 * mag);
            return f;
            }
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      symbols[clefEightSym] = Sym("clef eight", 0x38, 2);
      symbols[clefOneSym]   = Sym("clef one",   0x31, 2);
      symbols[clefFiveSym]  = Sym("clef five",  0x35, 2);

      symbols[stemSym]      = Sym("stem",  0xf08f, 0);
      symbols[stemSym]      = Sym("stem",  0xf08f, 0);
      symbols[dstemSym]     = Sym("dstem", 0xf090, 0);

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

//      symbols[wholerestSym]               = Sym("whole rest",         0xe100, 0, QPointF(.0,-1.) );
      symbols[wholerestSym]               = Sym("whole rest",         0xe100, 0);

      symbols[halfrestSym]                = Sym("half rest",          0xe101, 0);
      symbols[outsidewholerestSym]        = Sym("outside whole rest", 0xe102, 0);
      symbols[outsidehalfrestSym]         = Sym("outside half rest",  0xe103, 0);
      symbols[rest_M3]                    = Sym("rest M3",            0xe104, 0);
      symbols[longarestSym]               = Sym("longa rest",         0xe105, 0);
      symbols[breverestSym]               = Sym("breve rest",         0xe106, 0);
      symbols[quartrestSym]               = Sym("quart rest",         0xe107, 0);
      symbols[clasquartrestSym]           = Sym("clas quart rest",    0xe108, 0);
      symbols[eighthrestSym]              = Sym("eight rest",         0xe109, 0);
      symbols[sixteenthrestSym]           = Sym("16' rest",           0xe10a, 0);
      symbols[thirtysecondrestSym]        = Sym("32'  rest",          0xe10b, 0);
      symbols[sixtyfourthrestSym]         = Sym("64' rest",           0xe10c, 0);
      symbols[hundredtwentyeighthrestSym] = Sym("128' rest",          0xe10d, 0);

      symbols[sharpSym]                   = Sym("sharp",                    0xe10e, 0);
      symbols[naturalSym]                 = Sym("natural",                  0xe111, 0);
      symbols[flatSym]                    = Sym("flat",                     0xe112, 0);
      symbols[flatflatSym]                = Sym("flat flat",                0xe114, 0);
      symbols[sharpsharpSym]              = Sym("sharp sharp",              0xe116, 0);
      symbols[rightparenSym]              = Sym("right parenthesis",        0xe117, 0);
      symbols[leftparenSym]               = Sym("left parenthesis",         0xe118, 0);
      symbols[dotSym]                     = Sym("dot",                      0xe119, 0);
      symbols[brevisheadSym]              = Sym("brevis head",              0xe11a, 0);
      symbols[wholeheadSym]               = Sym("whole head",               0xe11b, 0);
      symbols[halfheadSym]                = Sym("half head",                0xe11c, 0);
      symbols[quartheadSym]               = Sym("quart head",               0xe11d, 0);

      symbols[wholediamondheadSym]        = Sym("whole diamond head",       0xe11e, 0);
      symbols[halfdiamondheadSym]         = Sym("half diamond head",        0xe11f, 0);
      symbols[diamondheadSym]             = Sym("diamond head",             0xe120, 0);
      symbols[wholetriangleheadSym]       = Sym("whole triangle head",      0xe121, 0);
      symbols[halftriangleheadSym]        = Sym("half triangle head",       0xe122, 0);
      symbols[triangleheadSym]            = Sym("triangle head",            0xe124, 0);

      symbols[wholeslashheadSym]          = Sym("whole slash head",         0xe126, 0);
      symbols[halfslashheadSym]           = Sym("half slash head",          0xe127, 0);
      symbols[quartslashheadSym]          = Sym("quart slash head",         0xe128, 0);

      symbols[wholecrossedheadSym]        = Sym("whole cross head",         0xe129, 0);
      symbols[halfcrossedheadSym]         = Sym("half cross head",          0xe12a, 0);
      symbols[crossedheadSym]             = Sym("cross head",               0xe12b, 0);
      symbols[xcircledheadSym]            = Sym("x circle head",            0xe12c, 0);

      symbols[ufermataSym]                = Sym("ufermata",                 0xe148, 0);
      symbols[dfermataSym]                = Sym("dfermata",                 0xe149, 0);
      symbols[thumbSym]                   = Sym("thumb",                    0xe150, 0);
      symbols[sforzatoaccentSym]          = Sym("sforza to accent",         0xe151, 0);
      symbols[esprSym]                    = Sym("espressivo",               0xe152, 0);
      symbols[staccatoSym]                = Sym("staccato",                 0xe153, 0);
      symbols[ustaccatissimoSym]          = Sym("ustaccatissimo",           0xe154, 0);
      symbols[dstaccatissimoSym]          = Sym("dstacattissimo",           0xe155, 0);

      symbols[tenutoSym]                  = Sym("tenuto",                   0xe156, 0);
      symbols[uportatoSym]                = Sym("uportato",                 0xe157, 0);
      symbols[dportatoSym]                = Sym("dportato",                 0xe158, 0);
      symbols[umarcatoSym]                = Sym("umarcato",                 0xe159, 0);
      symbols[dmarcatoSym]                = Sym("dmarcato",                 0xe15a, 0);
      symbols[ouvertSym]                  = Sym("ouvert",                   0xe15b, 0);
      symbols[plusstopSym]                = Sym("plus stop",                0xe15c, 0);
      symbols[upbowSym]                   = Sym("up bow",                   0xe15d, 0);
      symbols[downbowSym]                 = Sym("down bow",                 0xe15e, 0);
      symbols[reverseturnSym]             = Sym("reverse turn",             0xe15f, 0);
      symbols[turnSym]                    = Sym("turn",                     0xe160, 0);
      symbols[trillSym]                   = Sym("trill",                    0xe161, 0);
      symbols[upedalheelSym]              = Sym("upedal heel",              0xe162, 0);
      symbols[dpedalheelSym]              = Sym("dpedalheel",               0xe163, 0);
      symbols[upedaltoeSym]               = Sym("upedal toe",               0xe164, 0);
      symbols[dpedaltoeSym]               = Sym("dpedal toe",               0xe165, 0);

      symbols[flageoletSym]               = Sym("flageolet",                0xe166, 0);
      symbols[segnoSym]                   = Sym("segno",                    0xe167, 0);
      symbols[codaSym]                    = Sym("coda",                     0xe168, 0);
      symbols[varcodaSym]                 = Sym("varcoda",                  0xe169, 0);

      symbols[rcommaSym]                  = Sym("rcomma",                   0xe16a, 0);
      symbols[lcommaSym]                  = Sym("lcomma",                   0xe16b, 0);
      symbols[arpeggioSym]                = Sym("arpeggio",                 0xe16e, 0);
      symbols[trillelementSym]            = Sym("trillelement",             0xe16f, 0);
      symbols[arpeggioarrowdownSym]       = Sym("arpeggio arrow down",      0xe170, 0);
      symbols[arpeggioarrowupSym]         = Sym("arpeggio arrow up",        0xe171, 0);
      symbols[trilelementSym]             = Sym("trill element",            0xe172, 0);
      symbols[prallSym]                   = Sym("prall",                    0xe173, 0);
      symbols[mordentSym]                 = Sym("mordent",                  0xe174, 0);
      symbols[prallprallSym]              = Sym("prall prall",              0xe175, 0);
      symbols[prallmordentSym]            = Sym("prall mordent",            0xe176, 0);
      symbols[upprallSym]                 = Sym("up prall",                 0xe177, 0);
      symbols[upmordentSym]               = Sym("up mordent",               0xe178, 0);
      symbols[pralldownSym]               = Sym("prall down",               0xe179, 0);
      symbols[downprallSym]               = Sym("down prall",               0xe17a, 0);
      symbols[downmordentSym]             = Sym("down mordent",             0xe17b, 0);
      symbols[prallupSym]                 = Sym("prall up",                 0xe17c, 0);
      symbols[lineprallSym]               = Sym("line prall",               0xe17d, 0);
      symbols[eighthflagSym]              = Sym("eight flag",               0xe17f, 0);
      symbols[sixteenthflagSym]           = Sym("sixteenth flag",           0xe180, 0);
      symbols[thirtysecondflagSym]        = Sym("thirtysecond flag",        0xe181, 0);
      symbols[sixtyfourthflagSym]         = Sym("sixtyfour flag",           0xe182, 0);
      symbols[deighthflagSym]             = Sym("deight flag",              0xe183, 0);
      symbols[gracedashSym]               = Sym("grace dash",               0xe184, 0);
      symbols[dgracedashSym]              = Sym("dgrace dash",              0xe185, 0);
      symbols[dsixteenthflagSym]          = Sym("dsixteenth flag",          0xe186, 0);
      symbols[dthirtysecondflagSym]       = Sym("dthirtysecond flag",       0xe187, 0);
      symbols[dsixtyfourthflagSym]        = Sym("dsixtyfourth flag",        0xe188, 0);
      symbols[altoclefSym]                = Sym("alto clef",                0xe189, 0);

      symbols[caltoclefSym]               = Sym("calto clef",               0xe18a, 0);
      symbols[bassclefSym]                = Sym("bass clef",                0xe18b, 0);
      symbols[cbassclefSym]               = Sym("cbass clef",               0xe18c, 0);
      symbols[trebleclefSym]              = Sym("trebleclef",               0xe18d, 0);   //G-Clef
      symbols[ctrebleclefSym]             = Sym("ctrebleclef",              0xe18e, 0);
      symbols[percussionclefSym]          = Sym("percussion clef",          0xe18f, 0);
      symbols[cpercussionclefSym]         = Sym("cpercussion clef",         0xe190, 0);
      symbols[tabclefSym]                 = Sym("tab clef",                 0xe191, 0);
      symbols[ctabclefSym]                = Sym("ctab clef",                0xe192, 0);

      symbols[fourfourmeterSym]           = Sym("four four meter",          0xe193, 0);
      symbols[allabreveSym]               = Sym("allabreve",                0xe194, 0);
      symbols[pedalasteriskSym]           = Sym("pedalasterisk",            0xe195, 0);
      symbols[pedaldashSym]               = Sym("pedaldash",                0xe196, 0);
      symbols[pedaldotSym]                = Sym("pedaldot",                 0xe197, 0);

      symbols[pedalPSym]                  = Sym("pedalP",                   0xe198, 0);
      symbols[pedaldSym]                  = Sym("pedald",                   0xe199, 0);
      symbols[pedaleSym]                  = Sym("pedale",                   0xe19a, 0);
      symbols[pedalPedSym]                = Sym("pedal ped",                0xe19b, 0);

      symbols[brackettipsUp]              = Sym("bracket ticks up",         0xe19c, 0);
      symbols[brackettipsDown]            = Sym("bracket ticks down",       0xe19d, 0);

      symbols[accDiscantSym]              = Sym("acc discant",              0xe19e, 0);
      symbols[accDotSym]                  = Sym("acc dot",                  0xe19f, 0);
      symbols[accFreebaseSym]             = Sym("acc freebase",             0xe1a0, 0);
      symbols[accStdbaseSym]              = Sym("acc stdbase",              0xe1a1, 0);
      symbols[accBayanbaseSym]            = Sym("acc bayanbase",            0xe1a2, 0);
      symbols[accSBSym]                   = Sym("acc sb",                   0xf0a9, 0);
      symbols[accBBSym]                   = Sym("acc bb",                   0xf0aa, 0);
      symbols[accOldEESym]                = Sym("acc old ee",               0xe1a3, 0);
      symbols[accOldEESSym]               = Sym("acc old ees",              0xf0ac, 0);
      symbols[wholedoheadSym]             = Sym("whole do head",            0xf0ad, 0);
      symbols[halfdoheadSym]              = Sym("half do head",             0xf0ae, 0);
      symbols[doheadSym]                  = Sym("do head",                  0xf0af, 0);

      symbols[wholereheadSym]             = Sym("whole re head",             0xf0b0, 0);
      symbols[halfreheadSym]              = Sym("half re head",              0xf0b1, 0);
      symbols[reheadSym]                  = Sym("re head",                   0xf0b2, 0);
      symbols[wholemeheadSym]             = Sym("whole me head",             0xf0b3, 0);
      symbols[halfmeheadSym]              = Sym("half me head",              0xf0b4, 0);
      symbols[meheadSym]                  = Sym("me head",                   0xf0b5, 0);
      symbols[wholefaheadSym]             = Sym("whole fa head",             0xf0b6, 0);
      symbols[halffauheadSym]             = Sym("half fau head",             0xf0b7, 0);
      symbols[fauheadSym]                 = Sym("fau head",                  0xf0b8, 0);
      symbols[halffadheadSym]             = Sym("half fad head",             0xf0b9, 0);
      symbols[fadheadSym]                 = Sym("fad head",                  0xf0ba, 0);
      symbols[wholelaheadSym]             = Sym("whole la head",             0xf0bb, 0);
      symbols[halflaheadSym]              = Sym("half la head",              0xf0bc, 0);
      symbols[laheadSym]                  = Sym("la head",                   0xf0bd, 0);
      symbols[wholeteheadSym]             = Sym("whole te head",             0xf0be, 0);
      symbols[halfteheadSym]              = Sym("half te head",              0xf0bf, 0);
      symbols[teheadSym]                  = Sym("te head",                   0xf0c0, 0);

      symbols[s_quartheadSym]             = Sym("small quart head",          0xe11d, 1);
      symbols[s_halfheadSym]              = Sym("small half head",           0xe11c, 1);
      symbols[s_wholeheadSym]             = Sym("small whole head",          0xe11b, 1);
      symbols[s_brevisheadSym]            = Sym("small brevis head",         0xe11a, 1);
      symbols[s_dotSym]                   = Sym("small dot",                 0xe119, 1);

      symbols[s_eighthflagSym]            = Sym("small eight flag",          0xe17f, 1);
      symbols[s_sixteenthflagSym]         = Sym("small sixteenth flag",      0xe180, 1);
      symbols[s_thirtysecondflagSym]      = Sym("small thirtysecond flag",   0xe181, 1);
      symbols[s_sixtyfourthflagSym]       = Sym("small sixtyfourth flag",    0xe182, 1);
      symbols[s_deighthflagSym]           = Sym("small deight flag",         0xe183, 1);
      symbols[s_dsixteenthflagSym]        = Sym("small d sixteenth flag",    0xe186, 1);
      symbols[s_dthirtysecondflagSym]     = Sym("small d thirtysecond flag", 0xe187, 1);
      symbols[s_dsixtyfourthflagSym]      = Sym("small d sixtyfourth flag",  0xe188, 1);

      symbols[s_sharpSym]                 = Sym("small sharp",               0xe10e, 1);
      symbols[s_naturalSym]               = Sym("small natural",             0xe111, 1);
      symbols[s_flatSym]                  = Sym("small flat",                0xe112, 1);
      symbols[s_flatflatSym]              = Sym("small flat flat",           0xe114, 1);
      symbols[s_sharpsharpSym]            = Sym("small sharp sharp",         0xe116, 1);
      symbols[flipSym]                    = Sym("flip stem",  0xe0fd, 0);

      // used for GUI:
      symbols[note4Sym]                   = Sym("note 1/4",   0xe0fc, 0);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe0f8, 0);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe0f9, 0);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe0fa, 0);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe0fb, 0);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe0fd, 0);

#if 0
// some debug output
//      Sym* s = &symbols[clefEightSym];
      Sym* s = &symbols[quartheadSym];
      QFontMetricsF fm(s->font());
      printf("screen metrics:  quart  l %f  w %f(%f,%f) r %f\n",
         fm.leftBearing(s->code()), fm.width(s->code()),
         s->width(), s->bbox().x(),
         fm.rightBearing(s->code()));

      QFont nf(s->font());
      nf.setPointSizeF(s->font().pointSizeF() * 10.0);
      fm = QFontMetricsF(nf);
      printf("screen metrics10: quart l %f  w %f(%f,%f) r %f\n",
         fm.leftBearing(s->code()), fm.width(s->code()),
         s->width(), s->bbox().x(),
         fm.rightBearing(s->code()));

      QPrinter printer(QPrinter::HighResolution);
      QFontMetricsF nfm(s->font(), &printer);
      printf("printer metrics: quart  l %f  w %f(%f,%f) r %f\n",
         nfm.leftBearing(s->code()),
         nfm.width(s->code()),
         nfm.boundingRect(s->code()).width(),
         nfm.boundingRect(s->code()).x(),
         nfm.rightBearing(s->code()));
#endif
      }

