//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __LINE_H__
#define __LINE_H__

#include "spanner.h"
#include "globals.h"

class SLine;
class System;
class ScoreView;

//---------------------------------------------------------
//   LineSegment
//    Virtual base class for segmented lines segments
//    (OttavaSegment, PedalSegment, HairpinSegment, TrillSegment...)
//
//    This class describes one segment of an segmented
//    line object. Line objects can span multiple staves.
//    For every staff a segment is created.
//---------------------------------------------------------

class LineSegment : public Element {
   protected:
      QPointF _p2;
      QPointF _userOff2;            // depends on spatium
      QRectF r1, r2;
      LineSegmentType _segmentType;
      System* _system;

      virtual bool isEditable() { return true; }
      virtual void editDrag(int, const QPointF&);
      virtual bool edit(ScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString& s);
      virtual void updateGrips(int*, QRectF*) const;
      virtual QPointF gripAnchor(int) const;
      virtual void layout() {}

   public:
      LineSegment(Score* s);
      LineSegment(const LineSegment&);
      virtual LineSegment* clone() const = 0;
      virtual void draw(QPainter& p, ScoreView*) const = 0;
      SLine* line() const                         { return (SLine*)parent(); }
      const QPointF& userOff2() const             { return _userOff2;       }
      void setUserOff2(const QPointF& o)          { _userOff2 = o;          }
      void setUserXoffset2(qreal x)               { _userOff2.setX(x);      }
      void setPos2(const QPointF& p)              { _p2 = p;                }
      QPointF pos2() const                        { return _p2 + _userOff2; }
      void setSegmentType(LineSegmentType s)      { _segmentType = s;       }
      LineSegmentType segmentType() const         { return _segmentType;    }
      void setSystem(System* s)                   { _system = s;            }
      virtual void toDefault();
      virtual void spatiumChanged(double, double);
      virtual QPointF canvasPos() const;

      friend class SLine;
      };

//---------------------------------------------------------
//   SLine
//    virtual base class for Ottava, Pedal, Hairpin,
//    Trill and TextLine
//---------------------------------------------------------

class SLine : public Spanner {
   protected:
      QList<LineSegment*> segments;
      bool _diagonal;

   public:
      SLine(Score* s);
      SLine(const SLine&);

      virtual void layout();
      bool readProperties(QDomElement node);
      void writeProperties(Xml& xml, const SLine* proto = 0) const;
      virtual LineSegment* createLineSegment() = 0;
      void setLen(double l);
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);
      virtual QRectF bbox() const;
      const QList<LineSegment*>& lineSegments() const { return segments; }
      QList<LineSegment*>& lineSegments() { return segments; }

      virtual QPointF tick2pos(int grip, System** system);

      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      bool diagonal() const         { return _diagonal; }
      void setDiagonal(bool v)      { _diagonal = v;    }
      int tick() const;
      int tick2() const;
      };

typedef QList<LineSegment*>::iterator iLineSegment;
typedef QList<LineSegment*>::const_iterator ciLineSegment;

#endif

