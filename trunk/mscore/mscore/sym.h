//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sym.h,v 1.16 2006/04/06 13:03:11 wschweer Exp $
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

#ifndef __SYM_H__
#define __SYM_H__

extern void initSymbols();

//---------------------------------------------------------
//   SymCode
//---------------------------------------------------------

struct SymCode {
      QChar code;
      int style;
      SymCode(QChar c, int s) : code(c), style(s) {}
      };

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

class Sym {
      QChar _code;
      int fontId;
      const char* _name;
      mutable QRectF _bbox;   // cached bounding box at mag X 10 at mag 1.0
      mutable qreal  _width;  // cached width at mag X 10 at mag 1.0

   public:
      Sym() { _code = 0; }
      Sym(const char* name, const QChar& c, int fid);

      QFont font(double mag = 1.0) const;
      const char* name() const { return _name; }
      const QRectF bbox() const;
      double width() const;
      double height() const;
      QChar code() const           { return _code;   }
      int getFontId() const        { return fontId;  }
      void setCode(const QChar& c) { _code = c;      }
      void draw(QPainter& painter, qreal x, qreal y, int n) const;
      void draw(QPainter& painter, qreal x, qreal y) const;
      void draw(QPainter& painter) const;
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
      neomensmaximarestSym,
      neomenslongarestSym,
      neomensbreverestSym,
      rest_M3,
      varcodaSym,
      brackettipsUp,
      brackettipsDown,
      teheadSym,

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
      neomenssemibrevisrestSym,
      neomensminimahalfrestSym,
      neomenssemiminimarestSym,
      neomensfusarestSym,
      neomenssemifusarestSym,
      sharpSym,
      naturalSym,
      flatSym,
      flatflatSym,
      sharpsharpSym,
      rightparenSym,
      leftparenSym,
      dotSym,
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
      deighthflagSym,
      gracedashSym,
      dgracedashSym,
      dsixteenthflagSym,
      dthirtysecondflagSym,
      dsixtyfourthflagSym,
      stemSym,
      dstemSym,
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
      accSBSym,
      accBBSym,
      accOldEESym,
      accOldEESSym,
      wholedoheadSym,
      halfdoheadSym,
      doheadSym,
      wholereheadSym,
      halfreheadSym,
      reheadSym,
      wholemeheadSym,
      halfmeheadSym,
      meheadSym,
      wholefaheadSym,
      halffauheadSym,
      fauheadSym,
      halffadheadSym,
      fadheadSym,
      wholelaheadSym,
      halflaheadSym,
      laheadSym,
      wholeteheadSym,
      halfteheadSym,

      letterfSym,
      lettermSym,
      letterpSym,
      letterrSym,
      lettersSym,
      letterzSym,

      s_quartheadSym,
      s_halfheadSym,
      s_wholeheadSym,
      s_brevisheadSym,
      s_dotSym,
      s_eighthflagSym,
      s_sixteenthflagSym,
      s_thirtysecondflagSym,
      s_sixtyfourthflagSym,
      s_deighthflagSym,
      s_dsixteenthflagSym,
      s_dthirtysecondflagSym,
      s_dsixtyfourthflagSym,

      s_sharpSym,
      s_naturalSym,
      s_flatSym,
      s_flatflatSym,
      s_sharpsharpSym,

      plusSym,
      flipSym,
      note2Sym,
      note4Sym,
      note8Sym,
      note16Sym,
      note32Sym,
      note64Sym,
      dotdotSym,

      s_wholediamondheadSym,
      s_halfdiamondheadSym,
      s_diamondheadSym,
      s_wholetriangleheadSym,
      s_halftriangleheadSym,
      s_triangleheadSym,
      s_wholeslashheadSym,
      s_halfslashheadSym,
      s_quartslashheadSym,
      s_wholecrossedheadSym,
      s_halfcrossedheadSym,
      s_crossedheadSym,

      lastSym
      };

extern const Sym* findSymbol(QChar code, int fontId);
#endif

