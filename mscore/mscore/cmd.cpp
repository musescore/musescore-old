//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: cmd.cpp,v 1.74 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "textelement.h"
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

//---------------------------------------------------------
//   start
//---------------------------------------------------------

/**
 Clear the area to be redrawn.
*/

void Score::start()
      {
      refresh.setRect(0.0,0.0,0.0,0.0);
      updateAll = false;
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
//      printf("startCmd()\n");
      start();
      startUndo();
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd(Element* e)
      {
      undoOp(UndoOp::AddElement, e);
      layout();
      }

//---------------------------------------------------------
//   cmdRemove
//---------------------------------------------------------

void Score::cmdRemove(Element* e)
      {
      removeElement(e);
      undoOp(UndoOp::RemoveElement, e);
      layout();
      }

//---------------------------------------------------------
//   update
//---------------------------------------------------------

void Score::update(const QRectF& r)
      {
      for (std::list<Viewer*>::iterator i = viewer.begin(); i != viewer.end(); ++i)
            (*i)->dataChanged(this, r);
      }

//---------------------------------------------------------
//   end
//---------------------------------------------------------

/**
 Update the redraw area.
*/
void Score::end()
      {
      if (updateAll) {
            for (std::list<Viewer*>::iterator i = viewer.begin(); i != viewer.end(); ++i) {
                  (*i)->updateAll(this);
                  }
            updateAll = false;
            }
      else
            update(refresh);
      }

//---------------------------------------------------------
//   endCmd
//---------------------------------------------------------

/**
 End a GUI command by (if \a undo) ending a user-visble undo
 and (always) updating the redraw area.
*/

void Score::endCmd(bool undo)
      {
//      printf("endCmd() undo %d\n", undo);
      if (undo)
            endUndo();
      end();
      setPadState();
      }

//---------------------------------------------------------
//   cmdAddPitch
//---------------------------------------------------------

void Score::cmdAddPitch(int val)
      {
      //                        a   b   c   d   e   f   g
      static int offsets[] = {  9, 11,  0,  2,  4,  5,  7 };

      Note* on = getSelectedNote();
      if (on == 0)
            return;

      startCmd();
      setNoteEntry(true, true);
      int pitch  = on->pitch();
      int octave = pitch / 12;
      int ooff   = pitch % 12;
      int noff   = offsets[val];
      int diff   = noff - ooff;
      if (diff > 6)
            --octave;
      else if (diff < -6)
            ++octave;
      pitch  = octave * 12 + noff;
      if (pitch > 127)
            pitch = 127;
      if (pitch < 0)
            pitch = 0;
      Note* n    = addNote(on->chord(), pitch);
      select(n, 0, 0);
      padState.pitch = n->pitch();
      endCmd(true);
      }

//---------------------------------------------------------
//   cmdAddIntervall
//---------------------------------------------------------

void Score::cmdAddIntervall(int val)
      {
      Note* on = getSelectedNote();
      if (on == 0)
            return;

      startCmd();
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
            case 12:  pitch -= 2;  break;
            case 13:  pitch -= 4;  break;
            case 14:  pitch -= 5;  break;
            case 15:  pitch -= 7;  break;
            case 16:  pitch -= 9;  break;
            case 17:  pitch -= 11; break;
            case 18:  pitch -= 12; break;
            case 19:  pitch -= 14; break;
            default:
                  printf("cmdAddIntervall: unknown idx %d\n", val);
                  abort();
            }
      if (pitch > 127)
            pitch = 127;
      if (pitch < 0)
            pitch = 0;
      Note* n = addNote(on->chord(), pitch);
      select(n, 0, 0);
      padState.pitch = n->pitch();
      endCmd(true);
      }

//---------------------------------------------------------
//   setNote
//---------------------------------------------------------

/**
 Set note (\a pitch, \a len) at position \a tick / \a staff / \a voice.
*/

