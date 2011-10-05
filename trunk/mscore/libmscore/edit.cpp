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

#include "note.h"
#include "rest.h"
#include "chord.h"
#include "key.h"
#include "sig.h"
#include "clef.h"
#include "score.h"
#include "slur.h"
#include "hairpin.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "timesig.h"
#include "page.h"
#include "barline.h"
#include "tuplet.h"
#include "lyrics.h"
#include "image.h"
#include "keysig.h"
#include "beam.h"
#include "utils.h"
#include "harmony.h"
#include "system.h"
#include "navigate.h"
#include "articulation.h"
#include "drumset.h"
#include "measure.h"
#include "tempo.h"
#include "undo.h"
#include "tablature.h"
#include "stafftype.h"
#include "tupletmap.h"
#include "slurmap.h"
#include "tiemap.h"
#include "stem.h"

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = selection().element();
      if (el) {
            if (el->type() == NOTE)
                  return static_cast<Note*>(el);
            }
      selectNoteMessage();
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest
//---------------------------------------------------------

ChordRest* Score::getSelectedChordRest() const
      {
      Element* el = selection().element();
      if (el) {
            if (el->type() == NOTE)
                  return static_cast<Note*>(el)->chord();
            else if (el->type() == REST || el->type() == REPEAT_MEASURE)
                  return static_cast<Rest*>(el);
            else if (el->type() == CHORD)
                  return static_cast<Chord*>(el);
            }
      selectNoteRestMessage();
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest2
//---------------------------------------------------------

void Score::getSelectedChordRest2(ChordRest** cr1, ChordRest** cr2) const
      {
      *cr1 = 0;
      *cr2 = 0;
      foreach(Element* e, selection().elements()) {
            if (e->type() == NOTE)
                  e = e->parent();
            if (e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if (*cr1 == 0 || (*cr1)->tick() > cr->tick())
                        *cr1 = cr;
                  if (*cr2 == 0 || (*cr2)->tick() < cr->tick())
                        *cr2 = cr;
                  }
            }
      if (*cr1 == 0)
            selectNoteRestMessage();
      if (*cr1 == *cr2)
            *cr2 = 0;
      }

//---------------------------------------------------------
//   pos
//---------------------------------------------------------

int Score::pos()
      {
      Element* el = selection().element();
      if (selection().activeCR())
            el = selection().activeCR();
      if (el) {
            switch(el->type()) {
                  case NOTE:
                        el = el->parent();
                        // fall through
                  case REPEAT_MEASURE:
                  case REST:
                  case CHORD:
                        return static_cast<ChordRest*>(el)->tick();
                  default:
                        break;
                  }
            }
      return -1;
      }

//---------------------------------------------------------
//   addRest
//    create one Rest at tick with duration d
//    create segment if necessary
//---------------------------------------------------------

Rest* Score::addRest(int tick, int track, Duration d, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      Rest* rest       = new Rest(this, d);
      if (d.type() == Duration::V_MEASURE)
            rest->setDuration(measure->stretchedLen(staff(track/VOICES)));
      else
            rest->setDuration(d.fraction());
      rest->setTrack(track);
      rest->setTuplet(tuplet);
// printf("addRest at %d/%d len %s\n", tick, track, qPrintable(d.name()));
      undoAddCR(rest, measure, tick);
      return rest;
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, int track, Duration d, Tuplet* tuplet)
      {
      Rest* rest = new Rest(this, d);
      if (d.type() == Duration::V_MEASURE)
            rest->setDuration(s->measure()->stretchedLen(staff(track/VOICES)));
      else
            rest->setDuration(d.fraction());
      rest->setTrack(track);
      rest->setParent(s);
      rest->setTuplet(tuplet);
      undoAddElement(rest);
      return rest;
      }

//---------------------------------------------------------
//   addChord
//    Create one Chord at tick with duration d
//    - create segment if necessary.
//    - Use chord "oc" as prototype;
//    - if "genTie" then tie to chord "oc"
//---------------------------------------------------------

Chord* Score::addChord(int tick, Duration d, Chord* oc, bool genTie, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      if (measure->endTick() <= tick) {
            printf("Score::addChord(): end of score?\n");
            return 0;
            }

      Chord* chord = new Chord(this);
      chord->setTuplet(tuplet);
      chord->setTrack(oc->track());
      chord->setDurationType(d);
      chord->setDuration(d.fraction());

      foreach(Note* n, oc->notes()) {
            Note* nn = new Note(this);
            chord->add(nn);
            nn->setPitch(n->pitch(), n->tpc());
            }
      undoAddCR(chord, measure, tick);

      //
      // now as both chords are in place
      // (have segments as parent) we can add ties:
      //
      if (genTie) {
            int n = oc->notes().size();
            for(int i = 0; i < n; ++i) {
                  Note* n  = oc->notes()[i];
                  Note* nn = chord->notes()[i];
                  Tie* tie = new Tie(this);
                  tie->setStartNote(n);
                  tie->setEndNote(nn);
                  tie->setTrack(n->track());
                  undoAddElement(tie);
                  }
            }

      updateAccidentals(measure, chord->staffIdx());
      return chord;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const Duration& d)
      {
// printf("addClone %s at %d %s\n", cr->name(), tick, qPrintable(d.fraction().print()));
      ChordRest* newcr;
      // change a RepeatMeasure() into an Rest()
      if (cr->type() == REPEAT_MEASURE)
            newcr = new Rest(*static_cast<Rest*>(cr));
      else
            newcr = static_cast<ChordRest*>(cr->clone());
      newcr->setDurationType(d);
      newcr->setDuration(d.fraction());
      newcr->setTuplet(cr->tuplet());
      newcr->setSelected(false);

      undoAddCR(newcr, cr->measure(), tick);
      return newcr;
      }

//---------------------------------------------------------
//   setRest
//    create one or more rests to fill "l"
//---------------------------------------------------------

Rest* Score::setRest(int tick, int track, Fraction l, bool useDots, Tuplet* tuplet)
      {
      Measure* measure = tick2measure(tick);
      Rest* r = 0;

printf("setRest() %d/%d\n", l.numerator(), l.denominator());

      while (!l.isZero()) {
            //
            // divide into measures
            //
            Fraction f;
            if (tuplet) {
                  int ticks = (tuplet->tick() + tuplet->actualTicks()) - tick;

                  f = Fraction::fromTicks(ticks);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f *= t->ratio();
                  //
                  // restrict to tuplet len
                  //
                  if (f < l)
                        l = f;
                  }
            else if (measure->tick() < tick)
                  f = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
            else
                  f = measure->len();

            if (f > l)
                  f = l;

            if ((track % VOICES) && !measure->hasVoice(track) && (tick == measure->tick())) {
                  l -= f;
                  measure = measure->nextMeasure();
                  if (!measure)
                        break;
                  tick = measure->tick();
                  continue;
                  }

            printf("set rest %d/%d  -> measure %d/%d\n",
               f.numerator(), f.denominator(),
               measure->timesig().numerator(), measure->timesig().denominator()
               );

            if ((measure->timesig() == measure->len())   // not in pickup measure
               && (measure->tick() == tick)
               // && ((measure->timesig() / timeStretch) == f)
               && (measure->timesig() == f)
               && (f < Duration(Duration::V_BREVE).fraction())) {
                  Rest* rest = addRest(tick, track, Duration(Duration::V_MEASURE), tuplet);
                  tick += measure->timesig().ticks();
                  if (r == 0)
                        r = rest;
                  }
            else {
                  //
                  // compute list of durations which will fit l
                  //

                  Fraction ff = f / staff(track/VOICES)->timeStretch(tick);
printf(" create duration list from %d/%d\n", ff.numerator(), ff.denominator());
                  QList<Duration> dList = toDurationList(ff, useDots);
                  if (dList.isEmpty())
                        return 0;
                  foreach(Duration d, dList) {
                        printf("    duration %d/%d\n", d.fraction().numerator(), d.fraction().denominator());
                        }

                  Rest* rest = 0;
                  if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
                        foreach(Duration d, dList) {
                              rest = addRest(tick, track, d, tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->actualTicks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i) {
                              rest = addRest(tick, track, dList[i], tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->actualTicks();
                              }
                        }
                  }
            l -= f;
            measure = measure->nextMeasure();
            if (!measure)
                  break;
            tick = measure->tick();
            }
      return r;
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, int pitch)
      {
      Note* note = new Note(this);
      note->setParent(chord);
      note->setTrack(chord->track());
      note->setPitch(pitch);
      note->setTpcFromPitch();
      undoAddElement(note);
      select(note, SELECT_SINGLE, 0);
      return note;
      }

//---------------------------------------------------------
//   addRemoveTimeSigDialog
//---------------------------------------------------------

static int addRemoveTimeSigDialog()
      {
      int n = QMessageBox::question(0,
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "MuseScore"),
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "Rewrite measures\nuntil next time signature?"),
         QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
      if (n == QMessageBox::Cancel)
            return -1;
      if (n == QMessageBox::Yes)
            return 1;
      return 0;
      }

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
            printf("addCR: cannot insert cr: list too short\n");
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
                        QList<Duration> dList = toDurationList(rest, false);
                        if (dList.isEmpty())
                              return true;
                        int n = dList.size();
                        for (int i = 0; i < n; ++i) {
                              const Duration& d = dList[i];
                              Chord* c = static_cast<Chord*>(chord->clone());
                              c->setSelected(false);
                              if (i == 0) {
                                    foreach(Slur* s, chord->slurBack())
                                          c->addSlurBack(s);
                                    }
                              if (i == n-1) {
                                    foreach(Slur* s, chord->slurFor())
                                          c->addSlurFor(s);
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
                        QList<Duration> dList = toDurationList(rest, false);
                        if (dList.isEmpty())
                              return true;
                        foreach(const Duration& d, dList) {
                              ChordRest* cr1 = static_cast<ChordRest*>(cr->clone());
                              cr1->setDurationType(d);
                              cr1->setDuration(d.fraction());
                              Segment* s = m->getSegment(SegChordRest, tick);
                              s->add(cr1);
                              tick += cr1->actualTicks();
                              }
                        len -= rest;
                        m = m->nextMeasure();
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
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//    or section break
//---------------------------------------------------------

bool Score::rewriteMeasures(Measure* fm, Measure* lm, const Fraction& ns)
      {
      int measures = 1;
      bool empty = true;
      for (Measure* m = fm; m != lm; m = m->nextMeasure()) {
            if (!m->isFullMeasureRest())
                  empty = false;
            ++measures;
            }
      if (empty) {
            //
            // only change measure len
            //
            Measure* m = fm;
            for (int i = 0; i < measures; ++i, m = m->nextMeasure()) {
                  undo()->push(new ChangeMeasureProperties(
                     m, ns, ns, m->getBreakMultiMeasureRest(), m->repeatCount(),
                     m->userStretch(), m->noOffset(), m->irregular()));
                  int strack = 0;
                  int etrack = nstaves() * VOICES;
                  Segment* s = m->first(SegChordRest);
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = s->element(track);
                        if (e) {
                              Rest* rest = static_cast<Rest*>(e);
                              undo()->push(new ChangeDuration(rest, ns));
                              }
                        }
                  }
            return true;
            }
      undo()->push(new RemoveMeasures(fm, lm));
      Fraction k = fm->len() * measures;
      k /= ns;
      int nm = (k.numerator() + k.denominator() - 1)/ k.denominator();
      Measure* nfm = 0;
      Measure* nlm = 0;

      int tick = 0;
      for (int i = 0; i < nm; ++i) {
            Measure* m = new Measure(this);
            m->setPrev(nlm);
            if (nlm)
                  nlm->setNext(m);
            m->setTimesig(ns);
            m->setLen(ns);
            m->setTick(tick);
            tick += m->ticks();
            nlm = m;
            if (i == 0)
                  nfm = m;
            }
      //
      // rewrite notes from measure list fm into
      // measure list nfm
      //

      int stick  = fm->tick();
      int etick  = stick + measures * fm->ticks();
      int tracks = fm->staffList()->size() * VOICES;
      int detick = nm * ns.ticks();

      for (int track = 0; track < tracks;  ++track) {
            int tick = 0;
            for (Segment* s = fm->first(); s; s = s->next1()) {
                  if (s->tick() >= etick)
                        break;
                  if (s->subtype() != SegChordRest || s->element(track) == 0)
                        continue;
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  ChordRest* ncr = static_cast<ChordRest*>(cr->clone());

                  ncr->setSlurFor(cr->slurFor());
                  ncr->setSlurBack(cr->slurBack());

                  tick = s->tick() - stick;
                  int ticks = ncr->actualTicks();
                  if (!addCR(tick, ncr, nfm)) {
                        undo()->pop();
                        // TODO: unwind creation of measures
                        return false;
                        }
                  tick += ticks;
                  }
            //
            // fill last measure with rest(s) if necessary
            //
            if ((track % VOICES) == 0 && tick < detick) {
                  int restTicks = detick - tick;
                  Rest* rest = new Rest(this);
                  rest->setTrack(track);
                  rest->setDurationType(restTicks);
                  rest->setDuration(Fraction::fromTicks(restTicks));
                  addCR(tick, rest, nfm);
                  }
            }

      //
      // insert new calculated measures
      //
      nfm->setPrev(fm->prev());
      nlm->setNext(lm->next());
      undo()->push(new InsertMeasures(nfm, nlm));
      return true;
      }


//---------------------------------------------------------
//   warnTupletCrossing
//---------------------------------------------------------

static void warnTupletCrossing()
      {
      QMessageBox::warning(0,
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "MuseScore"),
         QT_TRANSLATE_NOOP("addRemoveTimeSig", "cannot rewrite measures:\n"
         "tuplet would cross measure")
         );
      }

//---------------------------------------------------------
//   rewriteMeasures
//    rewrite all measures up to the next time signature
//---------------------------------------------------------

void Score::rewriteMeasures(Measure* fm, const Fraction& ns)
      {
      Measure* lm  = fm;
      Measure* fm1 = fm;
      for (MeasureBase* m = fm; ; m = m->next()) {
            if (!m || (m->type() != MEASURE) || static_cast<Measure*>(m)->first(SegTimeSig)) {
                  if (!rewriteMeasures(fm1, lm, ns)) {
                        warnTupletCrossing();
                        for (Measure* m = fm1; m; m = m->nextMeasure()) {
                              if (m->first(SegTimeSig))
                                    break;
                              undo()->push(new ChangeMeasureTimesig(m, ns));
                              }
                        return;
                        }
                  if (!m || m->type() == MEASURE)
                        break;
                  while (m->type() != MEASURE)
                        m = m->next();
                  fm1 = static_cast<Measure*>(m);
                  if (fm1 == 0)
                        break;
                  }
            lm  = static_cast<Measure*>(m);
            }
      }

//---------------------------------------------------------
//   cmdAddTimeSig
//
//    Add or change time signature at measure in response
//    to gui command (drop timesig on measure or timesig)
//---------------------------------------------------------

void Score::cmdAddTimeSig(Measure* fm, int staffIdx, TimeSig* ts)
      {
printf("cmdAddTimeSig\n");

      Fraction ns  = ts->sig();
      int tick     = fm->tick();
      TimeSig* lts = staff(staffIdx)->timeSig(tick);
      Fraction stretch, lsig;
      if (lts) {
            stretch = lts->stretch();
            lsig    = lts->sig();
            }
      else {
            stretch.set(1,1);
            lsig.set(4,4);
            }

      int track    = staffIdx * VOICES;
      Segment* seg = fm->getSegment(SegTimeSig, tick);
      TimeSig* ots = static_cast<TimeSig*>(seg->element(track));
      if (ots) {
            //
            //  ignore if there is already a timesig
            //  with same values
            //
            if ((ots->subtype()   == ts->subtype())
               && (ots->sig().identical(ts->sig()))
               && (ots->stretch() == ts->stretch())) {
printf("  already there %d %d\n", ots->subtype(), ts->subtype());
                  delete ts;
                  return;
                  }
            }
      else {
            //
            //  check for local timesig (only staff value changes)
            //  or redundant time signature
            //
            if (lsig == ts->sig()) {
printf("  global sig does not change\n");
                  ts->setParent(seg);
                  ts->setTrack(track);
                  undoAddElement(ts);
                  timesigStretchChanged(ts, fm, staffIdx);
                  return;
                  }
            }
      if (seg)
            undoRemoveElement(seg);
printf("   cmdAddTimeSig1\n");
      int n;
      if (ots && ots->sig() == ts->sig() && ots->stretch() == ts->stretch()) {
            // only symbol changes
            n = 0;
            }
      else
            n = addRemoveTimeSigDialog();
      if (n == -1) {
            delete ts;
            return;
            }

      if (n == 0) {
            //
            // Set time signature of all measures up to next
            // time signature. Do not touch measure contents.
            //
            seg = new Segment(fm, SegTimeSig, tick);
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  TimeSig* nsig = new TimeSig(this);
                  nsig->setSubtype(ts->subtype());
                  nsig->setSig(ts->sig());
                  nsig->setTrack(staffIdx * VOICES);
                  seg->add(nsig);
                  }
            undoAddElement(seg);
            for (Measure* m = fm; m; m = m->nextMeasure()) {
                  if (m->first(SegTimeSig))
                        break;
                  undo()->push(new ChangeMeasureTimesig(m, ns));
                  }
            }
      else {
            //
            // rewrite all measures up to the next time signature
            //
            rewriteMeasures(fm, ns);
            Measure* nfm = fm->prev() ? fm->prev()->nextMeasure() : firstMeasure();
            Segment* seg = new Segment(nfm, SegTimeSig, nfm->tick());
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  TimeSig* nsig = new TimeSig(this);
                  nsig->setSubtype(ts->subtype());
                  nsig->setSig(ts->sig());
                  nsig->setTrack(staffIdx * VOICES);
                  seg->add(nsig);
                  }
            nfm->add(seg);
            }
      delete ts;
      }

