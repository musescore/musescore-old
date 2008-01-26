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

//---------------------------------------------------------
//   selectNoteMessage
//---------------------------------------------------------

static void selectNoteMessage()
      {
      QMessageBox::critical(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("please select a single note and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
      }

static void selectNoteRestMessage()
      {
      QMessageBox::critical(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("please select a single note or rest and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
      }

static void selectNoteSlurMessage()
      {
      QMessageBox::critical(0,
               QMessageBox::tr("MuseScore:"),
               QMessageBox::tr("please select a single note or slur and retry operation\n"),
               QMessageBox::Ok, QMessageBox::NoButton);
      }

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
//   setRest
//---------------------------------------------------------

Rest* Score::setRest(int tick, int len, int track, Measure* measure)
      {
      Rest* rest = 0;
      if (len / (division*4)) {
            int nlen =  len % (division*4);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division*4);
            }
      else if (len / (division*2)) {
            int nlen =  len % (division*2);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division*2);
            }
      else if (len / division) {
            int nlen =  len % (division);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division);
            }
      else if (len / (division/2)) {
            int nlen = len % (division/2);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division/2);
            }
      else if (len / (division/4)) {
            int nlen = len % (division/4);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division/4);
            }
      else if (len / (division/8)) {
            int nlen = len % (division/8);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division/8);
            }
      else if (len / (division/16)) {
            int nlen = len % (division/16);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division/16);
            }
      else if (len / (division/32)) {
            int nlen = len % (division/32);
            if (nlen) {
                  setRest(tick, nlen, track, measure);
                  tick += nlen;
                  }
            rest = new Rest(this, tick, division/32);
            }
      if (rest) {
            rest->setVoice(track % VOICES);
            rest->setStaffIdx(track / VOICES);
            Segment::SegmentType st = Segment::segmentType(rest->type());
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            rest->setParent(seg);
            cmdAdd(rest);
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
            for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
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
      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
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

      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
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
            nsig->setStaffIdx(staffIdx);
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

      int octave = pitch / 12;
      int note   = pitch % 12;

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
                        Staff* staff = note->staff();
                        seq->startNote(staff->midiChannel(), note->pitch(), 60);
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
                  setRest(tick, track, len, _padState.dot);
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

Slur* Score::cmdAddSlur()
      {
      Element* e = sel->element();
      if (!e || e->type() != NOTE) {
            printf("no note selected\n");
            return 0;
            }
      Note* note   = (Note*)(e);
      Chord* chord = note->chord();
      int staffIdx = chord->staffIdx();
      int voice    = chord->voice();

      int track = staffIdx * VOICES + voice;
      int tick2 = nextSeg(chord->tick(), track);

      if (tick2 == 0) {
            printf("cannot create slur: at end\n");
            return 0;
            }
      Slur* slur = new Slur(this);
      slur->setStaffIdx(staffIdx);
      slur->setStart(chord->tick(), track);
      slur->setEnd(tick2, track);
      slur->setParent(_layout);
      cmdAdd(slur);
      return slur;
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
            printf("at end\n");
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
            printf("1: next note for tie not found\n");
            return;
            }

      Tie* tie = new Tie(this);
      tie->setStartNote(note);
      tie->setEndNote(note);
      tie->setStaffIdx(staffIdx);
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
      if (el->type() == NOTE)
            el = el->parent();
      int tick1 = el->tick();
      int tick2 = tick1 + 4 * division;
      Hairpin* gabel = new Hairpin(this);
      gabel->setTick(tick1);
      gabel->setTick2(tick2);
      gabel->setSubtype(decrescendo ? 1 : 0);

      if (!cmdAddHairpin(gabel, el->pos()))
            delete gabel;
      else
            select(gabel, 0, 0);
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
                  chord->setStemDirection(chord->isUp() ? DOWN : UP);
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
//---------------------------------------------------------

void Score::cmdAddBSymbol(BSymbol* s, const QPointF& pos, const QPointF& off)
      {
      s->setSelected(false);

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
            s->setStaffIdx(staffIdx);
            s->setParent(measure);
            }
      else if (s->anchor() == ANCHOR_PARENT) {
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
                        s->setUserOff((pos - page->pos() - off) / _spatium);
                        s->setStaffIdx(0);
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
            }
      else {
            printf("Anchor type not implemented\n");
            delete s;
            return;
            }

      undoAddElement(s);
      addRefresh(s->abbox());
      select(s, 0, 0);
      }

