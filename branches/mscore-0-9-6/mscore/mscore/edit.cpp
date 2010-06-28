//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
      if (el && (el->type() == REST || el->type() == REPEAT_MEASURE || el->type() == NOTE || el->type() == CHORD)) {
            if (el->type() == NOTE)
                  el = el->parent();
            return el->tick();
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
      SegmentType st = SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      return addRest(seg, track, d, tuplet);
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, int track, Duration d, Tuplet* tuplet)
      {
      Rest* rest = new Rest(this, s->tick(), d);
      rest->setTrack(track);
      rest->setParent(s);
      rest->setTuplet(tuplet);
      cmdAdd(rest);
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
      SegmentType st = SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      Chord* chord = new Chord(this);
      chord->setTuplet(tuplet);
      chord->setTrack(oc->track());
      chord->setDuration(d);
      chord->setTick(tick);
      chord->setParent(seg);
      undoAddElement(chord);

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
      return chord;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const Duration& d)
      {
      ChordRest* newcr;
      // change a RepeatMeasure() into an Rest()
      if (cr->type() == REPEAT_MEASURE)
            newcr = new Rest(*static_cast<Rest*>(cr));
      else
            newcr = static_cast<ChordRest*>(cr->clone());
      newcr->setDuration(d);
      newcr->setTuplet(cr->tuplet());
      newcr->setTick(tick);
      newcr->setSelected(false);

      Segment* seg = cr->measure()->findSegment(SegChordRest, tick);
      if (seg == 0) {
            seg = cr->measure()->createSegment(SegChordRest, tick);
            undoAddElement(seg);
            }
      newcr->setParent(seg);
      undoAddElement(newcr);
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
                  f = sigmap()->measureRest(tick);
            else
                  f = measure->fraction();

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

            const AL::SigEvent ev(sigmap()->timesig(tick));
            if (ev.nominalEqualActual()   // not in pickup measure
               && (measure->tick() == tick)
               && (measure->fraction() == f)
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
      cmdAdd(note);
      mscore->play(note);
      setLayout(chord->measure());
      select(note, SELECT_SINGLE, 0);
      return note;
      }

//---------------------------------------------------------
//   changeTimeSig
//
// change time signature at tick into subtype st for all staves
// in response to gui command (drop timesig on measure or timesig)
//---------------------------------------------------------

void Score::changeTimeSig(int tick, int timeSigSubtype)
      {
      undoFixTicks();

      // record old tick lengths, since they will be modified when time is added/removed
      QVector<int> tickLens;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            tickLens.append(m->tickLen());

      Fraction ofraction(_sigmap->timesig(tick).fraction());
      Fraction nfraction(TimeSig::getSig(timeSigSubtype));

      if (ofraction == nfraction) {
            //
            // check if there is already a time signature symbol
            //
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  if (s->subtype() != SegTimeSig)
                        continue;
                  int etick = s->tick();
                  if (etick > tick)
                        break;
                  if (etick == tick) {
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Element* e = s->element(track);
                              if (e && e->subtype() != timeSigSubtype)
                                    undoChangeSubtype(e, timeSigSubtype);
                              }
                        return;
                        }
                  }
            // no TimeSig: we have to add a symbol
            addTimeSig(tick, timeSigSubtype);
            AL::SigEvent nSig(nfraction);
            undoChangeSig(tick, AL::SigEvent(), nSig);
            return;
            }

      AL::SigEvent oSig;
      AL::SigEvent nSig;
      AL::iSigEvent i = _sigmap->find(tick);
      if (i != _sigmap->end()) {
            oSig = i->second;
            AL::SigEvent e = _sigmap->timesig(tick - 1);
            if ((tick == 0) || (e.getNominal() != nfraction)) {
                  nSig = AL::SigEvent(nfraction);
                  }
            }
      else {
            nSig = AL::SigEvent(nfraction);
            }

      undoChangeSig(tick, oSig, nSig);

      //---------------------------------------------
      // remove unnessesary timesig symbols
      //---------------------------------------------

      int staves = nstaves();
      for (Segment* segment = firstSegment(); segment;) {
            Segment* nseg = segment->next1();
            if (segment->subtype() != SegTimeSig) {
                  segment = nseg;
                  continue;
                  }
            int etick = segment->tick();
            if (etick >= tick) {
                  AL::iSigEvent i = _sigmap->find(segment->tick());
                  if ((etick > tick) && (i->second.fraction() != nfraction))
                        break;
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        Element* e = segment->element(staffIdx * VOICES);
                        if (e)
                              undoRemoveElement(e);
                        }
                  undoRemoveElement(segment);   // segment is now empty
                  if (etick > tick)
                        break;
                  }
            segment = nseg;
            }

      //---------------------------------------------
      // modify measures
      //---------------------------------------------

      int j = 0;
      int ctick = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int newLen = _sigmap->ticksMeasure(ctick);
            ctick += newLen;
            int oldLen = tickLens[j];
            ++j;
            if (newLen == oldLen)
                  continue;
            m->adjustToLen(oldLen, newLen);
            }
      if (nSig.valid())
            addTimeSig(tick, timeSigSubtype);
      undoFixTicks();
      }

