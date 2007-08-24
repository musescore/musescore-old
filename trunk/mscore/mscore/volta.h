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

#include "element.h"

class Score;
class Xml;

//---------------------------------------------------------
//   Volta
//    brackets
//---------------------------------------------------------

enum {
      PRIMA_VOLTA = 1, SECONDA_VOLTA, TERZA_VOLTA, SECONDA_VOLTA2
      };

class Volta : public Element {
      QPointF _p1, _p2;

   public:
      Volta(Score* s) : Element(s) {}
      virtual Volta* clone() const { return new Volta(*this); }
      virtual ElementType type() const { return VOLTA; }
      virtual void draw(QPainter&);
      virtual void layout(ScoreLayout*);
      void setLen(qreal);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual QRectF bbox() const;
      virtual bool isMovable() const { return true; }
      };

#endif

