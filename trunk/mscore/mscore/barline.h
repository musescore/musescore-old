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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"

class ScoreView;
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
      qreal yoff;       // used during drag edit to extend y2

      void getY(double*, double*) const;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void draw(QPainter&, ScoreView*) const;
      virtual Space space() const;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void layout();

      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      void setSpan(int val)    { _span = val;  }
      int span() const         { return _span; }
      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)parent()->parent(); }

      virtual bool isEditable() const { return true; }
      virtual void endEdit();
      virtual void editDrag(int, const QPointF&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      int tick() const;
      };

#endif