//---------------------------------------------------------
//   cmdRemoveTimeSig
//---------------------------------------------------------

void Score::cmdRemoveTimeSig(TimeSig* ts)
      {
      if (ts->tick() == 0) {    // cannot remove time signature at tick 0
            QMessageBox::information(0,
               tr("MuseScore"),
               tr("The first time signature of a piece can not be removed.")
               );
            return;
            }
      undoFixTicks();
      // record old tick lengths, since they will be modified when time is added/removed
      QVector<int> tickLens;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            tickLens.append(m->tickLen());

      int tick = ts->tick();
      AL::iSigEvent si = _sigmap->find(tick);
      if (si == _sigmap->end()) {
            printf("cmdRemoveTimeSig: cannot find SigEvent at %d\n", tick);
            return;
            }
      AL::SigEvent oval = (*_sigmap)[tick];
      AL::iSigEvent nsi = si;
      ++nsi;

      undoRemoveElement(ts->segment());
      AL::SigEvent prev = _sigmap->timesig(tick-1);
      if(prev.nominalEqualActual())
          undoChangeSig(tick, oval, AL::SigEvent());
      else
          undoChangeSig(tick, oval, AL::SigEvent(prev.getNominal()));

      oval = _sigmap->timesig(tick);
      if (nsi->second == oval)
            undoChangeSig(nsi->first, oval, AL::SigEvent());

      Segment* s = ts->segment()->next1();;
      for (; s; s = s->next1()) {
            if (s->subtype() != SegTimeSig)
                  continue;
            TimeSig* e = static_cast<TimeSig*>(s->element(0));
            if (e) {
                  if (e->getSig() != oval.fraction())
                        s = 0;
                  break;
                  }
            }
      if (s)
            undoRemoveElement(s);


      //---------------------------------------------
      // modify measures
      //---------------------------------------------

      int j = 0;
      int ctick = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            int newLen = _sigmap->ticksMeasure(ctick);
            ctick += newLen;
            int oldLen = tickLens[j];
            ++j;
            if (newLen == oldLen)
                  continue;
            m->adjustToLen(oldLen, newLen);
            }
      undoFixTicks();
      }

//---------------------------------------------------------
//   addTimeSig
//---------------------------------------------------------

