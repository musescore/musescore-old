//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: measure.h,v 1.40 2006/04/12 14:58:10 wschweer Exp $
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
#include "viewer.h"
#include "canvas.h"
#include "boxproperties.h"
#include "symbol.h"
#include "system.h"

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      editMode      = false;
      _boxWidth     = Spatium(5.0);
      _boxHeight    = Spatium(10.0);
      _leftMargin   = 0.0;
      _rightMargin  = 0.0;
      _topMargin    = 0.0;
      _bottomMargin = 0.0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Box::layout(ScoreLayout* layout)
      {
      foreach (Element* el, _el)
            el->layout(layout);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Box::draw(QPainter& p) const
      {
      if (selected() || editMode || dropTarget()) {
            QPen pen(QColor(Qt::blue));
            pen.setWidthF(2.0 / p.matrix().m11());
            p.setPen(pen);
            p.drawRect(bbox());
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Box::startEdit(const QPointF&)
      {
      editMode = true;
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Box::edit(int, QKeyEvent*)
      {
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Box::editDrag(int, const QPointF&, const QPointF& d)
      {
      if (type() == VBOX)
            _boxHeight += d.y();
      else
            _boxWidth += d.x();
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
      if (_leftMargin != 0.0)
            xml.tag("leftMargin", _leftMargin);
      if (_rightMargin != 0.0)
            xml.tag("rightMargin", _rightMargin);
      if (_topMargin != 0.0)
            xml.tag("topMargin", _topMargin);
      if (_bottomMargin != 0.0)
            xml.tag("bottomMargin", _bottomMargin);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(QDomElement e)
      {
      score()->curTick = tick();

      int curTickPos = 0;
      int ct = e.attribute("tick", "-1").toInt();
      if (ct >= 0) {
            curTickPos = ct;
            setTick(curTickPos);
            }
      score()->curTick = tick();

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
                  t->setTick(curTickPos);
                  t->read(e);
                  add(t);
                  }
            else if (tag == "Symbol") {
                  Symbol* s = new Symbol(score());
                  s->setTick(curTickPos);
                  s->read(e);
                  add(s);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(Score* score)
   : Box(score)
      {
      }

HBox::~HBox()
      {
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void HBox::collectElements(QList<const Element*>& el) const
      {
      MeasureBase::collectElements(el);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HBox::layout(ScoreLayout* layout)
      {
      setbbox(QRectF(0.0, 0.0, boxWidth().point(), system()->height()));
      Box::layout(layout);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool HBox::acceptDrop(Viewer*, const QPointF&, int, int) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* HBox::drop(const QPointF&, const QPointF&, Element* e)
      {
      e->setParent(this);
      score()->select(e, 0, 0);
      score()->cmdAdd(e);
      return e;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool HBox::genPropertyMenu(QMenu* popup) const
      {
      QMenu* textMenu = popup->addMenu(tr("Add Text"));
      QAction* a = getAction("frame-text");
      textMenu->addAction(a);
      a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void HBox::propertyAction(const QString& cmd)
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
            score()->select(s, 0, 0);
            score()->canvas()->startEdit(s);
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool VBox::genPropertyMenu(QMenu* popup) const
      {
      QMenu* textMenu = popup->addMenu(tr("Add"));

      QAction* a = getAction("title-text");
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

      a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void VBox::propertyAction(const QString& cmd)
      {
      if (cmd == "props") {
            BoxProperties vp(this, 0);
            vp.exec();
            return;
            }
      Element* s = 0;
      if (cmd == "title-text") {
            s = new Text(score());
            s->setSubtype(TEXT_TITLE);
            }
      else if (cmd == "subtitle-text") {
            s = new Text(score());
            s->setSubtype(TEXT_SUBTITLE);
            }
      else if (cmd == "composer-text") {
            s = new Text(score());
            s->setSubtype(TEXT_COMPOSER);
            }
      else if (cmd == "poet-text") {
            s = new Text(score());
            s->setSubtype(TEXT_POET);
            }
      else if (cmd == "insert-hbox") {
            s = new HBox(score());
            }
      else {
            printf("VBox::propertyAction: %s unknown\n", qPrintable(cmd));
            return;
            }
      if (s) {
            s->setParent(this);
            score()->undoAddElement(s);
            score()->select(s, 0, 0);
            if (s->type() == TEXT)
                  score()->canvas()->startEdit(s);
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

void VBox::layout(ScoreLayout* layout)
      {
      setPos(QPointF());      // !?
      setbbox(QRectF(0.0, 0.0, system()->width(), boxHeight().point()));
      Box::layout(layout);
      int n    = _hboxList.size();
      double w = (system()->width() - 2.0 * leftMargin() * DPMM) / n;
      for (int i = 0; i < _hboxList.size(); ++i) {
            HBox* hb = _hboxList[i];
            hb->setPos(leftMargin() * DPMM + (i * w), topMargin() * DPMM);
            hb->setbbox(QRectF(0.0, 0.0, w,
               boxHeight().point() - (topMargin() + bottomMargin()) * DPMM));
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void VBox::add(Element* e)
      {
      if (e->type() == HBOX) {
            e->setParent(this);
            _hboxList.append((HBox*)e);
            }
      else
            MeasureBase::add(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void VBox::remove(Element* e)
      {
      if (e->type() == HBOX) {
            int idx = _hboxList.indexOf((HBox*)e);
            if (idx == -1)
                  printf("VBox(%p)::remove(%s,%p) not found\n", this, e->name(), e);
            _hboxList.removeAt(idx);
            }
      else
            MeasureBase::remove(e);
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void VBox::collectElements(QList<const Element*>& el) const
      {
      foreach(Element* e, _hboxList)
            e->collectElements(el);
      MeasureBase::collectElements(el);
      }


