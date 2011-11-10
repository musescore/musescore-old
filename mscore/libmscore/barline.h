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

#ifndef __BARLINE_H__
#define __BARLINE_H__

#include "element.h"

class MuseScoreView;
class Segment;
class QPainter;

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

class BarLine : public Element {
      int _span;
      qreal yoff;       // used during drag edit to extend y2

      void getY(qreal*, qreal*) const;
      ElementList _el;        ///< fermata or other articulations

      void drawDots(QPainter* painter, qreal x) const;

   public:
      BarLine(Score*);
      BarLine &operator=(const BarLine&);

      virtual BarLine* clone() const   { return new BarLine(*this); }
      virtual ElementType type() const { return BAR_LINE; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual void draw(QPainter*) const;
      virtual Space space() const;
      virtual QPointF pagePos() const;      ///< position in canvas coordinates
      virtual void layout();
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      virtual void add(Element*);
      virtual void remove(Element*);
      virtual QPainterPath shape() const;

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      void setSpan(int val)    { _span = val;  }
      int span() const         { return _span; }
      Segment* segment() const { return (Segment*)parent(); }
      Measure* measure() const { return (Measure*)(parent() ? parent()->parent() : 0); }

      virtual bool isEditable() const { return true; }
      virtual void endEdit();
      virtual void editDrag(const EditData&);
      virtual void endEditDrag();
      virtual void updateGrips(int*, QRectF*) const;
      int tick() const;

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      void setBarLineType(BarLineType i)  { Element::setSubtype(int(i));    }
      BarLineType barLineType() const     { return BarLineType(subtype());  }

      static BarLineType barLineType(const QString& s);

      ElementList* el()                { return &_el; }
      const ElementList* el() const    { return &_el; }
      };

#endif

