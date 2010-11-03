//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "bsp.h"

class System;
class Text;
class Measure;
class Xml;
class Score;

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
      int _pageOffset;              ///< Offset for page numbers.

   public:
      PageFormat();
      double width() const;         // return width in inch
      double height() const;        // height in inch
      QString name() const;
      void read(QDomElement);
      void readMusicXML(QDomElement, double);
      void write(Xml&);
      void writeMusicXML(Xml&, double);
      int pageOffset() const { return _pageOffset; }
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public Element {
      QList<System*> _systems;
      int _no;                      // page number
      Text* _footer;
      Text* _header;
      BspTree bspTree;

      QString replaceTextMacros(const QString&);

   public:
      Page(Score*);
      ~Page();
      virtual Page* clone() const            { return new Page(*this); }
      virtual ElementType type() const       { return PAGE; }
      const QList<System*>* systems() const  { return &_systems;   }
      QList<System*>* systems()              { return &_systems;   }

      void appendSystem(System* s);

      int no() const                     { return _no;        }
      void setNo(int n);
      bool isOdd() const                 { return (_no+1) & 1;    }

      double tm() const;            // margins in pixel
      double bm() const;
      double lm() const;
      double rm() const;
      double loWidth() const;
      double loHeight() const;

      Text* footer() const { return _footer; }
      Text* header() const { return _header; }

      void layout();

      virtual void draw(QPainter&, ScoreView*) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*));
      void clear();

      QList<const Element*> items(const QRectF& r)  { return bspTree.items(r); }
      QList<const Element*> items(const QPointF& p) { return bspTree.items(p); }
      void insertBsp(Element* e)                    { bspTree.insert(e);       }
      void removeBsp(Element* e)                    { bspTree.remove(e);       }
      void rebuildBspTree();
      };

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);
extern int paperSizeSizeToIndex(const double wi, const double hi);

#endif