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
#include "layout.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void HairpinSegment::draw(QPainter& p)
      {
      double h1 = point(style->hairpinHeight) * .5;
      double h2 = point(style->hairpinContHeight) * .5;

      QPen pen(p.pen());
      pen.setWidthF(::style->hairpinWidth.point());
      p.setPen(pen);

// printf("Draw %p %f\n", this, pos().x());

      QPointF pp2(pos2());

      if (hairpin()->subtype() == 0) {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        p.drawLine(QLineF(.0, .0, pp2.x(), pp2.y() + h1));
                        p.drawLine(QLineF(.0, .0, pp2.x(), pp2.y() - h1));
                        break;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h2, pp2.x(), pp2.y() + h1));
                        p.drawLine(QLineF(.0, -h2, pp2.x(), pp2.y() - h1));
                        break;
                  }
            }
      else {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        p.drawLine(QLineF(.0,  h1, pp2.x(), pp2.y()));
                        p.drawLine(QLineF(.0, -h1, pp2.x(), pp2.y()));
                        break;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        p.drawLine(QLineF(.0,  h1, pp2.x(), pp2.y() + h2));
                        p.drawLine(QLineF(.0, -h1, pp2.x(), pp2.y() - h2));
                        break;
                  }
            }
      LineSegment::draw(p);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF HairpinSegment::bbox() const
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
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void Hairpin::layout(ScoreLayout* layout)
      {
      SLine::layout(layout);
      }

//---------------------------------------------------------
//   createSegment
//---------------------------------------------------------

LineSegment* Hairpin::createSegment()
      {
      LineSegment* seg = new HairpinSegment(score());
      seg->setParent(this);
      seg->setStaff(staff());
      return seg;
      }

