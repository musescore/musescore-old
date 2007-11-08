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

//---------------------------------------------------------
//   HBox
//---------------------------------------------------------

HBox::HBox(Score* score)
   : MeasureBase(score)
      {
      _boxWidth  = 5 * _spatium;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void HBox::write(Xml& xml, int) const
      {
      xml.stag("HBox");
      xml.tag("width", _boxWidth);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void HBox::read(QDomElement e)
      {
      int curTickPos = e.attribute("tick", "0").toInt();
      setTick(curTickPos);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "width")
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
//   draw
//---------------------------------------------------------

void HBox::draw(QPainter& p) const
      {
      if (selected()) {
            p.drawRect(bbox());
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool HBox::startEdit(const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool HBox::edit(int, QKeyEvent*)
      {
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void HBox::editDrag(int, const QPointF&, const QPointF&)
      {
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void HBox::endEditDrag()
      {
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void HBox::endEdit()
      {
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void HBox::updateGrips(int* grips, QRectF*) const
      {
      *grips = 1;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF HBox::gripAnchor(int) const
      {
      QRectF r(abbox());
      return QPointF(r.x() + r.width() * .5, r.y() + r.height());
      }

//---------------------------------------------------------
//   VBox
//---------------------------------------------------------

VBox::VBox(Score* score)
   : MeasureBase(score)
      {
      _boxHeight = 8 * _spatium;
      editMode = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void VBox::write(Xml& xml, int) const
      {
      xml.stag("VBox");
      xml.tag("height", _boxHeight);
      foreach (const Element* el, _el)
            el->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void VBox::read(QDomElement e)
      {
      int curTickPos = e.attribute("tick", "0").toInt();
      setTick(curTickPos);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "height")
                  _boxHeight = val.toDouble();
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
//   draw
//---------------------------------------------------------

void VBox::draw(QPainter& p) const
      {
      if (selected() || editMode) {
            p.drawRect(bbox());
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void VBox::layout(ScoreLayout* layout)
      {
      foreach (Element* el, _el)
            el->layout(layout);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool VBox::startEdit(const QPointF&)
      {
      editMode = true;
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool VBox::edit(int, QKeyEvent*)
      {
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void VBox::editDrag(int, const QPointF&, const QPointF& d)
      {
      _boxHeight += d.y();
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void VBox::endEditDrag()
      {
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void VBox::endEdit()
      {
      editMode = false;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void VBox::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 1;
      QRectF r(abbox());
      grip[0].translate(QPointF(r.x() + r.width() * .5, r.y() + r.height()));
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF VBox::gripAnchor(int) const
      {
      return QPointF();
      }


