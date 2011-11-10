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

#ifndef __STAFFSTATE_H__
#define __STAFFSTATE_H__

#include "element.h"
#include "elementlayout.h"
#include "instrument.h"

class QPainter;

enum {
      STAFF_STATE_INSTRUMENT, STAFF_STATE_TYPE,
      STAFF_STATE_VISIBLE, STAFF_STATE_INVISIBLE
      };

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

class StaffState : public Element {
      qreal lw;
      QPainterPath path;

      Instrument _instrument;

      virtual void draw(QPainter*) const;
      virtual void layout();

   public:
      StaffState(Score*);
      virtual StaffState* clone() const { return new StaffState(*this); }
      virtual ElementType type() const   { return STAFF_STATE; }
      virtual void setSubtype(const QString&);
      virtual void setSubtype(int st)    { Element::setSubtype(st); }
      virtual const QString subtypeName() const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      Instrument instrument() const           { return _instrument; }
      void setInstrument(const Instrument& i) { _instrument = i;    }
      Segment* segment()                      { return (Segment*)parent(); }
      };

#endif
