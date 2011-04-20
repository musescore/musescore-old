//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "drumtools.h"
#include "preferences.h"
#include "segment.h"

#ifdef Q_WS_MAC
#define CONTROL_MODIFIER Qt::AltModifier
#else
#define CONTROL_MODIFIER Qt::ControlModifier
#endif

//---------------------------------------------------------
//   drumset
//---------------------------------------------------------

Drumset* InputState::drumset() const
      {
      if (_segment == 0)
            return 0;
      return _segment->score()->staff(_track/VOICES)->part()->instr(_segment->tick())->drumset();
      }

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

      if (!editObject)
            return;

      if (editObject->type() == LYRICS) {
            int found = false;
		if (ev->key() == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  // TODO: shift+tab events are filtered by qt
                  lyricsTab(modifiers & Qt::ShiftModifier, true, false);
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Left) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s))
                        _score->end();
                  else
                        lyricsTab(true, true, true);      // go to previous lyrics
                  found = true;
                  }
            else if (ev->key() == Qt::Key_Right) {
                  if (!ctrl && editObject->edit(this, curGrip, key, modifiers, s))
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
		else if (ev->key() == Qt::Key_Minus && !(modifiers & CONTROL_MODIFIER)) {
                  lyricsMinus();
                  found = true;
                  }
		else if (ev->key() == Qt::Key_Underscore && !(modifiers & CONTROL_MODIFIER)) {
                  lyricsUnderscore();
                  found = true;
                  }
            if (found) {
                  ev->accept();
                  return;
                  }
            }
      if (editObject->type() == HARMONY) {
            if (ev->key() == Qt::Key_Space && !(modifiers & CONTROL_MODIFIER)) {
                  chordTab(modifiers & Qt::ShiftModifier);
                  ev->accept();
                  return;
                  }
            }
      if (!((modifiers & Qt::ShiftModifier) && (key == Qt::Key_Backtab))) {
            if (editObject->edit(this, curGrip, key, modifiers, s)) {
                  updateGrips();
                  ev->accept();
                  _score->end();
                  return;
                  }
            if (editObject->isText() && (ev->key() == Qt::Key_Left || ev->key() == Qt::Key_Right)) {
                  ev->accept();
                  _score->end();
                  //return;
                  }
            }
      QPointF delta;
      qreal _spatium = editObject->spatium();
      qreal val      = preferences.nudgeStep * _spatium;
      if (modifiers & Qt::ControlModifier)
            val = preferences.nudgeStep10 * _spatium;
      else if (modifiers & Qt::AltModifier)
            val = preferences.nudgeStep50 * _spatium;
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
      EditData ed;
      ed.curGrip = curGrip;
      ed.delta   = delta;
      ed.view    = this;
      editObject->editDrag(ed);
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
            case PAD_NOTE128:
                  _is.setDuration(Duration::V_128TH);
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
      mscore->updateInputState(this);
      if (n < PAD_NOTE00 || n > PAD_DOTDOT)
            return;

      if (n >= PAD_NOTE00 && n <= PAD_NOTE128) {
            _is.setDots(0);
            //
            // if in "note enter" mode, reset
            // rest flag
            //
            if (noteEntryMode())
                  _is.rest = false;
            }

      if (noteEntryMode() || !selection().isSingle()) {
            mscore->updateInputState(this);    // updates dot state
            return;
            }

      //do not allow to add a dot on a full measure rest
      Element* e = selection().element();
      if (e && e->type() == REST) {
            Rest* r = static_cast<Rest*>(e);
            Duration d = r->durationType();
            if (d.type() == Duration::V_MEASURE) {
                  _is.setDots(0);
                  mscore->updateInputState(this);    // updates dot state
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
            cr->setDurationType(_is.duration());
            }
      else
            changeCRlen(cr, _is.duration());
      }

//---------------------------------------------------------
//   enableInputToolbar
//---------------------------------------------------------

static void enableInputToolbar(bool val)
      {
      static const char* actionNames[] = {
            "pad-rest", "pad-dot", "pad-dotdot", "note-longa",
            "note-breve", "pad-note-1", "pad-note-2", "pad-note-4",
            "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64",
            "pad-note-128",
//            "voice-1", "voice-2", "voice-3", "voice-4",
            "acciaccatura", "appoggiatura", "grace4", "grace16",
            "grace32", "beam-start", "beam-mid", "no-beam", "beam32",
            "auto-beam"
            };
      for (unsigned i = 0; i < sizeof(actionNames)/sizeof(*actionNames); ++i) {
            getAction(actionNames[i])->setEnabled(val);
            }
      }

