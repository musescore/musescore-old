//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: beam.h,v 1.6 2006/03/02 17:08:32 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "chordlist.h"
#include "element.h"

class Viewer;
class Chord;

//---------------------------------------------------------
//   BeamHint
//    beam hint for autobeamer
//
//    used for "start beam hints" list and "end beam hints"
//    list
//---------------------------------------------------------

struct BeamHint {
      int noteLenZ;
      int noteLenN;

      int timeSigZ;     // valid for this timesig; zero = valid for all
      int timeSigN;

      int posZ;
      int posN;

      BeamHint(int a, int b, int c, int d, int e, int f) {
            noteLenZ = a;
            noteLenN = b;
            timeSigZ = c;
            timeSigN = d;
            posZ     = e;
            posN     = f;
            }
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
      bool _up;

      bool _userModified;
      QPointF _p1, _p2;
      mutable int _id;          // used in read()/write()

      void layoutCrossStaff(int, int, Chord*, Chord*);

   public:
      Beam(Score* s);
      Beam(const Beam&);
      ~Beam();
      virtual Beam* clone() const         { return new Beam(*this); }
      virtual ElementType type() const    { return BEAM; }
      virtual QPointF canvasPos() const;  ///< position in canvas coordinates

      virtual bool isMovable() const                  { return false; }
      virtual bool startEdit(Viewer*, const QPointF&) { return true; }
      virtual void editDrag(int, const QPointF&);
      virtual void updateGrips(int*, QRectF*) const;

      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);

      virtual void resetUserOffsets();

      Measure* measure() const            { return (Measure*)parent(); }

      void layout1(ScoreLayout*);
      void layout(ScoreLayout*);

      const QList<ChordRest*>& elements() { return _elements;  }
      void clear()                        { _elements.clear(); }
      void add(ChordRest* a);
      void remove(ChordRest* a);
      QString xmlType(ChordRest*) const;
      virtual void move(double, double);
      virtual QRectF bbox() const;
      virtual void draw(QPainter&) const;
      bool up() const                     { return _up; }
      void setUp(bool v)                  { _up = v; }
      void setId(int i) const             { _id = i; }
      int id() const                      { return _id; }

      void setBeamDirection(Direction d)  { _direction = d; }
      bool isUp();
      };

#endif

