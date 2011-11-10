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

#ifndef __BREATH_H__
#define __BREATH_H__

#include "element.h"

class QPainter;

//---------------------------------------------------------
//   Breath
//    subtype() is index in symList
//---------------------------------------------------------

class Breath : public Element {
      static const int breathSymbols = 4;
      static int symList[breathSymbols];

      Segment* segment() const         { return (Segment*)parent(); }

   public:
      Breath(Score* s);
      virtual Breath* clone() const { return new Breath(*this); }
      virtual ElementType type() const { return BREATH; }
      virtual Space space() const;

      virtual void draw(QPainter*) const;
      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual QPointF pagePos() const;      ///< position in page coordinates
      };

#endif

