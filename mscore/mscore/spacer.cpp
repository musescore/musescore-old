//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "scoreview.h"

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

void Spacer::draw(QPainter& p, ScoreView*) const
      {
      if (score()->printing())
            return;
      QPen pen;
      if (selected())
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(preferences.layoutBreakColor);

      pen.setWidthF(spatium() * 0.4);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Spacer::layout()
      {
      double _spatium = spatium();

      path = QPainterPath();
      double a = _spatium;
      double b = _spatium * .5;
      double h = _space.val() * _spatium;

      path.lineTo(a, 0.0);
      path.moveTo(b, 0.0);
      path.lineTo(b, h);
      path.moveTo(0.0, h);
      path.lineTo(a, h);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool Spacer::acceptDrop(ScoreView*, const QPointF&, int, int) const
      {
      return false;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

Element* Spacer::drop(ScoreView*, const QPointF&, const QPointF&, Element* e)
      {
      return e;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Spacer::editDrag(int, const QPointF& delta)
      {
      _space += Spatium(delta.y() / spatium());
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
      double _spatium = spatium();
      QPointF p(_spatium * .5, _space.val() * _spatium);
      grip[0].translate(canvasPos() + p);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Spacer::bbox() const
      {
      double _spatium = spatium();
      return QRectF(-_spatium * .2, -_spatium * .2, _spatium * 1.4, (_space.val() + .4) * _spatium);
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


