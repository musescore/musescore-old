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

#ifndef __PAGE_H__
#define __PAGE_H__

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

   public:
      PageFormat();
      qreal width() const;         // return width in inch
      qreal height() const;        // height in inch
      QString name() const;
      void read(QDomElement,  Score*);
      void readMusicXML(QDomElement, qreal);
      void write(Xml&);
      void writeMusicXML(Xml&, qreal);
      };

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

class Page : public Element {
      QList<System*> _systems;
      int _no;                      // page number
      BspTree bspTree;
      bool bspTreeValid;

      QString replaceTextMacros(const QString&) const;
      void doRebuildBspTree();

   public:
      Page(Score*);
      ~Page();
      virtual Page* clone() const            { return new Page(*this); }
      virtual ElementType type() const       { return PAGE; }
      const QList<System*>* systems() const  { return &_systems;   }
      QList<System*>* systems()              { return &_systems;   }

      virtual void layout();
      virtual void write(Xml&) const;
      virtual void read(QDomElement);

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

      virtual void draw(Painter*) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);
      void clear();

      QList<const Element*> items(const QRectF& r);
      QList<const Element*> items(const QPointF& p);
      void rebuildBspTree() { bspTreeValid = false; }
      virtual QPointF canvasPos() const { return QPointF(); }     ///< position in page coordinates
      };

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);
extern int paperSizeSizeToIndex(const qreal wi, const qreal hi);

#endif
