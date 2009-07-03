/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License
 * as published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the Free
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
 * 02111-1307, USA
 */

#include <math.h>

#include "event.h"
#include "fluid.h"
#include "sfont.h"
#include "conv.h"
#include "gen.h"
#include "chorus.h"
#include "voice.h"

namespace FluidS {

/***************************************************************
 *
 *                         GLOBAL
 */

bool Fluid::initialized = false;

/* default modulators
 * SF2.01 page 52 ff:
 *
 * There is a set of predefined default modulators. They have to be
 * explicitly overridden by the sound font in order to turn them off.
 */

Mod default_vel2att_mod;        /* SF2.01 section 8.4.1  */
Mod default_vel2filter_mod;     /* SF2.01 section 8.4.2  */
Mod default_at2viblfo_mod;      /* SF2.01 section 8.4.3  */
Mod default_mod2viblfo_mod;     /* SF2.01 section 8.4.4  */
Mod default_att_mod;            /* SF2.01 section 8.4.5  */
Mod default_pan_mod;            /* SF2.01 section 8.4.6  */
Mod default_expr_mod;           /* SF2.01 section 8.4.7  */
Mod default_reverb_mod;         /* SF2.01 section 8.4.8  */
Mod default_chorus_mod;         /* SF2.01 section 8.4.9  */
Mod default_pitch_bend_mod;     /* SF2.01 section 8.4.10 */

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

Fluid::Fluid()
      {
      left_buf   = new float[FLUID_BUFSIZE];
      right_buf  = new float[FLUID_BUFSIZE];
      fx_buf[0]  = new float[FLUID_BUFSIZE];
      fx_buf[1]  = new float[FLUID_BUFSIZE];
      reverb     = 0;
      chorus     = 0;
      tuning     = 0;
      }

//---------------------------------------------------------
//   init
//    static initialization
//---------------------------------------------------------

void Fluid::init()
      {
      initialized = true;
      fluid_conversion_config();
      Voice::dsp_float_config();

      /* SF2.01 page 53 section 8.4.1: MIDI Note-On Velocity to Initial Attenuation */
      fluid_mod_set_source1(&default_vel2att_mod, /* The modulator we are programming here */
		       FLUID_MOD_VELOCITY,    /* Source. VELOCITY corresponds to 'index=2'. */
		       FLUID_MOD_GC           /* Not a MIDI continuous controller */
		       | FLUID_MOD_CONCAVE    /* Curve shape. Corresponds to 'type=1' */
		       | FLUID_MOD_UNIPOLAR   /* Polarity. Corresponds to 'P=0' */
		       | FLUID_MOD_NEGATIVE   /* Direction. Corresponds to 'D=1' */
		       );
      fluid_mod_set_source2(&default_vel2att_mod, 0, 0); /* No 2nd source */
      fluid_mod_set_dest(&default_vel2att_mod, GEN_ATTENUATION);  /* Target: Initial attenuation */
      fluid_mod_set_amount(&default_vel2att_mod, 960.0);          /* Modulation amount: 960 */



     /* SF2.01 page 53 section 8.4.2: MIDI Note-On Velocity to Filter Cutoff
      * Have to make a design decision here. The specs don't make any sense this way or another.
      * One sound font, 'Kingston Piano', which has been praised for its quality, tries to
      * override this modulator with an amount of 0 and positive polarity (instead of what
      * the specs say, D=1) for the secondary source.
      * So if we change the polarity to 'positive', one of the best free sound fonts works...
      */
      fluid_mod_set_source1(&default_vel2filter_mod, FLUID_MOD_VELOCITY, /* Index=2 */
		       FLUID_MOD_GC                        /* CC=0 */
		       | FLUID_MOD_LINEAR                  /* type=0 */
		       | FLUID_MOD_UNIPOLAR                /* P=0 */
                       | FLUID_MOD_NEGATIVE                /* D=1 */
		       );
      fluid_mod_set_source2(&default_vel2filter_mod, FLUID_MOD_VELOCITY, /* Index=2 */
		       FLUID_MOD_GC                                 /* CC=0 */
		       | FLUID_MOD_SWITCH                           /* type=3 */
		       | FLUID_MOD_UNIPOLAR                         /* P=0 */
		       // do not remove       | FLUID_MOD_NEGATIVE                         /* D=1 */
		       | FLUID_MOD_POSITIVE                         /* D=0 */
		       );
      fluid_mod_set_dest(&default_vel2filter_mod, GEN_FILTERFC);        /* Target: Initial filter cutoff */
      fluid_mod_set_amount(&default_vel2filter_mod, -2400);

      /* SF2.01 page 53 section 8.4.3: MIDI Channel pressure to Vibrato LFO pitch depth */
      fluid_mod_set_source1(&default_at2viblfo_mod, FLUID_MOD_CHANNELPRESSURE, /* Index=13 */
		       FLUID_MOD_GC                        /* CC=0 */
		       | FLUID_MOD_LINEAR                  /* type=0 */
		       | FLUID_MOD_UNIPOLAR                /* P=0 */
		       | FLUID_MOD_POSITIVE                /* D=0 */
		       );
      fluid_mod_set_source2(&default_at2viblfo_mod, 0,0); /* no second source */
      fluid_mod_set_dest(&default_at2viblfo_mod, GEN_VIBLFOTOPITCH);        /* Target: Vib. LFO => pitch */
      fluid_mod_set_amount(&default_at2viblfo_mod, 50);

      /* SF2.01 page 53 section 8.4.4: Mod wheel (Controller 1) to Vibrato LFO pitch depth */
      fluid_mod_set_source1(&default_mod2viblfo_mod, 1, /* Index=1 */
		       FLUID_MOD_CC                        /* CC=1 */
		       | FLUID_MOD_LINEAR                  /* type=0 */
		       | FLUID_MOD_UNIPOLAR                /* P=0 */
		       | FLUID_MOD_POSITIVE                /* D=0 */
		       );
      fluid_mod_set_source2(&default_mod2viblfo_mod, 0,0); /* no second source */
      fluid_mod_set_dest(&default_mod2viblfo_mod, GEN_VIBLFOTOPITCH);        /* Target: Vib. LFO => pitch */
      fluid_mod_set_amount(&default_mod2viblfo_mod, 50);

      /* SF2.01 page 55 section 8.4.5: MIDI continuous controller 7 to initial attenuation*/
      fluid_mod_set_source1(&default_att_mod, 7,                     /* index=7 */
		       FLUID_MOD_CC                              /* CC=1 */
		       | FLUID_MOD_CONCAVE                       /* type=1 */
		       | FLUID_MOD_UNIPOLAR                      /* P=0 */
		       | FLUID_MOD_NEGATIVE                      /* D=1 */
		       );
      fluid_mod_set_source2(&default_att_mod, 0, 0);                 /* No second source */
      fluid_mod_set_dest(&default_att_mod, GEN_ATTENUATION);         /* Target: Initial attenuation */
      fluid_mod_set_amount(&default_att_mod, 960.0);                 /* Amount: 960 */

      /* SF2.01 page 55 section 8.4.6 MIDI continuous controller 10 to Pan Position */
      fluid_mod_set_source1(&default_pan_mod, 10,                    /* index=10 */
		       FLUID_MOD_CC                              /* CC=1 */
		       | FLUID_MOD_LINEAR                        /* type=0 */
		       | FLUID_MOD_BIPOLAR                       /* P=1 */
		       | FLUID_MOD_POSITIVE                      /* D=0 */
		       );
      fluid_mod_set_source2(&default_pan_mod, 0, 0);                 /* No second source */
      fluid_mod_set_dest(&default_pan_mod, GEN_PAN);                 /* Target: pan */
  /* Amount: 500. The SF specs $8.4.6, p. 55 syas: "Amount = 1000
     tenths of a percent". The center value (64) corresponds to 50%,
     so it follows that amount = 50% x 1000/% = 500. */
      fluid_mod_set_amount(&default_pan_mod, 500.0);


      /* SF2.01 page 55 section 8.4.7: MIDI continuous controller 11 to initial attenuation*/
      fluid_mod_set_source1(&default_expr_mod, 11,                     /* index=11 */
		       FLUID_MOD_CC                              /* CC=1 */
		       | FLUID_MOD_CONCAVE                       /* type=1 */
		       | FLUID_MOD_UNIPOLAR                      /* P=0 */
		       | FLUID_MOD_NEGATIVE                      /* D=1 */
		       );
      fluid_mod_set_source2(&default_expr_mod, 0, 0);                 /* No second source */
      fluid_mod_set_dest(&default_expr_mod, GEN_ATTENUATION);         /* Target: Initial attenuation */
      fluid_mod_set_amount(&default_expr_mod, 960.0);                 /* Amount: 960 */



      /* SF2.01 page 55 section 8.4.8: MIDI continuous controller 91 to Reverb send */
      fluid_mod_set_source1(&default_reverb_mod, 91,                 /* index=91 */
		       FLUID_MOD_CC                              /* CC=1 */
		       | FLUID_MOD_LINEAR                        /* type=0 */
		       | FLUID_MOD_UNIPOLAR                      /* P=0 */
		       | FLUID_MOD_POSITIVE                      /* D=0 */
		       );
      fluid_mod_set_source2(&default_reverb_mod, 0, 0);              /* No second source */
      fluid_mod_set_dest(&default_reverb_mod, GEN_REVERBSEND);       /* Target: Reverb send */
      fluid_mod_set_amount(&default_reverb_mod, 200);                /* Amount: 200 ('tenths of a percent') */



      /* SF2.01 page 55 section 8.4.9: MIDI continuous controller 93 to Reverb send */
      fluid_mod_set_source1(&default_chorus_mod, 93,                 /* index=93 */
		       FLUID_MOD_CC                              /* CC=1 */
		       | FLUID_MOD_LINEAR                        /* type=0 */
		       | FLUID_MOD_UNIPOLAR                      /* P=0 */
		       | FLUID_MOD_POSITIVE                      /* D=0 */
		       );
      fluid_mod_set_source2(&default_chorus_mod, 0, 0);              /* No second source */
      fluid_mod_set_dest(&default_chorus_mod, GEN_CHORUSSEND);       /* Target: Chorus */
      fluid_mod_set_amount(&default_chorus_mod, 200);                /* Amount: 200 ('tenths of a percent') */



      /* SF2.01 page 57 section 8.4.10 MIDI Pitch Wheel to Initial Pitch ... */
      fluid_mod_set_source1(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEEL, /* Index=14 */
		       FLUID_MOD_GC                              /* CC =0 */
		       | FLUID_MOD_LINEAR                        /* type=0 */
		       | FLUID_MOD_BIPOLAR                       /* P=1 */
		       | FLUID_MOD_POSITIVE                      /* D=0 */
		       );
      fluid_mod_set_source2(&default_pitch_bend_mod, FLUID_MOD_PITCHWHEELSENS,  /* Index = 16 */
		       FLUID_MOD_GC                                        /* CC=0 */
		       | FLUID_MOD_LINEAR                                  /* type=0 */
		       | FLUID_MOD_UNIPOLAR                                /* P=0 */
		       | FLUID_MOD_POSITIVE                                /* D=0 */
		       );
      fluid_mod_set_dest(&default_pitch_bend_mod, GEN_PITCH);                 /* Destination: Initial pitch */
      fluid_mod_set_amount(&default_pitch_bend_mod, 12700.0);                 /* Amount: 12700 cents */
      }

//---------------------------------------------------------
//   init
//    instance initialization
//---------------------------------------------------------

void Fluid::init(int sr)
      {
      if (!initialized) // initialize all the conversion tables and other stuff
            init();

      sample_rate = double(sr);
      sfont_id    = 0;
      gain        = .2;
      state       = FLUID_SYNTH_PLAYING; // as soon as the synth is created it starts playing.
      noteid      = 0;
      tuning      = 0;

      for (int i = 0; i < 256; i++)
            freeVoices.append(new Voice(this));

      cur    = FLUID_BUFSIZE;
      reverb = new Reverb();
      chorus = new Chorus(sample_rate);
      reverb->setPreset(0);
      }

//---------------------------------------------------------
//   ~Fluid
//---------------------------------------------------------

Fluid::~Fluid()
      {
      state = FLUID_SYNTH_STOPPED;
      foreach(Voice* v, activeVoices)
            delete v;
      foreach(Voice* v, freeVoices)
            delete v;
      foreach(SFont* sf, sfonts)
            delete sf;
      foreach(BankOffset* bankOffset, bank_offsets)
            delete bankOffset;
      foreach(Channel* c, channel)
            delete c;

      delete[] left_buf;
      delete[] right_buf;
      delete[] fx_buf[0];
      delete[] fx_buf[1];

      if (reverb)
            delete reverb;
      if (chorus)
            delete chorus;

      /* free the tunings, if any */
      if (tuning) {
            for (int i = 0; i < 128; i++) {
                  if (tuning[i] != 0) {
                        for (int k = 0; k < 128; k++) {
                              if (tuning[i][k] != 0)
                                    free(tuning[i][k]);
                              }
                        free(tuning[i]);
                        }
                  }
            free(tuning);
            }
      }

//---------------------------------------------------------
//   freeVoice
//---------------------------------------------------------

void Fluid::freeVoice(Voice* v)
      {
      if (activeVoices.removeOne(v))
            freeVoices.append(v);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Fluid::play(const Event& event)
      {
      bool err = false;
      int ch   = event.channel();

      if (ch >= channel.size()) {
            for (int i = channel.size(); i < ch+1; i++)
                  channel.append(new Channel(this, i));
            }

      int type    = event.type();
      Channel* cp = channel[ch];

      if (type == ME_NOTEON) {
            int key = event.dataA();
            int vel = event.dataB();
            if (vel == 0) {
                  //
                  // process note off
                  //
                  foreach (Voice* v, activeVoices) {
                        if (v->ON() && (v->chan == ch) && (v->key == key))
                              v->noteoff();
                        }
                  return;
                  }
            if (cp->preset() == 0) {
                  log("channel has no preset");
                  err = true;
                  }
            else {
                  /*
                   * If the same note is hit twice on the same channel, then the older
                   * voice process is advanced to the release stage.  Using a mechanical
                   * MIDI controller, the only way this can happen is when the sustain
                   * pedal is held.  In this case the behaviour implemented here is
                   * natural for many instruments.  Note: One noteon event can trigger
                   * several voice processes, for example a stereo sample.  Don't
                   * release those...
                   */
                  foreach(Voice* v, activeVoices) {
                        if (v->isPlaying() && (v->chan == ch) && (v->key == key) && (v->get_id() != noteid))
                              v->noteoff();
                        }
                  err = !cp->preset()->noteon(this, noteid++, ch, key, vel, event.tuning());
                  }
            }
      else if (type == ME_CONTROLLER)  {
            switch(event.controller()) {
                  case CTRL_PROGRAM:
                        program_change(ch, event.value());
                        break;
                  case CTRL_PITCH:
                        cp->pitchBend(event.value());
                        break;
                  case CTRL_PRESS:
                        break;
                  default:
                        cp->setcc(event.controller(), event.value());
                        break;
                  }
            }
      if (err)
            qWarning("FluidSynth error: event 0x%2x channel %d: %s\n",
               type, ch, qPrintable(error()));
      }

//---------------------------------------------------------
//   damp_voices
//---------------------------------------------------------

void Fluid::damp_voices(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if ((v->chan == chan) && v->SUSTAINED())
                  v->noteoff();
            }
      }

//---------------------------------------------------------
//   get_cc
//---------------------------------------------------------

int Fluid::get_cc(int chan, int num)
      {
      return channel[chan]->cc[num];
      }

//---------------------------------------------------------
//   all_notes_off
//---------------------------------------------------------

void Fluid::all_notes_off(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->noteoff();
            }
      }

