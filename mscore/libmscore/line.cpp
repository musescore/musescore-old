//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: line.cpp 3678 2010-11-05 13:33:01Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "line.h"
#include "textline.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "system.h"
#include "utils.h"
#include "al/xml.h"

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(Score* s)
   : SpannerSegment(s)
      {
      }

LineSegment::LineSegment(const LineSegment& s)
   : SpannerSegment(s)
      {
      _p2       = s._p2;
      _userOff2 = s._userOff2;
      r1        = s.r1;
      r2        = s.r2;
      }

//---------------------------------------------------------
//   canvasPos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF LineSegment::canvasPos() const
      {
      return pos();
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Spanner(s)
      {
      _diagonal = false;
      setTrack(0);
      }

SLine::SLine(const SLine& s)
   : Spanner(s)
      {
      _diagonal = s._diagonal;
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

QPointF SLine::tick2pos(int grip, System** sys)
      {
      Segment* seg = static_cast<Segment*>(grip == 0 ? startElement() : endElement());
      Measure* m   = seg->measure();
      *sys         = m->system();

      qreal x = seg->canvasPos().x();

      if (anchor() == ANCHOR_SEGMENT) {
            if ((grip == 1)
               && ((*sys)->firstMeasure() == m)
               && (seg->tick() == m->tick())) {
                  m = m->prevMeasure();
                  if (m) {
                        *sys = m->system();
                        x = m->abbox().right();
                        }
                  }
            }
      else {
            // anchor() == MEASURE
            x = m->canvasPos().x();
            if (m->tick() < seg->tick()) {      // to end of last measure?
                  x += m->bbox().width();
                  }
            else if (grip == 1 && (*sys)->firstMeasure() == m) {
                  m = m->prevMeasure();
                  if (m) {
                        *sys = m->system();
                        x    += m->bbox().width();
                        }
                  }
            }
      qreal y = (*sys)->staffY(staffIdx());
      return QPointF(x, y);
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (parent() == 0) {
            //
            // when used in a palette, SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            if (!spannerSegments().isEmpty()) {
                  LineSegment* s = frontSegment();
                  s->layout();
                  setbbox(s->bbox());
                  }
            return;
            }

      System* s1;
      System* s2;
      QPointF p1 = tick2pos(0, &s1);
      QPointF p2 = tick2pos(1, &s2);

      QList<System*>* systems = score()->systems();
      int sysIdx1 = systems->indexOf(s1);
      int sysIdx2 = systems->indexOf(s2);
      int segmentsNeeded = 0;
      for (int i = sysIdx1; i < sysIdx2+1;  ++i) {
            if (systems->at(i)->isVbox())
                  continue;
            ++segmentsNeeded;
            }
      int segCount = spannerSegments().size();

      if (segmentsNeeded != segCount) {
            if (segmentsNeeded > segCount) {
                  int n = segmentsNeeded - segCount;
                  for (int i = 0; i < n; ++i) {
                        LineSegment* ls = createLineSegment();
                        add(ls);
                        // set user offset to previous segment's offset
                        if (segCount > 0)
                              ls->setUserOff(QPointF(0, segmentAt(segCount+i-1)->userOff().y()));
                        }
                  }
            else {
                  int n = segCount - segmentsNeeded;
                  printf("SLine: segments %d needed %d, remove %d\n", segCount, segmentsNeeded, n);
                  for (int i = 0; i < n; ++i) {
                        if (spannerSegments().isEmpty()) {
                              printf("SLine::layout(): no segment %d, %d expected\n", i, n);
                              break;
                              }
                        else {
                              LineSegment* seg = takeLastSegment();
                              delete seg;
                              }
                        }
                  }
            }

      int segIdx = 0;
      int si  = staffIdx();
      for (int i = sysIdx1; i <= sysIdx2; ++i) {
            System* system = systems->at(i);
            if (system->isVbox())
                  continue;
            LineSegment* seg = segmentAt(segIdx++);
            seg->setSystem(system);
            qreal x1 = system->firstMeasure()->first(SegChordRest)->canvasPos().x();
            qreal x2 = system->abbox().right();
            qreal y  = system->staffY(si);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setSubtype(SEGMENT_SINGLE);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(p2.x() - p1.x(), 0.0));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setSubtype(SEGMENT_BEGIN);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setSubtype(SEGMENT_MIDDLE);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  seg->setSubtype(SEGMENT_END);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(p2.x() - x1, 0.0));
                  }
            seg->layout();
            seg->move(ipos());
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SLine::readProperties(XmlReader* r)
      {
      if (Element::readProperties(r))
            return true;

      MString8 tag = r->tag();
      int i;
      QPointF pt;
      qreal d;

      if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            while (r->readElement()) {
                  if (r->readPoint("off1", &pt))
                        ls->setUserOff(pt * spatium());
                  else if (r->readPoint("off2", &pt))
                        ls->setUserOff2(pt * spatium());
                  else if (!ls->Element::readProperties(r))
                        r->unknown();
                  }
            add(ls);
            }
      else if (r->readInt("track", &i))
            setTrack(i);
      else if (r->readReal("length", &d))
            setLen(d);
      else if (r->readInt("diagonal", &i))
            setDiagonal(i);
      else if (r->readInt("anchor", &i))
            setAnchor(Anchor(i));
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setLen
//    used to create an element suitable for palette
//---------------------------------------------------------

void SLine::setLen(qreal l)
      {
      if (spannerSegments().isEmpty())
            add(createLineSegment());
      LineSegment* s = frontSegment();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

#if 0
//---------------------------------------------------------
//   change
//---------------------------------------------------------

void SLine::change(Element* o, Element* n)
      {
      int idx = segments.indexOf((LineSegment*)o);
      if (idx == -1) {
            printf("SLine: cannot change %p\n", o);
            return;
            }
      n->setParent(this);
      segments[idx] = (LineSegment*)n;
      }
#endif

//---------------------------------------------------------
//   bbox
//    used by palette: only one segment
//---------------------------------------------------------

QRectF SLine::bbox() const
      {
      if (spannerSegments().isEmpty())
            return QRectF();
      else
            return segmentAt(0)->bbox();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(XmlReader* r)
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
            if (!SLine::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int SLine::tick() const
      {
      if (startElement()->type() != SEGMENT)
            return -1;
      return static_cast<Segment*>(startElement())->tick();
      }

//---------------------------------------------------------
//   tick2
//---------------------------------------------------------

int SLine::tick2() const
      {
      if (endElement()->type() != SEGMENT)
            return -1;
      return static_cast<Segment*>(endElement())->tick();
      }

