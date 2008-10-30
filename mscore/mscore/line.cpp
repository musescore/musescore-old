//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: line.cpp,v 1.4 2006/03/13 21:35:59 wschweer Exp $
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

#include "line.h"
#include "textline.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"
#include "layout.h"
#include "viewer.h"
#include "system.h"

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(Score* s)
   : Element(s)
      {
      _segmentType = SEGMENT_SINGLE;
      _system = 0;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void LineSegment::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QPointF pp2(_p2 + _userOff2 * _spatium + canvasPos());
      grip[1].translate(pp2);
      grip[0].translate(canvasPos());
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      int staffIdx = parent()->staffIdx();
      QRectF sbb(_system->staff(staffIdx)->bbox());
      QPointF sp(sbb.topLeft() + _system->canvasPos());

      System* s;  // dummy
      QPointF p1(line()->tick2pos(grip, line()->tick(), staffIdx, &s));
      QPointF p2(sbb.x() + sbb.width() + _system->canvasPos().x(), sp.y());
      QPointF p3(line()->tick2pos(grip, line()->tick2(), staffIdx, &s));

      if (grip == 0) {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        return p1;
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        return sp;
                  }
            }
      else {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        return p3;
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        return p2;
                  }
            }
      return QPointF();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool LineSegment::startEdit(Viewer*, const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(Viewer*, int curGrip, QKeyEvent* ev)
      {
      if ((ev->modifiers() & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 1)
         )) {
            int segments = line()->lineSegments().size();
            int track = line()->track();
            int tick1 = line()->tick();
            int tick2 = line()->tick2();

            if (ev->key() == Qt::Key_Left) {
                  if (curGrip == 0) {
                        int t1 = score()->prevSeg1(tick1, track);
                        if (t1 >= 0)
                              tick1 = t1;
                        }
                  else if (curGrip == 1) {
                        int t2 = score()->prevSeg1(tick2, track);
                        if (t2 >= 0)
                              tick2 = t2;
                        if (tick1 > tick2)
                              return true;
                        }
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (curGrip == 0) {
                        int t1 = score()->nextSeg1(tick1, track);
                        if (t1 >= 0)
                              tick1 = t1;
                        if (tick1 >= tick2)
                              return true;
                        }
                  else if (curGrip == 1) {
                        int t2 = score()->nextSeg1(tick2, track);
                        if (t2 >= 0)
                              tick2 = t2;
                        }
                  }
            line()->setTick(tick1);
            line()->setTick2(tick2);

            line()->layout(score()->layout());
            if (line()->lineSegments().size() != segments)
                  score()->changeLineSegment(curGrip == 1);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   pos2anchor
//---------------------------------------------------------

QPointF LineSegment::pos2anchor(const QPointF& pos, int* tick) const
      {
      QPointF anchor;
      score()->pos2TickAnchor(pos, staffIdx(), tick, &anchor);
      return anchor;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(int curGrip, const QPointF& d)
      {
      QPointF delta(d.x() / _spatium, 0);

      if (line()->type() == TEXTLINE) {
            TextLine* tl = static_cast<TextLine*>(line());
            if (!tl->hasText())
                  delta.setY(d.y() / _spatium);
            }

      if (curGrip == 0) {
            _userOff  += delta;
            _userOff2 -= delta;
            }
      else
            _userOff2 += delta;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void LineSegment::endEditDrag()
      {
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void LineSegment::endEdit()
      {
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

void LineSegment::resetUserOffsets()
      {
      Element::resetUserOffsets();
      setUserOff2(QPointF());
      }

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Element(s)
      {
      setTick(0);
      _tick2 = 0;
      }

//---------------------------------------------------------
//   setTick2
//---------------------------------------------------------

void SLine::setTick2(int t)
      {
      _tick2 = t;
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

QPointF SLine::tick2pos(int grip, int tick, int staffIdx, System** system)
      {
      Segment* seg = _score->tick2segment(tick);
      if (seg == 0) {
            // this never should be triggered because tick2segment
            // always returns the last segment for tick > score end
            MeasureBase* mb = score()->measures()->last();;
            while (mb) {
                  if (mb->type() == MEASURE)
                        break;
                  mb = mb->prev();
                  }
            if (mb == 0 || mb->type() != MEASURE) {
                  *system = 0;
                  return QPointF();
                  }
            Measure* m = (Measure*)mb;
            seg = m->last();
            }
      System* sys = seg->measure()->system();

      // do not go into next system when tick2 is start of system
      if (grip == 1 && sys->firstMeasure()->tick() == seg->tick()) {
            seg = sys->firstMeasure()->prevMeasure()->last();
            sys = seg->measure()->system();
            }
      *system     = sys;
      return QPointF(seg->canvasPos().x(), sys->staff(staffIdx)->bbox().y() + sys->canvasPos().y());
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout(ScoreLayout* layout)
      {
      if (!parent()) {
            //
            // when used in a palette, SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            if (!segments.isEmpty()) {
                  LineSegment* s = segments.front();
                  s->layout(layout);
                  setbbox(s->bbox());
                  }
            return;
            }

      setPos(QPointF());
      setUserOff(QPointF());
      System* s1;
      System* s2;
      int staffI = staffIdx();
      QPointF p1 = tick2pos(0, tick(), staffI, &s1);
      QPointF p2 = tick2pos(1, _tick2, staffI, &s2);

      QList<System*>* systems = layout->systems();

      int sysIdx1        = systems->indexOf(s1);
      int sysIdx2        = systems->indexOf(s2, sysIdx1);
      int segmentsNeeded = sysIdx2 - sysIdx1 + 1;

      int segCount       = segments.size();
      if (segmentsNeeded != segCount) {
            if (segmentsNeeded > segCount) {
                  int n = segmentsNeeded - segCount;
                  for (int i = 0; i < n; ++i) {
                        LineSegment* ls = createLineSegment();
                        add(ls);
                        // set user offset to previous segment's offset
                        if (segCount > 0)
                              ls->setUserOff(QPointF(0, segments[segCount+i-1]->userOff().y()));
                        }
                  }
            else {
                  int n = segCount - segmentsNeeded;
                  for (int i = 0; i < n; ++i) {
                        if (segments.isEmpty()) {
                              printf("SLine::layout(): no segment %d, %d expected\n", i, n);
                              break;
                              }
                        else {
                              /* LineSegment* seg = */ segments.takeLast();
                              // delete seg;   // DEBUG: will be used later
                              }
                        }
                  }
            segCount = segmentsNeeded;
            }
      int segIdx = 0;
      for (int i = sysIdx1; i <= sysIdx2; ++i, ++segIdx) {
            System* system   = (*systems)[i];
            LineSegment* seg = segments[segIdx];
            seg->setSystem(system);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setLineSegmentType(SEGMENT_SINGLE);
                  seg->setPos(p1);
                  seg->setXpos2(p2.x() - p1.x());
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setLineSegmentType(SEGMENT_BEGIN);
                  seg->setPos(p1);
                  seg->setXpos2(seg->gripAnchor(1).x() - p1.x());
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setLineSegmentType(SEGMENT_MIDDLE);
                  seg->setPos(seg->gripAnchor(0));
                  seg->setXpos2(seg->gripAnchor(1).x() - seg->ipos().x());
                  }
            else if (i == sysIdx2) {
                  // end segment
                  seg->setLineSegmentType(SEGMENT_END);
                  seg->setPos(seg->gripAnchor(0));
                  seg->setXpos2(p2.x() - seg->ipos().x());
                  }
            seg->layout(layout);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      xml.tag("tick2", _tick2);
//      xml.tag("track", track());
      //
      // check if user has modified the default layout
      //
      bool modified = false;
      foreach(LineSegment* seg, segments) {
            if (!seg->userOff().isNull() || !seg->userOff2().isNull()) {
                  modified = true;
                  break;
                  }
            }
      if (!modified)
            return;

      //
      // write user modified layout
      //
      foreach(LineSegment* seg, segments) {
            xml.stag("Segment");
            xml.tag("off1", seg->userOff());
            xml.tag("off2", seg->userOff2());
            xml.etag();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SLine::readProperties(QDomElement e)
      {
      if (Element::readProperties(e))
            return true;
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();
      if (tag == "tick2")
            _tick2 = score()->fileDivision(i);
      else if (tag == "Segment") {
            LineSegment* ls = createLineSegment();
            for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                  if (e.tagName() == "off1")
                        ls->setUserOff(readPoint(e));
                  else if (e.tagName() == "off2")
                        ls->setUserOff2(readPoint(e));
                  else
                        domError(e);
                  }
            add(ls);
            }
      else if (e.tagName() == "track")
            setTrack(e.text().toInt());
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   setLen
//    used to create an element suitable for palette
//---------------------------------------------------------

void SLine::setLen(double l)
      {
      if (segments.isEmpty())
            add(createLineSegment());
      LineSegment* s = segments.front();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SLine::draw(QPainter& p) const
      {
      QList<const Element*> el;
      collectElements(el);
      foreach(const Element* e, el) {
            p.save();
            p.translate(e->pos());
            e->draw(p);
            p.restore();
            }
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void SLine::collectElements(QList<const Element*>& el) const
      {
      foreach(const LineSegment* seg, segments)
            seg->collectElements(el);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SLine::add(Element* e)
      {
      e->setParent(this);
      segments.append((LineSegment*) e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SLine::remove(Element*)
      {
      segments.clear();
      }

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

//---------------------------------------------------------
//   bbox
//    used by palette: only one segment
//---------------------------------------------------------

QRectF SLine::bbox() const
      {
      if (segments.isEmpty())
            return QRectF();
      else
            return segments[0]->bbox();
      }


//---------------------------------------------------------
//   write
//---------------------------------------------------------

void SLine::write(Xml& xml) const
      {
      xml.stag(name());
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(QDomElement e)
      {
      setTrack(0);  // set default track
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }


