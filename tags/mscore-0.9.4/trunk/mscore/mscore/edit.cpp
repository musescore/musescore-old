//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: edit.cpp,v 1.85 2006/04/12 14:58:10 wschweer Exp $
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
#include "padstate.h"
#include "score.h"
#include "slur.h"
#include "hairpin.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "layout.h"
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

//---------------------------------------------------------
//   getSelectedNote
//---------------------------------------------------------

Note* Score::getSelectedNote()
      {
      Element* el = sel->element();
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
      Element* el = sel->element();
      if (el) {
            if (el->type() == NOTE)
                  return ((Note*)el)->chord();
            else if (el->type() == REST)
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
      Element* el = sel->element();
      if (sel->activeCR())
            el = sel->activeCR();
      if (el && (el->type() == REST || el->type() == NOTE || el->type() == CHORD)) {
            if (el->type() == NOTE)
                  el = el->parent();
            return el->tick();
            }
      return -1;
      }

//---------------------------------------------------------
//   changeRest
//---------------------------------------------------------

void Score::changeRest(Rest* rest, int /*tick*/, int len)
      {
      rest->setLen(len);
      }

//---------------------------------------------------------
//   addRest
//    create one Rest at tick with len
//    create segment if necessary
//---------------------------------------------------------

Rest* Score::addRest(int tick, int len, int track)
      {
      Measure* measure = tick2measure(tick);
      if (measure->tickLen() == len && (len < (division * 8)))    // whole measure rest?
            len = 0;
      Rest* rest = new Rest(this, tick, len);
      rest->setTrack(track);
      Segment::SegmentType st = Segment::segmentType(rest->type());
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      rest->setParent(seg);
      cmdAdd(rest);
      return rest;
      }

//---------------------------------------------------------
//   setRest
//    create one or more rests to fill "len" ticks
//---------------------------------------------------------

Rest* Score::setRest(int tick, int len, int track)
      {
// printf("setRest %d %d\n", tick, len);
      if (len == 0)
            return 0;
      Measure* measure = tick2measure(tick);

      //
      // set whole measure rest if
      //    - rest covers whole measure
      //    - len < brevis
      if ((measure->tickLen() == len) && (len < (division * 8)))
            return addRest(tick, len, track);

      QList<int> restList;
      while (len > 0) {
            Duration dt;
            for (int i = 0; i < Duration::types - 1; ++i) {
                  dt.setType(Duration::DurationType(i));
                  int ticks = dt.ticks();
                  if ((len - ticks) >= 0) {
                        restList.append(ticks);
                        len -= ticks;
                        break;
                        }
                  }
            }
      if (restList.isEmpty()) {
            printf("setRest at %d len %d failed\n", tick, len);
            return 0;
            }
      Rest* rest = 0;
      if (((measure->tick() - tick) % restList[0]) == 0) {
            foreach(int len, restList) {
                  rest = addRest(tick, len, track);
                  tick += len;
                  }
            }
      else {
            for (int i = restList.size() - 1; i >= 0; --i) {
                  int len = restList[i];
                  rest = addRest(tick, len, track);
                  tick += len;
                  }
            }
      return rest;
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

      // record old tickLens, since they will be modified when time is added/removed
      QVector<int> tickLens;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            tickLens.append(m->tickLen());
            }

      //---------------------------------------------
      // modify measures
      //---------------------------------------------

      int j = 0;
      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            int newLen = sigmap->ticksMeasure(m->tick());
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
      int len            = _padState.tickLen;
      bool divideSegment = len >= (division/2);
      Position p;
      if (!getPosition(&p, pos, divideSegment)) {
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
      int voice               = _padState.voice;
      int track               = staffIdx * VOICES + voice;
      int headGroup           = 0;
      Direction stemDirection = AUTO;

      if (instr->useDrumset) {
            Drumset* ds   = instr->drumset;
            pitch         = _padState.drumNote;
            if (pitch < 0)
                  return;
            voice         = ds->voice(pitch);
            headGroup     = ds->noteHead(pitch);
            stemDirection = ds->stemDirection(pitch);
            }

      Segment* segment = p.measure->tick2segment(tick);
      ChordRest* cr    = 0;
      if (segment)
            cr = static_cast<ChordRest*>(segment->element(track));

      bool addToChord = false;
      int tl = len;
      if (_is.cr->tuplet())
            tl = _is.cr->tickLen();
      if (!replace && cr && (cr->tickLen() == tl) && (cr->type() == CHORD) && !_padState.rest) {
            const NoteList* nl = static_cast<Chord*>(cr)->noteList();
            Note* note = nl->find(pitch);
            if (note)
                  return;
            addToChord = true;
            }
      if (addToChord) {
            if (cr->type() == CHORD) {
                  Note* note = addNote(static_cast<Chord*>(cr), pitch);
                  select(note, SELECT_SINGLE, 0);
                  }
            else {
                  if (cr->tuplet())
                        setTupletChordRest(cr, pitch, len);
                  else
                        setNote(tick, track, pitch, len, headGroup, stemDirection);
                  }
            }
      else {
            // replace chord
            if (cr && cr->tuplet())
                  setTupletChordRest(cr, pitch, len);
            else {
                  if (_padState.rest)
                        setRest(tick, track, len, _padState.dots);
                  else
                        setNote(tick, track, pitch, len, headGroup, stemDirection);
                  }
            }
      setInputTrack(staffIdx * VOICES + voice);
      _padState.pitch = pitch;
      _is.setPos(tick + len);
      emit posChanged(_is.pos());
      }

//---------------------------------------------------------
//   modifyElement
//---------------------------------------------------------

void Canvas::modifyElement(Element* el)
      {
      if (el == 0) {
            printf("modifyElement: el==0\n");
            delete el;
            return;
            }
      Score* cs = el->score();
      if (cs->sel->state() != SEL_SINGLE) {
            printf("modifyElement: cs->sel->state() != SEL_SINGLE\n");
            delete el;
            return;
            }
      Element* e = cs->sel->element();
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
            case ATTRIBUTE:
                  chord->add((Articulation*)el);
                  break;
            default:
                  printf("modifyElement: %s not ATTRIBUTE\n", el->name());
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
      ChordRest* cr1 = note->chord();
      ChordRest* cr2 = nextChordRest(cr1);

      if (cr2 == 0) {
            printf("cannot create slur: at end\n");
            return;
            }
      Slur* slur = new Slur(this);
      slur->setStartElement(cr1);
      slur->setEndElement(cr2);
      slur->setParent(_layout);
      cmdAdd(slur);

      slur->layout(layout());
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
//   cmdAddStaccato
//     ":" typed on keyboard
//---------------------------------------------------------

void Score::cmdAddStaccato()
      {
      Note* note = getSelectedNote();
      if (!note)
            return;
      printf("not impl.: cmdAddStaccato\n");

//      Note* note    = (Note*)(e);
//      Chord* chord  = note->chord();
//      int staffIdx  = chord->staffIdx();
      //Vi er n� klar over hvor vi er, og kan operere p� dette stedet:
      }

//---------------------------------------------------------
//   addTie
//    shift+'S' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      Note* note = getSelectedNote();
      if (!note || note->tieFor())
            return;
      Chord* chord  = note->chord();
      if (noteEntryMode()) {
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
      pin->setParent(_layout);
      pin->layout(layout());
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
//   cmdFlipStemDirection
//---------------------------------------------------------

void Score::cmdFlipStemDirection()
      {
      QList<Element*>* el = sel->elements();
      if (el->isEmpty()) {
            selectNoteSlurMessage();
            return;
            }
      foreach(Element* e, *el) {
            if (e->type() == NOTE) {
                  Chord* chord = static_cast<Note*>(e)->chord();

                  if (chord->stemDirection() == AUTO)
                        chord->setStemDirection(chord->up() ? DOWN : UP);
                  else
                        chord->setStemDirection(chord->stemDirection() == UP ? DOWN : UP);
                  Direction dir = chord->stemDirection();
                  Beam* beam = chord->beam();
                  if (beam) {
                        bool set = false;
                        QList<ChordRest*> elements = beam->elements();
                        for (int i = 0; i < elements.size(); ++i) {
                              ChordRest* cr = elements[i];
                              if (!set) {
                                    if (cr->type() == CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          if (chord->stemDirection() != dir) {
                                                chord->setStemDirection(dir);
                                                undoOp(UndoOp::SetStemDirection, chord, int(dir));
                                                }
                                          set = true;
                                          }
                                    }
                              else {
                                    if (cr->type() == CHORD) {
                                          Chord* chord = static_cast<Chord*>(cr);
                                          if (chord->stemDirection() != AUTO) {
                                                chord->setStemDirection(AUTO);
                                                undoOp(UndoOp::SetStemDirection, chord, int(AUTO));
                                                }
                                          }
                                    }
                              }

                        }
                  else {
                        undoOp(UndoOp::SetStemDirection, chord, int(dir));
                        }
                  }
            else if (e->type() == SLUR_SEGMENT) {
                  SlurTie* slur = static_cast<SlurSegment*>(e)->slurTie();
                  slur->setSlurDirection(slur->isUp() ? DOWN : UP);
                  undoOp(UndoOp::FlipSlurDirection, slur);
                  }
            else if (e->type() == BEAM) {
                  Beam* beam = static_cast<Beam*>(e);
                  beam->setBeamDirection(beam->isUp() ? DOWN : UP);
                  undoOp(UndoOp::FlipBeamDirection, beam);
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
            s->setUserOff(offset / _spatium);
            s->setTick(segment->tick());
            s->setTrack(staffIdx * VOICES);
            s->setParent(measure);
            }
      else if (s->anchor() == ANCHOR_PARENT) {
#endif
            bool foundPage = false;
            foreach (Page* page, _layout->pages()) {
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
                        s->setUserOff((pos - m->canvasPos() - off) / _spatium);
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
                        TextC* in = static_cast<TextC*>(el);
                        UndoOp i;
                        i.type = UndoOp::ChangeInstrumentLong;
                        i.part = in->staff()->part();
                        i.s    = in->getHtml();
                        undoList.back()->push_back(i);
                        el->staff()->part()->setLongName(QString());
                        _layout->setInstrumentNames();
                        break;
                        }
                  else if (el->subtype() == TEXT_INSTRUMENT_SHORT) {
                        TextC* in = static_cast<TextC*>(el);
                        UndoOp i;
                        i.type = UndoOp::ChangeInstrumentShort;
                        i.part = in->staff()->part();
                        i.s    = in->getHtml();
                        undoList.back()->push_back(i);
                        el->staff()->part()->setShortName(QString());
                        _layout->setInstrumentNames();
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
            case ATTRIBUTE:
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
                        Rest* rest = new Rest(this, chord->tick(), chord->tickLen());
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
                  undoFixTicks();
                  undoRemoveElement(el);
                  cmdRemoveTime(el->tick(), el->tickLen());
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
//   cmdDeleteSelection
//---------------------------------------------------------

void Score::cmdDeleteSelection()
      {
      if (sel->state() == SEL_SYSTEM) {
            MeasureBase* is = sel->startSegment()->measure();
            bool createEndBar = false;
            if (is->next()) {
                  MeasureBase* ie = sel->endSegment()->measure();
                  if (ie) {
                        if (ie->tick() < sel->endSegment()->tick()) {
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
                              lastMeasure->setEndBarLineType(END_BAR, false);
                              }
                        }
                  }
            }
      else if (sel->state() == SEL_STAFF) {
            Segment* s1 = sel->startSegment();
            Segment* s2 = sel->endSegment();
            int track1  = sel->staffStart * VOICES;
            int track2  = sel->staffEnd * VOICES;
            for (Segment* s = s1; s != s2; s = s->next1()) {
                  if (s->subtype() != Segment::SegChordRest)
                        continue;
                  for (int track = track1; track < track2; ++track) {
                        if (track % VOICES == 0)
                              continue;
                        if (s->element(track)) {
                              Element* e = s->element(track);
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              if (cr->tuplet()) {
                                    cmdDeleteTuplet(cr->tuplet(), false);
                                    continue;
                                    }
                              undoRemoveElement(s->element(track));
                              }
                        }
                  if (s->isEmpty())
                        undoRemoveElement(s);
                  }

            for (int staffIdx = sel->staffStart; staffIdx < sel->staffEnd; ++staffIdx) {
                  int tick   = s1->tick();
                  int gapLen;
                  if (s2)
                        gapLen = s2->tick() - tick;
                  else {
                        MeasureBase* m = measures()->last();
                        gapLen = m->tick() + m->tickLen() - tick;
                        }

                  while (gapLen) {
                        Measure* m = tick2measure(tick);
                        int maxGap = m->tick() + m->tickLen() - tick;
                        int len = gapLen > maxGap ? maxGap : gapLen;
                        setRest(tick, staffIdx * VOICES, len, false);
                        gapLen -= len;
                        tick   += len;
                        }
                  }

            layoutAll = true;
            return;
            }
      else {
            // deleteItem modifies sel->elements() list,
            // so we need a local copy:
            foreach(Element* e, *sel->elements()) {
                  e->setSelected(false);  // in case item is not deleted
                  deleteItem(e);
                  }
            }
      sel->elements()->clear();
      select(0, SELECT_SINGLE, 0);
      updateEntryMode();
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
      ChordRest* cr = getSelectedChordRest();
      if (cr == 0)
            return;

      int normalNotes=2, actualNotes=3;
      switch (n) {
            case 2:                       // duplet
                  normalNotes = 3;
                  actualNotes = 2;
                  break;
            case 3:                       // triplet
                  normalNotes = 2;
                  actualNotes = 3;
                  break;
            case 4:                       // quadruplet
                  normalNotes = 6;
                  actualNotes = 4;
                  break;
            case 5:                       // quintuplet
                  normalNotes = 4;
                  actualNotes = 5;
                  break;
            case 6:                       // sextuplet
                  normalNotes = 4;
                  actualNotes = 6;
                  break;
            case 7:                       // septuplet
                  normalNotes = 4;        // (sometimes 6)
                  actualNotes = 7;
                  break;
            case 8:                       // octuplet
                  normalNotes = 6;
                  actualNotes = 8;
                  break;
            case 9:                       // nonuplet
                  normalNotes = 8;        // (sometimes 6)
                  actualNotes = 9;
                  break;
            default:
                  printf("illegal tuplet %d\n", n);
                  return;
            }

      int baseLen    = cr->tickLen() / normalNotes;

      Tuplet* tuplet = new Tuplet(this);
      tuplet->setNormalNotes(normalNotes);
      tuplet->setActualNotes(actualNotes);
      tuplet->setBaseLen(baseLen);
      tuplet->setTrack(cr->track());
      tuplet->setTick(cr->tick());
      Measure* measure = cr->measure();
      tuplet->setParent(measure);
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
            }
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(ChordRest* cr, Tuplet* tuplet)
      {
      int normalNotes  = tuplet->normalNotes();
      int actualNotes  = tuplet->actualNotes();
      int track        = cr->track();
      Measure* measure = cr->measure();

      int len = cr->tickLen();
      if (cr->type() == REST && len == 0)
            len = cr->measure()->tickLen();

      int baseLen = len / normalNotes;
      if (len % normalNotes) {
            printf("cannot handle tuplet (rest %d)\n", len % normalNotes);
            return;
            }

      //---------------------------------------------------
      //    - remove rest/note
      //    - replace with note + (actualNotes-1) rests
      //    - add 2 rests of 1/d2 duration as placeholder
      //---------------------------------------------------

      int tick = cr->tick();
      Segment* segment = cr->segment();
      undoRemoveElement(cr);
      if (segment->isEmpty())
            undoRemoveElement(segment);

      undoAddElement(tuplet);

      int ticks = baseLen * normalNotes / actualNotes;

      if (cr->type() == CHORD) {
            cr = new Chord(this);
            cr->setTick(tick);
            cr->setTuplet(tuplet);
            cr->setTrack(track);
            Note* note = new Note(this);
            note->setPitch(getSelectedNote()->pitch());
            note->setTpc(getSelectedNote()->tpc());
            note->setTrack(track);
            cr->add(note);
            }
      else {
            cr = new Rest(this);
            cr->setTick(tick);
            cr->setTuplet(tuplet);
            cr->setTrack(track);
            }
      cr->setTickLen(ticks);
      Duration dt;
      dt.setVal(baseLen);
      cr->setDuration(dt);

      Segment::SegmentType st = Segment::segmentType(cr->type());
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      cr->setParent(seg);
      undoAddElement(cr);

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTick(tick);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setTickLen(ticks);
            Duration dt;
            dt.setVal(baseLen);
            rest->setDuration(dt);
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
      _padState.voice = voice;
      if (_is.track % VOICES != voice) {
            setInputTrack((_is.track / VOICES) * VOICES + voice);
            layoutAll = true;
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

      foreach(Element* e, *sel->elements()) {
            if (e->color() != c) {
                  QColor color = e->color();
                  e->setColor(c);
                  e->setGenerated(false);
                  undoOp(UndoOp::ChangeColor, e, color);
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
      sel->deselectAll(this);
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
//   setTupletChordRest
//    if pitch == -1 set rest
//---------------------------------------------------------

Element* Score::setTupletChordRest(ChordRest* cr, int pitch, int len)
      {
// printf("setTupletChordRest %d %d\n", pitch, len);

      Tuplet* tuplet = cr->tuplet();
      int bl         = tuplet->baseLen();

      // make sure len is baseLen * 2^n or 1/baseLen*2^n for n = 1-8
      if (len > bl) {
            int i = 2;
            for (i = 2; i < 256; i <<= 1) {
                  if (bl * i == len)
                        break;
                  }
            if (i >= 256) {
                  printf("setTuplet: chord/rest does not fit; len %d, baseLen %d\n", len, bl);
                  return 0;
                  }
            }
      if (len < bl) {
            int i = 2;
            for (i = 2; i < 256; i <<= 1) {
                  if (len * i == bl)
                        break;
                  }
            if (i >= 256) {
                  printf("setTuplet: chord/rest does not fit; len %d, baseLen %d\n", len, bl);
                  return 0;
                  }
            }

      //---------------------------------------------------
      //    make gap for new note/rest
      //---------------------------------------------------

      const QList<DurationElement*>& crl = tuplet->elements();
      int n = crl.size();
      int i = 0;
      for (; i < n; ++i) {
            if (crl[i] == cr)
                  break;
            }
      if (i == n) {
            printf("setTupletChordRest: cr not found in tuplet\n");
            return 0;
            }
      int remaining = len;
      int ii = i;
      for (; ii < n; ++ii) {
            remaining -= crl[ii]->duration().ticks();
            if (remaining <= 0)
                  break;
            }
      if (remaining > 0) {
            printf("setTupletChordRest: note/rest does not fit\n");
            return 0;
            }
      remaining = len;
      ii   = i;
      Measure* measure = cr->measure();
      setLayout(measure);

      for (; ii < n; ++ii) {
            DurationElement* el = crl[ii];
            undoRemoveElement(el);
            if (el->isChordRest())
                  measure->cmdRemoveEmptySegment(static_cast<Segment*>(el->parent()));
            --n;
            --ii;
            remaining -= el->duration().ticks();
            if (remaining <= 0)
                  break;
            }

      //---------------------------------------------------
      //    set new note/rest
      //---------------------------------------------------

      Duration dt;
      dt.setVal(len);

      int tick = cr->tick();
      int tl   = len * tuplet->normalNotes() / tuplet->actualNotes();

      Element* el = 0;
      if (pitch != -1) {
            Note* note = new Note(this);
            el = note;
            note->setPitch(pitch);
            note->setTrack(cr->track());
            mscore->play(note);
            Chord* chord = new Chord(this);
            chord->setTick(tick);
            chord->add(note);
            chord->setTickLen(tl);
            chord->setDuration(dt);
            chord->setTrack(cr->track());
            Segment* segment = measure->findSegment(Segment::SegChordRest, tick);
            if (segment == 0) {
                  segment = measure->createSegment(Segment::SegChordRest, tick);
                  undoAddElement(segment);
                  }
            chord->setParent(segment);
            undoAddElement(chord);
            tuplet->add(chord);
            chord->setTuplet(tuplet);
            select(note, SELECT_SINGLE, cr->track()); // sets _is.cr
            spell(note);
            }
      else {
            Rest* rest = new Rest(this);
            el = rest;
            rest->setTrack(cr->track());
            rest->setTick(tick);
            rest->setTickLen(tl);
            rest->setDuration(dt);
            Segment* segment = measure->findSegment(Segment::SegChordRest, tick);
            if (segment == 0) {
                  segment = measure->createSegment(Segment::SegChordRest, tick);
                  undoAddElement(segment);
                  }
            rest->setParent(segment);
            undoAddElement(rest);
            rest->setTuplet(tuplet);
            tuplet->add(rest);
            select(rest, SELECT_SINGLE, cr->track()); // sets _is.cr
            }

      //---------------------------------------------------
      //    fill gap with rest(s)
      //---------------------------------------------------

      if (remaining < 0) {
            remaining = -remaining;
            tick += tl;
            printf("fill gap at %d len %d\n", tick, remaining);
            while (remaining > 0) {
                  Duration dt;
                  dt.setVal(remaining);
                  int tl = dt.ticks() * tuplet->normalNotes() / tuplet->actualNotes();
                  Rest* rest = new Rest(this);
                  rest->setTrack(cr->track());
                  rest->setTick(tick);
                  rest->setTickLen(tl);
                  rest->setDuration(dt);
                  Segment* segment = measure->findSegment(Segment::SegChordRest, tick);
                  if (segment == 0) {
                        segment = measure->createSegment(Segment::SegChordRest, tick);
                        undoAddElement(segment);
                        }
                  rest->setParent(segment);
                  undoAddElement(rest);
                  rest->setTuplet(tuplet);
                  tuplet->add(rest);
                  remaining -= dt.ticks();
                  tick += tl;
                  }
            }
      return el;
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void Score::cmdEnterRest()
      {
      if (!noteEntryMode())
            setNoteEntry(true);
      if (_is.cr == 0) {
            printf("cannot enter rest here\n");
            return;
            }
      ChordRest* cr = _is.cr;
      if (cr->tuplet()) {
            setTupletChordRest(cr, -1, _padState.tickLen);
            }
      else {
            setRest(_is.pos(), _is.track, _padState.tickLen, _padState.dots);
            _is.setPos(_is.pos() + _padState.tickLen);
            emit posChanged(_is.pos());
            }
      _padState.rest = false;  // continue with normal note entry
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
      Measure* measure = tuplet->measure();
      foreach(DurationElement* de, tuplet->elements()) {
            if (de->type() == CHORD || de->type() == REST)
                  removeChordRest(static_cast<ChordRest*>(de), true);
            else if (de->type() == TUPLET)
                  cmdDeleteTuplet(static_cast<Tuplet*>(de), replaceWithRest);
            else
                  printf("cmdDeleteTuplet: unknown type %s\n", de->name());
            }
      undoRemoveElement(tuplet);
      int len  = tuplet->tickLen();
      if (!replaceWithRest)
            return;

      int tick = tuplet->tick();
      Rest* rest = new Rest(this, tick, len);
      rest->setTrack(tuplet->track());
      Segment::SegmentType st = Segment::SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      rest->setParent(seg);
      undoAddElement(rest);
      }

//---------------------------------------------------------
//   updateEntryMode
//    given the tick position and staffIdx,
//    select current ChordRest
//
//   called after cmdDeleteSelection
//---------------------------------------------------------

void Score::updateEntryMode()
      {
      _is.cr = 0;
      if (!noteEntryMode())
            return;
      if (_is.cr == 0) {
            Segment* segment = tick2segment(_is.pos());
            if (segment) {
                  Element* e = segment->element(_is.track);
                  if (e && e->isChordRest())
                        _is.cr = static_cast<ChordRest*>(e);
                  else
                        printf("no CR at %d track %d\n", _is.pos(), _is.track);
                  }
            else {
                  setNoteEntry(false);
                  }
            }
      if (_is.cr) {
            if (_is.cr->type() == REST)
                  select(_is.cr, SELECT_SINGLE, 0);
            else if (_is.cr->type() == CHORD)
                  select(static_cast<Chord*>(_is.cr)->downNote(), SELECT_SINGLE, 0);
            }
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void Score::setPos(int tick)
      {
      _is.setPos(tick);
      emit posChanged(tick);
      updateEntryMode();
      }

