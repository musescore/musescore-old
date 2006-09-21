//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: keyb.cpp,v 1.55 2006/04/12 14:58:10 wschweer Exp $
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

#include "canvas.h"
#include "note.h"
#include "pad.h"
#include "padids.h"
#include "sym.h"
#include "padstate.h"
#include "note.h"
#include "score.h"
#include "rest.h"
#include "chord.h"
#include "select.h"
#include "input.h"
#include "key.h"
#include "measure.h"
#include "mscore.h"
#include "layout.h"
#include "slur.h"
#include "tuplet.h"

//---------------------------------------------------------
//   Canvas::keyPressEvent
//---------------------------------------------------------

void Canvas::keyPressEvent(QKeyEvent* ev)
      {
// printf("key key:%x state:%x text:<%s>\n", ev->key(),
// int(ev->modifiers()), ev->text().toLatin1().data());
      if (state == EDIT) {
            if ((ev->key() == Qt::Key_Tab) && (_score->editObject->type() == LYRICS))
                  _score->lyricsTab();
            else if (_score->edit(ev))
                  state = NORMAL;
            }
      else
            _score->keyPressEvent(ev);
      }

//---------------------------------------------------------
//   Canvas::keyPressEvent
//---------------------------------------------------------

void Score::keyPressEvent(QKeyEvent* ev)
      {
      keyState = ev->modifiers();
      if (keyState == 0) {
            switch(ev->key()) {
                  case Qt::Key_I:
                        mscore->editInstrList();
                        break;
                  case Qt::Key_Y:
                        mscore->clefMenu();
                        break;
                  case Qt::Key_K:
                        mscore->keyMenu();
                        break;
                  case Qt::Key_Z:
                        mscore->symbolMenu1();
                        break;
                  case Qt::Key_T:
                        mscore->timeMenu();
                        break;
                  case Qt::Key_L:
                        mscore->dynamicsMenu();
                        break;
                  case Qt::Key_N:
                        startNoteEntry();
                        break;
                  case Qt::Key_1:
                        cmdAddIntervall(1);
                        break;
                  case Qt::Key_2:
                        cmdAddIntervall(2);
                        break;
                  case Qt::Key_3:
                        cmdAddIntervall(3);
                        break;
                  case Qt::Key_4:
                        cmdAddIntervall(4);
                        break;
                  case Qt::Key_5:
                        cmdAddIntervall(5);
                        break;
                  case Qt::Key_6:
                        cmdAddIntervall(6);
                        break;
                  case Qt::Key_7:
                        cmdAddIntervall(7);
                        break;
                  case Qt::Key_8:
                        cmdAddIntervall(8);
                        break;
                  case Qt::Key_9:
                        cmdAddIntervall(9);
                        break;
                  case Qt::Key_Clear:
                        mscore->keyPadToggled(PAD_5);
                        break;
                  case Qt::Key_Plus:
                        mscore->keyPadToggled(PAD_PLUS);
                        break;
                  case Qt::Key_Minus:
                        mscore->keyPadToggled(PAD_MINUS);
                        break;
                  case Qt::Key_Asterisk:
                        // Mult
                        mscore->keyPadToggled(PAD_MULT);
                        break;
                  case '/':
                        mscore->keyPadToggled(PAD_DIV);
                        break;
                  }
            }

      if (keyState & Qt::KeypadModifier) {
            switch(ev->key()) {
                  case Qt::Key_Home:
                  case Qt::Key_7:
                        mscore->keyPadToggled(PAD_7);
                        break;
                  case Qt::Key_End:
                  case Qt::Key_1:
                        mscore->keyPadToggled(PAD_1);
                        break;
                  case Qt::Key_PageUp:
                  case Qt::Key_9:
                        mscore->keyPadToggled(PAD_9);
                        break;
                  case Qt::Key_PageDown:
                  case Qt::Key_3:
                        mscore->keyPadToggled(PAD_3);
                        break;
                  case Qt::Key_5:
                        mscore->keyPadToggled(PAD_5);
                        break;
                  case Qt::Key_Up:
                  case Qt::Key_8:
                        mscore->keyPadToggled(PAD_8);
                        break;
                  case Qt::Key_Down:
                  case Qt::Key_2:
                        mscore->keyPadToggled(PAD_2);
                        break;
                  case Qt::Key_Right:
                  case Qt::Key_6:
                        mscore->keyPadToggled(PAD_6);
                        break;
                  case Qt::Key_Left:
                  case Qt::Key_4:
                        mscore->keyPadToggled(PAD_4);
                        break;
                  case Qt::Key_Insert:
                  case Qt::Key_0:
                        mscore->keyPadToggled(PAD_0);
                        break;
                  case Qt::Key_Delete:
                  case ',':
                        mscore->keyPadToggled(PAD_COMMA);
                        break;
                  case Qt::Key_NumLock:
                        mscore->keyPadToggled(PAD_NUM);
                        break;
                  case Qt::Key_Enter:
                        mscore->keyPadToggled(PAD_ENTER);
                        break;
                  case Qt::Key_Clear:
                        mscore->keyPadToggled(PAD_5);
                        break;
                  case Qt::Key_Plus:
                        mscore->keyPadToggled(PAD_PLUS);
                        break;
                  case Qt::Key_Minus:
                        mscore->keyPadToggled(PAD_MINUS);
                        break;
                  case Qt::Key_Asterisk:
                        // Mult
                        mscore->keyPadToggled(PAD_MULT);
                        break;
                  case '/':
                        mscore->keyPadToggled(PAD_DIV);
                        break;
                  default:
                        ev->ignore();
                        printf("unknown key pad key 0x%02x\n", ev->key());
                        break;
                  }
            return;
            }

      startCmd();
      ChordRest* cr;
      Element* el = sel->element(); // single selection
      int note    = -1;

      switch (ev->key()) {
            case Qt::Key_Plus:
                  if (sel->state == SEL_SYSTEM || sel->state == SEL_STAFF) {
                        int startTick = sel->tickStart;
                        int endTick = sel->tickEnd;
                        for (Measure* im = _layout->first(); im; im = im->next()) {
                              if (im->tick() < startTick)
                                    continue;
                              if (im->tick() >= endTick)
                                    break;
                              double stretch = im->userStretch();
                              stretch += 0.1;
                              im->setUserStretch(stretch);
                              }
                        layout();
                        }
                  break;
            case Qt::Key_Minus:
                  if (sel->state == SEL_SYSTEM || sel->state == SEL_STAFF) {
                        int startTick = sel->tickStart;
                        int endTick = sel->tickEnd;
                        for (Measure* im = _layout->first(); im; im = im->next()) {
                              if (im->tick() < startTick)
                                    continue;
                              if (im->tick() >= endTick)
                                    break;
                              double stretch = im->userStretch();
                              stretch -= 0.1;
                              im->setUserStretch(stretch);
                              }
                        layout();
                        }
                  break;
            case Qt::Key_C:  note = 0; break;
            case Qt::Key_D:  note = 1; break;
            case Qt::Key_E:  note = 2; break;
            case Qt::Key_F:  note = 3; break;
            case Qt::Key_G:  note = 4; break;
            case Qt::Key_A:  note = 5; break;
            case Qt::Key_B:  note = 6; break;

            case Qt::Key_X:
                  cmdFlipStemDirection();
                  break;

            case Qt::Key_Space:
                  if (cis->pos == -1) {
                        setNoteEntry(true, true);
                        Element* el = sel->element();
                        if (el->type() == NOTE)
                              el = el->parent();
                        if (el->isChordRest())
                              cis->pos = ((ChordRest*)el)->tick();
                        }
                  if (cis->pos != -1) {
                        int len = padState.tickLen;
                        setRest(cis->pos, staff(cis->staff), cis->voice, len);
                        cis->pos += len;
                        }
                  moveCursor();
                  break;

            case Qt::Key_Up:
                  if (ev->modifiers() == 0 || ev->modifiers() == Qt::ControlModifier)
                        cmdUpDown(true, ev->modifiers() == Qt::ControlModifier);
                  else {
                        if (!el)
                              break;
                        Element* e = 0;
                        if (el->type() == NOTE) {
                              Note* note = (Note*)el;
                              if (ev->modifiers() == Qt::AltModifier)
                                    e = upAlt(el);
                              else if (ev->modifiers() == (Qt::AltModifier|Qt::ControlModifier))
                                    e = upAltCtrl(note);
                              else if (ev->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))
                                    e = moveUp(note);
                              }
                        else if (ev->modifiers() == Qt::AltModifier)
                              e = upAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  break;

            case Qt::Key_Down:
                  if (ev->modifiers() == 0 || ev->modifiers() == Qt::ControlModifier)
                        cmdUpDown(false, ev->modifiers() == Qt::ControlModifier);
                  else {
                        if (!el)
                              break;
                        Element* e = 0;
                        if (el->type() == NOTE || el->type() == CHORD) {
                              if (el->type() == NOTE) {
                                    Note* note = (Note*)el;
                                    if (ev->modifiers() == Qt::AltModifier)
                                          e = downAlt(el);
                                    else if (ev->modifiers() == (Qt::AltModifier|Qt::ControlModifier))
                                          e = downAltCtrl(note);
                                    else if (ev->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier))
                                          e = moveDown(note);
                                    }
                              }
                        else if (ev->modifiers() == Qt::AltModifier)
                              e = downAlt(el);
                        if (e) {
                              if (e->type() == NOTE)
                                    padState.pitch = ((Note*)e)->pitch();
                              select(e, 0, 0);
                              }
                        }
                  break;
            case Qt::Key_Right:
                  if (canvas()->getState() == Canvas::NOTE_ENTRY)
                        setNoteEntry(false, false);
                  cr = sel->lastChordRest();
                  if (cr) {
                        if (ev->modifiers() == Qt::ControlModifier)
                              el = nextMeasure(cr);
                        else
                              el = nextChordRest(cr);
                        if (el && el->type() == CHORD)
                              el = ((Chord*)el)->upNote();
                        if (el) {
                              select(el, 0, 0);
                              adjustCanvasPosition(el);
                              }
                        }
                  break;
            case Qt::Key_Left:
                  if (canvas()->getState() == Canvas::NOTE_ENTRY)
                        setNoteEntry(false, false);
                  cr = sel->firstChordRest();
                  if (cr) {
                        if (ev->modifiers() == Qt::ControlModifier)
                              el = prevMeasure(cr);
                        else
                              el = prevChordRest(cr);
                        if (el && el->type() == CHORD)
                              el = ((Chord*)el)->upNote();
                        if (el) {
                              select(el, 0, 0);
                              adjustCanvasPosition(el);
                              }
                        }
                  break;

            case Qt::Key_PageUp:
                  pagePrev();
                  break;
            case Qt::Key_PageDown:
                  pageNext();
                  break;
            case Qt::Key_Home:
                  pageTop();
                  break;
            case Qt::Key_End:
                  pageEnd();
                  break;
            case Qt::Key_S:
                  if (keyState == Qt::ShiftModifier)
                        cmdAddTie();
                  else
                        cmdAddSlur();
                  break;

            case Qt::Key_H:
                  cmdAddHairpin(ev->modifiers() & Qt::ShiftModifier);
                  break;

            case Qt::Key_Escape:
                  if (cis->pos != -1 || canvas()->getState() == Canvas::PALETTE) {
                        setNoteEntry(false, false);
                        break;
                        }
                  select(0, 0, 0);
                  break;

            case Qt::Key_Delete:
                  cmdDeleteSelection();
                  break;

            default:
                  endCmd(true);
                  return;
            }
      if (note != -1) {
            // c d e f g a b   entered
            int octave = padState.pitch / 12;
            static int ptab[15][7] = {
              //    c  d  e  f  g  a  b
                  { -1, 1, 3, 5, 6,  8, 10 },     // Bes
                  { -1, 2, 3, 5, 6,  8, 10 },     // Ges
                  {  0, 2, 3, 5, 6,  8, 10 },     // Des
                  {  0, 2, 3, 5, 7,  8, 10 },     // As
                  {  0, 2, 3, 5, 7,  8, 10 },     // Es
                  {  0, 2, 3, 5, 7,  9, 10 },     // B
                  {  0, 2, 4, 5, 7,  9, 11 },     // F
                  {  0, 2, 4, 5, 7,  9, 11 },     // C
                  {  0, 2, 4, 6, 7,  9, 11 },     // G
                  {  1, 2, 4, 6, 7,  9, 11 },     // D
                  {  1, 2, 4, 6, 8,  9, 11 },     // A
                  {  1, 3, 4, 6, 8,  9, 11 },     // E
                  {  1, 3, 4, 6, 8, 10, 11 },     // H
                  {  1, 3, 5, 6, 8, 10, 11 },     // Fis
                  {  1, 3, 5, 6, 8, 10, 12 },     // Cis
                  };
            int key   = keymap->key(cis->pos) + 7;
            int pitch = ptab[key][note];

            int delta = padState.pitch - (octave*12 + pitch);
            if (delta > 6)
                  padState.pitch = (octave+1)*12 + pitch;
            else if (delta < -6)
                  padState.pitch = (octave-1)*12 + pitch;
            else
                  padState.pitch = octave*12 + pitch;
            if (padState.pitch < 0)
                  padState.pitch = 0;
            if (padState.pitch > 127)
                  padState.pitch = 127;
            int len = padState.tickLen;
            ChordRest* cr = 0;
            if (cis->pos == -1) {
                  //
                  // start note entry mode at current selected
                  // note/rest
                  //
                  cr = setNoteEntry(true, false);
                  if (cr == 0) {
                        // cannot enter notes
                        // no note/rest selected?
                        endCmd(true);
                        return;
                        }
                  }
            else {
                  //
                  // look for next note position
                  //
                  cr = (ChordRest*)searchNote(cis->pos, cis->staff);
                  if (!cr->isChordRest()) {
                        cis->pos = -1;
                        endCmd(true);
                        return;
                        }
                  }
            if (ev->modifiers() & Qt::ShiftModifier) {
                  Note* on = getSelectedNote();
                  if (on) {
                        Note* n = addNote(on->chord(), padState.pitch);
                        select(n, 0, 0);
                        }
                  }
            else {
                  if (cr->tuplet()) {
                        len = cr->tuplet()->noteLen();
                        printf("current ist tuplet lä %d\n", len);
                        }
                  setNote(cis->pos, staff(cis->staff), cis->voice, padState.pitch, len);
                  }
            cis->pos += len;
            moveCursor();
            }
      endCmd(true);
      }