//---------------------------------------------------------
//   cmdAddHairpin
//---------------------------------------------------------

Element* Score::cmdAddHairpin(Hairpin* pin, const QPointF& pos)
      {
      int staffIdx = -1;
      int pitch, tick1;
      QPointF offset;
      Segment* segment;
      MeasureBase* measure = pos2measure(pos, &tick1, &staffIdx, &pitch, &segment, &offset);
      if (measure == 0 || measure->type() != MEASURE) {
            printf("addHairpin: cannot put object here\n");
            delete pin;
            return 0;
            }
      pin->setStaffIdx(staffIdx);
      pin->setParent(measure);
      cmdAdd(pin);
      return pin;
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
                  if (el->subtype() == TEXT_INSTRUMENT_SHORT) {
                        el->staff()->part()->setShortName(QString());
                        _layout->setInstrumentNames();
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
                        chord->remove((Note*) el);
                        undoOp(UndoOp::RemoveElement, el);
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
                        Rest* rest   = new Rest(this, chord->tick(), chord->tickLen());
                        rest->setStaffIdx(el->staffIdx());
                        rest->setParent(chord->parent());
                        rest->setVoice(el->voice());
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
                  if (el->voice() != 0)
                        undoRemoveElement(el);
                  break;

            case MEASURE:
                  undoFixTicks();
                  undoRemoveElement(el);
                  cmdRemoveTime(el->tick(), el->tickLen());
                  break;

            case ACCIDENTAL:
                  addAccidental((Note*)(el->parent()), ACC_NONE);
                  break;

            default:
                  printf("delete %s: not implemented\n", el->name());
            }
      }

//---------------------------------------------------------
//   cmdRemoveTime
//---------------------------------------------------------

