//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: hairpin.cpp 3592 2010-10-18 17:24:18Z wschweer $
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

#include "hairpin.h"
#include "style.h"
#include "m-al/xml.h"
#include "utils.h"
#include "preferences.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "staff.h"
#include "painter.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(Painter* p) const
      {
      qreal h1 = point(score()->styleS(ST_hairpinHeight)) * .5;
      qreal h2 = point(score()->styleS(ST_hairpinContHeight)) * .5;

      p->setPenWidth(point(score()->styleS(ST_hairpinWidth)));

      qreal x = pos2().x();
      qreal y = pos2().y();

      if (hairpin()->subtype() == 0) {
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        p->drawLine(.0, .0, x, y + h1);
                        p->drawLine(.0, .0, x, y - h1);
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        p->drawLine(.0,  h2, x, y + h1);
                        p->drawLine(.0, -h2, x, y - h1);
                        break;
                  }
            }
      else {
            switch(spannerSegmentType()) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        p->drawLine(.0,  h1, x, y);
                        p->drawLine(.0, -h1, x, y);
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        p->drawLine(.0,  h1, x, y + h2);
                        p->drawLine(.0, -h1, x, y - h2);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF HairpinSegment::bbox() const
      {
      qreal h = point(score()->styleS(ST_hairpinHeight));
      QRectF r(0.0, -h * .5, pos2().x(), h);
      qreal w = point(score()->styleS(ST_hairpinWidth));
      r.adjust(-w*.5, -w*.5, w, w);
      return r;
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
      setPos(0.0, spatium() * _yoffset);
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
//   read
//---------------------------------------------------------

void Hairpin::read(XmlReader* r)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();

      int i = -1;
      while (r->readAttribute()) {
            if (r->tag() == "id")
                  i = r->intValue();
            }
      setId(i);
      while (r->readElement()) {
            if (r->readInt("veloChange", &_veloChange))
                  ;
            else if (r->readInt("dynType", &i))
                  _dynType = DynamicType(i);
            else if (!SLine::readProperties(r))
                  r->unknown();
            }
      }

