//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "config.h"
#include "chordeditor.h"
#include "piano.h"
#include "ruler.h"
#include "chordview.h"
#include "staff.h"
#include "score.h"
#include "measure.h"
#include "voiceselector.h"
#include "note.h"
#include "mscore.h"
#include "undo.h"
#include "part.h"
#include "instrument.h"
#include "seq.h"
#include "preferences.h"
#include "seq.h"
#include "chord.h"

//---------------------------------------------------------
//   ChordEditor
//---------------------------------------------------------

ChordEditor::ChordEditor(Chord* c, QWidget* parent)
   : QDialog(parent)
      {
      _chord = c;
      setWindowTitle(QString("MuseScore"));

      QGridLayout* layout = new QGridLayout;
      setLayout(layout);
      layout->setSpacing(0);

      double xmag = .1;
      pianoroll  = new ChordView;
      pianoroll->scale(xmag, 1.0);
      layout->addWidget(pianoroll, 1, 0);

      ruler = new Ruler;
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);
      ruler->setMag(xmag, 1.0);

      layout->addWidget(ruler, 0, 0);

      Score* score = _chord->score();
      Staff* staff = _chord->staff();
      AL::TempoMap* tl    = score->tempomap();
      AL::TimeSigMap*  sl = score->sigmap();
      for (int i = 0; i < 3; ++i)
            locator[i].setContext(tl, sl);

      locator[0].setTick(480 * 5 + 240);  // some random test values
      locator[1].setTick(480 * 3 + 240);
      locator[2].setTick(480 * 12 + 240);

      pianoroll->setChord(_chord, locator);
      ruler->setScore(score, locator);

//      connect(pianoroll->verticalScrollBar(), SIGNAL(valueChanged(int)),   piano, SLOT(setYpos(int)));
      connect(pianoroll->horizontalScrollBar(), SIGNAL(valueChanged(int)), ruler, SLOT(setXpos(int)));
      connect(pianoroll,          SIGNAL(xposChanged(int)),           ruler,       SLOT(setXpos(int)));
      connect(pianoroll,          SIGNAL(magChanged(double,double)),  ruler,       SLOT(setMag(double,double)));
//      connect(pianoroll,          SIGNAL(magChanged(double,double)),  piano,       SLOT(setMag(double,double)));
//      connect(pianoroll,          SIGNAL(pitchChanged(int)),          pl,          SLOT(setPitch(int)));
//      connect(pianoroll,          SIGNAL(pitchChanged(int)),          piano,       SLOT(setPitch(int)));
//      connect(piano,       SIGNAL(pitchChanged(int)),          pl,          SLOT(setPitch(int)));
//      connect(pianoroll,          SIGNAL(posChanged(const AL::Pos&)), pos,         SLOT(setValue(const AL::Pos&)));
      connect(pianoroll,          SIGNAL(posChanged(const AL::Pos&)), ruler,       SLOT(setPos(const AL::Pos&)));
//      connect(ruler,       SIGNAL(posChanged(const AL::Pos&)), pos,         SLOT(setValue(const AL::Pos&)));
//      connect(ruler,       SIGNAL(locatorMoved(int)),                       SLOT(moveLocator(int)));
//      connect(pianoroll->scene(), SIGNAL(selectionChanged()),                      SLOT(selectionChanged()));
//      connect(piano,       SIGNAL(keyPressed(int)),                         SLOT(keyPressed(int)));
//      connect(piano,       SIGNAL(keyReleased(int)),                        SLOT(keyReleased(int)));
      resize(800, 400);
      }

#if 0
//---------------------------------------------------------
//   updateSelection
//---------------------------------------------------------

void PianorollEditor::updateSelection()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            pitch->setEnabled(true);
            pitch->setValue(note->pitch());
            veloType->setEnabled(true);
            velocity->setEnabled(true);
            updateVelocity(note);
            }
      else if (items.size() == 0) {
            velocity->setValue(0);
            velocity->setEnabled(false);
            pitch->setValue(0);
            pitch->setEnabled(false);
            veloType->setEnabled(false);
            veloType->setCurrentIndex(int(AUTO_VAL));
            }
      else {
            velocity->setEnabled(true);
            velocity->setValue(0);
            velocity->setReadOnly(false);
            pitch->setEnabled(true);
            pitch->setDeltaMode(true);
            pitch->setValue(0);
            veloType->setEnabled(true);
            veloType->setCurrentIndex(int(OFFSET_VAL));
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      updateSelection();
      _score->blockSignals(true);
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  _score->select(note, SELECT_SINGLE, 0);
            }
      else if (items.size() == 0) {
            _score->select(0, SELECT_SINGLE, 0);
            }
      else {
            _score->select(0, SELECT_SINGLE, 0);
            foreach(QGraphicsItem* item, items) {
                  Note* note = static_cast<Note*>(item->data(0).value<void*>());
                  if (note)
                        _score->select(note, SELECT_ADD, 0);
                  }
            }
      _score->setUpdateAll();
      _score->end();
      _score->blockSignals(false);
      }

