//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "scoreview.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "key.h"
#include "al/sig.h"
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
#include "seq.h"
#include "mscore.h"
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
#include "al/tempo.h"
#include "undo.h"

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
      rest->setDuration(d.type() == Duration::V_MEASURE ? measure->len() : d.fraction());
      rest->setTrack(track);
      rest->setTuplet(tuplet);
      undoAddCR(rest, measure, tick);
      return rest;
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, int track, Duration d, Tuplet* tuplet)
      {
      Rest* rest = new Rest(this, d);
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

      Chord* chord = new Chord(this);
      chord->setTuplet(tuplet);
      chord->setTrack(oc->track());
      chord->setDurationType(d);
      chord->setDuration(d.fraction());

      foreach(Note* n, oc->notes()) {
            Note* nn = new Note(this);
            chord->add(nn);
            nn->setPitch(n->pitch(), n->tpc());
            if (genTie) {
                  Tie* tie = new Tie(this);
                  tie->setStartNote(n);
                  tie->setEndNote(nn);
                  tie->setTrack(n->track());
                  undoAddElement(tie);
                  }
            }

      undoAddCR(chord, measure, tick);
      return chord;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const Duration& d)
      {
printf("addClone %s at %d %s\n", cr->name(), tick, qPrintable(d.fraction().print()));
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

      while (!l.isZero()) {
            //
            // divide into measures
            //
            Fraction f;
            if (tuplet) {
                  int ticks = (tuplet->tick() + tuplet->ticks()) - tick;

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

            if ((track % VOICES) && !measure->hasVoice(track)) {
                  l -= f;
                  measure = measure->nextMeasure();
                  if (!measure)
                        break;
                  tick = measure->tick();
                  continue;
                  }

            if ((measure->timesig() == measure->len())   // not in pickup measure
               && (measure->tick() == tick)
               && (measure->timesig() == f)
               && (f < Duration(Duration::V_BREVE).fraction())) {
                  Rest* rest = addRest(tick, track, Duration(Duration::V_MEASURE), tuplet);
                  tick += rest->ticks();
                  if (r == 0)
                        r = rest;
                  }
            else {
                  //
                  // compute list of durations which will fit l
                  //

                  QList<Duration> dList = toDurationList(f, useDots);
                  if (dList.isEmpty())
                        return 0;

                  Rest* rest = 0;
                  if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
                        foreach(Duration d, dList) {
                              rest = addRest(tick, track, d, tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->ticks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i) {
                              rest = addRest(tick, track, dList[i], tuplet);
                              if (r == 0)
                                    r = rest;
                              tick += rest->ticks();
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
      mscore->play(chord);
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
      if (tuplet && (tick + tuplet->ticks() > etick))
            return false;
      if (tick + cr->ticks() > etick) {
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
                              tick += c->ticks();
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
                              tick += cr1->ticks();
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
                  int ticks = ncr->ticks();
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
//                  if (fm1 == 0 || lm->first(SegTimeSig))
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

void Score::cmdAddTimeSig(Measure* fm, int timeSigSubtype)
      {
      Fraction ns(TimeSig::getSig(timeSigSubtype));

      int tick = fm->tick();
      Segment* seg = fm->first(SegTimeSig);
      if (seg && seg->element(0)) {
            TimeSig* ots = static_cast<TimeSig*>(seg->element(0));
            if ((ots->subtype() == timeSigSubtype)
               && (ns == fm->timesig())
               && (ns == fm->len())) {
                  printf("time sig aready there\n");
                  return;
                  }
            }
      int n = addRemoveTimeSigDialog();
      if (n == -1)
            return;
      if (seg)
            undoRemoveElement(seg);

      if (n == 0) {
            //
            // Set time signature of all measures up to next
            // time signature. Do not touch measure contents.
            //
            seg = new Segment(fm, SegTimeSig, tick);
            for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                  TimeSig* nsig = new TimeSig(this, timeSigSubtype);
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
                  TimeSig* nsig = new TimeSig(this, timeSigSubtype);
                  nsig->setTrack(staffIdx * VOICES);
                  seg->add(nsig);
                  }
            nfm->add(seg);
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
      int pitch       = line2pitch(line, clef, key.accidentalType());

printf("putNote at tick %d staff %d line %d key %d clef %d pitch %d\n",
   tick, staffIdx, line, key.accidentalType(), clef, pitch);

      const Instrument* instr       = st->part()->instr();
      _is.setTrack(staffIdx * VOICES + (_is.track() % VOICES));
      _is.pitch               = pitch;
      int headGroup           = 0;
      Direction stemDirection = AUTO;

      if (instr->useDrumset()) {
            Drumset* ds   = instr->drumset();
            pitch         = _is.drumNote();
            if (pitch < 0)
                  return;
            headGroup = ds->noteHead(pitch);
            if(headGroup < 0) {
                  return;
                  }
            stemDirection = ds->stemDirection(pitch);
            }

      _is.setSegment(s);
      expandVoice();
      ChordRest* cr = _is.cr();
      if (cr == 0)
            return;

      bool addToChord = false;

      //
      // TODO: if _is.duration != cr->duration then
      //       add a note with cr->duration tied to another note which adds
      //       to a total duration of _is.duration
      //
      if (!replace && (cr->durationType() == _is.duration()) && (cr->type() == CHORD) && !_is.rest) {

            Chord* chord = static_cast<Chord*>(cr);
            Note* note = chord->findNote(pitch);
            if (note) {
                  // remove note from chord
                  if (chord->notes().size() > 1)
                        undoRemoveElement(note);
                  return;
                  }
            addToChord = true;
            }
      if (addToChord && cr->type() == CHORD)
            addNote(static_cast<Chord*>(cr), pitch);
      else {
            // replace chord
            if (_is.rest)
                  pitch = -1;
            setNoteRest(cr, _is.track(), pitch, _is.duration().fraction(), headGroup, stemDirection);
            }
      moveToNextInputPos();
      }

//---------------------------------------------------------
//   modifyElement
//---------------------------------------------------------

void ScoreView::modifyElement(Element* el)
      {
      if (el == 0) {
            printf("modifyElement: el==0\n");
            return;
            }
      Score* cs = el->score();
      if (!cs->selection().isSingle()) {
            printf("modifyElement: cs->selection().state() != SEL_SINGLE\n");
            delete el;
            return;
            }
      Element* e = cs->selection().element();
      Chord* chord;
      if (e->type() == CHORD)
            chord = static_cast<Chord*>(e);
      else if (e->type() == NOTE)
            chord = static_cast<Note*>(e)->chord();
      else {
            printf("modifyElement: no note/Chord selected:\n  ");
            e->dump();
            delete el;
            return;
            }
      switch (el->type()) {
            case ARTICULATION:
                  chord->add(static_cast<Articulation*>(el));
                  break;
            default:
                  printf("modifyElement: %s not ARTICULATION\n", el->name());
                  delete el;
                  return;
            }
      cs->setLayoutAll(true);
      }

//---------------------------------------------------------
//   cmdAddTie
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      Note* note = getSelectedNote();
      if (!note || note->tieFor()) {
            if (!note)
                  printf("cmdAddTie: no note selected\n");
            else
                  printf("cmdAddTie: has already tie? noteFor: %p\n", note->tieFor());
            return;
            }
      Chord* chord  = note->chord();
      if (noteEntryMode()) {
            if (_is.cr() == 0) {
                  printf("cmdAddTie: no pos\n");
                  return;
                  }
            startCmd();
            Note* n = addPitch(note->pitch(), false);
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
            endCmd();
            return;
            }
      ChordRest* el = nextChordRest(chord);
      if (el == 0 || el->type() != CHORD) {
            if (debugMode)
                  printf("addTie: no next chord found\n");
            return;
            }
      Note* note2 = 0;
      foreach(Note* n, static_cast<Chord*>(el)->notes()) {
            if (n->pitch() == note->pitch()) {
                  note2 = n;
                  break;
                  }
            }
      if (note2 == 0) {
            if (debugMode)
                  printf("addTie: next note for tie not found\n");
            return;
            }
      startCmd();
      Tie* tie = new Tie(this);
      tie->setStartNote(note);
      tie->setEndNote(note2);
      tie->setTrack(note->track());
      undoAddElement(tie);
      select(note2, SELECT_SINGLE, 0);
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
      layoutAll = true;
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
                        undo()->push(new SetStemDirection(chord, dir));
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
            else if (e->type() == HAIRPIN_SEGMENT) {
                  Hairpin* hp = static_cast<HairpinSegment*>(e)->hairpin();
                  undo()->push(new ChangeSubtype(hp, hp->subtype() == 0 ? 1 : 0));
                  }
            else if (e->type() == ARTICULATION) {
                  int newSubtype = -1;
                  if (e->subtype() == UfermataSym)
                        newSubtype = DfermataSym;
                  else if (e->subtype() == DfermataSym)
                        newSubtype = UfermataSym;
                  else if (e->subtype() == TenutoSym) {
                        Articulation* a = static_cast<Articulation*>(e);
                        if (a->anchor() == A_TOP_CHORD)
                              a->setAnchor(A_BOTTOM_CHORD);
                        else if (a->anchor() == A_BOTTOM_CHORD)
                              a->setAnchor(A_TOP_CHORD);
                        else if (a->anchor() == A_CHORD) {
                              ChordRest* cr = a->chordRest();
                              a->setAnchor(cr->up() ? A_TOP_CHORD : A_BOTTOM_CHORD);
                              }
                        }
                  if (newSubtype != -1)
                        undoChangeSubtype(e, newSubtype);
                  }
            else if (e->type() == TUPLET)
                  undo()->push(new FlipTupletDirection(static_cast<Tuplet*>(e)));
            }
      layoutAll = true;
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
                  s->setUserOff(pos - m->canvasPos() - off);
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
            case TEXT:
                  if (el->subtype() == TEXT_INSTRUMENT_LONG) {
                        undo()->push(new ChangeInstrumentLong(el->staff()->part(), ""));
                        break;
                        }
                  else if (el->subtype() == TEXT_INSTRUMENT_SHORT) {
                        undo()->push(new ChangeInstrumentShort(el->staff()->part(), ""));
                        break;
                        }
                  undoRemoveElement(el);
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
                  undoRemoveElement(el->parent());
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
                  if (el->voice() != 0) {
                        undoRemoveElement(el);
                        Segment* seg = rest->segment();
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
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
                  else if (seg->subtype() == SegBarLine) {
                        undoRemoveElement(el);
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
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
      int etick = tick + len;
      foreach (Beam* b, _beams) {
            ChordRest* e1 = b->elements().front();
            ChordRest* e2 = b->elements().back();
            if ((e1->tick() >= tick && e1->tick() < etick)
               || (e2->tick() >= tick && e2->tick() < etick)) {
                  undoRemoveElement(b);
                  }
            }
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
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (selection().state() == SEL_RANGE) {
            Segment* s1 = selection().startSegment();
            Segment* s2 = selection().endSegment();
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
                              if (tuplet && (tuplet->tick() == tick) && (tuplet->lastTick() < tick2) ) {
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
                              if (cr->tuplet() && (cr->tuplet()->lastTick() < tick2)) {
                                    // remove complete top level tuplet

                                    Tuplet* t = cr->tuplet();
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
                  if (f.isValid() && !f.isZero())
                        setRest(tick, track, f, false, tuplet);
                  }
            }
      else {
            // deleteItem modifies selection().elements() list,
            // so we need a local copy:
            QList<Element*> el(selection().elements());
            foreach(Element* e, el) {
                  deleteItem(e);
                  deselect(e);
                  }
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   chordTab
//---------------------------------------------------------

void ScoreView::chordTab(bool back)
      {
      Harmony* cn      = (Harmony*)editObject;
      Segment* segment = cn->segment();
      int track        = cn->track();
      if (segment == 0) {
            printf("chordTab: no segment\n");
            return;
            }

      // search next chord
      if (back)
            segment = segment->prev1(SegChordRest);
      else
            segment = segment->next1(SegChordRest);
      if (segment == 0) {
            printf("no next segment\n");
            return;
            }
      endEdit();
      _score->startCmd();

      // search for next chord name
      cn = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  cn = h;
                  break;
                  }
            }

      if (!cn) {
            cn = new Harmony(_score);
            cn->setTrack(track);
            cn->setParent(segment);
            _score->undoAddElement(cn);
            }

      _score->select(cn, SELECT_SINGLE, 0);
      startEdit(cn, -1);
      adjustCanvasPosition(cn, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menue
//---------------------------------------------------------

Lyrics* Score::addLyrics()
      {
      Note* e = getSelectedNote();
      if (e == 0)
            return 0;

      Chord* chord     = e->chord();
//      Segment* segment = chord->segment();
//      int staff        = chord->staffIdx();

      QList<Lyrics*> ll = chord->lyricsList();
      int no = ll.size();
      Lyrics* lyrics = new Lyrics(this);
      lyrics->setTrack(chord->track());
      lyrics->setParent(chord);
      lyrics->setNo(no);
      undoAddElement(lyrics);
      select(lyrics, SELECT_SINGLE, 0);
      return lyrics;
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n)
      {
      _score->startCmd();
      if (noteEntryMode()) {
            _score->expandVoice();
            _score->changeCRlen(_score->inputState().cr(), _score->inputState().duration());
            if (_score->inputState().cr())
                  cmdTuplet(n, _score->inputState().cr());
            }
      else {
            QSet<ChordRest*> set;
            foreach(Element* e, _score->selection().elements()) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if(!set.contains(cr)) {
                              cmdTuplet(n, cr);
                              set.insert(cr);
                              }
                        }
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n, ChordRest* cr)
      {
      Fraction f(cr->duration());
      int tick    = cr->tick();
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator()*2) {
            ratio /= 2;
            fr    /= 2;
            }

      Tuplet* tuplet = new Tuplet(_score);
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setDuration(f);
      Duration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);
      _score->cmdCreateTuplet(cr, tuplet);

      const QList<DurationElement*>& cl = tuplet->elements();

      int ne = cl.size();
      DurationElement* el = 0;
      if (ne && cl[0]->type() == REST)
            el  = cl[0];
      else if (ne > 1)
            el = cl[1];
      if (el) {
            _score->select(el, SELECT_SINGLE, 0);
            if (!noteEntryMode()) {
                  sm->postEvent(new CommandEvent("note-input"));
                  qApp->processEvents();
                  }
            _score->inputState().setDuration(baseLen);
            _score->setPadState();
            }
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
      Segment* segment = ocr->segment();

      undoRemoveElement(ocr);
      if (segment->isEmpty())
            undoRemoveElement(segment);
      undoAddElement(tuplet);

      ChordRest* cr;
      if (ocr->type() == CHORD) {
            cr = new Chord(this);
            foreach(Note* oldNote, static_cast<Chord*>(ocr)->notes()) {
                  Note* note = new Note(this);
                  note->setPitch(oldNote->pitch(), oldNote->tpc());
                  note->setTrack(track);
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
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->ticks());

      undoAddCR(cr, measure, tick);

      int ticks = cr->ticks();

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setDurationType(tuplet->baseLen());
            rest->setDuration(tuplet->baseLen().fraction());
#if 0
            SegmentType st = Segment::segmentType(rest->type());
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = new Segment(measure, st, tick);
                  undoAddElement(seg);
                  }
            rest->setParent(seg);
#endif
            undoAddCR(rest, measure, tick);
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   changeVoice
//---------------------------------------------------------

void ScoreView::changeVoice(int voice)
      {
      InputState* is = &score()->inputState();
      if ((is->track() % VOICES) == voice)
            return;

      is->setTrack((is->track() / VOICES) * VOICES + voice);
      //
      // in note entry mode search for a valid input
      // position
      //
      if (!is->noteEntryMode || is->cr())
            return;

      is->setSegment(is->segment()->measure()->firstCRSegment());
      moveCursor();
      score()->setUpdateAll(true);
      score()->end();
      mscore->setPos(is->segment()->tick());
      }

//---------------------------------------------------------
//   toggleInvisible
//---------------------------------------------------------

void Score::toggleInvisible(Element* e)
      {
      undoToggleInvisible(e);

      e->setGenerated(false);
      refresh |= e->abbox();
      if (e->type() == BAR_LINE) {
            Element* pe = e->parent();
            if (pe->type() == SEGMENT && pe->subtype() == SegEndBarLine) {
                  Measure* m = static_cast<Segment*>(pe)->measure();
                  BarLine* bl = static_cast<BarLine*>(e);
                  m->setEndBarLineType(bl->barLineType(), false, e->visible(), e->color());
                  }
            }
#if 0       // TODOxx
      else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_SHORT) {
            Part* part = e->staff()->part();
            part->shortName()->setVisible(e->visible());
            }
      else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_LONG) {
            Part* part = e->staff()->part();
            part->longName()->setVisible(e->visible());
            }
#endif
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
                  undo()->push(new ChangeColor(e, c));
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
#if 0 // TODOxx
                  else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_SHORT) {
                        Part* part = e->staff()->part();
                        part->shortName()->setColor(e->color());
                        }
                  else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_LONG) {
                        Part* part = e->staff()->part();
                        part->longName()->setColor(e->color());
                        }
#endif
                  }
            }
      _selection.deselectAll();
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
printf("exchange voice %d %d, tick %d-%d, measure %p-%p\n", s, d, t1, t2, m1, m2);
      for (;;) {
            undoExchangeVoice(m1, s, d, selection().staffStart(), selection().staffEnd());
            MeasureBase* mb = m1->next();
            while (mb && mb->type() != MEASURE)
                  mb = mb->next();
            m1 = static_cast<Measure*>(mb);
            if (m1 == 0 || m1 == m2)
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
      Segment* seg  = setNoteRest(_is.cr(), track, -1, d.fraction(), 0, AUTO);
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr)
            nextInputPos(cr, false);
      _is.rest = false;  // continue with normal note entry
      endCmd();
      }

//---------------------------------------------------------
//   removeChordRest
//    remove chord or rest
//    remove associated segment if empty
//    remove beam
//---------------------------------------------------------

void Score::removeChordRest(ChordRest* cr, bool clearSegment)
      {
      undoRemoveElement(cr);
      if (clearSegment) {
            Segment* seg = cr->segment();
            if (seg->isEmpty()) {
                  undoRemoveElement(seg);
                  }
            }
      if (cr->beam()) {
            Beam* beam = cr->beam();
            if (beam->generated()) {
                  beam->parent()->remove(beam);
                  delete beam;
                  }
            else {
                  undoRemoveElement(beam);
                  }
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
            Segment* s = tick2segment(cr->tick() + cr->ticks());
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? static_cast<ChordRest*>(s->element(track)) : 0;
            }
      _is.setSegment(ncr ? ncr->segment() : 0);
      if (doSelect)
            select(ncr, SELECT_SINGLE, 0);
      if (ncr)
            emit posChanged(ncr->tick());
      }

