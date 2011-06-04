//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: box.cpp 3708 2010-11-16 09:54:31Z wschweer $
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

#include "box.h"
#include "text.h"
#include "score.h"
#include "barline.h"
#include "repeat.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"
#include "al/xml.h"
#include "painter.h"

static const qreal BOX_MARGIN = 0.0;

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      editMode      = false;
      _boxWidth     = Spatium(5.0);
      _boxHeight    = Spatium(10.0);
      _leftMargin   = BOX_MARGIN;
      _rightMargin  = BOX_MARGIN;
      _topMargin    = BOX_MARGIN;
      _bottomMargin = BOX_MARGIN;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout()
      {
      foreach (Element* el, _el)
            el->layout();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(Painter* /*p*/) const
      {
#if 0
      if (score() && score()->printing())
            return;
      if (selected() || editMode || dropTarget() || score()->showFrames()) {
            Pen pen;
            if (selected() || editMode || dropTarget())
                  pen = Pen(Color(0, 0, 255));
            else
                  pen = Pen(Color(80, 80, 80));      // Qt::gray));
            qreal w = 2.0 / p->matrix().m11();
            pen.setWidthF(w);
            p->setPen(pen);
            p->setBrush(QBrush());
            w *= .5;
            p->drawRect(bbox().adjusted(w, w, -w, -w));
            }
#endif
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(XmlReader* r)
      {
      _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;
//      qreal _spatium = spatium();

      qreal d;
      while (r->readElement()) {
            MString8 tag = r->tag();
            if (r->readReal("height", &d))
                  _boxHeight = Spatium(d);
            else if (r->readReal("width", &d))
                  _boxWidth = Spatium(d);
            else if (r->readReal("leftMargin", &_leftMargin))
                  ;
            else if (r->readReal("rightMargin", &_rightMargin))
                  ;
            else if (r->readReal("topMargin", &_topMargin))
                  ;
            else if (r->readReal("bottomMargin", &_bottomMargin))
                  ;
            else if (tag == "Text") {
                  Text* t;
                  if (type() == TBOX) {
                        t = static_cast<TBox*>(this)->getText();
                        t->read(r);
                        }
                  else {
                        t = new Text(score());
                        t->read(r);
                        add(t);
                        }
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->read(r);
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
                     || s.endsWith(".gif")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->setTrack(score()->curTrack);
                        image->read(e);
                        add(image);
                        }
                  }
#endif
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(r);
                  add(lb);
                  }
            else if (tag == "HBox") {
                  HBox* hb = new HBox(score());
                  hb->read(r);
                  add(hb);
                  }
            else if (tag == "VBox") {
                  VBox* vb = new VBox(score());
                  vb->read(r);
                  add(vb);
                  }
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a el to Box
//---------------------------------------------------------

void Box::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == LAYOUT_BREAK) {
            for (iElement i = _el.begin(); i != _el.end(); ++i) {
                  if ((*i)->type() == LAYOUT_BREAK && (*i)->subtype() == e->subtype()) {
                        return;
                        }
                  }
            switch(e->subtype()) {
                  case LAYOUT_BREAK_PAGE:
                        _pageBreak = true;
                        break;
                  case LAYOUT_BREAK_LINE:
                        _lineBreak = true;
                        break;
                  case LAYOUT_BREAK_SECTION:
                        _sectionBreak = true;
                        break;
                  }
            }
      if (e->type() == TEXT) {
            static_cast<Text*>(e)->setLayoutToParentWidth(true);
            }
      _el.append(e);
      if (e->type() == IMAGE)
            static_cast<Image*>(e)->reference();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HBox::layout()
      {
      if (parent() && parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x = vb->leftMargin() * DPMM;
            qreal y = vb->topMargin() * DPMM;
            qreal w = point(boxWidth());
            qreal h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
            setPos(x, y);
            setbbox(QRectF(0.0, 0.0, w, h));
            }
      else {
            setPos(0.0, 0.0);
            setbbox(QRectF(0.0, 0.0, point(boxWidth()), system()->height()));
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   layout2
//    height (bbox) is defined now
//---------------------------------------------------------

void HBox::layout2()
      {
      Box::layout();
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->type() == HBOX || parent()->type() == VBOX);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VBox::layout()
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      Box::layout();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void FBox::layout()
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      Box::layout();
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to fret diagram box
//---------------------------------------------------------

void FBox::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == FRET_DIAGRAM) {
            FretDiagram* fd = static_cast<FretDiagram*>(e);
            fd->setFlag(ELEMENT_MOVABLE, false);
            }
      else {
            printf("FBox::add: element not allowed\n");
            return;
            }
      _el.append(e);
      }

