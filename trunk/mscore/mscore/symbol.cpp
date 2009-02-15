//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
#include "xml.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "layout.h"
#include "page.h"
#include "score.h"
#include "viewer.h"
#include "image.h"

//---------------------------------------------------------
//   BSymbol
//---------------------------------------------------------

BSymbol::BSymbol(const BSymbol& s)
   : Element(s)
      {
      _leafs = s._leafs;
      foreach(Element* e, _leafs)
            e->setParent(this);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void BSymbol::add(Element* e)
      {
      if (e->type() == SYMBOL || e->type() == IMAGE) {
            e->setParent(this);
            _leafs.append(e);
            BSymbol* b = static_cast<BSymbol*>(e);
            foreach(Element* ee, b->getLeafs())
                  ee->setParent(b);
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
//   collectElements
//---------------------------------------------------------

void BSymbol::collectElements(QList<const Element*>& el) const
      {
      el.append(this);
      foreach (Element* e, _leafs)
            e->collectElements(el);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BSymbol::acceptDrop(Viewer* viewer, const QPointF&, int type, int) const
      {
      if (type == SYMBOL || type == IMAGE) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BSymbol::drop(const QPointF&, const QPointF&, Element* el)
      {
      if (el->type() == SYMBOL || el->type() == IMAGE) {
            el->setParent(this);
            score()->undoAddElement(el);
            return el;
            }
      else
            delete el;
      return 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void BSymbol::layout(ScoreLayout* l)
      {
      foreach(Element* e, _leafs)
            e->layout(l);
      if (parent() && parent()->type() == MEASURE) {
            Measure* m = static_cast<Measure*>(parent());
            double y = track() != -1 ? m->system()->staff(track() / VOICES)->y() : 0.0;
            double x = (tick() != -1) ? m->tick2pos(tick()) : 0.0;
            setPos(ipos() + QPointF(x, y));
            }
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF BSymbol::drag(const QPointF& pos)
      {
      QRectF r(abbox());
      foreach(const Element* e, _leafs)
            r |= e->abbox();
      setUserOff(pos / _spatium);
      r |= abbox();
      foreach(const Element* e, _leafs)
            r |= e->abbox();
      return r;
      }


//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(Score* s)
   : BSymbol(s)
      {
      _sym = 0;
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

void Symbol::layout(ScoreLayout* layout)
      {
      Element::layout(layout);
      BSymbol::layout(layout);
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(QPainter& p) const
      {
      symbols[_sym].draw(p, mag());
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Symbol::bbox() const
      {
      return symbols[_sym].bbox(mag());
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag("Symbol");
      xml.tag("name", symbols[_sym].name());
      Element::writeProperties(xml);
      foreach(const Element* e, getLeafs())
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(QDomElement e)
      {
      QPointF pos;
      int s = -1;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "name") {
                  for (int i = 0; i < symbols.size(); ++i) {
                        if (val == symbols[i].name()) {
                              s = i;
                              break;
                              }
                        }
                  if (s == -1) {
                        printf("unknown symbol <%s>, symbols %d\n",
                           qPrintable(val), symbols.size());
                        s = 0;
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  add(s);
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
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
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (s == -1) {
            printf("unknown symbol\n");
            s = 0;
            }
      setPos(pos);
      setSym(s);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Symbol::dragAnchor() const
      {
      if (parent()->type() == MEASURE) {
            Measure* measure = static_cast<Measure*>(parent());
            Segment* segment = measure->tick2segment(tick());
            System* s        = measure->system();
            double y         = measure->canvasPos().y() + s->staff(staffIdx())->y();
            QPointF anchor(segment->abbox().x(), y);
            return QLineF(canvasPos(), anchor);
            }
      else {
            return QLineF(canvasPos(), parent()->canvasPos());
            }
      }