void Score::setNote(int tick, Staff* staff, int voice, int pitch, int len)
      {
      bool addTie = padState.tie;

      int staffIdx = _staves->idx(staff);
      int track = staffIdx * VOICES + voice;
      Tie* tie  = 0;
      Note* note = 0;
      Tuplet* tuplet = 0;
      while (len) {
            int stick = tick;
            Measure* measure = tick2measure(stick);
            if (measure == 0) {
                  printf("setNote:  ...measure not found\n");
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
                        newSegment->setSegmentType(Segment::SegChordRest);
                        measure->insert(newSegment, segment);
                        segment = newSegment;
                        }
                  Element* element = segment->element(track);
                  int l = 0;
                  if (element && element->isChordRest()) {
                        ChordRest* cr = (ChordRest*)element;
                        tuplet        = cr->tuplet();
                        l             = cr->tickLen();
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

            note = new Note(this, pitch, false);
            note->setStaff(staff);

            if (seq && mscore->playEnabled()) {
                  Staff* staff = note->staff();
                  seq->startNote(staff->midiChannel(), note->pitch(), 64);
                  }

            if (tie) {
                  tie->setEndNote(note);
                  note->setTieBack(tie);
                  }
            Chord* chord = new Chord(this, tick);
            chord->setVoice(voice);
            chord->setStaff(staff);
            chord->add(note);
            chord->setTickLen(noteLen);
            chord->setStemDirection(preferences.stemDir[voice]);
            if (tuplet) {
                  chord->setTuplet(tuplet);
                  tuplet->add(chord);
                  }
            chord->setParent(measure);
            cmdAdd(chord);
            measure->layoutNoteHeads(staffIdx);
            select(note, 0, 0);

            tick += noteLen;

            if (len < 0) {
                  if (voice == 0) {
                        // only for first voice: fill with rest
                        setRest(tick, -len, staff, voice, measure);
                        }
                  }
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
      connectTies();
      layout();
      }

//---------------------------------------------------------
//   setRest
//---------------------------------------------------------

/**
 Set rest(\a len) at position \a tick / \a staff / \a voice.
*/

void Score::setRest(int tick, Staff* st, int voice, int len)
      {
      int staffIdx = staff(st);
      int track = staffIdx * VOICES + voice;
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

      Rest* rest = setRest(tick, len, st, voice, measure);
      if (tuplet) {
            rest->setTuplet(tuplet);
            tuplet->add(rest);
            }
      select(rest, 0, 0);
      if (noteLen - len > 0) {
            setRest(tick + len, noteLen - len, st, voice, measure);
            }
      layout();
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void Score::cmdAddText(int style)
      {
      if (editObject) {
            endEdit();
            endCmd(true);
            }
      Page* page = pages()->front();
      SystemList* sl = page->systems();
      if (sl == 0 || sl->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      MeasureList* ml = sl->front()->measures();
      if (ml == 0 || ml->empty()) {
            printf("first create measure, then repeat operation\n");
            return;
            }
      Measure* measure = ml->front();

      Text* s = new Text(this, style);
      s->setText(textStyles[style].name);

      startCmd();
      s->setAnchor(measure);
      s->setParent(page);
      undoOp(UndoOp::AddElement, s);
      layout();

      select(s, 0, 0);
      canvas()->startEdit(s);
      }

//---------------------------------------------------------
//   cmdAddTitle
//---------------------------------------------------------

void Score::cmdAddTitle()
      {
      cmdAddText(TEXT_STYLE_TITLE);
      }

//---------------------------------------------------------
//   cmdAddSubtitle
//---------------------------------------------------------

void Score::cmdAddSubTitle()
      {
      cmdAddText(TEXT_STYLE_SUBTITLE);
      }

//---------------------------------------------------------
//   cmdAddComposer
//---------------------------------------------------------

void Score::cmdAddComposer()
      {
      cmdAddText(TEXT_STYLE_COMPOSER);
      }

//---------------------------------------------------------
//   cmdAddPoet
//---------------------------------------------------------

void Score::cmdAddPoet()
      {
      cmdAddText(TEXT_STYLE_POET);
      }

//---------------------------------------------------------
//   upDown
//---------------------------------------------------------

/**
 Increment/decrement pitch of note by one or by an octave.
*/

void Score::cmdUpDown(bool up, bool octave)
      {
      ElementList el;

      for (iElement i = sel->elements()->begin(); i != sel->elements()->end(); ++i) {
            Element* e = *i;
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

      int newPitch = padState.pitch;
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
      padState.pitch = newPitch;
      sel->updateState();     // accidentals may have changed
      layout();
      }

//---------------------------------------------------------
//   cmdAppendMeasure
//---------------------------------------------------------

/**
 Append one measure.

 Keyboard callback, called from pulldown menu.
*/

void Score::cmdAppendMeasure()
      {
      cmdAppendMeasures(1);
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
      if (noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      startCmd();
      for (int i = 0; i < n; ++i) {
            Measure* last = _layout->last();
            int tick = last ? last->tick() + last->tickLen() : 0;
            Measure* measure  = new Measure(this);
            measure->setTick(tick);

            for (int idx = 0; idx < nstaves(); ++idx) {
                  Rest* rest = new Rest(this, tick, sigmap->ticksMeasure(tick));
                  Staff* staffp = staff(idx);
                  rest->setStaff(staffp);
                  measure->add(rest);
                  BarLine* barLine = 0;
                  if (staffp->isTop()) {
                        barLine = new BarLine(this);
                        barLine->setStaff(staffp);
                        measure->add(barLine);
                        }
                  }
            undoOp(UndoOp::InsertMeasure, measure);
            _layout->push_back(measure);
            }
      layout();
      endCmd(true);
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
      //
      // we need a local copy of sel->elements()
      // because "addAttribute()" modifies sel->elements()
      //
      ElementList el(*sel->elements());

      for (iElement ie = el.begin(); ie != el.end(); ++ie) {
            Element* el = *ie;
            if (el->type() != NOTE && el->type() != REST)
                  continue;
            addAttribute(el, new NoteAttribute(this, attr));
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
      //
      // we need a local copy of sel->elements()
      // because "addAccidental()" modifies sel->elements()
      //
      ElementList el(*sel->elements());

      for (iElement ie = el.begin(); ie != el.end(); ++ie) {
            Element* el = *ie;
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
      i.val1  = oNote->userAccidental();
      undoList.back()->push_back(i);
      oNote->changeAccidental(accidental);
      layout();
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
      ElementList* el = sel->elements();
      for (iElement i = el->begin(); i != el->end(); ++i)
            (*i)->setUserOff(QPointF(0.0, 0.0));
      layout();
      endCmd(true);
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      for (Measure* m = _layout->first(); m; m = m->next())
            m->setUserStretch(1.0);
      setDirty();
      layout();
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

Element* Score::moveUp(Note* note)
      {
      int rstaff   = note->staff()->rstaff();

      if (note->move() == -1) {
            return 0;
            }
      if (rstaff + note->move() <= 0) {
            return 0;
            }

      note->setMove(note->move() - 1);
      note->chord()->segment()->measure()->layoutNoteHeads(staff(note->staff()));
      layout();
      return 0;
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

Element* Score::moveDown(Note* note)
      {
      int staffIdx = staff(note->staff());
      Staff* staff = note->staff();
      Part* part   = staff->part();
      int rstaff   = staff->rstaff();
      int rstaves  = part->nstaves();

      if (note->move() == 1) {
            return 0;
            }
      if (rstaff + note->move() >= rstaves-1) {
            return 0;
            }
      note->setMove(note->move() + 1);
      note->chord()->segment()->measure()->layoutNoteHeads(staffIdx);
      layout();
      return 0;
      }

