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

#include "playpanel.h"
#include "al/sig.h"
#include "score.h"
#include "seq.h"
#include "mscore.h"
#include "measure.h"

const int MIN_VOL = -60;
const int MAX_VOL = 10;

//---------------------------------------------------------
//   PlayPanel
//---------------------------------------------------------

PlayPanel::PlayPanel(QWidget* parent)
   : QWidget(parent, Qt::Dialog)
      {
      cachedTickPosition = -1;
      cachedTimePosition = -1;
      cs                 = 0;
      setupUi(this);
      setScore(0);

      playButton->setDefaultAction(getAction("play"));
      rewindButton->setDefaultAction(getAction("rewind"));

      connect(volumeSlider, SIGNAL(valueChanged(double,int)), SLOT(volumeChanged(double,int)));
      connect(posSlider,    SIGNAL(sliderMoved(int)),         SLOT(setPos(int)));
      connect(tempoSlider,  SIGNAL(valueChanged(double,int)), SIGNAL(relTempoChanged(double,int)));
      connect(swingStyle,   SIGNAL(currentIndexChanged(int)), SLOT(swingStyleChanged(int)));
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
//   setScore
//---------------------------------------------------------

void PlayPanel::setScore(Score* s)
      {
      if (cs != 0 && cs == s)
            return;
      if (cs)
            disconnect(cs, SIGNAL(posChanged(int)), this, SLOT(setPos(int)));
      cs = s;
      if (cs)
            connect(cs, SIGNAL(posChanged(int)), this, SLOT(setPos(int)));
      if (cs) {
            MeasureBase* lm = cs->last();
            if (lm)
                  setEndpos(lm->tick() + lm->tickLen());
            }
      bool enable = (cs != 0);
      volumeSlider->setEnabled(enable);
      posSlider->setEnabled(enable);
      tempoSlider->setEnabled(enable);
      swingStyle->setEnabled(enable);
      if (cs) {
            setTempo(cs->tempomap()->tempo(0));
            setRelTempo(cs->tempomap()->relTempo());
            Measure* m = cs->lastMeasure();
            if (m)      
                  setEndpos(m ? m->tick() + m->tickLen() : 0);
            }
      else {
            setTempo(120.0);
            setRelTempo(100);
            setEndpos(0);
            }
      heartBeat2(seq->getCurTime());
      int tick, utick;
      seq->getCurTick(&tick, &utick);
      heartBeat(tick, utick);
      update();
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

void PlayPanel::setTempo(double val)
      {
      int tempo = lrint(val * 60.0);
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
      volumeSlider->setValue(val);
      }

//---------------------------------------------------------
//   volumeChanged
//---------------------------------------------------------

void PlayPanel::volumeChanged(double val, int)
      {
      emit volChange(val);
      }

//---------------------------------------------------------
//   setPos
//---------------------------------------------------------

void PlayPanel::setPos(int tick)
      {
      if (cachedTickPosition != tick)
            emit posChange(tick);
      heartBeat(tick, tick);
      }

//---------------------------------------------------------
//   swingStyleChanged
//---------------------------------------------------------

void PlayPanel::swingStyleChanged(int index)
      {
      if (cs == 0)
            return;
      switch (index){
            case 0:
                  cs->setSwingRatio(0);
                  break;
            case 1:
                  cs->setSwingRatio(0.333);
                  break;
            case 2:
                  cs->setSwingRatio(0.5);
                  break;
            }
      if (seq->isRunning()) {
            if (seq->isStopped()) {
                  seq->collectEvents();
                  }
            else {
                  seq->guiStop(); // stop
                  seq->start(); // start
                  }
            }
      }

//---------------------------------------------------------
//   heartBeat
//---------------------------------------------------------

void PlayPanel::heartBeat(int tick, int utick)
      {
      if (cachedTickPosition == utick)
            return;
      if (cs == 0)
            return;
      cachedTickPosition = utick;

      int bar, beat, t;
      cs->sigmap()->tickValues(tick, &bar, &beat, &t);

      char buffer[32];
      sprintf(buffer, "%03d.%02d", bar+1, beat+1);
      posLabel->setText(QString(buffer));
      posSlider->setValue(utick);
      }

//---------------------------------------------------------
//   heartBeat2
//---------------------------------------------------------

void PlayPanel::heartBeat2(int sec)
      {
      if (sec == cachedTimePosition)
            return;
      cachedTimePosition = sec;
      int m              = sec / 60;
      sec                = sec % 60;
      int h              = m / 60;
      m                  = m % 60;
      char buffer[32];
      sprintf(buffer, "%d:%02d:%02d", h, m, sec);
      timeLabel->setText(QString(buffer));
      }