//---------------------------------------------------------
//   timesigStretchChanged
//---------------------------------------------------------

void Score::timesigStretchChanged(TimeSig* ts, Measure* fm, int staffIdx)
      {
      for (Measure* m = fm; m; m = m->nextMeasure()) {
            if ((m != fm) && m->first(SegTimeSig))
                  break;
            int strack = staffIdx * VOICES;
            int etrack = strack + VOICES;
            for (Segment* s = m->first(SegChordRest); s; s = s->next(SegChordRest)) {
                  for (int track = strack; track < etrack; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (!cr)
                              continue;
                        if (cr->type() == REST && cr->durationType() == Duration::V_MEASURE) {
                              cr->setDuration(ts->actualSig());
                              }
                        else
                              printf("timeSigChanged: not implemented: chord/rest does not fit\n");
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveTimeSig
//---------------------------------------------------------

void Score::cmdRemoveTimeSig(TimeSig* ts)
      {
      int n = addRemoveTimeSigDialog();
      if (n == -1)
            return;

      undoRemoveElement(ts->segment());

      Measure* fm = ts->measure();
      Measure* lm = fm;
      Measure* pm = fm->prevMeasure();
      Fraction ns(pm ? pm->timesig() : Fraction(4,4));
      for (Measure* m = lm; m; m = m->nextMeasure()) {
            if (m->first(SegTimeSig))
                  break;
            lm = m;
            }

      if (n == 0) {
            //
            // Set time signature of all measures up to next
            // time signature. Do not touch measure contents.
            //
            for (Measure* m = fm; m; m = m->nextMeasure()) {
                  if (m->first(SegTimeSig))
                        break;
                  undo()->push(new ChangeMeasureTimesig(m, ns));
                  }
            }
      else {
            rewriteMeasures(fm, ns);
            }
      }

//---------------------------------------------------------
//   putNote
//    mouse click in state NOTE_ENTRY
//---------------------------------------------------------

void Score::putNote(const QPointF& pos, bool replace)
      {
      Position p;
      if (!getPosition(&p, pos, _is.voice())) {
            printf("cannot put note here, get position failed\n");
            return;
            }

      Segment* s      = p.segment;
      int tick        = s->tick();
      int staffIdx    = p.staffIdx;
      int line        = p.line;
      Staff* st       = staff(staffIdx);
      KeySigEvent key = st->keymap()->key(tick);
      int clef        = st->clef(tick);

printf("putNote at tick %d staff %d line %d key %d clef %d\n",
   tick, staffIdx, line, key.accidentalType(), clef);

      _is.setTrack(staffIdx * VOICES + _is.voice());
      _is.setSegment(s);
      const Instrument* instr = st->part()->instr();
      Direction stemDirection = AUTO;
      NoteVal nval;

      switch(st->staffType()->group()) {
            case PERCUSSION_STAFF: {
                  Drumset* ds   = instr->drumset();
                  nval.pitch    = _is.drumNote();
                  if (nval.pitch < 0)
                        return;
                  nval.headGroup = ds->noteHead(nval.pitch);
                  if (nval.headGroup < 0)
                        return;
                  stemDirection = ds->stemDirection(nval.pitch);
                  break;
                  }
            case TAB_STAFF: {
                  Tablature* neck = instr->tablature();
                  StaffTypeTablature * tab = (StaffTypeTablature*)st->staffType();
                  // if tablature is upside down, 'flip' string number
                  int string = tab->upsideDown() ? (tab->lines() - line - 1) : line;
                  // check the chord does not already contains a note on the same string
                  ChordRest* cr = _is.cr();
                  if(cr != 0 && cr->type() == CHORD)
                        foreach(Note * note, static_cast<Chord*>(cr)->notes())
                              if(note->string() == string)  // if line is the same
                                    return;                 // do nothing
                  // build a default NoteVal for that line
                  nval.pitch     = neck->getPitch(string, 0);
                  nval.fret      = 0;
                  nval.string    = string;
                  break;
                  }

            case PITCHED_STAFF:
                  nval.pitch = line2pitch(line, clef, key.accidentalType());
                  break;
            }

      _is.pitch = nval.pitch;
      expandVoice();
      ChordRest* cr = _is.cr();
      bool addToChord = false;

      if (cr) {
            Duration d = cr->durationType();
            Note* note = 0;
            if (cr->type() == CHORD) {
                  Fraction f = cr->duration();
                  note = static_cast<Chord*>(cr)->upNote();
                  if (note) {
                        Note* note2 = note;
                        while (note2->tieFor()) {
                              note2 = note2->tieFor()->endNote();
                              f += note2->chord()->duration();
                              }
                        Duration dd(f);
                        if (dd.isValid())
                              d = dd;
                        }
                  else
                        printf("note not found: %d!\n", nval.pitch);
                  }
            if (!replace
               && (d == _is.duration())
               && (cr->type() == CHORD)
               && !_is.rest)
                  {
                  Chord* chord = static_cast<Chord*>(cr);
                  note = chord->findNote(nval.pitch);
                  if (note) {
                        // remove note from chord
                        if (chord->notes().size() > 1)
                              undoRemoveElement(note);
                        return;
                        }
                  addToChord = true;
                  }
            }
      if (addToChord && cr->type() == CHORD)
            addNote(static_cast<Chord*>(cr), nval.pitch);
      else {
            // replace chord
            if (_is.rest)
                  nval.pitch = -1;
            setNoteRest(_is.segment(), _is.track(), nval, _is.duration().fraction(), stemDirection);
            }
      moveToNextInputPos();
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      QList<Note*> noteList;
      Element* el = selection().element();
      if (el && el->type() == NOTE)
            noteList.append(static_cast<Note*>(el));
      else if (el && el->type() == STEM) {
            Chord* chord = static_cast<Stem*>(el)->chord();
            noteList = chord->notes();
            }
      else
            noteList = selection().noteList();
      if (noteList.isEmpty()) {
            printf("no notes selected\n");
            return;
            }

      startCmd();
      foreach (Note* note, noteList) {
            if (note->tieFor()) {
                  printf("cmdAddTie: has already tie? noteFor: %p\n", note->tieFor());
                  continue;
                  }
            Chord* chord  = note->chord();
            if (noteEntryMode()) {
                  if (_is.cr() == 0) {
                        if (debugMode)
                              printf("cmdAddTie: no pos\n");
                        expandVoice();
                        }
                  Note* n = addPitch(note->pitch(), true);
                  if (n) {
                        n->setLine(note->line());
                        n->setTpc(note->tpc());
                        Tie* tie = new Tie(this);
                        tie->setStartNote(note);
                        tie->setEndNote(n);
                        tie->setTrack(note->track());
                        note->setTieFor(tie);
                        n->setTieBack(tie);
                        undoAddElement(tie);
                        nextInputPos(n->chord(), false);
                        }
                  continue;
                  }
            Note* note2 = searchTieNote(note);
            Part* part = chord->staff()->part();
            int strack = part->staves()->front()->idx() * VOICES;
            int etrack = strack + part->staves()->size() * VOICES;

            for (Segment* seg = chord->segment()->next1(SegChordRest); seg; seg = seg->next1(SegChordRest)) {
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
            if (note2 == 0) {
                  if (debugMode)
                        printf("addTie: next note for tie not found\n");
                  continue;
                  }
            Tie* tie = new Tie(this);
            tie->setStartNote(note);
            tie->setEndNote(note2);
            tie->setTrack(note->track());
            undoAddElement(tie);
            select(note2, SELECT_SINGLE, 0);
            }
      endCmd();
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    'H' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddHairpin(bool decrescendo)
      {
      ChordRest* cr1;
      ChordRest* cr2;
      getSelectedChordRest2(&cr1, &cr2);
      if (!cr1)
            return;
      if (cr2 == 0)
            cr2 = nextChordRest(cr1);
      if (cr2 == 0)
            return;

      Hairpin* pin = new Hairpin(this);
      pin->setSubtype(decrescendo ? 1 : 0);
      pin->setTrack(cr1->track());
      pin->setStartElement(cr1->segment());
      pin->setEndElement(cr2->segment());
      pin->setParent(cr1->segment());
      undoAddElement(pin);
      if (!noteEntryMode())
            select(pin, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdSetBeamMode
//---------------------------------------------------------

void Score::cmdSetBeamMode(int mode)
      {
      ChordRest* cr = getSelectedChordRest();
      if (cr == 0)
            return;
      cr->setBeamMode(BeamMode(mode));
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdFlip
//---------------------------------------------------------

void Score::cmdFlip()
      {
      const QList<Element*>& el = selection().elements();
      if (el.isEmpty()) {
            selectNoteSlurMessage();
            return;
            }
      foreach(Element* e, el) {
            if (e->type() == NOTE) {
                  Chord* chord = static_cast<Note*>(e)->chord();
                  if (chord->beam())
                        undo()->push(new FlipBeamDirection(chord->beam()));
                  else {
                        Direction dir = chord->stemDirection();
                        if (dir == AUTO)
                              dir = chord->up() ? DOWN : UP;
                        else
                              dir = dir == UP ? DOWN : UP;
                        undo()->push(new ChangeProperty(chord, P_STEM_DIRECTION, dir));
                        }
                  }
            else if (e->type() == SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  undo()->push(new FlipSlurDirection(slur));
                  }
            else if (e->type() == BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  undo()->push(new FlipBeamDirection(beam));
                  }
            else if (e->type() == HAIRPIN_SEGMENT)
                  undoChangeSubtype(e, e->subtype() == 0 ? 1 : 0);
            else if (e->type() == ARTICULATION) {
                  Articulation* a = static_cast<Articulation*>(e);
                  if (e->subtype() == Articulation_Staccato
                     || e->subtype() == Articulation_Tenuto
                     || e->subtype() == Articulation_Sforzatoaccent) {
                        ArticulationAnchor aa = a->anchor();
                        if (aa == A_TOP_CHORD)
                              aa = A_BOTTOM_CHORD;
                        else if (aa == A_BOTTOM_CHORD)
                              aa = A_TOP_CHORD;
                        else if (aa == A_CHORD)
                              aa = a->up() ? A_BOTTOM_CHORD : A_TOP_CHORD;
                        if (aa != a->anchor())
                              undo()->push(new ChangeProperty(a, P_ARTICULATION_ANCHOR, aa));
                        }
                  else {
                        Direction d = a->direction();
                        if (d == AUTO)
                              d = a->up() ? DOWN : UP;
                        else if (d == UP)
                              d = DOWN;
                        else
                              d = UP;
                        undo()->push(new ChangeProperty(a, P_DIRECTION, d));
                        }
                  return;   // no layoutAll
                  }
            else if (e->type() == TUPLET)
                  undo()->push(new FlipTupletDirection(static_cast<Tuplet*>(e)));
            else if (e->type() == NOTEDOT)
                  undo()->push(new FlipNoteDotDirection(static_cast<Note*>(e->parent())));
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddBSymbol
//    add Symbol or Image
//---------------------------------------------------------

void Score::cmdAddBSymbol(BSymbol* s, const QPointF& pos, const QPointF& off)
      {
      s->setSelected(false);
      bool foundPage = false;
      foreach (Page* page, pages()) {
            if (page->contains(pos)) {
                  const QList<System*>* sl = page->systems();
                  if (sl->isEmpty()) {
                        printf("addSymbol: cannot put symbol here: no system on page\n");
                        delete s;
                        return;
                        }
                  System* system = sl->front();
                  MeasureBase* m = system->measures().front();
                  if (m == 0) {
                        printf("addSymbol: cannot put symbol here: no measure in system\n");
                        delete s;
                        return;
                        }
                  s->setPos(0.0, 0.0);
                  s->setUserOff(pos - m->pagePos() - off);
                  s->setTrack(0);
                  s->setParent(m);
                  foundPage = true;
                  break;
                  }
            }
      if (!foundPage) {
            printf("addSymbol: cannot put symbol here: no page\n");
            delete s;
            return;
            }
      undoAddElement(s);
      addRefresh(s->abbox());
      select(s, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(Element* el)
      {
      switch(el->type()) {
            case INSTRUMENT_NAME: {
                  Part* part = el->staff()->part();

                  if (el->subtype() == TEXT_INSTRUMENT_LONG)
                        undo()->push(new ChangeInstrumentLong(0, part, QList<StaffNameDoc>()));
                  else if (el->subtype() == TEXT_INSTRUMENT_SHORT)
                        undo()->push(new ChangeInstrumentShort(0, part, QList<StaffNameDoc>()));
                  }
                  break;

            case TIMESIG:
                  cmdRemoveTimeSig(static_cast<TimeSig*>(el));
                  break;

            case OTTAVA_SEGMENT:
            case HAIRPIN_SEGMENT:
            case TRILL_SEGMENT:
            case TEXTLINE_SEGMENT:
            case VOLTA_SEGMENT:
            case SLUR_SEGMENT:
                  undoRemoveElement(static_cast<SpannerSegment*>(el)->spanner());
                  break;

            case NOTE:
                  {
                  Chord* chord = static_cast<Chord*>(el->parent());
                  if (chord->notes().size() > 1) {
                        undoRemoveElement(el);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }

            case CHORD:
                  {
                  Chord* chord = static_cast<Chord*>(el);
                  removeChordRest(chord, false);

                  // replace with rest if voice 0 or if in tuplet
                  Tuplet* tuplet = chord->tuplet();
                  // if ((el->voice() == 0 || tuplet) && (chord->noteType() == NOTE_NORMAL)) {
                  if (chord->noteType() == NOTE_NORMAL) {
                        Rest* rest = new Rest(this, chord->durationType());
                        rest->setDurationType(chord->durationType());
                        rest->setDuration(chord->duration());
                        rest->setTrack(el->track());
                        rest->setParent(chord->parent());
                        Segment* segment = chord->segment();
                        undoAddCR(rest, segment->measure(), segment->tick());
                        // undoAddElement(rest);
                        if (tuplet) {
                              tuplet->add(rest);
                              rest->setTuplet(tuplet);
                              rest->setDurationType(chord->durationType());
                              }
                        select(rest, SELECT_SINGLE, 0);
                        }
                  else  {
                        // remove segment if empty
                        Segment* seg = chord->segment();
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
                  }
                  break;

            case REST:
                  //
                  // only allow for voices != 0
                  //    e.g. voice 0 rests cannot be removed
                  //
                  {
                  Rest* rest = static_cast<Rest*>(el);
                  if (rest->tuplet() && rest->tuplet()->elements().empty())
                        undoRemoveElement(rest->tuplet());
                  if (el->voice() != 0)
                        undoRemoveElement(el);
                  }
                  break;

            case MEASURE:
                  {
                  Measure* measure = static_cast<Measure*>(el);
                  undoRemoveElement(el);
                  cmdRemoveTime(measure->tick(), measure->ticks());
                  }
                  break;

            case ACCIDENTAL:
                  changeAccidental(static_cast<Note*>(el->parent()), ACC_NONE);
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  Segment* seg = bl->segment();
                  Measure* m   = seg->measure();
                  if (seg->subtype() == SegStartRepeatBarLine)
                        undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatStart);
                  else if (seg->subtype() == SegBarLine)
                        undoRemoveElement(el);
                  else if (seg->subtype() == SegEndBarLine) {
                        if (m->endBarLineType() != NORMAL_BAR) {
                              undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatEnd);
                              Measure* nm = m->nextMeasure();
                              if (nm)
                                    undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                              undoChangeEndBarLineType(m, NORMAL_BAR);
                              m->setEndBarLineGenerated(true);
                              }
                        }
                  }
                  break;
            case TUPLET:
                  cmdDeleteTuplet(static_cast<Tuplet*>(el), true);
                  break;

            default:
                  undoRemoveElement(el);
                  break;
            }
      }

//---------------------------------------------------------
//   cmdRemoveTime
//---------------------------------------------------------

void Score::cmdRemoveTime(int tick, int len)
      {
/*      int etick = tick + len;
      foreach (Beam* b, _beams) {
            ChordRest* e1 = b->elements().front();
            ChordRest* e2 = b->elements().back();
            if ((e1->tick() >= tick && e1->tick() < etick)
               || (e2->tick() >= tick && e2->tick() < etick)) {
                  undoRemoveElement(b);
                  }
            }
      */
      undoInsertTime(tick, -len);
      }

//---------------------------------------------------------
//   cmdDeleteSelectedMeasures
//---------------------------------------------------------

void Score::cmdDeleteSelectedMeasures()
      {
      if (selection().state() != SEL_RANGE)
            return;
      MeasureBase* is = selection().startSegment()->measure();
      bool createEndBar = false;
      if (is->next()) {
            Segment* seg = selection().endSegment();
            MeasureBase* ie = seg ? seg->measure() : lastMeasure();
//printf("cmdDEleteSelectedMeasures %p - %p\n", is, ie);
//printf("  seg %s\n", seg->subTypeName());
            if (ie) {
                  if ((seg == 0) || (ie->tick() < selection().tickEnd())) {
                        // if last measure is selected
                        if (ie->type() == MEASURE)
                              createEndBar = static_cast<Measure*>(ie)->endBarLineType() == END_BAR;
                        deleteItem(ie);
                        }
                  if (ie != is) {
                        do {
                              ie = ie->prev();
                              if (ie == 0)
                                    break;
                              deleteItem(ie);
                              } while (ie != is);
                        }
                  }
            }
      else {
            createEndBar = true;
            deleteItem(is);
            }

      if (createEndBar) {
            MeasureBase* mb = _measures.last();
            while (mb && mb->type() != MEASURE)
                  mb = mb->prev();
            if (mb) {
                  Measure* lastMeasure = static_cast<Measure*>(mb);
                  if (lastMeasure->endBarLineType() == NORMAL_BAR) {
                        undoChangeEndBarLineType(lastMeasure, END_BAR);
                        }
                  }
            }
//      selection().clearElements();
      select(0, SELECT_SINGLE, 0);
      _is.setSegment(0);        // invalidate position
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (selection().state() == SEL_RANGE) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();

            bool fullMeasure = (s1->measure()->first(SegChordRest) == s1)
                               && (s2 == 0 || (s2->segmentType() == SegEndBarLine));

            int tick2   = s2 ? s2->tick() : INT_MAX;
            int track1  = selection().staffStart() * VOICES;
            int track2  = selection().staffEnd() * VOICES;
            for (int track = track1; track < track2; ++track) {
                  Fraction f;
                  int tick  = -1;
                  Tuplet* tuplet = 0;
                  for (Segment* s = s1; s != s2; s = s->next1()) {
                        if (s->element(track) &&
                           ((s->subtype() == SegBreath)
                           || (s->subtype() == SegGrace))) {
                              deleteItem(s->element(track));
                              continue;
                              }
                        if (s->subtype() != SegChordRest || !s->element(track))
                              continue;
                        ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                        if (tick == -1) {
                              // first ChordRest found:
                              int offset = cr->tick() - s->measure()->tick();
                              if (cr->measure()->tick() >= s1->tick() && offset) {
                                    f = Fraction::fromTicks(offset);
                                    tick = s->measure()->tick();
                                    }
                              else {
                                    tick   = s->tick();
                                    f      = Fraction();
                                    }
                              tuplet = cr->tuplet();
                              if (tuplet && (tuplet->tick() == tick) && ((tuplet->tick() + tuplet->actualTicks()) < tick2) ) {
                                    // remove complete top level tuplet

                                    Tuplet* t = cr->tuplet();
                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    f += t->duration();
                                    tuplet = 0;
                                    continue;
                                    }
                              }
                        if (tuplet != cr->tuplet()) {
                              Tuplet* t = cr->tuplet();
                              if (t && ((t->tick() + t->actualTicks()) < tick2)) {
                                    // remove complete top level tuplet

                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    f += t->duration();
                                    tuplet = 0;
                                    continue;
                                    }
                              if (f.isValid())
                                    setRest(tick, track, f, false, tuplet);
                              tick = cr->tick();
                              tuplet = cr->tuplet();
                              removeChordRest(cr, true);
                              f = cr->duration();
                              }
                        else {
                              removeChordRest(cr, true);
                              f += cr->duration();
                              }
                        }
                  if (f.isValid() && !f.isZero()) {
                        if (fullMeasure) {
                              // handle this as special case to be able to
                              // fix broken measures:
                              for (Measure* m = s1->measure(); m; m = m->nextMeasure()) {
                                    setRest(m->tick(), track, Fraction(m->len()), false, 0);
                                    if (s2 && (m == s2->measure()))
                                          break;
                                    }
                              }
                        else {
                              setRest(tick, track, f, false, tuplet);
                              }
                        }
                  }
            }
      else {
            // deleteItem modifies selection().elements() list,
            // so we need a local copy:
            QList<Element*> el(selection().elements());
            if (el.isEmpty())
                  printf("...nothing selected\n");
            foreach(Element* e, el)
                  deleteItem(e);
            deselectAll();
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menue
//---------------------------------------------------------

Lyrics* Score::addLyrics()
      {
      Element* el = selection().element();
      if (el == 0 || (el->type() != NOTE && el->type() != LYRICS)) {
            QMessageBox::information(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("No note or lyrics selected:\n"
                  "Please select a single note or lyrics and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
            return 0;
            }
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = static_cast<Note*>(el)->chord();
      else if (el->type() == LYRICS)
            cr = static_cast<Lyrics*>(el)->chordRest();
      else
            return 0;

      QList<Lyrics*> ll = cr->lyricsList();
      int no = ll.size();
      Lyrics* lyrics = new Lyrics(this);
      lyrics->setTrack(cr->track());
      lyrics->setParent(cr);
      lyrics->setNo(no);
      undoAddElement(lyrics);
      select(lyrics, SELECT_SINGLE, 0);
      return lyrics;
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//    replace cr with tuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(ChordRest* ocr, Tuplet* tuplet)
      {
printf("createTuplet at %d <%s> duration <%s> ratio <%s> baseLen <%s>\n",
  ocr->tick(), ocr->name(),
  qPrintable(ocr->duration().print()),
  qPrintable(tuplet->ratio().print()),
  qPrintable(tuplet->baseLen().fraction().print())
            );

      int track        = ocr->track();
      Measure* measure = ocr->measure();
      int tick         = ocr->tick();

      undoRemoveElement(ocr);
      undoAddElement(tuplet);

      ChordRest* cr;
      if (ocr->type() == CHORD) {
            cr = new Chord(this);
            foreach(Note* oldNote, static_cast<Chord*>(ocr)->notes()) {
                  Note* note = new Note(this);
                  note->setPitch(oldNote->pitch(), oldNote->tpc());
                  cr->add(note);
                  }
            }
      else
            cr = new Rest(this);

      Fraction an     = (tuplet->duration() * tuplet->ratio()) / tuplet->baseLen().fraction();
      int actualNotes = an.numerator() / an.denominator();

      cr->setTuplet(tuplet);
      cr->setTrack(track);
      cr->setDurationType(tuplet->baseLen());
      cr->setDuration(tuplet->baseLen().fraction());

printf("tuplet note duration %s  actualNotes %d  ticks %d\n",
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->actualTicks());

      undoAddCR(cr, measure, tick);

      int ticks = cr->actualTicks();

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setDurationType(tuplet->baseLen());
            rest->setDuration(tuplet->baseLen().fraction());
#if 0
            SegmentType st = Segment::segmentType(rest->type());
            Segment* seg = measure->undoGetSegment(st, tick);
            rest->setParent(seg);
#endif
            undoAddCR(rest, measure, tick);
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   colorItem
//---------------------------------------------------------

void Score::colorItem(Element* element)
      {
      QColor sc(element->color());
      QColor c = QColorDialog::getColor(sc);
      if (!c.isValid())
            return;

      foreach(Element* e, selection().elements()) {
            if (e->color() != c) {
                  undo()->push(new ChangeProperty(e, P_COLOR, c));
                  e->setGenerated(false);
                  refresh |= e->abbox();
                  if (e->type() == BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == SEGMENT && ep->subtype() == SegEndBarLine) {
                              Measure* m = static_cast<Segment*>(ep)->measure();
                              BarLine* bl = static_cast<BarLine*>(e);
                              m->setEndBarLineType(bl->barLineType(), false, e->visible(), e->color());
                              }
                        }
                  }
            }
      deselectAll();
      }

//---------------------------------------------------------
//   cmdExchangeVoice
//---------------------------------------------------------

void Score::cmdExchangeVoice(int s, int d)
      {
      if (selection().state() != SEL_RANGE) {
            selectStavesMessage();
            return;
            }
      int t1 = selection().tickStart();
      int t2 = selection().tickEnd();

      Measure* m1 = tick2measure(t1);
      Measure* m2 = tick2measure(t2);
      if (t2 > m2->tick())
            m2 = 0;

      for (;;) {
            undoExchangeVoice(m1, s, d, selection().staffStart(), selection().staffEnd());
            m1 = m1->nextMeasure();
            if ((m1 == 0) || (m2 && (m1->tick() == m2->tick())))
                  break;
            }
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void Score::cmdEnterRest(const Duration& d)
      {
      if (_is.track() == -1) {
            printf("cmdEnterRest: track -1\n");
            return;
            }
      startCmd();
      expandVoice();
      if (_is.cr() == 0) {
            printf("cannot enter rest here\n");
            return;
            }

      int track = _is.track();
      NoteVal nval;
      Segment* seg  = setNoteRest(_is.segment(), track, nval, d.fraction(), AUTO);
      if (seg) {
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (cr)
                  nextInputPos(cr, false);
            }
      _is.rest = false;  // continue with normal note entry
      endCmd();
      }

//---------------------------------------------------------
//   removeChordRest
//    remove chord or rest
//    remove associated segment if empty
//    remove beam
//    remove slurs
//---------------------------------------------------------

void Score::removeChordRest(ChordRest* cr, bool clearSegment)
      {
      undoRemoveElement(cr);
      if (clearSegment) {
            Segment* seg = cr->segment();
            if (seg->isEmpty())
                  undoRemoveElement(seg);
            }
      if (cr->beam()) {
            Beam* beam = cr->beam();
            if (beam->generated()) {
                  beam->parent()->remove(beam);
                  delete beam;
                  }
            else
                  undoRemoveElement(beam);
            }
      }

//---------------------------------------------------------
//   cmdDeleteTuplet
//    remove tuplet and replace with rest
//---------------------------------------------------------

void Score::cmdDeleteTuplet(Tuplet* tuplet, bool replaceWithRest)
      {
      foreach(DurationElement* de, tuplet->elements()) {
            if (de->type() == CHORD || de->type() == REST)
                  removeChordRest(static_cast<ChordRest*>(de), true);
            else
                  cmdDeleteTuplet(static_cast<Tuplet*>(de), false);
            }
      undoRemoveElement(tuplet);
      if (replaceWithRest) {
            Rest* rest = setRest(tuplet->tick(), tuplet->track(), tuplet->duration(), true, 0);
            if (tuplet->tuplet()) {
                  rest->setTuplet(tuplet->tuplet());
                  tuplet->tuplet()->add(rest);
                  }
            }
      }

//---------------------------------------------------------
//   nextInputPos
//---------------------------------------------------------

void Score::nextInputPos(ChordRest* cr, bool doSelect)
      {
      ChordRest* ncr = nextChordRest(cr);
      if ((ncr == 0) && (_is.track() % VOICES)) {
            Segment* s = tick2segment(cr->tick() + cr->actualTicks());
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? static_cast<ChordRest*>(s->element(track)) : 0;
            }
      _is.setSegment(ncr ? ncr->segment() : 0);
      if (doSelect)
            select(ncr, SELECT_SINGLE, 0);
//      if (ncr)
//            emit posChanged(ncr->tick());
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
            printf("cannot split here\n");
            return;
            }
      startCmd();
      deleteItem(measure);
      Measure* m1 = new Measure(this);
      Measure* m2 = new Measure(this);
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

//      m1->setNext(m2);
//      m2->setNext(measure->next());
      undoInsertMeasure(m2, measure->next());
      undoInsertMeasure(m1, m2);

      SlurMap* slurMap = new SlurMap[tracks];
      TieMap* tieMap   = new TieMap[tracks];

      for (int track = 0; track < tracks; ++track) {
            TupletMap tupletMap;
            for (Segment* s = measure->first(); s; s = s->next()) {
                  Element* oe = s->element(track);
                  if (oe == 0)
                        continue;
                  SegmentType st = s->segmentType();
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
                              printf("cloneStave: cannot find slur\n");
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
                                          printf("cloneStave: cannot find tie\n");
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

//---------------------------------------------------------
//   cmdJoinMeasure
//---------------------------------------------------------

void Score::cmdJoinMeasure(Measure* m1, Measure* m2)
      {
      Measure* m = new Measure(this);
      m->setTick(m1->tick());
      m->setNext(m2);
      m->setTimesig(m1->timesig());
      Fraction f;
      for (Measure* mm = m1; mm && mm != m2; mm = mm->nextMeasure())
            f += mm->len();
      m->setLen(f);

      startCmd();
      undo()->push(new RemoveMeasures(m1, m2->prevMeasure()));
      undoInsertMeasure(m, m2);

      int tracks       = nstaves() * VOICES;
      SlurMap* slurMap = new SlurMap[tracks];
      TieMap* tieMap   = new TieMap[tracks];

      for (int track = 0; track < tracks; ++track) {
            TupletMap tupletMap;
            for (Segment* s = m1->first(); s; s = s->next1()) {
                  if (s->measure() == m2)
                        break;
                  Element* oe = s->element(track);
                  if (oe == 0)
                        continue;

                  SegmentType st = s->segmentType();

                  // do not copy barlines except last
                  if ((st == SegEndBarLine) && (s->tick() < (m2->tick() + m2->ticks())))
                        continue;

                  Element* ne = oe->clone();
                  Segment* seg = m->getSegment(st, s->tick());
                  seg->add(ne);
                  if (oe->isChordRest()) {
                        ChordRest* ocr = static_cast<ChordRest*>(oe);
                        ChordRest* ncr = static_cast<ChordRest*>(ne);
                        Tuplet* ot     = ocr->tuplet();
                        if (ot) {
                              Tuplet* nt = tupletMap.findNew(ot);
                              if (nt == 0) {
                                    nt = new Tuplet(*ot);
                                    nt->clear();
                                    m->add(nt);
                                    tupletMap.add(ot, nt);
                                    }
                              ncr->setTuplet(nt);
                              nt->add(ncr);
                              }
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
                                    printf("cloneStave: cannot find slur\n");
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
                                                printf("cloneStave: cannot find tie\n");
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }
      delete[] slurMap;
      delete[] tieMap;
      endCmd();
      }

