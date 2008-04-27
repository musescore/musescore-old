//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: cmd.cpp,v 1.74 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

/**
 \file
 Handling of several GUI commands.
*/

#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "canvas.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "segment.h"
#include "text.h"
#include "sig.h"
#include "padstate.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "layout.h"
#include "preferences.h"
#include "page.h"
#include "barline.h"
#include "seq.h"
#include "mscore.h"
#include "tuplet.h"
#include "padids.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "textline.h"
#include "keysig.h"
#include "volta.h"
#include "dynamics.h"
#include "box.h"
#include "harmony.h"
#include "system.h"
#include "stafftext.h"


//---------------------------------------------------------
//   startCmd
//---------------------------------------------------------

/**
 Start a GUI command by clearing the redraw area
 and starting a user-visble undo.
*/

void Score::startCmd()
      {
      layoutAll = true;      ///< do a complete relayout

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (cmdActive) {
            // if (debugMode)
            fprintf(stderr, "Score::startCmd(): cmd already active\n");
            return;
            }
      undoList.push_back(new Undo(_is, sel));
      cmdActive = true;
      }

//---------------------------------------------------------
//   endCmd
//---------------------------------------------------------

/**
 End a GUI command by (if \a undo) ending a user-visble undo
 and (always) updating the redraw area.
*/

void Score::endCmd()
      {
      if (!cmdActive) {
            // if (debugMode)
                  fprintf(stderr, "Score::endCmd(): no cmd active\n");
            end();
            return;
            }
      if (undoList.back()->empty()) {
            // nothing to undo
            delete undoList.back();
            undoList.pop_back();
            }
      else {
            setDirty(true);
            for (iUndo i = redoList.begin(); i != redoList.end(); ++i)
                  delete *i;
            redoList.clear();
            getAction("undo")->setEnabled(true);
            getAction("redo")->setEnabled(false);
            }
      cmdActive = false;
      end();
      }

//---------------------------------------------------------
//   end
//---------------------------------------------------------

