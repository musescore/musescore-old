//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: hairpin.cpp,v 1.18 2006/03/28 14:58:58 wschweer Exp $
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
#include "xml.h"
#include "utils.h"
#include "preferences.h"
#include "score.h"
#include "measure.h"
#include "segment.h"
#include "system.h"
#include "layout.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter& p) const
      {
      double h1 = point(score()->style()->hairpinHeight) * .5;
      double h2 = point(score()->style()->hairpinContHeight) * .5;

      QPen pen(p.pen());
      pen.setWidthF(score()->style()->hairpinWidth.point());
      p.setPen(pen);

      qreal x = pos2().x();
      qreal y = pos2().y();

      if (hairpin()->subtype() == 0) {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        p.drawLine(QLineF(.0, .0, x, y + h1));
                        p.drawLine(QLineF(.0, .0, x, y - h1));
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h2, x, y + h1));
                        p.drawLine(QLineF(.0, -h2, x, y - h1));
                        break;
                  }
            }
      else {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h1, x, y));
                        p.drawLine(QLineF(.0, -h1, x, y));
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        p.drawLine(QLineF(.0,  h1, x, y + h2));
                        p.drawLine(QLineF(.0, -h1, x, y - h2));
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF HairpinSegment::bbox() const
      {
      double h = point(score()->style()->hairpinHeight);
      QRectF r(0.0, -h * .5, pos2().x(), h);
      double w = score()->style()->hairpinWidth.point();
      r.adjust(-w*.5, -w*.5, w, w);
      return r;
      }

//---------------------------------------------------------
//   Hairpin
//---------------------------------------------------------

Hairpin::Hairpin(Score* s)
   : SLine(s)
      {
      setAnchor(ANCHOR_SEGMENT);
      setOffsetType(OFFSET_SPATIUM);
      setYoff(7.0);
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick() to _tick2
//---------------------------------------------------------

void Hairpin::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      Element::layout(layout);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Hairpin::createLineSegment()
      {
      return new HairpinSegment(score());
      }

