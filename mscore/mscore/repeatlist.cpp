//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "repeatlist.h"
#include "score.h"
#include "measure.h"
#include "repeat.h"
#include "al/tempo.h"
#include "volta.h"

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
        
      QSet<Jump*> usedJump;
      QStack<RepeatLoop> rstack;
      int tickOffset = 0;
      
      //
      // check for necessary implicit repeat start
      //
      int loopCount = 0;
      for (Measure* m = _score->firstMeasure(); m; m = m->nextMeasure()) {
            if (m->repeatFlags() & RepeatStart)
                  ++ loopCount;
            if (m->repeatFlags() & RepeatEnd)
                  --loopCount;
            }
      bool implicitRepeat = loopCount == -1;
      if (implicitRepeat)
            rstack.push(RepeatLoop(_score->firstMeasure()));
      else if (loopCount != 0)
            printf("unbalanced repeat start-end\n");

      RepeatSegment* rs = new RepeatSegment;
      rs->tick       = 0;
      rs->utick      = 0;
      rs->utime      = 0.0;
      rs->timeOffset = 0.0;

      for (Measure* m = _score->firstMeasure(); m;) {
            int flags = m->repeatFlags();
            //
            //  check for repeat start
            //
            if ((flags & RepeatStart) && (rstack.isEmpty() || rstack.top().m != m)) {
// printf("startRepeat stack empty %d --- %p != %p\n", rstack.isEmpty(),
//                  rstack.isEmpty() ? 0 : rstack.top().m, m);
                  rstack.push(RepeatLoop(m));
                  }

            //
            // Voltas outside of loops are ignored.
            // Voltas on first measure of a loop are also ignored. Its
            // assumed that they belong to a previous loop.
            //
            if (!rstack.isEmpty() && !(flags & RepeatStart)) {
                  Volta* volta = _score->searchVolta(m->tick());
                  int n = rstack.top().count + 1;
                  if (volta && (rstack.size() > 1 || !volta->hasEnding(n))
                     && (rstack.top().type == RepeatLoop::LOOP_REPEAT))
                        {
                        tickOffset -= m->tickLen();   // skip this measure
                        rs->len = m->tick() - rs->tick;
                        if (rs->len) {
                              append(rs);
                              rs = new RepeatSegment;
                              }
                        rs->tick  = m->tick() + m->tickLen();
                        rs->utick = rs->tick + tickOffset;
                        }
                  }

            if (rstack.isEmpty()) {
                  // Jumps are only accepted outside of other repeats
                  if (flags & RepeatJump) {
                        Jump* s = 0;
                        foreach(Element* e, *m->el()) {
                              if (e->type() == JUMP) {
                                    s = static_cast<Jump*>(e);
                                    break;
                                    }
                              }
                        if (s && !usedJump.contains(s)) {
                              const QString& jumpTo = s->jumpTo();
                              Measure* nmb = _score->searchLabel(jumpTo);
                              if (nmb) {
                                    usedJump.insert(s);
                                    rstack.push(RepeatLoop(s->playUntil(), s->continueAt()));
                                    tickOffset += m->tick() + m->tickLen() - nmb->tick();

                                    rs->len = m->tick() + m->tickLen() - rs->tick;
                                    append(rs);
                                    rs = new RepeatSegment;
                                    rs->tick  = nmb->tick();
                                    rs->utick = rs->tick + tickOffset;

                                    m = nmb;
                                    if (implicitRepeat && m == _score->firstMeasure()) {
                                          rstack.push(RepeatLoop(m));
                                          }
                                    continue;
                                    }
                              else
                                    printf("JUMP: label <%s> not found\n",
                                       qPrintable(jumpTo));
                              }
                        else
                              printf("Jump not found\n");
                        }
                  }
            else if (rstack.top().type == RepeatLoop::LOOP_REPEAT) {
                  if (flags & RepeatEnd) {
                        //
                        // increment repeat count
                        //
                        // CHECK for nested repeats:
                        //    repeat a repeat inside a repeat only on first
                        //    pass of outer reapeat (?!?)
                        //
                        bool nestedRepeat = false;
                        int n = rstack.size();
                        if (n > 1 && rstack[n-2].type == RepeatLoop::LOOP_REPEAT) {
                              if (rstack[n-2].count)
                                    nestedRepeat = true;
                              }
                        //
                        // do not repeat in a jump (d.c. al fine etc.)
                        //
                        if (n > 1 && rstack[n-2].type == RepeatLoop::LOOP_JUMP) {
                              nestedRepeat = true;
                              }
                        if (!nestedRepeat && (++rstack.top().count < m->repeatCount())) {
                              //
                              // goto start of loop, fix tickOffset
                              //

                              Measure* nm = rstack.top().m;

                              rs->len = m->tick() + m->tickLen() - rs->tick;
                              append(rs);

                              tickOffset += m->tick() + m->tickLen() - nm->tick();

                              rs = new RepeatSegment;
                              rs->tick  = nm->tick();
                              rs->utick = nm->tick() + tickOffset;

                              m = nm;
                              continue;
                              }
                        if (!rstack.isEmpty()) {
                              rstack.pop();     // end this loop
                              if (flags & RepeatStart)      // if start/end repeat in one measure
                                    m = m->nextMeasure();
                              }
                        else
                              printf("repeatStack:: cannot pop\n");
                        continue;
                        }
                  }
            if (!rstack.isEmpty() && (rstack.top().type == RepeatLoop::LOOP_JUMP)) {
                  Measure* nm = _score->searchLabel(rstack.top().stop);
                  if (nm == 0)
                        printf("LOOP_JUMP: stop label <%s> not found\n", qPrintable(rstack.top().stop));
                  if (nm == m) {
                        if (m->nextMeasure() == 0)
                              break;
                        Measure* nmb = _score->searchLabel(rstack.top().cont, nm->nextMeasure());
                        if (nmb) {
                              tickOffset += m->tick() + m->tickLen() - nmb->tick();
                              rs->len = m->tick() + m->tickLen() - rs->tick;
                              append(rs);
                              rs = new RepeatSegment;
                              rs->tick  = nmb->tick();
                              rs->utick = rs->tick + tickOffset;
                              }
                        else if (!rstack.top().cont.isEmpty())
                              printf("Cont label <%s> not found\n", qPrintable(rstack.top().cont));
                        else {
                              rs->len = m->tick() + m->tickLen() - rs->tick;
                              append(rs);
                              rs = 0;
                              }

                        m = nmb;
                        if (!rstack.isEmpty())
                              rstack.pop();     // end this loop
                        else
                              printf("repeatStack:: cannot pop\n");
                        continue;
                        }
                  }
            m = m->nextMeasure();
            }
      if (!rstack.isEmpty()) {
            if (rstack.top().type == RepeatLoop::LOOP_JUMP
               && rstack.top().stop == "end")
                  ;
            else
                  printf("repeat stack not empty!\n");
            }

      Measure* lm = _score->lastMeasure();
      if (rs) {
            rs->len = lm->tick() - rs->tick + lm->tickLen();
            if (rs->len)
                  append(rs);
            else
                  delete rs;
            }
      update();
      }

