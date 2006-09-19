//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: symbols.h,v 1.16 2006/04/06 13:03:11 wschweer Exp $
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

#ifndef __SYMBOLS_H__
#define __SYMBOLS_H__

class Painter;

//---------------------------------------------------------
//   Sym
//---------------------------------------------------------

struct Sym {
      static int curSn;
      mutable int sn;

      QChar _code;
      int textStyle;
      int _fontSize;
      
      QString _name;
      QPointF _offset;  // drawing offset in spatium units

   public:
      Sym();
      Sym(const QString& name, int st, int c, const QPointF& _o = QPointF());

      const QFont font() const;
      const QString& name() const { return _name; }
      double width() const;
      double height() const;
      void draw(Painter& painter, qreal x, qreal y, int n) const;
      void draw(Painter& painter, qreal x, qreal y) const;
      void draw(Painter& painter) const;
      QRectF bbox() const;
      QChar code() const         { return _code; }
      QPointF offset() const     { return _offset; }
      void setOffset(QPointF& p) { _offset = p;  }
      int size() const           { return _fontSize; }
      void setSize(int n)        { _fontSize = n;    }
      static const Sym& buildin(const QString& name);
      };

extern const Sym clefEightSym;
extern const Sym clefOneSym;
extern const Sym clefFiveSym;
extern const Sym wholerestSym;
extern const Sym halfrestSym;
extern const Sym outsidewholerestSym;
extern const Sym outsidehalfrestSym;
extern const Sym longarestSym;
extern const Sym breverestSym;
extern const Sym quartrestSym;
extern const Sym eighthrestSym;
extern const Sym clasquartrestSym;
extern const Sym sixteenthrestSym;
extern const Sym thirtysecondrestSym;
extern const Sym sixtyfourthrestSym;
extern const Sym hundredtwentyeighthrestSym;
extern const Sym neomensmaximarestSym;
extern const Sym neomenslongarestSym;
extern const Sym neomensbreverestSym;
extern const Sym zeroSym;
extern const Sym oneSym;
extern const Sym twoSym;
extern const Sym threeSym;
extern const Sym fourSym;
extern const Sym fiveSym;
extern const Sym sixSym;
extern const Sym sevenSym;
extern const Sym eightSym;
extern const Sym nineSym;
extern const Sym neomenssemibrevisrestSym;
extern const Sym neomensminimahalfrestSym;
extern const Sym neomenssemiminimarestSym;
extern const Sym neomensfusarestSym;
extern const Sym neomenssemifusarestSym;
extern const Sym sharpSym;
extern const Sym naturalSym;
extern const Sym flatSym;
extern const Sym flatflatSym;
extern const Sym sharpsharpSym;
extern const Sym rightparenSym;
extern const Sym leftparenSym;
extern const Sym dotSym;
extern const Sym brevisheadSym;
extern const Sym wholeheadSym;
extern const Sym halfheadSym;
extern const Sym quartheadSym;
extern const Sym wholediamondheadSym;
extern const Sym halfdiamondheadSym;
extern const Sym diamondheadSym;
extern const Sym wholetriangleheadSym;
extern const Sym halftriangleheadSym;
extern const Sym triangleheadSym;
extern const Sym wholeslashheadSym;
extern const Sym halfslashheadSym;
extern const Sym quartslashheadSym;
extern const Sym wholecrossedheadSym;
extern const Sym halfcrossedheadSym;
extern const Sym crossedheadSym;
extern const Sym xcircledheadSym;
extern const Sym ufermataSym;
extern const Sym dfermataSym;
extern const Sym thumbSym;
extern const Sym sforzatoaccentSym;
extern const Sym esprSym;
extern const Sym staccatoSym;
extern const Sym ustaccatissimoSym;
extern const Sym dstaccatissimoSym;
extern const Sym tenutoSym;
extern const Sym uportatoSym;
extern const Sym dportatoSym;
extern const Sym umarcatoSym;
extern const Sym dmarcatoSym;
extern const Sym ouvertSym;
extern const Sym plusstopSym;
extern const Sym upbowSym;
extern const Sym downbowSym;
extern const Sym reverseturnSym;
extern const Sym turnSym;
extern const Sym trillSym;
extern const Sym upedalheelSym;
extern const Sym dpedalheelSym;
extern const Sym upedaltoeSym;
extern const Sym dpedaltoeSym;
extern const Sym flageoletSym;
extern const Sym segnoSym;
extern const Sym codaSym;
extern const Sym rcommaSym;
extern const Sym lcommaSym;
extern const Sym arpeggioSym;
extern const Sym trillelementSym;
extern const Sym arpeggioarrowdownSym;
extern const Sym arpeggioarrowupSym;
extern const Sym trilelementSym;
extern const Sym prallSym;
extern const Sym mordentSym;
extern const Sym prallprallSym;
extern const Sym prallmordentSym;
extern const Sym upprallSym;
extern const Sym downprallSym;
extern const Sym upmordentSym;
extern const Sym downmordentSym;
extern const Sym lineprallSym;
extern const Sym pralldownSym;
extern const Sym prallupSym;
extern const Sym eighthflagSym;
extern const Sym sixteenthflagSym;
extern const Sym thirtysecondflagSym;
extern const Sym sixtyfourthflagSym;
extern const Sym deighthflagSym;
extern const Sym gracedashSym;
extern const Sym dgracedashSym;
extern const Sym dsixteenthflagSym;
extern const Sym dthirtysecondflagSym;
extern const Sym dsixtyfourthflagSym;
extern const Sym stemSym;
extern const Sym dstemSym;
extern const Sym altoclefSym;
extern const Sym caltoclefSym;
extern const Sym bassclefSym;
extern const Sym cbassclefSym;
extern const Sym trebleclefSym;
extern const Sym ctrebleclefSym;
extern const Sym percussionclefSym;
extern const Sym cpercussionclefSym;
extern const Sym tabclefSym;
extern const Sym ctabclefSym;
extern const Sym fourfourmeterSym;
extern const Sym allabreveSym;
extern const Sym pedalasteriskSym;
extern const Sym pedaldashSym;
extern const Sym pedaldotSym;
extern const Sym pedalPSym;
extern const Sym pedaldSym;
extern const Sym pedaleSym;
extern const Sym pedalPedSym;
extern const Sym accDiscantSym;
extern const Sym accDotSym;
extern const Sym accFreebaseSym;
extern const Sym accStdbaseSym;
extern const Sym accBayanbaseSym;
extern const Sym accSBSym;
extern const Sym accBBSym;
extern const Sym accOldEESym;
extern const Sym accOldEESSym;
extern const Sym wholedoheadSym;
extern const Sym halfdoheadSym;
extern const Sym doheadSym;
extern const Sym wholereheadSym;
extern const Sym halfreheadSym;
extern const Sym reheadSym;
extern const Sym wholemeheadSym;
extern const Sym halfmeheadSym;
extern const Sym meheadSym;
extern const Sym wholefaheadSym;
extern const Sym halffauheadSym;
extern const Sym fauheadSym;
extern const Sym halffadheadSym;
extern const Sym fadheadSym;
extern const Sym wholelaheadSym;
extern const Sym halflaheadSym;
extern const Sym laheadSym;
extern const Sym wholeteheadSym;
extern const Sym halfteheadSym;
extern const Sym letterfSym;
extern const Sym lettermSym;
extern const Sym letterpSym;
extern const Sym letterrSym;
extern const Sym lettersSym;
extern const Sym letterzSym;

extern const Sym s_quartheadSym;
extern const Sym s_halfheadSym;
extern const Sym s_wholeheadSym;
extern const Sym s_brevisheadSym;
extern const Sym s_dotSym;
extern const Sym s_eighthflagSym;
extern const Sym s_sixteenthflagSym;
extern const Sym s_thirtysecondflagSym;
extern const Sym s_sixtyfourthflagSym;
extern const Sym s_deighthflagSym;
extern const Sym s_dsixteenthflagSym;
extern const Sym s_dthirtysecondflagSym;
extern const Sym s_dsixtyfourthflagSym;

extern const Sym s_sharpSym;
extern const Sym s_naturalSym;
extern const Sym s_flatSym;
extern const Sym s_flatflatSym;
extern const Sym s_sharpsharpSym;

extern const Sym plusSym;
#endif