void Score::addTimeSig(int tick, int timeSigSubtype)
      {
      Measure* measure = tick2measure(tick);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            TimeSig* nsig = new TimeSig(this, timeSigSubtype);
            nsig->setTrack(staffIdx * VOICES);
            nsig->setTick(tick);
            SegmentType st = Segment::segmentType(TIMESIG);
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            nsig->setParent(seg);
            undoAddElement(nsig);
            }
      layoutAll = true;
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

      int tick                = p.tick;
      int staffIdx            = p.staffIdx;
      int line                = p.line;
      Staff* st               = staff(staffIdx);
      KeySigEvent key         = st->keymap()->key(tick);
      int clef                = st->clef(tick);
      int pitch               = line2pitch(line, clef, key.accidentalType);
      Part* instr             = st->part();
      _is.track               = staffIdx * VOICES + (_is.track % VOICES);
      _is.pitch               = pitch;
      int headGroup           = 0;
      Direction stemDirection = AUTO;

      if (instr->useDrumset()) {
            Drumset* ds   = instr->drumset();
            pitch         = _is.drumNote();
            if (pitch < 0)
                  return;
            // voice         = ds->voice(pitch);
            headGroup     = ds->noteHead(pitch);
            if(headGroup < 0){
                  return;
                  }
            stemDirection = ds->stemDirection(pitch);
            }

      Segment* s = p.measure->tick2segment(tick);
      if (s == 0) {
            printf("cannot put note here, no segment found\n");
            return;
            }
      _is._segment = s;
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
      if (!replace && (cr->duration() == _is.duration()) && (cr->type() == CHORD) && !_is.rest) {

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
      if (addToChord) {
            if (cr->type() == CHORD) {
                  addNote(static_cast<Chord*>(cr), pitch);
                  }
            else
                  setNoteRest(cr, _is.track, pitch, _is.duration().fraction(), headGroup, stemDirection);
            }
      else {
            // replace chord
            if (_is.rest)
                  pitch = -1;
            setNoteRest(cr, _is.track, pitch, _is.duration().fraction(), headGroup, stemDirection);
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
//   addTie
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
      layoutAll = true;
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

      int tick1 = cr1->tick();

      if (cr2 == 0)
            cr2 = nextChordRest(cr1);
      int tick2;
      if (cr2)
            tick2 = cr2->tick();
      else
            tick2 = cr1->measure()->tick() + cr1->measure()->tickLen();

      Hairpin* pin = new Hairpin(this);
      pin->setTick(tick1);
      pin->setTick2(tick2);
      pin->setSubtype(decrescendo ? 1 : 0);
      pin->setTrack(cr1->track());
      pin->layout();
      cmdAdd(pin);
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
                        _undo->push(new FlipBeamDirection(chord->beam()));
                  else {
                        Direction dir = chord->stemDirection();
                        if (dir == AUTO)
                              dir = chord->up() ? DOWN : UP;
                        else
                              dir = dir == UP ? DOWN : UP;
                        _undo->push(new SetStemDirection(chord, dir));
                        }
                  }
            else if (e->type() == SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  _undo->push(new FlipSlurDirection(slur));
                  }
            else if (e->type() == BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  _undo->push(new FlipBeamDirection(beam));
                  }
            else if (e->type() == HAIRPIN_SEGMENT) {
                  Hairpin* hp = static_cast<HairpinSegment*>(e)->hairpin();
                  _undo->push(new ChangeSubtype(hp, hp->subtype() == 0 ? 1 : 0));
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
                              ChordRest* cr = static_cast<ChordRest*>(a->parent());
                              a->setAnchor(cr->up() ? A_TOP_CHORD : A_BOTTOM_CHORD);
                              }
                        }
                  if (newSubtype != -1)
                        _undo->push(new ChangeSubtype(e, newSubtype));
                  }
            else if (e->type() == TUPLET)
                  _undo->push(new FlipTupletDirection(static_cast<Tuplet*>(e)));
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
#if 0
      if (s->anchor() == ANCHOR_STAFF) {
            int staffIdx = -1;
            int pitch, tick;
            QPointF offset;
            Segment* segment;

            MeasureBase* measure = pos2measure(pos, &tick, &staffIdx, &pitch, &segment, &offset);
            if (measure == 0 || measure->type() != MEASURE) {
                  printf("addSymbol: cannot put symbol here: no measure\n");
                  delete s;
                  return;
                  }
            offset -= off;
            s->setPos(segment->x(), 0.0);
            s->setUserOff(offset);
            s->setTick(segment->tick());
            s->setTrack(staffIdx * VOICES);
            s->setParent(measure);
            }
      else if (s->anchor() == ANCHOR_PARENT) {
#endif
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
#if 0
            }
