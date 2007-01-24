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

//---------------------------------------------------------
//   SLine
//---------------------------------------------------------

SLine::SLine(Score* s)
   : Element(s)
      {
      mode  = NORMAL;
      _tick1 = 0;
      _tick2 = 0;
      }

//---------------------------------------------------------
//   layout
//    compute segments from tick1 tick2
//---------------------------------------------------------

void SLine::layout()
      {
      if (!parent())
            return;
      segments.clear();
      Segment* seg1 = _score->tick2segment(_tick1);
      Segment* seg2 = _score->tick2segment(_tick2);
      if (seg1 == 0 || seg2 == 0) {
            printf("SLine Layout: seg not found\n");
            return;
            }
      Measure* measure1 = seg1->measure();
      Measure* measure2 = seg2->measure();
      System* system1   = measure1->system();
      System* system2   = measure2->system();

      QPointF ppos(parent() ? parent()->apos() : QPointF());
      QPointF p1 = QPointF(seg1->x(), 0) + measure1->apos() - ppos;
      QPointF p2 = QPointF(seg2->x(), 0) + measure2->apos() - ppos;

      iSystem is = _score->systems()->begin();
      while (is != _score->systems()->end()) {
            if (*is == system1)
                  break;
            ++is;
            }
      for (;is != _score->systems()->end(); ++is) {
            LineSegment hps;
            if (*is == system1)
                  hps.p1 = p1;
            else {
                  Measure* m = (*is)->measures()->front();
                  hps.p1 = (*is)->pos() + m->pos() - ppos;
                  }
            hps.p2 = p2;
            segments.push_back(hps);
            if (*is == system2)
                  break;
            hps.p2 = (*is)->pos() + QPointF((*is)->bbox().width() - _spatium * 0.5, 0) - ppos;

            if (*is == system1) {
                  if (*is == system2)
                        layoutSingleSegment(&hps);
                  else
                        layoutFirstSegment(&hps);
                  }
            else if (*is == system2)
                  layoutLastSegment(&hps);
            else
                  layoutMidleSegment(&hps);
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool SLine::startEdit(QMatrix& matrix)
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

bool SLine::startEditDrag(const QPointF& p)
      {
      if (bbr1.contains(p))
            mode = DRAG1;
      else if (bbr2.contains(p))
            mode = DRAG2;
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

bool SLine::editDrag(QMatrix&, QPointF*, const QPointF& d)
      {
      QPointF delta(d.x(), 0);    // only x-axis move
      if (mode == DRAG1) {
            off1  += delta / _spatium;
            r1.translate(delta);
            bbr1.translate(delta);
            }
      else if (mode == DRAG2) {
            off2 += delta / _spatium;
            r2.translate(delta);
            bbr2.translate(delta);
            }
      else
            return false;
      layout();
      return true;
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
//TODO      layout2();
      return false;
      }

//---------------------------------------------------------
//   endEditDrag
//---------------------------------------------------------

bool SLine::endEditDrag()
      {
//      layout2();
      return false;
      }

//---------------------------------------------------------
//   contains
//    return true if p is inside of bounding box of object
//    p is relative to the coordinate system of parent()
//---------------------------------------------------------

bool SLine::contains(const QPointF& p) const
      {
      for (ciLineSegment i = segments.begin(); i != segments.end(); ++i) {
            const LineSegment* s = &*i;
            if (s->bbox.contains(p - pos()))
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
      xml.tag("tick1", _tick1);
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
      else if (tag == "tick1")
            _tick1 = score()->fileDivision(i);
      else if (tag == "tick2")
            _tick2 = score()->fileDivision(i);
      else
            return false;
      return true;
      }


