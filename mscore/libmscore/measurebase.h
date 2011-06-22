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

#ifndef __MEASUREBASE_H__
#define __MEASUREBASE_H__

/**
 \file
 Definition of MeasureBase class.
*/

#include "element.h"

class Score;
class System;
class Measure;
class LayoutBreak;

//---------------------------------------------------------
//   MeasureWidth
//---------------------------------------------------------

/**
 result of layoutX().
*/

struct MeasureWidth {
      double stretchable;
      double nonStretchable;

      MeasureWidth() {}
      MeasureWidth(double a, double b) {
            stretchable = a;
            nonStretchable = b;
            }
      };

//---------------------------------------------------------
//   MeasureBase
//---------------------------------------------------------

/**
      Base class for Measure, HBox and VBox
*/

class MeasureBase : public Element {
      MeasureBase* _next;
      MeasureBase* _prev;
      int _tick;

   protected:
      MeasureWidth _mw;
      ElementList _el;        ///< Measure(/tick) relative -elements: with defined start time
                              ///< but outside the staff

      bool _dirty;
      bool _lineBreak;        ///< Forced line break
      bool _pageBreak;        ///< Forced page break
      LayoutBreak* _sectionBreak;

   public:
      MeasureBase(Score* score);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);
      virtual MeasureBase* clone() const = 0;
      virtual void setScore(Score* s);

      MeasureBase* next() const              { return _next;   }
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Measure* nextMeasure() const;
      Measure* prevMeasure() const;

      virtual int ticks() const              { return 0;       }
      virtual void write(Xml&, int, bool) const = 0;

      virtual void scanElements(void* data, void (*func)(void*, Element*));
      virtual void scanVisibleElements(void* data, void (*func)(void*, Element*), bool onlyVisible);
      MeasureWidth& layoutWidth()            { return _mw;        }
      void setLayoutWidth(const MeasureWidth& w) { _mw = w; }
      ElementList* el()                      { return &_el; }
      const ElementList* el() const          { return &_el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }

      bool lineBreak() const                 { return _lineBreak; }
      bool pageBreak() const                 { return _pageBreak; }
      LayoutBreak* sectionBreak() const      { return _sectionBreak; }
      void setLineBreak(bool v)              { _lineBreak = v;    }
      void setPageBreak(bool v)              { _pageBreak = v;    }
      void setSectionBreak(LayoutBreak* v)   { _sectionBreak = v; }

      virtual void moveTicks(int diff)       { setTick(tick() + diff); }

      virtual double distanceUp(int) const        { return 0.0; }
      virtual double distanceDown(int) const      { return 0.0; }
      virtual Spatium userDistanceUp(int) const   { return Spatium(0.0); }
      virtual Spatium userDistanceDown(int) const { return Spatium(0.0); }

      virtual void add(Element*);
      virtual void remove(Element*);
      void setDirty(bool val = true)              { _dirty = val; }
      bool dirty() const                          { return _dirty; }
      virtual void spatiumChanged(double oldValue, double newValue);

      int tick() const                       { return _tick;         }
      void setTick(int t)                    { _tick = t;            }

      double pause() const;
      };

#endif

