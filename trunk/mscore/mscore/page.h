//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: page.h,v 1.18 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __PAGE_H__
#define __PAGE_H__

#include "element.h"

class SystemList;
class System;
class Text;
class Measure;
class Xml;
class Score;
class Painter;
class ScoreLayout;

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
      QPrinter::PageSize qtsize;
      const char* name;
      double w, h;
      PaperSize(QPrinter::PageSize s, const char* n, double wi, double hi)
         : qtsize(s), name(n), w(wi), h(hi) {}
      };

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

struct PageFormat {
      int size;         // index in paperSizes[]
      double evenLeftMargin;
      double evenRightMargin;
      double evenTopMargin;
      double evenBottomMargin;
      double oddLeftMargin;
      double oddRightMargin;
      double oddTopMargin;
      double oddBottomMargin;
      bool landscape;
      bool twosided;

   public:
      PageFormat();
      double width() const;
      double height() const;
      const char* name() const;
      void read(QDomNode);
      void write(Xml&);
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public Element {
      ScoreLayout* _layout;
      SystemList* _systems;
      int _no;                            // page number
      Text* _pageNo;
      Text* _copyright;
      ElementList _elements;

   public:
      Page(ScoreLayout*);
      ~Page();
      virtual Element* clone() const { return new Page(*this); }
      virtual ElementType type() const   { return PAGE; }

      SystemList* systems() const        { return _systems;   }
      void appendSystem(System* s);
      double addMeasure(Measure*, double);

      int no() const                     { return _no;        }
      void setNo(int n)                  { _no = n;           }
      bool isOdd() const                 { return (_no+1) & 1;    }

      double tm() const;       // margins
      double bm() const;
      double lm() const;
      double rm() const;
      double loWidth() const;
      double loHeight() const;

      ElementList* pel()                 { return &_elements; }
      const ElementList* pel() const     { return &_elements; }
      Text* pageNo() const              { return _pageNo;    }
      Text* copyright() const           { return _copyright; }

      virtual void add(Element*);
      virtual void remove(Element* el);

      void drawBorder(Painter& p) const;
      void layout();

      virtual void draw(Painter&p);
      };

//---------------------------------------------------------
//   PageList
//---------------------------------------------------------

class PageList : public pstl::plist<Page*> {
   public:
      PageList() {}
      void update();
      };

typedef PageList::iterator iPage;
typedef PageList::const_iterator ciPage;
typedef PageList::reverse_iterator riPage;
typedef PageList::const_reverse_iterator criPage;

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);

#endif
