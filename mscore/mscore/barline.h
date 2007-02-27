//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: barline.h,v 1.2 2006/03/28 14:58:58 wschweer Exp $
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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"

enum BarType {
      NORMAL_BAR, DOUBLE_BAR, START_REPEAT, END_REPEAT,
      BROKEN_BAR, END_BAR, INVISIBLE_BAR
      };

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

class BarLine : public Element {
      qreal _height;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      virtual void draw(QPainter&);
      virtual void dump() const;
      virtual void setSubtype(int t);
      virtual QRectF bbox() const;

      virtual bool acceptDrop(const QPointF&, int, const QDomNode&) const;
      virtual void drop(const QPointF&, int, const QDomNode&);
      void setHeight(qreal v) { _height = v; }
      };

#endif

