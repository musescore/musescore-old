//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: playpanel.cpp,v 1.15 2006/03/02 17:08:41 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "playpanel.h"
#include "sig.h"
#include "score.h"
#include "seq.h"

const int MIN_VOL = -60;
const int MAX_VOL = 10;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

PlayPanel::PlayPanel(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      setupUi(this);
      volumeSlider->setRange(MIN_VOL * 1000, MAX_VOL * 1000);
      posSlider->setRange(0, seq->getEndTick());
      tempoSlider->setValue(tempoSlider->maximum() + tempoSlider->minimum() - 100);

      int lineStep = (MAX_VOL - MIN_VOL) * 10;
      volumeSlider->setSingleStep(lineStep);
      volumeSlider->setPageStep(lineStep * 10);
      volumeSlider->setInvertedAppearance(true);  // cannot be set from designer

      connect(volumeSlider, SIGNAL(sliderMoved(int)), SLOT(volumeChanged(int)));
      connect(posSlider,    SIGNAL(sliderMoved(int)), SLOT(posChanged(int)));

      connect(rewindButton, SIGNAL(clicked()),        SIGNAL(rewindTriggered()));
      connect(stopButton,   SIGNAL(toggled(bool)),    SIGNAL(stopToggled(bool)));
      connect(playButton,   SIGNAL(toggled(bool)),    SIGNAL(playToggled(bool)));
      connect(tempoSlider,  SIGNAL(sliderMoved(int)), SIGNAL(relTempoChanged(int)));
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void PlayPanel::closeEvent(QCloseEvent* ev)
      {
      emit closed();
      QWidget::closeEvent(ev);
      }

//---------------------------------------------------------
//   setEndpos
//---------------------------------------------------------

void PlayPanel::setEndpos(int val)
      {
      posSlider->setRange(0, val);
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void PlayPanel::setTempo(int val)
      {
      int tempo = int(60000000.0 / double(val));
      tempoLabel->setText(QString("%1 bpm").arg(tempo, 3));
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void PlayPanel::setRelTempo(int val)
      {
      relTempo->setText(QString("%1 %").arg(val, 3));
      tempoSlider->setValue(val);
      }

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void PlayPanel::setVolume(float val)
      {
      int vol = int(log10(val)*20000.0);
      volumeSlider->setValue((MAX_VOL + MIN_VOL)*1000 - vol);
      }

//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void PlayPanel::volumeChanged(int val)
      {
      float volume;
      val = (MAX_VOL + MIN_VOL) * 1000 - val;

      if (val <= MIN_VOL * 1000)
            volume = 0.0;
      else {
            float fval = float(val)/20000.0;
            volume = pow(10.0, fval);
            }
      emit volChange(volume);
      }

//---------------------------------------------------------
//   posChanged
//---------------------------------------------------------

void PlayPanel::posChanged(int val)
      {
      emit posChange(val);
      }

//---------------------------------------------------------
//   setStop
//---------------------------------------------------------

void PlayPanel::setStop(bool val)
      {
      stopButton->blockSignals(true);
      stopButton->setChecked(val);
      stopButton->blockSignals(false);
      }

//---------------------------------------------------------
//   setPlay
//---------------------------------------------------------

void PlayPanel::setPlay(bool val)
      {
      playButton->blockSignals(true);
      playButton->setChecked(val);
      playButton->blockSignals(false);
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PlayPanel::heartBeat(int tickpos)
      {
      int bar, beat, tick;
      cs->sigmap->tickValues(tickpos, &bar, &beat, &tick);
      char buffer[32];
      sprintf(buffer, "%03d.%02d", bar+1, beat+1);
      posLabel->setText(QString(buffer));
      posSlider->setValue(tickpos);
      }

//---------------------------------------------------------
//   enableSeek
//---------------------------------------------------------

void PlayPanel::enableSeek(bool val)
      {
      posSlider->setEnabled(val);
      }