//---------------------------------------------------------
//   all_sounds_off
//    immediately stop all notes on this channel.
//---------------------------------------------------------

void Fluid::all_sounds_off(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->off();
            }
      }

//---------------------------------------------------------
//   system_reset
//
//    Purpose:
//    Respond to the MIDI command 'system reset' (0xFF, big red 'panic' button)
//---------------------------------------------------------

void Fluid::system_reset()
      {
      foreach(Voice* v, activeVoices)
            v->off();
      foreach(Channel* c, channel)
            c->reset();
      chorus->reset();
      reverb->reset();
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const MidiPatch* Fluid::getPatchInfo(bool onlyDrums, const MidiPatch* pp) const
      {
      static MidiPatch patch;

      foreach(const SFont* sf, sfonts) {
            BankOffset* bo = get_bank_offset0(sf->id());
            int bankOffset = bo ? bo->offset : 0;
            foreach (Preset* p, sf->getPresets()) {
                  int bank = p->get_banknum() + bankOffset;
                  if (onlyDrums && p->get_banknum() != 128)
                        continue;
                  int program = p->get_num();
                  if (pp == 0) {
                        patch.name = p->get_name();
                        patch.bank = bank;
                        patch.prog = program;
                        patch.drum = onlyDrums;
                        return &patch;
                        }
                  if (pp->name == p->get_name() && pp->bank == bank && pp->prog == program)
                        pp = 0;
                  }
            }
      return 0;
      }

/*
 * fluid_synth_modulate_voices
 *
 * tell all synthesis processes on this channel to update their
 * synthesis parameters after a control change.
 */
void Fluid::modulate_voices(int chan, int is_cc, int ctrl)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->modulate(is_cc, ctrl);
            }
      }