/**
 Update the redraw area.
*/
void Score::end()
      {
      if (layoutAll) {
            updateAll = true;
            _layout->layout();
            }
      else if (layoutStart) {
            updateAll = true;
            _layout->reLayout(layoutStart);
            }

      foreach(Viewer* v, viewer) {
            if (updateAll)
                  v->updateAll(this);
            else {
                  // update a little more:
                  int dx = lrint(v->matrix().m11() + .5);
                  int dy = lrint(v->matrix().m22() + .5);
                  QRectF r(refresh.adjusted(-dx, -dy, 2 * dx, 2 * dy));
                  v->dataChanged(r);
                  }
            }
      refresh    = QRectF();
      layoutAll  = false;
      updateAll  = false;
      layoutStart = 0;
      setPadState();
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd(Element* e)
      {
      e->setSelected(false);
      undoAddElement(e);
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd1(Element* e, const QPointF& pos, const QPointF& dragOffset)
      {
      e->setSelected(false);
      int staffIdx = -1;
      int pitch, tick;
      QPointF offset;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &tick, &staffIdx, &pitch, &segment, &offset);
      if (mb == 0 || mb->type() != MEASURE) {
            printf("cmdAdd: cannot put object here\n");
            delete e;
            return;
            }
      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;
      Measure* measure = (Measure*)mb;
      e->setTrack(track);
      e->setParent(_layout);

      // calculate suitable endposition
      int tick2 = measure->last()->tick();
      MeasureBase* m2 = measure;
      while (tick2 <= tick) {
            m2 = m2->next();
            if (m2 == 0)
                  break;
            if (m2->type() != MEASURE)
                  continue;
            tick2 = m2->tick();
            }

      switch(e->type()) {
            case VOLTA:
                  {
                  Volta* volta = (Volta*)e;
                  int tick;
                  Measure* m = pos2measure3(pos, &tick);
                  volta->setTick(m->tick());
                  volta->setTick2(m->tick() + m->tickLen());
                  volta->layout(layout());
                  const QList<LineSegment*> lsl = volta->lineSegments();
                  if (lsl.isEmpty()) {
                        delete e;
                        return;
                        }
                  else {
                        LineSegment* ls = lsl.front();
                        QPointF uo(pos - ls->canvasPos() - dragOffset);
                        ls->setUserOff(uo / _spatium);
                        }
                  }
                  break;
            case PEDAL:
            case OTTAVA:
            case TRILL:
            case HAIRPIN:
            case TEXTLINE:
                  {
                  SLine* line = (SLine*)e;
                  line->setTick(tick);
                  line->setTick2(tick2);
                  line->layout(layout());
                  LineSegment* ls = line->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case DYNAMIC:
                  {
                  Dynamic* dyn = (Dynamic*)e;
                  dyn->setTick(tick);
                  dyn->setParent(measure);

                  System* s    = measure->system();
                  QRectF sb(s->staff(staffIdx)->bbox());
                  sb.translate(s->pos() + s->page()->pos());
                  QPointF anchor(segment->abbox().x(), sb.topLeft().y());
                  dyn->layout(layout());
                  QPointF uo(pos - anchor - e->ipos() - dragOffset);
                  dyn->setUserOff(uo / _spatium);
                  }
                  break;
            default:
                  break;
            }
      cmdAdd(e);
      select(e, 0, 0);
      }

//---------------------------------------------------------
//   cmdRemove
//---------------------------------------------------------

void Score::cmdRemove(Element* e)
      {
      switch(e->type()) {
            case CLEF:
                  {
                  Clef* clef    = (Clef*)e;
                  Staff* staff  = clef->staff();
                  ClefList* cl  = staff->clef();
                  int tick      = clef->tick();
                  iClefEvent ki = cl->find(tick);
                  if (ki == cl->end()) {
                        printf("cmdRemove(KeySig): cannot find keysig at %d\n", tick);
                        return;
                        }
                  int oval = (*cl)[tick];
                  iClefEvent nki = ki;
                  ++nki;
                  cl->erase(ki);
                  undoOp(UndoOp::ChangeClef, staff, tick, oval, -1000);

                  undoRemoveElement(clef);

                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(clef->parent()));

                  oval = cl->clef(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeClef, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (MeasureBase* mb = measure; mb; mb = mb->next()) {
                        if (mb->type() != MEASURE)
                              continue;
                        Measure* m = (Measure*)mb;
                        bool found = false;
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->subtype() != Segment::SegClef)
                                    continue;
                              //
                              // we assume keySigs are only in first track (voice 0)
                              //
                              int track = staff->idx() * VOICES;
                              Clef* e = (Clef*)segment->element(track);
                              int etick = segment->tick();
                              if (!e || (etick != tick))
                                    continue;
                              if (etick > tick)
                                    break;
                              undoRemoveElement(e);
                              m->cmdRemoveEmptySegment(segment);
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
                  break;

            case KEYSIG:
                  {
                  KeySig* ks   = (KeySig*)e;
                  Staff* staff = ks->staff();
                  KeyList* kl  = staff->keymap();
                  int tick     = ks->tick();
                  iKeyEvent ki = kl->find(tick);
                  if (ki == kl->end()) {
                        printf("cmdRemove(KeySig): cannot find keysig at %d\n", tick);
                        return;
                        }
                  int oval = (*kl)[tick];
                  iKeyEvent nki = ki;
                  ++nki;
                  kl->erase(ki);
                  undoOp(UndoOp::ChangeKeySig, staff, tick, oval, -1000);

                  undoRemoveElement(ks);

                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(ks->parent()));

                  oval = kl->key(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeKeySig, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (MeasureBase* mb = measure; mb; mb = mb->next()) {
                        if (mb->type() != MEASURE)
                              continue;
                        Measure* m = (Measure*)mb;
                        bool found = false;
                        for (Segment* segment = m->first(); segment; segment = segment->next()) {
                              if (segment->subtype() != Segment::SegKeySig)
                                    continue;
                              //
                              // we assume keySigs are only in first track (voice 0)
                              //
                              int track = staff->idx() * VOICES;
                              KeySig* e = (KeySig*)segment->element(track);
                              int etick = segment->tick();
                              if (!e || (etick != tick))
                                    continue;
                              if (etick > tick)
                                    break;
                              undoRemoveElement(e);
                              m->cmdRemoveEmptySegment(segment);
                              found = true;
                              break;
                              }
                        if (found)
                              break;
                        }
                  }
                  break;

            case TEMPO_TEXT:
                  {
                  int tick = e->tick();
                  iTEvent i = tempomap->find(tick);
                  if (i != tempomap->end())
                        undoChangeTempo(tick, i->second, TEvent());
                  else
                        printf("remove tempotext: tempo event at %d not found\n", tick);
                  undoRemoveElement(e);
                  }
                  break;

            default:
                  {
                  Segment* seg = 0;
                  if (e->parent()->type() == SEGMENT)
                        seg = (Segment*) e->parent();
                  undoRemoveElement(e);
                  if (seg && seg->isEmpty())
                        undoRemoveElement(seg);
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   cmdAddPitch
//    c d e f g a b entered:
//       insert note or add note to chord
//---------------------------------------------------------

void Score::cmdAddPitch(int note, bool addFlag)
      {
      int octave    = _padState.pitch / 12;
      ChordRest* cr = 0;
      if (!noteEntryMode()) {
            //
            // start note entry mode at current selected
            // note/rest
            //
            cr = setNoteEntry(true, false);
            if (cr == 0) {
                  // cannot enter notes
                  // no note/rest selected?
                  return;
                  }
            }
      else {
            //
            // look for next note position
            //
            Measure* m = tick2measure(_is.pos);
            m->createVoice(_is.track);
            cr = (ChordRest*)searchNote(_is.pos, _is.track);
            if (!cr || !cr->isChordRest())
                  return;
            }
      if (!noteEntryMode())
            return;

      int key = 0;
      if (!preferences.alternateNoteEntryMethod)
            key = cr->staff()->keymap()->key(_is.pos);
      int pitch = pitchKeyAdjust(note, key);

      int delta = _padState.pitch - (octave*12 + pitch);
      if (delta > 6)
            _padState.pitch = (octave+1)*12 + pitch;
      else if (delta < -6)
            _padState.pitch = (octave-1)*12 + pitch;
      else
            _padState.pitch = octave*12 + pitch;
      if (_padState.pitch < 0)
            _padState.pitch = 0;
      if (_padState.pitch > 127)
            _padState.pitch = 127;

      if (addFlag) {
            // add note to chord
            Note* on = getSelectedNote();
            if (on) {
                  Note* n = addNote(on->chord(), _padState.pitch);
                  select(n, 0, 0);
                  setLayoutAll(false);
                  setLayoutStart(on->chord()->measure());
                  }
            }
      else {
            // insert note
            int len = _padState.tickLen;
            if (cr->tuplet()) {
                  len = cr->tuplet()->noteLen();
                  }
            setNote(_is.pos, _is.track, _padState.pitch, len);
            if (_is.slur) {
                  Element* e = searchNote(_is.pos, _is.track);
                  if (e) {
                        if (e->type() == NOTE)
                              e = e->parent();
                        if (_is.slur->startElement()->tick() == e->tick())
                              _is.slur->setStartElement(e);
                        else
                              _is.slur->setEndElement(e);
                        }
                  }
            else {
                  setLayoutAll(false);
                  setLayoutStart(cr->measure());
                  }
            _is.pos += len;
            }
      }

//---------------------------------------------------------
//   cmdAddInterval
//---------------------------------------------------------

void Score::cmdAddInterval(int val)
      {
      Note* on = getSelectedNote();
      if (on == 0)
            return;

      setNoteEntry(true, true);
      Staff* staff = on->staff();
      int key = staff->keymap()->key(on->chord()->tick());

      int kt[15] = {
            //  cb gb db ab  eb bb  f  c  g  d  a  e   b  f# c#
            // -7  -6 -5 -4 -3  -2 -1  0  1  2  3  4   5  6  7
               11,  6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1
            };

      int po = 12 - kt[key + 7];

      static int pt[12][16] = {
            //   2   3   4  5   6    7  OK   9   -2  -3  -4, -5, -6, -7,   OK   -9

      /*C */   { 2,  4,  5, 7,  9,  11, 12, 14,  -1, -3, -5, -7, -8, -10, -12, -13 },
      /*C#*/   { 1,  3,  4, 6,  8,  10, 12, 13,  -2, -4, -6, -8, -9, -11, -12, -14 },
      /*D */   { 2,  3,  5, 7,  9,  10, 12, 14,  -2, -3, -5, -7, -9, -10, -12, -14 },
      /*D#*/   { 1,  2,  4, 6,  8,   9, 12, 13,  -1, -3, -4, -6, -8, -10, -12, -13 },
      /*E */   { 1,  3,  5, 7,  8,  10, 12, 13,  -2, -4, -5, -7, -9, -11, -12, -14 },
      /*F */   { 2,  4,  6, 7,  9,  11, 12, 14,  -1, -3, -5, -6, -8, -10, -12, -13 },
      /*F#*/   { 1,  3,  5, 6,  8,  10, 12, 13,  -1, -2, -4, -6, -7,  -9, -12, -13 },
      /*G */   { 2,  4,  5, 7,  9,  10, 12, 14,  -2, -3, -5, -7, -8, -10, -12, -14 },
      /*G#*/   { 1,  3,  4, 6,  8,   9, 12, 13,  -1, -3, -4, -6, -8,  -9, -12, -13 },
      /*A */   { 2,  3,  5, 7,  8,  10, 12, 14,  -2, -4, -5, -7, -9, -10, -12, -14 },
      /*A#*/   { 1,  2,  4, 6,  7,   9, 11, 13,  -1, -3, -5, -6, -8, -10, -12, -13 },
      /*B */   { 1,  3,  5, 6,  8,  10, 12, 13,  -2, -4, -6, -7, -9, -11, -12, -14 },
            };

      static int it[19] = {
         // -9, -8, -7, -6, -5, -4, -3, -2, -1,  0,  1, 2, 3, 4, 5, 6, 7, 8, 9
            15, 14, 13, 12, 11, 10,  9,  8, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7
            };

      int pitch = on->pitch();
      int idx = it[val + 9];
      int interval = 0;
      if (idx != -1)
            interval = pt[(pitch+po) % 12][idx];

      pitch += interval;

      if (pitch > 127)
            pitch = 127;
      if (pitch < 0)
            pitch = 0;
      Note* n = addNote(on->chord(), pitch);
      select(n, 0, 0);
      _padState.pitch = n->pitch();
      }

//---------------------------------------------------------
//   setGraceNote
///   Create a grace note in front of a normal note.
///   \arg chord is the normal note
///   \arg pitch is the pitch of the grace note
///   \arg is the grace note type
///   \len is the visual duration of the grace note (1/16 or 1/32)
//---------------------------------------------------------

void Score::setGraceNote(Chord* chord, int pitch, NoteType type, int len)
      {
      Segment* seg     = chord->segment();
      Measure* measure = seg->measure();
      int tick         = chord->tick();
      int track        = chord->track();

      Segment::SegmentType st = Segment::SegGrace;
      seg = measure->createSegment(st, tick);

      undoAddElement(seg);

      Note* note = new Note(this);
      note->setPitch(pitch);
      note->setTrack(track);
      note->setTickLen(len);

      chord = new Chord(this);
      chord->setTick(tick);
      chord->setTrack(track);
      chord->add(note);

      chord->setTickLen(len);
      chord->setStemDirection(UP);
      chord->setNoteType(type);
      chord->setParent(seg);

      undoAddElement(chord);
      select(note, 0, 0);
      }

//---------------------------------------------------------
//   setNote
//---------------------------------------------------------

/**
 Set note (\a pitch, \a len) at position \a tick / \a staff / \a voice.
*/

void Score::setNote(int tick, int track, int pitch, int len)
      {
      bool addTie    = _padState.tie;
      Tie* tie       = 0;
      Note* note     = 0;
      Tuplet* tuplet = 0;

      while (len) {
            int stick = tick;
            Measure* measure = tick2measure(stick);
            if (measure == 0 || (stick >= (measure->tick() + measure->tickLen()))) {
                  measure = (Measure*)appendMeasure(MEASURE);
                  if (measure == 0)
                        return;
                  }
            Segment* segment = measure->first();
            int noteLen      = 0;
            while (segment) {
                  for (; segment; segment = segment->next()) {
                        if (segment->tick() >= stick)
                              break;
                        }
                  if (segment == 0 || segment->tick() != stick) {
                        Segment* newSegment = new Segment(measure, stick);
                        newSegment->setSubtype(Segment::SegChordRest);
                        measure->insert(newSegment, segment);
                        segment = newSegment;
                        }
                  Element* element = segment->element(track);
                  int l = 0;
                  if (element && element->isChordRest()) {
                        ChordRest* cr = (ChordRest*)element;
                        tuplet        = cr->tuplet();
                        l             = cr->tickLen();
                        if (l == 0 && element->type() == REST)    // whole measure rest?
                              l = measure->tickLen();
                        if (tuplet)
                              tuplet->remove(cr);
                        undoRemoveElement(element);
                        if (segment->isEmpty())
                              undoRemoveElement(segment);
                        }
                  segment = segment->next();
                  if (l == 0) {
                        if (segment == 0)
                              l = measure->tick() + measure->tickLen() - stick;
                        else
                              l = segment->tick() - stick;
                        }
                  len     -= l;
                  noteLen += l;
                  stick   += l;
                  if (len <= 0)
                        break;
                  }
            if (len < 0)
                  noteLen += len;

            note = new Note(this);
            note->setPitch(pitch);
            note->setTrack(track);

            if (seq && mscore->playEnabled())
                  seq->startNote(note->staff()->part(), note->pitch(), 64, 1000);

            if (tie) {
                  tie->setEndNote(note);
                  note->setTieBack(tie);
                  }
            Chord* chord = new Chord(this);
            chord->setTick(tick);
            chord->setTrack(track);
            chord->add(note);
            chord->setTickLen(noteLen);
            chord->setStemDirection(preferences.stemDir[track % VOICES]);
            if (tuplet)
                  chord->setTuplet(tuplet);
            Segment::SegmentType st = Segment::SegChordRest;
            Segment* seg = measure->findSegment(st, tick);
            if (seg == 0) {
                  seg = measure->createSegment(st, tick);
                  undoAddElement(seg);
                  }
            chord->setParent(seg);
            undoAddElement(chord);
            select(note, 0, 0);
            spell(note);

            tick += noteLen;

            if (len < 0)
                  setRest(tick, -len, track);
            if (len <= 0)
                  break;
            //
            //  Note does not fit on current measure, create Tie to
            //  next part of note
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setTrack(note->track());
            note->setTieFor(tie);
            }
      if (note && addTie) {
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setTrack(note->track());
            note->setTieFor(tie);
            }
      _layout->connectTies();
      }

//---------------------------------------------------------
//   setRest
//---------------------------------------------------------

/**
 Set rest(\a len) at position \a tick / \a track
 return false if rest could not be set
*/

bool Score::setRest(int tick, int track, int len, bool useDots)
      {
      int stick = tick;
      Measure* measure = tick2measure(stick);
      if (measure == 0 || (measure->tick() + measure->tickLen()) == tick) {
            printf("setRest:  ...measure not found\n");
            return false;
            }
      Segment* segment = measure->first();
      int noteLen      = 0;
      Tuplet* tuplet   = 0;
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
                  ChordRest* cr = (ChordRest*) element;
                  tuplet = cr->tuplet();
                  if (tuplet)
                        tuplet->remove(cr);
                  l = cr->tickLen();
                  if (l == 0)
                        l = measure->tickLen();
                  undoRemoveElement(element);
                  if (segment->isEmpty())
                        undoRemoveElement(segment);
                  }
            segment = segment->next();
            if (l == 0) {
                  if (segment == 0)
                        l = measure->tick() + measure->tickLen() - stick;
                  else
                        l = segment->tick() - stick;
                  }
            noteLen += l;
            stick   += l;
            if (noteLen >= len)     // collected enough time?
                  break;
            }
      if (noteLen < len)
            printf("setRest: cannot find segment! rest: %d\n", len - noteLen);

      Rest* rest;
      if (useDots) {
            rest = new Rest(this);
            rest->setTick(tick);
            rest->setTickLen(len);
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
            rest = setRest(tick, len, track);
            }
      if (tuplet)
            rest->setTuplet(tuplet);
      select(rest, 0, 0);
      if (noteLen - len > 0)
            setRest(tick + len, noteLen - len, track);
      layoutAll = true;
      return true;
      }

//---------------------------------------------------------
//   cmdAddChordName
//---------------------------------------------------------

void Score::cmdAddChordName()
      {
      if (editObject) {
            endEdit();
            endCmd();
            }
      Page* page = _layout->pages().front();
      const QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      const QList<MeasureBase*>& ml = sl->front()->measures();
      if (ml.empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      Element* el = sel->element();
      if (!el || (el->type() != NOTE && el->type() != REST)) {
            QMessageBox::information(0, "MuseScore: Text Entry",
               tr("No note or rest selected:\n"
               "please select a note or rest were you want to\n"
               "start text entry"));
            endCmd();
            return;
            }
      if (el->type() == NOTE)
            el = el->parent();
      Harmony* s = new Harmony(this);
      s->setTrack(el->track());
      s->setParent(((Chord*)el)->measure());
      s->setTick(el->tick());
      undoAddElement(s);
      layoutAll = true;
      select(s, 0, 0);
      canvas()->startEdit(s);
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void Score::cmdAddText(int subtype)
      {
      if (editObject) {
            endEdit();
            endCmd();
            }
      Page* page = _layout->pages().front();
      const QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      const QList<MeasureBase*>& ml = sl->front()->measures();
      if (ml.empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      Text* s = 0;
      switch(subtype) {
            case TEXT_TITLE:
            case TEXT_SUBTITLE:
            case TEXT_COMPOSER:
            case TEXT_POET:
            case TEXT_INSTRUMENT_EXCERPT:
                  {
                  MeasureBase* measure = ml.front();
                  if (measure->type() != VBOX) {
                        measure = new VBox(this);
                        measure->setTick(0);
                        addMeasure(measure);
	                  undoOp(UndoOp::InsertMeasure, measure);
                        }
                  s = new Text(this);
                  s->setSubtype(subtype);
                  s->setParent(measure);
                  }
                  break;
            case TEXT_COPYRIGHT:
                  {
                  s = new Text(this);
                  s->setParent(page);
                  s->setSubtype(subtype);
                  }
                  break;

            case TEXT_TRANSLATOR:
            case TEXT_MEASURE_NUMBER:
            case TEXT_PAGE_NUMBER_ODD:
            case TEXT_PAGE_NUMBER_EVEN:
            case TEXT_FINGERING:
            case TEXT_INSTRUMENT_LONG:
            case TEXT_INSTRUMENT_SHORT:
            case TEXT_LYRIC:
            case TEXT_TUPLET:
            default:
                  printf("add text type %d not supported\n", subtype);
                  break;

            case TEXT_CHORD:
            case TEXT_REHEARSAL_MARK:
                  {
                  Element* el = sel->element();
                  if (!el || (el->type() != NOTE && el->type() != REST)) {
                        QMessageBox::information(0, "MuseScore: Text Entry",
                        tr("No note or rest selected:\n"
                           "please select a note or rest were you want to\n"
                           "start text entry"));
                        break;
                        }
                  if (el->type() == NOTE)
                        el = el->parent();
                  s = new Text(this);
                  s->setTrack(subtype == TEXT_SYSTEM ? 0 : el->track());
                  s->setSubtype(subtype);
                  s->setParent(((ChordRest*)el)->measure());
                  s->setTick(el->tick());
                  }
                  break;
            case TEXT_STAFF:
            case TEXT_SYSTEM:
                  {
                  Element* el = sel->element();
                  if (!el || (el->type() != NOTE && el->type() != REST)) {
                        QMessageBox::information(0, "MuseScore: Text Entry",
                        tr("No note or rest selected:\n"
                           "please select a note or rest were you want to\n"
                           "start text entry"));
                        break;
                        }
                  if (el->type() == NOTE)
                        el = el->parent();
                  s = new StaffText(this);
                  if (subtype == TEXT_SYSTEM) {
                        s->setTrack(0);
                        s->setSystemFlag(true);
                        }
                  else {
                        s->setTrack(el->track());
                        s->setSystemFlag(false);
                        }
                  s->setSubtype(subtype);
                  s->setParent(((ChordRest*)el)->measure());
                  s->setTick(el->tick());
                  }
                  break;
            }

      if (s) {
            undoAddElement(s);
            layoutAll = true;
            select(s, 0, 0);
            canvas()->startEdit(s);
            }
      else
            endCmd();
      }

//---------------------------------------------------------
//   upDown
//---------------------------------------------------------

/**
 Increment/decrement pitch of note by one or by an octave.
*/

void Score::upDown(bool up, bool octave)
      {
      layoutAll = false;
      layoutStart = 0;        // DEBUG
      ElementList el;
      foreach(const Element* e, *sel->elements()) {
            if (e->type() != NOTE)
                  continue;
            Note* note = (Note*)e;
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            if (layoutStart == 0)
                  layoutStart = note->chord()->segment()->measure();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  iElement ii;
                  for (ii = el.begin(); ii != el.end(); ++ii) {
                        if (*ii == note)
                              break;
                        }
                  if (ii == el.end())
                        el.push_back(note);
                  }
            }
      if (el.empty())
            return;

      int newPitch = _padState.pitch;
      for (iElement i = el.begin(); i != el.end(); ++i) {
            Note* oNote = (Note*)(*i);
            int pitch   = oNote->pitch();
            if (octave)
                  newPitch = pitch + (up ? 12 : -12);
            else
                  newPitch = up ? pitch+1 : pitch-1;
            if (newPitch < 0)
                  newPitch = 0;
            if (newPitch > 127)
                  newPitch = 127;

            undoChangePitch(oNote, newPitch);

            // play new note with velocity 80 for 0.3 sec:
            if (seq && mscore->playEnabled())
                  seq->startNote(oNote->staff()->part(), newPitch, 80, 300);
            }
      _padState.pitch = newPitch;
      sel->updateState();     // accidentals may have changed
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//    - keyboard callback
//    - called from pulldown menu
//---------------------------------------------------------

/**
 Append \a n measures.

 Keyboard callback, called from pulldown menu.
*/

void Score::cmdAppendMeasures(int n)
      {
      startCmd();
      appendMeasures(n, MEASURE);
      endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* Score::appendMeasure(int type)
      {
      MeasureBase* last = _measures.last();
      int tick = last ? last->tick() + last->tickLen() : 0;
      MeasureBase* mb = 0;
      if (type == MEASURE)
            mb = new Measure(this);
      else if (type == HBOX)
            mb = new HBox(this);
      else if (type == VBOX)
            mb = new VBox(this);
      mb->setTick(tick);

      if (type == MEASURE) {
            Measure* measure = (Measure*)mb;
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Rest* rest = new Rest(this, tick, 0);
                  rest->setTrack(staffIdx * VOICES);
                  Segment* s = measure->getSegment(rest);
                  s->add(rest);
                  }
            }
      undoOp(UndoOp::InsertMeasure, mb);
      mb->setNext(0);
      _layout->add(mb);
      layoutAll = true;
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n, int type)
      {
      if (noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      for (int i = 0; i < n; ++i)
            appendMeasure(type);
      }

//---------------------------------------------------------
//   cmdInsertMeasures
//    - keyboard callback
//    - called from pulldown menu
// Added from cmdAppendMeasures by DK 06.08.07
//---------------------------------------------------------

void Score::cmdInsertMeasures(int n)
      {
      startCmd();
      insertMeasures(n, MEASURE);
      endCmd();
      }

//---------------------------------------------------------
//   insertMeasures
//  Changed by DK 05.08.07, loop for inserting "n"
//    Measures to be added
//    (loopcounter ino -- insert numbers)
//---------------------------------------------------------

void Score::insertMeasures(int n, int type)
      {
	if (sel->state() != SEL_STAFF && sel->state() != SEL_SYSTEM) {
		QMessageBox::warning(0, "MuseScore",
			tr("No Measure selected:\n"
			"please select a measure and try again"));
		return;
            }

	int tick  = sel->tickStart;
	int ticks = sigmap->ticksMeasure(tick);

	for (int ino = 0; ino < n; ++ino) {
		MeasureBase* m = 0;
            if (type == MEASURE)
                  m = new Measure(this);
            else if (type == HBOX)
                  m = new HBox(this);
            else if (type == VBOX)
                  m = new VBox(this);
		m->setTick(tick);
            if (type == MEASURE) {
      		m->setTickLen(ticks);
	      	for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
		      	Rest* rest    = new Rest(this, tick, 0);  // whole measure rest
	      		rest->setTrack(staffIdx * VOICES);
		      	Segment* s = ((Measure*)m)->getSegment(rest);
			      s->add(rest);
		            }
                  undoFixTicks();
		      }
            addMeasure(m);
	      undoOp(UndoOp::InsertMeasure, m);
            if (type == MEASURE) {
                  if (tick == 0) {
                        SigEvent e1 = sigmap->timesig(tick);
                        undoChangeSig(0, e1, SigEvent());
                        undoSigInsertTime(tick, ticks);
                        undoChangeSig(tick, SigEvent(), e1);
                        // TODO: move time signature
                        }
                  undoInsertTime(tick, ticks);
                  undoFixTicks();
                  }
            }
      select(0,0,0);
	layoutAll = true;
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

/**
 Add attribute \a attr to all selected notes/rests.

 Called from padToggle() to add note prefix/accent.
*/

void Score::addAttribute(int attr)
      {
      foreach(Element* el, *sel->elements()) {
            if (el->type() != NOTE && el->type() != REST)
                  continue;
            NoteAttribute* na = new NoteAttribute(this);
            na->setSubtype(attr);
            addAttribute(el, na);
            }
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to all selected notes.
*/

void Score::addAccidental(int idx)
      {
      foreach(Element* el, *sel->elements()) {
            if (el->type() == NOTE)
                  addAccidental((Note*)el, idx);
            }
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to note \a oNote.
*/

void Score::addAccidental(Note* oNote, int accidental)
      {
      UndoOp i;
      i.type     = UndoOp::ChangeAccidental;
      i.element1 = oNote;
      i.val1     = oNote->pitch();
      i.val2     = oNote->tpc();
      i.val3     = oNote->accidentalSubtype();
      undoList.back()->push_back(i);
      oNote->changeAccidental(accidental);
      layoutAll = true;
      }

//---------------------------------------------------------
//   addAttribute
//---------------------------------------------------------

void Score::addAttribute(Element* el, NoteAttribute* atr)
      {
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = ((Note*)el)->chord();
      else if (el->type() == REST)
            cr = (Rest*)el;
      else
            return;
      atr->setParent(cr);
      NoteAttribute* oa = cr->hasAttribute(atr);
      if (oa) {
            delete atr;
            atr = 0;
            cmdRemove(oa);
            }
      else
            cmdAdd(atr);
      }

//---------------------------------------------------------
//   toggleInvisible
//---------------------------------------------------------

void Score::toggleInvisible(Element* obj)
      {
      obj->setVisible(!obj->visible());
      undoOp(UndoOp::ToggleInvisible, obj);
      refresh |= obj->abbox();
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

/**
 Reset user offset for all selected notes.

 Called from pulldown menu.
*/

void Score::resetUserOffsets()
      {
      startCmd();
      QList<Element*> el = *sel->elements();
      for (iElement i = el.begin(); i != el.end(); ++i)
            (*i)->resetUserOffsets();
      layoutAll = true;
      endCmd();
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            if (m->type() == MEASURE)
                  ((Measure*)m)->setUserStretch(1.0);
            }
      setDirty();
      layoutAll = true;
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(Note* note)
      {
      int rstaff = note->staff()->rstaff();

      if (note->staffMove() == -1) {
            return;
            }
      if (rstaff + note->staffMove() <= 0) {
            return;
            }

      note->setStaffMove(note->staffMove() - 1);
      layoutAll = true;
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(Note* note)
      {
//      int staffIdx = staff(note->staff());
      Staff* staff = note->staff();
      Part* part   = staff->part();
      int rstaff   = staff->rstaff();
      int rstaves  = part->nstaves();

      if (note->staffMove() == 1) {
            return;
            }
      if (rstaff + note->staffMove() >= rstaves-1) {
            return;
            }
      note->setStaffMove(note->staffMove() + 1);
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddStretch
//    TODO: cmdAddStretch: undo
//---------------------------------------------------------

void Score::cmdAddStretch(double val)
      {
      if (sel->state() != SEL_SYSTEM && sel->state() != SEL_STAFF)
            return;
      int startTick = sel->tickStart;
      int endTick   = sel->tickEnd;
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            if (m->type() != MEASURE)
                  continue;
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            double stretch = ((Measure*)m)->userStretch();
            stretch += val;
            ((Measure*)m)->setUserStretch(stretch);
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdInsertClef
//---------------------------------------------------------

void Score::cmdInsertClef(int type)
      {
      if (!noteEntryMode())
            return;
      int tick = inputPos();
      staff(inputTrack()/VOICES)->changeClef(tick, type);
      }

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      if (sel->state() != SEL_SYSTEM && sel->state() != SEL_STAFF) {
            printf("no system or staff selected\n");
            return;
            }
      int startTick = sel->tickStart;
      int endTick   = sel->tickEnd;
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            if (m->type() != MEASURE)
                  continue;
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            Measure* measure = (Measure*)m;
            for (Segment* seg = measure->first(); seg; seg = seg->next()) {
                  if (seg->subtype() != Segment::SegChordRest)
                        continue;
                  for (int track = 0; track < nstaves() * VOICES; ++track) {
                        ChordRest* cr = (ChordRest*)seg->element(track);
                        if (cr == 0)
                              continue;
                        if (cr->type() == CHORD) {
                              if (cr->beamMode() != BEAM_AUTO)
                                    undoChangeBeamMode(cr, BEAM_AUTO);
                              }
                        else if (cr->type() == REST) {
                              if (cr->beamMode() != BEAM_NO)
                                    undoChangeBeamMode(cr, BEAM_NO);
                              }
                        }
                  }
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QString& cmd)
      {
      if (debugMode)
            printf("cmd <%s>\n", cmd.toLatin1().data());

      if (editObject) {                          // in edit mode?
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            endCmd();
            }
      if (cmd == "print")
            printFile();
      else if (cmd == "undo") {
            doUndo();
            setLayoutAll(true);
            end();
            }
      else if (cmd == "redo") {
            doRedo();
            setLayoutAll(true);
            end();
            }
      else if (cmd == "note-input") {
            setNoteEntry(true, false);
            _padState.rest = false;
            end();
            }
      else if (cmd == "escape") {
            if (noteEntryMode())
                  setNoteEntry(false, false);
            select(0, 0, 0);
            end();
            }
      else if (cmd == "pause")
            seq->pause();
      else if (cmd == "play")
            seq->start();
      else if (cmd == "repeat")
            preferences.playRepeats = !preferences.playRepeats;
      else if (cmd == "rewind")
            seq->rewindStart();
      else if (cmd == "play-next-measure")
            seq->nextMeasure();
      else if (cmd == "play-next-chord")
            seq->nextChord();
      else if (cmd == "play-prev-measure")
            seq->prevMeasure();
      else if (cmd == "play-prev-chord")
            seq->prevChord();
      else if (cmd == "seek-begin")
            seq->rewindStart();
      else if (cmd == "seek-end")
            seq->seekEnd();
      else if (cmd == "save-style")
            saveStyle();
      else {
            if (cmdActive) {
                  printf("Score::cmd(): cmd already active\n");
                  return;
                  }
            startCmd();
            if (cmd == "append-measure")
                  appendMeasures(1, MEASURE);
            else if (cmd == "insert-measure")
		      insertMeasures(1, MEASURE);
            else if (cmd == "insert-hbox")
		      insertMeasures(1, HBOX);
            else if (cmd == "insert-vbox")
		      insertMeasures(1, VBOX);
            else if (cmd == "append-hbox") {
		      MeasureBase* mb = appendMeasure(HBOX);
                  select(mb, 0, 0);
                  }
            else if (cmd == "append-vbox") {
		      MeasureBase* mb = appendMeasure(VBOX);
                  select(mb, 0, 0);
                  }
            else if (cmd == "page-prev") {
                  pagePrev();
                  setLayoutAll(false);
                  }
            else if (cmd == "page-next") {
                  pageNext();
                  setLayoutAll(false);
                  }
            else if (cmd == "page-top") {
                  pageTop();
                  setLayoutAll(false);
                  }
            else if (cmd == "page-end") {
                  pageEnd();
                  setLayoutAll(false);
                  }
            else if (cmd == "add-tie")
                  cmdAddTie();
            else if (cmd == "add-slur")
                  cmdAddSlur();
            else if (cmd == "add-hairpin")
                  cmdAddHairpin(false);
            else if (cmd == "add-hairpin-reverse")
                  cmdAddHairpin(true);
            else if (cmd == "delete")
                  cmdDeleteSelection();
            else if (cmd == "rest") {
                  if (!noteEntryMode()) {
                        setNoteEntry(true, true);
                        Element* el = sel->element();
                        if (el) {
                              if (el->type() == NOTE)
                                    el = el->parent();
                              if (el->isChordRest())
                                    _is.pos = ((ChordRest*)el)->tick();
                              }
                        }
                  if (noteEntryMode()) {
                        if (setRest(_is.pos, _is.track, _padState.tickLen, _padState.dots))
                              _is.pos += _padState.tickLen;
                        }
                  _padState.rest = false;  // continue with normal note entry
                  }
            else if (cmd == "pitch-up")
                  upDown(true, false);
            else if (cmd == "pitch-down")
                  upDown(false, false);
            else if (cmd == "pitch-up-octave")
                  upDown(true, true);
            else if (cmd == "pitch-down-octave")
                  upDown(false, true);
            else if (cmd == "move-up") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE)
                        moveUp((Note*)el);
                  setLayoutAll(false);
                  }
            else if (cmd == "move-down") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        moveDown((Note*)el);
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "up-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = upAlt(el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _padState.pitch = ((Note*)e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, 0, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "down-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = downAlt(el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _padState.pitch = ((Note*)e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, 0, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "top-chord" ) {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = upAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _padState.pitch = ((Note*)e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, 0, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "bottom-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = downAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _padState.pitch = ((Note*)e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, 0, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "next-chord"
               || cmd == "prev-chord"
               || cmd == "next-measure"
               || cmd == "prev-measure") {
                  move(cmd);
                  setLayoutAll(false);
                  }

            else if (cmd == "note-c")
                  cmdAddPitch(0, false);
            else if (cmd == "note-d")
                  cmdAddPitch(1, false);
            else if (cmd == "note-e")
                  cmdAddPitch(2, false);
            else if (cmd == "note-f")
                  cmdAddPitch(3, false);
            else if (cmd == "note-g")
                  cmdAddPitch(4, false);
            else if (cmd == "note-a")
                  cmdAddPitch(5, false);
            else if (cmd == "note-b")
                  cmdAddPitch(6, false);
            else if (cmd == "chord-c")
                  cmdAddPitch(0, true);
            else if (cmd == "chord-d")
                  cmdAddPitch(1, true);
            else if (cmd == "chord-e")
                  cmdAddPitch(2, true);
            else if (cmd == "chord-f")
                  cmdAddPitch(3, true);
            else if (cmd == "chord-g")
                  cmdAddPitch(4, true);
            else if (cmd == "chord-a")
                  cmdAddPitch(5, true);
            else if (cmd == "chord-b")
                  cmdAddPitch(6, true);
            else if (cmd == "pad-note-1")
                  padToggle(PAD_NOTE1);
            else if (cmd == "pad-note-2")
                  padToggle(PAD_NOTE2);
            else if (cmd == "pad-note-4")
                  padToggle(PAD_NOTE4);
            else if (cmd == "pad-note-8")
                  padToggle(PAD_NOTE8);
            else if (cmd == "pad-note-16")
                  padToggle(PAD_NOTE16);
            else if (cmd == "pad-note-32")
                  padToggle(PAD_NOTE32);
            else if (cmd == "pad-note-64")
                  padToggle(PAD_NOTE64);
            else if (cmd == "pad-rest")
                  padToggle(PAD_REST);
            else if (cmd == "pad-dot")
                  padToggle(PAD_DOT);
            else if (cmd == "pad-dotdot")
                  padToggle(PAD_DOTDOT);
            else if (cmd == "beam-start")
                  padToggle(PAD_BEAM_START);
            else if (cmd == "beam-mid")
                  padToggle(PAD_BEAM_MID);
            else if (cmd == "no-beam")
                  padToggle(PAD_BEAM_NO);
            else if (cmd == "beam-32")
                  padToggle(PAD_BEAM32);
            else if (cmd == "pad-tie") {
                  _padState.tie = !_padState.tie;
                  if (!noteEntryMode() && sel->state() == SEL_SINGLE)
                        cmdAddTie();
                  }
            else if (cmd == "pad-sharp2") {
                  _padState.prefix = _padState.prefix != 3 ? 3 : 0;
                  addAccidental(_padState.prefix);
                  }
            else if (cmd == "pad-sharp") {
                  _padState.prefix = _padState.prefix != 1 ? 1 : 0;
                  addAccidental(_padState.prefix);
                  }
            else if (cmd == "pad-nat") {
                  _padState.prefix = _padState.prefix != 5 ? 5 : 0;
                  addAccidental(_padState.prefix);
                  }
            else if (cmd == "pad-flat") {
                  _padState.prefix = _padState.prefix != 2 ? 2 : 0;
                  addAccidental(_padState.prefix);
                  }
            else if (cmd == "pad-flat2") {
                  _padState.prefix = _padState.prefix != 4 ? 4 : 0;
                  addAccidental(_padState.prefix);
                  }
            else if (cmd == "flip")
                  cmdFlipStemDirection();
            else if (cmd == "voice-1")
                  changeVoice(0);
            else if (cmd == "voice-2")
                  changeVoice(1);
            else if (cmd == "voice-3")
                  changeVoice(2);
            else if (cmd == "voice-4")
                  changeVoice(3);
            else if (cmd.startsWith("interval")) {
                  int n = cmd.mid(8).toInt();
                  cmdAddInterval(n);
                  }
            else if (cmd == "duplet")
                  cmdTuplet(2);
            else if (cmd == "triplet")
                  cmdTuplet(3);
            else if (cmd == "quadruplet")
                  cmdTuplet(4);
            else if (cmd == "quintuplet")
                  cmdTuplet(5);
            else if (cmd == "sextuplet")
                  cmdTuplet(6);
            else if (cmd == "septuplet")
                  cmdTuplet(7);
            else if (cmd == "octuplet")
                  cmdTuplet(8);
            else if (cmd == "nonuplet")
                  cmdTuplet(9);
            else if (cmd == "stretch+")
                  cmdAddStretch(0.1);
            else if (cmd == "stretch-")
                  cmdAddStretch(-0.1);
            else if (cmd == "cut") {
                  if (sel->state() == SEL_SINGLE) {
                        QMimeData* mimeData = new QMimeData;
                        Element* el = sel->element();
                        mimeData->setData(mimeSymbolFormat, el->mimeData(QPointF()));
                        QApplication::clipboard()->setMimeData(mimeData);
                        deleteItem(el);
                        }
                  }
            else if (cmd == "copy") {
                  QString mimeType = sel->mimeType();
                  if (!mimeType.isEmpty()) {
                        QMimeData* mimeData = new QMimeData;
                        mimeData->setData(mimeType, sel->mimeData());
                        QApplication::clipboard()->setMimeData(mimeData);
                        }
                  }
            else if (cmd == "paste") {
                  const QMimeData* ms = QApplication::clipboard()->mimeData();
                  if (sel->state() == SEL_SINGLE && ms && ms->hasFormat(mimeSymbolFormat)) {
                        QByteArray data(ms->data(mimeSymbolFormat));
                        QDomDocument doc;
                        int line, column;
                        QString err;
                        if (!doc.setContent(data, &err, &line, &column)) {
                              printf("error reading paste data at line %d column %d: %s\n",
                                 line, column, qPrintable(err));
                              printf("%s\n", data.data());
                              return;
                              }
                        docName = "--";
                        QDomElement e = doc.documentElement();
                        QPointF dragOffset;
                        int type    = Element::readType(e, &dragOffset);
                        Element* el = Element::create(type, this);
                        el->read(e);
                        addRefresh(sel->element()->abbox());   // layout() ?!
                        sel->element()->drop(QPointF(), QPointF(), el);
                        addRefresh(sel->element()->abbox());
                        }
                  else if (sel->state() == SEL_STAFF && ms && ms->hasFormat(mimeStaffListFormat))
                        pasteStaff(ms);
                  else if (sel->state() == SEL_SYSTEM && ms && ms->hasFormat(mimeMeasureListFormat)) {
                        printf("paste system\n");
                        }
                  else {
                        printf("paste not supported: sel state %d ms %p\n", sel->state(), ms);
                        if (ms) {
                              QStringList formats = ms->formats();
                              printf("Formate:\n");
                              foreach(QString s, formats)
                                    printf("format <%s>\n", s.toLatin1().data());
                              }
                        }
                  }
            else if (cmd == "lyrics")
                  return addLyrics();
            else if (cmd == "tempo")
                  addTempo();
            else if (cmd == "metronome")
                  addMetronome();
            else if (cmd == "pitch-spell")
                  spell();
            else if (cmd == "title-text")
                  return cmdAddText(TEXT_TITLE);
            else if (cmd == "subtitle-text")
                  return cmdAddText(TEXT_SUBTITLE);
            else if (cmd == "composer-text")
                  return cmdAddText(TEXT_COMPOSER);
            else if (cmd == "poet-text")
                  return cmdAddText(TEXT_POET);
            else if (cmd == "copyright-text")
                  return cmdAddText(TEXT_COPYRIGHT);
            else if (cmd == "system-text")
                  return cmdAddText(TEXT_SYSTEM);
            else if (cmd == "staff-text")
                  return cmdAddText(TEXT_STAFF);
            else if (cmd == "chord-text")
                  return cmdAddChordName();
            else if (cmd == "rehearsalmark-text")
                  return cmdAddText(TEXT_REHEARSAL_MARK);
            else if (cmd == "select-all") {
                  MeasureBase* mb = _measures.last();
                  if (mb) {   // check for empty score
                        sel->setState(SEL_SYSTEM);
                        sel->tickStart  = 0;
                        sel->tickEnd    = _measures.last()->tick() + _measures.last()->tickLen();
                        sel->staffStart = 0;
                        sel->staffEnd   = nstaves();
                        }
                  }
            else if (cmd == "transpose") {
                  transpose();
                  }
            else if (cmd == "reset-beammode")
                  cmdResetBeamMode();
            else if (cmd == "clef-violin")
                  cmdInsertClef(CLEF_G);
            else if (cmd == "clef-bass")
                  cmdInsertClef(CLEF_F);
            else if (cmd == "load-style") {
                  loadStyle();
                  setLayoutAll(true);
                  }
            else if (cmd == "tuplet-dialog")
                  tupletDialog();
            else
                  printf("unknown cmd <%s>\n", qPrintable(cmd));
            endCmd();
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(const QMimeData* ms)
      {
      if (sel->state() != SEL_STAFF) {
            printf("  cannot paste to selection\n");
            return;
            }
      int tickStart  = sel->tickStart;

      Measure* measure = 0;
      for (MeasureBase* e = _measures.first(); e; e = e->next()) {
            if (e->type() != MEASURE)
                  continue;
            if (e->tick() == tickStart) {
                  measure = (Measure*)e;
                  break;
                  }
            }
      if (measure->tick() != tickStart) {
            printf("  cannot find measure\n");
            return;
            }
      QByteArray data(ms->data(mimeStaffListFormat));
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading paste data at line %d column %d: %s\n",
               line, column, qPrintable(err));
            printf("%s\n", data.data());
            return;
            }
      docName = "--";
      pasteStaff(doc.documentElement(), measure, sel->staffStart);
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(QDomElement e, Measure* measure, int dstStaffStart)
      {
      int srcStaffStart = -1;
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "StaffList") {
                  domError(e);
                  continue;
                  }
            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "Staff") {
                        domError(ee);
                        continue;
                        }
                  Measure* m = measure;
                  int srcStaffIdx = ee.attribute("id", "0").toInt();
                  if (srcStaffStart == -1)
                        srcStaffStart = srcStaffIdx;
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        if (eee.tagName() != "Measure") {
                              domError(eee);
                              continue;
                              }
                        Measure* sm = new Measure(this);
                        sm->read(eee, srcStaffIdx);
                        if (dstStaffIdx < nstaves())
                              cmdReplaceElements(sm, m, srcStaffIdx, dstStaffIdx);
                        delete sm;
                        MeasureBase* mb = m;
                        do {
                              mb = mb->next();
                              } while (mb && mb->type() != MEASURE);
                        m = (Measure*)mb;
                        if (m == 0)
                              break;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdReplaceElements
//---------------------------------------------------------

void Score::cmdReplaceElements(Measure* sm, Measure* dm, int srcStaffIdx, int dstStaffIdx)
      {
      //
      // TODO: handle special cases: sm->tickLen() != ds->tickLen()
      //

      select(0, 0, 0);
      // clear staff in destination Measure
      for (Segment* s = dm->first(); s;) {
            int startTrack = dstStaffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (e)
                        undoRemoveElement(e);
                  }
            Segment* ns = s->next();
            dm->cmdRemoveEmptySegment(s);
            s = ns;
            }

      foreach(Tuplet* tuplet, *dm->tuplets()) {
            if (tuplet->staffIdx() == dstStaffIdx)
                  undoRemoveElement(tuplet);
            }

      int trackOffset   = (dstStaffIdx - srcStaffIdx) * VOICES;
      foreach(Tuplet* tuplet, *sm->tuplets()) {
            tuplet->setParent(dm);
            tuplet->setTrack(tuplet->track() + trackOffset);
            undoAddElement(tuplet);
            }

      // add src elements to destination
      int srcTickOffset = sm->tick();
      int dstTickOffset = dm->tick();

      for (Segment* s = sm->first(); s; s = s->next()) {
            int startTrack = srcStaffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            int tick       = s->tick() - srcTickOffset + dstTickOffset;
            Segment* ns    = dm->findSegment((Segment::SegmentType)s->subtype(), tick);
            if (ns == 0) {
                  ns = dm->createSegment((Segment::SegmentType)s->subtype(), tick);
                  undoAddElement(ns);
                  }
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (!e || !e->isChordRest())
                        continue;
                  e->setParent(ns);
                  e->setTick(tick);
                  e->setTrack(e->track() + trackOffset);
                  undoAddElement(e);
                  e->setSelected(false);
                  if (e->type() == REST)
                        select(e, Qt::ShiftModifier, 0);
                  else {
                        Chord* c = (Chord*)e;
                        NoteList* nl = c->noteList();
                        for (iNote in = nl->begin(); in != nl->end(); ++in) {
                              select(in->second, Qt::ShiftModifier, 0);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   move
//---------------------------------------------------------

void Score::move(const QString& cmd)
      {
      ChordRest* cr = sel->lastChordRest();
      if (cr) {
            Element* el = 0;
            if (cmd == "next-chord")
                  el = nextChordRest(cr);
            else if (cmd == "prev-chord")
                  el = prevChordRest(cr);
            else if (cmd == "next-measure")
                  el = nextMeasure(cr);
            else if (cmd == "prev-measure")
                  el = prevMeasure(cr);
            if (el) {
                  int tick = el->tick();
                  if (el->type() == CHORD) {
                        el = ((Chord*)el)->upNote();
                        mscore->play(el);
                        }
                  select(el, 0, 0);
                  adjustCanvasPosition(el);
                  if (noteEntryMode()) {
                        _is.pos = tick;
                        }
                  }
            }
      }
