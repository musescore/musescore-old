//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h,v 1.58 2006/04/12 14:58:10 wschweer Exp $
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

#ifndef __VOLTA_H__
#define __VOLTA_H__

#include "line.h"

class Score;
class Xml;
class Volta;

//---------------------------------------------------------
//   VoltaSegment
//---------------------------------------------------------

class VoltaSegment : public LineSegment {
   protected:
   public:
      VoltaSegment(Score* s) : LineSegment(s) {}
      Volta* volta() const                { return (Volta*)parent(); }
      virtual ElementType type() const    { return VOLTA_SEGMENT; }
      virtual VoltaSegment* clone() const { return new VoltaSegment(*this); }
      virtual void draw(QPainter&);
      virtual QRectF bbox() const;
      virtual QPointF gripAnchor(int) const;
      QPointF pos2anchor(const QPointF& pos, int* tick) const;
      };

//---------------------------------------------------------
//   Volta
//    brackets
//---------------------------------------------------------

enum {
      PRIMA_VOLTA = 1, SECONDA_VOLTA, TERZA_VOLTA, SECONDA_VOLTA2
      };

class Volta : public SLine {
      QList<int> _endings;

   public:
      Volta(Score* s) : SLine(s) {}
      virtual Volta* clone() const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual void layout(ScoreLayout*);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual LineSegment* createLineSegment();
      virtual QPointF tick2pos(int tick, System** system);
      QList<int> endings() const { return _endings; }
      virtual void setSubtype(int val);
      };

#endif

