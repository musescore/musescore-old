//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: hairpin.cpp,v 1.18 2006/03/28 14:58:58 wschweer Exp $
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

#include "hairpin.h"
#include "style.h"
#include "xml.h"
#include "utils.h"
#include "preferences.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Hairpin::write(Xml& xml) const
      {
      xml.stag("HairPin");
      SLine::writeProperties(xml);
      xml.etag("HairPin");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Hairpin::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!SLine::readProperties(node))
                  domError(node);
            }
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Hairpin::setSubtype(int st)
      {
      Element::setSubtype(st);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Hairpin::bbox() const
      {
      double h1 = point(style->hairpinHeight) * .5;
//      double h2 = point(style->hairpinContHeight) * .5;

      QRectF r(0, 0, 0, 0);
      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            LineSegment* s = (LineSegment*)(&*i);
            ciLineSegment ii = i;
            ++ii;
            QPointF pp1(s->p1);
            QPointF pp2(s->p2);

            if (i == segments.begin())
                  pp1 += off1 * _spatium;
            if (ii == segments.end())
                  pp2 += off2 * _spatium;

            s->bbox.setCoords(pp1.x(), pp1.y() - h1, pp2.x(), pp2.y() + h1);
            r |= s->bbox;
            }

      if (mode != NORMAL) {
            r |= bbr1;
            r |= bbr2;
            }

      qreal lw = ::style->hairpinWidth.point() * .5;
      r.adjust(-lw, -lw, lw, lw);
      return r;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Hairpin::draw(QPainter& p)
      {
      double h1 = point(style->hairpinHeight) * .5;
      double h2 = point(style->hairpinContHeight) * .5;

      QPen pen(p.pen());
      pen.setWidthF(::style->hairpinWidth.point());
      p.setPen(pen);

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

            if (i == segments.begin() && subtype() == 0) {
                  p.drawLine(QLineF(pp1.x(), pp1.y(), pp2.x(), pp2.y() + h1));
                  p.drawLine(QLineF(pp1.x(), pp1.y(), pp2.x(), pp2.y() - h1));
                  }
            else if (ii == segments.end() && subtype() == 1) {
                  p.drawLine(QLineF(pp1.x(), pp1.y() + h1, pp2.x(), pp2.y()));
                  p.drawLine(QLineF(pp1.x(), pp1.y() - h1, pp2.x(), pp2.y()));
                  }
            else if (subtype() == 1) {
                  p.drawLine(QLineF(pp1.x(), pp1.y()+h1, pp2.x(), pp2.y()+h2));
                  p.drawLine(QLineF(pp1.x(), pp1.y()-h1, pp2.x(), pp2.y()-h2));
                  }
            else {
                  p.drawLine(QLineF(pp1.x(), pp1.y()+h2, pp2.x(), pp2.y()+h1));
                  p.drawLine(QLineF(pp1.x(), pp1.y()-h2, pp2.x(), pp2.y()-h1));
                  }
            }

      if (mode != NORMAL) {
            qreal lw = 2.0/p.matrix().m11();
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
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void Hairpin::layout()
      {
      SLine::layout();
      setPos(0, _spatium * 6);
      }

//---------------------------------------------------------
//   setLen
//    used to create a Hairpin object suitable for palette
//---------------------------------------------------------

void Hairpin::setLen(double l)
      {
      LineSegment hps;
      hps.p1 = QPointF(0, 0);
      hps.p2 = QPointF(l, 0);
      segments.push_back(hps);
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      }
