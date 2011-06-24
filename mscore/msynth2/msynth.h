//=============================================================================
//  MuseSynth
//  Music Software Synthesizer
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENSE.GPL
//=============================================================================

#ifndef __MSYNTH_H__
#define __MSYNTH_H__

#include <math.h>
#include <QtCore/QMutex>
#include <QtCore/QList>

class Event;
class Voice;
class MSynth;
class Channel;
class Instrument;

//---------------------------------------------------------
//   Effect
//---------------------------------------------------------

class Effect {

   public:
      Effect() {}
      virtual void process(int frames) {}
      };

//---------------------------------------------------------
//   MSynth
//---------------------------------------------------------

class MSynth {
      static const int SILENT_BLOCKS = 32*5;

      static bool initialized;
      static int _samplerate;
      QMutex mutex;
      int silentBlocks;

      QList<Instrument*> instruments;
      QList<Channel*> channel;
      QList<Voice*> freeVoices;
      QList<Voice*> _activeVoices;
      Effect* reverb;
      Effect* chorus;

      void programChange(int channel, int program);
      void noteOn(Channel*, int key, int velocity, float tuning);
      static void init();

   public:
      MSynth();
      ~MSynth();
      static int samplerate()     { return _samplerate; }
      void setSamplerate(int val) { _samplerate = val; }

      void process(int frames, float* l, float* r, int stride);
      void play(const Event& event);
      bool loadInstrument(const char*);

      Instrument* instrument(int program) const;
      void stopVoice(Voice* v);

      const QList<Voice*>& activeVoices() { return _activeVoices; }

      static float act2hz(float c)    { return 8.176 * pow(2.0, (double)c / 1200.0); }
      static float ct2hz(float cents) { return act2hz(qBound(1500.0f, cents, 13500.0f)); }
      static float cb2amp(int);
      };

#endif

