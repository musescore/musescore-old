//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: fluid.cpp,v 1.12 2006/07/28 16:34:36 wschweer Exp $
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

#ifdef USE_GLOBAL_FLUID

#include "preferences.h"
#include "fluid.h"
#include "score.h"
#include "mscore.h"
#include "seq.h"

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

Fluid::Fluid()
      {
      _fluidsynth = 0;
      fluid_font = 0;
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

void Fluid::init(int sampleRate, int channels)
      {
      fluid_settings_t* settings = new_fluid_settings();
      fluid_settings_setnum(settings, "synth.sample-rate", float(sampleRate));
      fluid_settings_setint(settings, "synth.midi-channels", channels);
      fluid_settings_setint(settings, "synth.audio-channels", 1);

      _fluidsynth = new_fluid_synth(settings);
      }

//---------------------------------------------------------
//   loadSoundFont
//    return false on error;
//---------------------------------------------------------

bool Fluid::loadSoundFont(const QString& sfont)
      {
      int fontId = fluid_synth_sfload(_fluidsynth, qPrintable(sfont), true);
      if (fontId == -1) {
            fprintf(stderr, "Fluid: %s\n", fluid_synth_error(_fluidsynth));
            return false;
            }
      fluid_synth_set_gain(_fluidsynth, 0.2);
      return true;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Fluid::process(unsigned n, float* l, float* r, int stride)
      {
      fluid_synth_write_float(_fluidsynth, n, l, 0, 1, r, 0, 1);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Fluid::play(const MidiOutEvent& e)
      {
      int ch      = e.type & 0xf;
      int channel = e.port * 16 + ch;
// printf("note %d %d %d\n", channel, pitch, velo);

      int err = 0;
      switch(e.type & 0xf0) {
            case ME_NOTEON:
                  if (e.b == 0) {
                        err = fluid_synth_noteoff(_fluidsynth, channel, e.a);
                        if (midiOutputTrace)
                              printf("MidiOut: %2d:%2d NoteOff %3d\n", e.port, ch, e.a);
                        if (channel == 9)       // hack: drum noteoff
                              err = 0;
                        }
                  else {
                        err = fluid_synth_noteon(_fluidsynth, channel, e.a, e.b);
                        if (midiOutputTrace)
                              printf("MidiOut: %2d:%2d NoteOn  %3d %3d\n", e.port, ch, e.a, e.b);
                        }
                  break;
            case ME_CONTROLLER:
                  fluid_synth_cc(_fluidsynth, channel, e.a, e.b);
                  if (midiOutputTrace)
                        printf("MidiOut: %2d:%2d Ctrl    %3d %3d\n", e.port, ch, e.a, e.b);
                  break;

            case ME_PROGRAM:
                  // if e.b == 1 then a drum instrument is requested and we set lbank to 128
                  err = fluid_synth_program_change(_fluidsynth, channel, e.a);
                  if (midiOutputTrace)
                        printf("MidiOut: %2d:%2d Prog %3d\n", e.port, ch, e.a);
                  break;
            }

      if (err)
            fprintf(stderr, "FluidSynth error: event 0x%2x channel %d dataA %d dataB %d: %s\n",
               e.type & 0xff, channel, e.a, e.b, fluid_synth_error(_fluidsynth));
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const MidiPatch* Fluid::getPatchInfo(bool onlyDrums, const MidiPatch* p) const
      {
      if (_fluidsynth == 0)
            return 0;
      if (p == 0) {
            // get font at font stack index 0
            fluid_font = fluid_synth_get_sfont(_fluidsynth, 0);
            if (fluid_font == 0)
                  return 0;
            (*fluid_font->iteration_start)(fluid_font);
            }
      fluid_preset_t preset;

      while ((*fluid_font->iteration_next)(fluid_font, &preset)) {
            patch.bank = fluid_sfont_get_id(fluid_font);
            int bank    = (*preset.get_banknum)(&preset);
            if (onlyDrums && bank != 128)
                  continue;
            patch.name  = (*preset.get_name)(&preset);
            patch.bank = bank;
            patch.prog  = (*preset.get_num)(&preset);
            patch.drum  = onlyDrums;
            return &patch;
            }
      return 0;
      }
#endif

