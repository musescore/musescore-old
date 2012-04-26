//=============================================================================
//  MuseScore
//  Music Composition & Notation
//
//  Copyright (C) 2002-2012 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "score.h"
#include "measure.h"
#include "segment.h"
#include "chord.h"
#include "note.h"
#include "slur.h"
#include "tuplet.h"
#include "slurmap.h"
#include "tiemap.h"
#include "tupletmap.h"
#include "undo.h"

//---------------------------------------------------------
//   addCR
//    tick - insert position in measure list m
//    cr   - Chord/Rest to insert
//    m    - list of measures starting at tick 0
//
//    return true on success
//---------------------------------------------------------

static bool addCR(int tick, ChordRest* cr, Measure* ml)
      {
      Measure* m = ml;
      int mticks = m->ticks();
      for (;m; m = m->nextMeasure()) {
            if (tick >= m->tick() && tick < (m->tick() + mticks))
                  break;
            }
      if (m == 0) {
            qDebug("addCR: cannot insert cr: list too short");
            return false;
            }
      int etick = m->tick() + m->ticks();
      Tuplet* tuplet = cr->tuplet();
      while (tuplet && tuplet->tuplet())
            tuplet = tuplet->tuplet();
      if (tuplet && (tick + tuplet->actualTicks() > etick))
            return false;

      if (tick + cr->actualTicks() > etick) {
            //
            // split cr
            //
            Fraction len(cr->duration());
            Chord* chord = cr->type() == CHORD ? static_cast<Chord*>(cr) : 0;
            if (chord) {
                  int notes = chord->notes().size();
                  Tie* ties[notes];
                  for (int i = 0; i < notes; ++i)
                        ties[i] = 0;
                  while (!len.isZero()) {
                        Fraction rest = Fraction::fromTicks(m->tick() + m->ticks() - tick);
                        if (rest > len)
                              rest = len;
                        QList<TDuration> dList = toDurationList(rest, false);
                        if (dList.isEmpty())
                              return true;
                        int n = dList.size();
                        for (int i = 0; i < n; ++i) {
                              const TDuration& d = dList[i];
                              Chord* c = static_cast<Chord*>(chord->clone());
                              c->setSelected(false);
                              if (i == 0) {
                                    foreach(Spanner* s, chord->spannerBack()) {
                                          if (s->type() == SLUR)
                                                c->addSlurBack(static_cast<Slur*>(s));
                                          }
                                    }
                              if (i == n-1) {
                                    foreach(Spanner* s, chord->spannerFor()) {
                                          if (s->type() == SLUR)
                                                c->addSlurFor(static_cast<Slur*>(s));
                                          }
                                    }
                              for (int i = 0; i < notes; ++i) {
                                    Tie* tie = ties[i];
                                    Note* note = c->notes().at(i);
                                    if (tie == 0) {
                                          tie = new Tie(c->score());
                                          ties[i] = tie;
                                          tie->setTrack(c->track());
                                          tie->setStartNote(note);
                                          note->setTieFor(tie);
                                          }
                                    else {
                                          tie->setEndNote(note);
                                          note->setTieBack(tie);
                                          ties[i] = 0;
                                          }
                                    }
                              c->setDurationType(d);
                              c->setDuration(d.fraction());
                              Segment* s = m->getSegment(SegChordRest, tick);
                              s->add(c);
                              tick += c->actualTicks();
                              }
                        len -= rest;
                        m = m->nextMeasure();
                        }
                  for (int i = 0; i < notes; ++i)
                        delete ties[i];
                  }
            else {
                  while (!len.isZero()) {
                        Fraction rest = Fraction::fromTicks(m->tick() + m->ticks() - tick);
                        if (rest > len)
                              rest = len;
                        QList<TDuration> dList = toDurationList(rest, false);
                        if (dList.isEmpty())
                              return true;
                        foreach(TDuration d, dList) {
                              ChordRest* cr1 = static_cast<ChordRest*>(cr->clone());
                              cr1->setDuration(d.fraction());
                              if (m->len() == d.fraction())
                                    cr1->setDurationType(TDuration::V_MEASURE);
                              else
                                    cr1->setDurationType(d);
                              Segment* s = m->getSegment(SegChordRest, tick);
                              s->add(cr1);
                              tick += cr1->actualTicks();
                              }
                        len -= rest;
                        m = m->nextMeasure();
                        if (m == 0) {
                              qDebug("eof: len %d %d", len.numerator(), len.denominator());
                              break;
                              }
                        }
                  }

            delete cr;
            }
      else {
            Segment* s = m->getSegment(SegChordRest, tick);
            s->add(cr);
            }
      return true;
      }

