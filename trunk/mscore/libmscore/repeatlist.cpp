//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2009-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "repeatlist.h"
#include "score.h"
#include "measure.h"
#include "repeat.h"
#include "al/tempo.h"
#include "volta.h"
#include "segment.h"

//---------------------------------------------------------
//   searchVolta
//    return volta at tick
//---------------------------------------------------------

Volta* Score::searchVolta(int tick) const
      {
      Measure* fm = firstMeasure();
      for (Segment* s = fm->first(SegChordRest); s; s = s->next1(SegChordRest)) {
            foreach(Spanner* e, s->spannerFor()) {
                  if (e->type() != VOLTA)
                        continue;
                  int tick1 = static_cast<Segment*>(e->startElement())->tick();
                  int tick2 = static_cast<Segment*>(e->endElement())->tick();

                  if (tick >= tick1 && tick < tick2)
                        return static_cast<Volta*>(e);
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   searchLabel
//---------------------------------------------------------

Measure* Score::searchLabel(const QString& s)
      {
      if (s == "start")
            return firstMeasure();
      for (Segment* segment = firstMeasure()->first(); segment; segment = segment->next1()) {
            foreach(const Element* e, segment->annotations()) {
                  if (e->type() == MARKER) {
                        const Marker* marker = static_cast<const Marker*>(e);
                        if (marker->label() == s)
                              return segment->measure();
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   RepeatLoop
//---------------------------------------------------------

struct RepeatLoop {
      enum LoopType { LOOP_REPEAT, LOOP_JUMP };

      LoopType type;
      Measure* m;   // start measure of LOOP_REPEAT
      int count;
      QString stop, cont;

      RepeatLoop() {}
      RepeatLoop(Measure* _m)  {
            m     = _m;
            count = 0;
            type  = LOOP_REPEAT;
            }
      RepeatLoop(const QString s, const QString c)
         : stop(s), cont(c)
            {
            m    = 0;
            type = LOOP_JUMP;
            }
      };

//---------------------------------------------------------
//   RepeatSegment
//---------------------------------------------------------

RepeatSegment::RepeatSegment()
      {
      tick       = 0;
      len        = 0;
      utick      = 0;
      utime      = 0.0;
      timeOffset = 0.0;
      }

//---------------------------------------------------------
//   RepeatList
//---------------------------------------------------------

RepeatList::RepeatList(Score* s)
      {
      _score = s;
      idx1  = 0;
      idx2  = 0;
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void RepeatList::update()
      {
      const AL::TempoMap* tl = _score->tempomap();

      int utick = 0;
      double t  = 0;

      foreach(RepeatSegment* s, *this) {
            s->utick      = utick;
            s->utime      = t;
            double ct     = tl->tick2time(s->tick);
            s->timeOffset = t - ct;
            utick        += s->len;
            t            += tl->tick2time(s->tick + s->len) - ct;
            }
      }

//---------------------------------------------------------
//   utick2tick
//---------------------------------------------------------

int RepeatList::utick2tick(int tick) const
      {
      unsigned n = size();
      unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i+1)->utick))) {
                  idx1 = i;
                  return tick - (at(i)->utick - at(i)->tick);
                  }
            }
      if (debugMode) {
            printf("utick %d not found in RepeatList\n", tick);
            abort();
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2utick
//---------------------------------------------------------

int RepeatList::tick2utick(int tick) const
      {
      foreach (const RepeatSegment* s, *this) {
            if (tick >= s->tick && tick < (s->tick + s->len))
                  return s->utick + (tick - s->tick);
            }
      return 0;
      }

//---------------------------------------------------------
//   utick2utime
//---------------------------------------------------------

double RepeatList::utick2utime(int tick) const
      {
      unsigned n = size();
      unsigned ii = (idx1 < n) && (tick >= at(idx1)->utick) ? idx1 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((tick >= at(i)->utick) && ((i + 1 == n) || (tick < at(i+1)->utick))) {
                  int t     = tick - (at(i)->utick - at(i)->tick);
                  double tt = _score->tempomap()->tick2time(t) + at(i)->timeOffset;
                  return tt;
                  }
            }
      return 0.0;
      }

//---------------------------------------------------------
//   utime2utick
//---------------------------------------------------------

int RepeatList::utime2utick(double t) const
      {
      unsigned n = size();
      unsigned ii = (idx2 < n) && (t >= at(idx2)->utime) ? idx2 : 0;
      for (unsigned i = ii; i < n; ++i) {
            if ((t >= at(i)->utime) && ((i + 1 == n) || (t < at(i+1)->utime))) {
                  idx2 = i;
                  return _score->tempomap()->time2tick(t - at(i)->timeOffset) + (at(i)->utick - at(i)->tick);
                  }
            }
      if (debugMode) {
            printf("time %f not found in RepeatList\n", t);
            abort();
            }
      return 0;
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void RepeatList::dump() const
      {
      printf("Dump Repeat List:\n");
      foreach(const RepeatSegment* s, *this) {
            printf("%p  %6d %6d len %d  %f + %f\n", s, s->utick, s->tick, s->len,
               s->utime, s->timeOffset);
            }
      }

//---------------------------------------------------------
//   unwind
//    implements:
//          - repeats
//          - volta
//          - d.c. al fine
//          - d.s. al fine
//          - d.s. al coda
//---------------------------------------------------------

void RepeatList::unwind()
      {
      foreach(RepeatSegment* s, *this)
            delete s;
      clear();

      rs                 = new RepeatSegment;
      rs->tick           = 0;
      Measure* endRepeat = 0;
      Measure* continueAt = 0;
      int loop           = 0;
      int repeatCount    = 0;
      bool isGoto        = false;

      Measure* fm = _score->firstMeasure();
      if (!fm)
            return;

      for (Measure* m = fm; m; m = m->nextMeasure())
            m->setPlaybackCount(0);

      for (Measure* m = fm; m;) {
            m->setPlaybackCount(m->playbackCount() + 1);
            int flags = m->repeatFlags();

//            printf("repeat m%d(%d) loop %d repeatCount %d isGoto %d endRepeat %p\n",
//               m->no(), m->playbackCount(), loop, repeatCount, isGoto, endRepeat);

            if (endRepeat) {
                  Volta* volta = _score->searchVolta(m->tick());
                  if (volta && !volta->hasEnding(m->playbackCount())) {
                        // skip measure
                        if (rs->tick < m->tick()) {
                              rs->len = m->tick() - rs->tick;
                              append(rs);
                              rs = new RepeatSegment;
                              }
                        rs->tick = m->tick() + m->ticks();
                        }
                  }
            else {
                  // Jumps are only accepted outside of other repeats
                  if (flags & RepeatJump) {
                        Jump* s = 0;
                        for (Segment* seg = m->first(); seg; seg = seg->next()) {
                              foreach(Element* e, seg->annotations()) {
                                    if (e->type() == JUMP) {
                                          s = static_cast<Jump*>(e);
                                          break;
                                          }
                                    }
                              if (s)
                                    break;
                              }
                        if (s) {
                              Measure* nm = _score->searchLabel(s->jumpTo());
                              endRepeat   = _score->searchLabel(s->playUntil());
                              continueAt  = _score->searchLabel(s->continueAt());
                              isGoto      = true;

                              if (nm) {
                                    rs->len = m->tick() + m->ticks() - rs->tick;
                                    append(rs);
                                    rs = new RepeatSegment;
                                    rs->tick  = nm->tick();
                                    m = nm;
                                    continue;
                                    }
                              }
                        else
                              printf("Jump not found\n");
                        }
                  }

            if (isGoto && (endRepeat == m)) {
                  if (continueAt == 0) {
                        rs->len = m->tick() + m->ticks() - rs->tick;
                        if (rs->len)
                              append(rs);
                        else
                              delete rs;
                        update();
                        return;
                        }
                  rs->len = m->tick() + m->ticks() - rs->tick;
                  append(rs);
                  rs       = new RepeatSegment;
                  rs->tick = continueAt->tick();
                  m        = continueAt;
                  isGoto   = false;
                  continue;
                  }
            else if (flags & RepeatEnd) {
                  if (endRepeat == m) {
                        ++loop;
                        if (loop >= repeatCount) {
                              endRepeat = 0;
                              loop = 0;
                              }
                        else {
                              m = jumpToStartRepeat(m);
                              }
                        }
                  else if (endRepeat == 0) {
                        endRepeat   = m;
                        repeatCount = m->repeatCount();
                        loop        = 1;
                        m = jumpToStartRepeat(m);
                        continue;
                        }
                  }
            m = m->nextMeasure();
            }

      if (rs) {
            Measure* lm = _score->lastMeasure();
            rs->len     = lm->tick() - rs->tick + lm->ticks();
            if (rs->len)
                  append(rs);
            else
                  delete rs;
            }
      update();
      }

//---------------------------------------------------------
//   jumpToStartRepeat
//---------------------------------------------------------

Measure* RepeatList::jumpToStartRepeat(Measure* m)
      {
      Measure* nm;
      for (nm = m; nm && nm != _score->firstMeasure(); nm = nm->prevMeasure()) {
            if (nm->repeatFlags() & RepeatStart)
                  break;
            }
      rs->len = m->tick() + m->ticks() - rs->tick;
      append(rs);

      rs        = new RepeatSegment;
      rs->tick  = nm->tick();
      return nm;
      }


