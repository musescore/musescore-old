//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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

void SpannerSegment::startEdit(MuseScoreView*s , const QPointF& p)
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
      _yoffset      = 0.0;
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

void Spanner::startEdit(MuseScoreView*, const QPointF&)
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

