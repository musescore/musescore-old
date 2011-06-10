//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#ifndef __SYM_H__
#define __SYM_H__

#include "style.h"

extern void initSymbols(int);

#if QT_VERSION >= 0x040800
#define USE_GLYPHS  1            // does not work for printing
#endif

enum SymbolType {
      SYMBOL_UNKNOWN,
      SYMBOL_COPYRIGHT,
      SYMBOL_FRACTION
      };

//---------------------------------------------------------
//   SymCode
//---------------------------------------------------------

struct SymCode {
      int code;
      int fontId;
      const char* text;
      SymbolType type;
      bool show;
      SymCode(int c, int id, const char* t = 0, SymbolType type = SYMBOL_UNKNOWN, bool show = true)
         : code(c), fontId(id), text(t), type(type), show(show) {}
      };

extern QMap<const char*, SymCode*> charReplaceMap;

extern SymCode pSymbols[];

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

class Sym {
      int _code;
      int fontId;
      const char* _name;
      QFont _font;
      double w;
      QRectF _bbox;
      QPointF _attach;
#ifdef USE_GLYPHS
      QGlyphRun glyphs;
      void genGlyphs();
#endif

   public:
      Sym() { _code = 0; }
      Sym(const char* name, int c, int fid, double x=0.0, double y=0.0);
      Sym(const char* name, int c, int fid, const QPointF&, const QRectF&);

      const char* name() const             { return _name; }
      void setName(const char* s)          { _name = s; }
      const QRectF bbox(double mag) const;
      double height(double mag) const      { return _bbox.height() * mag; }
      double width(double mag) const       { return w * mag;  }
      QPointF attach(double mag) const     { return _attach * mag;   }
      int code() const                     { return _code;    }
      int getFontId() const                { return fontId;   }
      int setFontId(int v)                 { return fontId = v;   }
      QFont font() const                   { return _font;    }
      void draw(QPainter& painter, double mag, qreal x, qreal y, int n) const;
      void draw(QPainter& painter, double mag, qreal x = 0.0, qreal y = 0.0) const;
      void setAttach(const QPointF& r)       { _attach = r; }
      bool isValid() const                   { return _code != 0; }
      QRectF getBbox() const    { return _bbox; }
      QPointF getAttach() const { return _attach; }
      QString toString() const;
      };

enum {
      clefEightSym,
      clefOneSym,
      clefFiveSym,

      wholerestSym,
      halfrestSym,
      outsidewholerestSym,
      outsidehalfrestSym,
      longarestSym,
      breverestSym,
      rest4Sym,
      rest8Sym,
      clasquartrestSym,
      rest16Sym,
      rest32Sym,
      rest64Sym,
      rest128Sym,
      rest_M3,

      varcodaSym,
      brackettipsRightUp,
      brackettipsRightDown,
      brackettipsLeftUp,
      brackettipsLeftDown,

      zeroSym,
      oneSym,
      twoSym,
      threeSym,
      fourSym,
      fiveSym,
      sixSym,
      sevenSym,
      eightSym,
      nineSym,

      sharpSym,
      sharpArrowUpSym,
      sharpArrowDownSym,
      sharpArrowBothSym,
      sharpslashSym,
      sharpslash2Sym,
      sharpslash3Sym,
      sharpslash4Sym,

      naturalSym,
      naturalArrowUpSym,
      naturalArrowDownSym,
      naturalArrowBothSym,
      flatSym,
      flatArrowUpSym,
      flatArrowDownSym,
      flatArrowBothSym,

      flatslashSym,
      flatslash2Sym,
      mirroredflat2Sym,
      mirroredflatSym,
      mirroredflatslashSym,
      flatflatSym,
      flatflatslashSym,
      sharpsharpSym,

      rightparenSym,
      leftparenSym,
      dotSym,

