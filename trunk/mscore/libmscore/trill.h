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

#ifndef __TRILL_H__
#define __TRILL_H__

#include "line.h"

class Trill;
class Accidental;
class QPainter;

//---------------------------------------------------------
//   TrillSegment
//---------------------------------------------------------

class TrillSegment : public LineSegment {
   protected:
   public:
      TrillSegment(Score* s) : LineSegment(s) {}
      Trill* trill() const                { return (Trill*)spanner(); }
      virtual ElementType type() const    { return TRILL_SEGMENT; }
      virtual TrillSegment* clone() const { return new TrillSegment(*this); }
      virtual void draw(QPainter*) const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
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
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      Segment* segment() const          { return (Segment*)parent(); }
      };

#endif

