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

#ifndef __BRACKET_H__
#define __BRACKET_H__

#include "element.h"
#include "painter.h"

class System;
class Painter;

// System Brackets
enum { BRACKET_NORMAL, BRACKET_AKKOLADE, NO_BRACKET = -1};

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

class Bracket : public Element {
      qreal h2;

      int _column, _span;

      PainterPath path;
      qreal yoff;

   public:
      Bracket(Score*);
      virtual Bracket* clone() const { return new Bracket(*this); }
      virtual ElementType type() const { return BRACKET; }

      int span() const       { return _span; }
      void setSpan(int val)  { _span = val; }
      int level() const      { return _column; }
      void setLevel(int v)   { _column = v; }
      System* system() const { return (System*)parent(); }

      virtual void setHeight(qreal);
      virtual qreal width() const;

      virtual void draw(Painter*) const;
      virtual void read(XmlReader*);
      virtual void layout();
      };

#endif

