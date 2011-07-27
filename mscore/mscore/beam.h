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

#ifndef __BEAM_H__
#define __BEAM_H__

#include "element.h"
#include "durationtype.h"

class ChordRest;
class ScoreView;
class Chord;

//---------------------------------------------------------
//   BeamHint
//    beam hint for autobeamer
//
//    used for "start beam hints" list and "end beam hints"
//    list
//---------------------------------------------------------

struct BeamHint {
      Fraction noteLen;
      Fraction timeSig;     // valid for this timesig; zero = valid for all
      Fraction pos;

      BeamHint(int a, int b, int c, int d, int e, int f)
         : noteLen(a, b), timeSig(c, d), pos(e, f) {}
      };

//---------------------------------------------------------
//   BeamSegment
//---------------------------------------------------------

struct BeamSegment {
      QPointF p1, p2;

      BeamSegment() {}
      BeamSegment(const QPointF& a, const QPointF& b) : p1(a), p2(b) {}
      void move(double x, double y) {
            QPointF m(x, y);
            p1 += m;
            p2 += m;
            }
      };

typedef QList<BeamSegment*> BeamSegmentList;
typedef BeamSegmentList::iterator iBeamSegment;
typedef BeamSegmentList::const_iterator ciBeamSegment;

//---------------------------------------------------------
//   Beam
//    Balken
//---------------------------------------------------------

class Beam : public Element {
      QList<ChordRest*> _elements;
      BeamSegmentList beamSegments;
      Direction _direction;
      int _up;                  // -1: unknown  0: down   1: up

      bool _userModified[2];    // 0: auto/down  1: up
      QPointF _p1[2], _p2[2];
      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      Chord* c1;
      Chord* c2;
      bool isGrace;
      bool cross;
      Duration maxDuration;
      qreal slope;

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const         { return new Beam(*this); }
      virtual ElementType type() const    { return BEAM; }
      virtual QPointF canvasPos() const;  ///< position in canvas coordinates

      virtual bool isMovable() const                  { return false; }
      virtual bool isEditable() { return true; }
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      virtual void toDefault();

      void layout1();
      void layout();

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      void add(ChordRest* a);
      void remove(ChordRest* a);
      void writeMusicXml(Xml& xml, ChordRest* cr) const;
      virtual void move(double, double);
      virtual void draw(QPainter&) const;
      int up() const                      { return _up; }
      void setUp(int v)                   { _up = v; }
      void setId(int i) const             { _id = i; }
      int id() const                      { return _id; }

      void setBeamDirection(Direction d);
      bool isUp();
      };

extern bool endBeam(const Fraction&, ChordRest* cr, int p);
#endif