void Score::cmdRemoveTime(int tick, int len)
      {
      int tick2 = tick + len;
      foreach(Element* el, _layout->_gel) {
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
                        Rest* rest    = new Rest(this, m->tick(), 0);
                        rest->setStaffIdx(staffIdx);
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
      Text* cn         = (Text*)editObject;
      Measure* measure = (Measure*)cn->parent();
      Segment* segment = measure->tick2segment(cn->tick());
      if (segment == 0) {
            printf("chordTab: no segment\n");
            return;
            }
      // int staff     = cn->staffIdx();

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

      Text* ocn = cn;
      cn        = 0;

      measure         = segment->measure();
      ElementList* el = measure->el();
      foreach(Element* e, *el) {
            if (e->type() == TEXT && e->subtype() == TEXT_CHORD
               && e->tick() == segment->tick()) {
                  cn = (Text*)e;
                  break;
                  }
            }

      if (!cn) {
            cn = new Text(this);
            cn->setSubtype(TEXT_CHORD);
            cn->setTick(segment->tick());
            cn->setStaffIdx(ocn->staffIdx());
            cn->setParent(measure);
            undoAddElement(cn);
            }

      select(cn, 0, 0);
      canvas()->startEdit(cn);
      ((Text*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsTab
//---------------------------------------------------------

void Score::lyricsTab(bool back)
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = (Segment*)(lyrics->parent());
      int staff        = lyrics->staffIdx();

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
      if (segment == 0)
            return;

      canvas()->setState(Canvas::NORMAL);
      Lyrics* oldLyrics = lyrics;
      switch(oldLyrics->syllabic()) {
            case Lyrics::SINGLE:
            case Lyrics::END:
                  break;
            case Lyrics::BEGIN:
                  oldLyrics->setSyllabic(Lyrics::SINGLE);
                  break;
            case Lyrics::MIDDLE:
                  oldLyrics->setSyllabic(Lyrics::END);
                  break;
            }
      endCmd();

      startCmd();
      LyricsList* ll = segment->lyricsList(staff);
      lyrics = ll->value(oldLyrics->no());
      if (!lyrics)
            lyrics = new Lyrics(this);
      else {
            lyrics->parent()->remove(lyrics);
            undoOp(UndoOp::RemoveElement, lyrics);
            }

      switch(lyrics->syllabic()) {
            case Lyrics::SINGLE:
            case Lyrics::BEGIN:
                  break;
            case Lyrics::END:
                  lyrics->setSyllabic(Lyrics::SINGLE);
                  break;
            case Lyrics::MIDDLE:
                  lyrics->setSyllabic(Lyrics::BEGIN);
                  break;
            }

      lyrics->setTick(segment->tick());
      lyrics->setStaffIdx(oldLyrics->staffIdx());
      lyrics->setParent(segment);
      lyrics->setNo(oldLyrics->no());
      undoAddElement(lyrics);

      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      ((Lyrics*)editObject)->moveCursorToEnd();

      layoutAll = true;
      }

//---------------------------------------------------------
//   lyricsMinus
//---------------------------------------------------------

void Score::lyricsMinus()
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = (Segment*)(lyrics->parent());
      int staff        = lyrics->staffIdx();
      int track        = staff * VOICES;

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      // search next chord
      while ((segment = segment->next1())) {
            Element* el = segment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }
      if (segment == 0) {
            return;
            }

      startCmd();

      Lyrics* oldLyrics = lyrics;

      LyricsList* ll = segment->lyricsList(staff);
      lyrics = ll->value(oldLyrics->no());
      if (!lyrics)
            lyrics = new Lyrics(this);

      switch(oldLyrics->syllabic()) {
            case Lyrics::SINGLE:
                  oldLyrics->setSyllabic(Lyrics::BEGIN);
                  break;
            case Lyrics::BEGIN:
            case Lyrics::MIDDLE:
                  break;
            case Lyrics::END:
                  oldLyrics->setSyllabic(Lyrics::MIDDLE);
                  break;
            }
      lyrics->setSyllabic(Lyrics::END);

      lyrics->setTick(segment->tick());
      lyrics->setStaffIdx(oldLyrics->staffIdx());
      lyrics->setParent(segment);
      lyrics->setNo(oldLyrics->no());
      undoAddElement(lyrics);

      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);
      ((Lyrics*)editObject)->moveCursorToEnd();

      setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsReturn
//---------------------------------------------------------

void Score::lyricsReturn()
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = (Segment*)(lyrics->parent());

      canvas()->setState(Canvas::NORMAL);
      endCmd();

      startCmd();

      Lyrics* oldLyrics = lyrics;

      lyrics = new Lyrics(this);
      lyrics->setTick(segment->tick());
      lyrics->setStaffIdx(oldLyrics->staffIdx());
      lyrics->setParent(segment);
      lyrics->setNo(oldLyrics->no() + 1);
      undoAddElement(lyrics);
      select(lyrics, 0, 0);
      canvas()->startEdit(lyrics);

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
      lyrics->setStaffIdx(chord->staffIdx());
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
      if (note == 0)
            return;
      Chord* chord = note->chord();

      int normalNotes=2, actualNotes=3;
      switch (n) {
            case 2:
                  // normalNotes = 3;
                  // actualNotes = 2;
                  printf("duole not implemented\n");
                  return;
            case 3:
                  normalNotes = 2;
                  actualNotes = 3;
                  break;
            case 5:
                  normalNotes = 4;
                  actualNotes = 5;
                  break;
            }
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

      int voice    = chord->voice();
      int staffIdx = chord->staffIdx();
      int tick     = chord->tick();
      int pitch    = note->pitch();

      Segment* segment = chord->segment();
      undoRemoveElement(chord);
      undoOp(UndoOp::RemoveElement, chord);
      if (segment->isEmpty())
            undoRemoveElement(segment);

      Tuplet* tuplet = new Tuplet(this);
      tuplet->setNormalNotes(normalNotes);
      tuplet->setActualNotes(actualNotes);
      tuplet->setBaseLen(baseLen);
      tuplet->setStaffIdx(staffIdx);
      Measure* measure = chord->measure();
      tuplet->setParent(measure);
      undoAddElement(tuplet);

      int ticks = baseLen * normalNotes / actualNotes;

      note = new Note(this);
      note->setPitch(pitch);
      note->setStaffIdx(staffIdx);

      chord = new Chord(this);
      chord->setTick(tick);
      chord->setTuplet(tuplet);
      chord->setVoice(voice);
      chord->setStaffIdx(staffIdx);
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
            rest->setVoice(voice);
            rest->setStaffIdx(staffIdx);
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