//---------------------------------------------------------
//   setInputState
//---------------------------------------------------------

void Score::setInputState(Element* e)
      {
// printf("setInputState %s\n", e ? e->name() : "--");
      bool enable = e && (e->type() == NOTE || e->type() == REST);
      enableInputToolbar(enable);
      if (e == 0) {
            mscore->showDrumTools(0, 0);
            return;
            }

      _is.setDrumNote(-1);
//      _is.setDrumset(0);
      if (e->type() == NOTE) {
            Note* note    = static_cast<Note*>(e);
            Chord* chord  = note->chord();
            _is.setDuration(chord->durationType());
            _is.rest      = false;
            _is.setTrack(note->track());
            _is.pitch     = note->pitch();
            _is.noteType  = note->noteType();
            _is.beamMode  = chord->beamMode();
            }
      else if (e->type() == REST) {
            Rest* rest   = static_cast<Rest*>(e);
            if (rest->durationType().type() == Duration::V_MEASURE)
                  _is.setDuration(Duration::V_QUARTER);
            else
                  _is.setDuration(rest->durationType());
            _is.rest     = true;
            _is.setTrack(rest->track());
            _is.beamMode = rest->beamMode();
            _is.noteType = NOTE_NORMAL;
            }
      else {
/*            _is.rest     = false;
            _is.setDots(0);
            _is.setDuration(Duration::V_INVALID);
            _is.noteType = NOTE_INVALID;
            _is.beamMode = BEAM_INVALID;
            _is.noteType = NOTE_NORMAL;
*/
            }
      if (e->type() == NOTE || e->type() == REST) {
            const Instrument* instr   = e->staff()->part()->instr();
            if (instr->useDrumset()) {
                  if (e->type() == NOTE)
                        _is.setDrumNote(static_cast<Note*>(e)->pitch());
                  else
                        _is.setDrumNote(-1);
//                  _is.setDrumset(instr->drumset());
                  }
            }
      mscore->updateInputState(this);
      }

//---------------------------------------------------------
//   updateInputState
//---------------------------------------------------------

void MuseScore::updateInputState(Score* score)
      {
      InputState& is = score->inputState();
      showDrumTools(is.drumset(), score->staff(is.track() / VOICES));

      getAction("pad-rest")->setChecked(is.rest);
      getAction("pad-dot")->setChecked(is.duration().dots() == 1);
      getAction("pad-dotdot")->setChecked(is.duration().dots() == 2);

      getAction("note-longa")->setChecked(is.duration()  == Duration::V_LONG);
      getAction("note-breve")->setChecked(is.duration()  == Duration::V_BREVE);
      getAction("pad-note-1")->setChecked(is.duration()  == Duration::V_WHOLE);
      getAction("pad-note-2")->setChecked(is.duration()  == Duration::V_HALF);
      getAction("pad-note-4")->setChecked(is.duration()  == Duration::V_QUARTER);
      getAction("pad-note-8")->setChecked(is.duration()  == Duration::V_EIGHT);
      getAction("pad-note-16")->setChecked(is.duration() == Duration::V_16TH);
      getAction("pad-note-32")->setChecked(is.duration() == Duration::V_32ND);
      getAction("pad-note-64")->setChecked(is.duration() == Duration::V_64TH);
      getAction("pad-note-128")->setChecked(is.duration() == Duration::V_128TH);

      // uncheck all voices if multi-selection
      int voice = score->selection().isSingle() ? is.voice() : -1;
      getAction("voice-1")->setChecked(voice == 0);
      getAction("voice-2")->setChecked(voice == 1);
      getAction("voice-3")->setChecked(voice == 2);
      getAction("voice-4")->setChecked(voice == 3);

      getAction("acciaccatura")->setChecked(is.noteType == NOTE_ACCIACCATURA);
      getAction("appoggiatura")->setChecked(is.noteType == NOTE_APPOGGIATURA);
      getAction("grace4")->setChecked(is.noteType  == NOTE_GRACE4);
      getAction("grace16")->setChecked(is.noteType == NOTE_GRACE16);
      getAction("grace32")->setChecked(is.noteType == NOTE_GRACE32);

      getAction("beam-start")->setChecked(is.beamMode == BEAM_BEGIN);
      getAction("beam-mid")->setChecked(is.beamMode   == BEAM_MID);
      getAction("no-beam")->setChecked(is.beamMode    == BEAM_NO);
      getAction("beam32")->setChecked(is.beamMode     == BEAM_BEGIN32);
      getAction("auto-beam")->setChecked(is.beamMode  == BEAM_AUTO);
      getAction("repitch")->setChecked(is.repitchMode());
      }