/*
 * fluid_synth_modulate_voices_all
 *
 * Tell all synthesis processes on this channel to update their
 * synthesis parameters after an all control off message (i.e. all
 * controller have been reset to their default value).
 */
void Fluid::modulate_voices_all(int chan)
      {
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->modulate_all();
            }
      }

/*
 * fluid_synth_get_pitch_bend
 */
void Fluid::get_pitch_bend(int chan, int* ppitch_bend)
      {
      *ppitch_bend = channel[chan]->getPitchBend();
      }

/*
 * Fluid_synth_pitch_wheel_sens
 */
void Fluid::pitch_wheel_sens(int chan, int val)
      {
      /* set the pitch-bend value in the channel */
      channel[chan]->pitchWheelSens(val);
      }

/*
 * fluid_synth_get_pitch_wheel_sens
 *
 * Note : this function was added after version 1.0 API freeze.
 * So its API is not in the synth.h file. It should be added in some later
 * version of fluidsynth. Maybe v2.0 ? -- Antoine Schmitt May 2003
 */

void Fluid::get_pitch_wheel_sens(int chan, int* pval)
      {
      // get the pitch-bend value in the channel
      *pval = channel[chan]->pitch_wheel_sensitivity;
      }

/*
 * fluid_synth_get_preset
 */
