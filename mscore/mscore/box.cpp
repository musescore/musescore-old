//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "mscore.h"
#include "barline.h"
#include "repeat.h"
#include "scoreview.h"
#include "scoreview.h"
#include "boxproperties.h"
#include "symbol.h"
#include "system.h"
#include "image.h"
#include "layoutbreak.h"
#include "fret.h"

static const double BOX_MARGIN = 0.0;

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

void Box::draw(QPainter& p, ScoreView*) const
      {
      if (score() && score()->printing())
            return;
      if (selected() || editMode || dropTarget() || score()->showFrames()) {
            QPen pen;
            if (selected() || editMode || dropTarget())
                  pen = QPen(QColor(Qt::blue));
            else
                  pen = QPen(QColor(Qt::gray));
            double w = 2.0 / p.matrix().m11();
            pen.setWidthF(w);
            p.setPen(pen);
            p.setBrush(QBrush());
            w *= .5;
            p.drawRect(bbox().adjusted(w, w, -w, -w));
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Box::startEdit(ScoreView*, const QPointF&)
      {
      editMode = true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(ScoreView*, int /*grip*/, int /*key*/, Qt::KeyboardModifiers, const QString&)
      {
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(int, const QPointF& d)
      {
      if (type() == VBOX)
            _boxHeight += Spatium(d.y() / spatium());
      else {
            _boxWidth += Spatium(d.x() / spatium());
            foreach(Element* e, _el) {
                  if (e->type() == TEXT) {
                        static_cast<Text*>(e)->setModified(true);  // force relayout
                        }
                  }
            }
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void Box::endEditDrag()
      {
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Box::endEdit()
      {
      editMode = false;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Box::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      QRectF r(abbox());
      if (type() == HBOX)
            grip[0].translate(QPointF(r.x() + r.width(), r.y() + r.height() * .5));
      else if (type() == VBOX)
            grip[0].translate(QPointF(r.x() + r.width() * .5, r.y() + r.height()));
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Box::write(Xml& xml) const
      {
      xml.stag(name());
      if (type() == VBOX)
            xml.tag("height", _boxHeight.val());
      else if (type() == HBOX)
            xml.tag("width", _boxWidth.val());
      if (_leftMargin != BOX_MARGIN)
            xml.tag("leftMargin", _leftMargin);
      if (_rightMargin != BOX_MARGIN)
            xml.tag("rightMargin", _rightMargin);
      if (_topMargin != BOX_MARGIN)
            xml.tag("topMargin", _topMargin);
      if (_bottomMargin != BOX_MARGIN)
            xml.tag("bottomMargin", _bottomMargin);
      Element::writeProperties(xml);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(QDomElement e)
      {
//      if (score()->mscVersion() < 119)
            _leftMargin = _rightMargin = _topMargin = _bottomMargin = 0.0;

      double _spatium = spatium();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "height") {
                  double v = val.toDouble();
                  if (score()->mscVersion() <= 100)
                        v /= _spatium;
                  _boxHeight = Spatium(v);
                  }
            else if (tag == "width") {
                  double v = val.toDouble();
                  if (score()->mscVersion() <= 100)
                        v /= _spatium;
                  _boxWidth = Spatium(v);
                  }
            else if (tag == "leftMargin")
                  _leftMargin = val.toDouble();
            else if (tag == "rightMargin")
                  _rightMargin = val.toDouble();
            else if (tag == "topMargin")
                  _topMargin = val.toDouble();
            else if (tag == "bottomMargin")
                  _bottomMargin = val.toDouble();
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->read(e);
                  add(t);
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
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "HBox") {
                  HBox* hb = new HBox(score());
                  hb->read(e);
                  add(hb);
                  }
            else if (tag == "VBox") {
                  VBox* vb = new VBox(score());
                  vb->read(e);
                  add(vb);
                  }
            else if (!Element::readProperties(e))
                  domError(e);
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
                        if (debugMode)
                              printf("warning: layout break already set\n");
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
            double x = vb->leftMargin() * DPMM;
            double y = vb->topMargin() * DPMM;
            double w = point(boxWidth());
            double h = vb->height() - (vb->topMargin() + vb->bottomMargin()) * DPMM;
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
//   acceptDrop
//---------------------------------------------------------

bool Box::acceptDrop(ScoreView*, const QPointF&, int type, int) const
      {
      if (type == LAYOUT_BREAK)
            return true;
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Box::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      switch(e->type()) {
            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (_pageBreak || _lineBreak) {
                        if (
                           (lb->subtype() == LAYOUT_BREAK_PAGE && _pageBreak)
                           || (lb->subtype() == LAYOUT_BREAK_LINE && _lineBreak)
                           || (lb->subtype() == LAYOUT_BREAK_SECTION && _sectionBreak)
                           ) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        foreach(Element* elem, _el) {
                              if (elem->type() == LAYOUT_BREAK) {
                                    score()->undoChangeElement(elem, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // this are system elements
                  lb->setParent(this);
                  score()->undoAddElement(lb);
                  return lb;
                  }
            default:
                  e->setParent(this);
                  score()->select(e, SELECT_SINGLE, 0);
                  score()->undoAddElement(e);
                  return e;
            }
      return 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool HBox::genPropertyMenu(QMenu* popup) const
      {
      QMenu* textMenu = popup->addMenu(tr("Add"));
      QAction* a = getAction("frame-text");
      textMenu->addAction(a);
      a = getAction("picture");
      textMenu->addAction(a);

      a = popup->addAction(tr("Frame Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void HBox::propertyAction(ScoreView* viewer, const QString& cmd)
      {
      if (cmd == "props") {
            BoxProperties vp(this, 0);
            vp.exec();
            }
      else if (cmd == "frame-text") {
            Text* s = new Text(score());
            s->setSubtype(TEXT_FRAME);
            s->setParent(this);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            viewer->startEdit(s);
            score()->setLayoutAll(true);
            }
      else if (cmd == "picture") {
            score()->addImage(this);
            }
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF HBox::drag(const QPointF& pos)
      {
      QPointF delta(pos - startDragPosition());
      QRectF r(abbox());
      qreal diff = delta.x();
      qreal x1   = userOff().x() + diff;
      if (parent()->type() == VBOX) {
            VBox* vb = static_cast<VBox*>(parent());
            qreal x2 = parent()->width() - width() - (vb->leftMargin() + vb->rightMargin()) * DPMM;
            if (x1 < 0.0)
                  x1 = 0.0;
            else if (x1 > x2)
                  x1 = x2;
            }
      setUserOff(QPointF(x1, 0.0));
      setStartDragPosition(pos);
      return abbox() | r;
      }

//---------------------------------------------------------
//   isMovable
//---------------------------------------------------------

bool HBox::isMovable() const
      {
      return parent() && (parent()->type() == HBOX || parent()->type() == VBOX);
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool VBox::genPropertyMenu(QMenu* popup) const
      {
      QMenu* textMenu = popup->addMenu(tr("Add"));

      QAction* a = getAction("frame-text");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("title-text");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("subtitle-text");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("composer-text");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("poet-text");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("insert-hbox");
      a->blockSignals(true);
      textMenu->addAction(a);

      a = getAction("picture");
      textMenu->addAction(a);

      a = popup->addAction(tr("Frame Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void VBox::propertyAction(ScoreView* viewer, const QString& cmd)
      {
      if (cmd == "props") {
            BoxProperties vp(this, 0);
            vp.exec();
            return;
            }
      Element* s = 0;
      if (cmd == "frame-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_FRAME);
            t->setTextStyle(TEXT_STYLE_FRAME);
            s = t;
            }
      else if (cmd == "title-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_TITLE);
            t->setTextStyle(TEXT_STYLE_TITLE);
            s = t;
            }
      else if (cmd == "subtitle-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_SUBTITLE);
            t->setTextStyle(TEXT_STYLE_SUBTITLE);
            s = t;
            }
      else if (cmd == "composer-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_COMPOSER);
            t->setTextStyle(TEXT_STYLE_COMPOSER);
            s = t;
            }
      else if (cmd == "poet-text") {
            Text* t = new Text(score());
            t->setSubtype(TEXT_POET);
            t->setTextStyle(TEXT_STYLE_POET);
            s = t;
            }
      else if (cmd == "insert-hbox") {
            s = new HBox(score());
            double w = width() - leftMargin() * DPMM - rightMargin() * DPMM;
            static_cast<HBox*>(s)->setBoxWidth(Spatium(w / spatium()));
            }
      else if (cmd == "picture")
            score()->addImage(this);
      else {
            printf("VBox::propertyAction: %s unknown\n", qPrintable(cmd));
            return;
            }
      if (s) {
            s->setParent(this);
            score()->undoAddElement(s);
            score()->select(s, SELECT_SINGLE, 0);
            viewer->startEdit(s);
            score()->setLayoutAll(true);
            }
      getAction("title-text")->blockSignals(false);
      getAction("subtitle-text")->blockSignals(false);
      getAction("composer-text")->blockSignals(false);
      getAction("poet-text")->blockSignals(false);
      getAction("insert-hbox")->blockSignals(false);
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
///   The text box layout() adjusts the frame height to text
///   height.
//---------------------------------------------------------

void TBox::layout()
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), point(boxHeight())));
      if (_el.size() == 1) {
            Text* text = static_cast<Text*>(_el[0]);
            if (text->type() != TEXT)
                  return;
            text->layout();
            qreal h;
            if (text->isEmpty()) {
                  QFontMetricsF fm(text->font());
                  h = fm.lineSpacing();
                  }
            else
                  h = text->height();
            setbbox(QRectF(0.0, 0.0, system()->width(), h));
            }
      }

//---------------------------------------------------------
//   add
///   Add new Element \a e to text box
//---------------------------------------------------------

void TBox::add(Element* e)
      {
      e->setParent(this);
      if (e->type() == TEXT) {
            Text* text = static_cast<Text*>(e);
            text->setLayoutToParentWidth(true);
            text->setFlag(ELEMENT_MOVABLE, false);
            }
      else {
            printf("TBox::add: element not allowed\n");
            return;
            }
      _el.append(e);
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

