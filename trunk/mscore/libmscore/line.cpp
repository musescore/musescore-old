//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "line.h"
#include "textline.h"
#include "segment.h"
#include "measure.h"
#include "score.h"
#include "xml.h"
#include "system.h"
#include "utils.h"

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
//   updateGrips
//---------------------------------------------------------

void LineSegment::updateGrips(int* grips, QRectF* grip) const
      {
      *grips = 2;
      QPointF pp2(_p2 + _userOff2 + pagePos());
      grip[1].translate(pp2);
      grip[0].translate(pagePos());
      }

//---------------------------------------------------------
//   setGrip
//---------------------------------------------------------

void LineSegment::setGrip(int grip, const QPointF& p)
      {
      QPointF pt(p * spatium());

      if (grip == 0) {
            QPointF delta = pt - (pagePos() - gripAnchor(grip));
            setUserOff(userOff() + delta);
            _userOff2 -= delta;
            }
      else {
            setUserOff2(pt - pagePos() - _p2 + gripAnchor(grip));
            }
      layout();
      }

//---------------------------------------------------------
//   getGrip
//---------------------------------------------------------

QPointF LineSegment::getGrip(int grip) const
      {
      QPointF pt;
      if (grip == 0)
            pt = pagePos() - gripAnchor(grip);
      else
            pt = _p2 + _userOff2 + pagePos() - gripAnchor(grip);
      return pt / spatium();
      }

