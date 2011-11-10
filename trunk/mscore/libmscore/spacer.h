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

class QPainter;

// spacer subtypes
enum { SPACER_UP, SPACER_DOWN };

//---------------------------------------------------------
//   Spacer
//---------------------------------------------------------

class Spacer : public Element {
//      PROPERTY(qreal, gap, Gap)
      qreal _gap;

      QPainterPath path;

      void layout0();

   public:
      Spacer(Score*);
      Spacer(const Spacer&);
      virtual Spacer* clone() const    { return new Spacer(*this); }
      virtual ElementType type() const { return SPACER; }

      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      virtual void draw(QPainter*) const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void setSubtype(int val);
      virtual void spatiumChanged(qreal, qreal);
      void setGap(qreal sp);
      qreal gap() const     { return _gap; }
      };

#endif
