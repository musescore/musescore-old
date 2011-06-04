//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: symbol.cpp 3592 2010-10-18 17:24:18Z wschweer $
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

#include "symbol.h"
#include "sym.h"
#include "al/xml.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "page.h"
#include "score.h"
#include "image.h"
#include "segment.h"

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(const BSymbol& s)
   : Element(s), ElementLayout(s)
      {
      foreach(Element* e, s._leafs) {
            Element* ee = e->clone();
            ee->setParent(this);
            _leafs.append(ee);
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BSymbol::add(Element* e)
      {
      if (e->type() == SYMBOL || e->type() == IMAGE) {
            e->setParent(this);
            _leafs.append(e);
            }
      else
            printf("BSymbol::add: unsupported type %s\n", e->name());
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void BSymbol::remove(Element* e)
      {
      if (e->type() == SYMBOL || e->type() == IMAGE) {
            if (!_leafs.removeOne(e))
                  printf("BSymbol::remove: element <%s> not found\n", e->name());
            }
      else
            printf("BSymbol::remove: unsupported type %s\n", e->name());
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void BSymbol::scanElements(void* data, void (*func)(void*, Element*))
      {
      func(data, this);
      foreach (Element* e, _leafs)
            e->scanElements(data, func);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BSymbol::layout()
      {
      foreach(Element* e, _leafs)
            e->layout();
      }

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(Score* s)
   : BSymbol(s)
      {
      _sym = 0;
      _small = false;
      }

Symbol::Symbol(Score* s, int sy)
   : BSymbol(s)
      {
      _sym = sy;
      _small = false;
      }

Symbol::Symbol(const Symbol& s)
   : BSymbol(s)
      {
      _sym   = s._sym;
      _small = s._small;
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Symbol::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Symbol::layout()
      {
      foreach(Element* e, leafs())
            e->layout();
      ElementLayout::layout(this);
      BSymbol::layout();
      _bbox = symbols[score()->symIdx()][_sym].bbox(magS());
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(Painter* p) const
      {
      symbols[score()->symIdx()][_sym].draw(p, magS());
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(XmlReader* r)
      {
      QPointF pos;
      int s = -1;

      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;

            if (r->readString("name", &val)) {
                  for (int i = 0; i < lastSym; ++i) {
                        if (val == symbols[0][i].name()) {
                              s = i;
                              break;
                              }
                        }
                  if (s == -1) {
                        printf("unknown symbol <%s>, symbols %d\n",
                           qPrintable(val), lastSym);
                        s = 0;
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(r);
                  s->adjustReadPos();
                  add(s);
                  }
#if 0
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  XmlReader* ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->read(e);
                        add(image);
                        }
                  }
#endif
            else if (r->readBool("small", &_small))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      if (s == -1) {
            printf("unknown symbol\n");
            s = 0;
            }
      setPos(pos);
      setSym(s);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF BSymbol::canvasPos() const
      {
      if (parent() && (parent()->type() == SEGMENT)) {
            qreal xp = x();
            for (Element* e = parent(); e; e = e->parent())
                  xp += e->x();
            qreal yp = y();
            Segment* s = static_cast<Segment*>(parent());
            yp += s->measure()->system()->staffY(staffIdx());
            return QPointF(xp, yp);
            }
      else
            return Element::canvasPos();
      }


