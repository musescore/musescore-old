//=============================================================================
//  MusE Score
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

#ifndef __SYM_H__
#define __SYM_H__

#include "style.h"

extern void initSymbols();

enum SymbolType {
      SYMBOL_UNKNOWN,
      SYMBOL_COPYRIGHT,
      SYMBOL_FRACTION
      };

//---------------------------------------------------------
//   SymCode
//---------------------------------------------------------

struct SymCode {
      QChar code;
      int fontId;
      const char* text;
      SymbolType type;
      bool show;
      SymCode(QChar c, int id, const char* t = 0, SymbolType type = SYMBOL_UNKNOWN, bool show = true)
         : code(c), fontId(id), text(t), type(type), show(show) {}
      };

extern QMap<const char*, SymCode*> charReplaceMap;

extern SymCode pSymbols[];

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

class Sym {
      QChar _code;
      int fontId;
      const char* _name;
      QFont _font;
      double w;
      QRectF _bbox;

   public:
      Sym() { _code = 0; }
      Sym(const char* name, const QChar& c, int fid);

      const char* name() const             { return _name; }
      const QRectF bbox(double mag) const;
      double height(double mag) const      { return _bbox.height() * mag; }

      double width(double mag) const       { return w * mag;  }
      QChar code() const                   { return _code;    }
      int getFontId() const                { return fontId;   }
      QFont font() const                   { return _font;   }
      void setCode(const QChar& c)         { _code = c;       }
      void draw(QPainter& painter, double mag, qreal x, qreal y, int n) const;
      void draw(QPainter& painter, double mag, qreal x, qreal y) const;
      void draw(QPainter& painter) const;
      void draw(QPainter& painter, double mag) const;
      static void writeCtable();
      };

extern QVector<Sym> symbols;

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
      quartrestSym,
      eighthrestSym,
      clasquartrestSym,
      sixteenthrestSym,
      thirtysecondrestSym,
      sixtyfourthrestSym,
      hundredtwentyeighthrestSym,

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
      wholeheadSym,
      halfheadSym,
      quartheadSym,
      wholediamondheadSym,
      halfdiamondheadSym,
      diamondheadSym,
      wholetriangleheadSym,
      halftriangleheadSym,
      triangleheadSym,
      wholeslashheadSym,
      halfslashheadSym,
      quartslashheadSym,
      wholecrossedheadSym,
      halfcrossedheadSym,
      crossedheadSym,
      xcircledheadSym,
      ufermataSym,
      dfermataSym,
      thumbSym,
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

      letterfSym,
      lettermSym,
      letterpSym,
      letterrSym,
      lettersSym,
      letterzSym,

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

      lastSym
      };

extern const Sym* findSymbol(QChar code, int fontId);
extern QString symToHtml(const Sym&, int leftMargin=0);
extern QString symToHtml(const Sym&, const Sym&, int leftMargin=0);
extern QFont fontId2font(int id);
#endif

