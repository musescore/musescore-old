//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#include <QtGui/QPrinter>
#include "element.h"
#include "bsp.h"

class System;
class Text;
class Measure;
class Xml;
class Score;
class Painter;

//---------------------------------------------------------
//   PaperSize
//---------------------------------------------------------

struct PaperSize {
      int qtsize;
      const char* name;
      qreal w, h;            // size in inch
      PaperSize(int s, const char* n, qreal wi, qreal hi)
         : qtsize(s), name(n), w(wi), h(hi) {}
      };

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

struct PageFormat {
      int size;                     // index in paperSizes[]
      qreal _width;
      qreal _height;
      qreal evenLeftMargin;        // values in inch
      qreal evenRightMargin;
      qreal evenTopMargin;
      qreal evenBottomMargin;
      qreal oddLeftMargin;
      qreal oddRightMargin;
      qreal oddTopMargin;
      qreal oddBottomMargin;
      bool landscape;
      bool twosided;
      int _pageOffset;              ///< Offset for page numbers.

   public:
      PageFormat();
      qreal width() const;         // return width in inch
      qreal height() const;        // height in inch
      QString name() const;
      void read(XmlReader*);
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
      QList<const Element*> _elements;

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
      bool isOdd() const;

      qreal tm() const;            // margins in pixel
      qreal bm() const;
      qreal lm() const;
      qreal rm() const;
      qreal loWidth() const;
      qreal loHeight() const;

      Text* footer() const { return _footer; }
      Text* header() const { return _header; }

      void layout();

      virtual void scanElements(void* data, void (*func)(void*, Element*));
      void clear();

      const QList<const Element*>& items()  { return _elements; }
      void rebuildBspTree();
      };

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);
extern int paperSizeSizeToIndex(const qreal wi, const qreal hi);

#endif