Preset* Fluid::get_preset(unsigned int sfontnum, unsigned banknum, unsigned prognum)
      {
      SFont* sf = get_sfont_by_id(sfontnum);
      if (sf) {
            int offset     = get_bank_offset(sfontnum);
            Preset* preset = sf->get_preset(banknum - offset, prognum);
            if (preset != 0)
                  return preset;
            }
      return 0;
      }

Preset* Fluid::get_preset(char* sfont_name, unsigned banknum, unsigned prognum)
      {
      SFont* sf = get_sfont_by_name(sfont_name);
      if (sf) {
            int offset = get_bank_offset(sf->id());
            return sf->get_preset(banknum - offset, prognum);
            }
      return 0;
      }

//---------------------------------------------------------
//   find_preset
//---------------------------------------------------------

Preset* Fluid::find_preset(unsigned banknum, unsigned prognum)
      {
      Preset* preset = 0;
      foreach(SFont* sf, sfonts) {
            int offset = get_bank_offset(sf->id());
            preset = sf->get_preset(banknum - offset, prognum);
            if (preset)
                  break;
            }
      return preset;
      }

//---------------------------------------------------------
//   program_change
//---------------------------------------------------------

void Fluid::program_change(int chan, int prognum)
      {
      Channel* c       = channel[chan];
      unsigned banknum = c->getBanknum();
      c->setPrognum(prognum);

      Preset* preset = find_preset(banknum, prognum);

      unsigned sfont_id = preset? preset->sfont->id() : 0;
      c->setSfontnum(sfont_id);
      c->setPreset(preset);
      }

/*
 * fluid_synth_bank_select
 */
void Fluid::bank_select(int chan, unsigned int bank)
      {
      channel[chan]->setBanknum(bank);
      }

/*
 * fluid_synth_sfont_select
 */
void Fluid::sfont_select(int chan, unsigned int sfont_id)
      {
      channel[chan]->setSfontnum(sfont_id);
      }

/*
 * fluid_synth_get_program
 */
void Fluid::get_program(int chan, unsigned* sfont_id, unsigned* bank_num, unsigned* preset_num)
      {
      Channel* c       = channel[chan];
      *sfont_id        = c->getSfontnum();
      *bank_num        = c->getBanknum();
      *preset_num      = c->getPrognum();
      }

//---------------------------------------------------------
//   program_select
//---------------------------------------------------------

