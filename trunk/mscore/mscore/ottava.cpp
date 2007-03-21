//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: ottava.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "layout.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void OttavaSegment::draw(QPainter& p)
      {
      qreal ottavaLineWidth    = _spatium * .18;
      qreal ottavaTextDistance = _spatium * .5;

      QPointF pp1(.0, .0);
      QPointF pp2(_p2);

      QFont f(textStyles[TEXT_STYLE_DYNAMICS].font());
      p.setFont(f);
      QFontMetricsF fm(f);
      QRectF bb(fm.boundingRect(ottava()->text()));

      qreal h = textHeight;   // bb.height() * .5;
      p.drawText(QPointF(0.0, h), ottava()->text());
      pp1 += QPointF(bb.width() + ottavaTextDistance, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(ottavaLineWidth);
      pen.setStyle(Qt::DashLine);
      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));
      p.drawLine(QLineF(pp2, QPointF(pp2.x(), h)));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF OttavaSegment::bbox() const
      {
      double h = point(style->hairpinHeight);
      QRectF r(.0, -h * .5, _p2.x(), h);
      if (mode) {
            r |= r1;
            r |= r2;
            }
      return r;
      }

//---------------------------------------------------------
//   Ottava
//---------------------------------------------------------

Ottava::Ottava(Score* s)
   : SLine(s)
      {
      setSubtype(0);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Ottava::setSubtype(int val)
      {
      Element::setSubtype(val);
      switch(val) {
            case 0:
                  _text = "8va";
                  break;
            case 1:
                  _text = "15va";
                  break;
            case 2:
                  _text = "8vb";
                  break;
            case 3:
                  _text = "15vb";
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout(ScoreLayout* layout)
      {
#if 0
      double _spatium = layout->spatium();
      SLine::layout(layout);

      qreal ottavaDistance = _spatium * 2.5;
      qreal y = 0.0;
      if (parent()) {
            Measure* measure = (Measure*)parent();
            System* system   = measure->system();
            SysStaff* sstaff = system->staff(staffIdx());
            y = sstaff->bbox().top() - ottavaDistance;
            }

      setPos(ipos().x(), y);

      QFontMetricsF fm(textStyles[TEXT_STYLE_DYNAMICS].fontMetrics());
      qreal h1 = 0.0;
      int n = text.size();
      for (int i = 0; i < n; ++i) {
            qreal h = fm.boundingRect(text[i]).height();
            if (h > h1)
                  h1 = h;
            }
      h1 *= .5;
      textHeight = h1;

      QRectF r(0, 0, 0, 0);
      for (iLineSegment i = segments.begin(); i != segments.end(); ++i) {
            LineSegment* s = &*i;
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
      setbbox(r);
#endif
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag("Ottava");
      SLine::writeProperties(xml);
      xml.etag("Ottava");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!SLine::readProperties(node))
                  domError(node);
            }
      }

//---------------------------------------------------------
//   createSegment
//---------------------------------------------------------

LineSegment* Ottava::createSegment()
      {
      LineSegment* seg = new OttavaSegment(score());
      seg->setParent(this);
      seg->setStaff(staff());
      return seg;
      }


