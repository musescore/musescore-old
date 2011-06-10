//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: page.cpp 3670 2010-11-03 12:59:56Z wschweer $
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

#include "page.h"
#include "score.h"
#include "text.h"
#include "m-al/xml.h"
#include "preferences.h"
#include "measure.h"
#include "style.h"
#include "chord.h"
#include "beam.h"
#include "tuplet.h"
#include "note.h"
#include "barline.h"
#include "slur.h"
#include "hook.h"
#include "lyrics.h"
#include "bracket.h"
#include "line.h"
#include "staff.h"
#include "system.h"
#include "segment.h"

//---------------------------------------------------------
//   Page
//---------------------------------------------------------

Page::Page(Score* s)
   : Element(s),
   _no(0), _footer(0), _header(0)
      {
      setPos(0.0, 0.0);
      }

Page::~Page()
      {
      delete _footer;
      delete _header;
      }

//---------------------------------------------------------
//   tm
//---------------------------------------------------------

qreal Page::tm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddTopMargin : pf->evenTopMargin) * DPI;
      }

//---------------------------------------------------------
//   bm
//---------------------------------------------------------

qreal Page::bm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddBottomMargin : pf->evenBottomMargin) * DPI;
      }

//---------------------------------------------------------
//   lm
//---------------------------------------------------------

qreal Page::lm() const
      {
      PageFormat* pf = score()->pageFormat();
      return ((!pf->twosided || isOdd()) ? pf->oddLeftMargin : pf->evenLeftMargin) * DPI;
      }

//---------------------------------------------------------
//   rm
//---------------------------------------------------------

qreal Page::rm() const
      {
      PageFormat* pf = score()->pageFormat();

      return ((!pf->twosided || isOdd()) ? pf->oddRightMargin : pf->evenRightMargin) * DPI;
      }

//---------------------------------------------------------
//   loWidth
//---------------------------------------------------------

qreal Page::loWidth() const
      {
      return score()->pageFormat()->width() * DPI;
      }

//---------------------------------------------------------
//   loHeight
//---------------------------------------------------------

qreal Page::loHeight() const
      {
      return score()->pageFormat()->height() * DPI;
      }

//---------------------------------------------------------
//   appendSystem
//---------------------------------------------------------

void Page::appendSystem(System* s)
      {
      s->setParent(this);
      _systems.append(s);
      }

//---------------------------------------------------------
//   setNo
//---------------------------------------------------------

