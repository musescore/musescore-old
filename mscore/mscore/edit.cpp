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

#include "canvas.h"
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
#include "tempo.h"
#include "undo.h"

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = selection()->element();
      if (el) {
            if (el->type() == NOTE)
                  return (Note*)el;
            }
      selectNoteMessage();
      return 0;
      }

//---------------------------------------------------------
//   getSelectedChordRest
//---------------------------------------------------------

ChordRest* Score::getSelectedChordRest() const
      {
      Element* el = selection()->element();
      if (el) {
            if (el->type() == NOTE)
                  return ((Note*)el)->chord();
            else if (el->type() == REST || el->type() == REPEAT_MEASURE)
                  return (Rest*)el;
            }
      selectNoteRestMessage();
      return 0;
      }

//---------------------------------------------------------
//   pos
//---------------------------------------------------------

int Score::pos()
      {
      Element* el = selection()->element();
      if (selection()->activeCR())
            el = selection()->activeCR();
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

Rest* Score::addRest(int tick, int track, Duration d)
      {
      Measure* measure = tick2measure(tick);
      Segment::SegmentType st = Segment::SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      return addRest(seg, track, d);
      }

//---------------------------------------------------------
//   addRest
//---------------------------------------------------------

Rest* Score::addRest(Segment* s, int track, Duration d)
      {
      Measure* m = s->measure();
      int tick = s->tick();
      if ((m->tick() == tick) && (m->tickLen() == d.ticks()) && (d < Duration(Duration::V_BREVE)))
            d.setType(Duration::V_MEASURE);
      Rest* rest = new Rest(this, tick, d);
      rest->setTrack(track);
      rest->setParent(s);
      cmdAdd(rest);
      return rest;
      }

//---------------------------------------------------------
//   addClone
//---------------------------------------------------------

ChordRest* Score::addClone(ChordRest* cr, int tick, const Duration& d)
      {
      ChordRest* newcr = (ChordRest*)cr->clone();
      newcr->setDuration(d);
      newcr->setTuplet(cr->tuplet());
      newcr->setTick(tick);
      Segment* seg = cr->measure()->findSegment(Segment::SegChordRest, tick);
      if (seg == 0) {
            seg = cr->measure()->createSegment(Segment::SegChordRest, tick);
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

void Score::setRest(int tick, int track, Fraction l)
      {
      //
      // compute list of durations which will fit l
      //
      QList<Duration> dList = toDurationList(l);
      if (dList.isEmpty())
            return;

      Measure* measure = tick2measure(tick);
      if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
            foreach(Duration d, dList)
                  tick += addRest(tick, track, d)->ticks();
            }
      else {
            for (int i = dList.size() - 1; i >= 0; --i)
                  tick += addRest(tick, track, dList[i])->ticks();
            }
      }

//---------------------------------------------------------
//   setRest
//---------------------------------------------------------

/**
 Set rest(\a len) at position \a tick / \a track
 return false if rest could not be set
*/

void Score::setRest(int tick, int track, Fraction l, bool useDots)
      {
printf("setRest at %d(%d) len %d/%d useDots %d\n", tick, track, l.zaehler(), l.nenner(), useDots);
      setRest(tick, track, l);
#if 0 // TODO-Z
      int stick = tick;
      Measure* measure = tick2measure(stick);
      if (measure == 0 || (measure->tick() + measure->tickLen()) == tick) {
            printf("setRest(%d,%d,%d,%d):  ...measure not found (%p)\n", tick, track, int(len.type()), useDots, measure);
            return false;
            }
      int measureEndTick = measure->tick() + measure->tickLen();
      if (tick + len > measureEndTick)
            len -= (tick + len) - measureEndTick;
      if (len <= 0)
            return false;

      setLayout(measure);
      Segment* segment = measure->first();
      int noteLen      = 0;
      while (segment) {
            for (; segment; segment = segment->next()) {
                  if (segment->tick() >= stick)
                        break;
                  }
            if (segment == 0 || segment->tick() != stick) {
                  printf("setRest: ..no segment at tick %d found\n", stick);
                  return false;
                  }
            Element* element = segment->element(track);
            int l = 0;
            if (element && element->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  if (cr->tuplet()) {
                        l = cr->tuplet()->tickLen();
                        cmdDeleteTuplet(cr->tuplet(), false);
                        }
                  else {
                        l = cr->tickLen();
                        if (l == 0)
                              l = measure->tickLen();
                        undoRemoveElement(element);
                        if (segment->isEmpty())
                              undoRemoveElement(segment);
                        }
                  }
            // do not count len of grace note
            if (segment->subtype() == Segment::SegGrace) {
                  l = 0;
                  segment = segment->next();
                  }
            else {
                  segment = segment->next();
                  if (l == 0) {
                        if (segment == 0)
                              l = measure->tick() + measure->tickLen() - stick;
                        else
                              l = segment->tick() - stick;
                        }
                  }
            noteLen += l;
            stick   += l;
            if (noteLen >= len)     // collected enough time?
                  break;
            }
      if (noteLen < len)
            printf("setRest: cannot find segment! rest: %d\n", len - noteLen);

      Rest* rest = 0;
      Duration d;
      d.setVal(len);
      if (useDots) {
            rest = new Rest(this);
            rest->setTick(tick);
            rest->setDuration(d);
            rest->setTrack(track);
            Segment::SegmentType st = Segment::SegChordRest;
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            rest->setParent(seg);
            undoAddElement(rest);
            }
      else {
            rest = setRest(tick, d, track);
            }
      if (rest)
            select(rest, SELECT_SINGLE, 0);
      int restLen = noteLen - len;
      if (restLen > 0) {
            Duration d;
            d.setVal(restLen);
            setRest(tick + len, d, track);
            }
      layoutAll = true;
#endif
      }


//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

Note* Score::addNote(Chord* chord, int pitch)
      {
      Note* note = new Note(this);
      note->setPitch(pitch);
      note->setParent(chord);
      cmdAdd(note);
      spell(note);
      setLayout(chord->measure());
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

      // record old tickLens, since they will be modified when time is added/removed
      QVector<int> tickLens;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = static_cast<Measure*>(mb);
            tickLens.append(m->tickLen());
            }

      int oz, on;
      sigmap->timesig(tick, oz, on);

      int z, n;
      TimeSig::getSig(timeSigSubtype, &n, &z);
      if ((oz == z) && (on == n)) {
            //
            // check if there is already a time signature symbol
            //
            Segment* ts = 0;
            for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        if (segment->subtype() != Segment::SegTimeSig)
                              continue;
                        int etick = segment->tick();
                        if (etick == tick) {
                              ts = segment;
                              break;
                              }
                        }
                  if (ts) {
                        for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Element* e = ts->element(track);
                              if (e && e->subtype() != timeSigSubtype)
                                    undoChangeSubtype(e, timeSigSubtype);
                              }
                        return;
                        }
                  }
            // no: we have to add a symbol
            addTimeSig(tick, timeSigSubtype);
            return;
            }

      SigEvent oSig;
      SigEvent nSig;
      iSigEvent i = sigmap->find(tick);
      if (i != sigmap->end()) {
            oSig = i->second;
            SigEvent e = sigmap->timesig(tick - 1);
            if ((tick == 0) || (e.nominator != z) || (e.denominator != n)) {
                  nSig = SigEvent(z, n);
                  }
            }
      else {
            nSig = SigEvent(z, n);
            }

      undoChangeSig(tick, oSig, nSig);

      //---------------------------------------------
      // remove unnessesary timesig symbols
      //---------------------------------------------

      int staves = nstaves();
      Segment* segment = 0;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() == MEASURE) {
                  segment = ((Measure*)mb)->first();
                  break;
                  }
            }
      for (; segment;) {
            Segment* nseg = segment->next1();
            if (segment->subtype() != Segment::SegTimeSig) {
                  segment = nseg;
                  continue;
                  }
            int etick = segment->tick();
            if (etick >= tick) {
                  iSigEvent i = sigmap->find(segment->tick());
                  if ((etick > tick) && (i->second.nominator != z || i->second.denominator != n))
                        break;
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        Element* e = segment->element(staffIdx * VOICES);
                        if (e)
                              undoRemoveElement(e);
                        }
                  undoRemoveElement(segment);
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
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = static_cast<Measure*>(mb);
            int newLen = sigmap->ticksMeasure(ctick);
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
//   addTimeSig
//---------------------------------------------------------

void Score::addTimeSig(int tick, int timeSigSubtype)
      {
      Measure* measure = tick2measure(tick);
      for (int staffIdx = 0; staffIdx < _staves.size(); ++staffIdx) {
            TimeSig* nsig = new TimeSig(this, timeSigSubtype);
            nsig->setTrack(staffIdx * VOICES);
            nsig->setTick(tick);
            Segment::SegmentType st = Segment::segmentType(TIMESIG);
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
      int key                 = st->keymap()->key(tick);
      int clef                = st->clef(tick);
      int pitch               = line2pitch(line, clef, key);
      Instrument* instr       = st->part()->instrument();
      _is.track               = staffIdx * VOICES + (_is.track % VOICES);
      _is.pitch               = pitch;
      int headGroup           = 0;
      Direction stemDirection = AUTO;

      if (instr->useDrumset) {
            Drumset* ds   = instr->drumset;
            pitch         = _is.drumNote;
            if (pitch < 0)
                  return;
            // voice         = ds->voice(pitch);
            headGroup     = ds->noteHead(pitch);
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

      if (!replace && (cr->duration() == _is.duration) && (cr->type() == CHORD) && !_is.rest) {
            const NoteList* nl = static_cast<Chord*>(cr)->noteList();
            Note* note = nl->find(pitch);
            if (note) {
                  // remove note from chord
                  if (nl->size() > 1)
                        undoRemoveElement(note);
                  return;
                  }
            addToChord = true;
            }
      if (addToChord) {
            if (cr->type() == CHORD) {
                  Note* note = addNote(static_cast<Chord*>(cr), pitch);
                  select(note, SELECT_SINGLE, 0);
                  }
            else
                  setNoteRest(cr, _is.track, pitch, _is.duration, headGroup, stemDirection);
            }
      else {
            // replace chord
            if (_is.rest)
                  pitch = -1;
            setNoteRest(cr, _is.track, pitch, _is.duration, headGroup, stemDirection);
            }
      moveToNextInputPos();
      }

//---------------------------------------------------------
//   modifyElement
//---------------------------------------------------------

void Canvas::modifyElement(Element* el)
      {
      if (el == 0) {
            printf("modifyElement: el==0\n");
            return;
            }
      Score* cs = el->score();
      if (cs->selection()->state() != SEL_SINGLE) {
            printf("modifyElement: cs->selection()->state() != SEL_SINGLE\n");
            delete el;
            return;
            }
      Element* e = cs->selection()->element();
      Chord* chord;
      if (e->type() == CHORD)
            chord = (Chord*) e;
      else if (e->type() == NOTE)
            chord = ((Note*)e)->chord();
      else {
            printf("modifyElement: no note/Chord selected:\n  ");
            e->dump();
            delete el;
            return;
            }
      switch (el->type()) {
            case ARTICULATION:
                  chord->add((Articulation*)el);
                  break;
            default:
                  printf("modifyElement: %s not ARTICULATION\n", el->name());
                  delete el;
                  return;
            }
      cs->setLayoutAll(true);
      }

//---------------------------------------------------------
//   addSlur
//    'S' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddSlur()
      {
      if (noteEntryMode() && _is.slur) {
            QList<SlurSegment*>* el = _is.slur->slurSegments();
            if (!el->isEmpty())
                  el->front()->setSelected(false);
            ((ChordRest*)_is.slur->startElement())->addSlurFor(_is.slur);
            ((ChordRest*)_is.slur->endElement())->addSlurBack(_is.slur);
            _is.slur = 0;
            return;
            }
      Note* note = getSelectedNote();
      if (!note)
            return;
      cmdAddSlur(note);
      }

//---------------------------------------------------------
//   addSlur
//---------------------------------------------------------

void Score::cmdAddSlur(Note* note)
      {
      ChordRest* cr1 = note->chord();
      ChordRest* cr2 = nextChordRest(cr1);

      if (cr2 == 0) {
            printf("cannot create slur: at end\n");
            return;
            }
      Slur* slur = new Slur(this);
      slur->setStartElement(cr1);
      slur->setEndElement(cr2);
      slur->setParent(0);
      cmdAdd(slur);

      slur->layout();
      QList<SlurSegment*>* el = slur->slurSegments();

      if (noteEntryMode()) {
            _is.slur = slur;
            if (!el->isEmpty())
                  el->front()->setSelected(true);
            ((ChordRest*)slur->startElement())->removeSlurFor(slur); // set again when leaving slur mode
            ((ChordRest*)slur->endElement())->removeSlurBack(slur);
            }
      else {
            //
            // start slur in edit mode
            //
            if (!el->isEmpty()) {
                  SlurSegment* ss = el->front();
                  canvas()->startEdit(ss);
                  }
            }
      }

//---------------------------------------------------------
//   addTie
//    shift+'S' typed on keyboard
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
            Note* n = cmdAddPitch1(note->pitch(), false);
            if (n) {
                  Tie* tie = new Tie(this);
                  tie->setStartNote(note);
                  tie->setEndNote(n);
                  tie->setTrack(note->track());
                  note->setTieFor(tie);
                  n->setTieBack(tie);
                  undoAddElement(tie);
                  }
            return;
            }
      ChordRest* el = nextChordRest(chord);
      if (el == 0 || el->type() != CHORD) {
            if (debugMode)
                  printf("addTie: no next chord found\n");
            return;
            }
      NoteList* nl = static_cast<Chord*>(el)->noteList();
      Note* note2 = 0;
      for (iNote i = nl->begin(); i != nl->end(); ++i) {
            if (i->second->pitch() == note->pitch()) {
                  note2 = i->second;
                  break;
                  }
            }
      if (note2 == 0) {
            if (debugMode)
                  printf("addTie: next note for tie not found\n");
            return;
            }

      Tie* tie = new Tie(this);
      tie->setStartNote(note);
      tie->setEndNote(note2);
      tie->setTrack(note->track());
      undoAddElement(tie);
      layoutAll = true;
      select(note2, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    'H' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddHairpin(bool decrescendo)
      {
      ChordRest* cr = getSelectedChordRest();
      if (!cr)
            return;

      int tick1 = cr->tick();

      ChordRest* cr2 = nextChordRest(cr);
      int tick2;
      if (cr2)
            tick2 = cr2->tick();
      else
            tick2 = cr->measure()->tick() + cr->measure()->tickLen();

      Hairpin* pin = new Hairpin(this);
      pin->setTick(tick1);
      pin->setTick2(tick2);
      pin->setSubtype(decrescendo ? 1 : 0);
      pin->setTrack(cr->track());
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
      QList<Element*>* el = selection()->elements();
      if (el->isEmpty()) {
            selectNoteSlurMessage();
            return;
            }
      foreach(Element* e, *el) {
            if (e->type() == NOTE) {
                  Chord* chord = static_cast<Note*>(e)->chord();

                  Direction dir = chord->stemDirection();

                  if (dir == AUTO)
                        dir = chord->up() ? DOWN : UP;
                  else
                        dir = dir == UP ? DOWN : UP;

                  _undo->push(new SetStemDirection(chord, dir));

                  Beam* beam = chord->beam();
                  if (beam) {
                        bool set = false;
                        QList<ChordRest*> elements = beam->elements();
                        for (int i = 0; i < elements.size(); ++i) {
                              ChordRest* cr = elements[i];
                              if (!set) {
                                    if (cr->type() == CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          if (chord->stemDirection() != dir)
                                                _undo->push(new SetStemDirection(chord, dir));
                                          set = true;
                                          }
                                    }
                              else {
                                    if (cr->type() == CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          if (chord->stemDirection() != AUTO)
                                                _undo->push(new SetStemDirection(chord, AUTO));
                                          }
                                    }
                              }

                        }
                  else {
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
                              a->setAnchor(cr->isUp() ? A_TOP_CHORD : A_BOTTOM_CHORD);
                              }
                        }
                  if (newSubtype != -1)
                        _undo->push(new ChangeSubtype(e, newSubtype));
                  }
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
                  if (chord->noteList()->size() > 1) {
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
                  if ((el->voice() == 0 || tuplet) && (chord->noteType() == NOTE_NORMAL)) {
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
                  addAccidental(static_cast<Note*>(el->parent()), ACC_NONE);
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl  = static_cast<BarLine*>(el);
                  Segment* seg = bl->segment();
                  Measure* m   = seg->measure();
                  if (seg->subtype() == Segment::SegStartRepeatBarLine)
                        undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatStart);
                  else if (seg->subtype() == Segment::SegBarLine) {
                        undoRemoveElement(el);
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
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
      int idx = 0;
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
            ++idx;
            }

      int tick2 = tick + len;
      foreach(Element* el, _gel) {
            if (el->type() == SLUR) {
                  Slur* s = (Slur*) el;
                  if (s->tick() >= tick && s->tick2() < tick2)
                        undoRemoveElement(el);
                  }
            else if (el->isSLine()) {
                  SLine* s = (SLine*) el;
                  if (s->tick() >= tick && s->tick2() < tick2)
                        undoRemoveElement(el);
                  }
            }

      //-----------------
      SigEvent e1 = sigmap->timesig(tick + len);
      for (ciSigEvent i = sigmap->begin(); i != sigmap->end(); ++i) {
            if (i->first >= tick && (i->first < tick2))
                  undoChangeSig(i->first, i->second, SigEvent());
            }
      undoSigInsertTime(tick, -len);
      SigEvent e2 = sigmap->timesig(tick);
      if (!(e1 == e2)) {
            ciSigEvent i = sigmap->find(tick);
            if (i == sigmap->end())
                  undoChangeSig(tick, SigEvent(), e1);
            }
      //-----------------

      for (ciTEvent i = tempomap->begin(); i != tempomap->end(); ++i) {
            if (i->first >= tick && (i->first < tick2))
                  undoChangeTempo(i->first, i->second, TEvent());
            }
      foreach(Staff* staff, _staves) {
            ClefList* cl = staff->clefList();
            KeyList*  kl = staff->keymap();
            for (ciClefEvent i = cl->begin(); i != cl->end(); ++i) {
                  if (i->first >= tick && (i->first < tick2) && i->first != 0)
                        undoChangeClef(staff, i->first, i->second, NO_CLEF);
                  }
            for (ciKeyEvent i = kl->begin(); i != kl->end(); ++i) {
                  if (i->first >= tick && (i->first < tick2) && i->first != 0)
                        undoChangeKey(staff, i->first, i->second, NO_KEY);
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
      MeasureBase* is = selection()->startSegment()->measure();
      bool createEndBar = false;
      if (is->next()) {
            MeasureBase* ie = selection()->endSegment()->measure();
            if (ie) {
                  if (ie->tick() < selection()->endSegment()->tick()) {
                        // if last measure is selected
                        deleteItem(ie);
                        createEndBar = true;
                        }
                  do {
                        ie = ie->prev();
                        deleteItem(ie);
                        } while (ie != is);
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
      selection()->elements()->clear();
      select(0, SELECT_SINGLE, 0);
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (selection()->state() == SEL_SYSTEM) {
            cmdDeleteSelectedMeasures();
            return;
            }

      if (selection()->state() == SEL_STAFF) {
            Segment* s1 = selection()->startSegment();
            Segment* s2 = selection()->endSegment();
            int track1  = selection()->staffStart * VOICES;
            int track2  = selection()->staffEnd * VOICES;
printf("delete staff %d-%d\n", track1, track2);
            for (Segment* s = s1; s != s2; s = s->next1()) {
                  if (s->subtype() != Segment::SegChordRest)
                        continue;
                  for (int track = track1; track < track2; ++track) {
//                        if (track % VOICES == 0)
//                              continue;
                        if (s->element(track)) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                              if (cr->tuplet()) {
                                    //
                                    // remove top level tuplet
                                    //
                                    Tuplet* t = cr->tuplet();
                                    while (t->tuplet())
                                          t = t->tuplet();
                                    cmdDeleteTuplet(t, false);
                                    continue;
                                    }
printf("  delete %s %s\n", cr->name(), qPrintable(cr->duration().name()));
                              undoRemoveElement(cr);
                              }
                        }
                  if (s->isEmpty())
                        undoRemoveElement(s);
                  }

            for (int staffIdx = selection()->staffStart; staffIdx < selection()->staffEnd; ++staffIdx) {
                  int tick   = s1->tick();
                  int gapLen;
                  if (s2)
                        gapLen = s2->tick() - tick;
                  else {
                        MeasureBase* m = measures()->last();
                        gapLen = m->tick() - tick;
                        if (m->type() == MEASURE)
                              gapLen += static_cast<Measure*>(m)->tickLen();
                        }

                  while (gapLen) {
                        Measure* m = tick2measure(tick);
                        int maxGap = m->tick() + m->tickLen() - tick;
                        int len    = gapLen > maxGap ? maxGap : gapLen;
                        Duration d;
                        d.setVal(len);
                        Fraction f = d.fraction();
printf("  setRest ticks %d  %d/%d\n", len, f.zaehler(), f.nenner());
                        setRest(tick, staffIdx * VOICES, f, false);
                        gapLen -= len;
                        tick   += len;
                        }
                  }
            }
      else {
printf("delete elements\n");
            // deleteItem modifies selection()->elements() list,
            // so we need a local copy:
            foreach(Element* e, *selection()->elements()) {
                  e->setSelected(false);  // in case item is not deleted
                  deleteItem(e);
                  }
            }
      selection()->elements()->clear();
      select(0, SELECT_SINGLE, 0);
      layoutAll = true;
      }

//---------------------------------------------------------
//   chordTab
//---------------------------------------------------------

void Score::chordTab(bool back)
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
                  if (segment->subtype() == Segment::SegChordRest)
                        break;
                  }
            }
      else {
            while ((segment = segment->next1())) {
                  if (segment->subtype() == Segment::SegChordRest)
                        break;
                  }
            }
      if (segment == 0) {
            printf("no next segment\n");
            return;
            }

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      startCmd();

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
            cn = new Harmony(this);
            cn->setTick(segment->tick());
            cn->setTrack(track);
            cn->setParent(measure);
            undoAddElement(cn);
            }

      select(cn, SELECT_SINGLE, 0);
      canvas()->startEdit(cn);
      adjustCanvasPosition(cn, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   changeLineSegment
//    switch to first/last LineSegment while editing
//---------------------------------------------------------

void Score::changeLineSegment(bool last)
      {
      LineSegment* segment = static_cast<LineSegment*>(editObject);

      LineSegment* newSegment;
      if (last)
            newSegment = segment->line()->lineSegments().back();
      else
            newSegment = segment->line()->lineSegments().front();

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      startCmd();
      canvas()->startEdit(newSegment, -2);      // do not change curGrip
      layoutAll = true;
      }

//---------------------------------------------------------
//   addLyrics
//    called from Keyboard Accelerator & menue
//---------------------------------------------------------

void Score::addLyrics()
      {
      Note* e = getSelectedNote();
      if (e == 0) {
            endCmd();
            return;
            }

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
      canvas()->startEdit(lyrics);
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void Score::cmdTuplet(int n)
      {
      ChordRest* cr;
      Duration duration;

      if (noteEntryMode()) {
            cr       = _is.cr();
            duration = _is.duration;
            }
      else {
            cr = getSelectedChordRest();
            if (cr == 0)
                  return;
            duration = cr->duration();
            if (duration.type() == Duration::V_MEASURE) {
                  int tz, tn;
                  getSigmap()->timesig(cr->tick(), tz, tn);
                  Fraction f(tz, tn);
                  duration = Duration(f);
                  }
            }
      int tick = cr->tick();
      if (duration.dots() > 1) {
            printf("cannot create tuplet for this duration\n");
            return;
            }
      Fraction f(duration.fraction());

      Tuplet* tuplet = new Tuplet(this);
      Tuplet* ot = cr->tuplet();

      if (duration.dots() == 0) {
            switch (n) {
                  case 2:                       // duplet
                  case 4:                       // quadruplet
                  case 8:                       // octuplet
                        printf("this does not create a tuplet\n");
                        return;
                  case 3:                       // triplet
                        tuplet->setRatio(3, 2);
                        duration = duration.shift(1);
                        break;
                  case 5:                       // quintuplet
                        {
                        // HACK: whole measure rest with time signature 6/8
                        int tz, tn;
                        getSigmap()->timesig(tick, tz, tn);
                        int ticks = duration.type() != Duration::V_MEASURE ? cr->ticks() : cr->measure()->tickLen();
                        if (ticks == (3 * division) &&  tz == 6 && tn == 8)
                              tuplet->setRatio(5, 6);
                        else {
                              //duration = duration.shift(1);
                              //tuplet->setRatio(5, 2);
                              // alternativ:
                              duration = duration.shift(2);
                              tuplet->setRatio(5, 4);
                              }
                        }
                        break;
                  case 6:                       // sextuplet
                        tuplet->setRatio(6, 4);
                        duration = duration.shift(1);
                        break;
                  case 7:                       // septuplet
                        tuplet->setRatio(7, 4);
                        // tuplet->setRatio(6, 4);  alternative
                        duration = duration.shift(2);
                        break;
                  case 9:                       // nonuplet
                        tuplet->setRatio(9, 8);
                        duration = duration.shift(3);
                        break;
                  }
            }
      else {
            duration.setDots(0);
            switch (n) {
                  case 3:                       // triplet
                  case 6:                       // sextuplet
                        printf("this does not create a tuplet\n");
                        return;
                  case 2:                       // duplet
                        tuplet->setRatio(4, 3);
                        duration = duration.shift(1);
                        break;
                  case 4:                       // quadruplet
                        tuplet->setRatio(4, 3);
                        duration = duration.shift(1);
                        break;
                  case 5:                       // quintuplet
                        tuplet->setRatio(5, 3);
                        duration = duration.shift(1);
                        break;
                  case 7:                       // septuplet
                        tuplet->setRatio(7, 6);
                        duration = duration.shift(2);
                        break;
                  case 8:                       // octuplet
                        tuplet->setRatio(8, 6);
                        duration = duration.shift(1);
                        break;
                  case 9:                       // nonuplet
                        tuplet->setRatio(9, 6);
                        duration = duration.shift(2);
                        break;
                  }
            }
      if (noteEntryMode() && (duration != cr->duration())) {
            cmdEnterRest();
            cr = getSelectedChordRest();
            }

      //
      // "duration" is the duration of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setDuration(cr->duration());
      tuplet->setBaseLen(duration);

      printf("cmdTuplet: "); tuplet->dump();

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);
      cmdCreateTuplet(cr, tuplet);

      const QList<DurationElement*>& cl = tuplet->elements();

      int ne = cl.size();
      DurationElement* el = 0;
      if (ne && cl[0]->type() == REST)
            el  = cl[0];
      else if (ne > 1)
            el = cl[1];
      if (el) {
            select(el, SELECT_SINGLE, 0);
            setNoteEntry(true);
            _is.duration = duration;      // ??
            setPadState();
            }
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//    replace cr with tuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(ChordRest* cr, Tuplet* tuplet)
      {
printf("createTuplet\n");

      int track        = cr->track();
      Measure* measure = cr->measure();
      int tick         = cr->tick();
      Segment* segment = cr->segment();

printf("  remove %p\n", cr);
      undoRemoveElement(cr);
      if (segment->isEmpty())
            undoRemoveElement(segment);
      undoAddElement(tuplet);

      if (cr->type() == CHORD) {
            cr = new Chord(this);
            Note* note = new Note(this);
            note->setPitch(getSelectedNote()->pitch());
            note->setTpc(getSelectedNote()->tpc());
            note->setTrack(track);
            cr->add(note);
            }
      else
            cr = new Rest(this);

      int actualNotes  = tuplet->actualNotes();

      cr->setTick(tick);
      cr->setTuplet(tuplet);
      cr->setTrack(track);
      cr->setDuration(tuplet->baseLen());

printf("tuplet note duration %s  actualNotes %d  ticks %d\n",
      qPrintable(tuplet->baseLen().name()), actualNotes, cr->ticks());

      Segment::SegmentType st = Segment::segmentType(cr->type());
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
            Segment::SegmentType st = Segment::segmentType(rest->type());
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

void Score::changeVoice(int voice)
      {
      if ((_is.track % VOICES) == voice)
            return;

      _is.track = (_is.track / VOICES) * VOICES + voice;
      //
      // in note entry mode search for a valid input
      // position
      //
      if (!_is.noteEntryMode || _is.cr())
            return;

      _is._segment = _is._segment->measure()->firstCRSegment();
      emit posChanged(_is._segment->tick());

//      foreach(Viewer* v, viewer)
//            v->moveCursor();
      updateAll = true;
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

      foreach(Element* e, *selection()->elements()) {
            if (e->color() != c) {
                  _undo->push(new ChangeColor(e, c));
                  e->setGenerated(false);
                  refresh |= e->abbox();
                  if (e->type() == BAR_LINE) {
                        Element* ep = e->parent();
                        if (ep->type() == SEGMENT && ep->subtype() == Segment::SegEndBarLine) {
                              Measure* m = static_cast<Segment*>(ep)->measure();
                              m->setEndBarLineType(e->subtype(), false, e->visible(), e->color());
                              }
                        }
                  }
            }
      selection()->deselectAll(this);
      }

//---------------------------------------------------------
//   cmdExchangeVoice
//---------------------------------------------------------

void Score::cmdExchangeVoice(int s, int d)
      {
      if (selection()->state() != SEL_STAFF) {
            selectStavesMessage();
            return;
            }
      int t1 = selection()->startSegment()->tick();
      int t2 = selection()->endSegment()->tick();

      Measure* m1 = tick2measure(t1);
      Measure* m2 = tick2measure(t2);
printf("exchange voice %d %d, tick %d-%d, measure %p-%p\n", s, d, t1, t2, m1, m2);
      for (;;) {
            undoExchangeVoice(m1, s, d, selection()->staffStart, selection()->staffEnd);
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

void Score::cmdEnterRest()
      {
      cmdEnterRest(_is.duration);
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void Score::cmdEnterRest(const Duration& d)
      {
printf("cmdEnterRest %s\n", qPrintable(d.name()));
      if (!noteEntryMode())
            setNoteEntry(true);
      if (_is.cr() == 0) {
            printf("cannot enter rest here\n");
            return;
            }

      int track = _is.track;
      Segment* seg = setNoteRest(_is.cr(), track, -1, d, 0, AUTO);
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr)
            nextInputPos(cr, false);
      _is.rest = false;  // continue with normal note entry
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
            Segment* seg = static_cast<Segment*>(cr->parent());
            if (seg->isEmpty())
                  undoRemoveElement(seg);
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
            Rest* rest = addRest(tuplet->tick(), tuplet->track(), tuplet->duration());
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
