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

#include <stdio.h>

#include "voice.h"
#include "instrument.h"
#include "channel.h"
#include "msynth.h"

float Voice::interpCoeff[INTERP_MAX][4];

//---------------------------------------------------------
//   set
//---------------------------------------------------------

void Envelope::set(float start, float end, int steps)
      {
      val = start;
      count = steps;
      increment = (end - start) / steps;
      }

void Envelope::setTime(float start, float end, float ms)
      {
      val       = start;
      count     = int(ms * MSynth::samplerate() / 1000);
      increment = (end - start) / count;
      }

//---------------------------------------------------------
//   init
//---------------------------------------------------------

void Voice::init()
      {
      // Initialize the coefficients for the interpolation. The math comes
      // from a mail, posted by Olli Niemitalo to the music-dsp mailing
      // list (I found it in the music-dsp archives
      // http://www.smartelectronix.com/musicdsp/).

      for (int i = 0; i < INTERP_MAX; i++) {
            double x = (double) i / (double) INTERP_MAX;
            interpCoeff[i][0] = (float)(x * (-0.5 + x * (1 - 0.5 * x)));
            interpCoeff[i][1] = (float)(1.0 + x * x * (1.5 * x - 2.5));
            interpCoeff[i][2] = (float)(x * (0.5 + x * (2.0 - 1.5 * x)));
            interpCoeff[i][3] = (float)(0.5 * x * x * (x - 1.0));
            }
      }

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

Voice::Voice()
      {
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void Voice::stop()
      {
      _state            = VOICE_STOP;
      stopEnv.setTime(1.0, 0.0, 100.0);       // 100 ms decay
      }

//---------------------------------------------------------
//   sustained
//---------------------------------------------------------

void Voice::sustained()
      {
      _state = VOICE_SUSTAINED;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void Voice::start(Channel* c, int key, int v, float t)
      {
      _channel      = c;
      _key          = key;
      _velocity     = v;
      _tuning       = t;
      _state        = VOICE_PLAYING;
      Instrument* i = _channel->instrument();
      zone          =  i->zone(_key, _velocity);
      sample        = zone->sample();
      data          = sample->data();
      eidx          = sample->frames();
      stopEnv.val   = 1.0;
      double pi     = MSynth::ct2hz(key * 100.0)/MSynth::ct2hz(zone->keyBase() * 100.0);
      phaseIncr.set(pi);
      phase.set(0);
      amp           = MSynth::cb2amp((128-_velocity) * 3) * .4;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Voice::process(int frames, float* lb, float* rb, int stride)
      {
      while (frames) {
            int idx = phase.index();
            if (idx >= eidx) {
                  MSynth* s = _channel->msynth();
                  s->stopVoice(this);
                  break;
                  }
            float* coeffs = interpCoeff[phase.fract()];

            *lb   += ( coeffs[0] * data[(idx-1)*2]
                     + coeffs[1] * data[(idx+0)*2]
                     + coeffs[2] * data[(idx+1)*2]
                     + coeffs[3] * data[(idx+2)*2])
                     * stopEnv.val * amp;
            *rb   += ( coeffs[0] * data[(idx-1)*2+1]
                     + coeffs[1] * data[(idx+0)*2+1]
                     + coeffs[2] * data[(idx+1)*2+1]
                     + coeffs[3] * data[(idx+2)*2+1])
                     * stopEnv.val * amp;

            phase += phaseIncr;

            --frames;
            lb += stride;
            rb += stride;
            if (_state == VOICE_STOP) {
                  stopEnv.step();
                  if (!stopEnv.count) {
                        MSynth* s = _channel->msynth();
                        s->stopVoice(this);
                        break;
                        }
                  }
            }
      }