      longaupSym,
      longadownSym,
      brevisheadSym,
      brevisdoubleheadSym,
      wholeheadSym,
      halfheadSym,
      quartheadSym,
      wholediamondheadSym,
      halfdiamondheadSym,
      diamondheadSym,
      s0triangleHeadSym,
      d1triangleHeadSym,
      u1triangleHeadSym,
      u2triangleHeadSym,
      d2triangleHeadSym,
      wholeslashheadSym,
      halfslashheadSym,
      quartslashheadSym,
      wholecrossedheadSym,
      halfcrossedheadSym,
      crossedheadSym,
      xcircledheadSym,
      s0doHeadSym,
      d1doHeadSym,
      u1doHeadSym,
      d2doHeadSym,
      u2doHeadSym,
      s0reHeadSym,
      u1reHeadSym,
      d1reHeadSym,
      u2reHeadSym,
      d2reHeadSym,
      s0miHeadSym,
      s1miHeadSym,
      s2miHeadSym,
      u0faHeadSym,
      d0faHeadSym,
      u1faHeadSym,
      d1faHeadSym,
      u2faHeadSym,
      d2faHeadSym,
      s0laHeadSym,
      s1laHeadSym,
      s2laHeadSym,
      s0tiHeadSym,
      u1tiHeadSym,
      d1tiHeadSym,
      u2tiHeadSym,
      d2tiHeadSym,

      s0solHeadSym,
      s1solHeadSym,
      s2solHeadSym,

      ufermataSym,
      dfermataSym,

      snappizzicatoSym,
      thumbSym,
      ushortfermataSym,
      dshortfermataSym,
      ulongfermataSym,
      dlongfermataSym,
      uverylongfermataSym,
      dverylongfermataSym,

      sforzatoaccentSym,
      esprSym,
      staccatoSym,
      ustaccatissimoSym,
      dstaccatissimoSym,
      tenutoSym,
      uportatoSym,
      dportatoSym,
      umarcatoSym,
      dmarcatoSym,
      ouvertSym,
      halfopenSym,
      plusstopSym,
      upbowSym,
      downbowSym,
      reverseturnSym,
      turnSym,
      trillSym,
      upedalheelSym,
      dpedalheelSym,

      upedaltoeSym,
      dpedaltoeSym,
      flageoletSym,
      segnoSym,
      varsegnoSym,
      codaSym,

      rcommaSym,
      lcommaSym,

      arpeggioSym,
      trillelementSym,
      arpeggioarrowdownSym,
      arpeggioarrowupSym,
      trilelementSym,
      prallSym,
      mordentSym,
      prallprallSym,
      prallmordentSym,
      upprallSym,

      downprallSym,
      upmordentSym,
      downmordentSym,
      lineprallSym,
      pralldownSym,
      prallupSym,
      schleiferSym,

      caesuraCurvedSym,
      caesuraStraight,

      eighthflagSym,
      sixteenthflagSym,
      thirtysecondflagSym,
      sixtyfourthflagSym,
      flag128Sym,
      deighthflagSym,
      gracedashSym,
      dgracedashSym,
      dsixteenthflagSym,
      dthirtysecondflagSym,
      dsixtyfourthflagSym,
      dflag128Sym,
      altoclefSym,
      caltoclefSym,
      bassclefSym,
      cbassclefSym,
      trebleclefSym,
      ctrebleclefSym,
      percussionclefSym,
      cpercussionclefSym,
      tabclefSym,
      ctabclefSym,
      fourfourmeterSym,
      allabreveSym,
      pedalasteriskSym,
      pedaldashSym,
      pedaldotSym,
      pedalPSym,
      pedaldSym,
      pedaleSym,
      pedalPedSym,
      accDiscantSym,
      accDotSym,
      accFreebaseSym,
      accStdbaseSym,
      accBayanbaseSym,
      accOldEESym,
      accpushSym,
      accpullSym,

      letterfSym,
      lettermSym,
      letterpSym,
      letterrSym,
      lettersSym,
      letterzSym,
      letterTSym,
      letterSSym,
      letterPSym,

      plusSym,
      note2Sym,
      note4Sym,
      note8Sym,
      note16Sym,
      note32Sym,
      note64Sym,
      dotdotSym,

      wholediamond2headSym,
      halfdiamond2headSym,
      diamond2headSym,

      sqlongaupSym,
      sqlongadownSym,
      sqbrevisheadSym,

      tempimpprolmajSym,
      tempperfprolminSym,
      tempperfcutSym,
      tempperfprolmajSym,

      tabclef2Sym,


      lastSym
      };

extern QVector<Sym> symbols[2];

extern QString symToHtml(const Sym&, int leftMargin=0, const TextStyle* ts = 0, double sp=10.0);
extern QString symToHtml(const Sym&, const Sym&, int leftMargin=0);
extern QFont fontId2font(int id);
#endif

