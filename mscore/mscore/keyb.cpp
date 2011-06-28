//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "scoreview.h"
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
#include "slur.h"
#include "tuplet.h"
#include "text.h"
#include "staff.h"
#include "part.h"

//---------------------------------------------------------
//   Canvas::editKey
//---------------------------------------------------------

void ScoreView::editKey(QKeyEvent* ev)
      {
      int key                         = ev->key();
      Qt::KeyboardModifiers modifiers = ev->modifiers();
      QString s                       = ev->text();
      bool ctrl                       = modifiers == Qt::ControlModifier;

      if (debugMode)
            printf("keyPressEvent key 0x%02x mod 0x%04x <%s>\n",
               key, int(modifiers), qPrintable(s));

      Element* e = editObject;
      if (!e)
            return;

      if (e->type() == LYRICS) {
            int found = false;
#ifdef Q_WS_MAC
            if (ev->key() == Qt::Key_Space && !(modifiers & Qt::AltModifier)) {
#else
		if (ev->key() == Qt::Key_Space && !(modifiers & Qt::ControlModifier)) {
#endif
                  // TODO: shift+tab events are filtered by qt
                  lyricsTab(modifiers & Qt::ShiftModifier, true, false);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Left) {
                  if (!ctrl && e->edit(this, curGrip, key, modifiers, s))
                        _score->end();
                  else
                        lyricsTab(true, true, true);      // go to previous lyrics
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (!ctrl && e->edit(this, curGrip, key, modifiers, s))
                        _score->end();
                  else
                        lyricsTab(false, false, true);    // go to next lyrics
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Up) {
                  lyricsUpDown(true, true);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Down) {
                  lyricsUpDown(false, true);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Return) {
                  lyricsReturn();
                  found = true;
                  }
#ifdef Q_WS_MAC
            else if (ev->key() == Qt::Key_Minus && !(modifiers & Qt::AltModifier)) {
#else
		else if (ev->key() == Qt::Key_Minus && !(modifiers & Qt::ControlModifier)) {
#endif
                  lyricsMinus();
                  found = true;
                  }
#ifdef Q_WS_MAC
		else if (ev->key() == Qt::Key_Underscore && !(modifiers & Qt::AltModifier)) {
#else
		else if (ev->key() == Qt::Key_Underscore && !(modifiers & Qt::ControlModifier)) {
#endif
                  lyricsUnderscore();
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (e->type() == HARMONY) {
#ifdef Q_WS_MAC
            if (ev->key() == Qt::Key_Space && !(modifiers & Qt::AltModifier)) {
#else
            if (ev->key() == Qt::Key_Space && !(modifiers & Qt::ControlModifier)) {
#endif
                  chordTab(modifiers & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            if (ev->key() == Qt::Key_Tab || ev->key() == Qt::Key_Backtab) {
                  chordTabTab(ev->key() == Qt::Key_Backtab?true:(modifiers & Qt::ShiftModifier));
                  ev->accept();
                  return;
                  }
            }
      if (!((modifiers & Qt::ShiftModifier) && (key == Qt::Key_Backtab))) {
            if (e->edit(this, curGrip, key, modifiers, s)) {
                  updateGrips();
                  ev->accept();
                  _score->end();
                  return;
                  }
            if (e->isTextB() && (ev->key() == Qt::Key_Left || ev->key() == Qt::Key_Right)) {
                  ev->accept();
                  _score->end();
                  //return;
                  }
            }
      QPointF delta;
      qreal val = 10.0;
      if (modifiers & Qt::ControlModifier)
            val = 1.0;
      else if (modifiers & Qt::AltModifier)
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
                  _is.setDuration(Duration::V_LONG);
                  break;
            case PAD_NOTE0:
                  _is.setDuration(Duration::V_BREVE);
                  break;
            case PAD_NOTE1:
                  _is.setDuration(Duration::V_WHOLE);
                  break;
            case PAD_NOTE2:
                  _is.setDuration(Duration::V_HALF);
                  break;
            case PAD_NOTE4:
                  _is.setDuration(Duration::V_QUARTER);
                  break;
            case PAD_NOTE8:
                  _is.setDuration(Duration::V_EIGHT);
                  break;
            case PAD_NOTE16:
                  _is.setDuration(Duration::V_16TH);
                  break;
            case PAD_NOTE32:
                  _is.setDuration(Duration::V_32ND);
                  break;
            case PAD_NOTE64:
                  _is.setDuration(Duration::V_64TH);
                  break;
            case PAD_REST:
                  _is.rest = !_is.rest;
                  break;
            case PAD_DOT:
                  if (_is.duration().dots() == 1)
                        _is.setDots(0);
                  else
                        _is.setDots(1);
                  break;
            case PAD_DOTDOT:
                  if (_is.duration().dots() == 2)
                        _is.setDots(0);
                  else
                        _is.setDots(2);
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
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.rest = false;
            }

      if (noteEntryMode() || !selection().isSingle()) {
            setPadState();    // updates dot state
            return;
            }

      //do not allow to add a dot on a full measure rest
      Element* e = selection().element();
      if (e && e->type() == REST) {
            Rest* r = static_cast<Rest*>(e);
            Duration d = r->duration();
            if (d.type() == Duration::V_MEASURE) {
                  _is.setDots(0);
                  setPadState();    // updates dot state
                  // return;
                  }
            }

      Element* el = selection().element();
      if (el->type() == NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setDuration(_is.duration());
            }
      else
            changeCRlen(cr, _is.duration());
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState(Element* e)
      {
      _is.setDrumNote(-1);
      _is.setDrumset(0);

      if (e->type() == NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            _is.setDuration(chord->duration());
            _is.rest      = false;
            _is.track     = note->track();
            _is.pitch     = note->pitch();
            _is.noteType  = note->noteType();
            _is.beamMode  = chord->beamMode();
            }
      else if (e->type() == REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->duration().type() == Duration::V_MEASURE)
                  _is.setDuration(Duration::V_QUARTER);
            else
                  _is.setDuration(rest->duration());
            _is.rest     = true;
            _is.track    = rest->track();
            _is.beamMode = rest->beamMode();
            }
      else {
            _is.rest     = false;
            _is.setDots(0);
            _is.setDuration(Duration::V_INVALID);
            _is.noteType = NOTE_INVALID;
            _is.beamMode = BEAM_INVALID;
            }
      if (e->type() == NOTE || e->type() == REST) {
            Part* instr   = e->staff()->part();
            if (instr->useDrumset()) {
                  if (e->type() == NOTE)
                        _is.setDrumNote(static_cast<Note*>(e)->pitch());
                  else
                        _is.setDrumNote(-1);
                  _is.setDrumset(instr->drumset());
                   }
            }
      setPadState();
      }

//---------------------------------------------------------
//   setPadState
//---------------------------------------------------------

void Score::setPadState()
      {
      getAction("pad-rest")->setChecked(_is.rest);
      getAction("pad-dot")->setChecked(_is.duration().dots() == 1);
      getAction("pad-dotdot")->setChecked(_is.duration().dots() == 2);

      getAction("note-longa")->setChecked(_is.duration()  == Duration::V_LONG);
      getAction("note-breve")->setChecked(_is.duration()  == Duration::V_BREVE);
      getAction("pad-note-1")->setChecked(_is.duration()  == Duration::V_WHOLE);
      getAction("pad-note-2")->setChecked(_is.duration()  == Duration::V_HALF);
      getAction("pad-note-4")->setChecked(_is.duration()  == Duration::V_QUARTER);
      getAction("pad-note-8")->setChecked(_is.duration()  == Duration::V_EIGHT);
      getAction("pad-note-16")->setChecked(_is.duration() == Duration::V_16TH);
      getAction("pad-note-32")->setChecked(_is.duration() == Duration::V_32ND);
      getAction("pad-note-64")->setChecked(_is.duration() == Duration::V_64TH);

      int voice = _is.voice();
      getAction("voice-1")->setChecked(voice == 0);
      getAction("voice-2")->setChecked(voice == 1);
      getAction("voice-3")->setChecked(voice == 2);
      getAction("voice-4")->setChecked(voice == 3);

      getAction("pad-acciaccatura")->setChecked(_is.noteType == NOTE_ACCIACCATURA);
      getAction("pad-appoggiatura")->setChecked(_is.noteType == NOTE_APPOGGIATURA);
      getAction("pad-grace4")->setChecked(_is.noteType  == NOTE_GRACE4);
      getAction("pad-grace16")->setChecked(_is.noteType == NOTE_GRACE16);
      getAction("pad-grace32")->setChecked(_is.noteType == NOTE_GRACE32);

      getAction("beam-start")->setChecked(_is.beamMode == BEAM_BEGIN);
      getAction("beam-mid")->setChecked(_is.beamMode   == BEAM_MID);
      getAction("no-beam")->setChecked(_is.beamMode    == BEAM_NO);
      getAction("beam32")->setChecked(_is.beamMode     == BEAM_BEGIN32);
      getAction("auto-beam")->setChecked(_is.beamMode  == BEAM_AUTO);

      mscore->updateDrumset();
      }

