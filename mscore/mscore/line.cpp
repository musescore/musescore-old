//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
#include "xml.h"
#include "scoreview.h"
#include "system.h"
#include "utils.h"

//---------------------------------------------------------
//   LineSegment
//---------------------------------------------------------

LineSegment::LineSegment(Score* s)
   : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE | ELEMENT_SEGMENT);
      _segmentType = SEGMENT_SINGLE;
      _system      = 0;
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
//   canvasPos
//    return position in canvas coordinates
//---------------------------------------------------------

QPointF LineSegment::canvasPos() const
      {
      return pos();
      }

//---------------------------------------------------------
//   gripAnchor
//---------------------------------------------------------

QPointF LineSegment::gripAnchor(int grip) const
      {
      if (_segmentType == SEGMENT_MIDDLE) {
            double y = _system->staffY(staffIdx());
            double x;
            if (grip == 0)
                  x = _system->firstMeasure()->abbox().left();
            else
                  x = _system->lastMeasure()->abbox().right();
            return QPointF(x, y);
            }
      else {
            System* s;  // dummy
            return line()->tick2pos(grip, &s);
            }
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool LineSegment::edit(ScoreView* sv, int curGrip, int key, Qt::KeyboardModifiers modifiers, const QString&)
      {
      if (!((modifiers & Qt::ShiftModifier)
         && ((_segmentType == SEGMENT_SINGLE)
              || (_segmentType == SEGMENT_BEGIN && curGrip == 0)
              || (_segmentType == SEGMENT_END && curGrip == 1))))
            return false;

      LineSegmentType st = _segmentType;
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
                        if (s2 && (s2->system()->firstMeasure() == s2->measure())
                           && (s2->tick() == s2->measure()->tick())) {
                              bspDirty = true;
                              }
                        s2 = nextSeg1(s2, track);
                        }
                  else {
                        Measure* m = s2->measure()->nextMeasure();
                        if (m)
                              s2 = m->last();
                        }
                  }
            }
      if (s1 == 0 || s2 == 0 || s1->tick() >= s2->tick())
            return true;

      LineSegment* ls = 0;
      if (l->startElement() != s1) {
            if (s1->system() != (static_cast<Segment*>(l->startElement())->system())) {
                  bspDirty = true;
                  if (key == Qt::Key_Right)
                        ls = l->lineSegments().takeFirst();
                  }
            static_cast<Segment*>(l->startElement())->remove(l);
            l->setStartElement(s1);
            s1->add(parent());
            }
      else if (l->endElement() != s2) {
            if (removeSegment) {
                  bspDirty = true;
                  if (key == Qt::Key_Left)
                        ls = l->lineSegments().takeLast();
                  }
            static_cast<Segment*>(l->endElement())->removeSpannerBack(line());
            l->setEndElement(s2);
            s2->addSpannerBack(line());
            }
      l->layout();

      LineSegment* nls = 0;
      if (st == SEGMENT_SINGLE)
            nls = curGrip ? l->lineSegments().back() : l->lineSegments().front();
      else if (st == SEGMENT_BEGIN)
            nls = l->lineSegments().front();
      else if (st == SEGMENT_END)
            nls = l->lineSegments().back();
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
   : Spanner(s)
      {
      _diagonal = false;
      setTrack(0);
      }

SLine::SLine(const SLine& s)
   : Spanner(s)
      {
      _diagonal = s._diagonal;
      foreach(LineSegment* ls, s.segments)
            add(ls->clone());
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

QPointF SLine::tick2pos(int grip, System** sys)
      {
      Segment* seg = static_cast<Segment*>(grip == 0 ? startElement() : endElement());
      Measure* m   = seg->measure();
      *sys         = m->system();

      double x = seg->canvasPos().x();

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
      double y = (*sys)->staffY(staffIdx());
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
            if (!segments.isEmpty()) {
                  LineSegment* s = segments.front();
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
      int segCount = segments.size();

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
                              LineSegment* seg = segments.takeLast();
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
            LineSegment* seg = segments[segIdx++];
            seg->setSystem(system);
            double x1 = system->firstMeasure()->first(SegChordRest)->canvasPos().x();
            double x2 = system->abbox().right();
            double y  = system->staffY(si);

            if (sysIdx1 == sysIdx2) {
                  // single segment
                  seg->setSegmentType(SEGMENT_SINGLE);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(p2.x() - p1.x(), 0.0));
                  }
            else if (i == sysIdx1) {
                  // start segment
                  seg->setSegmentType(SEGMENT_BEGIN);
                  seg->setPos(p1);
                  seg->setPos2(QPointF(x2 - p1.x(), 0.0));
                  }
            else if (i > 0 && i != sysIdx2) {
                  // middle segment
                  seg->setSegmentType(SEGMENT_MIDDLE);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(x2 - x1, 0.0));
                  }
            else if (i == sysIdx2) {
                  // end segment
                  seg->setSegmentType(SEGMENT_END);
                  seg->setPos(QPointF(x1, y));
                  seg->setPos2(QPointF(p2.x() - x1, 0.0));
                  }
            seg->layout();
            seg->move(ipos());
            }
/*      printf("layout:\n");
      foreach(LineSegment* ls, segments) {
            printf("  %p %d\n", ls, int(ls->segmentType()));
            }
      */
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
            if (!segments.isEmpty()) {
                  LineSegment* s = segments.front();
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
      LineSegment* ls = static_cast<LineSegment*>(e);
      ls->setParent(this);
      segments.append(ls);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SLine::remove(Element* e)
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
      xml.stag(QString("%1 id=\"%2\"").arg(name()).arg(id()));
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

