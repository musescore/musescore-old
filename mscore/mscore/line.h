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
//---------------------------------------------------------

struct LineSegment {
      QPointF p1;
      QPointF p2;
      QRectF bbox;
      };

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

class SLine : public Element {
   protected:
      QList<LineSegment> segments;

      int _tick2;
      QPointF off1, off2;   // user offset (in spatium units)
      QRectF r1, r2;        // "grips" for p1, p2
      QRectF bbr1, bbr2;    // bounding boxes for grips

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
      virtual ElementType type() const { return LINE; }
      void setTick2(int t);
      int tick2() const    { return _tick2; }
      virtual void layout(ScoreLayout*);
      bool readProperties(QDomNode node);
      void writeProperties(Xml& xml) const;
      };

typedef QList<LineSegment>::iterator iLineSegment;
typedef QList<LineSegment>::const_iterator ciLineSegment;

#endif

