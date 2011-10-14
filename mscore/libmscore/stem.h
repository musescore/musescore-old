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

#ifndef __STEM_H__
#define __STEM_H__

#include "element.h"

class Chord;
class Painter;

//---------------------------------------------------------
//   Stem
//    Notenhals
//---------------------------------------------------------

/**
 Graphic representation of a note stem.
*/

class Stem : public Element {
      qreal _len;
      Spatium _userLen;

   public:
      Stem(Score*);
      Stem &operator=(const Stem&);

      virtual Stem* clone() const      { return new Stem(*this); }
      virtual ElementType type() const { return STEM; }
      virtual void draw(Painter*) const;
      void setLen(qreal v)            { _len = v; }
      qreal stemLen() const           { return _len + point(_userLen); }
      virtual bool isEditable() const { return true; }
      virtual void layout();

      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement e);
      virtual void toDefault();
      Spatium userLen() const         { return _userLen; }
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      Chord* chord() const            { return (Chord*)parent(); }
      };

#endif

