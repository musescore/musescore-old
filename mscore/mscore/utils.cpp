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

#include <sys/time.h>

#include "score.h"
#include "page.h"
#include "segment.h"
#include "recordbutton.h"
#include "greendotbutton.h"
#include "clef.h"
#include "utils.h"
#include "system.h"
#include "measure.h"
#include "pitchspelling.h"

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
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = static_cast<Measure*>(mb);
            int st = m->tick();
            int l  = m->tickLen();
            if (tick >= st && tick < (st+l))
                  return m;
            // hack:
            MeasureBase* nmb;
            for (nmb = mb->next(); nmb; nmb = nmb->next()) {
                  if (nmb->type() == MEASURE)
                        break;
                  }
            if (nmb == 0)
                  return m;
            }
      printf("-tick2measure %d not found\n", tick);
//      if (debugMode) {
        printf("first %p\n", first());
            for (MeasureBase* m = first(); m; m = m->next()) {
                  int st = m->tick();
                  int l  = m->tickLen();
                  printf("%d - %d\n", st, st+l);
                  }
//            }
      return 0;
      }

//---------------------------------------------------------
//   tick2measureBase
//---------------------------------------------------------

MeasureBase* Score::tick2measureBase(int tick) const
      {
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            int st = mb->tick();
            int l  = mb->tickLen();
            if (tick >= st && tick < (st+l))
                  return mb;
            }
//      printf("tick2measureBase %d not found\n", tick);
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
//            printf("no seg found\n");
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
//            printf("no seg found\n");
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
//            printf("no seg found\n");
            return -1;
            }
      return seg->tick();
      }

//---------------------------------------------------------
//   pitchKeyAdjust
//    change entered note to sounding pitch dependend
//    on key.
//    Example: if F is entered in G-major, a Fis is played
//    key -7 ... +7
//---------------------------------------------------------

int pitchKeyAdjust(int step, int key)
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
      return ptab[key+7][step];
      }

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int y2pitch(double y, int clef, double _spatium)
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
      int l      = clefTable[clef].pitchOffset - line;
      int octave = 0;
      while (l < 0) {
            l += 7;
            octave++;
            }
//      if (l > 74)
//            l = 74;
      octave += l / 7;
      l       = l % 7;

      int pitch = pitchKeyAdjust(l, key) + octave * 12;

      if (pitch > 127)
            pitch = 127;
      else if (pitch < 0)
            pitch = 0;
      return pitch;
      }

//---------------------------------------------------------
//   quantizeLen
//---------------------------------------------------------

int quantizeLen(int len, int raster)
      {
      int rl = ((len + raster - 1) / raster) * raster;      // round up to raster
#if 0
      rl /= 2;
      if (rl == 0)
            rl = 1;
      rl = ((len + rl - 1) / rl) * rl;
#endif
      return rl;
      }

//---------------------------------------------------------
//   curTime
//---------------------------------------------------------

