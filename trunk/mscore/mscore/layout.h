//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layout.h,v 1.5 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "bsp.h"
#include "measure.h"
#include "element.h"

class Score;
class PageFormat;
class Page;
class Measure;
class QPaintDevice;
class System;

//---------------------------------------------------------
//   MeasureBaseList
//---------------------------------------------------------

class MeasureBaseList {
      int _size;
      MeasureBase* _first;
      MeasureBase* _last;

   public:
      MeasureBaseList() {
            _first = 0;
            _last  = 0;
            _size  = 0;
            };
      MeasureBase* first() const { return _first; }
      MeasureBase* last()  const { return _last; }
      void clear()               { _first = _last = 0; }
      void push_back(MeasureBase* e);
      void push_front(MeasureBase* e);
      void insert(MeasureBase*, MeasureBase*);
      void erase(MeasureBase*);
      void change(MeasureBase* o, MeasureBase* n);
      };

//---------------------------------------------------------
//   ScoreLayout
//---------------------------------------------------------

class ScoreLayout : public Element {
      double _spatium;
      PageFormat* _pageFormat;
      QPaintDevice* _paintDevice;
      BspTree bspTree;

      //
      // modified by layout()
      //
      MeasureBaseList _measures;           // here are the notes
      //
      // generated objects by layout():
      //
      QList<Page*> _pages;          // pages are build from systems
      QList<System*> _systems;      // measures are akkumulated to systems

      bool _needLayout;

      Page* addPage();
      bool layoutPage();
      bool layoutSystem1(double& minWidth, double w, bool);
      QList<System*> layoutSystemRow(qreal x, qreal y, qreal w, bool);
      void processSystemHeader(Measure* m);
      System* getNextSystem(bool, bool);

      // values used during doLayout:
      int curPage;
      int curSystem;
      MeasureBase* curMeasure;

   protected:
      QList<Element*> _gel;   // global elements: Slur, SLine

   public:
      ScoreLayout(Score*);
      ~ScoreLayout();
      virtual ElementType type() const        { return LAYOUT; }
      virtual Element* clone() const          { abort(); }

      void layout()                           { _needLayout = true; }
      void doLayout();
      void reLayout(Measure*);
      Score* score() const                    { return _score; }
      double spatium() const                  { return _spatium; }
      void setSpatium(double v)               { _spatium = v; }
      PageFormat* pageFormat() const          { return _pageFormat; }
      void setPageFormat(const PageFormat& pf);
      QList<Page*> pages() const              { return _pages; }
      QList<System*>* systems()               { return &_systems; }
      bool needLayout() const                 { return _needLayout; }

      MeasureBase* first() const              { return _measures.first(); }
      MeasureBase* last()  const              { return _measures.last();  }
      void push_back(MeasureBase* el)         { _measures.push_back(el);  }
      void clear()                            { _measures.clear(); }
      void erase(MeasureBase* im);
      void insert(MeasureBase* im, MeasureBase* m);
      void change(MeasureBase* o, MeasureBase* n) { _measures.change(o, n); }

      void setPaintDevice(QPaintDevice* d)    { _paintDevice = d; }
      QPaintDevice* paintDevice() const       { return _paintDevice; }
      QList<const Element*> items(const QRectF& r)  { return bspTree.items(r); }
      QList<const Element*> items(const QPointF& p) { return bspTree.items(p); }
      void setInstrumentNames();
      void connectTies();
      void searchHiddenNotes();
      QList<Element*>* gel()                  { return &_gel; }
      const QList<Element*>* gel() const      { return &_gel; }

      virtual void add(Element*);
      virtual void remove(Element*);

      friend class Score;
      };
#endif

