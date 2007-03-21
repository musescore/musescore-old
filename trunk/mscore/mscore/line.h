//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: line.h,v 1.2 2006/03/02 17:08:34 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "element.h"

class SLine;

//---------------------------------------------------------
//   LineSegment
//    Virtual base class for OttavaSegment, PedalSegment
//    HairpinSegment and TrillSegment.
//    This class describes part of a line object. Line objects
//    can span multiple staves. For every staff an Segment
//    is created.
//---------------------------------------------------------

class LineSegment : public Element {
   public:
      enum SegmentType {
            SEGMENT_SINGLE, SEGMENT_BEGIN, SEGMENT_MIDDLE, SEGMENT_END
            };

   protected:
      QPointF _p2;
      QPointF _userOff2;
      SegmentType _segmentType;

      QRectF r1, r2, bbr1, bbr2;     // "grips" for dragging

      enum { NORMAL, DRAG1, DRAG2 };
      int mode;

      virtual bool isMovable() const { return true; }
      virtual void endDrag();
      virtual bool startEdit(QMatrix&, const QPointF&);
      virtual bool startEditDrag(Viewer*, const QPointF&);
      virtual bool editDrag(Viewer*, QPointF*, const QPointF&);
      virtual bool edit(QKeyEvent*);
      virtual bool endEditDrag();
      virtual void endEdit();

   public:
      LineSegment(Score* s);
      virtual void draw(QPainter& p);
      SLine* line() const                 { return (SLine*)parent(); }
      const QPointF& userOff2() const     { return _userOff2;  }
      void setUserOff2(const QPointF& o)  { _userOff2 = o;     }
      void setPos2(const QPointF& p)      { _p2 = p; }
      QPointF pos2() const                { return _p2 + _userOff2 * _spatium; }
      QPointF canvasPos2() const          { return _p2 + _userOff2 * _spatium + canvasPos(); }
      void setSegmentType(SegmentType s)  { _segmentType = s;  }
      };

//---------------------------------------------------------
//   SLine
//    virtual base class for Ottava, Pedal, Hairpin and
//    Trill
//---------------------------------------------------------

class SLine : public Element {
   protected:
      QList<LineSegment*> segments;
      int _tick2;

   public:
      SLine(Score* s);
      virtual void draw(QPainter& p);
      void setTick2(int t);
      int tick2() const    { return _tick2; }
      virtual void layout(ScoreLayout*);
      bool readProperties(QDomNode node);
      void writeProperties(Xml& xml) const;
      virtual LineSegment* createSegment() = 0;
      void setLen(double l);
      void collectElements(QList<Element*>& el);
      void setOff(const QPointF&);
      virtual void add(Element*);
      virtual void remove(Element*);
      };

typedef QList<LineSegment*>::iterator iLineSegment;
typedef QList<LineSegment*>::const_iterator ciLineSegment;

#endif

