//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: system.h,v 1.23 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "element.h"
#include "spatium.h"

class Staff;
class SStaff;
class BarLine;
class Clef;
class Page;
class TextElement;
class Painter;
class MeasureList;
class Bracket;

//---------------------------------------------------------
//   SysStaff
//    one staff of a system
//---------------------------------------------------------

class SysStaff {
      QRectF _bbox;   // bbox of SStaff
      Spatium _distance;

   public:
      int idx;
      Bracket* bracket;
      SStaff*  sstaff;
      TextElement* instrumentName;

      const QRectF& bbox() const      { return _bbox; }
      void setbbox(const QRectF& r)   { _bbox = r; }
      void move(double x, double y);

      double distance() const        { return point(_distance); }
      void setDistance(double v)     { _distance.set(v); }
      void setDistance(Spatium v)    { _distance = v; }

      SysStaff();
      ~SysStaff();
      };

typedef std::vector<SysStaff*> SysStaffList;
typedef SysStaffList::iterator iSysStaff;
typedef SysStaffList::const_iterator ciSysStaff;

//---------------------------------------------------------
//   System
//    one row of measures for all instruments;
//    a complete piece of the timeline
//---------------------------------------------------------

class System : public Element {
      MeasureList* ml;
      SysStaffList _staves;
      BarLine* barLine;               // left hand bar; connects staves in system
      bool _pageBreak;

      void setInstrumentName(int staff);

   public:
      System(Score*);
      ~System();
      virtual ElementType type() const { return SYSTEM; }

      virtual void add(Element*);
      virtual void remove(Element*);

      Page* page() const                 { return (Page*)parent(); }

      double layout(const QPointF&, double);
      void layout2();                     // called after Measure layout
      void clear();                       // clear measure list

      MeasureList* measures() const { return ml; }

      const QRectF& bboxStaff(int staff) const;
      SysStaffList* staves()               { return &_staves; }
      SysStaff* staff(int n)               { return _staves[n]; }

      double distance(int n) const;
      void setDistance(int n, double v)    { _staves[n]->setDistance(v); }
      bool pageBreak() const               { return _pageBreak; }
      void setPageBreak(bool val)          { _pageBreak = val; }

      SysStaff* insertStaff(Staff*, int);
      void insertSysStaff(SysStaff*, int);
      SysStaff* removeStaff(int);

      virtual void draw(Painter&);

      BarLine* getBarLine() const         { return barLine; }
      bool pos2tick(const QPointF&, int* tick, Staff** staff, int* pitch) const;
      int y2staff(qreal y) const;
      Element* findSelectableElement(QPointF p) const;
      void setInstrumentNames();
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;
      Measure* prevMeasure(Measure*) const;
      Measure* nextMeasure(Measure*) const;
      };

//---------------------------------------------------------
//   SystemList
//---------------------------------------------------------

typedef pstl::plist<System*>::iterator iSystem;
typedef pstl::plist<System*>::const_iterator ciSystem;
typedef pstl::plist<System*>::const_reverse_iterator rciSystem;
typedef pstl::plist<System*>::reverse_iterator riSystem;

class SystemList : public pstl::plist<System*> {
   public:
      SystemList() {}
      };

#endif

