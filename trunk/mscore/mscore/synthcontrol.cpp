//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp 2054 2009-08-28 16:15:01Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "msynth/synti.h"
#include "preferences.h"
#include "mixer.h"
#include "aeolus/aeolus/aeolus.h"
#include "score.h"
#include "file.h"
#include "msynth/sparm_p.h"
#include "icons.h"

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

SynthControl::SynthControl(MasterSynth* s, QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      sfButton->setIcon(*icons[fileOpen_ICON]);
      saveReverbPreset->setIcon(*icons[fileSave_ICON]);
      saveChorusPreset->setIcon(*icons[fileSave_ICON]);

      synti = s;

      reverbRoomSize->setId(0);
      reverbDamp->setId(1);
      reverbWidth->setId(2);
      reverb->setId(3);
      chorus->setId(4);

      masterTuning->setValue(synti->masterTuning());
      setGain(synti->gain());

      reverb->setValue(synti->parameter(SParmId(FLUID_ID, 0, 3).val).fval());
      roomSizeBox->setValue(synti->parameter(SParmId(FLUID_ID, 0, 0).val).fval());
      dampBox->setValue(synti->parameter(SParmId(FLUID_ID, 0, 1).val).fval());
      widthBox->setValue(synti->parameter(SParmId(FLUID_ID, 0, 2).val).fval());

      chorus->setValue(synti->parameter(SParmId(FLUID_ID, 1, 4).val).fval());
      chorusSpeed->setValue(synti->parameter(SParmId(FLUID_ID, 1, 1).val).fval());
      chorusDepth->setValue(synti->parameter(SParmId(FLUID_ID, 1, 2).val).fval());

      reverbDelay->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_REVSIZE).val));
      reverbDelay->setId(AEOLUS_REVSIZE);
      connect(reverbDelay, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      reverbTime->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_REVTIME).val));
      reverbTime->setId(AEOLUS_REVTIME);
      connect(reverbTime, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      position->init(synti->parameter(SParmId(AEOLUS_ID, 0, AEOLUS_STPOSIT).val));
      position->setId(AEOLUS_STPOSIT);

      connect(position, SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));

      aeolusSection[0][0] = aeolusAzimuth3;
      aeolusSection[0][1] = aeolusWidth3;
      aeolusSection[0][2] = aeolusDirect3;
      aeolusSection[0][3] = aeolusReflect3;
      aeolusSection[0][4] = aeolusReverb3;

      aeolusSection[1][0] = aeolusAzimuth2;
      aeolusSection[1][1] = aeolusWidth2;
      aeolusSection[1][2] = aeolusDirect2;
      aeolusSection[1][3] = aeolusReflect2;
      aeolusSection[1][4] = aeolusReverb2;

      aeolusSection[2][0] = aeolusAzimuth1;
      aeolusSection[2][1] = aeolusWidth1;
      aeolusSection[2][2] = aeolusDirect1;
      aeolusSection[2][3] = aeolusReflect1;
      aeolusSection[2][4] = aeolusReverb1;

      aeolusSection[3][0] = aeolusAzimuthP;
      aeolusSection[3][1] = aeolusWidthP;
      aeolusSection[3][2] = aeolusDirectP;
      aeolusSection[3][3] = aeolusReflectP;
      aeolusSection[3][4] = aeolusReverbP;

      for (int i = 0; i < 4; ++i) {
            for (int k = 0; k < 5; ++k) {
                  aeolusSection[i][k]->init(synti->parameter(SParmId(AEOLUS_ID, i+1, k).val));
                  aeolusSection[i][k]->setId(((i+1) << 8) + k);
                  connect(aeolusSection[i][k], SIGNAL(valueChanged(double, int)), SLOT(setAeolusValue(double, int)));
                  }
            }

      soundFontUp->setEnabled(false);
      soundFontDown->setEnabled(false);
      soundFontDelete->setEnabled(false);
      soundFontAdd->setEnabled(false);

      connect(sfButton,        SIGNAL(clicked()),                SLOT(selectSoundFont()));
      connect(gain,            SIGNAL(valueChanged(double,int)), SLOT(gainChanged(double,int)));
      connect(masterTuning,    SIGNAL(valueChanged(double)),     SLOT(masterTuningChanged(double)));

      connect(reverb,          SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbRoomSize,  SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbDamp,      SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));
      connect(reverbWidth,     SIGNAL(valueChanged(double,int)), SLOT(reverbValueChanged(double,int)));

      connect(chorus,          SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusSpeed,     SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));
      connect(chorusDepth,     SIGNAL(valueChanged(double,int)), SLOT(chorusValueChanged(double,int)));

      connect(soundFontUp,     SIGNAL(clicked()),                SLOT(sfUpClicked()));
      connect(soundFontDown,   SIGNAL(clicked()),                SLOT(sfDownClicked()));
      connect(soundFontDelete, SIGNAL(clicked()),                SLOT(sfDeleteClicked()));
      connect(soundFontAdd,    SIGNAL(clicked()),                SLOT(sfAddClicked()));
      connect(soundFont,       SIGNAL(textChanged(const QString&)), SLOT(sfChanged(const QString&)));
      connect(soundFonts,      SIGNAL(currentRowChanged(int)),   SLOT(currentSoundFontChanged(int)));
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void SynthControl::setScore(Score* cs)
      {
      setWindowTitle("MuseScore:Synthesizer " + (cs ? cs->name() : tr("no score")));
      Synth* sy = synti->synth("Fluid");
      soundFonts->clear();
      if (sy)
            soundFonts->addItems(sy->soundFonts());
      }

