//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "drumtools.h"
#include "mscore.h"
#include "palette.h"
#include "chord.h"
#include "note.h"
#include "drumset.h"
#include "score.h"
#include "preferences.h"
#include "seq.h"
#include "editdrumset.h"

//---------------------------------------------------------
//   DrumTools
//---------------------------------------------------------

DrumTools::DrumTools(QWidget* parent)
   : QDockWidget(parent)
      {
      drumset = 0;
      _score  = 0;
      setObjectName("drum-tools");
      setWindowTitle(tr("Drum Tools"));
      setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

      QWidget* w = new QWidget(this);
      w->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));
      QHBoxLayout* layout = new QHBoxLayout;
      w->setLayout(layout);

      QVBoxLayout* layout1 = new QVBoxLayout;
      QToolButton* tb = new QToolButton;
      tb->setText(tr("Drumset"));
      layout1->addWidget(tb);
      layout1->addStretch();
      layout->addLayout(layout1);

      drumPalette = new Palette;
      drumPalette->setName(tr("Drums"));
      drumPalette->setMag(0.8);
      drumPalette->setSelectable(true);
      drumPalette->setGrid(42, 60);
      PaletteScrollArea* sa = new PaletteScrollArea(drumPalette);
      layout->addWidget(sa);

      setWidget(w);
//      setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed));

      w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();
      connect(tb, SIGNAL(clicked()), SLOT(editDrumset()));
      void boxClicked(int);
      connect(drumPalette, SIGNAL(boxClicked(int)), SLOT(drumNoteSelected(int)));
      }

//---------------------------------------------------------
//   drumTools
//---------------------------------------------------------

DrumTools* MuseScore::drumTools()
      {
      if (!_drumTools) {
            _drumTools = new DrumTools(this);
            addDockWidget(Qt::BottomDockWidgetArea, _drumTools);
            }
      return _drumTools;
      }

//---------------------------------------------------------
//   hideDrumTools
//---------------------------------------------------------

void MuseScore::hideDrumTools()
      {
      if (_drumTools)
            _drumTools->hide();
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void DrumTools::setDrumset(Score* s, Drumset* ds)
      {
      _score  = s;
      drumset = ds;
      drumPalette->clear();
      if (drumset == 0)
            return;
      int drumInstruments = 0;
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (drumset->isValid(pitch))
                  ++drumInstruments;
            }
      int i = 0;
      double _spatium = gscore->spatium();
      for (int pitch = 0; pitch < 128; ++pitch) {
            if (!ds->isValid(pitch))
                  continue;
            bool up;
            int line      = ds->line(pitch);
            int noteHead  = ds->noteHead(pitch);
            int voice     = ds->voice(pitch);
            Direction dir = ds->stemDirection(pitch);
            if (dir == UP)
                  up = true;
            else if (dir == DOWN)
                  up = false;
            else
                  up = line > 4;

            Chord* chord = new Chord(gscore);
            chord->setDurationType(Duration::V_QUARTER);
            chord->setStemDirection(dir);
            chord->setTrack(voice);
            Note* note = new Note(gscore);
            note->setParent(chord);
            note->setTrack(voice);
            note->setPitch(pitch);
            note->setTpcFromPitch();
            note->setLine(line);
            note->setPos(0.0, _spatium * .5 * line);
            note->setHeadGroup(noteHead);
            chord->add(note);
            Stem* stem = new Stem(gscore);
            stem->setLen((up ? -3.0 : 3.0) * _spatium);
            chord->setStem(stem);
            stem->setPos(note->stemPos(up));
            drumPalette->append(chord, qApp->translate("drumset", qPrintable(drumset->name(pitch))));
            ++i;
            }
      }

//---------------------------------------------------------
//   editDrumset
//---------------------------------------------------------

void DrumTools::editDrumset()
      {
      EditDrumset* eds = new EditDrumset(drumset, this);
      eds->exec();
      }

//---------------------------------------------------------
//   drumNoteSelected
//---------------------------------------------------------

void DrumTools::drumNoteSelected(int val)
      {
      Element* element = drumPalette->element(val);
      Chord* ch        = static_cast<Chord*>(element);
      Note* note       = ch->downNote();
      int ticks        = preferences.defaultPlayDuration;
      int pitch        = note->pitch();
//      seq->startNote(part->instr()->channel(0), pitch, 80, ticks, 0.0);
      _score->inputState().setDrumNote(note->pitch());
      }