//---------------------------------------------------------
//   changeSelection
//---------------------------------------------------------

void PianorollEditor::changeSelection(int)
      {
      gv->scene()->blockSignals(true);
      gv->scene()->clearSelection();
      QList<QGraphicsItem*> il = gv->scene()->items();
      foreach(QGraphicsItem* item, il) {
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  item->setSelected(note->selected());
            }
      gv->scene()->blockSignals(false);
      }

//---------------------------------------------------------
//   veloTypeChanged
//---------------------------------------------------------

void PianorollEditor::veloTypeChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      Note* note = (Note*)item->data(0).value<void*>();
      if ((note == 0) || (ValueType(val) == note->veloType()))
            return;

      _score->undo()->beginMacro();
      _score->undo()->push(new ChangeVelocity(note, ValueType(val), note->veloOffset()));
      _score->undo()->endMacro(_score->undo()->current()->childCount() == 0);
      updateVelocity(note);
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void PianorollEditor::updateVelocity(Note* note)
      {
      ValueType vt = note->veloType();
      if (vt != ValueType(veloType->currentIndex())) {
            veloType->setCurrentIndex(int(vt));
            switch(vt) {
                  case AUTO_VAL:
                        velocity->setReadOnly(true);
                        velocity->setSuffix("");
                        velocity->setRange(0, 127);
                        break;
                  case USER_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("");
                        velocity->setRange(0, 127);
                        break;
                  case OFFSET_VAL:
                        velocity->setReadOnly(false);
                        velocity->setSuffix("%");
                        velocity->setRange(-200, 200);
                        break;
                  }
            }
      switch(vt) {
            case AUTO_VAL:
            case USER_VAL:
                  // TODO velocity->setValue(note->velocity());
                  break;
            case OFFSET_VAL:
                  velocity->setValue(note->veloOffset());
                  break;
            }
      }

//---------------------------------------------------------
//   velocityChanged
//---------------------------------------------------------

void PianorollEditor::velocityChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      Note* note = (Note*)item->data(0).value<void*>();
      if (note == 0)
            return;
      ValueType vt = note->veloType();

      if (vt == AUTO_VAL)
            return;

      _score->undo()->beginMacro();
      _score->undo()->push(new ChangeVelocity(note, vt, val));
      _score->undo()->endMacro(_score->undo()->current()->childCount() == 0);
      }

//---------------------------------------------------------
//   keyPressed
//---------------------------------------------------------

void PianorollEditor::keyPressed(int pitch)
      {
      seq->startNote(staff->part()->instr()->channel(0), pitch, 80, 0, 0.0);
      }

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void PianorollEditor::keyReleased(int /*pitch*/)
      {
      seq->stopNotes();
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PianorollEditor::heartBeat(Seq* seq)
      {
      unsigned t = seq->getCurTick();
      if (locator[0].tick() != t) {
            locator[0].setTick(t);
            gv->moveLocator(0);
            ruler->update();
            if (preferences.followSong)
                  gv->ensureVisible(t);
            }
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void PianorollEditor::moveLocator(int i)
      {
      if (locator[i].valid()) {
            seq->seek(locator[i].tick());
            gv->moveLocator(i);
            }
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void PianorollEditor::cmd(QAction* a)
      {
      score()->startCmd();
      if (a->data() == "delete") {
            QList<QGraphicsItem*> items = gv->items();
            foreach(QGraphicsItem* item, items) {
                  Note* note = static_cast<Note*>(item->data(0).value<void*>());
                  if (note) {
                        score()->deleteItem(note);
                        }
                  }
            }

      gv->setStaff(staff, locator);
      score()->endCmd();
      }
#endif

