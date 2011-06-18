//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __HAIRPIN_H__

#include "line.h"
#include "mscore.h"

class Score;
class Hairpin;
class Painter;

//---------------------------------------------------------
//   HairpinSegment
//---------------------------------------------------------

class HairpinSegment : public LineSegment {
      QLineF l1, l2;

   protected:
   public:
      HairpinSegment(Score* s) : LineSegment(s) {}
      Hairpin* hairpin() const              { return (Hairpin*)parent(); }
      virtual HairpinSegment* clone() const { return new HairpinSegment(*this); }
      virtual ElementType type() const      { return HAIRPIN_SEGMENT; }
      virtual void draw(Painter*) const;
      virtual void layout();
      };

//---------------------------------------------------------
//   Hairpin
//
//    subtype: 0 = crescendo,  1 = decrescendo
//---------------------------------------------------------

class Hairpin : public SLine {
      int _veloChange;
      DynamicType _dynType;

   public:
      Hairpin(Score* s);
      virtual Hairpin* clone() const   { return new Hairpin(*this); }
      virtual ElementType type() const { return HAIRPIN; }
      Segment* segment() const         { return (Segment*)parent(); }
      virtual void layout();
      virtual LineSegment* createLineSegment();
      int veloChange() const           { return _veloChange; }
      void setVeloChange(int v)        { _veloChange = v;    }
      DynamicType dynType() const      { return _dynType; }
      void setDynType(DynamicType t)   { _dynType = t;    }
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      };

#define __HAIRPIN_H__

#endif