//---------------------------------------------------------
//   padTriggered
//---------------------------------------------------------

void MuseScore::padTriggered(QAction* action)
      {
      if (cs)
            cs->padToggle(action->data().toInt());
      }

void MuseScore::padTriggered(int n)
      {
      if (cs)
            cs->padToggle(n);
      }

//---------------------------------------------------------
//   padToggle
//    called from keyPadToggle
//    menu button callback
//---------------------------------------------------------

void Score::padToggle(int n)
      {
      startCmd();
      switch (n) {
            case PAD_NOTE1:
                  padState.len = division * 4;
                  break;
            case PAD_NOTE2:
                  padState.len = division * 2;
                  break;
            case PAD_NOTE4:
                  padState.len = division;
                  break;
            case PAD_NOTE8:
                  padState.len = division/2;
                  break;
            case PAD_NOTE16:
                  padState.len = division/4;
                  break;
            case PAD_NOTE32:
                  padState.len = division/8;
                  break;
            case PAD_NOTE64:
                  padState.len = division/16;
                  break;
            case PAD_SHARP2:
                  padState.prefix = padState.prefix != 3 ? 3 : 0;
                  break;
            case PAD_SHARP:
                  padState.prefix = padState.prefix != 1 ? 1 : 0;
                  break;
            case PAD_NAT:
                  padState.prefix = padState.prefix != 5 ? 5 : 0;
                  break;
            case PAD_FLAT:
                  padState.prefix = padState.prefix != 2 ? 2 : 0;
                  break;
            case PAD_FLAT2:
                  padState.prefix = padState.prefix != 4 ? 4 : 0;
                  break;
            case PAD_REST:
                  padState.rest = !padState.rest;
                  break;
            case PAD_DOT:
                  padState.dot = !padState.dot;
                  break;
            case PAD_TIE:
                  padState.tie = !padState.tie;
                  break;
            case PAD_ESCAPE:
                  canvas()->setState(Canvas::NORMAL);
                  break;
            case PAD_TENUTO:
                  addAttribute(TenutoSym);
                  break;
            case PAD_AKZENT:
                  addAttribute(SforzatoaccentSym);
                  break;
            case PAD_STACCATO:
                  addAttribute(StaccatoSym);
                  break;
            case PAD_BEAM_START:
                  cmdSetBeamMode(BEAM_BEGIN);
                  break;
            case PAD_BEAM_MID:
                  cmdSetBeamMode(BEAM_MID);
                  break;
            case PAD_BEAM_NO:
                  cmdSetBeamMode(BEAM_NO);
                  break;
            case PAD_BEAM32:
                  cmdSetBeamMode(BEAM_BEGIN32);
                  break;
            case PAD_FLIP:
                  cmdFlipStemDirection();
                  break;
            case PAD_VOICE0:
                  padState.voice = 0;
                  break;
            case PAD_VOICE1:
                  padState.voice = 1;
                  break;
            case PAD_VOICE2:
                  padState.voice = 2;
                  break;
            case PAD_VOICE3:
                  padState.voice = 3;
                  break;
            }
      setPadState();
      if (n >= PAD_NOTE1 && n < PAD_TIE) {
            if (n >= PAD_NOTE1 && n <= PAD_NOTE64) {
                  padState.dot = false;
                  //
                  // if in "note enter" mode, reset
                  // rest flag
                  //
                  if (cis->pos != -1)
                        padState.rest = false;
                  }
            if (cis->pos == -1 && sel->state == SEL_SINGLE) {
                  Element* el = sel->element();
                  if (el->type() == NOTE)
                        el = el->parent();
                  if (el->isChordRest()) {
                        int tick = ((ChordRest*)el)->tick();
                        int len = padState.tickLen;
                        if (padState.rest)
                              setRest(tick, staff(cis->staff), cis->voice, len);
                        else
                              setNote(tick, staff(cis->staff), cis->voice, padState.pitch, len);
                        }
                  }
            }
      else if (n >= PAD_SHARP2 && n <= PAD_FLAT2)
            addAccidental(padState.prefix);
      else if (n >= PAD_VOICE0 && n <= PAD_VOICE3)
            changeVoice(padState.voice);
      else if (n == PAD_TIE) {
            if (cis->pos == -1 && sel->state == SEL_SINGLE) {
                  Element* el = sel->element();
                  if (el->type() == NOTE) {
            		Tie* tie = new Tie(this);
                        tie->setParent(el);

            		cmdAdd(tie);
      			connectTies();
                        }
                  }
            }
      endCmd(true);
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void setPadState(Element* obj)
      {
      ElementType type = obj->type();
      int len = -1;
      padState.tie = false;

      if (type == NOTE) {
            Note* note = (Note*)obj;
            Chord* chord = note->chord();
            Tuplet* tuplet = chord->tuplet();
            len = tuplet ? tuplet->baseLen() : chord->tickLen();
            Accidental* prefix  = ((Note*)obj)->accidental();
            padState.prefix = prefix ? prefix->idx() : 0;
            padState.rest   = false;
            padState.voice  = obj->voice();
            padState.pitch  = ((Note*)obj)->pitch();
            padState.tie    = ((Note*)obj)->tieFor();
            }
      else if (type == REST) {
            Rest* rest = (Rest*)obj;
            Tuplet* tuplet = rest->tuplet();
            len = tuplet ? tuplet->baseLen() : rest->tickLen();
            padState.prefix = 0;
            padState.rest   = true;
            padState.voice  = obj->voice();
            }
      else {
            padState.rest   = false;
            padState.len    = 0;
            padState.prefix = 0;
            }
      if (len == -1) {
            padState.dot = false;
            return;
            }
      struct nv {
            int ticks;
            int pad;
            } values[] = {
            { division*4 ,  PAD_NOTE1},
            { division*2 ,  PAD_NOTE2},
            { division ,    PAD_NOTE4},
            { division/2 ,  PAD_NOTE8},
            { division/4 ,  PAD_NOTE16},
            { division/8 ,  PAD_NOTE32},
            { division/16 , PAD_NOTE64},
            };

      for (unsigned int i = 0; i < sizeof(values)/sizeof(*values); ++i) {
            int n = values[i].ticks;
            if (len / n) {
                  padState.len = n;
                  int rest     = len % n;
                  if (len <= (division * 4))
                        padState.dot = (rest == n/2);
                  else
                        padState.dot = false;
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState()
      {
      mscore->setEntry(false, PAD_ESCAPE);
      mscore->setEntry(padState.rest, PAD_REST);
      mscore->setEntry(padState.dot, PAD_DOT);
      mscore->setEntry(padState.tie, PAD_TIE);
      mscore->setEntry(padState.len == division*4, PAD_NOTE1);
      mscore->setEntry(padState.len == division*2, PAD_NOTE2);
      mscore->setEntry(padState.len == division, PAD_NOTE4);
      mscore->setEntry(padState.len == division/2, PAD_NOTE8);
      mscore->setEntry(padState.len == division/4, PAD_NOTE16);
      mscore->setEntry(padState.len == division/8, PAD_NOTE32);
      mscore->setEntry(padState.len == division/16, PAD_NOTE64);
      mscore->setEntry(padState.prefix == 3, PAD_SHARP2);
      mscore->setEntry(padState.prefix == 1, PAD_SHARP);
      mscore->setEntry(padState.prefix == 5, PAD_NAT);
      mscore->setEntry(padState.prefix == 2, PAD_FLAT);
      mscore->setEntry(padState.prefix == 4, PAD_FLAT2);
      for (int i = 0; i < VOICES; ++i)
            mscore->setEntry(padState.voice == i, PAD_VOICE0 + i);
      padState.tickLen = padState.len +
         (padState.dot ? padState.len/2 : 0);
      }

