//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __SPACER_H__
#define __SPACER_H__

#include "element.h"

class Painter;

// spacer subtypes
enum { SPACER_UP, SPACER_DOWN };

//---------------------------------------------------------
//   Spacer
//---------------------------------------------------------

class Spacer : public Element {
      PROPERTY(Spatium, gap, Gap)
      QPainterPath path;

   public:
      Spacer(Score*);
      Spacer(const Spacer&);
      virtual Spacer* clone() const    { return new Spacer(*this); }
      virtual ElementType type() const { return SPACER; }

      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
//      void setSpace(const Spatium& sp) { _space = sp;   }
//      Spatium space() const            { return _space; }
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      };

#endif
