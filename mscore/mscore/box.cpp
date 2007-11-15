//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: measure.h,v 1.40 2006/04/12 14:58:10 wschweer Exp $
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

#include "box.h"
#include "text.h"
#include "score.h"
#include "mscore.h"
#include "barline.h"
#include "repeat.h"
#include "viewer.h"

//---------------------------------------------------------
//   Box
//---------------------------------------------------------

Box::Box(Score* score)
   : MeasureBase(score)
      {
      editMode   = false;
      _boxWidth  = 5 * _spatium;
      _boxHeight = 10 * _spatium;
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
            _boxWidth += d.y();
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

void Box::write(Xml& xml, int) const
      {
      xml.stag(name());
      if (type() == VBOX)
            xml.tag("height", _boxHeight);
      else if (type() == HBOX)
            xml.tag("width", _boxWidth);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Box::read(QDomElement e)
      {
      int curTickPos = e.attribute("tick", QString("%1").arg(score()->curTick)).toInt();
      setTick(curTickPos);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "height")
                  _boxHeight = val.toDouble();
            else if (tag == "width")
                  _boxWidth = val.toDouble();
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->setTick(curTickPos);
                  t->read(e);
                  add(t);
                  }
            else if (tag == "Repeat") {
                  Repeat* repeat = new Repeat(score());
                  repeat->setTick(curTickPos);
                  repeat->read(e);
                  add(repeat);
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

void HBox::layout(ScoreLayout*)
      {
      setbbox(QRectF(0.0, 0.0, boxWidth(), system()->height()));
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool HBox::acceptDrop(Viewer* viewer, const QPointF&, int type, int subtype) const
      {
      if (type == REPEAT &&
         (subtype == RepeatCoda || subtype == RepeatCodetta || subtype == RepeatVarcoda)
         ) {
            viewer->setDropTarget(this);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* HBox::drop(const QPointF&, const QPointF&, Element* e)
      {
      printf("HBox::drop %s\n", e->name());
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
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void HBox::propertyAction(const QString& cmd)
      {
      printf("VBox:propertyAction <%s>\n", qPrintable(cmd));

      if (cmd == "frame-text") {
            Text* s = new Text(score());
            s->setSubtype(TEXT_FRAME);
            s->setParent(this);
            s->setText("frame");          // debug
            score()->undoAddElement(s);
            score()->setLayoutAll(true);
            score()->select(s, 0, 0);
            }
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool VBox::genPropertyMenu(QMenu* popup) const
      {
      QMenu* textMenu = popup->addMenu(tr("Add Text"));
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
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void VBox::propertyAction(const QString& cmd)
      {
      printf("VBox:propertyAction <%s>\n", qPrintable(cmd));

      if (cmd == "title-text") {
            Text* s = new Text(score());
            s->setSubtype(TEXT_TITLE);
            s->setParent(this);
            s->setText("Title");
            score()->undoAddElement(s);
            score()->setLayoutAll(true);
            score()->select(s, 0, 0);
            }
      getAction("title-text")->blockSignals(false);
      getAction("subtitle-text")->blockSignals(false);
      getAction("composer-text")->blockSignals(false);
      getAction("poet-text")->blockSignals(false);
      }

