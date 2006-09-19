//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: fluid.h,v 1.6 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __FLUID_H__
#define __FLUID_H__

#include "config.h"
#include "synti.h"

#ifdef USE_FLUID
#include <fluidsynth.h>

//---------------------------------------------------------
//   ISynth
//---------------------------------------------------------

class ISynth : public Synth {
      _fluid_synth_t* _fluidsynth;

      mutable fluid_sfont_t* fluid_font;
      mutable MidiPatch patch;

      char* sfont;
      int fontId;

   public:
      ISynth();
      ~ISynth() {}
      virtual bool init(int sampleRate);
      virtual bool loadSoundFont(const char*);
      virtual void process(unsigned, float*, float*);
      virtual void playNote(int channel, int pitch, int velo);
      virtual bool setController(int ch, int ctrl, int val);
      virtual const MidiPatch* getPatchInfo(int ch, const MidiPatch* p) const;
      };

#else
//---------------------------------------------------------
//   ISynth
//---------------------------------------------------------

class ISynth : public Synth {

   public:
      ISynth() {}
      ~ISynth() {}
      virtual bool init(int) {
            fprintf(stderr, "no synth available\n");
            return true;
            }
      virtual bool loadSoundFont(const char*) { return true; }
      virtual void process(unsigned, float*, float*) {}
      virtual void playNote(int, int, int) {}
      virtual bool setController(int, int, int) { return true;}
      virtual const MidiPatch* getPatchInfo(int, const MidiPatch*) const { return 0; }
      };

#endif
#endif


