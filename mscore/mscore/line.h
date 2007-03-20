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

//---------------------------------------------------------
//   LineSegment
//    Virtual base class for OttavaSegment, PedalSegment
//    HairpinSegment and TrillSegment.
//    This class describes part of a line object. Line objects
//    can span multiple staves. For every staff an Segment
//    is created.
//---------------------------------------------------------

class LineSegment : public Element {
   protected:
      QPointF p2;
      QPointF _userOff2;

   public:
      LineSegment(Score* s) : Element(s) {}
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

      int mode;
      enum { NORMAL, DRAG1, DRAG2 };

      virtual QPointF  dragOff() const;
      virtual bool contains(const QPointF& p) const;
      virtual bool isMovable() const { return true; }
      virtual void endDrag();
      virtual bool startEdit(QMatrix&, const QPointF&);
      virtual bool startEditDrag(Viewer*, const QPointF&);
      virtual bool endEditDrag();
      virtual bool editDrag(Viewer*, QPointF*, const QPointF&);
      virtual void endEdit();
      virtual bool edit(QKeyEvent*);

   public:
      SLine(Score* s);
      void setTick2(int t);
      int tick2() const    { return _tick2; }
      virtual void layout(ScoreLayout*);
      bool readProperties(QDomNode node);
      void writeProperties(Xml& xml) const;
      virtual LineSegment* createSegment() = 0;
      };

typedef QList<LineSegment*>::iterator iLineSegment;
typedef QList<LineSegment*>::const_iterator ciLineSegment;

#endif

