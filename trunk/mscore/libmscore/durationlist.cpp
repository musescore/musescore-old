//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id: duration.h 5151 2011-12-29 09:10:31Z wschweer $
//
//  Copyright (C) 2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "durationlist.h"
#include "measure.h"
#include "segment.h"
#include "rest.h"
#include "chord.h"
#include "score.h"
#include "slur.h"
#include "note.h"
#include "tuplet.h"
#include "utils.h"

//---------------------------------------------------------
//   moveSlur
//---------------------------------------------------------

static void moveSlur(ChordRest* src, ChordRest* dst, QHash<Slur*, Slur*>* map)
      {
      foreach(Slur* oldSlur, src->slurFor()) {
            Slur* newSlur = new Slur(*oldSlur);
            map->insert(oldSlur, newSlur);
            dst->addSlurFor(newSlur);
            newSlur->setStartElement(dst);
            }
      foreach(Slur* oldSlur, src->slurBack()) {
            Slur* newSlur = map->value(oldSlur);
            if (newSlur) {
                  dst->addSlurBack(newSlur);
                  newSlur->setEndElement(dst);
                  map->remove(oldSlur);
                  }
            else {
                  printf("moveSlur: slur not found\n");
                  // TODO: handle failure case:
                  // - remove start slur from chord/rest
                  // - delete slur
                  }
            }
      }

//---------------------------------------------------------
//   append
//---------------------------------------------------------

void DurationList::append(DurationElement* e, QHash<Slur*, Slur*>* map)
      {
      Q_ASSERT(e->type() == TUPLET || e->type() == CHORD || e->type() == REST);

      _duration += e->duration();

      if (e->type() == TUPLET || e->type() == CHORD || isEmpty() || back()->type() != REST) {
            Element* element = e->clone();
            QList<DurationElement*>::append(static_cast<DurationElement*>(element));
            if (element->type() == TUPLET) {
                  Tuplet* srcTuplet = static_cast<Tuplet*>(e);
                  Tuplet* dstTuplet = static_cast<Tuplet*>(element);
                  foreach(const DurationElement* de, srcTuplet->elements())
                        dstTuplet->add(de->clone());
                  }
            else if (map) {
                  ChordRest* src = static_cast<ChordRest*>(e);
                  ChordRest* dst = static_cast<ChordRest*>(element);
                  moveSlur(src, dst, map);
                  }
            }
      else {
            // akkumulate rests
            DurationElement* rest = back();
            Fraction d = rest->duration();
            d += e->duration();
            rest->setDuration(d);
            }
      }

//---------------------------------------------------------
//   appendGap
//---------------------------------------------------------

