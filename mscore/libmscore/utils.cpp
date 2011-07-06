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

#include <sys/time.h>

#include "config.h"
#include "score.h"
#include "page.h"
#include "segment.h"
#include "clef.h"
#include "utils.h"
#include "system.h"
#include "measure.h"
#include "pitchspelling.h"
#include "chordrest.h"
#include "part.h"
#include "staff.h"
#include "note.h"
#include "chord.h"

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
            int l  = m->ticks();
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
                  int l  = m->ticks();
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
            int l  = mb->ticks();
            if (tick >= st && tick < (st+l))
                  return mb;
            }
//      printf("tick2measureBase %d not found\n", tick);
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Score::tick2segment(int tick, bool first, SegmentTypes st) const
      {
      Measure* m = tick2measure(tick);
      if (m == 0) {
            printf("   no segment for tick %d\n", tick);
            return 0;
            }
      for (Segment* segment = m->first(st); segment;) {
            int t1 = segment->tick();
            Segment* nsegment = segment->next(st);
            int t2 = nsegment ? nsegment->tick() : INT_MAX;
            if (((tick == t1) && first) || ((tick == t1) && (tick < t2)))
                  return segment;
            segment = nsegment;
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2segmentEnd
//---------------------------------------------------------

/**
 Find a segment containing a note or rest in \a track ending at \a tick
 Return the segment or null
 */

Segment* Score::tick2segmentEnd(int track, int tick) const
      {
//      printf("tick2segmentEnd(track=%d, tick=%d)", track, tick);
      // loop over all measures
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = static_cast<Measure*>(mb);
            int st = m->tick();
            int l  = m->ticks();
            if (tick > st && tick <= (st+l)) {
                  // loop over all segments
                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        Element* el = segment->element(track);
                        if (!el)
                              continue;
                        if (!el->isChordRest())
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        // TODO LVI: check if following is correct, see exceptions in
                        // ExportMusicXml::chord() and ExportMusicXml::rest()
                        int endTick = cr->tick() + cr->actualTicks();
                        if (endTick < tick)
                              continue; // not found yet
                        else if (endTick == tick) {
//                              printf(" found seg=%p at tick=%d\n", segment, cr->tick());
                              return segment; // found it
                              }
                        else {
                              // endTick > tick (beyond the tick we are looking for)
//                              printf("\n");
                              return 0;
                              }
                        }
                  }
            }
//      printf("\n");
      return 0;
      }

//---------------------------------------------------------
//   getStaff
//---------------------------------------------------------

