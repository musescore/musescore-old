//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

#include "volta.h"
#include "style.h"
#include "layout.h"
#include "system.h"
#include "xml.h"
#include "score.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Volta::draw(QPainter& p)
      {
      qreal voltaLineWidth = _spatium * .18;
      qreal h              = _spatium * 1.8;

      QPointF p0(_p1.x(), h);
      QPointF p3(_p2.x(), h);

      QPen pen(p.pen());
      pen.setWidthF(voltaLineWidth);
      p.setPen(pen);
      p.drawLine(QLineF(p0, _p1));
      p.drawLine(QLineF(_p1, _p2));
      if (subtype() != 4)
            p.drawLine(QLineF(_p2, p3));

      TextStyle* s = score()->textStyle(TEXT_STYLE_TEMPO);
      QFont f(s->family, s->size);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      p.setFont(f);

      QPointF tp(p0.x() + _spatium * .5, p0.y());

      switch(subtype()) {
            default:
            case PRIMA_VOLTA:
                  p.drawText(tp, "1.");
                  break;
            case SECONDA_VOLTA:
            case SECONDA_VOLTA2:
                  p.drawText(tp, "2.");
                  break;
            case TERZA_VOLTA:
                  p.drawText(tp, "3.");
                  break;
            }
      }

//---------------------------------------------------------
//   setLen
//---------------------------------------------------------

void Volta::setLen(qreal l)
      {
      _p2.setX(l);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout(ScoreLayout* layout)
      {
      if (!parent())
            return;
      double _spatium = layout->spatium();
      qreal voltaHeight   = _spatium * 1.8;
      qreal voltaDistance = _spatium * .7;

      Measure* measure = (Measure*)parent();
      System* system   = measure->system();
      SysStaff* sstaff = system->staff(staffIdx());
      qreal y  = sstaff->bbox().top();
      qreal x2 = measure->width() - _spatium * .5;

      _p1.setX(0.0);
      _p1.setY(0.0);
      _p2.setX(x2);
      _p2.setY(0.0);

      setPos(0.0, y - (voltaHeight + voltaDistance));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Volta::bbox() const
      {
      qreal voltaHeight   = _spatium * 1.8;
      return QRectF(0.0, 0.0, _p2.x() - _p1.x(), voltaHeight);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      xml.stag("Volta");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }


