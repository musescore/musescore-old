//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "hairpin.h"
#include "style.h"
#include "xml.h"
#include "utils.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "undo.h"
#include "staff.h"
#include "mscore.h"

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void HairpinSegment::layout()
      {
      QTransform t;
      qreal _spatium = spatium();
      qreal h1 = score()->styleS(ST_hairpinHeight).val() * _spatium * .5;
      qreal h2 = score()->styleS(ST_hairpinContHeight).val() * _spatium * .5;

      qreal len;
      qreal x = pos2().x();
      if (x < _spatium)             // minimum size of hairpin
            x = _spatium;
      qreal y = pos2().y();
      len     = sqrt(x * x + y * y);
      t.rotateRadians(asin(y/len));

      if (hairpin()->subtype() == 0) {
            // crescendo
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        l1 = QLineF(.0, .0, len, h1);
                        l2 = QLineF(.0, .0, len, - h1);
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        l1 = QLineF(.0,  h2, len, h1);
                        l2 = QLineF(.0, -h2, len, - h1);
                        break;
                  }
            }
      else {
            // decrescendo
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        l1 = QLineF(.0,  h1, len, 0.0);
                        l2 = QLineF(.0, -h1, len, 0.0);
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        l1 = QLineF(.0,  h1, len, + h2);
                        l2 = QLineF(.0, -h1, len, - h2);
                        break;
                  }
            }
      l1 = t.map(l1);
      l2 = t.map(l2);

      QRectF r = QRectF(l1.p1(), l1.p2()).normalized() | QRectF(l2.p1(), l2.p2()).normalized();
      qreal w = point(score()->styleS(ST_hairpinWidth));
      setbbox(r.adjusted(-w*.5, -w*.5, w, w));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter* painter) const
      {
      QPen pen(painter->pen());
      pen.setWidthF(point(score()->styleS(ST_hairpinWidth)));
      painter->setPen(pen);
      painter->drawLine(QLineF(l1.x1(), l1.y1(), l1.x2(), l1.y2()));
      painter->drawLine(QLineF(l2.x1(), l2.y1(), l2.x2(), l2.y2()));
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      setLen(spatium() * 7);   // for use in palettes
      _veloChange = 10;
      _dynType    = DYNAMIC_PART;
      _yoffset    = 8.0;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Hairpin::layout()
      {
      setPos(0.0, 0.0);
      SLine::layout();
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Hairpin::createLineSegment()
      {
      return new HairpinSegment(score());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Hairpin::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      xml.tag("veloChange", _veloChange);
      if (_dynType != DYNAMIC_PART)
            xml.tag("dynType", _dynType);
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Hairpin::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            const QString& tag(e.tagName());
            int val = e.text().toInt();
            if (tag == "veloChange")
                  _veloChange = val;
            else if (e.tagName() == "dynType")
                  _dynType = DynamicType(val);
            else if (!SLine::readProperties(e))
                  domError(e);
            }
      }

