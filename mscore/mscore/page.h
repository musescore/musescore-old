//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: page.h,v 1.18 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __PAGE_H__
#define __PAGE_H__

#include "element.h"

class System;
class Text;
class Measure;
class Xml;
class Score;
class ScoreLayout;

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
      QPrinter::PageSize qtsize;
      const char* name;
      double w, h;            // size in inch
      PaperSize(QPrinter::PageSize s, const char* n, double wi, double hi)
         : qtsize(s), name(n), w(wi), h(hi) {}
      };

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

struct PageFormat {
      int size;                     // index in paperSizes[]
      double _width;
      double _height;
      double evenLeftMargin;        // values in inch
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
      double width() const;         // return width in inch
      double height() const;        // height in inch
      const char* name() const;
      void read(QDomElement);
      void write(Xml&);
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public Element {
      ScoreLayout* _layout;
      QList<System*> _systems;
      int _no;                            // page number
      Text* _pageNo;
      Text* _copyright;

   public:
      Page(ScoreLayout*);
      virtual Page* clone() const        { return new Page(*this); }
      virtual ElementType type() const   { return PAGE; }

      virtual void add(Element*);
      virtual void remove(Element*);

      const QList<System*>* systems() const  { return &_systems;   }
      QList<System*>* systems()              { return &_systems;   }

      void appendSystem(System* s);

      int no() const                     { return _no;        }
      void setNo(int n)                  { _no = n;           }
      bool isOdd() const                 { return (_no+1) & 1;    }

      double tm() const;            // margins in pixel
      double bm() const;
      double lm() const;
      double rm() const;
      double loWidth() const;
      double loHeight() const;

      Text* pageNo() const               { return _pageNo;    }
      Text* copyright() const            { return _copyright; }

      void layout(ScoreLayout*);

      virtual void draw(QPainter&p) const;
      void collectElements(QList<const Element*>& el) const;
      void clear();
      };

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);

#endif
