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

#include "seq_event.h"
#include "fluid.h"
#include "sfont.h"
#include "conv.h"
#include "gen.h"
#include "chorus.h"
#include "voice.h"
#include "sparm_p.h"

namespace FluidS {

extern SFont* createSf();


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

static const Mod defaultMod[] = {
      { GEN_ATTENUATION, FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_FILTERFC,
         FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE,
         FLUID_MOD_VELOCITY, FLUID_MOD_GC | FLUID_MOD_SWITCH | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE,
         -2400 },
      { GEN_VIBLFOTOPITCH, FLUID_MOD_CHANNELPRESSURE, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 50 },
      { GEN_VIBLFOTOPITCH, 1, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 50 },
      { GEN_ATTENUATION, 7, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_PAN, 10, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 500.0 },
      { GEN_ATTENUATION, 11, FLUID_MOD_CC | FLUID_MOD_CONCAVE | FLUID_MOD_UNIPOLAR | FLUID_MOD_NEGATIVE, 0, 0, 960.0 },
      { GEN_REVERBSEND, 91, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 200 },
      { GEN_CHORUSSEND, 93, FLUID_MOD_CC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE, 0, 0, 200 },
      { GEN_PITCH,
           FLUID_MOD_PITCHWHEEL,     FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_BIPOLAR  | FLUID_MOD_POSITIVE,
           FLUID_MOD_PITCHWHEELSENS, FLUID_MOD_GC | FLUID_MOD_LINEAR | FLUID_MOD_UNIPOLAR | FLUID_MOD_POSITIVE,
        12700.0 },
      };

//
// list of fluid paramters as saved in score
//
static SyntiParameter params[] = {
      SyntiParameter(SParmId(FLUID_ID, 1, 0).val, "RevRoomsize", 0.0),
      SyntiParameter(SParmId(FLUID_ID, 1, 1).val, "RevDamp",   0.0),
      SyntiParameter(SParmId(FLUID_ID, 1, 2).val, "RevWidth",  0.0),
      SyntiParameter(SParmId(FLUID_ID, 1, 3).val, "RevGain",   0.0),

      SyntiParameter(SParmId(FLUID_ID, 2, 0).val, "ChoType",   0.0),
      SyntiParameter(SParmId(FLUID_ID, 2, 1).val, "ChoSpeed",  0.0),
      SyntiParameter(SParmId(FLUID_ID, 2, 2).val, "ChoDepth",  0.0),
      SyntiParameter(SParmId(FLUID_ID, 2, 3).val, "ChoBlocks", 0.0),
      SyntiParameter(SParmId(FLUID_ID, 2, 4).val, "ChoGain",   0.0),
      };

SFont* Fluid::sf;

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

Fluid::Fluid()
      {
      reverb    = 0;
      chorus    = 0;
      silentBlocks = 0;
      sf        = createSf();
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
      sf = createSf();
      }

//---------------------------------------------------------
//   init
//    instance initialization
//---------------------------------------------------------

void Fluid::init(int sr)
      {
      if (!initialized) // initialize all the conversion tables and other stuff
            init();

      sample_rate = float(sr);
      _gain       = 1.0;

      _state       = FLUID_SYNTH_PLAYING; // as soon as the synth is created it starts playing.
      noteid       = 0;
      for (int i = 0; i < 128; ++i)
            _tuning[i] = i * 100.0;
      _masterTuning = 440.0;

      for (int i = 0; i < 512; i++)
            freeVoices.append(new Voice(this));

      reverb = new Reverb();
      chorus = new Chorus(sample_rate);
      reverb->setPreset(0);
      }

//---------------------------------------------------------
//   ~Fluid
//---------------------------------------------------------

Fluid::~Fluid()
      {
      _state = FLUID_SYNTH_STOPPED;
      foreach(Voice* v, activeVoices)
            delete v;
      foreach(Voice* v, freeVoices)
            delete v;
      foreach(Channel* c, channel)
            delete c;

      delete reverb;
      delete chorus;
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

void Fluid::play(const SeqEvent& event)
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
                  err = true;
                  }
            else {
                  foreach(Voice* v, activeVoices) {
                        if (v->isPlaying() && (v->chan == ch) && (v->key == key))
                              v->noteoff();
                        }
                  err = !cp->preset()->noteon(this, ch, key, vel, event.tuning());
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
//   allNotesOff
//---------------------------------------------------------

void Fluid::allNotesOff()
      {
      foreach(Voice* v, activeVoices)
            v->noteoff();
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

/*
 * fluid_synth_modulate_voices
 *
 * tell all synthesis processes on this channel to update their
 * synthesis parameters after a control change.
 */
void Fluid::modulate_voices(int chan, bool is_cc, int ctrl)
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
 * fluid_synth_get_preset
 */
Preset* Fluid::get_preset(unsigned banknum, unsigned prognum)
      {
      return sf->preset(banknum, prognum);
      }

//---------------------------------------------------------
//   program_change
//---------------------------------------------------------

void Fluid::program_change(int chan, int prognum)
      {
      Channel* c       = channel[chan];
      unsigned banknum = c->getBanknum();
      c->setPrognum(prognum);

      Preset* preset = get_preset(banknum, prognum);

      c->setSfontnum(0);
      c->setPreset(preset);
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
 * fluid_synth_set_reverb
 */
void Fluid::set_reverb(float roomsize, float damping, float width, float level)
      {
      reverb->setroomsize(roomsize);
      reverb->setdamp(damping);
      reverb->setwidth(width);
      reverb->setlevel(level);
      }

/*
 * fluid_synth_set_chorus
 */
void Fluid::set_chorus(int nr, float level, float speed, float depth_ms, int type)
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

void Fluid::process(unsigned len, short* p)
      {
      foreach (Voice* v, activeVoices)
            v->write(len, p);
      }

/*
 * fluid_synth_free_voice_by_kill
 *
 * selects a voice for killing. the selection algorithm is a refinement
 * of the algorithm previously in fluid_synth_alloc_voice.
 */

void Fluid::free_voice_by_kill()
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

            /* take a rough estimate of loudness into account. Louder voices are more important. */
            if (v->volenv_section != FLUID_VOICE_ENVATTACK) {
                  this_voice_prio += v->volenv_val * 1000.;
                  }

            /* check if this voice has less priority than the previous candidate. */
            if (this_voice_prio < best_prio) {
                  best_voice = v;
                  best_prio = this_voice_prio;
                  }
            }
      if (best_voice)
            best_voice->off();
      }

//---------------------------------------------------------
//   alloc_voice
//---------------------------------------------------------

Voice* Fluid::alloc_voice(Sample* sample, int chan, int key, int vel, float vt)
      {
      Channel* c = 0;

      /* check if there's an available synthesis process */
      if (freeVoices.isEmpty())
            free_voice_by_kill();

      if (freeVoices.isEmpty()) {
            return 0;
            }

      Voice* v = freeVoices.takeLast();
      activeVoices.append(v);
#if 0
            char buffer[64];
            sprintf(buffer, "voices %d", activeVoices.size());
            CPLOG(buffer);
#endif
      if (chan >= 0)
            c = channel[chan];

      v->init(sample, c, key, vel, vt);

      /* add the default modulators to the synthesis process. */
      for (unsigned i = 0; i < sizeof(defaultMod)/sizeof(*defaultMod); ++i)
            v->add_mod(&defaultMod[i],  FLUID_VOICE_DEFAULT);
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
//                  if (existing_voice->get_id() == voice->get_id())
//                        continue;
                  existing_voice->kill_excl();
                  }
            }
      voice->voice_start();
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

float Fluid::get_gen(int chan, int param)
      {
      if ((param < 0) || (param >= GEN_LAST)) {
            return 0.0;
            }
      return channel[chan]->getGen(param);
      }

SyntiParameter Fluid::parameter(int id) const
      {
      for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
            SyntiParameter& p = params[i];
            if (id == p.id()) {
                  SParmId spid(p.id());
                  int group = spid.subsystemId;
                  int no    = spid.paramId;
                  if (group == 1)
                        params[i].set(reverb->parameter(no));
                  else if (group == 2)
                        params[i].set(chorus->parameter(no));
                  return params[i];
                  }
            }
      return SyntiParameter();
      }

