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

enum LayoutBreakType {
      LAYOUT_BREAK_PAGE, LAYOUT_BREAK_LINE, LAYOUT_BREAK_SECTION
      };

//---------------------------------------------------------
//   LayoutBreak
//    symbols for line break, page break etc.
//---------------------------------------------------------

class LayoutBreak : public Element {
      LayoutBreakType _subtype;
      qreal lw;
      QPainterPath path;
      qreal _pause;
      bool _startWithLongNames;
      bool _startWithMeasureOne;

      virtual void draw(QPainter*) const;
      void layout0();
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      void* pSubtype() { return  &_subtype; }
      void* pPause()   { return  &_pause;   }

   public:
      LayoutBreak(Score*);
      virtual LayoutBreak* clone() const { return new LayoutBreak(*this); }

      virtual ElementType type() const { return LAYOUT_BREAK; }
      void setSubtype(LayoutBreakType);
      LayoutBreakType subtype() const  { return _subtype; }

      virtual bool acceptDrop(MuseScoreView*, const QPointF&, Element*) const;
      virtual Element* drop(const DropData&);
      virtual void write(Xml&) const;
      virtual void read(const QDomElement&);
      Measure* measure() const            { return (Measure*)parent();   }
      qreal pause() const                 { return _pause;               }
      void setPause(qreal v)              { _pause = v;                  }
      bool startWithLongNames() const     { return _startWithLongNames;  }
      void setStartWithLongNames(bool v)  { _startWithLongNames = v;     }
      bool startWithMeasureOne() const    { return _startWithMeasureOne; }
      void setStartWithMeasureOne(bool v) { _startWithMeasureOne = v;    }

      virtual QVariant getProperty(P_ID propertyId) const;
      virtual bool setProperty(P_ID propertyId, const QVariant&);
      virtual bool setProperty(const QString&, const QDomElement&);
      Property<LayoutBreak>* property(P_ID id) const;
      virtual QVariant propertyDefault(P_ID) const;

      static Property<LayoutBreak> propertyList[];
      };

#endif
