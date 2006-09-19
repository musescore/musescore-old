//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: trill.cpp,v 1.6 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "trill.h"
#include "style.h"
#include "painter.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"

static const QChar TC(0xe161);
static const QChar TLC(0xe16f);

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      text = TC;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Trill::setSubtype(int val)
      {
      Element::setSubtype(val);
      }

//---------------------------------------------------------
//   bboxUpdate
//---------------------------------------------------------

void Trill::bboxUpdate()
      {
      QRectF rr(trillSym.bbox());

      QRectF r(0, 0, 0, 0);
      for (iLineSegment i = segments.begin(); i != segments.end(); ++i) {
            LineSegment* s = &*i;
            iLineSegment ii = i;
            ++ii;
            QPointF pp1(s->p1);
            QPointF pp2(s->p2);

            if (i == segments.begin())
                  pp1 += off1 * _spatium;
            if (ii == segments.end())
                  pp2 += off2 * _spatium;

            s->bbox.setRect(pp1.x(), rr.y(), pp2.x() - pp1.x(), rr.height());
            r |= s->bbox;
            }

      if (mode != NORMAL) {
            r |= bbr1;
            r |= bbr2;
            }
      setbbox(r);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Trill::draw1(Painter& p) const
      {
      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            const LineSegment* s = &*i;

            ciLineSegment ii = i;
            ++ii;
            QPointF pp1(s->p1);
            QPointF pp2(s->p2);

            if (i == segments.begin())
                  pp1 += off1 * _spatium;
            if (ii == segments.end())
                  pp2 += off2 * _spatium;

            if (i == segments.begin()) {
                  qreal x  = pp1.x();
                  qreal y  = pp1.y();
                  QRectF b = trillSym.bbox();
                  trillSym.draw(p, x - b.x(), pp1.y());

                  x += b.width();

                  QRectF b1 = trillelementSym.bbox();
                  int n     = lrint((pp2.x() - x) / trillelementSym.width());
                  trillelementSym.draw(p, x + b1.x(), y + b1.y(), n);
                  }
            else {
                  printf("Trill::draw1(): ===not impl. segments %zd\n",
                     segments.size());
                  }
            }

      if (mode != NORMAL) {
            qreal lw = 2.0 / p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            if (mode == DRAG1) {
                  p.setBrush(Qt::blue);
                  p.drawRect(r1);
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r2);
                  }
            else {
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r1);
                  p.setBrush(Qt::blue);
                  p.drawRect(r2);
                  }
            }
      }

//---------------------------------------------------------
//   setLen
//    used for palette operations
//---------------------------------------------------------

void Trill::setLen(qreal l)
      {
      LineSegment hps;
      hps.p1 = QPointF(0, 0);
      hps.p2 = QPointF(l, 0);
      segments.push_back(hps);
      bboxUpdate();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      SLine::layout();
      qreal trillDistance = _spatium * 2.5;

      Measure* measure = (Measure*)parent();
      System* system   = measure->system();
      SysStaff* sstaff = system->staff(staffIdx());

      setPos(0.0, sstaff->bbox().top() - trillDistance);
      bboxUpdate();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Trill::write(Xml& xml) const
      {
      xml.stag("Trill");
      SLine::writeProperties(xml);
      xml.etag("Trill");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Trill::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!SLine::readProperties(node))
                  domError(node);
            }
      }

