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
#include "partedit.h"

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(Synth* s, QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      synth = s;

      reverbRoomSize->setId(0);
      reverbDamp->setId(1);
      reverbWidth->setId(2);
      reverb->setId(3);
      chorus->setId(4);

      soundFont->setText(preferences.soundFont);
      masterTuning->setValue(preferences.tuning);
      setMasterGain(preferences.masterGain);

      reverb->setValue(preferences.reverbGain);
      roomSizeBox->setValue(preferences.reverbRoomSize);
      dampBox->setValue(preferences.reverbDamp);
      widthBox->setValue(preferences.reverbWidth);

      chorus->setValue(preferences.chorusGain);
      chorusSpeed->setValue(synth->effectParameter(1, 1));
      chorusDepth->setValue(synth->effectParameter(1, 2));

      connect(sfButton, SIGNAL(clicked()), SLOT(selectSoundFont()));
      connect(gain,     SIGNAL(valueChanged(double,int)), SLOT(masterGainChanged(double,int)));
      connect(masterTuning, SIGNAL(valueChanged(double)),       SLOT(masterTuningChanged(double)));

      connect(reverb,         SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbRoomSize, SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbDamp, SIGNAL(valueChanged(double,int)),     SLOT(reverbValueChanged(double,int)));
      connect(reverbWidth, SIGNAL(valueChanged(double,int)),    SLOT(reverbValueChanged(double,int)));

      connect(chorus,      SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusSpeed, SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusDepth, SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      }

//---------------------------------------------------------
//   setMasterGain
//---------------------------------------------------------

void SynthControl::setMasterGain(float val)
      {
      gain->setValue(val);
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
            synthControl = new SynthControl(seq->getDriver()->getSynth(), this);
            connect(synthControl, SIGNAL(closed()), SLOT(closeSynthControl()));
            connect(seq, SIGNAL(masterVolumeChanged(float)), synthControl, SLOT(setMasterGain(float)));
            connect(synthControl, SIGNAL(masterGainChanged(float)), seq, SLOT(setMasterVolume(float)));

            if (iledit) {
                  connect(synthControl, SIGNAL(soundFontChanged()), iledit,
                     SLOT(patchListChanged()));
                  }
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
         || (preferences.reverbRoomSize != reverbRoomSize->value())
         || (preferences.reverbDamp != reverbDamp->value())
         || (preferences.reverbWidth != reverbWidth->value())
         || (preferences.reverbGain != reverb->value())
         || (preferences.chorusGain != chorus->value())
         ) {
            preferences.dirty  = true;
            }
      preferences.soundFont  = soundFont->text();
      preferences.tuning     = masterTuning->value();
      preferences.masterGain = gain->value();

      preferences.reverbRoomSize = reverbRoomSize->value();
      preferences.reverbDamp     = reverbDamp->value();
      preferences.reverbWidth    = reverbWidth->value();
      preferences.reverbGain     = reverb->value();
      preferences.chorusGain     = chorus->value();
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
            emit soundFontChanged();
            updatePreferences();
            }
      }

//---------------------------------------------------------
//   masterGainChanged
//---------------------------------------------------------

void SynthControl::masterGainChanged(double val, int)
      {
      emit masterGainChanged(val);
      updatePreferences();
      }

//---------------------------------------------------------
//   masterTuningChanged
//---------------------------------------------------------

void SynthControl::masterTuningChanged(double val)
      {
      synth->setMasterTuning(val);
      }

//---------------------------------------------------------
//   setMeter
//---------------------------------------------------------

void SynthControl::setMeter(float l, float r, float left_peak, float right_peak)
      {
      gain->setMeterVal(0, l, left_peak);
      gain->setMeterVal(1, r, right_peak);
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void SynthControl::stop()
      {
      gain->setMeterVal(0, .0, .0);
      gain->setMeterVal(1, .0, .0);
      }

//---------------------------------------------------------
//   reverbValueChanged
//---------------------------------------------------------

void SynthControl::reverbValueChanged(double val, int idx)
      {
      synth->setEffectParameter(0, idx, val);
      updatePreferences();
      }

//---------------------------------------------------------
//   chorusValueChanged
//---------------------------------------------------------

void SynthControl::chorusValueChanged(double val, int idx)
      {
      synth->setEffectParameter(1, idx, val);
      updatePreferences();
      }

