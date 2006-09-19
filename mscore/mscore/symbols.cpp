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
#include "symbols.h"
#include "spatium.h"
#include "utils.h"
#include "painter.h"

int Sym::curSn = 0;

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

Sym::Sym(const QString& name, int st, int c, const QPointF& o)
   : _code(c), textStyle(st), _name(name), _offset(o)
      {
      sn        = -1;
      _fontSize = -1;
      }

Sym::Sym() 
      { 
      sn        = -1;
      _code     = 0;
      textStyle = 0;
      _fontSize = -1;
      }

//---------------------------------------------------------
//   font
//    Create symbol font
//---------------------------------------------------------

const QFont Sym::font() const
      {
      QFont _font;
      _font.setFamily(textStyles[textStyle].family);
      int pixelSize = _fontSize == -1 ? textStyles[textStyle].size : _fontSize;
      if (textStyles[textStyle].sizeIsSpatiumDependent)
            pixelSize = lrint(pixelSize * _spatium * .2);
      _font.setPixelSize(pixelSize);
      return _font;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Sym::bbox() const
      {
      QFontMetricsF fm(font());
      return fm.boundingRect(QString(_code)).translated(_offset * _spatium);
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

double Sym::width() const
      {
      QFontMetricsF fm(font());
      return fm.width(_code);
      }

//---------------------------------------------------------
//   height
//---------------------------------------------------------

double Sym::height() const
      {
      return bbox().height();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y) + (_offset * _spatium), QString(_code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter, qreal x, qreal y, int n) const
      {
      painter.setFont(font());
      painter.drawText(QPointF(x, y) + (_offset * _spatium), QString(n, _code));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Sym::draw(Painter& painter) const
      {
      painter.setFont(font());
      painter.drawText(_offset * _spatium, QString(_code));
      }

//---------------------------------------------------------
//   buildin
//---------------------------------------------------------

const Sym& Sym::buildin(const QString& name)
      {
      if (name == "flat")
            return flatSym;
      if (name == "sharp")
            return sharpSym;
      return flatSym;   // DEBUG
      }

#define S1 TEXT_STYLE_SYMBOL1
#define S3 TEXT_STYLE_SYMBOL3

const Sym clefEightSym    ("clef eight", TEXT_STYLE_FINGERING, 0x38);
const Sym clefOneSym      ("clef eight", TEXT_STYLE_FINGERING, 0x31);
const Sym clefFiveSym     ("clef eight", TEXT_STYLE_FINGERING, 0x35);

const Sym stemSym                   ("stem",                     S1, 0xf08f);
const Sym dstemSym                  ("dstem",                    S1, 0xf090);

const Sym plusSym                   ("plus",                     S1, 0x2b);

const Sym zeroSym                   ("zero",                     S1, 0x30);
const Sym oneSym                    ("one",                      S1, 0x31);
const Sym twoSym                    ("two",                      S1, 0x32);
const Sym threeSym                  ("three",                    S1, 0x33);
const Sym fourSym                   ("four",                     S1, 0x34);
const Sym fiveSym                   ("five",                     S1, 0x35);
const Sym sixSym                    ("six",                      S1, 0x36);
const Sym sevenSym                  ("seven",                    S1, 0x37);
const Sym eightSym                  ("eight",                    S1, 0x38);
const Sym nineSym                   ("nine",                     S1, 0x39);

const Sym letterfSym                ("f",                        S1, 0x66);
const Sym lettermSym                ("m",                        S1, 0x6d);
const Sym letterpSym                ("p",                        S1, 0x70);
const Sym letterrSym                ("r",                        S1, 0x72);
const Sym lettersSym                ("s",                        S1, 0x73);
const Sym letterzSym                ("z",                        S1, 0x7a);

const Sym wholerestSym              ("whole rest",               S1, 0xe100, QPointF(.0,-1.) );
const Sym halfrestSym               ("half rest",                S1, 0xe101);
const Sym outsidewholerestSym       ("outside whole rest",       S1, 0xe102);
const Sym outsidehalfrestSym        ("outside half rest",        S1, 0xe103);
const Sym rest_M3                   ("rest M3",                  S1, 0xe104);
const Sym longarestSym              ("longa rest",               S1, 0xe105);
const Sym breverestSym              ("breve rest",               S1, 0xe106);
const Sym quartrestSym              ("quart rest",               S1, 0xe107);
const Sym clasquartrestSym          ("clas quart rest",          S1, 0xe108);
const Sym eighthrestSym             ("eight rest",               S1, 0xe109);
const Sym sixteenthrestSym          ("16' rest",                 S1, 0xe10a);
const Sym thirtysecondrestSym       ("32'  rest",                S1, 0xe10b);
const Sym sixtyfourthrestSym        ("64' rest",                 S1, 0xe10c);
const Sym hundredtwentyeighthrestSym("128' rest",                S1, 0xe10d);

const Sym sharpSym                  ("sharp",                    S1, 0xe10e);
const Sym naturalSym                ("natural",                  S1, 0xe111);
const Sym flatSym                   ("flat",                     S1, 0xe112);
const Sym flatflatSym               ("flat flat",                S1, 0xe114);
const Sym sharpsharpSym             ("sharp sharp",              S1, 0xe116);
const Sym rightparenSym             ("right parenthesis",        S1, 0xe117);
const Sym leftparenSym              ("left parenthesis",         S1, 0xe118);
const Sym dotSym                    ("dot",                      S1, 0xe119);
const Sym brevisheadSym             ("brevis head",              S1, 0xe11a);
const Sym wholeheadSym              ("whole head",               S1, 0xe11b);
const Sym halfheadSym               ("half head",                S1, 0xe11c);
const Sym quartheadSym              ("quart head",               S1, 0xe11d);

const Sym wholediamondheadSym       ("whole diamond head",       S1, 0xe11e);
const Sym halfdiamondheadSym        ("half diamond head",        S1, 0xe11f);
const Sym diamondheadSym            ("diamond head",             S1, 0xe120);
const Sym wholetriangleheadSym      ("whole triangle head",      S1, 0xe121);
const Sym halftriangleheadSym       ("half triangle head",       S1, 0xe122);
const Sym triangleheadSym           ("triangle head",            S1, 0xe124);

const Sym wholeslashheadSym         ("whole slash head",         S1, 0xe126);
const Sym halfslashheadSym          ("half slash head",          S1, 0xe127);
const Sym quartslashheadSym         ("quart slash head",         S1, 0xe128);

const Sym wholecrossedheadSym       ("whole cross head",         S1, 0xe129);
const Sym halfcrossedheadSym        ("half cross head",          S1, 0xe12a);
const Sym crossedheadSym            ("cross head",               S1, 0xe12b);
const Sym xcircledheadSym           ("x circle head",            S1, 0xe12c);

const Sym ufermataSym               ("ufermata",                 S1, 0xe148);
const Sym dfermataSym               ("dfermata",                 S1, 0xe149);
const Sym thumbSym                  ("thumb",                    S1, 0xe150);
const Sym sforzatoaccentSym         ("sforza to accent",         S1, 0xe151);
const Sym esprSym                   ("espressivo",               S1, 0xe152);
const Sym staccatoSym               ("staccato",                 S1, 0xe153);
const Sym ustaccatissimoSym         ("ustaccatissimo",           S1, 0xe154);
const Sym dstaccatissimoSym         ("dstacattissimo",           S1, 0xe155);

const Sym tenutoSym                 ("tenuto",                   S1, 0xe156);
const Sym uportatoSym               ("uportato",                 S1, 0xe157);
const Sym dportatoSym               ("dportato",                 S1, 0xe158);
const Sym umarcatoSym               ("umarcato",                 S1, 0xe159);
const Sym dmarcatoSym               ("dmarcato",                 S1, 0xe15a);
const Sym ouvertSym                 ("ouvert",                   S1, 0xe15b);
const Sym plusstopSym               ("plus stop",                S1, 0xe15c);
const Sym upbowSym                  ("up bow",                   S1, 0xe15d);
const Sym downbowSym                ("down bow",                 S1, 0xe15e);
const Sym reverseturnSym            ("reverse turn",             S1, 0xe15f);
const Sym turnSym                   ("turn",                     S1, 0xe160);
const Sym trillSym                  ("trill",                    S1, 0xe161);
const Sym upedalheelSym             ("upedal heel",              S1, 0xe162);
const Sym dpedalheelSym             ("dpedalheel",               S1, 0xe163);
const Sym upedaltoeSym              ("upedal toe",               S1, 0xe164);
const Sym dpedaltoeSym              ("dpedal toe",               S1, 0xe165);

const Sym flageoletSym              ("flageolet",                S1, 0xe166);
const Sym segnoSym                  ("segno",                    S1, 0xe167);
const Sym codaSym                   ("coda",                     S1, 0xe168);
const Sym varcodaSym                ("varcoda",                  S1, 0xe169);

const Sym rcommaSym                 ("rcomma",                   S1, 0xe16a);
const Sym lcommaSym                 ("lcomma",                   S1, 0xe16b);
const Sym arpeggioSym               ("arpeggio",                 S1, 0xe16e);
const Sym trillelementSym           ("trillelement",             S1, 0xe16f);
const Sym arpeggioarrowdownSym      ("arpeggio arrow down",      S1, 0xe170);
const Sym arpeggioarrowupSym        ("arpeggio arrow up",        S1, 0xe171);
const Sym trilelementSym            ("trill element",            S1, 0xe172);
const Sym prallSym                  ("prall",                    S1, 0xe173);
const Sym mordentSym                ("mordent",                  S1, 0xe174);
const Sym prallprallSym             ("prall prall",              S1, 0xe175);
const Sym prallmordentSym           ("prall mordent",            S1, 0xe176);
const Sym upprallSym                ("up prall",                 S1, 0xe177);
const Sym upmordentSym              ("up mordent",               S1, 0xe178);
const Sym pralldownSym              ("prall down",               S1, 0xe179);
const Sym downprallSym              ("down prall",               S1, 0xe17a);
const Sym downmordentSym            ("down mordent",             S1, 0xe17b);
const Sym prallupSym                ("prall up",                 S1, 0xe17c);
const Sym lineprallSym              ("line prall",               S1, 0xe17d);
const Sym eighthflagSym             ("eight flag",               S1, 0xe17f);
const Sym sixteenthflagSym          ("sixteenth flag",           S1, 0xe180);
const Sym thirtysecondflagSym       ("thirtysecond flag",        S1, 0xe181);
const Sym sixtyfourthflagSym        ("sixtyfour flag",           S1, 0xe182);
const Sym deighthflagSym            ("deight flag",              S1, 0xe183);
const Sym gracedashSym              ("grace dash",               S1, 0xe184);
const Sym dgracedashSym             ("dgrace dash",              S1, 0xe185);
const Sym dsixteenthflagSym         ("dsixteenth flag",          S1, 0xe186);
const Sym dthirtysecondflagSym      ("dthirtysecond flag",       S1, 0xe187);
const Sym dsixtyfourthflagSym       ("dsixtyfourth flag",        S1, 0xe188);
const Sym altoclefSym               ("alto clef",                S1, 0xe189);

const Sym caltoclefSym              ("calto clef",               S1, 0xe18a);
const Sym bassclefSym               ("bass clef",                S1, 0xe18b);
const Sym cbassclefSym              ("cbass clef",               S1, 0xe18c);
const Sym trebleclefSym             ("trebleclef",               S1, 0xe18d);   //G-Clef
const Sym ctrebleclefSym            ("ctrebleclef",              S1, 0xe18e);
const Sym percussionclefSym         ("percussion clef",          S1, 0xe18f);
const Sym cpercussionclefSym        ("cpercussion clef",         S1, 0xe190);
const Sym tabclefSym                ("tab clef",                 S1, 0xe191);
const Sym ctabclefSym               ("ctab clef",                S1, 0xe192);

const Sym fourfourmeterSym          ("four four meter",          S1, 0xe193);
const Sym allabreveSym              ("allabreve",                S1, 0xe194);
const Sym pedalasteriskSym          ("pedalasterisk",            S1, 0xe195);
const Sym pedaldashSym              ("pedaldash",                S1, 0xe196);
const Sym pedaldotSym               ("pedaldot",                 S1, 0xe197);

const Sym pedalPSym                 ("pedalP",                   S1, 0xe198);
const Sym pedaldSym                 ("pedald",                   S1, 0xe199);
const Sym pedaleSym                 ("pedale",                   S1, 0xe19a);
const Sym pedalPedSym               ("pedal ped",                S1, 0xe19b);

const Sym brackettipsUp             ("bracket ticks up",         S1, 0xe19c);
const Sym brackettipsDown           ("bracket ticks down",       S1, 0xe19d);

const Sym accDiscantSym             ("acc discant",              S1, 0xe19e);
const Sym accDotSym                 ("acc dot",                  S1, 0xe19f);
const Sym accFreebaseSym            ("acc freebase",             S1, 0xe1a0);
const Sym accStdbaseSym             ("acc stdbase",              S1, 0xe1a1);
const Sym accBayanbaseSym           ("acc bayanbase",            S1, 0xe1a2);
const Sym accSBSym                  ("acc sb",                   S1, 0xf0a9);
const Sym accBBSym                  ("acc bb",                   S1, 0xf0aa);
const Sym accOldEESym               ("acc old ee",               S1, 0xe1a3);
const Sym accOldEESSym              ("acc old ees",              S1, 0xf0ac);
const Sym wholedoheadSym            ("whole do head",            S1, 0xf0ad);
const Sym halfdoheadSym             ("half do head",             S1, 0xf0ae);
const Sym doheadSym                 ("do head",                  S1, 0xf0af);

const Sym wholereheadSym            ("whole re head",            S1, 0xf0b0);
const Sym halfreheadSym             ("half re head",             S1, 0xf0b1);
const Sym reheadSym                 ("re head",                  S1, 0xf0b2);
const Sym wholemeheadSym            ("whole me head",            S1, 0xf0b3);
const Sym halfmeheadSym             ("half me head",             S1, 0xf0b4);
const Sym meheadSym                 ("me head",                  S1, 0xf0b5);
const Sym wholefaheadSym            ("whole fa head",            S1, 0xf0b6);
const Sym halffauheadSym            ("half fau head",            S1, 0xf0b7);
const Sym fauheadSym                ("fau head",                 S1, 0xf0b8);
const Sym halffadheadSym            ("half fad head",            S1, 0xf0b9);
const Sym fadheadSym                ("fad head",                 S1, 0xf0ba);
const Sym wholelaheadSym            ("whole la head",            S1, 0xf0bb);
const Sym halflaheadSym             ("half la head",             S1, 0xf0bc);
const Sym laheadSym                 ("la head",                  S1, 0xf0bd);
const Sym wholeteheadSym            ("whole te head",            S1, 0xf0be);
const Sym halfteheadSym             ("half te head",             S1, 0xf0bf);
const Sym teheadSym                 ("te head",                  S1, 0xf0c0);

//----------small symbols

const Sym s_quartheadSym            ("small quart head",          S3, 0xe11d);
const Sym s_halfheadSym             ("small half head",           S3, 0xe11c);
const Sym s_wholeheadSym            ("small whole head",          S3, 0xe11b);
const Sym s_brevisheadSym           ("small brevis head",         S3, 0xe11a);
const Sym s_dotSym                  ("small dot",                 S3, 0xe119);

const Sym s_eighthflagSym           ("small eight flag",          S3, 0xe17f);
const Sym s_sixteenthflagSym        ("small sixteenth flag",      S3, 0xe180);
const Sym s_thirtysecondflagSym     ("small thirtysecond flag",   S3, 0xe181);
const Sym s_sixtyfourthflagSym      ("small sixtyfourth flag",    S3, 0xe182);
const Sym s_deighthflagSym          ("small deight flag",         S3, 0xe183);
const Sym s_dsixteenthflagSym       ("small d sixteenth flag",    S3, 0xe186);
const Sym s_dthirtysecondflagSym    ("small d thirtysecond flag", S3, 0xe187);
const Sym s_dsixtyfourthflagSym     ("small d sixtyfourth flag",  S3, 0xe188);

const Sym s_sharpSym                ("small sharp",               S3, 0xe10e);
const Sym s_naturalSym              ("small natural",             S3, 0xe111);
const Sym s_flatSym                 ("small flat",                S3, 0xe112);
const Sym s_flatflatSym             ("small flat flat",           S3, 0xe114);
const Sym s_sharpsharpSym           ("small sharp sharp",         S3, 0xe116);

