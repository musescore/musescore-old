//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: ottava.cpp,v 1.3 2006/03/28 14:58:58 wschweer Exp $
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

#include "ottava.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "layout.h"
#include "score.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void OttavaSegment::draw(QPainter& p)
      {
      qreal ottavaLineWidth    = _spatium * .18;
      qreal ottavaTextDistance = _spatium * .5;

      QPointF pp2(pos2());

      QFont f(score()->textStyle(TEXT_STYLE_DYNAMICS)->font());
      p.setFont(f);
      QFontMetricsF fm(f);
      QString txt;
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN)
            txt = ottava()->text;
      else
            txt = QString("(%1)").arg(ottava()->text);
      QRectF bb(fm.boundingRect(txt));
      qreal h = ottava()->textHeight;
      p.drawText(QPointF(0.0, h), txt);
      QPointF pp1(bb.width() + ottavaTextDistance, 0.0);

      QPen pen(p.pen());
      pen.setWidthF(ottavaLineWidth);
      pen.setStyle(Qt::DashLine);
      QVector<qreal> dashes;
      dashes << _spatium * .5 << _spatium * .5;
      pen.setDashPattern(dashes);

      p.setPen(pen);
      p.drawLine(QLineF(pp1, pp2));
      if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END)
            p.drawLine(QLineF(pp2, QPointF(pp2.x(), h)));
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF OttavaSegment::bbox() const
      {
      QPointF pp1;
      QPointF pp2(pos2());

      qreal h1 = ottava()->textHeight;
      QRectF r(.0, -h1, pp2.x(), 2 * h1);
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
                  text = "8va";
                  _pitchShift = 12;
                  break;
            case 1:
                  text = "15va";
                  _pitchShift = 24;
                  break;
            case 2:
                  text = "8vb";
                  _pitchShift = -12;
                  break;
            case 3:
                  text = "15vb";
                  _pitchShift = -24;
                  break;
            }
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Ottava::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);

      qreal y = -3.0 * layout->spatium();
      setPos(ipos().x(), y);

      QFontMetricsF fm(score()->textStyle(TEXT_STYLE_DYNAMICS)->fontMetrics());
      qreal h1 = 0.0;
      int n = text.size();
      for (int i = 0; i < n; ++i) {
            qreal h = fm.boundingRect(text[i]).height();
            if (h > h1)
                  h1 = h;
            }
      textHeight = h1 * .5;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Ottava::write(Xml& xml) const
      {
      xml.stag("Ottava");
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Ottava::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Ottava::createLineSegment()
      {
      LineSegment* seg = new OttavaSegment(score());
      return seg;
      }