void Page::setNo(int n)
      {
      _no = n;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Page::layout()
      {
      setbbox(QRectF(0.0, 0.0, loWidth(), loHeight()));
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Page::scanElements(void* data, void (*func)(void*, Element*))
      {
      if (_header)
            func(data, _header);
      if (_footer)
            func(data, _footer);
      foreach(System* s, _systems)
            s->scanElements(data, func);
      func(data, this);
      }

//---------------------------------------------------------
//   PageFormat
//---------------------------------------------------------

PageFormat::PageFormat() :
   evenLeftMargin(10.0 / INCH),
   evenRightMargin(10.0 / INCH),
   evenTopMargin(10.0 / INCH),
   evenBottomMargin(20.0 / INCH),
   oddLeftMargin(10.0 / INCH),
   oddRightMargin(10.0 / INCH),
   oddTopMargin(10.0 / INCH),
   oddBottomMargin(20.0 / INCH),
   landscape(false),
   twosided(true),
   _pageOffset(0)
      {
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString PageFormat::name() const
      {
      return QString(paperSizes[size].name);
      }

//---------------------------------------------------------
//   width
//    return in inch
//---------------------------------------------------------

qreal PageFormat::width() const
      {
      if (paperSizes[size].qtsize == QPrinter::Custom)
            return landscape ? _height : _width;
      return landscape ? paperSizes[size].h : paperSizes[size].w;
      }

//---------------------------------------------------------
//   height
//    return in inch
//---------------------------------------------------------

qreal PageFormat::height() const
      {
      if (paperSizes[size].qtsize == QPrinter::Custom)
            return landscape ? _width : _height;
      return landscape ? paperSizes[size].w : paperSizes[size].h;
      }

//---------------------------------------------------------
//   read
//  <page-layout>
//	  <page-height>
//	  <page-width>
//      <pageFormat>A6</pageFormat>
//      <landscape>1</landscape>
//      <page-margins>
//         <left-margin>28.3465</left-margin>
//         <right-margin>28.3465</right-margin>
//         <top-margin>28.3465</top-margin>
//         <bottom-margin>56.6929</bottom-margin>
//         </page-margins>
//      </page-layout>
//
//    sizes are given in units of 1/10 spatium;
//---------------------------------------------------------

void PageFormat::read(XmlReader* r)
      {
      landscape = false;      // for compatibility with old versions
      while (r->readElement()) {
            MString8 tag(r->tag());
            QString val;

            if (r->readString("pageFormat", &val)) {
                  size = paperSizeNameToIndex(val);
                  }
            else if (r->readBool("landscape", &landscape))
                  ;
            else if (tag == "page-margins") {
                  int type = 0; // both
                  while (r->readAttribute()) {
                        if (r->tag() == "type") {
                              QString s = r->stringValue();
                              if (s == "odd")
                                    type = 1;
                              else if (s == "even")
                                    type = 2;
                              else if (s == "both")
                                    type = 0;
                              }
                        }
                  qreal lm = 0.0, rm = 0.0, tm = 0.0, bm = 0.0;
                  while (r->readElement()) {
                        tag = r->tag();
                        if (r->readReal("left-margin", &lm))
                              lm = lm * .5 / PPI;
                        else if (r->readReal("right-margin", &rm))
                              rm = rm * .5 / PPI;
                        else if (r->readReal("top-margin", &tm))
                              tm = tm * .5 / PPI;
                        else if (r->readReal("bottom-margin", &bm))
                              bm = bm * .5 / PPI;
                        else
                              r->unknown();
                        }
                  twosided = type != 0;
                  if (type == 1 || type == 0) {
                        oddLeftMargin   = lm;
                        oddRightMargin  = rm;
                        oddTopMargin    = tm;
                        oddBottomMargin = bm;
                        }
                  if (type == 2 || type == 0) {
                        evenLeftMargin   = lm;
                        evenRightMargin  = rm;
                        evenTopMargin    = tm;
                        evenBottomMargin = bm;
                        }
                  }
            else if (r->readString("page-height", &val)) {
                  size = paperSizeNameToIndex("Custom");
                  _height = val.toDouble() * 0.5 / PPI;
                  }
            else if (r->readString("page-width", &val)) {
                  size = paperSizeNameToIndex("Custom");
                  _width = val.toDouble() * .5 / PPI;
                  }
            else if (r->readInt("page-offset", &_pageOffset))
                  ;
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Page::clear()
      {
      _systems.clear();
      }

//---------------------------------------------------------
//   rebuildBspTree
//---------------------------------------------------------

void Page::rebuildBspTree()
      {
      _elements.clear();
      foreach(System* s, _systems)
            foreach(MeasureBase* m, s->measures()) {
                  m->scanElements(&_elements, collectElements);
            }
      scanElements(&_elements, collectElements);

      // TODO: optimize away:
      QRectF bb(abbox());
      foreach(Beam* b, score()->beams()) {
            ChordRest* cr = b->elements().front();
            if (cr->segment()->measure()->system()->page() == this)
                  _elements.append(b);
            }
      }

//---------------------------------------------------------
//   replaceTextMacros
//---------------------------------------------------------

QString Page::replaceTextMacros(const QString& s)
      {
      int pageno = no() + 1 + _score->pageFormat()->_pageOffset;
      QString d;
      int n = s.size();
      for (int i = 0; i < n; ++i) {
            QChar c = s[i];
            if (c == '$' && (i < (n-1))) {
                  QChar c = s[i+1];
                  switch(c.toAscii()) {
                        case 'p':
                              d += QString("%1").arg(pageno);
                              break;
                        case 'n':
                              d += QString("%1").arg(_score->pages().size());
                              break;
                        case '$':
                              d += '$';
                              break;
                        case ':':
                              {
                              QString tag;
                              int k = i+2;
                              for (; k < n; ++k) {
                                    if (s[k].toAscii() == ':')
                                          break;
                                    tag += s[k];
                                    }
                              if (k != n) {       // found ':' ?
                                    d += score()->metaTag(tag);
                                    i = k-1;
                                    }
                              }
                              break;
                        default:
                              d += '$';
                              d += c;
                              break;
                        }
                  ++i;
                  }
            else
                  d += c;
            }
      return d;
      }

//---------------------------------------------------------
//   isOdd
//---------------------------------------------------------

bool Page::isOdd() const
      {
      return (_no + 1 + score()->pageFormat()->pageOffset()) & 1;
      }

