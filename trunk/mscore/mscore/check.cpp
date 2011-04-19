//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer and others
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

#include "score.h"
#include "slur.h"
#include "measure.h"
#include "tuplet.h"
#include "chordrest.h"
#include "rest.h"
#include "segment.h"
#include "staff.h"

//---------------------------------------------------------
//   checkSlurs
//    helper routine to check for sanity slurs
//---------------------------------------------------------

void Score::checkSlurs()
      {
#if 0 //TODO1
      foreach(Element* e, _gel) {
            if (e->type() != SLUR)
                  continue;
            Slur* s = (Slur*)e;
            Element* n1 = s->startElement();
            Element* n2 = s->endElement();
            if (n1 == 0 || n2 == 0 || n1 == n2) {
                  printf("unconnected slur: removing\n");
                  if (n1) {
                        ((ChordRest*)n1)->removeSlurFor(s);
                        ((ChordRest*)n1)->removeSlurBack(s);
                        }
                  if (n1 == 0)
                        printf("  start at %d(%d) not found\n", s->tick(), s->track());
                  if (n2 == 0)
                        printf("  end at %d(%d) not found\n", s->tick2(), s->track2());
                  if ((n1 || n2) && (n1==n2))
                        printf("  start == end\n");
                  int idx = _gel.indexOf(s);
                  _gel.removeAt(idx);
                  }
            }
#endif
      }

//---------------------------------------------------------
//   checkTuplets
//    helper routine to check for tuplet sanity
//---------------------------------------------------------

void Score::checkTuplets()
      {
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* t, *m->tuplets()) {
                  if (t->elements().empty()) {
printf("empty tuplet: removing\n");
                        m->tuplets()->removeAll(t);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   checkScore
//---------------------------------------------------------

void Score::checkScore()
      {
      for (Segment* s = firstMeasure()->first(); s;) {
            Segment* ns = s->next1();

            if (s->subtype() & (SegChordRest | SegGrace)) {
                  bool empty = true;
                  foreach(Element* e, s->elist()) {
                        if (e) {
                              empty = false;
                              break;
                              }
                        }
                  if (empty) {
                        // Measure* m = s->measure();
printf("checkScore: remove empty ChordRest segment\n");
//                        m->remove(s);
                        }
                  }
            s = ns;
            }

      checkSlurs();
      checkTuplets();

      ChordRest* lcr;
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            int track = staffIdx * VOICES;
            int tick  = 0;
            Staff* st = staff(staffIdx);
            for (Segment* s = firstMeasure()->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (!cr)
                        continue;
                  if (s->tick() != tick) {
                        if (lcr) {
                              Fraction timeStretch = st->timeStretch(lcr->tick());
                              Fraction f = cr->globalDuration() * timeStretch;
                              printf("Chord/Rest gap at tick %d(%s+%d)-%d(%s) staffIdx %d measure %d (len = %d)\n",
                                 tick, lcr->name(), f.ticks(),
                                 s->tick(), cr->name(), staffIdx, cr->measure()->no(),
                                 cr->tick() - tick);
                              }
                        else {
                              printf("Chord/Rest gap at tick %d-%d(%s) staffIdx %d measure %d (len = %d)\n",
                                 tick,
                                 s->tick(), cr->name(), staffIdx, cr->measure()->no(),
                                 cr->tick() - tick);
                              }
#if 0
                        if (cr->tick() > tick) {
                              int ttick = tick;
                              int ticks = cr->tick() - tick;

                              Fraction f = Fraction::fromTicks(ticks) / st->timeStretch(ttick);
                              printf("  insert %d/%d\n", f.numerator(), f.denominator());

                              while (ticks > 0) {
                                    Measure* m = tick2measure(ttick);
                                    int len    = ticks;
                                    // split notes on measure boundary
                                    if ((ttick + len) > m->tick() + m->ticks())
                                          len = m->tick() + m->ticks() - ttick;
                                    Fraction timeStretch = st->timeStretch(ttick);
                                    Fraction ff          = Fraction::fromTicks(len);
printf("    - insert %d/%d\n", ff.numerator(), ff.denominator());
                                    if (ff.numerator() == 0)
                                          break;
                                    Fraction fff = ff / timeStretch;

                                    QList<Duration> dl = toDurationList(fff, true);
                                    foreach(Duration d, dl) {
                                          Rest* rest = new Rest(this);
                                          rest->setDurationType(d);
                                          rest->setDuration(d.fraction());
                                          rest->setColor(Qt::red);
printf("    -   Rest %d/%d\n", d.fraction().numerator(), d.fraction().denominator());
                                          rest->setTrack(track);
                                          Segment* s = m->getSegment(rest, ttick);
                                          s->add(rest);
                                          ttick += (d.fraction() * timeStretch).ticks();
                                          }
                                    ticks -= len;
                                    }
                              }
#endif
                        tick = s->tick();
                        }
                  Fraction timeStretch = st->timeStretch(tick);
                  Fraction f = cr->globalDuration() * timeStretch;
//                  printf("%s %d + %d = %d\n", cr->name(), tick, f.ticks(), tick + f.ticks());
                  tick      += f.ticks();
                  lcr        = cr;
                  }
            }
      }