bool Fluid::program_select(int chan, unsigned sfont_id, unsigned bank_num, unsigned preset_num)
      {
      Channel* c     = channel[chan];
      Preset* preset = get_preset(sfont_id, bank_num, preset_num);
      if (preset == 0) {
            log("There is no preset with bank number %d and preset number %d in SoundFont %d", bank_num, preset_num, sfont_id);
            return false;
            }

      /* inform the channel of the new bank and program number */
      c->setSfontnum(sfont_id);
      c->setBanknum(bank_num);
      c->setPrognum(preset_num);
      c->setPreset(preset);
      return true;
      }

/*
 * fluid_synth_program_select2
 */
bool Fluid::program_select2(int chan, char* sfont_name, unsigned bank_num, unsigned preset_num)
      {
      Channel* c = channel[chan];
      SFont* sf = get_sfont_by_name(sfont_name);
      if (sf == 0)
            return log("Could not find SoundFont %s", sfont_name);
      int offset     = get_bank_offset(sf->id());
      Preset* preset = sf->get_preset(bank_num - offset, preset_num);
      if (preset == 0)
            return log("There is no preset with bank number %d and preset number %d in SoundFont %s",
               bank_num, preset_num, sfont_name);

      /* inform the channel of the new bank and program number */
      c->setSfontnum(sf->id());
      c->setBanknum(bank_num);
      c->setPrognum(preset_num);
      c->setPreset(preset);
      return true;
      }

/*
 * fluid_synth_update_presets
 */
void Fluid::update_presets()
      {
      foreach(Channel* c, channel)
            c->setPreset(get_preset(c->getSfontnum(), c->getBanknum(), c->getPrognum()));
      }

/*
 * fluid_synth_set_gain
 */
void Fluid::set_gain(float g)
      {
      g = qBound(0.0f, g, 10.0f);
      gain = g;

      foreach(Voice* v, activeVoices)
            v->set_gain(g);
      }

/*
 * fluid_synth_program_reset
 *
 * Resend a bank select and a program change for every channel. This
 * function is called mainly after a SoundFont has been loaded,
 * unloaded or reloaded.  */

void Fluid::program_reset()
      {
      /* try to set the correct presets */
      int n = channel.size();
      for (int i = 0; i < n; i++)
            program_change(i, channel[i]->getPrognum());
      }

/*
 * fluid_synth_set_reverb_preset
 */
bool Fluid::set_reverb_preset(int num)
      {
      return reverb->setPreset(num);
      }

/*
 * fluid_synth_set_reverb
 */
void Fluid::set_reverb(double roomsize, double damping, double width, double level)
      {
      reverb->setroomsize(roomsize);
      reverb->setdamp(damping);
      reverb->setwidth(width);
      reverb->setlevel(level);
      }

/*
 * fluid_synth_set_chorus
 */
