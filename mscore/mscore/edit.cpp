//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: edit.cpp,v 1.85 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
      if (el && (el->type() == REST || el->type() == NOTE)) {
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
      rest->setTickLen(len);
      }

//---------------------------------------------------------
//   addRest
//    create one Rest at tick with len
//    create segment if necessary
//---------------------------------------------------------

Rest* Score::addRest(int tick, int len, int track)
      {
      Measure* measure = tick2measure(tick);
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
      Rest* rest = 0;
      if (len / (division*4)) {
            rest = addRest(tick, division*4, track);
            int nlen =  len % (division*4);
            if (nlen) {
                  setRest(tick + division*4, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division*2)) {
            rest = addRest(tick, division*2, track);
            int nlen =  len % (division*2);
            if (nlen) {
                  setRest(tick + division*2, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / division) {
            rest = addRest(tick, division, track);
            int nlen =  len % (division);
            if (nlen) {
                  setRest(tick + division, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division/2)) {
            rest = addRest(tick, division/2, track);
            int nlen = len % (division/2);
            if (nlen) {
                  setRest(tick + division/2, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division/4)) {
            rest = addRest(tick, division/4, track);
            int nlen = len % (division/4);
            if (nlen) {
                  setRest(tick + division/4, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division/8)) {
            rest = addRest(tick, division/8, track);
            int nlen = len % (division/8);
            if (nlen) {
                  setRest(tick + division/8, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division/16)) {
            rest = addRest(tick, division/16, track);
            int nlen = len % (division/16);
            if (nlen) {
                  setRest(tick + division/16, nlen, track);
                  tick += nlen;
                  }
            }
      else if (len / (division/32)) {
            rest = addRest(tick, division/32, track);
            int nlen = len % (division/32);
            if (nlen) {
                  setRest(tick + division/32, nlen, track);
                  tick += nlen;
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
                  TimeSig* ts = (TimeSig*)(segment->element(0));
                  if (ts == 0) {
                        printf("empty SegTimeSig\n");
                        if (debugMode)
                              abort();
                        return;
                        }
                  if ((etick > tick) && (ts->subtype() != timeSigSubtype))
                        break;
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        Element* e = segment->element(staffIdx * VOICES);
                        if (e)
                              undoRemoveElement(e);
                        else
                              printf("     +++no TimeSig in Staff\n");
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

      for (MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            int newLen = sigmap->ticksMeasure(m->tick());
            int oldLen = m->tickLen();
            if (newLen == oldLen)
                  continue;
            m->adjustToLen(oldLen, newLen);
            }
      if (nSig.valid())
            addTimeSig(tick, timeSigSubtype);
      fixTicks();
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
      int tick, line = -1;
      int staffIdx = -1;
      Segment* segment;
      Measure* m = pos2measure2(pos, &tick, &staffIdx, &line, &segment);
      if (m == 0)
            return;

      int key   = staff(staffIdx)->keymap()->key(tick);
      int clef  = staff(staffIdx)->clef()->clef(tick);
      int pitch = line2pitch(line, clef, key);
      int len   = _padState.tickLen;

      int voice = _padState.voice;
      int track = staffIdx * VOICES + voice;

      ChordRest* cr = (ChordRest*)segment->element(track);
      bool addToChord = false;
      if (!replace && cr && (cr->tickLen() == len) && (cr->type() == CHORD) && !_padState.rest) {
            const NoteList* nl = ((Chord*)cr)->noteList();
            Note* note = nl->find(pitch);
            if (note)
                  return;
            addToChord = true;
            }
      if (addToChord) {
            if (cr->tuplet())
                  len = cr->tuplet()->noteLen();

            if (cr->type() == CHORD) {
                  Note* note = addNote((Chord*)cr, pitch);
                  select(note, 0, 0);
                  if (seq && mscore->playEnabled()) {
                        seq->startNote(note->staff()->part(), note->pitch(), 60, 1000);
                        }
                  }
            else {
                  setNote(tick, track, pitch, len);
                  }
            }
      else {
            // replace chord
            if (cr && cr->tuplet())
                  len = cr->tuplet()->noteLen();
            if (_padState.rest)
                  setRest(tick, track, len, _padState.dots);
            else
                  setNote(tick, track, pitch, len);
            }
      _is.track       = staffIdx * VOICES + voice;

      _padState.pitch = pitch;
      _is.pos         = tick + len;
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
                  chord->add((NoteAttribute*)el);
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
      Element* e = sel->element();
      if (!e || e->type() != NOTE) {
            printf("no note selected\n");
            return;
            }
      ChordRest* cr1 = ((Note*)e)->chord();
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
//   addTie
//    shift+'S' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddTie()
      {
      Element* e = sel->element();
      if (!e || e->type() != NOTE) {
            printf("no note selected\n");
            return;
            }

      Note* note    = (Note*)(e);
      Chord* chord  = note->chord();
      int staffIdx  = chord->staffIdx();
      ChordRest* el = nextChordRest(chord);
      if (el == 0 || el->type() != CHORD) {
            if (debugMode)
                  printf("addTie: no next chord found\n");
            return;
            }
      NoteList* nl = ((Chord*)el)->noteList();
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
      tie->setEndNote(note);
      tie->setTrack(staffIdx * VOICES);
      note->setTieFor(tie);
      _layout->connectTies();
      layoutAll = true;
      select(tie, 0, 0);
      }

//---------------------------------------------------------
//   cmdAddHairpin
//    'H' typed on keyboard
//---------------------------------------------------------

void Score::cmdAddHairpin(bool decrescendo)
      {
      Element* el = sel->element();
      if (!el) {
            printf("selState != single\n");
            return;
            }

      if (el->type() != NOTE) {
            printf("please select note and try again\n");
            return;
            }
      el = el->parent();

      int tick1 = el->tick();
      int tick2 = tick1 + 4 * division;
      Hairpin* pin = new Hairpin(this);
      pin->setTick(tick1);
      pin->setTick2(tick2);
      pin->setSubtype(decrescendo ? 1 : 0);
      pin->setTrack(el->track());
      pin->setParent(_layout);
      pin->layout(layout());
#if 0
      LineSegment* ls = line->lineSegments().front();
      QPointF uo(pos - ls->canvasPos() - dragOffset);
      ls->setUserOff(uo / _spatium);
#endif
      cmdAdd(pin);
      select(pin, 0, 0);
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
      Element* el = sel->element();
      if (el && el->type() == NOTE) {
            Chord* chord = ((Note*)el)->chord();

            if (chord->stemDirection() == AUTO)
                  chord->setStemDirection(chord->up() ? DOWN : UP);
            else
                  chord->setStemDirection(chord->stemDirection() == UP ? DOWN : UP);
            Direction dir = chord->stemDirection();
            Beam* beam = chord->beam();
            if (beam) {
                  bool set = false;
                  QList<ChordRest*> elements = beam->getElements();
                  for (int i = 0; i < elements.size(); ++i) {
                        ChordRest* cr = elements[i];
                        if (!set) {
                              if (cr->type() == CHORD) {
                                    Chord* chord = (Chord*)cr;
                                    if (chord->stemDirection() != dir) {
                                          chord->setStemDirection(dir);
                                          undoOp(UndoOp::SetStemDirection, chord, int(dir));
                                          }
                                    set = true;
                                    }
                              }
                        else {
                              if (cr->type() == CHORD) {
                                    Chord* chord = (Chord*)cr;
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
      else if (el && el->type() == SLUR_SEGMENT) {
            SlurTie* slur = ((SlurSegment*) el)->slurTie();
            slur->setSlurDirection(slur->isUp() ? DOWN : UP);
            undoOp(UndoOp::FlipSlurDirection, slur);
            }
      else {
            selectNoteSlurMessage();
            return;
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
      select(s, 0, 0);
      }

//---------------------------------------------------------
//   deleteItem
//---------------------------------------------------------

void Score::deleteItem(Element* el)
      {
      switch(el->type()) {
            case TEXT:
                  if (el->subtype() == TEXT_INSTRUMENT_LONG) {
                        el->staff()->part()->setLongName(QString());
                        _layout->setInstrumentNames();
                        layoutAll = true;
                        break;
                        }
                  else if (el->subtype() == TEXT_INSTRUMENT_SHORT) {
                        el->staff()->part()->setShortName(QString());
                        _layout->setInstrumentNames();
                        break;
                        }
                  else if (el->subtype() == TEXT_COPYRIGHT) {
                        undoChangeCopyright(QString(""));
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
            case KEYSIG:
            case IMAGE:
            case TIE:
            case TEMPO_TEXT:
            case MARKER:
            case JUMP:
            case BREATH:
            case ARPEGGIO:
            case HARMONY:
            case TREMOLO:
            case STAFF_TEXT:
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
                  Chord* chord = (Chord*)(el->parent());
                  int notes = chord->noteList()->size();
                  if (notes > 1) {
                        undoRemoveElement(el);
                        break;
                        }
                  // else fall through
                  el = chord;
                  }

            case CHORD:
                  {
                  Chord* chord = (Chord*) el;
                  undoRemoveElement(chord);
                  if ((el->voice() == 0) && (chord->noteType() == NOTE_NORMAL)) {
                        //
                        // voice 0 chords are always replaced by rests
                        //
                        Rest* rest = new Rest(this, chord->tick(), chord->tickLen());
                        rest->setTrack(el->track());
                        rest->setParent(chord->parent());
                        undoAddElement(rest);
                        }
                  else {
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
                  Rest* rest = (Rest*)el;
                  if (rest->tuplet() && rest->tuplet()->elements()->empty())
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
                  addAccidental((Note*)(el->parent()), ACC_NONE);
                  break;

            case BAR_LINE:
                  {
                  BarLine* bl      = (BarLine*)el;
                  Segment* segment = bl->segment();
                  Measure* m       = segment->measure();
                  if (segment->subtype() == Segment::SegStartRepeatBarLine) {
                        undoChangeRepeatFlags(m, m->repeatFlags() & ~RepeatStart);
                        }
                  }
                  break;

            default:
                  printf("deleteItem: %s: not implemented\n", el->name());
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
            ClefList* cl = staff->clef();
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
            Measure* is = tick2measure(sel->tickStart);
            if (is->next()) {
                  Measure* ie = tick2measure(sel->tickEnd);
                  if (ie) {
                        do {
                              ie = (Measure*)(ie->prev());        // TODO: MeasureBase
                              deleteItem(ie);
                              } while (ie != is);
                        }
                  }
            else
                  deleteItem(is);
            }
      else if (sel->state() == SEL_STAFF) {
            //
            // see also measure->drop() REPEAT_MEASURE
            //
            int sstaff = sel->staffStart;
            int estaff = sel->staffEnd;
            select(0, 0, 0);
            Measure* is = tick2measure(sel->tickStart);
            Measure* ie = tick2measure(sel->tickEnd);
            if (is == ie)
                  ie = 0;
            for (MeasureBase* mb = is; mb && mb != ie; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  for (int staffIdx = sstaff; staffIdx < estaff; ++staffIdx) {
                        bool rmFlag = false;
                        for (Segment* s = m->first(); s; s = s->next()) {
                              if (s->subtype() == Segment::SegEndBarLine
                                 || s->subtype() == Segment::SegTimeSigAnnounce
                                 || s->subtype() == Segment::SegStartRepeatBarLine)
                                    continue;
                              if (s->subtype() == Segment::SegChordRest)
                                    rmFlag = true;
                              if (rmFlag) {
                                    int strack = staffIdx * VOICES;
                                    int etrack = strack + VOICES;
                                    for (int track = strack; track < etrack; ++track) {
                                          Element* el = s->element(track);
                                          if (el)
                                                undoRemoveElement(el);
                                          }
                                    }
                              if (s->isEmpty()) {
                                    undoRemoveElement(s);
                                    }
                              }
                        //
                        // add whole measure rest
                        //

                        Segment* seg  = m->findSegment(Segment::SegChordRest, m->tick());
                        if (seg == 0) {
                              seg = m->createSegment(Segment::SegChordRest, m->tick());
                              undoAddElement(seg);
                              }
                        Rest* rest = new Rest(this, m->tick(), 0);
                        rest->setTrack(staffIdx * VOICES);
                        rest->setParent(seg);
                        undoAddElement(rest);
                        foreach(Element* el, *m->el()) {
                              if (el->type() == SLUR && el->staffIdx() == staffIdx)
                                    undoRemoveElement(el);
                              }
                        }

                  }
            for (MeasureBase* m = is; m && m != ie; m = m->next()) {
                  for (int staffIdx = sstaff; staffIdx < estaff; ++staffIdx)
                        select(m, Qt::ShiftModifier, staffIdx);
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
      select(0, 0, 0);
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
printf("create Harmony tick %d\n", cn->tick());
            cn->setTrack(track);
            cn->setParent(measure);
            undoAddElement(cn);
            }

      select(cn, 0, 0);
      canvas()->startEdit(cn);
      ((Harmony*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   changeLineSegment
//    switch to first/last LineSegment while editing
//---------------------------------------------------------

void Score::changeLineSegment(bool last)
      {
      LineSegment* segment = (LineSegment*)editObject;

      LineSegment* newSegment;
      if (last)
            newSegment = segment->line()->lineSegments().back();
      else
            newSegment = segment->line()->lineSegments().front();

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      startCmd();
      canvas()->startEdit(newSegment);
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
      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void Score::cmdTuplet(int n)
      {
      Note* note = getSelectedNote();
      if (note == 0) {
            printf("cmdTuplet: no note selected\n");
            return;
            }
      Chord* chord = note->chord();

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

      int baseLen    = chord->tickLen() / normalNotes;
      Tuplet* tuplet = new Tuplet(this);
      tuplet->setNormalNotes(normalNotes);
      tuplet->setActualNotes(actualNotes);
      tuplet->setBaseLen(baseLen);
      tuplet->setTrack(chord->track());
      Measure* measure = chord->measure();
      tuplet->setParent(measure);

      cmdCreateTuplet(chord, tuplet);
      }

//---------------------------------------------------------
//   cmdCreateTuplet
//---------------------------------------------------------

void Score::cmdCreateTuplet(Chord* chord, Tuplet* tuplet)
      {
      int normalNotes  = tuplet->normalNotes();
      int actualNotes  = tuplet->actualNotes();
      int track        = chord->track();
      Measure* measure = chord->measure();

      int baseLen = chord->tickLen() / normalNotes;
      if (chord->tickLen() % normalNotes) {
            printf("cannot handle tuplet (rest %d)\n", chord->tickLen() % normalNotes);
            return;
            }

      //---------------------------------------------------
      //    - remove rest/note
      //    - replace with note + (actualNotes-1) rests
      //    - add 2 rests of 1/d2 duration as placeholder
      //---------------------------------------------------

//      int voice    = chord->voice();
      int tick     = chord->tick();
      int pitch    = getSelectedNote()->pitch();

      Segment* segment = chord->segment();
      undoRemoveElement(chord);
      if (segment->isEmpty())
            undoRemoveElement(segment);

      undoAddElement(tuplet);

      int ticks = baseLen * normalNotes / actualNotes;

      Note* note = new Note(this);
      note->setPitch(pitch);
      note->setTrack(track);

      chord = new Chord(this);
      chord->setTick(tick);
      chord->setTuplet(tuplet);
      chord->setTrack(track);
      chord->add(note);

      chord->setTickLen(ticks);
      Segment::SegmentType st = Segment::segmentType(chord->type());
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      chord->setParent(seg);
      undoAddElement(chord);

      for (int i = 0; i < (actualNotes-1); ++i) {
            tick += ticks;
            Rest* rest = new Rest(this);
            rest->setTick(tick);
            rest->setTuplet(tuplet);
            rest->setTrack(track);
            rest->setTickLen(ticks);
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
            _is.track = (_is.track / VOICES) * VOICES + voice;
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
                  undoOp(UndoOp::ChangeColor, e, color);
                  refresh |= e->abbox();
                  }
            }
      sel->deselectAll(this);
      }

