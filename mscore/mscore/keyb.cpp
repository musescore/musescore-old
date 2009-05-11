//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: keyb.cpp,v 1.55 2006/04/12 14:58:10 wschweer Exp $
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
#include "padids.h"
#include "sym.h"
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
#include "staff.h"
#include "part.h"

//---------------------------------------------------------
//   Canvas::keyPressEvent
//---------------------------------------------------------

void Canvas::keyPressEvent(QKeyEvent* ev)
      {
      int key                         = ev->key();
      Qt::KeyboardModifiers modifiers = ev->modifiers();
      QString s                       = ev->text();

      if (debugMode) {
            printf("key key:%x modifiers:%x text:<%s>\n", key,
               int(modifiers), qPrintable(s));
            }

      if (state != EDIT && state != DRAG_EDIT) {
            ev->ignore();
            if (debugMode)
                  printf("  ignore\n");
            return;
            }
      Element* e = _score->editObject;
      if (key == Qt::Key_Escape) {  // TODO: no ESC arrived here because its defined as
            if (state == DRAG_EDIT) //       a shortcut
                  e->endEditDrag();
            setState(NORMAL);
            ev->accept();
            return;
            }
      if (e->type() == LYRICS) {
            int found = false;
            if (ev->key() == Qt::Key_Space && !(ev->modifiers() & Qt::ControlModifier)) {
                  // TODO: shift+tab events are filtered by qt
                  _score->lyricsTab(ev->modifiers() & Qt::ShiftModifier, true);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Left) {
                  if (e->edit(this, curGrip, key, modifiers, s))
                        _score->end();
                  else
                        _score->lyricsTab(true, true);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (e->edit(this, curGrip, key, modifiers, s))
                        _score->end();
                  else
                        _score->lyricsTab(false, false);
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
            else if (ev->key() == Qt::Key_Underscore && !(ev->modifiers() & Qt::ControlModifier)) {
                  _score->lyricsUnderscore();
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (e->type() == HARMONY) {
            if (ev->key() == Qt::Key_Space && !(ev->modifiers() & Qt::ControlModifier)) {
                  _score->chordTab(ev->modifiers() & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            }
      if (e->edit(this, curGrip, key, modifiers, s)) {
            updateGrips();
            ev->accept();
            _score->end();
            return;
            }
      QPointF delta;
      qreal val = 10.0;
      if (ev->modifiers() & Qt::ControlModifier)
            val = 1.0;
      else if (ev->modifiers() & Qt::AltModifier)
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
      e->editDrag(curGrip, delta);
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
            case PAD_NOTE00:
                  _is.len = division * 16;
                  break;
            case PAD_NOTE0:
                  _is.len = division * 8;
                  break;
            case PAD_NOTE1:
                  _is.len = division * 4;
                  break;
            case PAD_NOTE2:
                  _is.len = division * 2;
                  break;
            case PAD_NOTE4:
                  _is.len = division;
                  break;
            case PAD_NOTE8:
                  _is.len = division/2;
                  break;
            case PAD_NOTE16:
                  _is.len = division/4;
                  break;
            case PAD_NOTE32:
                  _is.len = division/8;
                  break;
            case PAD_NOTE64:
                  _is.len = division/16;
                  break;
            case PAD_REST:
                  _is.rest = !_is.rest;
                  break;
            case PAD_DOT:
                  if (_is.dots == 1)
                        _is.dots = 0;
                  else
                        _is.dots = 1;
                  break;
            case PAD_DOTDOT:
                  if (_is.dots == 2)
                        _is.dots = 0;
                  else
                        _is.dots = 2;
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
      if (n < PAD_NOTE00 || n > PAD_DOTDOT)
            return;

      if (n >= PAD_NOTE00 && n <= PAD_NOTE64) {
            _is.dots = 0;
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.rest = false;
            }

      if (noteEntryMode() || sel->state() != SEL_SINGLE) {
            setPadState();    // updates dot state
            return;
            }

      Element* el = sel->element();
      if (el->type() == NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      int tick      = cr->tick();
      int len       = _is.tickLen;
      if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setTickLen(len);
            }
      else {
            if (cr->tuplet()) {
                  int pitch = _is.rest ? -1 : _is.pitch;
                  setTupletChordRest(cr, pitch, len);
                  }
            else {
                  if (_is.rest) {
                        setRest(tick, _is.track, len, _is.dots);
                        }
                  else {
                        changeCRlen(cr, len);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState(Element* e)
      {
      int len       = -1;

      _is.drumNote = -1;
      _is.drumset  = 0;

      if (e->type() == NOTE) {
            Note* note          = static_cast<Note*>(e);
            Chord* chord        = note->chord();
            len = chord->duration().ticks(chord->dots());

            Accidental* prefix  = note->accidental();
            _is.prefix    = prefix ? prefix->subtype() : 0;
            _is.rest      = false;
            _is.track     = note->track();
            _is.pitch     = note->pitch();
            _is.noteType  = note->noteType();
            _is.beamMode  = chord->beamMode();
            }
      else if (e->type() == REST) {
            Rest* rest = static_cast<Rest*>(e);
            len        = rest->duration().ticks(rest->dots());

            if (len == 0)           // whole measure rest?
                  len = rest->segment()->measure()->tickLen();

            _is.prefix   = 0;
            _is.rest     = true;
            _is.track    = rest->track();
            _is.beamMode = rest->beamMode();
            }
      else {
            _is.rest     = false;
            _is.len      = 0;
            _is.prefix   = 0;
            _is.noteType = NOTE_INVALID;
            _is.beamMode = BEAM_INVALID;
            }
      if (e->type() == NOTE || e->type() == REST) {
            Instrument* instr   = e->staff()->part()->instrument();
            if (instr->useDrumset) {
                  if (e->type() == NOTE)
                        _is.drumNote = static_cast<Note*>(e)->pitch();
                  else
                        _is.drumNote = -1;
                  _is.drumset  = instr->drumset;
                   }
            }
      if (len == -1) {
            _is.dots = 0;
            return;
            }
      Duration d;
      headType(len, &d, &(_is.dots));
      _is.len = d.ticks();
      setPadState();
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState()
      {
      getAction("pad-rest")->setChecked(_is.rest);
      getAction("pad-dot")->setChecked(_is.dots == 1);
      getAction("pad-dotdot")->setChecked(_is.dots == 2);

      getAction("note-longa")->setChecked(_is.len == division * 16);
      getAction("note-breve")->setChecked(_is.len == division * 8);
      getAction("pad-note-1")->setChecked(_is.len == division * 4);
      getAction("pad-note-2")->setChecked(_is.len == division*2);
      getAction("pad-note-4")->setChecked(_is.len == division);
      getAction("pad-note-8")->setChecked(_is.len == division/2);
      getAction("pad-note-16")->setChecked(_is.len == division/4);
      getAction("pad-note-32")->setChecked(_is.len == division/8);
      getAction("pad-note-64")->setChecked(_is.len == division/16);

      getAction("pad-sharp2")->setChecked(_is.prefix == 3);
      getAction("pad-sharp")->setChecked(_is.prefix == 1);
      getAction("pad-nat")->setChecked(_is.prefix == 5);
      getAction("pad-flat")->setChecked(_is.prefix == 2);
      getAction("pad-flat2")->setChecked(_is.prefix == 4);

      int voice = _is.voice();
      getAction("voice-1")->setChecked(voice == 0);
      getAction("voice-2")->setChecked(voice == 1);
      getAction("voice-3")->setChecked(voice == 2);
      getAction("voice-4")->setChecked(voice == 3);

      getAction("pad-acciaccatura")->setChecked(_is.noteType == NOTE_ACCIACCATURA);
      getAction("pad-appoggiatura")->setChecked(_is.noteType == NOTE_APPOGGIATURA);
      getAction("pad-grace4")->setChecked(_is.noteType == NOTE_GRACE4);
      getAction("pad-grace16")->setChecked(_is.noteType == NOTE_GRACE16);
      getAction("pad-grace32")->setChecked(_is.noteType == NOTE_GRACE32);

      getAction("beam-start")->setChecked(_is.beamMode == BEAM_BEGIN);
      getAction("beam-mid")->setChecked(_is.beamMode == BEAM_MID);
      getAction("no-beam")->setChecked(_is.beamMode == BEAM_NO);
      getAction("beam32")->setChecked(_is.beamMode == BEAM_BEGIN32);
      getAction("auto-beam")->setChecked(_is.beamMode == BEAM_AUTO);

      _is.tickLen = _is.len;
      if (_is.dots == 1)
            _is.tickLen += _is.len / 2;
      else if (_is.dots == 2)
            _is.tickLen += ((_is.len * 3)/4);
      else if (_is.dots)
            printf("too many dots: %d\n", _is.dots);
      mscore->updateDrumset();
      }

