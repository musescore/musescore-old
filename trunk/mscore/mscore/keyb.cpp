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
#include "text.h"

//---------------------------------------------------------
//   Canvas::keyPressEvent
//---------------------------------------------------------

void Canvas::keyPressEvent(QKeyEvent* ev)
      {
      int key = ev->key();
//printf("key key:%x state:%x text:<%s>\n", key,
//    int(ev->modifiers()), ev->text().toLatin1().data());

      if (state != EDIT && state != DRAG_EDIT) {
            ev->ignore();
            return;
            }
      Element* e = _score->editObject;
      if (key == Qt::Key_Escape) {
            if (state == DRAG_EDIT)
                  e->endEditDrag();
            setState(NORMAL);
            _score->endCmd();
            ev->accept();
            return;
            }
      if (e->type() == LYRICS) {
            int found = false;
            if (ev->key() == Qt::Key_Space && !(ev->modifiers() & Qt::ControlModifier)) {
                  // TODO: shift+tab events are filtered by qt
                  _score->lyricsTab(ev->modifiers() & Qt::ShiftModifier);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Return) {
                  _score->lyricsReturn();
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Minus && !(ev->modifiers() & Qt::ControlModifier)) {
                  _score->lyricsMinus();
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (e->type() == TEXT && e->subtype() == TEXT_CHORD) {
            if (ev->key() == Qt::Key_Space && !(ev->modifiers() & Qt::ControlModifier)) {
                  _score->chordTab(ev->modifiers() & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            }
      if (e->edit(curGrip, ev)) {
            updateGrips();
            _score->end();
            ev->accept();
            return;
            }
      QPointF delta;
      qreal val = 1.0;
      if (ev->modifiers() & Qt::ControlModifier)
            val = 0.1;
      switch (ev->key()) {
            case Qt::Key_Left:
                  delta = QPointF(-val, 0);
                  break;
            case Qt::Key_Right:
                  delta = QPointF(val, 0);
                  break;
            case Qt::Key_Up:
                  delta = QPointF(0, -val);
                  break;
            case Qt::Key_Down:
                  delta = QPointF(0, val);
                  break;
            case Qt::Key_Tab:
                  if (curGrip < (grips-1))
                        ++curGrip;
                  else
                        curGrip = 0;
                  val = 0.0;
                  break;
            default:
                  ev->ignore();
                  return;
            }
      e->editDrag(curGrip, grip[curGrip].center(), delta);
      updateGrips();
      _score->end();
      ev->accept();
      }

//---------------------------------------------------------
//   padToggle
//    called from keyPadToggle
//    menu button callback
//---------------------------------------------------------

void Score::padToggle(int n)
      {
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
            case PAD_REST:
                  padState.rest = !padState.rest;
                  break;
            case PAD_DOT:
                  padState.dot = !padState.dot;
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
            }
      setPadState();
      if (n >= PAD_NOTE1 && n <= PAD_DOT) {
            if (n >= PAD_NOTE1 && n <= PAD_NOTE64) {
                  padState.dot = false;
                  //
                  // if in "note enter" mode, reset
                  // rest flag
                  //
                  if (noteEntryMode())
                        padState.rest = false;
                  }
            if (!noteEntryMode() && sel->state() == SEL_SINGLE) {
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
            padState.prefix = prefix ? prefix->subtype() : 0;
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
      getAction("pad-rest")->setChecked(padState.rest);
      getAction("pad-dot")->setChecked(padState.dot);
      getAction("pad-tie")->setChecked(padState.tie);

      getAction("pad-note-1")->setChecked(padState.len == division * 4);
      getAction("pad-note-2")->setChecked(padState.len == division*2);
      getAction("pad-note-4")->setChecked(padState.len == division);
      getAction("pad-note-8")->setChecked(padState.len == division/2);
      getAction("pad-note-16")->setChecked(padState.len == division/4);
      getAction("pad-note-32")->setChecked(padState.len == division/8);
      getAction("pad-note-64")->setChecked(padState.len == division/16);

      getAction("pad-sharp2")->setChecked(padState.prefix == 3);
      getAction("pad-sharp")->setChecked(padState.prefix == 1);
      getAction("pad-nat")->setChecked(padState.prefix == 5);
      getAction("pad-flat")->setChecked(padState.prefix == 2);
      getAction("pad-flat2")->setChecked(padState.prefix == 4);

      getAction("voice-1")->setChecked(padState.voice == 0);
      getAction("voice-2")->setChecked(padState.voice == 1);
      getAction("voice-3")->setChecked(padState.voice == 2);
      getAction("voice-4")->setChecked(padState.voice == 3);

      padState.tickLen = padState.len + (padState.dot ? padState.len/2 : 0);
      }

