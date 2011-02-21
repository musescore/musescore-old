//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "page.h"
#include "score.h"
#include "image.h"
#include "segment.h"
#include "painter.h"

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
//   acceptDrop
//---------------------------------------------------------

bool BSymbol::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      return type == SYMBOL || type == IMAGE;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* BSymbol::drop(const DropData& data)
      {
      Element* el = data.element;
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

void BSymbol::layout()
      {
      foreach(Element* e, _leafs)
            e->layout();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF BSymbol::drag(const QPointF& pos)
      {
      QRectF r(abbox());
      foreach(const Element* e, _leafs)
            r |= e->abbox();
      setUserOff(pos);
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
//      _small = false;
      }

Symbol::Symbol(Score* s, int sy)
   : BSymbol(s)
      {
      _sym = sy;
//      _small = false;
      }

Symbol::Symbol(const Symbol& s)
   : BSymbol(s)
      {
      _sym   = s._sym;
//      _small = s._small;
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
//      double m = parent() ? parent()->mag() : 1.0;
//      if (_small)
//            m *= score()->styleD(ST_smallNoteMag);
//      setMag(m);
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
      if (type() != NOTEDOT || !staff()->useTablature())
            symbols[score()->symIdx()][_sym].draw(*p->painter(), magS());
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("name", symbols[score()->symIdx()][_sym].name());
      Element::writeProperties(xml);
//      xml.tag("small", _small);
      foreach(const Element* e, leafs())
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
                  for (int i = 0; i < symbols[0].size(); ++i) {
                        if (val == symbols[0][i].name()) {
                              s = i;
                              break;
                              }
                        }
                  if (s == -1) {
                        printf("unknown symbol <%s>, symbols %d\n",
                           qPrintable(val), symbols[0].size());
                        s = 0;
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(e);
                  s->adjustReadPos();
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
            else if (tag == "small") {
                  ; // _small = val.toInt();
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
            Segment* seg     = segment();
            Measure* measure = seg->measure();
            System* s        = measure->system();
            double y         = measure->canvasPos().y() + s->staff(staffIdx())->y();
            QPointF anchor(seg->abbox().x(), y);
            return QLineF(canvasPos(), anchor);
            }
      else {
            return QLineF(canvasPos(), parent()->canvasPos());
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Symbol::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("symbol editor"));
      a->setData("edit");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Symbol::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "edit") {
printf("not implemented: symbol editor\n");
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   canvasPos
//---------------------------------------------------------

QPointF BSymbol::canvasPos() const
      {
      if (parent() && (parent()->type() == SEGMENT)) {
            double xp = x();
            for (Element* e = parent(); e; e = e->parent())
                  xp += e->x();
            double yp = y();
            Segment* s = static_cast<Segment*>(parent());
            yp += s->measure()->system()->staffY(staffIdx());
            return QPointF(xp, yp);
            }
      else
            return Element::canvasPos();
      }

//---------------------------------------------------------
//   FSymbol
//---------------------------------------------------------

FSymbol::FSymbol(Score* s)
  : Element(s)
      {
      _font.setStyleStrategy(QFont::NoFontMerging);
      }

FSymbol::FSymbol(const FSymbol& s)
  : Element(s)
      {
      _font = s._font;
      _code = s._code;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void FSymbol::draw(Painter* painter) const
      {
      QString s;
      painter->setFont(_font);
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      painter->drawText(0, 0, s);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void FSymbol::write(Xml& xml) const
      {
      xml.stag(name());
      xml.tag("font", _font.family());
      xml.tag("fontsize", _font.pixelSize());
      xml.tag("code", _code);
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void FSymbol::read(QDomElement e)
      {
      QPointF pos;
      int s = -1;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "font")
                  _font.setFamily(val);
            else if (tag == "fontsize")
                  _font.setPixelSize(val.toInt());
            else if (tag == "code")
                  _code = val.toInt();
            else if (!Element::readProperties(e))
                  domError(e);
            }
      setPos(pos);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FSymbol::layout()
      {
      QString s;
      if (_code & 0xffff0000) {
            s = QChar(QChar::highSurrogate(_code));
            s += QChar(QChar::lowSurrogate(_code));
            }
      else
            s = QChar(_code);
      QFontMetricsF fm(_font);
      setbbox(fm.boundingRect(s));
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool FSymbol::genPropertyMenu(QMenu*) const
      {
      return false;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void FSymbol::propertyAction(ScoreView*, const QString&)
      {
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void FSymbol::setFont(const QFont& f)
      {
      _font = f;
      _font.setStyleStrategy(QFont::NoFontMerging);
      }

