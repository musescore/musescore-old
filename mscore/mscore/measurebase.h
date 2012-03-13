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

   protected:
      MeasureWidth _mw;
      bool _dirty;

      ElementList _el;        ///< Measure(/tick) relative -elements: with defined start time
                              ///< but outside the staff

      bool _lineBreak;        ///< Forced line break
      bool _pageBreak;        ///< Forced page break

   public:
      MeasureBase(Score* score);
      ~MeasureBase();
      MeasureBase(const MeasureBase&);
      virtual MeasureBase* clone() const = 0;

      MeasureBase* next() const              { return _next;   }
      void setNext(MeasureBase* e)           { _next = e;      }
      MeasureBase* prev() const              { return _prev;   }
      void setPrev(MeasureBase* e)           { _prev = e;      }

      Measure* nextMeasure();
      Measure* prevMeasure();

      virtual int tickLen() const            { return 0;       }
      virtual void write(Xml&, int, bool) const = 0;

      virtual void scanElements(void* data, void (*func)(void*, Element*));
      MeasureWidth& layoutWidth()            { return _mw;        }
      ElementList* el()                      { return &_el; }
      const ElementList* el() const          { return &_el; }
      System* system() const                 { return (System*)parent(); }
      void setSystem(System* s)              { setParent((Element*)s);   }
      bool lineBreak() const                 { return _lineBreak; }
      bool pageBreak() const                 { return _pageBreak; }
      void setLineBreak(bool v)              { _lineBreak = v;    }
      void setPageBreak(bool v)              { _pageBreak = v;    }
      virtual void moveTicks(int diff)       { setTick(tick() + diff); }
      virtual double distance(int) const     { return 0.0; }
      virtual Spatium userDistance(int) const { return Spatium(0.0); }
      virtual void add(Element*);
      virtual void remove(Element*);
      void setDirty()                        { _dirty = true; }
      virtual void spatiumChanged(double oldValue, double newValue);
      };

#endif

