//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

class Segment;
class Painter;

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

class BarLine : public Element {
      int _span;
      qreal yoff;       // used during drag edit to extend y2

      void getY(qreal*, qreal*) const;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void read(XmlReader*);
      virtual void draw(Painter*) const;
      virtual Space space() const;
      virtual QPointF canvasPos() const;      ///< position in canvas coordinates
      virtual void layout();

      void setSpan(int val)    { _span = val;  }
      int span() const         { return _span; }
      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)parent()->parent(); }

      int tick() const;

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      void setBarLineType(BarLineType i)  { Element::setSubtype(int(i));    }
      BarLineType barLineType() const     { return BarLineType(subtype());  }
      static BarLineType barLineType(const QString& s);
      };

#endif

