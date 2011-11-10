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
class QPainter;
class MeasureBase;

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

class PageFormat {
      qreal _width;
      qreal _height;
      qreal _printableWidth;        // _width - left margin - right margin
      qreal _evenLeftMargin;        // values in inch
      qreal _oddLeftMargin;
      qreal _evenTopMargin;
      qreal _evenBottomMargin;
      qreal _oddTopMargin;
      qreal _oddBottomMargin;
      int _size;                    // index in paperSizes[]
      bool _landscape;
      bool _twosided;

   public:
      PageFormat();

      qreal width() const;         // return width in inch
      void setWidth(qreal val)  { _width = val; }
      qreal height() const;        // height in inch
      void setHeight(qreal val) { _height = val; }

      QString name() const;
      void read(QDomElement,  Score*);
      void readMusicXML(QDomElement, qreal);
      void write(Xml&) const;
      void writeMusicXML(Xml&, qreal) const;
      qreal evenLeftMargin() const   { return _evenLeftMargin;   }
      qreal oddLeftMargin() const    { return _oddLeftMargin;    }
      qreal evenTopMargin() const    { return _evenTopMargin;    }
      qreal evenBottomMargin() const { return _evenBottomMargin; }
      qreal oddTopMargin() const     { return _oddTopMargin;     }
      qreal oddBottomMargin() const  { return _oddBottomMargin;  }
      qreal printableWidth() const   { return _printableWidth;   }

      void setEvenLeftMargin(qreal val)   { _evenLeftMargin = val;   }
      void setOddLeftMargin(qreal val)    { _oddLeftMargin = val;    }
      void setEvenTopMargin(qreal val)    { _evenTopMargin = val;    }
      void setEvenBottomMargin(qreal val) { _evenBottomMargin = val; }
      void setOddTopMargin(qreal val)     { _oddTopMargin = val;     }
      void setOddBottomMargin(qreal val)  { _oddBottomMargin = val;  }
      void setPrintableWidth(qreal val)   { _printableWidth = val;   }

      bool landscape() const      { return _landscape; }
      void setLandscape(bool val) { _landscape = val; }

      bool twosided() const       { return _twosided; }
      void setTwosided(bool val)  { _twosided = val; }

      int size() const            { return _size; }
      void setSize(int);

      // convenience functions
      qreal evenRightMargin() const  { return width() - _printableWidth - _evenLeftMargin; }
      qreal oddRightMargin() const   { return width() - _printableWidth - _oddLeftMargin;  }
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

      virtual void draw(QPainter*) const;
      virtual void scanElements(void* data, void (*func)(void*, Element*), bool all=true);

      QList<const Element*> items(const QRectF& r);
      QList<const Element*> items(const QPointF& p);
      void rebuildBspTree() { bspTreeValid = false; }
      virtual QPointF pagePos() const { return QPointF(); }     ///< position in page coordinates
      QList<System*> searchSystem(const QPointF& pos) const;
      Measure* searchMeasure(const QPointF& p) const;
      MeasureBase* pos2measure(const QPointF&, int* staffIdx, int* pitch,
         Segment**, QPointF* offset) const;
      };

extern const PaperSize paperSizes[];
extern int paperSizeNameToIndex(const QString&);
extern int paperSizeSizeToIndex(const qreal wi, const qreal hi);

#endif
