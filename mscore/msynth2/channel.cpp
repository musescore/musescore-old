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
#include "channel.h"
#include "event.h"
#include "voice.h"

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel(MSynth* ms, int i)
      {
      _msynth  = ms;
      _idx     = i;
      _instrument = ms->instrument(0);
      _sustain = 0;
      }

//---------------------------------------------------------
//   pitchBend
//---------------------------------------------------------

void Channel::pitchBend(int)
      {
      }

//---------------------------------------------------------
//   controller
//---------------------------------------------------------

void Channel::controller(int ctrl, int val)
      {
      if (ctrl == CTRL_SUSTAIN) {
            if (_sustain != val) {
                  _sustain = val;
                  if (_sustain < 0x40) {
                        foreach(Voice* v, _msynth->activeVoices()) {
                              if (v->stateSustained())
                                    v->stop();
                              }
                        }
                  }
            }
      }

