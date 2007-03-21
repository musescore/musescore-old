//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: line.cpp,v 1.4 2006/03/13 21:35:59 wschweer Exp $
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
      mode  = NORMAL;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LineSegment::draw(QPainter& p)
      {
      if (mode != NORMAL) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            if (mode == DRAG1) {
                  p.setBrush(Qt::blue);
                  p.drawRect(r1);
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r2);
                  }
            else {
                  p.setBrush(Qt::NoBrush);
                  p.drawRect(r1);
                  p.setBrush(Qt::blue);
                  p.drawRect(r2);
                  }
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool LineSegment::startEdit(QMatrix& matrix, const QPointF&)
      {
      mode = DRAG2;
      QPointF pp2(_p2 + _userOff2 * _spatium);

      qreal w   = 8.0 / matrix.m11();
      qreal h   = 8.0 / matrix.m22();
      QRectF r(-w/2, -h/2, w, h);
      qreal lw  = 1.0 / matrix.m11();
      QRectF br = r.adjusted(-lw, -lw, lw, lw);
      r1        = r;
      bbr1      = br;
      r2        = r.translated(pp2);
      bbr2      = br.translated(pp2);
      return true;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool LineSegment::startEditDrag(Viewer* viewer, const QPointF& p)
      {
      int tick1 = line()->tick();
      int tick2 = line()->tick2();

      if (bbr1.contains(p)) {
            mode = DRAG1;
            QPointF anchor1 = score()->tick2Anchor(tick1, staffIdx());
            QLineF l(anchor1, canvasPos());
            viewer->setDropAnchor(l);
            }
      else if (bbr2.contains(p)) {
            mode = DRAG2;
            QPointF anchor2 = score()->tick2Anchor(tick2, staffIdx());
            QLineF l(anchor2, canvasPos2());
            viewer->setDropAnchor(l);
            }
      else {
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

bool LineSegment::editDrag(Viewer* viewer, QPointF*, const QPointF& d)
      {
      QPointF delta(d.x(), 0);    // only x-axis move
      if (mode == DRAG1) {
            r2.translate(-delta);
            bbr2.translate(-delta);

            int tick;
            QPointF anchor;
            QPointF apos2(canvasPos2());
            QPointF apos(canvasPos() + delta);
            score()->pos2TickAnchor(apos, staff(), &tick, &anchor);
            viewer->setDropAnchor(QLineF(anchor, apos));
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_BEGIN)
                  line()->setTick(tick);
            setUserXoffset((apos.x() - anchor.x()) / _spatium);
            setPos(anchor - parent()->canvasPos());

            score()->pos2TickAnchor(apos2, staff(), &tick, &anchor);
printf("%f %f\n", _p2.x(), (anchor-parent()->canvasPos()).x());
            _p2 = anchor - parent()->canvasPos();
            _userOff2.setX((apos2.x() - anchor.x()) / _spatium);

            // qreal dx = (anchor - mapToCanvas(ipos())).x();
            //  = setX(_userOff2.x() - (delta.x() - dx) / _spatium);
            }
      else {
            r2.translate(delta);
            bbr2.translate(delta);

            int tick;
            QPointF anchor;
            QPointF apos(canvasPos2() + delta);
            score()->pos2TickAnchor(apos, staff(), &tick, &anchor);
            viewer->setDropAnchor(QLineF(anchor, apos));;
            if (_segmentType == SEGMENT_SINGLE || _segmentType == SEGMENT_END)
                  line()->setTick2(tick);
            qreal dx = (anchor - mapToCanvas(_p2)).x();
 printf("%f %f\n", apos.x() - anchor.x(), dx);
            qreal x2 = (apos.x() - anchor.x()) / _spatium;
            _userOff2.setX(x2);
            }
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool LineSegment::edit(QKeyEvent* ev)
      {
      QPointF delta;
      switch (ev->key()) {
            case Qt::Key_Up:
                  _userOff.ry() += -.3;
                  break;
            case Qt::Key_Down:
                  _userOff.ry() += .3;
                  break;
            case Qt::Key_Left:
                  delta = QPointF(-1, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(1, 0);
                  break;
            case Qt::Key_Tab:
                  if (mode == DRAG1)
                        mode = DRAG2;
                  else if (mode == DRAG2)
                        mode = DRAG1;
                  break;
            }
printf("EDIT\n");
      if (mode == DRAG1) {
            setUserOff(userOff() + delta);
            r1.moveTopLeft(r1.topLeft() + delta * _spatium);
            bbr1.moveTopLeft(bbr1.topLeft() + delta * _spatium);
            }
      else if (mode == DRAG2) {
            setUserOff2(userOff() + delta);
            r2.moveTopLeft(r2.topLeft() + delta * _spatium);
            bbr2.moveTopLeft(bbr2.topLeft() + delta * _spatium);
            }
      return false;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void LineSegment::endDrag()
      {
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

bool LineSegment::endEditDrag()
      {
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void LineSegment::endEdit()
      {
      mode = NORMAL;
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
            if (!segments.isEmpty())
                  setbbox(segments.front()->bbox());
            return;
            }

      Segment* seg1 = _score->tick2segment(tick());
      Segment* seg2 = _score->tick2segment(_tick2);
      if (seg1 == 0 || seg2 == 0) {
            printf("SLine Layout: seg not found\n");
            return;
            }
      System* system1 = seg1->measure()->system();
      System* system2 = seg2->measure()->system();

      QPointF ppos(parent()->canvasPos());
      QPointF p1 = seg1->canvasPos() - ppos;
      QPointF p2 = seg2->canvasPos() - ppos;

//      QPointF p1 = seg1->mapToElement(this, seg1->pos());
//      QPointF p2 = seg2->mapToElement(this, seg2->pos());

      setPos(p1);
      p1   -= ipos();
      p2   -= ipos();
//      ppos -= ipos();

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
      if (segmentsNeeded != segments.size()) {
            // if line break changes do a complete re-layout;
            // this especially removes all user editing

            printf("Delete all line segments\n");
            // TODO: selected segments?
            foreach(LineSegment* seg, segments)
                  delete seg;
            segments.clear();
            }

      int seg = 0;
      for (; is != layout->systems()->end(); ++is, ++seg) {
            if (seg >= segments.size()) {
                  printf("create new line segment\n");
                  segments.append(createSegment());
                  }
            LineSegment* hps = segments[seg];
            if (seg == 0) {
                  hps->setPos(p1);
                  }
            else {
printf("TEST2\n");
//TODO                  hps->setPos((*is)->pos() /* + m->pos()*/ - ppos);
                  }
            if (*is == system2) {
                  hps->setPos2(p2);
                  break;
                  }
printf("TEST1\n");
//TODO            hps->setPos2((*is)->pos() + QPointF((*is)->bbox().width() - _spatium * 0.5, 0) - ppos);
            }
      foreach(LineSegment* seg, segments)
            seg->layout(layout);
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      xml.tag("tick2", _tick2);
#if 0
      if (!off1.isNull())
            xml.tag("off1", off1);
      if (!off2.isNull())
            xml.tag("off2", off2);
#endif
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool SLine::readProperties(QDomNode node)
      {
      if (Element::readProperties(node))
            return true;
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
#if 0
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();
      if (tag == "off1")
            off1 = readPoint(node);
      else if (tag == "off2")
            off2 = readPoint(node);
      else if (tag == "tick2")
            _tick2 = score()->fileDivision(i);
      else
            return false;
#endif
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
//            p.save();
//            p.translate(seg->canvasPos());
            seg->draw(p);
//            p.restore();
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
//   setOff
//    set user offset for first segment
//    called from Score->cmdAdd()
//---------------------------------------------------------

void SLine::setOff(const QPointF& o)
      {
      if (!segments.isEmpty())
            segments.front()->setUserOff(o);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SLine::add(Element* e)
      {
      //TODO multi segment
      segments.append((LineSegment*) e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SLine::remove(Element*)
      {
      //TODO multi segment
      segments.clear();
      }
