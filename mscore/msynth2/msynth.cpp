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

#include "msynth.h"
#include "event.h"
#include "voice.h"
#include "channel.h"
#include "instrument.h"

#include <stdio.h>
#include <assert.h>

static const int CB_AMP_SIZE = 961;
static float cb2ampTab[CB_AMP_SIZE];

bool MSynth::initialized = false;
int MSynth::_samplerate = 44100;

//---------------------------------------------------------
//   MSynth
//---------------------------------------------------------

MSynth::MSynth()
      {
      if (!initialized)
            init();
      reverb      = new Effect;   // dummy
      chorus      = new Effect;   // dummy
      silentBlocks = 0;
      }

//---------------------------------------------------------
//   ~MSynth
//---------------------------------------------------------

MSynth::~MSynth()
      {
      delete reverb;
      delete chorus;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void MSynth::process(int frames, float* l, float* r, int stride)
      {
      const int bytes = frames * sizeof(float);
      memset(l, 0, bytes);
      memset(r, 0, bytes);

      if (mutex.tryLock()) {
            if (_activeVoices.isEmpty())
                  --silentBlocks;
            else {
                  silentBlocks = SILENT_BLOCKS;
                  foreach(Voice* v, _activeVoices)
                        v->process(frames, l, r, stride);
                  }
            if (silentBlocks > 0) {
                  reverb->process(frames);
                  chorus->process(frames);
                  }
            mutex.unlock();
            }
      }

//---------------------------------------------------------
//   programChange
//---------------------------------------------------------

void MSynth::programChange(int channel, int program)
      {

      }

//---------------------------------------------------------
//   noteOn
//---------------------------------------------------------

void MSynth::noteOn(Channel* channel, int key, int velocity, float tuning)
      {
      Voice* voice;
      if (freeVoices.isEmpty())
            voice = new Voice;
      else
            voice = freeVoices.takeLast();
      voice->start(channel, key, velocity, tuning);
      _activeVoices.append(voice);
      }

//---------------------------------------------------------
//   stopVoice
//---------------------------------------------------------

void MSynth::stopVoice(Voice* v)
      {
      _activeVoices.removeOne(v);
      freeVoices.append(v);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void MSynth::play(const Event& event)
      {
      bool err       = false;
      int channelIdx = event.channel();

      if (channelIdx >= channel.size()) {
            for (int i = channel.size(); i < channelIdx + 1; i++)
                  channel.append(new Channel(this, i));
            }

      int type    = event.type();
      Channel* cp = channel[channelIdx];

      if (type == ME_NOTEON) {
            int key = event.dataA();
            int vel = event.dataB();
            if (vel == 0) {
                  //
                  // process note off
                  //
                  foreach (Voice* v, _activeVoices) {
                        if ((v->channel() == cp) && (v->key() == key)) {
                              if (cp->sustain() > 0x40)
                                    v->sustained();
                              else
                                    v->stop();
                              }
                        }
                  return;
                  }
            foreach(Voice* v, _activeVoices) {
                  if (v->statePlaying() && (v->channel() == cp) && (v->key() == key)) {
                        // this can not happen on a real device
printf("retrigger...\n");
                        v->stop();
                        }
                  }
            noteOn(cp, key, vel, event.tuning());
            }

      else if (type == ME_CONTROLLER)  {
            switch(event.controller()) {
                  case CTRL_PROGRAM:
                        programChange(channelIdx, event.value());
                        break;
                  case CTRL_PITCH:
                        cp->pitchBend(event.value());
                        break;
                  default:
                        cp->controller(event.controller(), event.value());
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   loadInstrument
//    return true on success
//---------------------------------------------------------

bool MSynth::loadInstrument(const char* path)
      {
      Instrument* instr = new Instrument(this);
      if (instr->load(path)) {
            instruments.append(instr);
            return true;
            }
      delete instr;
      return false;
      }

//---------------------------------------------------------
//   instrument
//---------------------------------------------------------

Instrument* MSynth::instrument(int program) const
      {
      foreach(Instrument* i, instruments) {
            if (i->program() == program)
                  return i;
            }
      printf("instrument for program %d not found\n", program);
      return 0;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void MSynth::init()
      {
      initialized = true;
      Voice::init();

      for (int i = 0; i < CB_AMP_SIZE; ++i)
            cb2ampTab[i] = (float) pow(10.0, (double) i / -200.0);
      }

//---------------------------------------------------------
//   cb2amp
//    convert centibel value 0-960 into
//    attenuation (1.0 - 0.0)
//---------------------------------------------------------

float MSynth::cb2amp(int cb)
      {
      if (cb < 0)
            return 1.0;
      if (cb >= CB_AMP_SIZE)
            return 0.0;
      return cb2ampTab[cb];
      }