void Fluid::setParameter(int id, float value)
      {
      SParmId spid(id);
      if (spid.syntiId != FLUID_ID)
            return;
      if (spid.subsystemId == 0)
            reverb->setParameter(spid.paramId, value);
      else if (spid.subsystemId == 1)
            chorus->setParameter(spid.paramId, value);
      }

//---------------------------------------------------------
//   state
//---------------------------------------------------------

SyntiState Fluid::state() const
      {
      SyntiState sp;

      //
      // fill in struct with actual values
      //
      for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
            SyntiParameter& p = params[i];
            SParmId spid(p.id());

            int group = spid.subsystemId;
            int no    = spid.paramId;

            if (group == 1)
                  params[i].set(reverb->parameter(no));
            else if (group == 2)
                  params[i].set(chorus->parameter(no));
            sp.append(params[i]);
            }
      return sp;
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Fluid::setState(const SyntiState& sp)
      {
      foreach(const SyntiParameter& p, sp) {
            int id = p.id();
            if (id == -1) {
                  //
                  // if id of parameter is invalid, name must be
                  // valid; lookup name in params table
                  //
                  for (unsigned i = 0; i < sizeof(params)/sizeof(*params); ++i) {
                        SyntiParameter& p2 = params[i];
                        if (p2.name() == p.name()) {
                              id = p2.id();
                              break;
                              }
                        }
                  if (id == -1 && p.name() == "soundfont")
                        id = SParmId(FLUID_ID, 0, 0).val;
                  if (id == -1)     // not for this synthesizer
                        continue;
                  }
            SParmId spid(id);
            if (spid.syntiId != FLUID_ID)
                  continue;
            int group = spid.subsystemId;
            int no    = spid.paramId;

// printf("Fluid::setState: "); p.print(); printf("\n");

            if (group == 0) {
                  }
            else if (group == 1) {
                  reverb->setParameter(no, p.fval());
                  }
            else if (group == 2) {
                  chorus->setParameter(no, p.fval());
                  }
            }
      }
}
