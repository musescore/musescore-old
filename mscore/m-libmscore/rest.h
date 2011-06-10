//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"

class TimeDuration;
class Painter;

//---------------------------------------------------------
//    Rest
///   This class implements a rest. Todo: detailed
///   description of Rest class.
///   More text.
//---------------------------------------------------------

class Rest : public ChordRest {
      int _sym;

      int dotline;            // depends on rest symbol
      qreal _mmWidth;        // width of multi measure rest
      qreal _yoffset;        // in spatium units

      virtual QRectF drag(const QPointF& s);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void setUserOffset(qreal x, qreal y);

   public:
      Rest(Score* s = 0);
      Rest(Score*, const TimeDuration&);
      virtual Rest* clone() const      { return new Rest(*this); }
      virtual ElementType type() const { return REST; }

      virtual void draw(Painter*) const;
      virtual void read(XmlReader*, const QList<Tuplet*>&, const QList<Slur*>&);
      virtual void scanElements(void* data, void (*func)(void*, Element*));

      virtual void layout();
      virtual QRectF bbox() const;

      void setMMWidth(qreal val)   { _mmWidth = val; }
      qreal mmWidth() const        { return _mmWidth; }
      static int getSymbol(TimeDuration::DurationType type, int line, int* yoffset);

      void setYoff(qreal d) { _yoffset = d;        }
      qreal yoff() const    { return _yoffset;     }
      };

#endif