void DurationList::appendGap(const Fraction& d)
      {
      _duration += d;
      if (!isEmpty() && (back()->type() == REST)) {
            DurationElement* rest = back();
            Fraction dd = rest->duration();
            dd += rest->duration();
            rest->setDuration(dd);
            }
      else {
            Rest* rest = new Rest(0);
            rest->setDuration(d);
            QList<DurationElement*>::append(static_cast<DurationElement*>(rest));
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void DurationList::read(int track, const Segment* fs, const Segment* es,
   QHash<Slur*, Slur*>* map)
      {
      int tick = fs->tick();
      int gap = 0;
      for (const Segment* s = fs; s; s = s->next1()) {
            if (s->subtype() != SegChordRest)
                  continue;
            DurationElement* e = static_cast<ChordRest*>(s->element(track));
            if (!e)
                  continue;
            gap = s->tick() - tick;
            if (e->tuplet()) {
                  Tuplet* tuplet = e->tuplet();
                  if (tuplet->elements().front() != e) {
                        qDebug("DurationList::read: cannot start in middle of tuplet");
                        abort();
                        }
                  e = tuplet;

                  // find last chord/rest in (possibly nested) tuplet:
                  DurationElement* de = tuplet;
                  while (de) {
                        de = tuplet->elements().back();
                        if (de->type() != TUPLET)
                              break;
                        }
                  s = static_cast<ChordRest*>(de)->segment();
                  // continue with first chord/rest after tuplet
                  }
            if (gap)
                  appendGap(Fraction::fromTicks(gap));
            append(e, map);
            tick += e->duration().ticks();;
            if (s == es)
                  break;
            }
      gap = es->tick() - tick;
      if (gap) {
            appendGap(Fraction::fromTicks(gap));
            }
      }

//---------------------------------------------------------
//   writeTuplet
//---------------------------------------------------------

Tuplet* DurationList::writeTuplet(Tuplet* tuplet, Measure* measure, int tick) const
      {
      Tuplet* dt = tuplet->clone();
      dt->setParent(measure);
      foreach (DurationElement* e, tuplet->elements()) {
            if (e->isChordRest()) {
                  Element* ne = e->clone();
                  Segment* segment = measure->getSegment(SegChordRest, tick);
                  segment->add(ne);
                  dt->add(ne);
                  }
            else {
                  Tuplet* nt = writeTuplet(static_cast<Tuplet*>(e), measure, tick);
                  dt->add(nt);
                  }
            tick += e->globalDuration().ticks();
            }
      return dt;
      }

//---------------------------------------------------------
//   canWrite
//    check if list can be written to measure list m
//---------------------------------------------------------

bool DurationList::canWrite(const Fraction& measureLen) const
      {
      Fraction pos;
      Fraction rest = measureLen;
      foreach(DurationElement* e, *this) {
            Fraction duration = e->duration();
            if (duration > rest && e->type() == TUPLET)
                  return false;
            while (!duration.isZero()) {
                  if (e->type() == REST && duration >= rest && rest == measureLen) {
                        duration -= rest;
                        pos = measureLen;
                        }
                  else {
                        Fraction d = qMin(rest, duration);
                        duration -= d;
                        rest -= d;
                        pos += d;
                        }
                  if (pos == measureLen) {
                        pos  = Fraction();
                        rest = measureLen;
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   write
//    rewrite notes into measure list m
//---------------------------------------------------------

bool DurationList::write(int track, Measure* measure, QHash<Slur*, Slur*>* map) const
      {
      Fraction pos;
      Measure* m       = measure;
      Score* score     = m->score();
      Fraction rest    = m->len();
      Segment* segment = 0;
      foreach (DurationElement* e, *this) {
            Fraction duration = e->duration();
            if (duration > rest && e->type() == TUPLET) {
                  // cannot split tuplet
                  qDebug("DurationList::write: cannot split tuplet\n");
                  return false;
                  }
            while (duration.numerator() > 0) {
                  segment = m->getSegment(SegChordRest, m->tick() + pos.ticks());
                  if (e->type() == REST && (duration >= rest || e == back()) && rest == m->len()) {
                        //
                        // handle full measure rest
                        //
                        if ((track % VOICES) == 0) {
                              // write only for voice 1
                              Rest* r = new Rest(score, TDuration::V_MEASURE);
                              r->setDuration(m->len());
                              r->setTrack(track);
                              segment->add(r);
                              }
                        duration -= m->len();
                        pos      += m->len();
                        rest.set(0, 1);
                        }
                  else {
                        Fraction d = qMin(rest, duration);
                        if (e->type() == REST) {
                              Rest* r = new Rest(score, TDuration(d));
                              r->setTrack(track);
                              segment->add(r);
                              moveSlur(static_cast<ChordRest*>(e), r, map);
                              duration -= d;
                              rest -= d;
                              pos += d;
                              }
                        else if (e->type() == CHORD) {
                              Chord* c = static_cast<Chord*>(e)->clone();
                              c->setScore(score);
                              c->setTrack(track);
                              c->setDuration(d);
                              c->setDurationType(TDuration(d));
                              moveSlur(static_cast<ChordRest*>(e), c, map);
                              segment->add(c);
                              duration -= d;
                              rest     -= d;
                              pos      += d;
                              foreach(Note* note, c->notes()) {
                                    if (!duration.isZero() || note->tieFor()) {
                                          Tie* tie = new Tie(score);
                                          note->add(tie);
                                          }
                                    else
                                          note->setTieFor(0);
                                    note->setTieBack(0);
                                    }
                              }
                        else if (e->type() == TUPLET) {
                              writeTuplet(static_cast<Tuplet*>(e), m, m->tick() + pos.ticks());
                              duration -= d;
                              rest     -= d;
                              pos      += d;
                              }
                        }
                  if (pos == m->len()) {
                        pos  = Fraction();
                        m    = m->nextMeasure();
                        if (m)
                              rest = m->len();
                        }
                  }
            }

      //
      // connect ties
      //
      for (Segment* s = measure->first(); s; s = s->next1()) {
            Element* el = s->element(track);
            if (el == 0 || el->type() != CHORD)
                  continue;
            foreach(Note* n, static_cast<Chord*>(el)->notes()) {
                  Tie* tie = n->tieFor();
                  if (!tie)
                        continue;
                  Note* nn = searchTieNote(n);
                  if (nn) {
                        tie->setEndNote(nn);
                        nn->setTieBack(tie);
                        }
                  }
            if (s == segment)
                  break;
            }
      return true;
      }

//---------------------------------------------------------
//   canWrite
//---------------------------------------------------------

bool ScoreRange::canWrite(const Fraction& f) const
      {
      foreach(DurationList dl, tracks) {
            if (!dl.canWrite(f))
                  return false;
            }
      return true;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void ScoreRange::read(Segment* first, Segment* last, int startTrack, int endTrack)
      {
      spannerMap.clear();
      for (int track = startTrack; track < endTrack; ++track) {
            DurationList dl;
            dl.read(track, first, last, &spannerMap);
            tracks.append(dl);
            }
      if (!spannerMap.isEmpty())
            printf("ScoreRange::read(): dangling Slurs\n");
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

bool ScoreRange::write(int track, Measure* m) const
      {
      spannerMap.clear();
      foreach(DurationList dl, tracks) {
            if (!dl.write(track, m, &spannerMap))
                  return false;
            ++track;
            }
      if (!spannerMap.isEmpty())
            printf("ScoreRange::write(): dangling Slurs\n");
      return true;
      }

//---------------------------------------------------------
//   duration
//---------------------------------------------------------

Fraction ScoreRange::duration() const
      {
      return tracks.isEmpty() ? Fraction() : tracks[0].duration();
      }

