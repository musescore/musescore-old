//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp 3012 2010-04-28 17:12:41Z wschweer $
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

#include "config.h"
#include "preferences.h"
#include "event.h"
#include "instrument.h"
#include "synti.h"
#include "fluid/fluid.h"
#ifdef AEOLUS
#include "aeolus/aeolus/aeolus.h"
#endif

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

MasterSynth::MasterSynth()
      {
      _gain = 1.0;
      }

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

MasterSynth::~MasterSynth()
      {
      foreach(Synth* s, syntis)
            delete s;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MasterSynth::init(int sampleRate)
      {
      bool useJackFlag      = preferences.useJackAudio || preferences.useJackMidi;
      bool useAlsaFlag      = preferences.useAlsaAudio;
      bool usePortaudioFlag = preferences.usePortaudioAudio;

      if (useJackFlag || useAlsaFlag || usePortaudioFlag) {
            syntis.append(new FluidS::Fluid());
#ifdef AEOLUS
            syntis.append(new Aeolus());
#endif
            }
      if (syntis.isEmpty())
            return;
      foreach(Synth* s, syntis)
            s->init(sampleRate);
      QString p;
      if (!preferences.soundFont.isEmpty())
            p = preferences.soundFont;
      else
            p = QString(getenv("DEFAULT_SOUNDFONT"));
      if (p.isEmpty()) {
            QMessageBox::critical(0, QWidget::tr("MuseScore: Load SoundFont"),
               QWidget::tr("No SoundFont configured\n"
               "Playback will be disabled."));
            }
      else {
            if (debugMode)
                  printf("load soundfont <%s>\n", qPrintable(p));
            bool rv = syntis[0]->loadSoundFont(p);
            if (!rv) {
                  QString s = QWidget::tr("Loading SoundFont\n"
                     "\"%1\"\n"
                     "failed. Playback will be disabled.\n\n"
                     "Go to Display > Synthesizer \n"
                     "and check that the file location is correct").arg(p);
                  QMessageBox::critical(0, QWidget::tr("MuseScore: Load SoundFont"), s);
                  }
            }
      foreach(Synth* s, syntis) {
            s->setMasterTuning(preferences.tuning);
            s->setEffectParameter(0, 0, preferences.reverbRoomSize);
            s->setEffectParameter(0, 1, preferences.reverbDamp);
            s->setEffectParameter(0, 2, preferences.reverbWidth);
            s->setEffectParameter(0, 3, preferences.reverbGain);
            s->setEffectParameter(1, 4, preferences.chorusGain);
            }
      _gain = preferences.masterGain;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void MasterSynth::process(unsigned n, float* l, float* r, int stride)
      {
      foreach(Synth* s, syntis)
            s->process(n, l, r, stride, _gain);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void MasterSynth::play(const Event& event, int syntiIdx)
      {
      syntis[syntiIdx]->play(event);
      }

//---------------------------------------------------------
//   loadSoundFont
//---------------------------------------------------------

bool MasterSynth::loadSoundFont(const QString& s)
      {
      foreach(Synth* synti, syntis)
            synti->loadSoundFont(s);
      return true;
      }

//---------------------------------------------------------
//   soundFont
//---------------------------------------------------------

QString MasterSynth::soundFont() const
      {
      return "";
      }

//---------------------------------------------------------
//   synthNameToIndex
//---------------------------------------------------------

int MasterSynth::synthNameToIndex(const QString& name) const
      {
      int idx = 0;
      foreach(Synth* s, syntis) {
            if (s->name() == name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString MasterSynth::synthIndexToName(int idx) const
      {
      if (idx >= syntis.size())
            return QString();
      return QString(syntis[idx]->name());
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> MasterSynth::getPatchInfo() const
      {
      QList<MidiPatch*> pl;
      int idx = 0;
      foreach(Synth* s, syntis) {
            QList<MidiPatch*> ip = s->getPatchInfo();
            foreach(MidiPatch* mp, ip)
                  mp->synti = idx;
            pl += ip;
            ++idx;
            }
      return pl;
      }

//---------------------------------------------------------
//   getSynth
//---------------------------------------------------------

Synth* MasterSynth::getSynth(int n)
      {
      return syntis[n];
      }

//---------------------------------------------------------
//   getSyntis
//---------------------------------------------------------

const QList<Synth*>& MasterSynth::getSyntis() const
      {
      return syntis;
      }

