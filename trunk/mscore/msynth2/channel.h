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

#ifndef __MCHANNEL_H__
#define __MCHANNEL_H__

class MSynth;
class Instrument;

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel {
      MSynth* _msynth;
      Instrument* _instrument;
      int _idx;
      int _sustain;

   public:
      Channel(MSynth*, int idx);
      void pitchBend(int);
      void controller(int ctrl, int val);
      Instrument* instrument() const { return _instrument; }
      MSynth* msynth() const { return _msynth; }
      int sustain() const { return _sustain; }
      };


#endif

