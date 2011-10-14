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

#ifndef __ARPEGGIO_H__
#define __ARPEGGIO_H__

#include "element.h"

class Chord;
class Painter;

// Arpeggio types
enum { ARP_NORMAL, ARP_UP, ARP_DOWN, ARP_BRACKET};

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

class Arpeggio : public Element {
      Spatium _userLen1;
      Spatium _userLen2;
      qreal _height;
      int _span;              // spanning staves

      virtual QLineF dragAnchor() const;
      virtual QPointF gripAnchor(int) const;

   public:
      Arpeggio(Score* s);
      virtual Arpeggio* clone() const  { return new Arpeggio(*this); }
      virtual ElementType type() const { return ARPEGGIO; }
      Chord* chord() const             { return (Chord*)parent(); }
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual bool isEditable() const { return true; }
      virtual void editDrag(const EditData&);
      virtual void updateGrips(int*, QRectF*) const;
      virtual bool edit(MuseScoreView*, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&);

      void read(QDomElement e);
      void write(Xml& xml) const;
      int span() const      { return _span; }
      void setSpan(int val) { _span = val; }

      void setHeight(qreal);
      };

#endif

