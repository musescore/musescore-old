//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: barline.h,v 1.2 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"

class Viewer;
class Segment;

enum BarType {
      NORMAL_BAR, DOUBLE_BAR, START_REPEAT, END_REPEAT,
      BROKEN_BAR, END_BAR, END_START_REPEAT
      };

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

class BarLine : public Element {
      int _span;
      qreal yoff;

      double getY2() const;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void draw(QPainter&) const;
      virtual QRectF bbox() const;
      virtual void space(double& min, double& extra) const;

      virtual bool acceptDrop(Viewer*, const QPointF&, int, int) const;
      virtual Element* drop(const QPointF&, const QPointF&, Element*);
      void setSpan(int val)    { _span = val;  }
      int span() const         { return _span; }
      Segment* segment() const { return (Segment*)parent(); }

      virtual bool startEdit(const QPointF&);
      virtual void endEdit();
      virtual void editDrag(int, const QPointF&, const QPointF&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      };

#endif