//---------------------------------------------------------
//   pagePos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF LineSegment::pagePos() const
      {
      QPointF pt(pos());
      if (parent())
            pt += parent()->pos();
      return pt;
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      if (spannerSegmentType() == SEGMENT_MIDDLE) {
            qreal y = system()->staffY(staffIdx());
            qreal x;
            if (grip == 0)
                  x = system()->firstMeasure()->abbox().left();
            else
                  x = system()->lastMeasure()->abbox().right();
            return QPointF(x, y);
            }
      else {
            System* s;
            QPointF pt(line()->linePos(grip, &s));
            return pt + s->pagePos();
            }
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(MuseScoreView* sv, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (!((modifiers & Qt::ShiftModifier)
         && ((spannerSegmentType() == SEGMENT_SINGLE)
              || (spannerSegmentType() == SEGMENT_BEGIN && curGrip == 0)
              || (spannerSegmentType() == SEGMENT_END && curGrip == 1))))
            return false;

      SpannerSegmentType st = spannerSegmentType();
      SLine* l    = line();
      Segment* s1 = static_cast<Segment*>(l->startElement());
      Segment* s2 = static_cast<Segment*>(l->endElement());

      int track        = l->track();
      bool snapSegment = l->anchor() == ANCHOR_SEGMENT;

      bool removeSegment = false;
      bool bspDirty = false;

      if (key == Qt::Key_Left) {
            if (curGrip == 0) {
                  if (snapSegment)
                        s1 = prevSeg1(s1, track);
                  else {
                        Measure* m = s1->measure()->prevMeasure();
                        s1 = m ? m->first(SegChordRest) : s1;
                        }
                  }
            else if (curGrip == 1) {
                  if (snapSegment) {
                        s2 = prevSeg1(s2, track);
                        if (s2
                           && (s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick())) {
                              removeSegment = true;
                              }
                        }
                  else {
                        if (s2 && (s2->system()->firstMeasure() == s2->measure()))
                              removeSegment = true;
                        Measure* m = s2->measure()->prevMeasure();
                        if (m)
                              s2 = m->last();
                        else {
                              // at end of score?
                              s2 = m->first(SegChordRest);
                              }
                        }
                  }
            }
      else if (key == Qt::Key_Right) {
            if (curGrip == 0) {
                  if (snapSegment)
                        s1 = nextSeg1(s1, track);
                  else {
                        Measure* m  = s1->measure()->nextMeasure();
                        s1 = m ? m->first(SegChordRest) : s1;
                        }
                  }
            else if (curGrip == 1) {
                  if (snapSegment) {
                        if ((s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick()))
                              bspDirty = true;
                        s2 = nextSeg1(s2, track);
                        }
                  else {
                        Measure* m = s2->measure()->nextMeasure();
                        if (m)
                              s2 = m->last();
                        if (s2->system()->firstMeasure() == s2->measure())
                              bspDirty = true;
                        }
                  }
            }
      if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
            return true;

      LineSegment* ls = 0;
      LinkedElements* links = l->links();
      if (l->startElement() != s1) {
            if (s1->system() != (static_cast<Segment*>(l->startElement())->system())) {
                  bspDirty = true;
                  if (key == Qt::Key_Right)
                        ls = l->takeFirstSegment();
                  }
            if (links) {
                  int tick = s1->tick();
                  foreach(Element* e, *links) {
                        Score* score = e->score();
                        SLine* ll = static_cast<SLine*>(e);
                        static_cast<Segment*>(ll->endElement())->removeSpannerBack(ll);
                        Measure* m = score->tick2measure(tick);
                        Segment* segment1 = m->findSegment(SegChordRest, tick);
                        ll->setStartElement(segment1);
                        segment1->add(ll);
                        }
                  }
            else {
                  static_cast<Segment*>(l->startElement())->remove(l);
                  l->setStartElement(s1);
                  s1->add(l);
                  }
            }
      else if (l->endElement() != s2) {
            if (removeSegment) {
                  bspDirty = true;
                  if (key == Qt::Key_Left)
                        ls = l->takeLastSegment();
                  }
            if (links) {
                  int tick = s2->tick();
                  foreach(Element* e, *links) {
                        Score* score = e->score();
                        SLine* ll = static_cast<SLine*>(e);
                        static_cast<Segment*>(ll->endElement())->removeSpannerBack(ll);
                        Measure* m = score->tick2measure(tick);
                        Segment* segment2 = m->findSegment(SegChordRest, tick);
                        ll->setEndElement(segment2);
                        segment2->addSpannerBack(ll);
                        }
                  }
            else {
                  static_cast<Segment*>(l->endElement())->removeSpannerBack(l);
                  l->setEndElement(s2);
                  s2->addSpannerBack(l);
                  }
            }
      l->layout();

      LineSegment* nls = 0;
      if (st == SEGMENT_SINGLE)
            nls = curGrip ? l->backSegment() : l->frontSegment();
      else if (st == SEGMENT_BEGIN)
            nls = l->frontSegment();
      else if (st == SEGMENT_END)
            nls = l->backSegment();

      if (nls && (nls != this))
            sv->changeEditElement(nls);

      if (bspDirty)
            _score->rebuildBspTree();
      delete ls;
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void LineSegment::editDrag(const EditData& ed)
      {
      QPointF delta(ed.delta.x(), line()->diagonal() ? ed.delta.y() : 0.0);

      if (ed.curGrip == 0) {
            setUserOff(userOff() + delta);
            _userOff2 -= delta;
            }
      else
            _userOff2 += delta;
      layout();
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void LineSegment::spatiumChanged(qreal ov, qreal nv)
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
//   linePos
//    return System() coordinates
//---------------------------------------------------------

QPointF SLine::linePos(int grip, System** sys)
      {
      Segment* seg = static_cast<Segment*>(grip == 0 ? startElement() : endElement());
      Measure* m   = seg->measure();
      *sys         = m->system();
      qreal _spatium = spatium();

      qreal x = seg->pos().x() + m->pos().x();

      if (anchor() == ANCHOR_SEGMENT) {
            if (grip == 1) {
                  if (((*sys)->firstMeasure() == m) && (seg->tick() == m->tick())) {
                        m = m->prevMeasure();
                        if (m) {
                              *sys = m->system();
                              x = seg->pos().x() + m->bbox().right();
                              }
                        }
                  }
            }
      else {
            // anchor() == MEASURE
            if (grip == 0) {
                  x = m->pos().x();
                  }
            else {
                  x = m->pos().x() + m->bbox().right();
                  if (type() == VOLTA) {
                        if (seg->subtype() == SegEndBarLine) {
                              Element* e = seg->element(0);
                              if (e && e->type() == BAR_LINE) {
                                    if (e->subtype() == START_REPEAT)
                                          x -= e->width() - _spatium * .5;
                                    else
                                          x -= _spatium * .5;
                                    }
                              }
                        }
                  }
            }
      qreal y = (*sys)->staff(staffIdx())->y();
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
      if (startElement() == 0 || endElement() == 0) {
            printf("SLine::layout() failed: %s %s\n", parent()->name(), name());
            printf("   start %p   end %p\n", startElement(), endElement());
            return;
            }

      System* s1;
      System* s2;
      QPointF p1 = linePos(0, &s1);
      QPointF p2 = linePos(1, &s2);

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

            Measure* m = system->firstMeasure();
            qreal x1 = m->first(SegChordRest)->pos().x() + m->pos().x();
            qreal x2 = system->bbox().right();
            qreal y  = system->staff(si)->y();

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
            seg->move(QPointF(0.0, _yoffset * spatium()));
            seg->adjustReadPos();
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
      if (anchor() != ANCHOR_SEGMENT && (proto == 0 || proto->anchor() != anchor()))
            xml.tag("anchor", anchor());
      if (score() == gscore) {
            // when used as icon
            if (!spannerSegments().isEmpty()) {
                  LineSegment* s = frontSegment();
                  xml.tag("length", s->pos2().x());
                  }
            else
                  xml.tag("length", spatium() * 4);
            return;
            }
      //
      // check if user has modified the default layout
      //
      bool modified = false;
      int n = spannerSegments().size();
      for (int i = 0; i < n; ++i) {
            const LineSegment* seg = segmentAt(i);
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
      for (int i = 0; i < n; ++i) {
            const LineSegment* seg = segmentAt(i);
            xml.stag("Segment");
//            xml.tag("off1", seg->userOff() / spatium());   // is saved as "offset"
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
      // setTick(score()->curTick);
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "tick2")
            __setTick2(score()->fileDivision(i));
      else if (tag == "tick")
            __setTick1(score()->fileDivision(i));
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

void SLine::setLen(qreal l)
      {
      if (spannerSegments().isEmpty())
            add(createLineSegment());
      LineSegment* s = frontSegment();
      s->setPos(QPointF());
      s->setPos2(QPointF(l, 0));
      }

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
//   write
//---------------------------------------------------------

void SLine::write(Xml& xml) const
      {
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
      SLine::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void SLine::read(QDomElement e)
      {
      foreach(SpannerSegment* seg, spannerSegments())
            delete seg;
      spannerSegments().clear();
      setId(e.attribute("id", "-1").toInt());

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!SLine::readProperties(e))
                  domError(e);
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

