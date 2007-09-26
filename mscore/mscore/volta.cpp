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

void VoltaSegment::draw(QPainter& p)
      {
      QPointF _p1;
      QPointF _p2(pos2());

      qreal voltaLineWidth = _spatium * .18;
      qreal h              = _spatium * 1.9;

      QPointF p0(_p1.x(), h);
      QPointF p3(_p2.x(), h);

      QPen pen(p.pen());
      pen.setWidthF(voltaLineWidth);
      p.setPen(pen);
      p.drawLine(QLineF(p0, _p1));
      p.drawLine(QLineF(_p1, _p2));
      if (subtype() != 4)
            p.drawLine(QLineF(_p2, p3));

      TextStyle* s = score()->textStyle(TEXT_STYLE_VOLTA);
      QFont f(s->family, s->size);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      p.setFont(f);

      QPointF tp(p0.x() + _spatium * .5, p0.y());

      switch(volta()->subtype()) {
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
//   bbox
//---------------------------------------------------------

QRectF VoltaSegment::bbox() const
      {
      qreal voltaHeight   = _spatium * 1.8;
      return QRectF(0.0, 0.0, pos2().x(), voltaHeight);
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF VoltaSegment::gripAnchor(int curGrip) const
      {
      int tick;
      QPointF anchor;

      if (curGrip == 0) {
            QPointF pp1(canvasPos());
            Measure* m = score()->pos2measure3(pp1, &tick);
            anchor = QPointF(m->abbox().topLeft());
            }
      else {
            QPointF pp2(_p2 + _userOff2 * _spatium + canvasPos());
            Measure* m = score()->pos2measure3(pp2, &tick);
            if (tick != m->tick())
                  anchor = QPointF(m->abbox().topRight());
            else
                  anchor = QPointF(m->abbox().topLeft());
            }
      return anchor;
      }

//---------------------------------------------------------
//   pos2anchor
//---------------------------------------------------------

QPointF VoltaSegment::pos2anchor(const QPointF& pos, int* tick) const
      {
      Measure* m = score()->pos2measure3(pos, tick);
      QPointF anchor;
      if (*tick != m->tick())
            anchor = QPointF(m->abbox().topRight());
      else
            anchor = QPointF(m->abbox().topLeft());
      return anchor;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Volta::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Volta::write(Xml& xml) const
      {
      xml.stag("Volta");
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Volta::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   createSegment
//---------------------------------------------------------

LineSegment* Volta::createSegment()
      {
      VoltaSegment* seg = new VoltaSegment(score());
      seg->setParent(this);
      seg->setStaff(staff());
      return seg;
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

QPointF Volta::tick2pos(int tick, System** system)
      {
      Measure* m = score()->tick2measure(tick);
      *system = m->system();
      return m->canvasPos();
      }