//---------------------------------------------------------
//   setGain
//---------------------------------------------------------

void SynthControl::setGain(float val)
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
            synthControl = new SynthControl(seq->getSynti(), this);
            synthControl->setScore(cs);
            connect(synthControl, SIGNAL(closed()), SLOT(closeSynthControl()));
            connect(seq, SIGNAL(gainChanged(float)), synthControl, SLOT(setGain(float)));
            connect(synthControl, SIGNAL(gainChanged(float)), seq, SLOT(setGain(float)));

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
      QString s = ::getSoundFont(soundFont->text());
      if (!s.isEmpty()) {
            soundFont->setText(s);
            soundFontAdd->setEnabled(true);
            }
      }

//---------------------------------------------------------
//   sfChanged
//---------------------------------------------------------

void SynthControl::sfChanged(const QString& s)
      {
      if (!s.isEmpty())
            soundFontAdd->setEnabled(true);
      }

//---------------------------------------------------------
//   sfDeleteClicked
//---------------------------------------------------------

void SynthControl::sfDeleteClicked()
      {
      int row = soundFonts->currentRow();
      if (row >= 0) {
            QString s(soundFonts->item(row)->text());
            Synth* sy = synti->synth("Fluid");
            if (sy)
                  sy->removeSoundFont(s);
            delete soundFonts->takeItem(row);
            }
      }

//---------------------------------------------------------
//   sfAddClicked
//---------------------------------------------------------

void SynthControl::sfAddClicked()
      {
      QString s(soundFont->text());
      if (!s.isEmpty()) {
            int n = soundFonts->count();
            QStringList sl;
            for (int i = 0; i < n; ++i) {
                  QListWidgetItem* item = soundFonts->item(i);
                  sl.append(item->text());
                  }
            if (sl.contains(s)) {
                  QMessageBox::warning(this,
                     tr("MuseScore"),
                     tr("Soundfont already loaded"));
                  }
            else {
                  Synth* sy = synti->synth("Fluid");
                  if (sy) {
                        bool loaded = sy->addSoundFont(s);
                        if (!loaded) {
                              QMessageBox::warning(this,
                                 tr("MuseScore"),
                                 tr("cannot load soundfont"));
                              }
                        else {
                              soundFonts->addItem(s);
                              }
                        // QListWidgetItem* item = soundFonts->item(soundFonts->count()-1);
                        // item->setCheckState(loaded ? Qt::Checked : Qt::Unchecked);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   gainChanged
//---------------------------------------------------------

void SynthControl::gainChanged(double val, int)
      {
      emit gainChanged(val);
      }

//---------------------------------------------------------
//   masterTuningChanged
//---------------------------------------------------------

void SynthControl::masterTuningChanged(double val)
      {
      synti->setMasterTuning(val);
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
      synti->setParameter(SParmId(FLUID_ID, 0, idx).val, val);
      }

//---------------------------------------------------------
//   chorusValueChanged
//---------------------------------------------------------

void SynthControl::chorusValueChanged(double val, int idx)
      {
      synti->setParameter(SParmId(FLUID_ID, 1, idx).val, val);
      }

//---------------------------------------------------------
//   setAeolusValue
//---------------------------------------------------------

void SynthControl::setAeolusValue(double val, int idx)
      {
      synti->setParameter(SParmId(AEOLUS_ID, idx >> 8, idx & 0xff).val, val);
      }

//---------------------------------------------------------
//   currentSoundFontChanged
//---------------------------------------------------------

void SynthControl::currentSoundFontChanged(int row)
      {
      int rows = soundFonts->count();
      soundFontUp->setEnabled(row > 0);
      soundFontDown->setEnabled(row < rows);
      soundFontDelete->setEnabled(row >= 0);
      }

//---------------------------------------------------------
//   sfUpClicked
//---------------------------------------------------------

void SynthControl::sfUpClicked()
      {
      int row  = soundFonts->currentRow();
      if (row <= 0)
            return;
      Synth* sy = synti->synth("Fluid");
      if (sy) {
            QStringList sfonts = sy->soundFonts();
            sfonts.swap(row, row-1);
            sy->loadSoundFonts(sfonts);
            soundFonts->clear();
            soundFonts->addItems(sfonts);
            }
      }

//---------------------------------------------------------
//   sfDownClicked
//---------------------------------------------------------

void SynthControl::sfDownClicked()
      {
      int rows = soundFonts->count();
      int row  = soundFonts->currentRow();
      if (row + 1 >= rows)
            return;

      Synth* sy = synti->synth("Fluid");
      if (sy) {
            QStringList sfonts = sy->soundFonts();
            sfonts.swap(row, row+1);
            sy->loadSoundFonts(sfonts);
            soundFonts->clear();
            soundFonts->addItems(sfonts);
            }
      }


