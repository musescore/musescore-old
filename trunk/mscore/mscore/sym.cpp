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

QVector<Sym> symbols(lastSym);

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const QString& name, const QChar& c, const QFont& f, const QPointF& o)
   : _code(c), _font(f), _name(name), _offset(o)
      {
      updateBoundingRect();
      }

Sym::Sym()
      {
      _code = 0;
      }

void Sym::setOffset(QPointF& p)
      {
      _offset = p;
      updateBoundingRect();
      }

void Sym::setCode(const QChar& c)
      {
      _code = c;
      updateBoundingRect();
      }

//---------------------------------------------------------
//   updateBoundingRect
//---------------------------------------------------------

void Sym::updateBoundingRect()
      {
      QFontMetricsF fm(_font);
      _boundingRect = fm.boundingRect(_code).translated(_offset * _spatium);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y) const
      {
      painter.setFont(_font);
      painter.drawText(QPointF(x, y) + (_offset * _spatium), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y, int n) const
      {
      painter.setFont(_font);
      painter.drawText(QPointF(x, y) + (_offset * _spatium), QString(n, _code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter) const
      {
      painter.setFont(_font);
      painter.drawText(_offset * _spatium, QString(_code));
      }

//---------------------------------------------------------
//   buildin
//---------------------------------------------------------

int Sym::buildin(const QString& name)
      {
      if (name == "flat")
            return flatSym;
      if (name == "sharp")
            return sharpSym;
      return flatSym;               // DEBUG
      }

//---------------------------------------------------------
//   initSymbols
//---------------------------------------------------------

void initSymbols()
      {
      QFont f1("emmentaler");
      f1.setPixelSize(lrint(20.0 * .2 * _spatium));
      QFont f3("emmentaler");
      f3.setPixelSize(lrint(14.0 * .2 * _spatium));

      QFont f4("Times New Roman");
      f4.setPixelSize(lrint(8.0 * .2 * _spatium));

      symbols[clefEightSym] = Sym("clef eight", 0x38, f4);
      symbols[clefOneSym]   = Sym("clef one",   0x31, f4);
      symbols[clefFiveSym]  = Sym("clef five",  0x35, f4);

      symbols[stemSym]                    = Sym("stem", 0xf08f, f1);
      symbols[stemSym]                    = Sym("stem",               0xf08f, f1);
      symbols[dstemSym]                   = Sym("dstem",              0xf090, f1);

      symbols[plusSym]                    = Sym("plus",               0x2b, f1);

      symbols[zeroSym]                    = Sym("zero",               0x30, f1);
      symbols[oneSym]                     = Sym("one",                0x31, f1);
      symbols[twoSym]                     = Sym("two",                0x32, f1);
      symbols[threeSym]                   = Sym("three",              0x33, f1);
      symbols[fourSym]                    = Sym("four",               0x34, f1);
      symbols[fiveSym]                    = Sym("five",               0x35, f1);
      symbols[sixSym]                     = Sym("six",                0x36, f1);
      symbols[sevenSym]                   = Sym("seven",              0x37, f1);
      symbols[eightSym]                   = Sym("eight",              0x38, f1);
      symbols[nineSym]                    = Sym("nine",               0x39, f1);

      symbols[letterfSym]                 = Sym("f",                  0x66, f1);
      symbols[lettermSym]                 = Sym("m",                  0x6d, f1);
      symbols[letterpSym]                 = Sym("p",                  0x70, f1);
      symbols[letterrSym]                 = Sym("r",                  0x72, f1);
      symbols[lettersSym]                 = Sym("s",                  0x73, f1);
      symbols[letterzSym]                 = Sym("z",                  0x7a, f1);

      symbols[wholerestSym]               = Sym("whole rest",         0xe100, f1, QPointF(.0,-1.) );
      symbols[halfrestSym]                = Sym("half rest",          0xe101, f1);
      symbols[outsidewholerestSym]        = Sym("outside whole rest", 0xe102, f1);
      symbols[outsidehalfrestSym]         = Sym("outside half rest",  0xe103, f1);
      symbols[rest_M3]                    = Sym("rest M3",            0xe104, f1);
      symbols[longarestSym]               = Sym("longa rest",         0xe105, f1);
      symbols[breverestSym]               = Sym("breve rest",         0xe106, f1);
      symbols[quartrestSym]               = Sym("quart rest",         0xe107, f1);
      symbols[clasquartrestSym]           = Sym("clas quart rest",    0xe108, f1);
      symbols[eighthrestSym]              = Sym("eight rest",         0xe109, f1);
      symbols[sixteenthrestSym]           = Sym("16' rest",           0xe10a, f1);
      symbols[thirtysecondrestSym]        = Sym("32'  rest",          0xe10b, f1);
      symbols[sixtyfourthrestSym]         = Sym("64' rest",           0xe10c, f1);
      symbols[hundredtwentyeighthrestSym] = Sym("128' rest",          0xe10d, f1);

      symbols[sharpSym]                   = Sym("sharp",                    0xe10e, f1);
      symbols[naturalSym]                 = Sym("natural",                  0xe111, f1);
      symbols[flatSym]                    = Sym("flat",                     0xe112, f1);
      symbols[flatflatSym]                = Sym("flat flat",                0xe114, f1);
      symbols[sharpsharpSym]              = Sym("sharp sharp",              0xe116, f1);
      symbols[rightparenSym]              = Sym("right parenthesis",        0xe117, f1);
      symbols[leftparenSym]               = Sym("left parenthesis",         0xe118, f1);
      symbols[dotSym]                     = Sym("dot",                      0xe119, f1);
      symbols[brevisheadSym]              = Sym("brevis head",              0xe11a, f1);
      symbols[wholeheadSym]               = Sym("whole head",               0xe11b, f1);
      symbols[halfheadSym]                = Sym("half head",                0xe11c, f1);
      symbols[quartheadSym]               = Sym("quart head",               0xe11d, f1);

      symbols[wholediamondheadSym]        = Sym("whole diamond head",       0xe11e, f1);
      symbols[halfdiamondheadSym]         = Sym("half diamond head",        0xe11f, f1);
      symbols[diamondheadSym]             = Sym("diamond head",             0xe120, f1);
      symbols[wholetriangleheadSym]       = Sym("whole triangle head",      0xe121, f1);
      symbols[halftriangleheadSym]        = Sym("half triangle head",       0xe122, f1);
      symbols[triangleheadSym]            = Sym("triangle head",            0xe124, f1);

      symbols[wholeslashheadSym]          = Sym("whole slash head",         0xe126, f1);
      symbols[halfslashheadSym]           = Sym("half slash head",          0xe127, f1);
      symbols[quartslashheadSym]          = Sym("quart slash head",         0xe128, f1);

      symbols[wholecrossedheadSym]        = Sym("whole cross head",         0xe129, f1);
      symbols[halfcrossedheadSym]         = Sym("half cross head",          0xe12a, f1);
      symbols[crossedheadSym]             = Sym("cross head",               0xe12b, f1);
      symbols[xcircledheadSym]            = Sym("x circle head",            0xe12c, f1);

      symbols[ufermataSym]                = Sym("ufermata",                 0xe148, f1);
      symbols[dfermataSym]                = Sym("dfermata",                 0xe149, f1);
      symbols[thumbSym]                   = Sym("thumb",                    0xe150, f1);
      symbols[sforzatoaccentSym]          = Sym("sforza to accent",         0xe151, f1);
      symbols[esprSym]                    = Sym("espressivo",               0xe152, f1);
      symbols[staccatoSym]                = Sym("staccato",                 0xe153, f1);
      symbols[ustaccatissimoSym]          = Sym("ustaccatissimo",           0xe154, f1);
      symbols[dstaccatissimoSym]          = Sym("dstacattissimo",           0xe155, f1);

      symbols[tenutoSym]                  = Sym("tenuto",                   0xe156, f1);
      symbols[uportatoSym]                = Sym("uportato",                 0xe157, f1);
      symbols[dportatoSym]                = Sym("dportato",                 0xe158, f1);
      symbols[umarcatoSym]                = Sym("umarcato",                 0xe159, f1);
      symbols[dmarcatoSym]                = Sym("dmarcato",                 0xe15a, f1);
      symbols[ouvertSym]                  = Sym("ouvert",                   0xe15b, f1);
      symbols[plusstopSym]                = Sym("plus stop",                0xe15c, f1);
      symbols[upbowSym]                   = Sym("up bow",                   0xe15d, f1);
      symbols[downbowSym]                 = Sym("down bow",                 0xe15e, f1);
      symbols[reverseturnSym]             = Sym("reverse turn",             0xe15f, f1);
      symbols[turnSym]                    = Sym("turn",                     0xe160, f1);
      symbols[trillSym]                   = Sym("trill",                    0xe161, f1);
      symbols[upedalheelSym]              = Sym("upedal heel",              0xe162, f1);
      symbols[dpedalheelSym]              = Sym("dpedalheel",               0xe163, f1);
      symbols[upedaltoeSym]               = Sym("upedal toe",               0xe164, f1);
      symbols[dpedaltoeSym]               = Sym("dpedal toe",               0xe165, f1);

      symbols[flageoletSym]               = Sym("flageolet",                0xe166, f1);
      symbols[segnoSym]                   = Sym("segno",                    0xe167, f1);
      symbols[codaSym]                    = Sym("coda",                     0xe168, f1);
      symbols[varcodaSym]                 = Sym("varcoda",                  0xe169, f1);

      symbols[rcommaSym]                  = Sym("rcomma",                   0xe16a, f1);
      symbols[lcommaSym]                  = Sym("lcomma",                   0xe16b, f1);
      symbols[arpeggioSym]                = Sym("arpeggio",                 0xe16e, f1);
      symbols[trillelementSym]            = Sym("trillelement",             0xe16f, f1);
      symbols[arpeggioarrowdownSym]       = Sym("arpeggio arrow down",      0xe170, f1);
      symbols[arpeggioarrowupSym]         = Sym("arpeggio arrow up",        0xe171, f1);
      symbols[trilelementSym]             = Sym("trill element",            0xe172, f1);
      symbols[prallSym]                   = Sym("prall",                    0xe173, f1);
      symbols[mordentSym]                 = Sym("mordent",                  0xe174, f1);
      symbols[prallprallSym]              = Sym("prall prall",              0xe175, f1);
      symbols[prallmordentSym]            = Sym("prall mordent",            0xe176, f1);
      symbols[upprallSym]                 = Sym("up prall",                 0xe177, f1);
      symbols[upmordentSym]               = Sym("up mordent",               0xe178, f1);
      symbols[pralldownSym]               = Sym("prall down",               0xe179, f1);
      symbols[downprallSym]               = Sym("down prall",               0xe17a, f1);
      symbols[downmordentSym]             = Sym("down mordent",             0xe17b, f1);
      symbols[prallupSym]                 = Sym("prall up",                 0xe17c, f1);
      symbols[lineprallSym]               = Sym("line prall",               0xe17d, f1);
      symbols[eighthflagSym]              = Sym("eight flag",               0xe17f, f1);
      symbols[sixteenthflagSym]           = Sym("sixteenth flag",           0xe180, f1);
      symbols[thirtysecondflagSym]        = Sym("thirtysecond flag",        0xe181, f1);
      symbols[sixtyfourthflagSym]         = Sym("sixtyfour flag",           0xe182, f1);
      symbols[deighthflagSym]             = Sym("deight flag",              0xe183, f1);
      symbols[gracedashSym]               = Sym("grace dash",               0xe184, f1);
      symbols[dgracedashSym]              = Sym("dgrace dash",              0xe185, f1);
      symbols[dsixteenthflagSym]          = Sym("dsixteenth flag",          0xe186, f1);
      symbols[dthirtysecondflagSym]       = Sym("dthirtysecond flag",       0xe187, f1);
      symbols[dsixtyfourthflagSym]        = Sym("dsixtyfourth flag",        0xe188, f1);
      symbols[altoclefSym]                = Sym("alto clef",                0xe189, f1);

      symbols[caltoclefSym]               = Sym("calto clef",               0xe18a, f1);
      symbols[bassclefSym]                = Sym("bass clef",                0xe18b, f1);
      symbols[cbassclefSym]               = Sym("cbass clef",               0xe18c, f1);
      symbols[trebleclefSym]              = Sym("trebleclef",               0xe18d, f1);   //G-Clef
      symbols[ctrebleclefSym]             = Sym("ctrebleclef",              0xe18e, f1);
      symbols[percussionclefSym]          = Sym("percussion clef",          0xe18f, f1);
      symbols[cpercussionclefSym]         = Sym("cpercussion clef",         0xe190, f1);
      symbols[tabclefSym]                 = Sym("tab clef",                 0xe191, f1);
      symbols[ctabclefSym]                = Sym("ctab clef",                0xe192, f1);

      symbols[fourfourmeterSym]           = Sym("four four meter",          0xe193, f1);
      symbols[allabreveSym]               = Sym("allabreve",                0xe194, f1);
      symbols[pedalasteriskSym]           = Sym("pedalasterisk",            0xe195, f1);
      symbols[pedaldashSym]               = Sym("pedaldash",                0xe196, f1);
      symbols[pedaldotSym]                = Sym("pedaldot",                 0xe197, f1);

      symbols[pedalPSym]                  = Sym("pedalP",                   0xe198, f1);
      symbols[pedaldSym]                  = Sym("pedald",                   0xe199, f1);
      symbols[pedaleSym]                  = Sym("pedale",                   0xe19a, f1);
      symbols[pedalPedSym]                = Sym("pedal ped",                0xe19b, f1);

      symbols[brackettipsUp]              = Sym("bracket ticks up",         0xe19c, f1);
      symbols[brackettipsDown]            = Sym("bracket ticks down",       0xe19d, f1);

      symbols[accDiscantSym]              = Sym("acc discant",              0xe19e, f1);
      symbols[accDotSym]                  = Sym("acc dot",                  0xe19f, f1);
      symbols[accFreebaseSym]             = Sym("acc freebase",             0xe1a0, f1);
      symbols[accStdbaseSym]              = Sym("acc stdbase",              0xe1a1, f1);
      symbols[accBayanbaseSym]            = Sym("acc bayanbase",            0xe1a2, f1);
      symbols[accSBSym]                   = Sym("acc sb",                   0xf0a9, f1);
      symbols[accBBSym]                   = Sym("acc bb",                   0xf0aa, f1);
      symbols[accOldEESym]                = Sym("acc old ee",               0xe1a3, f1);
      symbols[accOldEESSym]               = Sym("acc old ees",              0xf0ac, f1);
      symbols[wholedoheadSym]             = Sym("whole do head",            0xf0ad, f1);
      symbols[halfdoheadSym]              = Sym("half do head",             0xf0ae, f1);
      symbols[doheadSym]                  = Sym("do head",                  0xf0af, f1);

      symbols[wholereheadSym]             = Sym("whole re head",             0xf0b0, f1);
      symbols[halfreheadSym]              = Sym("half re head",              0xf0b1, f1);
      symbols[reheadSym]                  = Sym("re head",                   0xf0b2, f1);
      symbols[wholemeheadSym]             = Sym("whole me head",             0xf0b3, f1);
      symbols[halfmeheadSym]              = Sym("half me head",              0xf0b4, f1);
      symbols[meheadSym]                  = Sym("me head",                   0xf0b5, f1);
      symbols[wholefaheadSym]             = Sym("whole fa head",             0xf0b6, f1);
      symbols[halffauheadSym]             = Sym("half fau head",             0xf0b7, f1);
      symbols[fauheadSym]                 = Sym("fau head",                  0xf0b8, f1);
      symbols[halffadheadSym]             = Sym("half fad head",             0xf0b9, f1);
      symbols[fadheadSym]                 = Sym("fad head",                  0xf0ba, f1);
      symbols[wholelaheadSym]             = Sym("whole la head",             0xf0bb, f1);
      symbols[halflaheadSym]              = Sym("half la head",              0xf0bc, f1);
      symbols[laheadSym]                  = Sym("la head",                   0xf0bd, f1);
      symbols[wholeteheadSym]             = Sym("whole te head",             0xf0be, f1);
      symbols[halfteheadSym]              = Sym("half te head",              0xf0bf, f1);
      symbols[teheadSym]                  = Sym("te head",                   0xf0c0, f1);

      symbols[s_quartheadSym]             = Sym("small quart head",          0xe11d, f3);
      symbols[s_halfheadSym]              = Sym("small half head",           0xe11c, f3);
      symbols[s_wholeheadSym]             = Sym("small whole head",          0xe11b, f3);
      symbols[s_brevisheadSym]            = Sym("small brevis head",         0xe11a, f3);
      symbols[s_dotSym]                   = Sym("small dot",                 0xe119, f3);

      symbols[s_eighthflagSym]            = Sym("small eight flag",          0xe17f, f3);
      symbols[s_sixteenthflagSym]         = Sym("small sixteenth flag",      0xe180, f3);
      symbols[s_thirtysecondflagSym]      = Sym("small thirtysecond flag",   0xe181, f3);
      symbols[s_sixtyfourthflagSym]       = Sym("small sixtyfourth flag",    0xe182, f3);
      symbols[s_deighthflagSym]           = Sym("small deight flag",         0xe183, f3);
      symbols[s_dsixteenthflagSym]        = Sym("small d sixteenth flag",    0xe186, f3);
      symbols[s_dthirtysecondflagSym]     = Sym("small d thirtysecond flag", 0xe187, f3);
      symbols[s_dsixtyfourthflagSym]      = Sym("small d sixtyfourth flag",  0xe188, f3);

      symbols[s_sharpSym]                 = Sym("small sharp",               0xe10e, f3);
      symbols[s_naturalSym]               = Sym("small natural",             0xe111, f3);
      symbols[s_flatSym]                  = Sym("small flat",                0xe112, f3);
      symbols[s_flatflatSym]              = Sym("small flat flat",           0xe114, f3);
      symbols[s_sharpsharpSym]            = Sym("small sharp sharp",         0xe116, f3);
      symbols[flipSym]                    = Sym("flip stem",  0xe0fd, f1);

      // used for GUI:
      symbols[note4Sym]                   = Sym("note 1/4",   0xe0fc, f1);
      symbols[note8Sym]                   = Sym("note 1/8",   0xe0f8, f1);
      symbols[note16Sym]                  = Sym("note 1/16",  0xe0f9, f1);
      symbols[note32Sym]                  = Sym("note 1/32",  0xe0fa, f1);
      symbols[note64Sym]                  = Sym("note 1/64",  0xe0fb, f1);
      symbols[dotdotSym]                  = Sym("dot dot",    0xe0fd, f1);
      }

