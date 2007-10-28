//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: system.h,v 1.23 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
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
class BarLine;
class Clef;
class Page;
class Text;
class Bracket;
class Lyrics;
class Segment;
class ScoreLayout;

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

/**
 One staff of a System.
*/

class SysStaff {
      QRectF _bbox;           ///< Bbox of StaffLines.
      Spatium _distance;      ///< distance to next staff

   public:
      int idx;
      QList<Bracket*> brackets;
      Text* instrumentName;

      const QRectF& bbox() const      { return _bbox; }
      void setbbox(const QRectF& r)   { _bbox = r; }
      void move(double x, double y);

      double distance() const        { return point(_distance); }
      void setDistance(double v)     { _distance.set(v); }
      void setDistance(Spatium v)    { _distance = v; }

      SysStaff();
      ~SysStaff();
      };

typedef QList<SysStaff*> SysStaffList;
typedef SysStaffList::iterator iSysStaff;
typedef SysStaffList::const_iterator ciSysStaff;

//---------------------------------------------------------
//   System
//---------------------------------------------------------

/**
 One row of measures for all instruments;
 a complete piece of the timeline.
*/

class System : public Element {
      QList<Measure*> ml;
      SysStaffList _staves;
      BarLine* barLine;       ///< Left hand bar, connects staves in system.
      bool _pageBreak;
      qreal _width;

      void setInstrumentName(int staff);
      void setDistance(int n, Spatium v)   { _staves[n]->setDistance(v); }
      void layoutLyrics(ScoreLayout*, Lyrics*, Segment*, int staffIdx);

   public:
      System(Score*);
      ~System();
      virtual System* clone() const { return new System(*this); }
      virtual ElementType type() const { return SYSTEM; }

      virtual QRectF bbox() const;
      void setWidth(qreal v)  { _width = v; }

      virtual void add(Element*);
      virtual void remove(Element*);

      Page* page() const                 { return (Page*)parent(); }

      double layout(ScoreLayout*, const QPointF&, double);
      void layout2(ScoreLayout*);         ///< Called after Measure layout.
      void clear();                       ///< Clear measure list.

      QList<Measure*>& measures()          { return ml; }

      QRectF bboxStaff(int staff) const;
      SysStaffList* staves()               { return &_staves; }
      SysStaff* staff(int n)               { return _staves[n]; }

      double distance(int n) const         { return _staves[n]->distance(); }
      void setDistance(int n, double v)    { _staves[n]->setDistance(v); }
      bool pageBreak() const               { return _pageBreak; }
      void setPageBreak(bool val)          { _pageBreak = val; }

      SysStaff* insertStaff(Staff*, int);
      void insertSysStaff(SysStaff*, int);
      SysStaff* removeStaff(int);

      BarLine* getBarLine() const         { return barLine; }
      int y2staff(qreal y) const;
      void setInstrumentNames();
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;
      Measure* prevMeasure(const Measure*) const;
      Measure* nextMeasure(const Measure*) const;
      };

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;

#endif

