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
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Element(s)
      {
      mode  = NORMAL;
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
      if (!parent())
            return;

      Segment* seg1 = _score->tick2segment(tick());
      Segment* seg2 = _score->tick2segment(_tick2);
      if (seg1 == 0 || seg2 == 0) {
            printf("SLine Layout: seg not found\n");
            return;
            }
      Measure* measure1 = seg1->measure();
      Measure* measure2 = seg2->measure();
      System* system1   = measure1->system();
      System* system2   = measure2->system();

      QPointF ppos(parent() ? parent()->canvasPos() : QPointF());
      QPointF p1 = QPointF(seg1->x(), 0) + measure1->canvasPos() - ppos;
      QPointF p2 = QPointF(seg2->x(), 0) + measure2->canvasPos() - ppos;
      setPos(p1);
      p1   -= ipos();
      p2   -= ipos();
      ppos -= ipos();

      if (segments.size() != 1) {
            LineSegment s;
            s.p1 = QPointF(0.0, 0.0);
            s.p2 = QPointF(0.0, 0.0);
            segments.append(s);
            }
      LineSegment& s = segments[0];
      s.p1 = p1;
      s.p2 = p2;
printf("Layout tick %d-%d  anchor %f\n", tick(), _tick2, p1.x());

#if 0
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
      if (segmentsNeeded != segments.size())
            segments.clear();

      int seg = 0;
      for (; is != layout->systems()->end(); ++is, ++seg) {
            if (seg+1 >= segments.size())
                  segments.append(LineSegment());
            LineSegment* hps = &segments[seg];
            if (seg == 0)
                  hps->p1 = p1;
            else
                  hps->p1 = (*is)->pos() /* + m->pos()*/ - ppos;
            if (*is == system2) {
                  hps->p2 = p2;
                  break;
                  }
            hps->p2 = (*is)->pos() + QPointF((*is)->bbox().width() - _spatium * 0.5, 0) - ppos;
            }
#endif
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool SLine::startEdit(QMatrix& matrix, const QPointF&)
      {
      mode = DRAG2;
      LineSegment& s1 = segments.front();
      LineSegment& s2 = segments.back();
      QPointF pp1(s1.p1 + off1 * _spatium);
      QPointF pp2(s2.p2 + off2 * _spatium);

      qreal w = 8.0 / matrix.m11();
      qreal h = 8.0 / matrix.m22();
      QRectF r(-w/2, -h/2, w, h);
      qreal lw = 1.0 / matrix.m11();
      QRectF br = r.adjusted(-lw, -lw, lw, lw);
      r1   = r.translated(pp1);
      bbr1 = br.translated(pp1);
      r2   = r.translated(pp2);
      bbr2 = br.translated(pp2);
      return true;
      }

//---------------------------------------------------------
//   startEditDrag
//---------------------------------------------------------

bool SLine::startEditDrag(Viewer* viewer, const QPointF& p)
      {
      if (bbr1.contains(p)) {
            mode = DRAG1;

            LineSegment& s1 = segments.front();
            QPointF pp1(s1.p1 + off1 * _spatium);
            pp1 += canvasPos();
            QPointF anchor1 = score()->tick2Anchor(tick(), staffIdx());
            QLineF l(anchor1, pp1);
            viewer->setDropAnchor(l);
            }
      else if (bbr2.contains(p)) {
            mode = DRAG2;

            LineSegment& s2 = segments.back();
            QPointF pp2(s2.p2 + off2 * _spatium);
            pp2 += canvasPos();
            QPointF anchor2 = score()->tick2Anchor(_tick2, staffIdx());
            QLineF l(anchor2, pp2);
            viewer->setDropAnchor(l);
            }
      else {
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   dragOff
//---------------------------------------------------------

QPointF SLine::dragOff() const
      {
      if (mode == DRAG1)
            return off1;
      else if (mode == DRAG2)
            return off2;
      else
            return QPointF(0.0, 0.0);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

bool SLine::editDrag(Viewer* viewer, QPointF*, const QPointF& d)
      {
      QPointF delta(d.x(), 0);    // only x-axis move
      if (mode == DRAG1) {
            r1.translate(delta);
            bbr1.translate(delta);

            LineSegment& s1 = segments.front();
            QPointF pp1(s1.p1 + off1 * _spatium + delta);
            int tick;
            QPointF anchor;
            QPointF apos(pp1 + canvasPos());
            score()->pos2TickAnchor(apos, staff(), &tick, &anchor);
            viewer->setDropAnchor(QLineF(anchor, apos));
            setTick(tick);

//            s1.p1.setX((anchor1 - canvasPos()).x());
            off1.setX((apos.x() - anchor.x()) / _spatium);
printf("editDrag setTick %d  anchor %f offset %f\n",
    tick, anchor.x() - canvasPos().x(), off1.x());
            }
      else if (mode == DRAG2) {
            off2 += delta / _spatium;
            r2.translate(delta);
            bbr2.translate(delta);

            LineSegment& s2 = segments.back();
            QPointF pp2(s2.p2 + off2 * _spatium);
            pp2 += canvasPos();
            QPointF anchor2;
            score()->pos2TickAnchor(pp2, staff(), &_tick2, &anchor2);
            QLineF l(anchor2, pp2);
            viewer->setDropAnchor(l);
            }
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

bool SLine::endEditDrag()
      {
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void SLine::endEdit()
      {
      mode = NORMAL;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool SLine::edit(QKeyEvent* ev)
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
      if (mode == DRAG1) {
            off1 += delta;
            r1.moveTopLeft(r1.topLeft() + delta * _spatium);
            }
      else if (mode == DRAG2) {
            off2 += delta;
            r2.moveTopLeft(r2.topLeft() + delta * _spatium);
            }
      return false;
      }

//---------------------------------------------------------
//   contains
//    return true if p is inside of bounding box of object
//    p is in canvas coordinates
//---------------------------------------------------------

bool SLine::contains(const QPointF& p) const
      {
      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            const LineSegment* s = &*i;
            if (s->bbox.contains(p - canvasPos()))
                  return true;
            }
      if (bbr1.contains(p-pos()) || bbr2.contains(p-pos()))
            return true;
      return false;
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void SLine::endDrag()
      {
//      layout2();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void SLine::writeProperties(Xml& xml) const
      {
      Element::writeProperties(xml);
      xml.tag("tick2", _tick2);
      if (!off1.isNull())
            xml.tag("off1", off1);
      if (!off2.isNull())
            xml.tag("off2", off2);
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
      return true;
      }

