//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __BEND_H__
#define __BEND_H__

#include "element.h"
#include "pitchvalue.h"

class Painter;

//---------------------------------------------------------
//   Bend
//---------------------------------------------------------

class Bend : public Element {
      QList<PitchValue> _points;
      double _lw;
      QPointF notePos;
      double noteWidth;

   public:
      Bend(Score* s);
      virtual Bend* clone() const { return new Bend(*this); }
      virtual ElementType type() const { return BEND; }
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual void write(Xml&) const;
      virtual void read(QDomElement e);
      QList<PitchValue>& points()                { return _points; }
      const QList<PitchValue>& points() const    { return _points; }
      void setPoints(const QList<PitchValue>& p) { _points = p;    }
      };

#endif

