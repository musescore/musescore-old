//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: cmd.cpp,v 1.74 2006/04/12 14:58:10 wschweer Exp $
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
#include "keysig.h"
#include "volta.h"
#include "dynamics.h"

//---------------------------------------------------------
//   start
//---------------------------------------------------------

/**
 Clear the area to be redrawn.
*/

void Score::start()
      {
      refresh.setRect(0.0,0.0,0.0,0.0);
      updateAll   = false;
      layoutStart = 0;          ///< start a relayout at this measure
      layoutAll   = false;      ///< do a complete relayout
      }

//---------------------------------------------------------
//   startCmd
//---------------------------------------------------------

/**
 Start a GUI command by clearing the redraw area
 and starting a user-visble undo.
*/

void Score::startCmd()
      {
      start();
      layoutAll = true;       // debug

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (cmdActive) {
            if (debugMode)
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
            if (debugMode)
                  fprintf(stderr, "endCmd: no cmd active\n");
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
      else if (layoutStart)
            reLayout(layoutStart);

      foreach(Viewer* v, viewer) {
//            if (noteEntryMode())
//                  v->moveCursor();
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
      updateAll = false;
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
      Staff* staff = 0;
      int pitch, tick;
      QPointF offset;
      Segment* segment;
      Measure* measure = pos2measure(pos, &tick, &staff, &pitch, &segment, &offset);
      if (measure == 0) {
            printf("cmdAdd: cannot put object here\n");
            delete e;
            return;
            }
      e->setStaff(staff);
      e->setParent(_layout);

      // calculate suitable endposition
      int tick2 = measure->last()->tick();
      Measure* m2 = measure;
      while (tick2 <= tick) {
            m2 = m2->next();
            if (m2 == 0)
                  break;
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
                  volta->layout(mainLayout());
                  LineSegment* ls = volta->lineSegments().front();
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo / _spatium);
                  }
                  break;
            case PEDAL:
            case OTTAVA:
            case TRILL:
            case HAIRPIN:
                  {
                  SLine* line = (SLine*)e;
                  line->setTick(tick);
                  line->setTick2(tick2);
                  line->layout(mainLayout());
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
                  int staffIdx = staff->idx();
                  QRectF sb(s->staff(staffIdx)->bbox());
                  sb.translate(s->pos() + s->page()->pos());
                  QPointF anchor(segment->abbox().x(), sb.topLeft().y());
                  dyn->layout(mainLayout());
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

                  undoOp(UndoOp::RemoveElement, clef);
                  removeElement(clef);
                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(clef->parent()));

                  oval = cl->clef(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeClef, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (Measure* m = measure; m; m = m->next()) {
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

                  undoOp(UndoOp::RemoveElement, ks);
                  removeElement(ks);
                  Measure* measure = tick2measure(tick);
                  measure->cmdRemoveEmptySegment((Segment*)(ks->parent()));

                  oval = kl->key(tick);
                  if (nki->second != oval)
                        break;

                  undoOp(UndoOp::ChangeKeySig, staff, nki->first, oval, -1000);

                  tick = nki->first;
                  for (Measure* m = measure; m; m = m->next()) {
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
      static int ptab[15][7] = {
//             c  d  e  f  g   a  b
            { -1, 1, 3, 4, 6,  8, 10 },     // Bes
            { -1, 1, 3, 5, 6,  8, 10 },     // Ges
            {  0, 1, 3, 5, 6,  8, 10 },     // Des
            {  0, 1, 3, 5, 7,  8, 10 },     // As
            {  0, 2, 3, 5, 7,  8, 10 },     // Es
            {  0, 2, 3, 5, 7,  9, 10 },     // B
            {  0, 2, 4, 5, 7,  9, 10 },     // F
            {  0, 2, 4, 5, 7,  9, 11 },     // C
            {  0, 2, 4, 6, 7,  9, 11 },     // G
            {  1, 2, 4, 6, 7,  9, 11 },     // D
            {  1, 2, 4, 6, 8,  9, 11 },     // A
            {  1, 3, 4, 6, 8,  9, 11 },     // E
            {  1, 3, 4, 6, 8, 10, 11 },     // H
            {  1, 3, 5, 6, 8, 10, 11 },     // Fis
            {  1, 3, 5, 6, 8, 10, 12 },     // Cis
            };

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
                  setNoteEntry(false, false);
            }
      if (!noteEntryMode()) {
            printf("cmdAddPitch: pos not set, or no chord rest found\n");
            return;
            }

      int key   = cr->staff()->keymap()->key(_is.pos) + 7;
      int pitch = ptab[key][note];

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
                  }
            }
      else {
            // insert note
            int len = _padState.tickLen;
            if (cr->tuplet())
                  len = cr->tuplet()->noteLen();
            setNote(_is.pos, _is.track, _padState.pitch, len);
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
      int pitch = on->pitch();
      switch(val) {
            case 1:   break;
            case 2:   pitch += 2;  break;
            case 3:   pitch += 4;  break;
            case 4:   pitch += 5;  break;
            case 5:   pitch += 7;  break;
            case 6:   pitch += 9;  break;
            case 7:   pitch += 11; break;
            case 8:   pitch += 12; break;
            case 9:   pitch += 14; break;
            case -2:  pitch -= 2;  break;
            case -3:  pitch -= 4;  break;
            case -4:  pitch -= 5;  break;
            case -5:  pitch -= 7;  break;
            case -6:  pitch -= 9;  break;
            case -7:  pitch -= 11; break;
            case -8:  pitch -= 12; break;
            case -9:  pitch -= 14; break;
            default:
                  printf("cmdAddInterval: unknown idx %d\n", val);
                  abort();
            }
      if (pitch > 127)
            pitch = 127;
      if (pitch < 0)
            pitch = 0;
      Note* n = addNote(on->chord(), pitch);
      select(n, 0, 0);
      _padState.pitch = n->pitch();
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
                  measure = appendMeasure();
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
                        segment->setElement(track, 0);
                        undoOp(UndoOp::RemoveElement, element);
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
            note->setStaff(staff(track / VOICES));

            if (seq && mscore->playEnabled()) {
                  seq->startNote(note->staff()->midiChannel(), note->pitch(), 64);
                  }

            if (tie) {
                  tie->setEndNote(note);
                  note->setTieBack(tie);
                  }
            Chord* chord = new Chord(this);
            chord->setTick(tick);
            chord->setVoice(track % VOICES);
            chord->setStaff(staff(track / VOICES));
            chord->add(note);
            chord->setTickLen(noteLen);
            chord->setStemDirection(preferences.stemDir[track % VOICES]);
            if (tuplet) {
                  chord->setTuplet(tuplet);
//                  tuplet->add(chord);
                  }
            Segment::SegmentType st = Segment::segmentType(chord->type());
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
                  setRest(tick, -len, track, measure);
            if (len <= 0)
                  break;
            //
            //  Note does not fit on current measure, create Tie to
            //  next part of note
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setStaff(note->staff());
            note->setTieFor(tie);
            }
      if (note && addTie) {
            tie = new Tie(this);
            tie->setStartNote(note);
            tie->setStaff(note->staff());
            note->setTieFor(tie);
            }
      _layout->connectTies();
      }

//---------------------------------------------------------
//   setRest
//---------------------------------------------------------

/**
 Set rest(\a len) at position \a tick / \a staff / \a voice.
*/

void Score::setRest(int tick, int track, int len)
      {
      int stick = tick;
      Measure* measure = tick2measure(stick);
      if (measure == 0) {
            printf("setRest:  ...measure not found\n");
            return;
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
                  return;
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
                  segment->setElement(track, 0);
                  undoOp(UndoOp::RemoveElement, element);
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

      Rest* rest = setRest(tick, len, track, measure);
      if (tuplet) {
            rest->setTuplet(tuplet);
//            tuplet->add(rest);
            }
      select(rest, 0, 0);
      if (noteLen - len > 0) {
            setRest(tick + len, noteLen - len, track, measure);
            }
      layoutAll = true;
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
      Page* page = _layout->pages()->front();
      QList<System*>* sl = page->systems();
      if (sl == 0 || sl->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      const QList<Measure*>& ml = sl->front()->measures();
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
                  {
                  Measure* measure = ml.front();
                  s = new Text(this);
                  s->setSubtype(subtype);
                  s->setAnchorMeasure(measure);
                  s->setParent(page);
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
            case TEXT_SYSTEM:
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
                  s->setStaff(el->staff());
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
      ElementList el;
      foreach(Element* e, *sel->elements()) {
            if (e->type() != NOTE)
                  continue;
            Note* note = (Note*)e;
            while (note->tieBack())
                  note = note->tieBack()->startNote();
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

            UndoOp i;
            i.type  = UndoOp::ChangePitch;
            i.obj   = oNote;
            i.val1  = oNote->pitch();
            undoList.back()->push_back(i);

            oNote->changePitch(newPitch);
            }
      _padState.pitch = newPitch;
      sel->updateState();     // accidentals may have changed
      layoutAll = true;
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
      appendMeasures(n);
      endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

Measure* Score::appendMeasure()
      {
      Measure* last = _layout->last();
      int tick = last ? last->tick() + last->tickLen() : 0;
      Measure* measure  = new Measure(this);
      measure->setTick(tick);

      for (int idx = 0; idx < nstaves(); ++idx) {
            Rest* rest = new Rest(this, tick, 0);
            Staff* staffp = staff(idx);
            rest->setStaff(staffp);
            Segment* s = measure->getSegment(rest);
            s->add(rest);
            }
      undoOp(UndoOp::InsertMeasure, measure);
      _layout->push_back(measure);
      layoutAll = true;
      return measure;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n)
      {
      if (noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      for (int i = 0; i < n; ++i)
            appendMeasure();
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
      insertMeasures(n, MEASURE_NORMAL);
      endCmd();
      }

//---------------------------------------------------------
//   insertMeasures
//  Changed by DK 05.08.07, loop for inserting "n"
//    Measures to be added
//    (loopcounter ino -- insert numbers)
//---------------------------------------------------------

void Score::insertMeasures(int n, int subtype)
      {
	if (sel->state() != SEL_STAFF && sel->state() != SEL_SYSTEM) {
		QMessageBox::warning(0, "MuseScore",
			tr("No Measure selected:\n"
			"please select a measure and try again"));
		return;
            }

	int tick   = sel->tickStart;
	int ticks  = sigmap->ticksMeasure(tick-1);

	for (int ino = 0; ino < n; ++ino) {
		Measure* m = new Measure(this);
            m->setSubtype(subtype);
		m->setTick(tick);
		m->setTickLen(ticks);
		for (int idx = 0; idx < nstaves(); ++idx) {
			Rest* rest    = new Rest(this, tick, 0);  // whole measure rest
			Staff* staffp = staff(idx);
			rest->setStaff(staffp);
			Segment* s = m->getSegment(rest);
			s->add(rest);
		      }
            addMeasure(m);
	      undoOp(UndoOp::InsertMeasure, m);
            undoInsertTime(tick, ticks);
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
      i.type  = UndoOp::ChangeAccidental;
      i.obj   = oNote;
      i.val1  = oNote->pitch();
      i.val2  = oNote->tpc();
      i.val3  = oNote->accidentalSubtype();
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
      for (Measure* m = _layout->first(); m; m = m->next())
            m->setUserStretch(1.0);
      setDirty();
      layoutAll = true;
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(Note* note)
      {
      int rstaff   = note->staff()->rstaff();

      if (note->move() == -1) {
            return;
            }
      if (rstaff + note->move() <= 0) {
            return;
            }

      note->setMove(note->move() - 1);
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

      if (note->move() == 1) {
            return;
            }
      if (rstaff + note->move() >= rstaves-1) {
            return;
            }
      note->setMove(note->move() + 1);
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
      for (Measure* im = _layout->first(); im; im = im->next()) {
            if (im->tick() < startTick)
                  continue;
            if (im->tick() >= endTick)
                  break;
            double stretch = im->userStretch();
            stretch += val;
            im->setUserStretch(stretch);
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
            endCmd();
            canvas()->setState(Canvas::NORMAL);  //calls endEdit()
            }
      if (cmd == "print")
            printFile();
      else if (cmd == "undo") {
            start();
            doUndo();
            setLayoutAll(true);
            end();
            }
      else if (cmd == "redo") {
            start();
            doRedo();
            setLayoutAll(true);
            end();
            }
      else if (cmd == "note-input") {
            start();
            setNoteEntry(true, false);
            _padState.rest = false;
            end();
            }
      else if (cmd == "pause")
            seq->pause();
      else if (cmd == "play")
            seq->start();
      else if (cmd == "repeat") {
            if (playRepeats == false)
                  playRepeats = true;
            else
                  playRepeats = false;
            }
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
      else {
            if (cmdActive) {
                  printf("Score::cmd(): cmd already active\n");
                  return;
                  }
            startCmd();
            if (cmd == "append-measure")
                  appendMeasures(1);
            else if (cmd == "insert-measure")
		      insertMeasures(1, MEASURE_NORMAL);
            else if (cmd == "insert-hbox")
		      insertMeasures(1, MEASURE_HBOX);
            else if (cmd == "insert-vbox")
		      insertMeasures(1, MEASURE_VBOX);
            else if (cmd == "page-prev")
                  pagePrev();
            else if (cmd == "page-next")
                  pageNext();
            else if (cmd == "page-top")
                  pageTop();
            else if (cmd == "page-end")
                  pageEnd();
            else if (cmd == "add-tie")
                  cmdAddTie();
            else if (cmd == "add-slur") {
                  Slur* slur = cmdAddSlur();
                  //
                  // start slur in edit mode
                  //
                  // slur->setSelected(true);
                  if (slur) {
                        slur->layout(mainLayout());
                        QList<SlurSegment*>* el = slur->elements();
                        if (!el->isEmpty()) {
                              SlurSegment* ss = el->front();
                              if (canvas()->startEdit(ss)) {
                                    return;
                                    }
                              }
                        }
                  }
            else if (cmd == "add-hairpin")
                  cmdAddHairpin(false);
            else if (cmd == "add-hairpin-reverse")
                  cmdAddHairpin(true);
            else if (cmd == "escape") {
                  if (noteEntryMode())
                        setNoteEntry(false, false);
                  select(0, 0, 0);
                  }
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
                        int len = _padState.tickLen;
                        setRest(_is.pos, _is.track, len);
                        _is.pos += len;
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
                  }
            else if (cmd == "move-down") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        moveDown((Note*)el);
                        }
                  }
            else if (cmd == "up-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = upAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    _padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "down-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = downAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    _padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "top-chord" ) {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = upAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE)
                                    _padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "bottom-chord") {
                  Element* el = sel->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = downAltCtrl((Note*)el);
                        if (e) {
                              if (e->type() == NOTE)
                                    _padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  }
            else if (cmd == "next-chord"
               || cmd == "prev-chord"
               || cmd == "next-measure"
               || cmd == "prev-measure")
                  move(cmd);

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
                  if (!noteEntryMode() && sel->state() == SEL_SINGLE) {
                        Element* el = sel->element();
                        if (el->type() == NOTE) {
                  		Tie* tie = new Tie(this);
                              tie->setParent(el);
            	      	cmdAdd(tie);
                              _layout->connectTies();
                              }
                        }
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
            else if (cmd == "quintuplet")
                  cmdTuplet(5);
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
                              printf("error reading paste data\n");
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
            else if (cmd == "chord-text")
                  return cmdAddText(TEXT_CHORD);
            else if (cmd == "rehearsalmark-text")
                  return cmdAddText(TEXT_REHEARSAL_MARK);
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

      Measure* measure;
      for (measure = _layout->first(); measure; measure = measure->next()) {
            if (measure->tick() == tickStart)
                  break;
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
            printf("error reading paste data\n");
            return;
            }
      docName = "--";
      pasteStaff(doc.documentElement(), measure, sel->staffStart);
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(QDomElement e, Measure* measure, int staffStart)
      {
      int srcStaffStart = -1;
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "Staff") {
                  Measure* m = measure;
                  int staffIdx = e.attribute("id", "0").toInt();
                  if (srcStaffStart == -1)
                        srcStaffStart = staffIdx;
                  staffIdx = staffStart - srcStaffStart + staffIdx;
                  for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                        if (ee.tagName() == "Measure") {
                              Measure* sm = new Measure(this);
                              sm->read(ee, staffIdx);
                              cmdReplaceElements(sm, m, staffIdx);
                              delete sm;
                              m = m->next();
                              if (m == 0)
                                    break;
                              }
                        else
                              domError(ee);
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   cmdReplaceElements
//---------------------------------------------------------

void Score::cmdReplaceElements(Measure* sm, Measure* dm, int staffIdx)
      {
      //
      // TODO: handle special cases: sm->tickLen() != ds->tickLen()
      //

      select(0, 0, 0);
      // clear staff in destination Measure
      for (Segment* s = dm->first(); s;) {
            int startTrack = staffIdx * VOICES;
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
      // add src elements to destination
      int srcTickOffset = sm->tick();
      int dstTickOffset = dm->tick();

      for (Segment* s = sm->first(); s; s = s->next()) {
            int startTrack = staffIdx * VOICES;
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
                  if (el->type() == CHORD)
                        el = ((Chord*)el)->upNote();
                  select(el, 0, 0);
                  adjustCanvasPosition(el);
                  if (noteEntryMode()) {
                        _is.pos = tick;
                        }
                  }
            }
      }
