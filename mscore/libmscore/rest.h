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

#ifndef __REST_H__
#define __REST_H__

#include "chordrest.h"

class Duration;

//---------------------------------------------------------
//    Rest
///   This class implements a rest. Todo: detailed
///   description of Rest class.
///   More text.
//---------------------------------------------------------

class Rest : public ChordRest {
      Q_DECLARE_TR_FUNCTIONS(Rest)

      // values calculated by layout:
      int _sym;
      int dotline;            // depends on rest symbol
      qreal _mmWidth;         // width of multi measure rest

      virtual QRectF drag(const QPointF& s);
      virtual qreal upPos()   const;
      virtual qreal downPos() const;
      virtual qreal centerX() const;
      void setUserOffset(qreal x, qreal y);

   public:
      Rest(Score* s = 0);
      Rest(Score*, const Duration&);
      virtual Rest* clone() const      { return new Rest(*this); }
      virtual ElementType type() const { return REST; }

      virtual void draw(Painter*) const;
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement, const QList<Tuplet*>&, QList<Slur*>*);
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void layout();

      void setMMWidth(qreal val);
      qreal mmWidth() const        { return _mmWidth; }
      static int getSymbol(Duration::DurationType type, int line, int lines,  int* yoffset);

      int getDotline() const { return dotline; }
      int sym() const        { return _sym;    }
      };

#endif

