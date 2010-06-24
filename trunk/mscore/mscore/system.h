//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
class Clef;
class Page;
class TextC;
class Bracket;
class Lyrics;
class Segment;
class MeasureBase;

//---------------------------------------------------------
//   SysStaff
//---------------------------------------------------------

/**
 One staff of a System.
*/

class SysStaff {
      QRectF _bbox;           ///< Bbox of StaffLines.
      Spatium _distance;      ///< distance to next staff
      bool _show;             ///< derived from Staff or false if empty
                              ///< staff is hidden
   public:
      int idx;
      QList<Bracket*> brackets;
      TextC* instrumentName;

      const QRectF& bbox() const     { return _bbox; }
      QRectF& rbb()                  { return _bbox; }
      double y() const               { return _bbox.y(); }
      double right() const           { return _bbox.right(); }
      void setbbox(const QRectF& r)  { _bbox = r; }
      void move(double x, double y);

      Spatium distance() const       { return _distance; }
      void setDistance(Spatium v)    { _distance = v; }

      bool show() const              { return _show; }
      void setShow(bool v)           { _show = v; }

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
      bool _pageBreak;
      qreal _leftMargin;      ///< left margin for instrument name, brackets etc.
      bool _firstSystem;      ///< used to decide between long and short instrument
                              ///< names; set by score()->doLayout()

      bool _vbox;             ///< contains only one VBox in ml

      void setInstrumentName(int staff);
      void setDistance(int n, Spatium v)   { _staves[n]->setDistance(v); }

   public:
      System(Score*);
      ~System();
      virtual System* clone() const    { return new System(*this); }
      virtual ElementType type() const { return SYSTEM; }

      virtual void add(Element*);
      virtual void remove(Element*);
      virtual void change(Element* o, Element* n);

      virtual void scanElements(void* data, void (*func)(void*, Element*));

      Page* page() const                 { return (Page*)parent(); }

      virtual void layout(double xoffset);
      void layout2();         ///< Called after Measure layout.
      void clear();                       ///< Clear measure list.

      QList<MeasureBase*>& measures()      { return ml; }

      QRectF bboxStaff(int staff) const;
      QList<SysStaff*>* staves()             { return &_staves;   }
      const QList<SysStaff*>* staves() const { return &_staves;   }
      double staffY(int idx) const;
      SysStaff* staff(int idx) const         { return _staves[idx]; }

      Spatium distance(int idx) const        { return _staves[idx]->distance(); }
      bool pageBreak() const                 { return _pageBreak; }
      void setPageBreak(bool val)            { _pageBreak = val; }

      SysStaff* insertStaff(int);
      SysStaff* removeStaff(int);

      Line* getBarLine() const             { return barLine; }
      int y2staff(qreal y) const;
      void setInstrumentNames();
      int snap(int tick, const QPointF p) const;
      int snapNote(int tick, const QPointF p, int staff) const;

      Measure* firstMeasure() const;
      Measure* lastMeasure() const;

      MeasureBase* prevMeasure(const MeasureBase*) const;
      MeasureBase* nextMeasure(const MeasureBase*) const;

      double leftMargin() const   { return _leftMargin; }
      void setFirstSystem(bool v) { _firstSystem = v;   }
      bool isVbox() const         { return _vbox;       }
      void setVbox(bool v)        { _vbox = v;          }
      void layoutLyrics(Lyrics*, Segment*, int staffIdx);
      };

typedef QList<System*>::iterator iSystem;
typedef QList<System*>::const_iterator ciSystem;

#endif

