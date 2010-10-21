//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "spanner.h"

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_SEGMENT);
      setSubtype(SEGMENT_SINGLE);
      _system = 0;
      }

SpannerSegment::SpannerSegment(const SpannerSegment& s)
   : Element(s)
      {
      _system = s._system;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SpannerSegment::startEdit(ScoreView*s , const QPointF& p)
      {
      parent()->startEdit(s, p);
      }

//---------------------------------------------------------
//   Spanner
//---------------------------------------------------------

Spanner::Spanner(Score* s)
   : Element(s)
      {
      _startElement = 0;
      _endElement   = 0;
      _anchor       = ANCHOR_SEGMENT;
      }

Spanner::Spanner(const Spanner& s)
   : Element(s)
      {
      _startElement = s._startElement;
      _endElement   = s._endElement;
      _anchor       = s._anchor;
      _yoffset      = s._yoffset;
      foreach(SpannerSegment* ss, s.segments)
            add(ss->clone());
      }

Spanner::~Spanner()
      {
      foreach(SpannerSegment* ss, spannerSegments())
            delete ss;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Spanner::add(Element* e)
      {
      SpannerSegment* ls = static_cast<SpannerSegment*>(e);
      ls->setParent(this);
      segments.append(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(Element* e)
      {
      segments.removeOne(static_cast<SpannerSegment*>(e));
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Spanner::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach(SpannerSegment* seg, segments)
            seg->scanElements(data, func);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Spanner::startEdit(ScoreView*, const QPointF&)
      {
      oStartElement = _startElement;
      oEndElement   = _endElement;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void Spanner::setSelected(bool f)
      {
      foreach(SpannerSegment* ss, segments)
            ss->setSelected(f);
      Element::setSelected(f);
      }

