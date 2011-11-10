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

#ifndef __LAYOUTBREAK_H__
#define __LAYOUTBREAK_H__

#include "element.h"

class QPainter;

// layout break subtypes:

enum { LAYOUT_BREAK_PAGE, LAYOUT_BREAK_LINE, LAYOUT_BREAK_SECTION };

//---------------------------------------------------------
//   LayoutBreak
//    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak : public Element {
      qreal lw;
      QPainterPath path;
      qreal _pause;
      bool _startWithLongNames;
      bool _startWithMeasureOne;

      virtual void draw(QPainter*) const;
      void layout0();
      virtual void spatiumChanged(qreal /*oldValue*/, qreal /*newValue*/);

   public:
      LayoutBreak(Score*);
      virtual LayoutBreak* clone() const { return new LayoutBreak(*this); }
      virtual ElementType type() const { return LAYOUT_BREAK; }
      virtual void setSubtype(const QString&);
      virtual void setSubtype(int st);
      virtual const QString subtypeName() const;
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);
      Measure* measure() const            { return (Measure*)parent();   }
      qreal pause() const                 { return _pause;               }
      void setPause(qreal v)              { _pause = v;                  }
      bool startWithLongNames() const     { return _startWithLongNames;  }
      void setStartWithLongNames(bool v)  { _startWithLongNames = v;     }
      bool startWithMeasureOne() const    { return _startWithMeasureOne; }
      void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v;    }
      };

#endif