double curTime()
      {
#if defined (__MINGW32__) || defined (__APPLE__)
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
      QMessageBox::information(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("No note selected:\n"
                         "Please select a single note and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectNoteRestMessage()
      {
      QMessageBox::information(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("No note or rest selected:\n"
                         "Please select a single note or rest and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectNoteSlurMessage()
      {
      QMessageBox::information(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("Please select a single note or slur and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

void selectStavesMessage()
      {
      QMessageBox::information(0,
         QMessageBox::tr("MuseScore:"),
         QMessageBox::tr("Please select one or more staves and retry operation\n"),
         QMessageBox::Ok, QMessageBox::NoButton);
      }

static const char* vall[] = {
            "c","c#","d","d#","e","f","f#","g","g#","a","a#","h"
            };
static const char* valu[] = {
            "C","C#","D","D#","E","F","F#","G","G#","A","A#","H"
            };

//---------------------------------------------------------
//   pitch2string
//---------------------------------------------------------

QString pitch2string(int v)
      {
      if (v < 0 || v > 127)
            return QString("----");
      int octave = (v / 12) - 2;
      int i = v % 12;
      QString s(octave < 0 ? valu[i] : vall[i]);
      return QString("%1%2").arg(octave < 0 ? valu[i] : vall[i]).arg(octave);
      }

//---------------------------------------------------------
//   Interval
//    steps - semitones
//---------------------------------------------------------

Interval intervalList[26] = {
      { 0, 0 },         // Perfect Unison
      { 0, 1 },         // Augmented Unison

      { 1, 0 },         // Diminished Second
      { 1, 1 },         // Minor Second
      { 1, 2 },         // Major Second
      { 1, 3 },         // Augmented Second

      { 2, 2 },         // Diminished Third
      { 2, 3 },         // Minor Third
      { 2, 4 },         // Major Third
      { 2, 5 },         // Augmented Third

      { 3, 4 },         // Diminished Fourth
      { 3, 5 },         // Perfect Fourth
      { 3, 6 },         // Augmented Fourth

      { 4, 6 },         // Diminished Fifth
      { 4, 7 },         // Perfect Fifth
      { 4, 8 },         // Augmented Fifth

      { 5, 7 },         // Diminished Sixth
      { 5, 8 },         // Minor Sixth
      { 5, 9 },         // Major Sixth
      { 5, 10 },        // Augmented Sixth

      { 6, 9 },         // Diminished Seventh
      { 6, 10 },        // Minor Seventh
      { 6, 11 },        // Major Seventh
      { 6, 12 },        // Augmented Seventh

      { 7, 11 },        // Diminshed Octave
      { 7, 12 }         // Perfect Octave
      };

//---------------------------------------------------------
//   searchInterval
//---------------------------------------------------------

int searchInterval(int steps, int semitones)
      {
      unsigned n = sizeof(intervalList)/sizeof(*intervalList);
      for (unsigned i = 0; i < n; ++i) {
            if ((intervalList[i].steps == steps) && (intervalList[i].semitones == semitones))
                  return i;
            }
      return -1;
      }

//---------------------------------------------------------
//   transposeInterval
//---------------------------------------------------------

void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, int interval, TransposeDirection dir)
      {
      int steps     = intervalList[interval].steps;
      int semitones = intervalList[interval].semitones;

      if (dir == TRANSPOSE_DOWN) {
            steps     = -steps;
            semitones = -semitones;
            *rpitch    = pitch - intervalList[interval].semitones;
            }
      else
            *rpitch    = pitch + intervalList[interval].semitones;

      int step, alter;

      for (;;) {
            int octave = (pitch / 12);

            step       = tpc2step(tpc) + steps;
            while (step < 0) {
                  step += 7;
                  octave -= 1;
                  }
            while (step >= 7) {
                  step -= 7;
                  octave += 1;
                  }

            int p1     = tpc2pitch(step2tpc(step, 0)) + octave * 12;
            alter      = semitones - (p1 - pitch);
printf("Interval(%d,%d,%d) step %d octave %d p1 %d(%d-%d) alter %d\n",
    interval, steps, semitones, step, octave, p1, pitch, *rpitch, alter);
            if (alter > 2)
                  steps += 1;
            else if (alter < -2)
                  steps -= 1;
            else
                  break;
            }
      *rtpc  = step2tpc(step, alter);
      }

//---------------------------------------------------------
//   transposeTpc
//---------------------------------------------------------

int transposeTpc(int tpc, int interval, TransposeDirection dir)
      {
      if (tpc == INVALID_TPC || interval == 0 || interval == 25) // perfect unison & perfect octave
            return tpc;

      int steps     = intervalList[interval].steps;
      int semitones = intervalList[interval].semitones;

      if (dir == TRANSPOSE_DOWN) {
            steps     = -steps;
            semitones = -semitones;
            }

      int step, alter;
      int pitch = tpc2pitch(tpc);

      for (;;) {
            step       = tpc2step(tpc) + steps;
            while (step < 0)
                  step += 7;
            while (step >= 7)
                  step -= 7;
            int p1     = tpc2pitch(step2tpc(step, 0));
            alter      = semitones - (p1 - pitch);
            if (alter > 2)
                  steps -= 1;
            else if (alter < -2)
                  steps += 1;
            else
                  break;
            }
      return step2tpc(step, alter);
      }


