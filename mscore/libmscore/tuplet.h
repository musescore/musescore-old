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

#ifndef __TUPLET_H__
#define __TUPLET_H__

#include <QtCore/QPointF>
#include "duration.h"

class Text;
class Slur;
class Painter;

//------------------------------------------------------------------------
//   Tuplet
//     Example of 1/8 triplet:
//       _baseLen     = 1/8
//       _actualNotes = 3
//       _normalNotes = 2     (3 notes played in the time of 2/8)
//
//    the tuplet has a len of _baseLen * _normalNotes
//    a tuplet note has len of _baseLen * _normalNotes / _actualNotes
//------------------------------------------------------------------------

class Tuplet : public DurationElement {
      int _tick;

   public:
      enum { SHOW_NUMBER, SHOW_RELATION, NO_TEXT };
      enum { AUTO_BRACKET, SHOW_BRACKET, SHOW_NO_BRACKET };

   private:
      QList<DurationElement*> _elements;
      int _numberType;
      int _bracketType;
      bool _hasBracket;

      Fraction _ratio;
      TimeDuration _baseLen;      // 1/8 for a triplet of 1/8

      Direction _direction;
      bool _isUp;

      bool _userModified;
      QPointF p1, p2;
      QPointF _p1, _p2;
      mutable int _id;        // used during read/write

      Text* _number;
      QPointF bracketL[4];
      QPointF bracketR[3];

      virtual void setSelected(bool f);

   public:
      Tuplet(Score*);
      ~Tuplet();
      virtual Tuplet* clone() const    { return new Tuplet(*this); }
      virtual ElementType type() const { return TUPLET; }
      virtual QRectF bbox() const;
      virtual void setTrack(int val);

      virtual void add(Element*);
      virtual void remove(Element*);

      virtual Measure* measure() const { return (Measure*)parent(); }

      int numberType() const        { return _numberType;       }
      int bracketType() const       { return _bracketType;      }
      void setNumberType(int val)   { _numberType = val;        }
      void setBracketType(int val)  { _bracketType = val;       }
      bool hasBracket() const       { return _hasBracket;       }

      Fraction ratio() const        { return _ratio;            }
      void setRatio(const Fraction& r) { _ratio = r;            }

      const QList<DurationElement*>& elements() const { return _elements; }
      int lastTick() const          { return _elements.last()->tick(); }
      void clear()                  { _elements.clear(); }

      virtual void layout();
      Text* number() const { return _number; }

      void read(XmlReader*, const QList<Tuplet*>&, const QList<Slur*>&);

      virtual void draw(Painter*) const;
      int id() const                       { return _id;          }
      void setId(int i) const              { _id = i;             }

      TimeDuration baseLen() const             { return _baseLen;     }
      void setBaseLen(const TimeDuration& d)   { _baseLen = d;        }

      void setDirection(Direction d)       { _direction = d; }
      Direction direction() const          { return _direction; }
      bool isUp() const                    { return _isUp; }
      virtual int tick() const             { return _tick; }
      void setTick(int val)                { _tick = val; }
      };

#endif

