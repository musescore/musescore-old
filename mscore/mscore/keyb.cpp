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
#include "staff.h"
#include "part.h"

//---------------------------------------------------------
//   Canvas::keyPressEvent
//---------------------------------------------------------

void Canvas::keyPressEvent(QKeyEvent* ev)
      {
      int key = ev->key();
      if (debugMode) {
            printf("key key:%x modifiers:%x text:<%s>\n", key,
               int(ev->modifiers()), qPrintable(ev->text()));
            }

      if (state != EDIT && state != DRAG_EDIT) {
            ev->ignore();
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
                  if (e->edit(this, curGrip, ev))
                        _score->end();
                  else
                        _score->lyricsTab(true, true);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (e->edit(this, curGrip, ev))
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
      if (e->edit(this, curGrip, ev)) {
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
            case PAD_NOTE1:
                  _padState.len = division * 4;
                  break;
            case PAD_NOTE2:
                  _padState.len = division * 2;
                  break;
            case PAD_NOTE4:
                  _padState.len = division;
                  break;
            case PAD_NOTE8:
                  _padState.len = division/2;
                  break;
            case PAD_NOTE16:
                  _padState.len = division/4;
                  break;
            case PAD_NOTE32:
                  _padState.len = division/8;
                  break;
            case PAD_NOTE64:
                  _padState.len = division/16;
                  break;
            case PAD_REST:
                  _padState.rest = !_padState.rest;
                  break;
            case PAD_DOT:
                  if (_padState.dots == 1)
                        _padState.dots = 0;
                  else
                        _padState.dots = 1;
                  break;
            case PAD_DOTDOT:
                  if (_padState.dots == 2)
                        _padState.dots = 0;
                  else
                        _padState.dots = 2;
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
      if (n >= PAD_NOTE1 && n <= PAD_DOTDOT) {
            if (n >= PAD_NOTE1 && n <= PAD_NOTE64) {
                  _padState.dots = 0;
                  //
                  // if in "note enter" mode, reset
                  // rest flag
                  //
                  if (noteEntryMode())
                        _padState.rest = false;
                  }
            if (!noteEntryMode() && sel->state() == SEL_SINGLE) {
                  Element* el = sel->element();
                  if (el->type() == NOTE)
                        el = el->parent();
                  if (el->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(el);
                        int tick      = cr->tick();
                        int len       = _padState.tickLen;
                        if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
                              //
                              // handle appoggiatura and acciaccatura
                              //
                              Chord* c = static_cast<Chord*>(cr);
                              cr->setTickLen(len);
                              for (iNote in = c->noteList()->begin(); in != c->noteList()->end(); ++in) {
                                    Note* n = in->second;
                                    n->setTickLen(len);
                                    }
                              }
                        else {
                              if (cr->tuplet()) {
                                    int pitch = _padState.rest ? -1 : _padState.pitch;
                                    setTupletChordRest(cr, pitch, len);
                                    }
                              else {
                                    if (_padState.rest)
                                          setRest(tick, _is.track, len, _padState.dots);
                                    else
                                          // setNote(tick, _is.track, _padState.pitch, len);
                                          changeCRlen(cr, len);
                                    }
                              }
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
      _padState.tie = false;

      _padState.drumNote = -1;
      _padState.drumset  = 0;
      if (e->type() == NOTE) {
            Note* note          = static_cast<Note*>(e);
            Chord* chord        = note->chord();
            len                 = chord->duration().ticks();
            Accidental* prefix  = note->accidental();
            _padState.prefix    = prefix ? prefix->subtype() : 0;
            _padState.rest      = false;
            _padState.voice     = note->voice();
            _padState.pitch     = note->pitch();
            _padState.tie       = note->tieFor();
            _padState.noteType  = note->noteType();
            _padState.beamMode  = chord->beamMode();
            }
      else if (e->type() == REST) {
            Rest* rest = static_cast<Rest*>(e);
            len        = rest->duration().ticks();

            if (len == 0)           // whole measure rest?
                  len = rest->segment()->measure()->tickLen();

            _padState.prefix   = 0;
            _padState.rest     = true;
            _padState.voice    = rest->voice();
            _padState.beamMode = rest->beamMode();
            }
      else {
            _padState.rest     = false;
            _padState.len      = 0;
            _padState.prefix   = 0;
            _padState.noteType = NOTE_INVALID;
            _padState.beamMode = BEAM_INVALID;
            }
      if (e->type() == NOTE || e->type() == REST) {
            Instrument* instr   = e->staff()->part()->instrument();
            if (instr->useDrumset) {
                  if (e->type() == NOTE)
                        _padState.drumNote = static_cast<Note*>(e)->pitch();
                  else
                        _padState.drumNote = -1;
                  _padState.drumset  = instr->drumset;
                   }
            }
      if (len == -1) {
            _padState.dots = 0;
            return;
            }
      Duration d;
      headType(len, &d, &(_padState.dots));
      _padState.len = d.ticks();
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState()
      {
      getAction("pad-rest")->setChecked(_padState.rest);
      getAction("pad-dot")->setChecked(_padState.dots == 1);
      getAction("pad-dotdot")->setChecked(_padState.dots == 2);
      getAction("pad-tie")->setChecked(_padState.tie);

      getAction("pad-note-1")->setChecked(_padState.len == division * 4);
      getAction("pad-note-2")->setChecked(_padState.len == division*2);
      getAction("pad-note-4")->setChecked(_padState.len == division);
      getAction("pad-note-8")->setChecked(_padState.len == division/2);
      getAction("pad-note-16")->setChecked(_padState.len == division/4);
      getAction("pad-note-32")->setChecked(_padState.len == division/8);
      getAction("pad-note-64")->setChecked(_padState.len == division/16);

      getAction("pad-sharp2")->setChecked(_padState.prefix == 3);
      getAction("pad-sharp")->setChecked(_padState.prefix == 1);
      getAction("pad-nat")->setChecked(_padState.prefix == 5);
      getAction("pad-flat")->setChecked(_padState.prefix == 2);
      getAction("pad-flat2")->setChecked(_padState.prefix == 4);
      getAction("pad-staccato")->setChecked(_padState.prefix == 16);

      getAction("voice-1")->setChecked(_padState.voice == 0);
      getAction("voice-2")->setChecked(_padState.voice == 1);
      getAction("voice-3")->setChecked(_padState.voice == 2);
      getAction("voice-4")->setChecked(_padState.voice == 3);

      getAction("pad-acciaccatura")->setChecked(_padState.noteType == NOTE_ACCIACCATURA);
      getAction("pad-appoggiatura")->setChecked(_padState.noteType == NOTE_APPOGGIATURA);
      getAction("pad-grace4")->setChecked(_padState.noteType == NOTE_GRACE4);
      getAction("pad-grace16")->setChecked(_padState.noteType == NOTE_GRACE16);
      getAction("pad-grace32")->setChecked(_padState.noteType == NOTE_GRACE32);

      getAction("beam-start")->setChecked(_padState.beamMode == BEAM_BEGIN);
      getAction("beam-mid")->setChecked(_padState.beamMode == BEAM_MID);
      getAction("no-beam")->setChecked(_padState.beamMode == BEAM_NO);
      getAction("beam32")->setChecked(_padState.beamMode == BEAM_BEGIN32);
      getAction("auto-beam")->setChecked(_padState.beamMode == BEAM_AUTO);

      _padState.tickLen = _padState.len;
      if (_padState.dots == 1)
            _padState.tickLen += _padState.len / 2;
      else if (_padState.dots == 2)
            _padState.tickLen += ((_padState.len * 3)/4);
      else if (_padState.dots)
            printf("too many dots: %d\n", _padState.dots);
      mscore->updateDrumset();
      }

