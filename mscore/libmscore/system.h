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

#ifndef __SYSTEM_H__
#define __SYSTEM_H__

/**
 \file
 Definition of classes SysStaff, System and SystemList.
*/

#include "element.h"
#include "spatium.h"

class Staff;
class StaffLines;
class Clef;
class Page;
class Bracket;
class Lyrics;
class Segment;
class MeasureBase;
class Text;
class InstrumentName;
class SpannerSegment;

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

/**
 One staff of a System.
*/

class SysStaff {
      QRectF _bbox;           ///< Bbox of StaffLines.
      Spatium _distanceUp;    ///< distance to previous staff
      Spatium _distanceDown;  ///< distance to next staff
      bool _show;             ///< derived from Staff or false if empty
                              ///< staff is hidden
   public:
      int idx;
      QList<Bracket*> brackets;
      QList<InstrumentName*> instrumentNames;

      const QRectF& bbox() const     { return _bbox; }
      QRectF& rbb()                  { return _bbox; }
      qreal y() const               { return _bbox.y(); }
      qreal right() const           { return _bbox.right(); }
      void setbbox(const QRectF& r)  { _bbox = r; }
      void move(qreal x, qreal y);

      Spatium distanceUp() const      { return _distanceUp;   }
      void setDistanceUp(Spatium v)   { _distanceUp = v;      }
      Spatium distanceDown() const    { return _distanceDown; }
      void setDistanceDown(Spatium v) { _distanceDown = v;    }

      bool show() const               { return _show; }
      void setShow(bool v)            { _show = v; }

      SysStaff();
      ~SysStaff();
      };

//---------------------------------------------------------
//   System
//---------------------------------------------------------

/**
 One row of measures for all instruments;
 a complete piece of the timeline.
*/

class System : public Element {
      QList<MeasureBase*> ml;
      QList<SysStaff*> _staves;
      Line* barLine;          ///< Left hand bar, connects staves in system.
      qreal _leftMargin;      ///< left margin for instrument name, brackets etc.
      bool _pageBreak;
      bool _firstSystem;      ///< used to decide between long and short instrument
                              ///< names; set by score()->doLayout()
      bool _vbox;             ///< contains only one VBox in ml

      QList<SpannerSegment*> _spannerSegments;

      void setDistanceUp(int n, Spatium v)   { _staves[n]->setDistanceUp(v); }
      void setDistanceDown(int n, Spatium v) { _staves[n]->setDistanceDown(v); }

   public:
      System(Score*);
      ~System();
      virtual System* clone() const    { return new System(*this); }
      virtual ElementType type() const { return SYSTEM; }

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      Page* page() const                 { return (Page*)parent(); }

      virtual void layout(qreal xoffset);
      void layout2();         ///< Called after Measure layout.
      void clear();                       ///< Clear measure list.

      QList<MeasureBase*>& measures()        { return ml; }

      QRectF bboxStaff(int staff) const      { return _staves[staff]->bbox(); }
      QList<SysStaff*>* staves()             { return &_staves;   }
      const QList<SysStaff*>* staves() const { return &_staves;   }
      qreal staffY(int staffIdx) const;
      SysStaff* staff(int staffIdx) const    { return _staves[staffIdx]; }

      Spatium distanceUp(int idx) const      { return _staves[idx]->distanceUp(); }
      Spatium distanceDown(int idx) const    { return _staves[idx]->distanceDown(); }
      bool pageBreak() const                 { return _pageBreak; }
      void setPageBreak(bool val)            { _pageBreak = val; }

      SysStaff* insertStaff(int);
      void removeStaff(int);

      Line* getBarLine() const             { return barLine; }
      int y2staff(qreal y) const;
      void setInstrumentNames(bool longName);
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      Measure* firstMeasure() const;
      Measure* lastMeasure() const;

      MeasureBase* prevMeasure(const MeasureBase*) const;
      MeasureBase* nextMeasure(const MeasureBase*) const;

      qreal leftMargin() const    { return _leftMargin; }
      void setFirstSystem(bool v) { _firstSystem = v;   }
      bool isVbox() const         { return _vbox;       }
      void setVbox(bool v)        { _vbox = v;          }
      void layoutLyrics(Lyrics*, Segment*, int staffIdx);
      };

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;

#endif

