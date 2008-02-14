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

#include "preferences.h"
#include "fluid.h"
#include "score.h"
#include "mscore.h"
#include "seq.h"

//---------------------------------------------------------
//   ISynth
//---------------------------------------------------------

ISynth::ISynth()
      {
      _fluidsynth = 0;
      sfont       = 0;
      fontId      = -1;
      fluid_font  = 0;
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool ISynth::init(int sampleRate)
      {
      fluid_settings_t* settings = new_fluid_settings();
      fluid_settings_setnum(settings, "synth.sample-rate", float(sampleRate));

      _fluidsynth = new_fluid_synth(settings);

      QString p;
      if (!preferences.soundFont.isEmpty())
            p = preferences.soundFont;
      else
            p = QString(getenv("DEFAULT_SOUNDFONT"));
      if (p.isEmpty()) {
            //
            // fallback to integrated soundfont
            //
            p = ":/data/piano1.sf2";
            }
      return loadSoundFont(p);
      }

//---------------------------------------------------------
//   loadSoundFont
//    return false on error;
//---------------------------------------------------------

bool ISynth::loadSoundFont(const QString& sfont)
      {
      if (fontId != -1)
            fluid_synth_sfunload(_fluidsynth, fontId, true);
#ifdef USE_GLOBAL_FLUID
      fontId = fluid_synth_sfload(_fluidsynth, qPrintable(sfont), true);
#else
      fontId = fluid_synth_sfload(_fluidsynth, sfont, true);
#endif
      if (fontId == -1) {
            fprintf(stderr, "ISynth: %s", fluid_synth_error(_fluidsynth));
            return false;
            }
      fluid_synth_set_gain(_fluidsynth, 0.2);
      return true;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void ISynth::process(unsigned n, float* l, float* r, int stride)
      {
#ifdef USE_GLOBAL_FLUID
      fluid_synth_write_float(_fluidsynth, n, l, 0, 1, r, 0, 1);
#else
      fluid_synth_write_float(_fluidsynth, n, l, r, stride);
#endif
      }

//---------------------------------------------------------
//   playNote
//---------------------------------------------------------

void ISynth::playNote(int channel, int pitch, int velo)
      {
// printf("note %d %d %d\n", channel, pitch, velo);

      int err;
      if (velo) {
            err = fluid_synth_noteon(_fluidsynth, channel, pitch, velo);
            if (err)
                  fprintf(stderr, "FluidSynth: error %d note on, ch %d pitch %d velo %d: %s\n",
                     err, channel, pitch, velo, fluid_synth_error(_fluidsynth));
            }
      else {
            err = fluid_synth_noteoff(_fluidsynth, channel, pitch);
            if (err)
                  fprintf(stderr, "FluidSynth: error %d note off, ch %d pitch %d: %s\n",
                     err, channel, pitch, fluid_synth_error(_fluidsynth));
            }
      }

//---------------------------------------------------------
//   setController
//    return true if busy
//---------------------------------------------------------

bool ISynth::setController(int ch, int ctrl, int val)
      {
      if (_fluidsynth == 0)
            return false;
      switch(ctrl) {
            case CTRL_PROGRAM:
                  {
                  int hbank = (val & 0xff0000) >> 16;
                  int lbank = (val & 0xff00) >> 8;
                  if (hbank > 127)  // map "dont care" to 0
                        hbank = 0;
                  if (lbank > 127)
                        lbank = 0;
                  if (lbank == 127 || ch == 9)       // drum HACK
                        lbank = 128;
                  int prog  = val & 0x7f;

                  fluid_synth_program_select(_fluidsynth, ch,
                      fontId, lbank, prog);
                  }
                  break;

            case CTRL_PITCH:
                  fluid_synth_pitch_bend (_fluidsynth, ch, val);
                  break;

            default:
                  fluid_synth_cc(_fluidsynth, ch, ctrl & 0x3fff, val);
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const MidiPatch* ISynth::getPatchInfo(int ch, const MidiPatch* p) const
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
            patch.hbank = fluid_sfont_get_id(fluid_font);
            int bank = (*preset.get_banknum)(&preset);
            if (ch == 9 && bank != 128) // show only drums for channel 10
                  continue;
            if (bank == 128)
                  bank = 127;
            patch.typ   = 0;
            patch.name  = (*preset.get_name)(&preset);
            patch.lbank = bank;
            patch.prog  = (*preset.get_num)(&preset);
            return &patch;
            }
      return 0;
      }