//---------------------------------------------------------
//   cmdSplitMeasure
//---------------------------------------------------------

void Score::cmdSplitMeasure(ChordRest* cr)
      {
      Segment* segment = cr->segment();
      Measure* measure = segment->measure();

      //TODO: check for split in tuplet
      if (cr->segment()->tick() == measure->tick()) {
            qDebug("cannot split here");
            return;
            }
      startCmd();
      deleteItem(measure);

      // create empty measures:
      Measure* m2 = static_cast<Measure*>(insertMeasure(MEASURE, measure->next(), true));
      Measure* m1 = static_cast<Measure*>(insertMeasure(MEASURE, m2, true));

      int tick = segment->tick();
      m1->setTick(measure->tick());
      m2->setTick(tick);
      int ticks1 = segment->tick() - measure->tick();
      int ticks2 = measure->ticks() - ticks1;
      m1->setTimesig(measure->timesig());
      m2->setTimesig(measure->timesig());
      m1->setLen(Fraction::fromTicks(ticks1));
      m2->setLen(Fraction::fromTicks(ticks2));

      int tracks = nstaves() * VOICES;
      SlurMap* slurMap = new SlurMap[tracks];
      TieMap* tieMap   = new TieMap[tracks];

      for (int track = 0; track < tracks; ++track) {
            TupletMap tupletMap;
            for (Segment* s = measure->first(); s; s = s->next()) {
                  Element* oe = s->element(track);
                  if (oe == 0)
                        continue;
                  SegmentType st = s->subtype();
                  Measure* m     = (s->tick() < tick) ? m1 : m2;
                  Segment* seg   = m->getSegment(st, s->tick());
                  Element* ne    = oe->clone();
                  if (st != SegChordRest) {
                        seg->add(ne);
                        continue;
                        }

                  ChordRest* ocr = static_cast<ChordRest*>(oe);
                  ChordRest* ncr = static_cast<ChordRest*>(ne);
                  if (s->tick() < tick && (s->tick() + ocr->actualTicks()) > tick) {
                        addCR(s->tick(), ncr, m1);
                        }
                  else {
                        Tuplet* ot = ocr->tuplet();
                        if (ot) {
                              Tuplet* nt = tupletMap.findNew(ot);
                              if (nt == 0) {
                                    nt = new Tuplet(*ot);
                                    nt->clear();
                                    m->add(nt);
                                    tupletMap.add(ot, nt);
                                    }
                              ncr->setTuplet(nt);
                              }
                        seg->add(ncr);
                        }
#if 0
                  foreach (Slur* s, ocr->slurFor()) {
                        Slur* slur = new Slur(this);
                        slur->setStartElement(ncr);
                        ncr->addSlurFor(slur);
                        slurMap[track].add(s, slur);
                        }
                  foreach (Slur* s, ocr->slurBack()) {
                        Slur* slur = slurMap[track].findNew(s);
                        if (slur) {
                              slur->setEndElement(ncr);
                              ncr->addSlurBack(slur);
                              }
                        else {
                              qDebug("cloneStave: cannot find slur");
                              }
                        }
                  foreach (Element* e, seg->annotations()) {
                        if (e->generated() || e->systemFlag())
                              continue;
                        Element* ne = e->clone();
                        seg->add(ne);
                        }
                  if (oe->type() == CHORD) {
                        Chord* och = static_cast<Chord*>(ocr);
                        Chord* nch = static_cast<Chord*>(ncr);
                        int n = och->notes().size();
                        for (int i = 0; i < n; ++i) {
                              Note* on = och->notes().at(i);
                              Note* nn = nch->notes().at(i);
                              if (on->tieFor()) {
                                    Tie* tie = new Tie(this);
                                    nn->setTieFor(tie);
                                    tie->setStartNote(nn);
                                    tieMap[track].add(on->tieFor(), tie);
                                    }
                              if (on->tieBack()) {
                                    Tie* tie = tieMap[track].findNew(on->tieBack());
                                    if (tie) {
                                          nn->setTieBack(tie);
                                          tie->setEndNote(nn);
                                          }
                                    else {
                                          qDebug("cloneStave: cannot find tie");
                                          }
                                    }
                              }
                        }
#endif
                  }
            }
      delete[] slurMap;
      delete[] tieMap;
      endCmd();
      }


