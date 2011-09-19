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
#include "system.h"

//---------------------------------------------------------
//   SpannerSegment
//---------------------------------------------------------

SpannerSegment::SpannerSegment(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_SEGMENT);
      setSubtype(SEGMENT_SINGLE);
      _spanner = 0;
      }

SpannerSegment::SpannerSegment(const SpannerSegment& s)
   : Element(s)
      {
      _spanner = s._spanner;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void SpannerSegment::startEdit(MuseScoreView*s , const QPointF& p)
      {
      spanner()->startEdit(s, p);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SpannerSegment::endEdit()
      {
      spanner()->endEdit();
      }

//---------------------------------------------------------
//   setSystem
//---------------------------------------------------------

void SpannerSegment::setSystem(System* s)
      {
      if (system() != s) {
            if (system())
                  system()->remove(this);
            }
      s->add(this);
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
      foreach(SpannerSegment* ss, s.segments) {
            SpannerSegment* nss = ss->clone();
            add(nss);
            }
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
      ls->setSpanner(this);
      segments.append(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Spanner::remove(Element* e)
      {
      SpannerSegment* ss = static_cast<SpannerSegment*>(e);
      if (ss->system())
            ss->system()->remove(ss);
      segments.removeOne(ss);
      }

//---------------------------------------------------------
//   scanElements
//    used for palettes
//---------------------------------------------------------

void Spanner::scanElements(void* data, void (*func)(void*, Element*), bool all)
      {
      Q_UNUSED(all)
      foreach(SpannerSegment* seg, segments)
            seg->scanElements(data, func, true);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Spanner::setScore(Score* s)
      {
      Element::setScore(s);
      foreach(SpannerSegment* seg, segments)
            seg->setScore(s);
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

//---------------------------------------------------------
//   isEdited
//    compare edited spanner with origSpanner
//---------------------------------------------------------

bool Spanner::isEdited(Spanner* originalSpanner) const
      {
      if (startElement() != originalSpanner->startElement()
         || endElement() != originalSpanner->endElement()) {
            return true;
            }
      if (spannerSegments().size() != originalSpanner->spannerSegments().size())
            return true;
      for (int i = 0; i < segments.size(); ++i) {
            if (segments[i]->isEdited(originalSpanner->segments[i]))
                  return true;
            }
      return false;
      }
