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
      QList<ChordRest*> elements;
      BeamSegmentList beamSegments;

   public:
      Beam(Score* s) : Element(s) {}
      ~Beam();
      virtual Beam* clone() const      { return new Beam(*this); }
      virtual ElementType type() const { return BEAM; }
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates

      Measure* measure() const { return (Measure*)parent(); }

      void layout1(ScoreLayout*);
      void layout(ScoreLayout*);

      void add(ChordRest* a)           { elements.append(a); }
      QList<ChordRest*> getElements()  { return elements; }
      void remove(ChordRest* a);
      QString xmlType(ChordRest*) const;
      virtual void move(double, double);
      virtual QRectF bbox() const;

      virtual void draw(QPainter&) const;
      };

#endif

