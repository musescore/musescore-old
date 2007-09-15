//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: utils.cpp,v 1.24 2006/03/02 17:08:43 wschweer Exp $
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

#include "utils.h"
#include "score.h"
#include "page.h"
#include "segment.h"
#include "layout.h"
#include "recordbutton.h"
#include "greendotbutton.h"

//---------------------------------------------------------
//   RecordButton
//---------------------------------------------------------

RecordButton::RecordButton(QWidget* parent)
   : SimpleButton(":/data/recordOn.svg", ":/data/recordOff.svg", parent)
      {
      setCheckable(true);
      setToolTip(tr("record"));
      }

//---------------------------------------------------------
//   GreendotButton
//---------------------------------------------------------

GreendotButton::GreendotButton(QWidget* parent)
   : SimpleButton(":/data/greendot.svg", ":/data/darkgreendot.svg", parent)
      {
      setCheckable(true);
      setToolTip(tr("record"));
      }

//---------------------------------------------------------
//   drawHandle
//---------------------------------------------------------

QRectF drawHandle(QPainter& p, const QPointF& pos, bool active)
      {
      p.save();
      p.setPen(QPen(QColor(Qt::blue), 2.0/p.matrix().m11()));
      if (active)
            p.setBrush(Qt::blue);
      else
            p.setBrush(Qt::NoBrush);
      qreal w = 8.0 / p.matrix().m11();
      qreal h = 8.0 / p.matrix().m22();

      QRectF r(-w/2, -h/2, w, h);
      r.translate(pos);
      p.drawRect(r);
      p.restore();
      return r;
      }

//---------------------------------------------------------
//   handleRect
//---------------------------------------------------------

QRectF handleRect(const QPointF& pos)
      {
      return QRectF(pos.x()-4, pos.y()-4, 8, 8);
      }

//---------------------------------------------------------
//   tick2measure
//---------------------------------------------------------

Measure* Score::tick2measure(int tick) const
      {
      for (Measure* m = _layout->first(); m; m = m->next()) {
            int st = m->tick();
            int l  = m->tickLen();
            if (tick >= st && tick < (st+l))
                  return m;
            // hack:
            if (m->next() == 0 && tick <= (st+l))
                  return m;
            }
      printf("tick2measure %d not found\n", tick);
      int idx = 0;
      for (Measure* m = _layout->first(); m; m = m->next()) {
            int st = m->tick();
            int l  = m->tickLen();
            printf("(%d)   %d - %d\n", idx, st, st+l);
            ++idx;
            }
      abort();
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Score::tick2segment(int tick) const
      {
      Measure* m = tick2measure(tick);
      if (m == 0) {
            printf("   no segment for tick %d\n", tick);
            return 0;
            }
      for (Segment* segment = m->first(); segment;) {
            Segment* nsegment = segment->next();
            int t1 = segment->tick();
            int t2 = nsegment ? nsegment->tick() : INT_MAX;
            if (tick >= t1 && tick < t2)
                  return segment;
            segment = nsegment;
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2pos
//    returned QPointF is relative to measure
//---------------------------------------------------------

QPointF Score::tick2pos(int tick, int /*staff*/)
      {
      Segment* seg = tick2segment(tick);
      if (seg) {
            return QPointF(seg->x(), 0);
            }
      else
            return QPointF();
      }

//---------------------------------------------------------
//   getStaff
//---------------------------------------------------------

int getStaff(System* system, const QPointF& p)
      {
      QPointF pp = p - system->page()->pos() - system->pos();
      for (int i = 0; i < system->page()->score()->nstaves(); ++i) {
            if (system->bboxStaff(i).contains(pp))
                  return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   nextSeg
//---------------------------------------------------------

int Score::nextSeg(int tick, int track)
      {
      Segment* seg = tick2segment(tick);
      while (seg) {
            seg = seg->next1();
            if (seg == 0)
                  break;
            if (seg->subtype() != Segment::SegChordRest)
                  continue;
            if (seg->element(track))
                  break;
            }
      if (seg == 0) {
            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

//---------------------------------------------------------
//   prevSeg
//---------------------------------------------------------

int Score::prevSeg(int tick, int track)
      {
      Segment* seg = tick2segment(tick);
      while (seg) {
            seg = seg->prev1();
            if (seg == 0)
                  break;
            if (seg->subtype() != Segment::SegChordRest)
                  continue;
            if (seg->element(track))
                  break;
            }
      if (seg == 0) {
            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

//---------------------------------------------------------
//   nextSeg1
//---------------------------------------------------------

int Score::nextSeg1(int tick, int& track)
      {
      Segment* seg   = tick2segment(tick);
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while (seg) {
            seg = seg->next1();
            if (seg == 0)
                  break;
            if (seg->subtype() != Segment::SegChordRest)
                  continue;
            if (seg->element(track))
                  break;
            for (track = startTrack; track < endTrack; ++track) {
                  if (seg->element(track))
                        break;
                  }
            if (track < endTrack)
                  break;
            }
      if (seg == 0) {
            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

//---------------------------------------------------------
//   prevSeg1
//---------------------------------------------------------

int Score::prevSeg1(int tick, int& track)
      {
      Segment* seg   = tick2segment(tick);
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while (seg) {
            seg = seg->prev1();
            if (seg == 0)
                  break;
            if (seg->subtype() != Segment::SegChordRest)
                  continue;
            if (seg->element(track))
                  break;
            for (track = startTrack; track < endTrack; ++track) {
                  if (seg->element(track))
                        break;
                  }
            if (track < endTrack)
                  break;
            }
      if (seg == 0) {
            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

