//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "scoreview.h"
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

LineSegment::LineSegment(const LineSegment& s)
   : Element(s)
      {
      _p2          = s._p2;
      _userOff2    = s._userOff2;
      r1           = s.r1;
      r2           = s.r2;
      _segmentType = s._segmentType;
      _system      = s._system;
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void LineSegment::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QPointF pp2(_p2 + _userOff2 + canvasPos());
      grip[1].translate(pp2);
      grip[0].translate(canvasPos());
      }

//---------------------------------------------------------
//   pos2
//---------------------------------------------------------

QPointF LineSegment::pos2() const
      {
      return _p2 + _userOff2;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      int staffIdx = parent()->staffIdx();
      QRectF sbb(_system->staff(staffIdx)->bbox());
      QPointF sp(sbb.topLeft() + _system->canvasPos());

      int tck = _system->measures().front()->tick();

      System* s;  // dummy

      if (grip == 0) {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_BEGIN:
                        return line()->tick2pos(grip, line()->tick(), staffIdx, &s);
                  case SEGMENT_MIDDLE:
                  case SEGMENT_END:
                        return line()->tick2pos(grip, tck, staffIdx, &s);
                  }
            }
      else {
            switch(_segmentType) {
                  case SEGMENT_SINGLE:
                  case SEGMENT_END:
                        return line()->tick2pos(grip, line()->tick2(), staffIdx, &s);
                  case SEGMENT_BEGIN:
                  case SEGMENT_MIDDLE:
                        return QPointF(sbb.x() + sbb.width() + _system->canvasPos().x(), sp.y());
                  }
            }
      return QPointF();
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(ScoreView* view, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if ((modifiers & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 1)
         )) {
            int segments = line()->lineSegments().size();
            int track = line()->track();
            int tick1 = line()->tick();
            int tick2 = line()->tick2();
            bool snapSegment = line()->anchor() == ANCHOR_SEGMENT;

            if (key == Qt::Key_Left) {
                  if (curGrip == 0) {
                        int t1;
                        if (snapSegment) {
                              t1 = score()->prevSeg1(tick1, track);
                              }
                        else {
                              Measure* m = score()->tick2measure(tick1)->prevMeasure();
                              t1 = m ? m->tick() : 0;
                              }
                        if (t1 >= 0)
                              tick1 = t1;
                        }
                  else if (curGrip == 1) {
                        int t2;
                        if (snapSegment) {
                              t2 = score()->prevSeg1(tick2, track);
                              }
                        else {
                              Measure* m = score()->tick2measure(tick2);
                              if (m->tick() == tick2) {
                                    Measure* mm = m->prevMeasure();
                                    t2 = mm ? mm->tick() : 0;
                                    }
                              else {
                                    // at end of score?
                                    t2 = m->tick();
                                    }
                              }
                        if (t2 >= 0)
                              tick2 = t2;
                        if (tick1 >= tick2)
                              return true;
                        }
                  }
            else if (key == Qt::Key_Right) {
                  if (curGrip == 0) {
                        int t1;
                        if (snapSegment) {
                              t1 = score()->nextSeg1(tick1, track);
                              }
                        else {
                              Measure* m  = score()->tick2measure(tick1);
                              Measure* mb = m->nextMeasure();
                              t1 = mb ? mb->tick() : m->tick() + m->tickLen();
                              }
                        if (t1 >= 0)
                              tick1 = t1;
                        if (tick1 >= tick2)
                              return true;
                        }
                  else if (curGrip == 1) {
                        int t2 = 0;
                        if (snapSegment) {
                              t2 = score()->nextSeg1(tick2, track);
                              }
                        else {
                              Measure* m  = score()->tick2measure(tick2);
                              if (m) {
                                    Measure* mb = m->nextMeasure();
                                    t2 = mb ? mb->tick() : m->tick() + m->tickLen();
                                    }
                              }
                        if (t2 > tick1)
                              tick2 = t2;
                        }
                  }
            line()->setTick(tick1);
            line()->setTick2(tick2);

            line()->layout();
            if (line()->lineSegments().size() != segments)
                  view->changeLineSegment(curGrip == 1);
            if (line()->type() == OTTAVA)
                  score()->fixPpitch();
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(int curGrip, const QPointF& d)
      {
      QPointF delta(d.x(), 0);

      if (line()->type() == TEXTLINE) {
            TextLine* tl = static_cast<TextLine*>(line());
            if (tl->diagonal())
                  delta.setY(d.y());
            }

      if (curGrip == 0) {
            setUserOff(userOff() + delta);
            _userOff2 -= delta;
            }
      else
            _userOff2 += delta;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(double ov, double nv)
      {
      Element::spatiumChanged(ov, nv);
      _userOff2 *= nv / ov;
      }

//---------------------------------------------------------
//   toDefault
//---------------------------------------------------------

void LineSegment::toDefault()
      {
      Element::toDefault();
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
      _diagonal = false;
      _anchor = ANCHOR_SEGMENT;
      }

SLine::SLine(const SLine& s)
   : Element(s)
      {
      _tick2    = s._tick2;
      _diagonal = s._diagonal;
      _anchor   = s._anchor;
      foreach(LineSegment* ls, s.segments)
            add(ls->clone());
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

      if (_anchor == ANCHOR_SEGMENT) {
            // do not go into next system when tick2 is start of system
            if (grip == 1 && sys->firstMeasure()->tick() == seg->tick()) {
                  Measure* m = sys->firstMeasure()->prevMeasure();
                  if (m) {
                        seg = m->last();
                        sys = seg->measure()->system();
                        }
                  }
            *system = sys;
            return QPointF(seg->canvasPos().x(), sys->staff(staffIdx)->bbox().y() + sys->canvasPos().y());
            }
      else {
            Measure* m = seg->measure();
            double y   = sys->staff(staffIdx)->bbox().y() + sys->canvasPos().y();
            double x   = m->canvasPos().x();
            if (m->tick() < seg->tick()) {      // to end of last measure?
                  x += m->bbox().width();
                  }
            else if (grip == 1 && sys->firstMeasure() == m) {
                  m = m->prevMeasure();
                  if (m) {
                        seg = m->last();
                        sys = seg->measure()->system();
                        x   = m->canvasPos().x() + m->bbox().width();
                        y   = sys->staff(staffIdx)->bbox().y() + sys->canvasPos().y();
                        }
                  }
            *system = sys;
            return QPointF(x, y);
            }
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (score() == gscore) {
            //
            // when used in a palette, SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            if (!segments.isEmpty()) {
                  LineSegment* s = segments.front();
                  s->layout();
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

      QList<System*>* systems = score()->systems();
      int sysIdx1        = systems->indexOf(s1);
      int sysIdx2        = systems->indexOf(s2);
      int segmentsNeeded = 0;
      for (int i = sysIdx1; i < sysIdx2+1;  ++i) {
            if ((*systems)[i]->isVbox())
                  continue;
            ++segmentsNeeded;
            }

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
                  printf("SLine: segments %d needed %d, remove %d\n", segCount, segmentsNeeded, n);
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
      for (int i = sysIdx1; i <= sysIdx2; ++i) {
            System* system   = (*systems)[i];
            if (system->isVbox())
                  continue;
            LineSegment* seg = segments[segIdx++];
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
            seg->layout();
            }
      }

//---------------------------------------------------------
//   writeProperties
//    write properties different from prototype
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml, const SLine* proto) const
      {
      Element::writeProperties(xml, proto);
      if (_diagonal && (proto == 0 || proto->diagonal() != _diagonal))
            xml.tag("diagonal", _diagonal);
      if (_anchor != ANCHOR_SEGMENT && (proto == 0 || proto->anchor() != _anchor))
            xml.tag("anchor", _anchor);
      if (score() == gscore) {
            // when used as icon
            if (!segments.isEmpty()) {
                  LineSegment* s = segments.front();
                  xml.tag("length", s->pos2().x());
                  }
            else
                  xml.tag("length", spatium() * 4);
            return;
            }
      xml.tag("tick2", _tick2);
      //
      // check if user has modified the default layout
      //
      bool modified = false;
      foreach(LineSegment* seg, segments) {
            if (!seg->userOff().isNull()
               || !seg->userOff2().isNull()
               || !seg->visible()) {
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
            xml.tag("off1", seg->userOff() / spatium());
            xml.tag("off2", seg->userOff2() / spatium());
            seg->Element::writeProperties(xml);
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
                        ls->setUserOff(readPoint(e) * spatium());
                  else if (e.tagName() == "off2")
                        ls->setUserOff2(readPoint(e) * spatium());
                  else if (!ls->Element::readProperties(e))
                        domError(e);
                  }
            add(ls);
            }
      else if (tag == "track")
            setTrack(i);
      else if (tag == "length")
            setLen(val.toDouble());
      else if (tag == "diagonal")
            setDiagonal(i);
      else if (tag == "anchor")
            setAnchor(Anchor(i));
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
//   scanElements
//---------------------------------------------------------

void SLine::scanElements(void* data, void (*func)(void*, Element*))
      {
      foreach(LineSegment* seg, segments)
            seg->scanElements(data, func);
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
      foreach(LineSegment* seg, segments)
            delete seg;
      segments.clear();
      setTrack(0);  // set default track
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   check
//    check for consistency; return false if element is
//    invalid
//---------------------------------------------------------

bool SLine::check() const
      {
      if (tick2() <= tick())
            return false;
      return true;
      }
