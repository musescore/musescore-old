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

#ifdef __MINGW32__
#include <sys/time.h>
#else
#include <time.h>
#endif

#include "score.h"
#include "page.h"
#include "segment.h"
#include "layout.h"
#include "recordbutton.h"
#include "greendotbutton.h"
#include "clef.h"
#include "utils.h"
#include "system.h"

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
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            int st = m->tick();
            int l  = m->tickLen();
            if (tick >= st && tick < (st+l))
                  return m;
            // hack:
            if (m->next() == 0 && tick <= (st+l))
                  return m;
            }
      printf("tick2measure %d not found\n", tick);
      if (debugMode) {
            int idx = 0;
            for (MeasureBase* m = _layout->first(); m; m = m->next()) {
                  int st = m->tick();
                  int l  = m->tickLen();
                  printf("(%d)   %d - %d\n", idx, st, st+l);
                  ++idx;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2measureBase
//---------------------------------------------------------

MeasureBase* Score::tick2measureBase(int tick) const
      {
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            int st = mb->tick();
            int l  = mb->tickLen();
            if (tick >= st && tick < (st+l))
                  return mb;
            }
      printf("tick2measureBase %d not found\n", tick);
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
//   nextSeg1
//---------------------------------------------------------

int Score::nextSeg1(int tick, int& track)
      {
      Segment* seg   = tick2segment(tick);
      if (seg == 0)
            return -1;
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while ((seg = seg->next1())) {
            if (seg->subtype() != Segment::SegChordRest)
                  continue;
            if (seg->element(track))
                  break;
            int t = startTrack;
            for (; t < endTrack; ++t) {
                  if (seg->element(t))
                        break;
                  }
            if (t < endTrack) {
                  track = t;
                  break;
                  }
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
            int t = startTrack;
            for (; t < endTrack; ++t) {
                  if (seg->element(t))
                        break;
                  }
            if (t < endTrack) {
                  track = t;
                  break;
                  }
            }
      if (seg == 0) {
            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

//---------------------------------------------------------
//   dots
//---------------------------------------------------------

static int getDots(int base, int rest, int* dots)
      {
      *dots = 0;
      if (rest >= base / 2) {
            *dots = *dots + 1;
            rest -= base / 2;
            }
      if (rest >= base / 4) {
            *dots = *dots + 1;
            rest -= base / 4;
            }
      return rest;
      }

//---------------------------------------------------------
//   headType
//    return for a given tickLen the baselen of a note
//    (which determines the head symbol) and a number of
//    dots (<= 2)
//
//    return the remaining ticks if any
//---------------------------------------------------------

int headType(int tickLen, DurationType* type, int* dots)
      {
      if (tickLen == 0) {
            *type = D_MEASURE;
            *dots = 0;
            return 0;
            }
      struct Duration {
            int ticks;
            DurationType type;
            };
      Duration base[] = {
            { division * 16, D_LONG    },
            { division * 8,  D_BREVE   },
            { division * 4,  D_WHOLE   },
            { division * 2,  D_HALF    },
            { division,      D_QUARTER },
            { division / 2,  D_EIGHT   },
            { division / 4,  D_16TH    },
            { division / 8,  D_32ND    },
            { division / 16, D_64TH    },
            { division / 32, D_128TH   },
            { division / 64, D_256TH   }
            };

      for (unsigned int i = 0; i < sizeof(base)/sizeof(*base); ++i) {
            if (tickLen / base[i].ticks) {
                  *type = base[i].type;
                  return getDots(base[i].ticks, tickLen % base[i].ticks, dots);
                  }
            }
      *type = D_QUARTER;
      *dots = 0;
      return 0;
      }

//---------------------------------------------------------
//   pitchKeyAdjust
//    change entered note to sounding pitch dependend
//    on key.
//    Example: if F is entered in G-major, a Fis is played
//    key -7 ... +7
//---------------------------------------------------------

int pitchKeyAdjust(int note, int key)
      {
      static int ptab[15][7] = {
//             c  d  e  f  g   a  b
            { -1, 1, 3, 4, 6,  8, 10 },     // Bes
            { -1, 1, 3, 5, 6,  8, 10 },     // Ges
            {  0, 1, 3, 5, 6,  8, 10 },     // Des
            {  0, 1, 3, 5, 7,  8, 10 },     // As
            {  0, 2, 3, 5, 7,  8, 10 },     // Es
            {  0, 2, 3, 5, 7,  9, 10 },     // B
            {  0, 2, 4, 5, 7,  9, 10 },     // F
            {  0, 2, 4, 5, 7,  9, 11 },     // C
            {  0, 2, 4, 6, 7,  9, 11 },     // G
            {  1, 2, 4, 6, 7,  9, 11 },     // D
            {  1, 2, 4, 6, 8,  9, 11 },     // A
            {  1, 3, 4, 6, 8,  9, 11 },     // E
            {  1, 3, 4, 6, 8, 10, 11 },     // H
            {  1, 3, 5, 6, 8, 10, 11 },     // Fis
            {  1, 3, 5, 6, 8, 10, 12 },     // Cis
            };
      return ptab[key+7][note];
      }

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int y2pitch(double y, int clef)
      {
      int l = lrint(y / _spatium * 2.0);
      return line2pitch(l, clef, 0);
      }

//---------------------------------------------------------
//   line2pitch
//    key  -7 ... +7
//---------------------------------------------------------

int line2pitch(int line, int clef, int key)
      {
      int l = clefTable[clef].pitchOffset - line;
      int octave = 0;
      while (l < 0) {
            l += 7;
            octave++;
            }
      if (l > 74)
            l = 74;
      octave += l / 7;
      l       = l % 7;
      return pitchKeyAdjust(l, key) + octave * 12;
      }

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

int quantizeLen(int, int len, int raster)
      {
      int rl = ((len + raster - 1) / raster) * raster;
      rl /= 2;
      if (rl == 0)
            rl = 1;
      rl = ((len + rl - 1) / rl) * rl;
      return rl;
      }

//---------------------------------------------------------
//   curTime
//---------------------------------------------------------

double curTime()
      {
#ifdef __MINGW32__
      struct timeval t;
      gettimeofday(&t, 0);
      return (double)((double)t.tv_sec + (t.tv_usec / 1000000.0));
#else
      struct timespec t;
      clock_gettime(CLOCK_MONOTONIC, &t);
      return double(t.tv_sec) + double(t.tv_nsec) / 1000000000.0;
#endif
      }

//---------------------------------------------------------
//   selectNoteMessage
//---------------------------------------------------------

void selectNoteMessage()
      {
      QMessageBox::critical(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("please select a single note and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectNoteRestMessage()
      {
      QMessageBox::critical(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("please select a single note or rest and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectNoteSlurMessage()
      {
      QMessageBox::critical(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("please select a single note or slur and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectStavesMessage()
      {
      QMessageBox::critical(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("please select one or more staves and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }


