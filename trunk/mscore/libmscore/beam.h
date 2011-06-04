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

#ifndef __BEAM_H__
#define __BEAM_H__

#include "element.h"
#include "durationtype.h"
#include "spanner.h"

class ChordRest;
class Chord;
class Painter;

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

//
// user offsets for beam or beam fragment
//
struct BeamFragment {
      QPointF p1[2];
      QPointF p2[2];
      };

//---------------------------------------------------------
//   Beam
//    Balken
//---------------------------------------------------------

class Beam : public Element {
      QList<ChordRest*> _elements;
      QList<QLineF*> beamSegments;
      Direction _direction;
      int _up;                  // -1: unknown  0: down   1: up
      qreal _grow1;             // define "feather" beams
      qreal _grow2;

      QList<BeamFragment*> fragments;

      bool _userModified[2];    // 0: auto/down  1: up

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      Chord* c1;
      Chord* c2;
      bool isGrace;
      bool cross;
      TimeDuration maxDuration;
      qreal slope;
      int cut;

      int editFragment;       // valid in edit mode

      void layout2(QList<ChordRest*>, SpannerSegmentType, int frag);

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const         { return new Beam(*this); }
      virtual ElementType type() const    { return BEAM; }
      virtual QPointF canvasPos() const;  ///< position in canvas coordinates

      virtual void read(XmlReader*);

      void layout1();
      void layout();

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      void add(ChordRest* a);
      void remove(ChordRest* a);
      virtual void move(qreal, qreal);
      virtual void draw(Painter*) const;
      int up() const                      { return _up; }
      void setUp(int v)                   { _up = v; }
      void setId(int i) const             { _id = i; }
      int id() const                      { return _id; }
      bool isUp() const                   { return _up; }

      void setBeamDirection(Direction d);
//      virtual QPainterPath shape() const;
      virtual bool contains(const QPointF& p) const;

      qreal grow1() const      { return _grow1; }
      qreal grow2() const      { return _grow2; }
      void setGrow1(qreal val) { _grow1 = val; }
      void setGrow2(qreal val) { _grow2 = val; }
      };

extern bool endBeam(const Fraction&, ChordRest* cr, int p);
#endif