#endif
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
                        _undo->push(new ChangeInstrumentLong(el->staff()->part(), ""));
                        break;
                        }
                  else if (el->subtype() == TEXT_INSTRUMENT_SHORT) {
                        _undo->push(new ChangeInstrumentShort(el->staff()->part(), ""));
                        break;
                        }
                  else if (el->subtype() == TEXT_COPYRIGHT) {
                        undoChangeCopyright(QString(""));
                        break;
                        }
                  else if (el->subtype() == TEXT_MEASURE_NUMBER) {
                        break;
                        }

            case SYMBOL:
            case COMPOUND:
            case DYNAMIC:
            case LYRICS:
            case ARTICULATION:
            case BRACKET:
            case VOLTA:
            case LAYOUT_BREAK:
            case CLEF:
            case IMAGE:
            case TIE:
            case TEMPO_TEXT:
            case MARKER:
            case JUMP:
            case BREATH:
            case ARPEGGIO:
            case HARMONY:
            case TREMOLO:
            case GLISSANDO:
            case STAFF_TEXT:
            case SPACER:
            case KEYSIG:
            case TIMESIG:
                  cmdRemove(el);
                  break;

            case HBOX:
            case VBOX:
                  undoRemoveElement(el);
                  break;

            case OTTAVA_SEGMENT:
            case HAIRPIN_SEGMENT:
            case TRILL_SEGMENT:
            case PEDAL_SEGMENT:
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
                        Rest* rest = new Rest(this, chord->tick(), chord->duration());
                        rest->setDuration(chord->duration());
                        rest->setTrack(el->track());
                        rest->setParent(chord->parent());
                        undoAddElement(rest);
                        if (tuplet) {
                              tuplet->add(rest);
                              rest->setTuplet(tuplet);
                              rest->setDuration(chord->duration());
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
                  undoFixTicks();
                  undoRemoveElement(el);
                  cmdRemoveTime(measure->tick(), measure->tickLen());
//                  cmdRemoveGlobals(measure->tick(), measure->tick() + measure->tickLen(), 0, staves());
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
                  printf("deleteItem: %s: not implemented\n", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   cmdRemoveTime
//---------------------------------------------------------

void Score::cmdRemoveTime(int tick, int len)
      {
      int etick = tick + len;
      foreach(Element* e, _gel) {
            if (e->type() == SLUR) {
                  Slur* slur = static_cast<Slur*>(e);
                  Element* e1 = slur->startElement();
                  Element* e2 = slur->endElement();
                  if ((e1->tick() >= tick && e1->tick() < etick)
                     || (e2->tick() >= tick && e2->tick() < etick)) {
                        undoRemoveElement(e);
                        }
                  }
            else if (e->isSLine()) {
                  SLine* s = static_cast<SLine*>(e);
                  if (s->tick() >= tick && s->tick2() < etick)
                        undoRemoveElement(e);
                  }
            }
      foreach (Beam* b, _beams) {
            Element* e1 = b->elements().front();
            Element* e2 = b->elements().back();
            if ((e1->tick() >= tick && e1->tick() < etick)
               || (e2->tick() >= tick && e2->tick() < etick)) {
                  undoRemoveElement(b);
                  }
            }

      //-----------------
      AL::SigEvent e1 = _sigmap->timesig(tick + len);
      for (AL::ciSigEvent i = _sigmap->begin(); i != _sigmap->end(); ++i) {
            if (i->first != 0 && i->first >= tick && (i->first < etick)) {
                  undoChangeSig(i->first, i->second, AL::SigEvent());
                  }
            }
      undoSigInsertTime(tick, -len);
      AL::SigEvent e2 = _sigmap->timesig(tick);
      if (!(e1 == e2)) {
            AL::ciSigEvent i = _sigmap->find(tick);
            if (i == _sigmap->end()) {
                  undoChangeSig(tick, AL::SigEvent(), e1);
                  }
            }
      //-----------------

      for (AL::ciTEvent i = _tempomap->begin(); i != _tempomap->end(); ++i) {
            if (i->first != 0 && i->first >= tick && (i->first < etick))
                  undoChangeTempo(i->first, i->second, AL::TEvent());
            }
      foreach(Staff* staff, _staves) {
            ClefList* cl = staff->clefList();
            KeyList*  kl = staff->keymap();
            for (ciClefEvent i = cl->begin(); i != cl->end(); ++i) {
                  if (i->first >= tick && (i->first < etick) && i->first != 0)
                        undoChangeClef(staff, i->first, i->second, NO_CLEF);
                  }
            for (ciKeyList i = kl->begin(); i != kl->end(); ++i) {
                  if (i->first >= tick && (i->first < etick) && i->first != 0)
                        undoChangeKey(staff, i->first, i->second, KeySigEvent());
                  }
            }
      undoInsertTime(tick, -len);
      undoFixTicks();
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
      _is._segment = 0;        // invalidate position
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
                              int offset = cr->tick() - cr->measure()->tick();
                              if (cr->measure()->tick() >= s1->tick() && offset) {
                                    f = Fraction::fromTicks(offset);
                                    tick = cr->measure()->tick();
                                    }
                              else {
                                    tick   = cr->tick();
                                    f      = Fraction();
                                    }
                              tuplet = cr->tuplet();
                              if (tuplet && (tuplet->tick() == tick) && (tuplet->lastTick() < tick2) ) {
                                    // remove complete top level tuplet

                                    Tuplet* t = cr->tuplet();
                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    f += t->fraction();
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
                                    f += t->fraction();
                                    tuplet = 0;
                                    continue;
                                    }
                              if (f.isValid())
                                    setRest(tick, track, f, false, tuplet);
                              tick = cr->tick();
                              tuplet = cr->tuplet();
                              removeChordRest(cr, true);
                              f = cr->fraction();
                              }
                        else {
                              removeChordRest(cr, true);
                              f += cr->fraction();
                              }
                        }
                  if (f.isValid())
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
      Measure* measure = (Measure*)cn->parent();
      Segment* segment = measure->tick2segment(cn->tick());
      int track        = cn->track();
      if (segment == 0) {
            printf("chordTab: no segment\n");
            return;
            }

      // search next chord
      if (back) {
            while ((segment = segment->prev1())) {
                  if (segment->subtype() == SegChordRest)
                        break;
                  }
            }
      else {
            while ((segment = segment->next1())) {
                  if (segment->subtype() == SegChordRest)
                        break;
                  }
            }
      if (segment == 0) {
            printf("no next segment\n");
            return;
            }
      endEdit();
      _score->startCmd();

      // search for next chord name
      cn              = 0;
      measure         = segment->measure();
      ElementList* el = measure->el();
      foreach(Element* e, *el) {
            if (e->type() == HARMONY && e->tick() == segment->tick()) {
                  cn = static_cast<Harmony*>(e);
                  break;
                  }
            }

      if (!cn) {
            cn = new Harmony(_score);
            cn->setTick(segment->tick());
            cn->setTrack(track);
            cn->setParent(measure);
            _score->undoAddElement(cn);
            }

      _score->select(cn, SELECT_SINGLE, 0);
      startEdit(cn, -1);
      adjustCanvasPosition(cn, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   changeLineSegment
//    switch to first/last LineSegment while editing
//---------------------------------------------------------

void ScoreView::changeLineSegment(bool last)
      {
      LineSegment* segment = static_cast<LineSegment*>(editObject);

      LineSegment* newSegment;
      if (last)
            newSegment = segment->line()->lineSegments().back();
      else
            newSegment = segment->line()->lineSegments().front();

      endEdit();
      _score->endCmd();

      _score->startCmd();
      startEdit(newSegment, -2);      // do not change curGrip
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
      Segment* segment = chord->segment();
      int tick         = chord->tick();
      int staff        = chord->staffIdx();

      Lyrics* lyrics;
      LyricsList* ll = segment->lyricsList(staff);
      int no = 0;
      if (ll)
            no = ll->size();
      lyrics = new Lyrics(this);
      lyrics->setTick(tick);
      lyrics->setTrack(chord->track());
      lyrics->setParent(segment);
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
            foreach(Element* e, _score->selection().elements()) {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  if (e->isChordRest())
                        cmdTuplet(n, static_cast<ChordRest*>(e));
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n, ChordRest* cr)
      {
      Fraction f  = cr->fraction();
      int tick    = cr->tick();
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
//       while (qAbs(ratio.numerator() - ratio.denominator()) > qAbs(ratio.numerator() - ratio.denominator() * 2)) {
      while (ratio.numerator() >= ratio.denominator()*2) {
            ratio /= 2;
            fr    /= 2;
            }

//      if (ratio == Fraction(1,1))   // this is not a tuplet
//            return;

      Tuplet* tuplet = new Tuplet(_score);
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setFraction(f);
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
  qPrintable(ocr->fraction().print()),
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
            Note* note = new Note(this);
            note->setPitch(static_cast<Chord*>(ocr)->upNote()->pitch(), static_cast<Chord*>(ocr)->upNote()->tpc());
            note->setTrack(track);
            cr->add(note);
            }
      else
            cr = new Rest(this);

      Fraction an     = (tuplet->fraction() * tuplet->ratio()) / tuplet->baseLen().fraction();
      int actualNotes = an.numerator() / an.denominator();
            // tuplet->ratio().numerator();

      cr->setTick(tick);
      cr->setTuplet(tuplet);
      cr->setTrack(track);
      cr->setDuration(tuplet->baseLen());

printf("tuplet note duration %s  actualNotes %d  ticks %d\n",
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->ticks());

      SegmentType st = Segment::segmentType(cr->type());
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      cr->setParent(seg);
      undoAddElement(cr);

      int ticks = cr->ticks();

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTick(tick);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setDuration(tuplet->baseLen());
            SegmentType st = Segment::segmentType(rest->type());
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            rest->setParent(seg);
            undoAddElement(rest);
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   changeVoice
//---------------------------------------------------------

void ScoreView::changeVoice(int voice)
      {
      InputState* is = &score()->inputState();
      if ((is->track % VOICES) == voice)
            return;

      is->track = (is->track / VOICES) * VOICES + voice;
      //
      // in note entry mode search for a valid input
      // position
      //
      if (!is->noteEntryMode || is->cr())
            return;

      is->_segment = is->_segment->measure()->firstCRSegment();
      moveCursor();
      score()->setUpdateAll(true);
      score()->end();
      mscore->setPos(is->_segment->tick());
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
                  m->setEndBarLineType(e->subtype(), false, e->visible(), e->color());
                  }
            }
      else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_SHORT) {
            Part* part = e->staff()->part();
            part->shortName()->setVisible(e->visible());
            }
      else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_LONG) {
            Part* part = e->staff()->part();
            part->longName()->setVisible(e->visible());
            }
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
                  _undo->push(new ChangeColor(e, c));
                  e->setGenerated(false);
                  refresh |= e->abbox();
                  if (e->type() == BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == SEGMENT && ep->subtype() == SegEndBarLine) {
                              Measure* m = static_cast<Segment*>(ep)->measure();
                              m->setEndBarLineType(e->subtype(), false, e->visible(), e->color());
                              }
                        }
                  else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_SHORT) {
                        Part* part = e->staff()->part();
                        part->shortName()->setColor(e->color());
                        }
                  else if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_LONG) {
                        Part* part = e->staff()->part();
                        part->longName()->setColor(e->color());
                        }
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
      startCmd();
      expandVoice();
      if (_is.cr() == 0) {
            printf("cannot enter rest here\n");
            return;
            }

      int track = _is.track;
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
printf("cmdDeleteTuplet\n");
      foreach(DurationElement* de, tuplet->elements()) {
            if (de->type() == CHORD || de->type() == REST)
                  removeChordRest(static_cast<ChordRest*>(de), true);
            else
                  cmdDeleteTuplet(static_cast<Tuplet*>(de), false);
            }
      undoRemoveElement(tuplet);
      if (replaceWithRest) {
            Rest* rest = setRest(tuplet->tick(), tuplet->track(), tuplet->fraction(), true, 0);
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
      if ((ncr == 0) && (_is.track % VOICES)) {
            Segment* s = tick2segment(cr->tick() + cr->ticks());
            int track = (cr->track() / VOICES) * VOICES;
            ncr = s ? static_cast<ChordRest*>(s->element(track)) : 0;
            }
      _is._segment = ncr ? ncr->segment() : 0;
      if (doSelect)
            select(ncr, SELECT_SINGLE, 0);
      if (ncr)
            emit posChanged(ncr->tick());
      }