int getStaff(System* system, const QPointF& p)
      {
      QPointF pp = p - system->page()->pos() - system->pos();
      for (int i = 0; i < system->page()->score()->nstaves(); ++i) {
            qreal sp = system->spatium();
            QRectF r = system->bboxStaff(i).adjusted(0.0, -sp, 0.0, sp);
            if (r.contains(pp))
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
            if (seg->subtype() != SegChordRest)
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

Segment* nextSeg1(Segment* seg, int& track)
      {
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while ((seg = seg->next1(SegChordRest))) {
            for (int t = startTrack; t < endTrack; ++t) {
                  if (seg->element(t)) {
                        track = t;
                        return seg;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   prevSeg1
//---------------------------------------------------------

Segment* prevSeg1(Segment* seg, int& track)
      {
      int staffIdx   = track / VOICES;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      while ((seg = seg->prev1(SegChordRest))) {
            for (int t = startTrack; t < endTrack; ++t) {
                  if (seg->element(t)) {
                        track = t;
                        return seg;
                        }
                  }
            }
      return 0;
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

/*!
 * Returns the string representation of the given pitch.
 *
 * Returns the latin letter name, accidental, and octave numeral.
 * Uses upper case only for pitches 0-24.
 *
 * @param v
 *  The pitch number of the note.
 *
 * @return
 *  The string representation of the note.
 */
QString pitch2string(int v)
      {
      if (v < 0 || v > 127)
            return QString("----");
      int octave = (v / 12) - 2;
      int i = v % 12;
      QString s(octave < 0 ? valu[i] : vall[i]);
      return QString("%1%2").arg(octave < 0 ? valu[i] : vall[i]).arg(octave);
      }

/*!
 * An array of all supported interval sorted by size.
 *
 * Because intervals can be spelled differently, this array
 * tracks all the different valid intervals. They are arranged
 * in diatonic then chromatic order.
 */
Interval intervalList[26] = {
      Interval(0, 0),         //  0 Perfect Unison
      Interval(0, 1),         //  1 Augmented Unison

      Interval(1, 0),         //  2 Diminished Second
      Interval(1, 1),         //  3 Minor Second
      Interval(1, 2),         //  4 Major Second
      Interval(1, 3),         //  5 Augmented Second

      Interval(2, 2),         //  6 Diminished Third
      Interval(2, 3),         //  7 Minor Third
      Interval(2, 4),         //  8 Major Third
      Interval(2, 5),         //  9 Augmented Third

      Interval(3, 4),         // 10 Diminished Fourth
      Interval(3, 5),         // 11 Perfect Fourth
      Interval(3, 6),         // 12 Augmented Fourth

      Interval(4, 6),         // 13 Diminished Fifth
      Interval(4, 7),         // 14 Perfect Fifth
      Interval(4, 8),         // 15 Augmented Fifth

      Interval(5, 7),         // 16 Diminished Sixth
      Interval(5, 8),         // 17 Minor Sixth
      Interval(5, 9),         // 18 Major Sixth
      Interval(5, 10),        // 19 Augmented Sixth

      Interval(6, 9),         // 20 Diminished Seventh
      Interval(6, 10),        // 21 Minor Seventh
      Interval(6, 11),        // 22 Major Seventh
      Interval(6, 12),        // 23 Augmented Seventh

      Interval(7, 11),        // 24 Diminshed Octave
      Interval(7, 12)         // 25 Perfect Octave
      };

/*!
 * Finds the most likely diatonic interval for a semitone distance.
 *
 * Uses the most common diatonic intervals.
 *
 * @param
 *  The number of semitones in the chromatic interval.
 *  Negative semitones will simply be made positive.
 *
 * @return
 *  The number of diatonic steps in the interval.
 */

int chromatic2diatonic(int semitones)
      {
      static int il[12] = {
            0,    // Perfect Unison
            3,    // Minor Second
            4,    // Major Second
            7,    // Minor Third
            8,    // Major Third
            11,   // Perfect Fourth
            12,   // Augmented Fourth
            14,   // Perfect Fifth
            17,   // Minor Sixth
            18,   // Major Sixth
            21,   // Minor Seventh
            22,   // Major Seventh
            // 25    Perfect Octave
            };
      bool down = semitones < 0;
      if (down)
            semitones = -semitones;
      int val = semitones % 12;
      int octave = semitones / 12;
      int intervalIndex = il[val];
      int steps = intervalList[intervalIndex].diatonic;
      steps = steps + octave * 7;
      return down ? -steps : steps;
      }

//---------------------------------------------------------
//   searchInterval
//---------------------------------------------------------

int searchInterval(int steps, int semitones)
      {
      unsigned n = sizeof(intervalList)/sizeof(*intervalList);
      for (unsigned i = 0; i < n; ++i) {
            if ((intervalList[i].diatonic == steps) && (intervalList[i].chromatic == semitones))
                  return i;
            }
      return -1;
      }

/*!
 * Transposes both pitch and spelling for a note given an interval.
 *
 * Uses addition for pitch and transposeTpc() for spelling.
 *
 * @param pitch
 *  The initial (current) pitch. (pitch)
 * @param tpc
 *  The initial spelling. (tpc)
 * @param rpitch
 *  A pointer to the transposed pitch, calculated by this function. (pitch)
 * @param rtpc
 *  A pointer to the transposed spelling. (tcp)
 * @param interval
 *  The interval to transpose by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharbs or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 */

void transposeInterval(int pitch, int tpc, int* rpitch, int* rtpc, Interval interval,
   bool useDoubleSharpsFlats)
      {
      *rpitch   = pitch + interval.chromatic;
      *rtpc = transposeTpc(tpc, interval, useDoubleSharpsFlats);

      }

/*!
 * Transposes a pitch spelling given an interval.
 *
 * This function transposes a pitch spelling using first
 * a diatonic transposition and then calculating any accidentals.
 * This insures that the note is changed by the correct number of
 * scale degrees unless it would require too many accidentals.
 *
 * @param tpc
 *  The initial pitch spelling.
 * @param interval
 *  The interval to be transposed by.
 * @param useDoubleSharpsFlats
 *  Determines whether the output may include double sharps or flats (Abb)
 *  or should use an enharmonic pitch (Abb = G).
 *
 * @return
 *  The transposed pitch spelling (tpc).
 */

int transposeTpc(int tpc, Interval interval, bool useDoubleSharpsFlats)
      {
      if (tpc == INVALID_TPC) // perfect unison & perfect octave
            return tpc;

      int minAlter;
      int maxAlter;
      if (useDoubleSharpsFlats) {
            minAlter = -2;
            maxAlter = 2;
            }
      else {
            minAlter = -1;
            maxAlter = 1;
            }
      int steps     = interval.diatonic;
      int semitones = interval.chromatic;

printf("transposeTpc tpc %d steps %d semitones %d\n", tpc, steps, semitones);
      if (semitones == 0 && steps == 0)
            return tpc;

      int step, alter;
      int pitch = tpc2pitch(tpc);

      for (int k = 0; k < 10; ++k) {
            step = tpc2step(tpc) + steps;
            while (step < 0)
                  step += 7;
            step   %= 7;
            int p1 = tpc2pitch(step2tpc(step, 0));
            alter  = semitones - (p1 - pitch);
            // alter  = p1 + semitones - pitch;
            if (alter < 0) {
                  alter *= -1;
                  alter = 12 - alter;
                  }
            alter %= 12;
	          if (alter > 6)
	               alter -= 12;
	          if (alter > maxAlter)
                  ++steps;
            else if (alter < minAlter)
                  --steps;
            else
                  break;
            printf("  again alter %d steps %d, step %d\n", alter, steps, step);
            }
      printf("  = step %d alter %d  tpc %d\n", step, alter, step2tpc(step, alter));
      return step2tpc(step, alter);
      }

static int _majorVersion, _minorVersion, _updateVersion;

/*!
 * Returns the program version
 *
 * @return
 *  Version in the format: MMmmuu
 *  Where M=Major, m=minor, and u=update
 */

int version()
      {
      QRegExp re("(\\d+)\\.(\\d+)\\.(\\d+)");
      if (re.indexIn(VERSION) != -1) {
            QStringList sl = re.capturedTexts();
            if (sl.size() == 4) {
                  _majorVersion = sl[1].toInt();
                  _minorVersion = sl[2].toInt();
                  _updateVersion = sl[3].toInt();
                  return _majorVersion * 10000 + _minorVersion * 100 + _updateVersion;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   majorVersion
//---------------------------------------------------------

int majorVersion()
      {
      version();
      return _majorVersion;
      }

//---------------------------------------------------------
//   minorVersion
//---------------------------------------------------------

int minorVersion()
      {
      version();
      return _minorVersion;
      }

//---------------------------------------------------------
//   updateVersion
//---------------------------------------------------------

int updateVersion()
      {
      version();
      return _updateVersion;
      }

//---------------------------------------------------------
//   diatonicUpDown
//    used to find the second note of a trill, mordent etc.
//    key  -7 ... +7
//---------------------------------------------------------

int diatonicUpDown(int key, int pitch, int steps)
      {
      static int ptab[15][7] = {
//             c  c#   d  d#    e   f  f#   g  g#  a  a#   b
            { -1,      1,       3,  5,      6,     8,      10 },     // Ces
            { -1,      1,       3,  5,      6,     8,      10 },     // Ges
            {  0,      1,       3,  5,      6,     8,      10 },     // Des
            {  0,      1,       3,  5,      7,     8,      10 },     // As
            {  0,      2,       3,  5,      7,     8,      10 },     // Es
            {  0,      2,       3,  5,      7,     9,      10 },     // B
            {  0,      2,       4,  5,      7,     9,      10 },     // F

            {  0,      2,       4,  5,      7,     9,      11 },     // C

            {  0,      2,       4,  6,      7,     9,      11 },     // G
            {  1,      2,       4,  6,      7,     9,      11 },     // D
            {  1,      2,       4,  6,      8,     9,      11 },     // A
            {  1,      3,       4,  6,      8,     9,      11 },     // E
            {  1,      3,       4,  6,      8,    10,      11 },     // H
            {  1,      3,       5,  6,      8,    10,      11 },     // Fis
            {  1,      3,       5,  6,      8,    10,      12 },     // Cis
            };

      key += 7;
      int step   = pitch % 12;
      int octave = pitch / 12;

      for (int i = 0; i < 7; ++i) {
            if (ptab[key][i] == step) {
                  if (steps > 0) {
                        while (steps--) {
                              ++i;
                              if (i == 7) {
                                    ++octave;
                                    i = 0;
                                    }
                              }
                        }
                  else {
                        while (++steps != 0) {
                              --i;
                              if (i < 0) {
                                    i = 6;
                                    --octave;
                                    }
                              }
                        }
                  step = ptab[key][i];
                  break;
                  }
            }
      pitch = octave  * 12 + step;
      if (pitch < 0)
            pitch = 0;
      if (pitch > 127)
            pitch = 128;
      return pitch;
      }

//---------------------------------------------------------
//   searchTieNote
//    search Note to tie to "note"
//---------------------------------------------------------

Note* searchTieNote(Note* note)
      {
      Note* note2  = 0;
      Chord* chord = note->chord();
      Segment* seg = chord->segment();
      Part* part   = chord->staff()->part();
      int strack   = part->staves()->front()->idx() * VOICES;
      int etrack   = strack + part->staves()->size() * VOICES;

      while ((seg = seg->next1(SegChordRest))) {
            bool noteFound = false;
            for (int track = strack; track < etrack; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                  if (cr == 0 || cr->type() != CHORD)
                        continue;
                  int staffIdx = cr->staffIdx() + cr->staffMove();
                  if (staffIdx != chord->staffIdx())
                        continue;
                  foreach(Note* n, static_cast<Chord*>(cr)->notes()) {
                        if (n->pitch() == note->pitch()) {
                              if (note2 == 0 || note->chord()->track() == chord->track())
                                    note2 = n;
                              }
                        else if (cr->track() == chord->track())
                              noteFound = true;
                        }
                  }
            if (noteFound || note2)
                  break;
            }
      return note2;
      }

