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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "element.h"

class Painter;

// layout break subtypes:

enum { LAYOUT_BREAK_PAGE, LAYOUT_BREAK_LINE, LAYOUT_BREAK_SECTION };

//---------------------------------------------------------
//   LayoutBreak
//    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak : public Element {
      qreal lw;
//      QPainterPath path;
      qreal _pause;

      virtual void draw(Painter*) const;
      virtual void layout();

   public:
      LayoutBreak(Score*);
      virtual LayoutBreak* clone() const { return new LayoutBreak(*this); }
      virtual ElementType type() const { return LAYOUT_BREAK; }
      virtual void setSubtype(const QString&);
      virtual void setSubtype(int st) { Element::setSubtype(st); }
      virtual const QString subtypeName() const;
      virtual void read(XmlReader*);
      qreal pause() const    { return _pause; }
      void setPause(qreal v) { _pause = v; }
      };

#endif