void Fluid::set_chorus(int nr, double level, double speed, double depth_ms, int type)
      {
      chorus->set_nr(nr);
      chorus->set_level((float)level);
      chorus->set_speed_Hz((float)speed);
      chorus->set_depth_ms((float)depth_ms);
      chorus->set_type(type);
      chorus->update();
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Fluid::process(unsigned len, float* lout, float* rout, int stride)
      {
      for (unsigned int i = 0; i < len; i++, cur++) {
            if (cur == FLUID_BUFSIZE) {
                  one_block();
                  cur = 0;
                  }
            *lout = left_buf[cur];
            *rout = right_buf[cur];
            lout += stride;
            rout += stride;
            }
      }

//---------------------------------------------------------
//   one_block
//---------------------------------------------------------

void Fluid::one_block()
      {
      static const int byte_size = FLUID_BUFSIZE * sizeof(float);

      /* clean the audio buffers */
      memset(left_buf,  0, byte_size);
      memset(right_buf, 0, byte_size);

      for (int i = 0; i < 2; i++)
            memset(fx_buf[i], 0, byte_size);

      if (activeVoices.isEmpty())
            --silentBlocks;
      else {
            silentBlocks = SILENT_BLOCKS;
            foreach(Voice* v, activeVoices)
                  v->write(left_buf, right_buf, fx_buf[0], fx_buf[1]);
            }

      if (silentBlocks > 0) {
            reverb->process(FLUID_BUFSIZE, fx_buf[0], left_buf, right_buf);
            chorus->process(FLUID_BUFSIZE, fx_buf[1], left_buf, right_buf);
            }
      }

/*
 * fluid_synth_free_voice_by_kill
 *
 * selects a voice for killing. the selection algorithm is a refinement
 * of the algorithm previously in fluid_synth_alloc_voice.
 */

Voice* Fluid::free_voice_by_kill()
      {
      float best_prio = 999999.;
      float this_voice_prio;
      Voice* best_voice = 0;

      foreach(Voice* v, activeVoices) {
            /* Determine, how 'important' a voice is.
             * Start with an arbitrary number */
            this_voice_prio = 10000.;

            /* Is this voice on the drum channel?
             * Then it is very important.
             * Also, forget about the released-note condition:
             * Typically, drum notes are triggered only very briefly, they run most
             * of the time in release phase.
             */
            if (v->chan == 9) {
                  this_voice_prio += 4000;

                  }
            else if (v->RELEASED()) {
                  /* The key for this voice has been released. Consider it much less important
                  * than a voice, which is still held.
                  */
                  this_voice_prio -= 2000.;
                  }

            if (v->SUSTAINED()) {
              /* The sustain pedal is held down on this channel.
               * Consider it less important than non-sustained channels.
               * This decision is somehow subjective. But usually the sustain pedal
               * is used to play 'more-voices-than-fingers', so it shouldn't hurt
               * if we kill one voice.
               */
                  this_voice_prio -= 1000;
                  }

            /* We are not enthusiastic about releasing voices, which have just been started.
             * Otherwise hitting a chord may result in killing notes belonging to that very same
             * chord.
             * So subtract the age of the voice from the priority - an older voice is just a little
             * bit less important than a younger voice.
             * This is a number between roughly 0 and 100.*/

            this_voice_prio -= (noteid - v->get_id());

            /* take a rough estimate of loudness into account. Louder voices are more important. */
            if (v->volenv_section != FLUID_VOICE_ENVATTACK) {
                  this_voice_prio += v->volenv_val * 1000.;
                  }

            /* check if this voice has less priority than the previous candidate. */
            if (this_voice_prio < best_prio)
                  best_voice = v;
            best_prio = this_voice_prio;
            }
      if (!best_voice)
            return 0;
      best_voice->off();
      return best_voice;
      }

//---------------------------------------------------------
//   alloc_voice
//---------------------------------------------------------

Voice* Fluid::alloc_voice(unsigned id, Sample* sample, int chan, int key, int vel, double vt)
      {
      Voice* v   = 0;
      Channel* c = 0;

      /* check if there's an available synthesis process */
      if (!freeVoices.isEmpty())
            v = freeVoices.takeLast();
      else
            v = free_voice_by_kill();

      if (v == 0) {
            log("Failed to allocate a synthesis process. (chan=%d,key=%d)", chan, key);
            return 0;
            }

      activeVoices.append(v);

      if (chan >= 0)
            c = channel[chan];

      v->init(sample, c, key, vel, id, gain, vt);

      /* add the default modulators to the synthesis process. */
      v->add_mod(&default_vel2att_mod, FLUID_VOICE_DEFAULT);    /* SF2.01 $8.4.1  */
      v->add_mod(&default_vel2filter_mod, FLUID_VOICE_DEFAULT); /* SF2.01 $8.4.2  */
      v->add_mod(&default_at2viblfo_mod, FLUID_VOICE_DEFAULT);  /* SF2.01 $8.4.3  */
      v->add_mod(&default_mod2viblfo_mod, FLUID_VOICE_DEFAULT); /* SF2.01 $8.4.4  */
      v->add_mod(&default_att_mod, FLUID_VOICE_DEFAULT);        /* SF2.01 $8.4.5  */
      v->add_mod(&default_pan_mod, FLUID_VOICE_DEFAULT);        /* SF2.01 $8.4.6  */
      v->add_mod(&default_expr_mod, FLUID_VOICE_DEFAULT);       /* SF2.01 $8.4.7  */
      v->add_mod(&default_reverb_mod, FLUID_VOICE_DEFAULT);     /* SF2.01 $8.4.8  */
      v->add_mod(&default_chorus_mod, FLUID_VOICE_DEFAULT);     /* SF2.01 $8.4.9  */
      v->add_mod(&default_pitch_bend_mod, FLUID_VOICE_DEFAULT); /* SF2.01 $8.4.10 */
      return v;
      }

//---------------------------------------------------------
//   start_voice
//---------------------------------------------------------

void Fluid::start_voice(Voice* voice)
      {
      /* Find the exclusive class of this voice. If set, kill all voices
      * that match the exclusive class and are younger than the first
      * voice process created by this noteon event. */

      /** Kill all voices on a given channel, which belong into
          excl_class.  This function is called by a SoundFont's preset in
          response to a noteon event.  If one noteon event results in
          several voice processes (stereo samples), ignore_ID must name
          the voice ID of the first generated voice (so that it is not
          stopped). The first voice uses ignore_ID=-1, which will
          terminate all voices on a channel belonging into the exclusive
          class excl_class.
      */

      /* Check if the voice belongs to an exclusive class. In that case,
         previous notes from the same class are released. */

      int excl_class = voice->GEN(GEN_EXCLUSIVECLASS);
      if (excl_class) {

            /* Kill all notes on the same channel with the same exclusive class */

            foreach(Voice* existing_voice, activeVoices) {
                  /* Existing voice does not play? Leave it alone. */
                  if (!existing_voice->isPlaying())
                        continue;

                  /* An exclusive class is valid for a whole channel (or preset).
                   * Is the voice on a different channel? Leave it alone. */
                  if (existing_voice->chan != voice->chan)
                        continue;

                  /* Existing voice has a different (or no) exclusive class? Leave it alone. */
                  if ((int)existing_voice->GEN(GEN_EXCLUSIVECLASS) != excl_class)
                        continue;

                  /* Existing voice is a voice process belonging to this noteon
                   * event (for example: stereo sample)?  Leave it alone. */
                  if (existing_voice->get_id() == voice->get_id())
                        continue;
                  existing_voice->kill_excl();
                  }
            }
      voice->voice_start();
      }

//---------------------------------------------------------
//   sfload
//---------------------------------------------------------

int Fluid::sfload(const QString& filename, int reset_presets)
      {
      if (filename.isEmpty()) {
            printf("Invalid filename");
            return -1;
            }

      SFont* sf = new SFont(this);
      if (!sf->read(filename)) {
            delete sf;
            sf = 0;
            printf("Failed to load SoundFont <%s>", qPrintable(filename));
            return -1;
            }

      sf->setId(++sfont_id);

      /* insert the sfont as the first one on the list */
      sfonts.prepend(sf);

      /* reset the presets for all channels */
      if (reset_presets)
            program_reset();

      return (int) sf->id();
      }

//---------------------------------------------------------
//   sfunload
//---------------------------------------------------------

bool Fluid::sfunload(unsigned int id, int reset_presets)
      {
      SFont* sf = get_sfont_by_id(id);

      if (!sf) {
            printf("No SoundFont with id = %d", id);
            return false;
            }

      sfonts.removeAll(sf);   // remove the SoundFont from the list

      /* reset the presets for all channels */
      if (reset_presets)
            program_reset();
      else
            update_presets();
      delete sf;
      return true;
      }

/* fluid_synth_sfreload
 *
 */
int Fluid::sfreload(unsigned int id)
      {
      SFont* sf = get_sfont_by_id(id);
      if (!sf) {
            log("No SoundFont with id = %d", id);
            return -1;
            }

      int index = sfonts.indexOf(sf);

      /* keep a copy of the SoundFont's filename */
      QString filename = sf->get_name();

      if (!sfunload(id, 0))
            return -1;

      if (!sf->read(filename)) {
            delete sf;
            sf = 0;
            log("Failed to load SoundFont \"%s\"", qPrintable(filename));
            return -1;
            }
      sf->setId(id);
      sfonts.insert(index, sf);
      update_presets();       // reset the presets for all channels
      return sf->id();
      }

/*
 * fluid_synth_add_sfont
 */
int Fluid::add_sfont(SFont* sf)
      {
	sf->setId(++sfont_id);

	/* insert the sfont as the first one on the list */
      sfonts.prepend(sf);

	/* reset the presets for all channels */
	program_reset();
	return sf->id();
      }

/*
 * fluid_synth_remove_sfont
 */
void Fluid::remove_sfont(SFont* sf)
      {
	int sfont_id = sf->id();
	sfonts.removeAll(sf);

	remove_bank_offset(sfont_id); /* remove a possible bank offset */
	program_reset();              /* reset the presets for all channels */
      }

/* fluid_synth_get_sfont_by_id
 *
 */
SFont* Fluid::get_sfont_by_id(unsigned int id)
      {
      foreach(SFont* sf, sfonts) {
            if (sf->id() == id)
                  return sf;
            }
      return 0;
      }

/* fluid_synth_get_sfont_by_name
 *
 */
SFont* Fluid::get_sfont_by_name(const QString& name)
      {
      foreach(SFont* sf, sfonts) {
            if (sf->get_name() == name)
                  return sf;
            }
      return 0;
      }

/*
 * fluid_synth_get_channel_preset
 */
Preset* Fluid::get_channel_preset(int chan)
      {
      return channel[chan]->preset();
      }

/* Sets the interpolation method to use on channel chan.
 * If chan is < 0, then set the interpolation method on all channels.
 */
void Fluid::set_interp_method(int chan, int interp_method)
      {
      foreach(Channel* c, channel) {
            if (chan < 0 || c->getNum() == chan)
                  c->setInterpMethod(interp_method);
            }
      }

Tuning* Fluid::get_tuning(int bank, int prog)
      {
      if ((bank < 0) || (bank >= 128)) {
            log("Bank number out of range");
            return 0;
            }
      if ((prog < 0) || (prog >= 128)) {
            log("Program number out of range");
            return 0;
            }
      if ((tuning == 0) || (tuning[bank] == 0) || (tuning[bank][prog] == 0)) {
            log("No tuning at bank %d, prog %d", bank, prog);
            return 0;
            }
      return tuning[bank][prog];
      }

Tuning* Fluid::create_tuning(int bank, int prog, char* name)
      {
      if ((bank < 0) || (bank >= 128)) {
            log("Bank number out of range");
            return 0;
            }
      if ((prog < 0) || (prog >= 128)) {
            log("Program number out of range");
            return 0;
            }
      if (tuning == 0) {
            tuning = new Tuning**[128];
            memset(tuning, 0, 128 * sizeof(Tuning**));
            }
      if (tuning[bank] == 0) {
            tuning[bank] = new Tuning*[128];
            memset(tuning[bank], 0, 128 * sizeof(Tuning*));
            }
      if (tuning[bank][prog] == 0)
            tuning[bank][prog] = new Tuning(name, bank, prog);
      if (tuning[bank][prog]->name() != name)
            tuning[bank][prog]->setName(name);
      return tuning[bank][prog];
      }

void Fluid::create_key_tuning(int bank, int prog, char* name, double* pitch)
      {
      Tuning* tuning = create_tuning(bank, prog, name);
      if (pitch)
            tuning->setAll(pitch);
      }

void Fluid::create_octave_tuning(int bank, int prog, char* name, double* pitch)
      {
      Tuning* tuning = create_tuning(bank, prog, name);
      tuning->setOctave(pitch);
      }

bool Fluid::tune_notes(int bank, int prog, int len, int *key, double* pitch)
      {
      Tuning* tuning = get_tuning(bank, prog);
      if (tuning == 0)
            return false;
      for (int i = 0; i < len; i++)
            tuning->setPitch(key[i], pitch[i]);
      return true;
      }

//---------------------------------------------------------
//   fluid_synth_select_tuning
//---------------------------------------------------------

bool Fluid::select_tuning(int chan, int bank, int prog)
      {
      Tuning* t = get_tuning(bank, prog);

      if (t == 0)
            return false;

      channel[chan]->setTuning(t);
      return true;
      }

//---------------------------------------------------------
//   fluid_synth_reset_tuning
//---------------------------------------------------------

void Fluid::reset_tuning(int chan)
      {
      channel[chan]->setTuning(0);
      }

void Fluid::tuning_iteration_start()
      {
      cur_tuning = 0;
      }

int Fluid::tuning_iteration_next(int* bank, int* prog)
      {
      int b = 0, p = 0;

      if (tuning == 0)
            return 0;

      if (cur_tuning != 0) {
            /* get the next program number */
            b = cur_tuning->getBank();
            p = 1 + cur_tuning->getProg();
            if (p >= 128) {
                  p = 0;
                  b++;
                  }
            }
      while (b < 128) {
            if (tuning[b] != 0) {
                  while (p < 128) {
                        if (tuning[b][p] != 0) {
                              cur_tuning = tuning[b][p];
                              *bank = b;
                              *prog = p;
                              return 1;
                              }
                        p++;
                        }
                  }
            p = 0;
            b++;
            }
      return 0;
      }

//---------------------------------------------------------
//   set_gen
//---------------------------------------------------------

void Fluid::set_gen(int chan, int param, float value)
      {
      channel[chan]->setGen(param, value, 0);
      foreach(Voice* v, activeVoices) {
            if (v->chan == chan)
                  v->set_param(param, value, 0);
            }
      }

/** Change the value of a generator. This function allows to control
    all synthesis parameters in real-time. The changes are additive,
    i.e. they add up to the existing parameter value. This function is
    similar to sending an NRPN message to the synthesizer. The
    function accepts a float as the value of the parameter. The
    parameter numbers and ranges are described in the SoundFont 2.01
    specification, paragraph 8.1.3, page 48. See also
    'fluid_gen_type'.

    Using the fluid_synth_set_gen2() function, it is possible to set
    the absolute value of a generator. This is an extension to the
    SoundFont standard. If 'absolute' is non-zero, the value of the
    generator specified in the SoundFont is completely ignored and the
    generator is fixed to the value passed as argument. To undo this
    behavior, you must call fluid_synth_set_gen2 again, with
    'absolute' set to 0 (and possibly 'value' set to zero).

    If 'normalized' is non-zero, the value is supposed to be
    normalized between 0 and 1. Before applying the value, it will be
    scaled and shifted to the range defined in the SoundFont
    specifications.

 */
void Fluid::set_gen2(int chan, int param, float value, int absolute, int normalized)
      {
      float v = (normalized)? fluid_gen_scale(param, value) : value;
      channel[chan]->setGen(param, v, absolute);

      foreach(Voice* vo, activeVoices) {
            if (vo->chan == chan)
                  vo->set_param(param, v, absolute);
            }
      }

float Fluid::get_gen(int chan, int param)
      {
      if ((param < 0) || (param >= GEN_LAST)) {
            log("Parameter number out of range");
            return 0.0;
            }
      return channel[chan]->getGen(param);
      }

#if 0
bool Fluid::stop(unsigned int id)
      {
      bool status = false;
      foreach(Voice* v, activeVoices) {
            if (v->get_id() == id) {
                  v->noteoff();
                  status = true;
                  }
            }
      return status;
      }
#endif

BankOffset* Fluid::get_bank_offset0(int sfont_id) const
      {
      foreach(BankOffset* offset, bank_offsets) {
		if (offset->sfont_id == sfont_id)
			return offset;
	      }
	return 0;
      }

int Fluid::set_bank_offset(int sfont_id, int offset)
      {
	BankOffset* bank_offset = get_bank_offset0(sfont_id);

	if (bank_offset == 0) {
		bank_offset = new BankOffset;
		bank_offset->sfont_id = sfont_id;
		bank_offset->offset   = offset;
		bank_offsets.prepend(bank_offset);
	      }
      else {
	      bank_offset->offset = offset;
            }
	return 0;
      }

int Fluid::get_bank_offset(int sfont_id)
      {
      BankOffset* bank_offset = get_bank_offset0(sfont_id);
      return (bank_offset == 0)? 0 : bank_offset->offset;
      }

void Fluid::remove_bank_offset(int sfont_id)
      {
	BankOffset* bank_offset = get_bank_offset0(sfont_id);
	if (bank_offset)
		bank_offsets.removeAll(bank_offset);
      }

/**
 * Print a message to the log.
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return Always returns -1
 */

bool Fluid::log(const char* fmt, ...)
      {
      char buf[512];
      va_list args;
      va_start (args, fmt);
      vsnprintf(buf, sizeof(buf), fmt, args);
      va_end (args);
      _error = buf;
      return false;
      }

Tuning::Tuning(const QString& n, int b, int p)
      {
      _name = n;
      bank = b;
      prog = p;

      for (int i = 0; i < 128; i++)
            pitch[i] = i * 100.0;
      }

void Tuning::setOctave(double* pitch_deriv)
      {
      for (int i = 0; i < 128; i++)
            pitch[i] = i * 100.0 + pitch_deriv[i % 12];
      }

void Tuning::setAll(double* p)
      {
      for (int i = 0; i < 128; i++)
            pitch[i] = p[i];
      }

void Tuning::setPitch(int k, double p)
      {
      if ((k >= 0) && (k < 128))
            pitch[k] = p;
      }
}
