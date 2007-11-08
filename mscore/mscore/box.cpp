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
      if (selected() || editMode) {
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
      int curTickPos = e.attribute("tick", "0").toInt();
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
                  t->setTick(score()->curTick);
                  t->read(e);
                  add(t);
                  }
            else
                  domError(e);
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

