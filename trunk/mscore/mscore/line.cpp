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
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"
#include "layout.h"
#include "viewer.h"

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(Score* s)
   : Element(s)
      {
      _segmentType = SEGMENT_SINGLE;
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

QPointF LineSegment::gripAnchor(int curGrip) const
      {
      int tick = curGrip == 0 ? line()->tick() : line()->tick2();
      System* system;
      return line()->tick2pos(tick, &system);
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool LineSegment::startEdit(const QPointF&)
      {
      return true;
      }

//---------------------------------------------------------
//   pos2anchor
//---------------------------------------------------------

QPointF LineSegment::pos2anchor(const QPointF& pos, int* tick) const
      {
      QPointF anchor;
      score()->pos2TickAnchor(pos, staff(), tick, &anchor);
      return anchor;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(int curGrip, const QPointF& start, const QPointF& d)
      {
      QPointF aapos(start + d);
      QPointF delta(d.x(), 0);    // only x-axis move
      int tick;
      QPointF anchor = pos2anchor(aapos, &tick);

      if (curGrip == 0) {
            r2.translate(-delta);

            QPointF apos(canvasPos() + delta);

            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN)
                  line()->setTick(tick);

            QPointF apos2(canvasPos2());
            setUserXoffset((apos.x() - anchor.x()) / _spatium);
            setXpos(anchor.x() - parent()->canvasPos().x());

            anchor = pos2anchor(apos2, &tick);
            _p2.setX(anchor.x() - canvasPos().x());
            _userOff2.setX((apos2.x() - anchor.x()) / _spatium);
            }
      else {
            r2.translate(delta);

            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END)
                  line()->setTick2(tick);

            QPointF apos(canvasPos2() + delta);

            setUserXoffset2((apos.x() - anchor.x()) / _spatium);
            setXpos2(anchor.x() - canvasPos().x());
            }
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

void LineSegment::endEditDrag()
      {
      // TODO: must move to different Measure?
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void LineSegment::endEdit()
      {
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

QRectF LineSegment::drag(const QPointF& s)
      {
      QRectF r(abbox());
      QPointF newOffset(s / _spatium);
      QPointF diff(userOff() - newOffset);
      setUserOff(newOffset);
//??      setUserXoffset2(userOff2().x() - diff.x());
      return abbox() | r;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void LineSegment::endDrag()
      {
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

QPointF SLine::tick2pos(int tick, System** system)
      {
      Segment* seg = _score->tick2segment(tick);
      *system      = seg->measure()->system();
      return seg->canvasPos();
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout(ScoreLayout* layout)
      {
printf("SLine::layout\n");
      if (!parent()) {
            //
            // when used in a palette, SLine has no parent and
            // tick and tick2 has no meaning so no layout is
            // possible and needed
            //
            if (!segments.isEmpty())
                  setbbox(segments.front()->bbox());
            return;
            }

      System* system1;
      System* system2;
      QPointF p1 = tick2pos(tick(), &system1) - parent()->canvasPos();
      QPointF p2 = tick2pos(_tick2, &system2) - parent()->canvasPos();

      iSystem is = layout->systems()->begin();
      while (is != layout->systems()->end()) {
            if (*is == system1)
                  break;
            ++is;
            }
      int segmentsNeeded = 1;
      for (iSystem iis = is; iis != layout->systems()->end(); ++iis, ++segmentsNeeded) {
            if (*iis == system2)
                  break;
            }

      int segCount = segments.size();
      if (segmentsNeeded != segCount) {
            // TODO: undo/redo ?
            if (segmentsNeeded > segCount) {
                  int n = segmentsNeeded - segCount;
                  for (int i = 0; i < n; ++i)
                        segments.append(createSegment());
                  }
            else {
                  int n = segCount - segmentsNeeded;
                  for (int i = 0; i < n; ++i) {
                        LineSegment* seg = segments.takeLast();
                        delete seg;
                        }
                  }
            segCount = segments.size();
            }
      int segIdx = 0;
      foreach(System* system, *layout->systems()) {
            LineSegment* seg = segments[segIdx];
            if (segIdx == 0)
                  seg->setPos(p1);
            else
                  seg->setPos(system->canvasPos() - parent()->canvasPos());
            if (system == system2) {
                  seg->setXpos2(p2.x());
                  break;
                  }
            seg->setXpos2(system->canvasPos().x()
               + system->bbox().width()
               - _spatium * 0.5
               - seg->canvasPos().x()
               );
            ++segIdx;
            }

      if (segCount == 1) {
            segments[0]->setSegmentType(SEGMENT_SINGLE);
            segments[0]->layout(layout);
            }
      else {
            for (int idx = 0; idx < segCount; ++idx) {
                  LineSegment* seg = segments[idx];
                  if (idx == 0)
                        seg->setSegmentType(SEGMENT_BEGIN);
                  else if (idx == (segCount-1))
                        seg->setSegmentType(SEGMENT_END);
                  else
                        seg->setSegmentType(SEGMENT_MIDDLE);
                  seg->layout(layout);
                  }
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      xml.tag("tick2", _tick2);
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
            LineSegment* ls = createSegment();
            for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                  if (e.tagName() == "off1")
                        ls->setUserOff(readPoint(e));
                  else if (e.tagName() == "off2")
                        ls->setUserOff2(readPoint(e));
                  else
                        domError(e);
                  }
            segments.append(ls);
            }
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
            segments.append(createSegment());
      LineSegment* s = segments.front();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SLine::draw(QPainter& p)
      {
      foreach(LineSegment* seg, segments) {
            p.save();
            p.translate(seg->pos());
            seg->draw(p);
            p.restore();
            }
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void SLine::collectElements(QList<Element*>& el)
      {
      foreach(LineSegment* seg, segments)
            el.append(seg);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SLine::add(Element* e)
      {
      segments.append((LineSegment*) e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SLine::remove(Element*)
      {
printf("SLine::remove=========\n");
      segments.clear();
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void SLine::change(Element* os, Element* ns)
      {
      int n = segments.size();
      for (int i = 0; i < n; ++i) {
            if (segments[i] == os) {
                  segments[i] = (LineSegment*)ns;
                  return;
                  }
            }
      printf("SLine::change: segment not found\n");
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


