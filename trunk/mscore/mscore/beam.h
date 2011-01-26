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
#include "spanner.h"

class ChordRest;
class ScoreView;
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
      int _up;                // -1: unknown  0: down   1: up
      double _grow1;          // define "feather" beams
      double _grow2;

      QList<BeamFragment*> fragments;     // beam splits across systems

      bool _userModified[2];    // 0: auto/down  1: up

      mutable int _id;          // used in read()/write()

      int minMove;              // set in layout1()
      int maxMove;
      Chord* c1;
      Chord* c2;
      bool isGrace;
      bool cross;
      Duration maxDuration;
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

      virtual bool isEditable() const { return true; }
      virtual void startEdit(ScoreView*, const QPointF&);
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
      virtual void draw(Painter*) const;
      int up() const                      { return _up; }
      void setUp(int v)                   { _up = v; }
      void setId(int i) const             { _id = i; }
      int id() const                      { return _id; }
      bool isUp() const                   { return _up; }

      void setBeamDirection(Direction d);
      virtual QPainterPath shape() const;
      virtual bool contains(const QPointF& p) const;
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);

      double grow1() const      { return _grow1; }
      double grow2() const      { return _grow2; }
      void setGrow1(double val) { _grow1 = val; }
      void setGrow2(double val) { _grow2 = val; }
      };

extern bool endBeam(const Fraction&, ChordRest* cr, int p);
#endif

