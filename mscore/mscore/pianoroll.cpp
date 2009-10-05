//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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
#include "pianoroll.h"
#include "piano.h"
#include "ruler.h"
#include "pianoscene.h"
#include "staff.h"
#include "score.h"
#include "measure.h"
#include "voiceselector.h"
#include "note.h"
#include "awl/pitchlabel.h"
#include "awl/pitchedit.h"
#include "awl/poslabel.h"
#include "mscore.h"
#include "undo.h"
#include "part.h"
#include "instrument.h"
#include "seq.h"
#include "preferences.h"

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(Staff* st, QWidget* parent)
   : QDialog(parent)
      {
      staff = st;
      score = staff->score();

      AL::TempoMap* tl = score->tempomap();
      AL::TimeSigMap*  sl = score->sigmap();
      for (int i = 0; i < 3; ++i)
            locator[i].setContext(tl, sl);

      locator[0].setTick(480 * 5 + 240);
      locator[1].setTick(480 * 3 + 240);
      locator[2].setTick(480 * 12 + 240);

      QGridLayout* layout = new QGridLayout;
      layout->setSpacing(0);

      QToolBar* tb = new QToolBar;
      tb->addAction(getAction("undo"));
      tb->addAction(getAction("redo"));
      tb->addSeparator();
      tb->addAction(getAction("sound-on"));
#ifdef HAS_MIDI
      tb->addAction(getAction("midi-on"));
#endif
      tb->addSeparator();
      tb->addAction(getAction("rewind"));
      tb->addAction(getAction("play"));
      tb->addSeparator();
      layout->addWidget(tb, 0, 0, 1, 2);

      tb = new QToolBar;
      layout->addWidget(tb, 1, 0, 1, 2);
      VoiceSelector* vs = new VoiceSelector;
      tb->addWidget(vs);

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Cursor:")));
      Awl::PosLabel* pos = new Awl::PosLabel(tl, sl);
      tb->addWidget(pos);
      Awl::PitchLabel* pl = new Awl::PitchLabel();
      tb->addWidget(pl);

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Velocity:")));
      veloType = new QComboBox;
      veloType->addItem(tr("auto"),   0);
      veloType->addItem(tr("user"),   1);
      veloType->addItem(tr("offset"), 2);
      tb->addWidget(veloType);

      velocity = new QSpinBox;
      velocity->setRange(-1, 127);
      velocity->setSpecialValueText("--");
      velocity->setReadOnly(true);
      tb->addWidget(velocity);

      tb->addWidget(new QLabel(tr("Pitch:")));
      pitch = new Awl::PitchEdit;
      pitch->setReadOnly(true);
      tb->addWidget(pitch);

      double xmag = .1;
      gv  = new PianoView(staff, locator);
      gv->scale(xmag, 1.0);
      layout->addWidget(gv, 3, 1);

      Ruler* ruler = new Ruler(score, locator);
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);
      ruler->setMag(xmag, 1.0);

      layout->addWidget(ruler, 2, 1);

      Piano* piano = new Piano;
      piano->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      piano->setFixedWidth(pianoWidth);
      layout->addWidget(piano, 3, 0);

      setLayout(layout);

      connect(gv->verticalScrollBar(), SIGNAL(valueChanged(int)), piano, SLOT(setYpos(int)));
      connect(gv->horizontalScrollBar(), SIGNAL(valueChanged(int)), ruler, SLOT(setXpos(int)));
      connect(gv,    SIGNAL(xposChanged(int)),           ruler, SLOT(setXpos(int)));
      connect(gv,    SIGNAL(magChanged(double,double)),  ruler, SLOT(setMag(double,double)));
      connect(gv,    SIGNAL(magChanged(double,double)),  piano, SLOT(setMag(double,double)));
      connect(gv,    SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(gv,    SIGNAL(pitchChanged(int)),          piano, SLOT(setPitch(int)));
      connect(piano, SIGNAL(pitchChanged(int)),          pl,    SLOT(setPitch(int)));
      connect(piano, SIGNAL(keyPressed(int)),                   SLOT(keyPressed(int)));
      connect(piano, SIGNAL(keyReleased(int)),                  SLOT(keyReleased(int)));
      connect(gv,    SIGNAL(posChanged(const AL::Pos&)), pos,   SLOT(setValue(const AL::Pos&)));
      connect(gv,    SIGNAL(posChanged(const AL::Pos&)), ruler, SLOT(setPos(const AL::Pos&)));
      connect(ruler, SIGNAL(posChanged(const AL::Pos&)), pos,   SLOT(setValue(const AL::Pos&)));
      connect(ruler, SIGNAL(locatorMoved(int)),          gv->scene(), SLOT(update()));
      connect(veloType,    SIGNAL(activated(int)),     SLOT(veloTypeChanged(int)));
      connect(velocity,    SIGNAL(valueChanged(int)),  SLOT(velocityChanged(int)));
      connect(gv->scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
      resize(800, 400);
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = (Note*)item->data(0).value<void*>();
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
//   veloTypeChanged
//---------------------------------------------------------

void PianorollEditor::veloTypeChanged(int val)
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() != 1)
            return;
      QGraphicsItem* item = items[0];
      Note* note = (Note*)item->data(0).value<void*>();
      if (ValueType(val) == note->veloType())
            return;

      score->undo()->beginMacro();
      score->undo()->push(new ChangeVelocity(note, ValueType(val), note->velocity(), note->veloOffset()));
      score->undo()->endMacro(score->undo()->current()->childCount() == 0);
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
                  velocity->setValue(note->velocity());
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
      ValueType vt = note->veloType();

      if (vt == AUTO_VAL)
            return;

      int velocity = note->velocity();
      int offset   = note->veloOffset();

      if (vt == USER_VAL)
            velocity = val;
      else
            offset = val;

      score->undo()->beginMacro();
      score->undo()->push(new ChangeVelocity(note, vt, velocity, offset));
      score->undo()->endMacro(score->undo()->current()->childCount() == 0);
      }

//---------------------------------------------------------
//   keyPressed
//---------------------------------------------------------

void PianorollEditor::keyPressed(int pitch)
      {
      Instrument* i = staff->part()->instrument();
      seq->startNote(i->channel[0], pitch, 80, 0, 0.0);
      }

//---------------------------------------------------------
//   keyReleased
//---------------------------------------------------------

void PianorollEditor::keyReleased(int pitch)
      {
      seq->stopNotes();
      }

