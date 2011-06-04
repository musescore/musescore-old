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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

class Trill;
class Accidental;
class Painter;

//---------------------------------------------------------
//   TrillSegment
//---------------------------------------------------------

class TrillSegment : public LineSegment {
   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s) {}
      Trill* trill() const                { return (Trill*)parent(); }
      virtual ElementType type() const    { return TRILL_SEGMENT; }
      virtual TrillSegment* clone() const { return new TrillSegment(*this); }
      virtual void draw(Painter*) const;
      virtual void layout();
      };

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

class Trill : public SLine {
      Accidental* _accidental;

   public:
      Trill(Score* s);
      virtual Trill* clone() const     { return new Trill(*this); }
      virtual ElementType type() const { return TRILL; }

      virtual void layout();
      virtual LineSegment* createLineSegment();
      virtual void add(Element*);
      virtual void remove(Element*);
      Accidental* accidental() const    { return _accidental; }
      void setAccidental(Accidental* a) { _accidental = a; }
      virtual void read(XmlReader*);

      Segment* segment() const          { return (Segment*)parent(); }
      };

#endif

