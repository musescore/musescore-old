//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pedal.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
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

#include "pedal.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "layout.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void PedalSegment::draw(QPainter& p)
      {
      qreal ottavaLineWidth    = _spatium * .18;
      qreal ottavaTextDistance = _spatium * .5;

      QPointF pp1;
      QPointF pp2(pos2());

      const QRectF& bb = symbols[pedal()->symbol].bbox();
      qreal h = bb.height() * .5;
      symbols[pedal()->symbol].draw(p, pp1.x(), h);

      pp1 += QPointF(bb.width() + ottavaTextDistance, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(ottavaLineWidth);
      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));
      p.drawLine(QLineF(pp2, QPointF(pp2.x(), -h)));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF PedalSegment::bbox() const
      {
      const QRectF& rr = symbols[pedal()->symbol].bbox();
      double h1 = rr.height();

      QPointF pp2(pos2());
      QRectF r(.0, -h1 * .5, pp2.x(), h1);
      return r;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Pedal::layout(ScoreLayout* layout)
      {
      if (!parent())
            return;

      SLine::layout(layout);
#if 0
      if (!parent())
            return;
      qreal pedalDistance = layout->spatium() * 6;

      Measure* measure = (Measure*)parent();
      System* system   = measure->system();
      SysStaff* sstaff = system->staff(staffIdx());
      qreal y = sstaff->bbox().top() + pedalDistance;
      setPos(ipos().x(), y);
#endif
      }

//---------------------------------------------------------
//   Pedal
//---------------------------------------------------------

Pedal::Pedal(Score* s)
   : SLine(s)
      {
      symbol = pedalPedSym;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Pedal::write(Xml& xml) const
      {
      xml.stag("Pedal");
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Pedal::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Pedal::createLineSegment()
      {
      LineSegment* seg = new PedalSegment(score());
      seg->setStaff(staff());
      return seg;
      }
