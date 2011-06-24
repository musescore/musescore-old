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

#ifndef __MVOICE_H__
#define __MVOICE_H__

#include <QtCore/QtGlobal>
#include <math.h>

class Channel;
class Zone;
class Sample;

static const int INTERP_MAX = 256;

//---------------------------------------------------------
//   Envelope
//---------------------------------------------------------

struct Envelope {
      float increment;
      float val;
      int count;

      void step() {
            if (count) {
                  --count;
                  val += increment;
                  }
            }
      void set(float start, float end, int steps);
      void setTime(float start, float end, float ms);
      };

//-----------------------------------------------------------------------------
//   Phase
//    Playing pointer for voice playback
//
//    When a sample is played back at a different pitch, the playing pointer
//    in the source sample will not advance exactly one sample per output sample.
//
//    This playing pointer is implemented using Phase.
//-----------------------------------------------------------------------------

struct Phase {
      int data;

      void operator+=(const Phase& p) { data += p.data;       }
      void set(int b)                 { data = b << 8;        }
      void set(double b)              { data = b * 256.0;     }
      int index() const               { return data >> 8;     }
      unsigned fract() const          { return data & 0xff;   }

      Phase() {}
      Phase(qint64 v) : data(v) {}
      };

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice {
      enum { VOICE_OFF, VOICE_PLAYING, VOICE_SUSTAINED, VOICE_STOP };

      int _state;
      Channel* _channel;
      int _key;
      int _velocity;
      float _tuning;
      Zone* zone;

      float amp;

      Sample* sample;
      Phase phase;
      Phase phaseIncr;

      float* data;
      int eidx;
      Envelope stopEnv;

      static float interpCoeff[INTERP_MAX][4];

   public:
      Voice();
      void start(Channel* channel, int key, int velocity, float tuning);

      void process(int frames, float* l, float* r, int stride);
      void stop();
      void sustained();
      bool statePlaying() const   { return _state == VOICE_PLAYING;   }
      bool stateSustained() const { return _state == VOICE_SUSTAINED; }
      Channel* channel() const    { return _channel; }
      int key() const             { return _key;     }

      static void init();
      };

#endif

