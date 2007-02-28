//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layout.h,v 1.5 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __LAYOUT_H__
#define __LAYOUT_H__

#include "bsp.h"
#include "measure.h"

class Score;
class PageFormat;
class PageList;
class Page;
class Measure;
class QPaintDevice;
class SystemList;
class System;

//---------------------------------------------------------
//   ElemList
//---------------------------------------------------------

class ElemList {
      int _size;
      Element* _first;
      Element* _last;

   public:
      ElemList() {
            _first = 0;
            _last  = 0;
            _size  = 0;
            };
      Element* first() const { return _first; }
      Element* last()  const { return _last; }
      void clear()           { _first = _last = 0; }
      void push_back(Element* e);
      void push_front(Element* e);
      void insert(Element*, Element*);
      void erase(Element*);
      };

//---------------------------------------------------------
//   ScoreLayout
//---------------------------------------------------------

class ScoreLayout {
      Score* _score;
      double _spatium;
      PageFormat* _pageFormat;
      QPaintDevice* _paintDevice;
      ElementList el;
      BspTree bspTree;

      //
      // modified by layout()

      ElemList _measures;           // here are the notes
      //
      // generated objects by layout():
      //
      PageList* _pages;        // pages are build from systems
      SystemList* _systems;    // measures are akkumulated to systems

      bool _needLayout;

      Page* addPage();
      bool layoutPage(Page* page, Measure*& im, QList<System*>::iterator& is);
      System* layoutSystem(Measure*& im, System*, qreal x, qreal y, qreal w);
      void clearGenerated(Measure* m);
      void addGenerated(Measure* m);
      void processSystemHeader(Measure* m);

   public:
      ScoreLayout();
      ~ScoreLayout();

      void setScore(Score*);
      void layout()                  { _needLayout = true; }
      void doLayout();
      Score* score() const           { return _score; }
      double spatium() const         { return _spatium; }
      void setSpatium(double v)      { _spatium = v; }
      PageFormat* pageFormat() const { return _pageFormat; }
      void setPageFormat(const PageFormat& pf);
      PageList* pages() const        { return _pages; }
      SystemList* systems() const    { return _systems; }
      bool needLayout() const        { return _needLayout; }
      Measure* first() const         { return (Measure*)_measures.first(); }
      Measure* last()  const         { return (Measure*)_measures.last();  }
      void push_back(Measure* el)    { _measures.push_back((Element*)el);  }
      void clear()                   { _measures.clear(); }
      void erase(Measure* im)        { _measures.erase(im); }
      void insert(Measure* im, Measure* m)    { _measures.insert(im, m); }
      void setPaintDevice(QPaintDevice* d)    { _paintDevice = d; }
      QPaintDevice* paintDevice() const       { return _paintDevice; }
      QList<Element*> items(const QRectF& r)  { return bspTree.items(r); }
      QList<Element*> items(const QPointF& p) { return bspTree.items(p); }
      void setInstrumentNames();
      void connectTies();
      };

#endif

