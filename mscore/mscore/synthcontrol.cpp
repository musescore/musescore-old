//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp 2054 2009-08-28 16:15:01Z wschweer $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "synthcontrol.h"
#include "mscore.h"
#include "seq.h"
#include "synti.h"
#include "preferences.h"

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(Synth* s, QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      synth = s;
      soundFont->setText(synth->soundFont());
      masterTuning->setValue(synth->masterTuning());
      gain->setValue(synth->masterGain());
      reverb->setValue(synth->reverbGain());
      chorus->setValue(synth->chorusGain());

      connect(sfButton, SIGNAL(clicked()), SLOT(selectSoundFont()));
      connect(gain,     SIGNAL(valueChanged(double, int)), SLOT(masterGainChanged(double, int)));
      connect(reverb,   SIGNAL(valueChanged(double, int)), SLOT(reverbGainChanged(double, int)));
      connect(chorus,   SIGNAL(valueChanged(double, int)), SLOT(chorusGainChanged(double, int)));
      connect(masterTuning, SIGNAL(valueChanged(double)), SLOT(masterTuningChanged(double)));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void SynthControl::closeEvent(QCloseEvent* ev)
      {
      emit closed();
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   showSynthControl
//---------------------------------------------------------

void MuseScore::showSynthControl(bool val)
      {
      if (synthControl == 0) {
            synthControl = new SynthControl(seq->getDriver()->getSynth(), 0);
            connect(synthControl, SIGNAL(closed()), SLOT(closeSynthControl()));
            }
      synthControl->setShown(val);
      }

//---------------------------------------------------------
//   closeSynthControl
//---------------------------------------------------------

void MuseScore::closeSynthControl()
      {
      getAction("synth-control")->setChecked(false);
      }

//---------------------------------------------------------
//   updatePreferences
//---------------------------------------------------------

void SynthControl::updatePreferences()
      {
      if ((preferences.soundFont != soundFont->text())
         || (preferences.tuning != masterTuning->value())
         || (preferences.masterGain != gain->value())
         || (preferences.chorusGain != chorus->value())
         || (preferences.reverbGain != reverb->value())
         ) {
            preferences.dirty  = true;
            }
      preferences.soundFont  = soundFont->text();
      preferences.tuning     = masterTuning->value();
      preferences.masterGain = gain->value();
      preferences.chorusGain = chorus->value();
      preferences.reverbGain = reverb->value();
      }

//---------------------------------------------------------
//   selectSoundFont
//---------------------------------------------------------

void SynthControl::selectSoundFont()
      {
      QString s = QFileDialog::getOpenFileName(
         this,
         tr("Choose Synthesizer SoundFont"),
         soundFont->text(),
         tr("SoundFont Files (*.sf2 *.SF2);;All (*)")
         );
      if (!s.isNull()) {
            soundFont->setText(s);
            synth->loadSoundFont(s);
            }
      }
//---------------------------------------------------------
//   masterGainChanged
//---------------------------------------------------------

void SynthControl::masterGainChanged(double val, int)
      {
      synth->setMasterGain(val);
      }

//---------------------------------------------------------
//   chorusGainChanged
//---------------------------------------------------------

void SynthControl::chorusGainChanged(double val, int)
      {
      synth->setChorusGain(val);
      }

//---------------------------------------------------------
//   reverbGainChanged
//---------------------------------------------------------

void SynthControl::reverbGainChanged(double val, int)
      {
      synth->setReverbGain(val);
      }

//---------------------------------------------------------
//   masterTuningChanged
//---------------------------------------------------------

void SynthControl::masterTuningChanged(double val)
      {
      synth->setMasterTuning(val);
      }

