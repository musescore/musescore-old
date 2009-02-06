//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.cpp,v 1.1 2006/03/27 14:16:24 wschweer Exp $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "spacer.h"
#include "preferences.h"
#include "score.h"
#include "canvas.h"
#include "layout.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      _space = Spatium(0);
      }

Spacer::Spacer(const Spacer& s)
   : Element(s)
      {
      _space = s._space;
      path   = s.path;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(QPainter& p) const
      {
      if (score()->printing())
            return;
      QPen pen;
      if (selected())
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(preferences.layoutBreakColor);

      pen.setWidthF(_spatium * 0.4);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Spacer::layout(ScoreLayout*)
      {
      path = QPainterPath();
      double a = _spatium;
      double b = _spatium * .5;
      double h = _space.point();

      path.lineTo(a, 0.0);
      path.moveTo(b, 0.0);
      path.lineTo(b, h);
      path.moveTo(0.0, h);
      path.lineTo(a, h);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Spacer::acceptDrop(Viewer*, const QPointF&, int, int) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Spacer::drop(const QPointF&, const QPointF&, Element* e)
      {
      return e;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Spacer::startEdit(Viewer*, const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(int, const QPointF& delta)
      {
      _space += delta.y();
      if (_space.val() < 2.0)
            _space = Spatium(2.0);
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Spacer::updateGrips(int* grips, QRectF* grip) const
      {
      *grips   = 1;
      QPointF p(_spatium * .5, point(_space));
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Spacer::bbox() const
      {
      return QRectF(-_spatium * .2, -_spatium * .2, _spatium * 1.4, _space.point() + 0.4 * _spatium);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Spacer::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.tag("space", _space.val());
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "space")
                  _space = Spatium(val.toDouble());
            else if (Element::readProperties(e))
                  domError(e);
            }
      }


