//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
      Hairpin* hairpin() const              { return (Hairpin*)spanner(); }
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

