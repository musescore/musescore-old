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


#ifndef __FLUID_S_H__
#define __FLUID_S_H__

#include "synti.h"
#include "list.h"
#include "rev.h"
#include "chorus.h"

namespace FluidS {

class Voice;
class SFont;
class Preset;
class Sample;
struct Tuning;
class Channel;
class Mod;
class Reverb;

#define FLUID_NUM_PROGRAMS      129
#define DRUM_INST_MASK         ((unsigned int)0x80000000)


enum fluid_loop {
      FLUID_UNLOOPED            = 0,
      FLUID_LOOP_DURING_RELEASE = 1,
      FLUID_NOTUSED             = 2,
      FLUID_LOOP_UNTIL_RELEASE  = 3
      };

enum fluid_synth_status {
      FLUID_SYNTH_CLEAN,
      FLUID_SYNTH_PLAYING,
      FLUID_SYNTH_QUIET,
      FLUID_SYNTH_STOPPED
      };

struct BankOffset {
      int sfont_id;
      int offset;
      };

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

class Fluid : public Synth {
      QList<SFont*> sfonts;               // the loaded soundfonts
      QList<BankOffset*> bank_offsets;    // the offsets of the soundfont banks
      int silentBlocks;
      static const int SILENT_BLOCKS = 32;

   public:
      int polyphony;                      // maximum polyphony
      bool with_reverb;                   // enable built-in reverb unit
      bool with_chorus;                   // enable built-in chorus unit
      double sample_rate;                 // The sample rate
      int midi_channels;                  // the number of MIDI channels (>= 16)
      int audio_channels;                 // the number of audio channels (1 channel=left+right)
      unsigned int state;                 // the synthesizer state
      unsigned int ticks;                 // the number of audio samples since the start
      unsigned startTime;                 // the start in msec, as returned by system clock

      unsigned int sfont_id;

      double gain;                        // master gain
      QList<Channel*> channel;            // the channels

      int nvoice;                         // the length of the synthesis process array
      QList<Voice*> voice;                // the synthesis processes
      unsigned int noteid;                // the id is incremented for every new note. it's used for noteoff's
      unsigned int storeid;
      int nbuf;                           // How many audio buffers are used? (depends on nr of audio channels / groups)

      fluid_real_t* left_buf;
      fluid_real_t* right_buf;
      fluid_real_t* fx_left_buf[2];
      fluid_real_t* fx_right_buf[2];

      Reverb* reverb;
      fluid_chorus_t* chorus;
      int cur;                            // the current sample in the audio buffers to be output

      Tuning*** tuning;                   // 128 banks of 128 programs for the tunings
      Tuning* cur_tuning;                 // current tuning in the iteration

   public:
      Fluid();
      ~Fluid();
      virtual void init(int sr, int ch);
      virtual bool loadSoundFont(const QString& s) { return sfload(s, true); }
      virtual void play(const MidiOutEvent&);
      virtual const MidiPatch* getPatchInfo(bool onlyDrums, const MidiPatch*) const;

      static bool initialized;

      static void init();

      int set_reverb_preset(int num);
      int one_block();

      Preset* get_preset(unsigned int sfontnum, unsigned int banknum, unsigned int prognum);
      Preset* find_preset(unsigned int banknum, unsigned int prognum);
      int all_notes_off(int chan);
      int all_sounds_off(int chan);
      int modulate_voices(int chan, int is_cc, int ctrl);
      int modulate_voices_all(int chan);
      int damp_voices(int chan);
      int kill_voice(Voice * voice);
      void kill_by_exclusive_class(Voice* voice);
      void release_voice_on_same_note(int chan, int key);
      void sfunload_macos9();
      void print_voice();

      /** This function assures that every MIDI channels has a valid preset
       *  (NULL is okay). This function is called after a SoundFont is
       *  unloaded or reloaded. */
      void update_presets();

      int update_gain(char* name, double value);
      int update_polyphony(char* name, int value);
      BankOffset* get_bank_offset0(int sfont_id) const;
      void remove_bank_offset(int sfont_id);

      char* error()                             { return fluid_error(); }

      int noteon(int chan, int key, int vel);
      int noteoff(int chan, int key);
      int get_cc(int chan, int num, int* pval);
      int system_reset();
      int pitch_bend(int chan, int val);
      int program_change(int chan, int prognum);
      int Fluiduning_iteration_next(int* bank, int* prog);
      void Fluiduning_iteration_start();

      int get_bank_offset(int sfont_id);
      int set_bank_offset(int sfont_id, int offset);
      int stop(unsigned int id);
      int start(unsigned int id, Preset* preset, int midi_chan, int key, int vel);
      int set_gen2(int chan, int param, float value, int absolute, int normalized);
      float get_gen(int chan, int param);
      int set_gen(int chan, int param, float value);
      int count_audio_channels() const   { return audio_channels; }
      int count_midi_channels() const    { return midi_channels; }
      int set_interp_method(int chan, int interp_method);
      void set_reverb_on(int on);
      void set_chorus_on(int on);
      int get_chorus_nr();
      double get_chorus_level();
      double get_chorus_speed_Hz();
      double get_chorus_depth_ms();
      int get_chorus_type();
      void get_voicelist(Voice* buf[], int bufsize, int ID);
      Preset* get_channel_preset(int chan);
      SFont* get_sfont_by_name(const QString& name);
      SFont* get_sfont_by_id(unsigned int id);
      SFont* get_sfont(unsigned int num);
      int sfcount() const                 { return sfonts.size(); }
      void remove_sfont(SFont* sf);
      int add_sfont(SFont* sf);
      int sfreload(unsigned int id);
      int sfunload(unsigned int id, int reset_presets);
      int sfload(const QString& filename, int reset_presets);
      void start_voice(Voice* voice);
      Voice* alloc_voice(Sample* sample, int chan, int key, int vel);
      Voice* free_voice_by_kill();

      virtual void process(unsigned len, float* lout, float* rout, int stride);

      void set_chorus(int nr, double level, double speed, double depth_ms, int type);
      void set_reverb(double roomsize, double damping, double width, double level);
      int program_reset();
      int get_internal_bufsize();
      int get_polyphony();
      int set_polyphony(int val);
      float get_gain();
      void set_gain(float g);
      int program_select2(int chan, char* sfont_name, unsigned bank_num, unsigned preset_num);
      int program_select(int chan, unsigned sfont_id, unsigned bank_num, unsigned preset_num);
      int get_program(int chan, unsigned* sfont_id, unsigned* bank_num, unsigned* preset_num);
      int sfont_select(int chan, unsigned int sfont_id);
      int bank_select(int chan, unsigned int bank);
      Preset* get_preset2(char* sfont_name, unsigned banknum, unsigned prognum);
      int get_pitch_wheel_sens(int chan, int* pval);
      int pitch_wheel_sens(int chan, int val);
      int get_pitch_bend(int chan, int* ppitch_bend);
      };

  /*
   *
   * Low level access
   *
   */

/** Get the offset of the bank numbers in a SoundFont. */
int fluid_synth_get_bank_offset(Fluid* synth, int sfont_id);

  /* Those are the default settings for the reverb */
#define FLUID_REVERB_DEFAULT_ROOMSIZE     0.2f
#define FLUID_REVERB_DEFAULT_DAMP         0.0f
#define FLUID_REVERB_DEFAULT_WIDTH        0.5f
#define FLUID_REVERB_DEFAULT_LEVEL        0.9f



  /*
   *
   * Chorus
   *
   */

enum fluid_chorus_mod {
      FLUID_CHORUS_MOD_SINE = 0,
      FLUID_CHORUS_MOD_TRIANGLE = 1
      };

  /* Those are the default settings for the chorus. */
#define FLUID_CHORUS_DEFAULT_N      3
#define FLUID_CHORUS_DEFAULT_LEVEL  2.0f
#define FLUID_CHORUS_DEFAULT_SPEED  0.3f
#define FLUID_CHORUS_DEFAULT_DEPTH  8.0f
#define FLUID_CHORUS_DEFAULT_TYPE   FLUID_CHORUS_MOD_SINE


  /*
   *
   * Synthesis parameters
   *
   */

  /* Flags to choose the interpolation method */
enum fluid_interp {
      /* no interpolation: Fastest, but questionable audio quality */
      FLUID_INTERP_NONE     = 0,
      /* Straight-line interpolation: A bit slower, reasonable audio quality */
      FLUID_INTERP_LINEAR   = 1,
      /* Fourth-order interpolation: Requires 50 % of the whole DSP processing time, good quality
       * Default. */
      FLUID_INTERP_DEFAULT  = 4,
      FLUID_INTERP_4THORDER = 4,
      FLUID_INTERP_7THORDER = 7,
      FLUID_INTERP_HIGHEST  = 7
      };


  /*
   *
   * Tuning
   *
   */

  /** Create a new key-based tuning with given name, number, and
      pitches. The array 'pitches' should have length 128 and contains
      the pitch in cents of every key in cents. However, if 'pitches' is
      NULL, a new tuning is created with the well-tempered scale.

      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param name The name of the tuning
      \param pitch The array of pitch values. The array length has to be 128.
  */
int fluid_synth_create_key_tuning(Fluid* synth, int tuning_bank, int tuning_prog,
				 char* name, double* pitch);

  /** Create a new octave-based tuning with given name, number, and
      pitches.  The array 'pitches' should have length 12 and contains
      derivation in cents from the well-tempered scale. For example, if
      pitches[0] equals -33, then the C-keys will be tuned 33 cents
      below the well-tempered C.

      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param name The name of the tuning
      \param pitch The array of pitch derivations. The array length has to be 12.
  */
int fluid_synth_create_octave_tuning(Fluid* synth, int tuning_bank, int tuning_prog,
				    char* name, double* pitch);

  /** Request a note tuning changes. Both they 'keys' and 'pitches'
      arrays should be of length 'num_pitches'. If 'apply' is non-zero,
      the changes should be applied in real-time, i.e. sounding notes
      will have their pitch updated. 'APPLY' IS CURRENTLY IGNORED. The
      changes will be available for newly triggered notes only.

      \param synth The synthesizer object
      \param tuning_bank The tuning bank number [0-127]
      \param tuning_prog The tuning program number [0-127]
      \param len The length of the keys and pitch arrays
      \param keys The array of keys values.
      \param pitch The array of pitch values.
      \param apply Flag to indicate whether to changes should be applied in real-time.
  */
int Fluidune_notes(Fluid* synth, int tuning_bank, int tuning_prog,
			  int len, int *keys, double* pitch, int apply);

  /** Select a tuning for a channel.

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  \param tuning_bank The tuning bank number [0-127]
  \param tuning_prog The tuning program number [0-127]
  */
int fluid_synth_select_tuning(Fluid* synth, int chan, int tuning_bank, int tuning_prog);

  /** Set the tuning to the default well-tempered tuning on a channel.

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  */
int fluid_synth_reset_tuning(Fluid* synth, int chan);

  /** Start the iteration throught the list of available tunings.

  \param synth The synthesizer object
  */
void Fluiduning_iteration_start(Fluid* synth);


  /** Get the next tuning in the iteration. This functions stores the
      bank and program number of the next tuning in the pointers given as
      arguments.

      \param synth The synthesizer object
      \param bank Pointer to an int to store the bank number
      \param prog Pointer to an int to store the program number
      \returns 1 if there is a next tuning, 0 otherwise
  */
int Fluiduning_iteration_next(Fluid* synth, int* bank, int* prog);

#define fluid_sample_refcount(_sample) ((_sample)->refcount)


/** Sample types */

#define FLUID_SAMPLETYPE_MONO	      1
#define FLUID_SAMPLETYPE_RIGHT	2
#define FLUID_SAMPLETYPE_LEFT	      4
#define FLUID_SAMPLETYPE_LINKED	8
#define FLUID_SAMPLETYPE_ROM	      0x8000

/* Sets the sound data of the sample
 *     Warning : if copy_data is FALSE, data should have 8 unused frames at start
 *     and 8 unused frames at the end.
 */
int fluid_sample_set_sound_data(Sample* sample, short *data,
			       unsigned int nbframes, short copy_data, int rootkey);

/**
 * FluidSynth log levels.
 */
enum fluid_log_level {
      FLUID_PANIC,   // The synth can't function correctly any more
      FLUID_ERR,     // Serious error occurred
      FLUID_WARN,    // Warning
      FLUID_INFO,    // Verbose informational messages
      FLUID_DBG,     // Debugging messages
      LAST_LOG_LEVEL
      };

int fluid_log(int level, const char* fmt, ...);
/*
 *
 *  Utility functions
 */

  /* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD           64

/**
 * @file gen.h
 * @brief Functions and defines for SoundFont generator effects.
 */

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
  GEN_STARTADDROFS,		/**< Sample start address offset (0-32767) */
  GEN_ENDADDROFS,		      /**< Sample end address offset (-32767-0) */
  GEN_STARTLOOPADDROFS,		/**< Sample loop start address offset (-32767-32767) */
  GEN_ENDLOOPADDROFS,		/**< Sample loop end address offset (-32767-32767) */
  GEN_STARTADDRCOARSEOFS,	/**< Sample start address coarse offset (X 32768) */
  GEN_MODLFOTOPITCH,		/**< Modulation LFO to pitch */
  GEN_VIBLFOTOPITCH,		/**< Vibrato LFO to pitch */
  GEN_MODENVTOPITCH,		/**< Modulation envelope to pitch */
  GEN_FILTERFC,			/**< Filter cutoff */
  GEN_FILTERQ,			/**< Filter Q */
  GEN_MODLFOTOFILTERFC,		/**< Modulation LFO to filter cutoff */
  GEN_MODENVTOFILTERFC,		/**< Modulation envelope to filter cutoff */
  GEN_ENDADDRCOARSEOFS,		/**< Sample end address coarse offset (X 32768) */
  GEN_MODLFOTOVOL,		/**< Modulation LFO to volume */
  GEN_UNUSED1,			/**< Unused */
  GEN_CHORUSSEND,		      /**< Chorus send amount */
  GEN_REVERBSEND,		      /**< Reverb send amount */
  GEN_PAN,			      /**< Stereo panning */
  GEN_UNUSED2,			/**< Unused */
  GEN_UNUSED3,			/**< Unused */
  GEN_UNUSED4,			/**< Unused */
  GEN_MODLFODELAY,		/**< Modulation LFO delay */
  GEN_MODLFOFREQ,		      /**< Modulation LFO frequency */
  GEN_VIBLFODELAY,		/**< Vibrato LFO delay */
  GEN_VIBLFOFREQ,		      /**< Vibrato LFO frequency */
  GEN_MODENVDELAY,		/**< Modulation envelope delay */
  GEN_MODENVATTACK,		/**< Modulation envelope attack */
  GEN_MODENVHOLD,		      /**< Modulation envelope hold */
  GEN_MODENVDECAY,		/**< Modulation envelope decay */
  GEN_MODENVSUSTAIN,		/**< Modulation envelope sustain */
  GEN_MODENVRELEASE,		/**< Modulation envelope release */
  GEN_KEYTOMODENVHOLD,		/**< Key to modulation envelope hold */
  GEN_KEYTOMODENVDECAY,		/**< Key to modulation envelope decay */
  GEN_VOLENVDELAY,		/**< Volume envelope delay */
  GEN_VOLENVATTACK,		/**< Volume envelope attack */
  GEN_VOLENVHOLD,		      /**< Volume envelope hold */
  GEN_VOLENVDECAY,		/**< Volume envelope decay */
  GEN_VOLENVSUSTAIN,		/**< Volume envelope sustain */
  GEN_VOLENVRELEASE,		/**< Volume envelope release */
  GEN_KEYTOVOLENVHOLD,		/**< Key to volume envelope hold */
  GEN_KEYTOVOLENVDECAY,		/**< Key to volume envelope decay */
  GEN_INSTRUMENT,		      /**< Instrument ID (shouldn't be set by user) */
  GEN_RESERVED1,		      /**< Reserved */
  GEN_KEYRANGE,			/**< MIDI note range */
  GEN_VELRANGE,			/**< MIDI velocity range */
  GEN_STARTLOOPADDRCOARSEOFS,	/**< Sample start loop address coarse offset (X 32768) */
  GEN_KEYNUM,			/**< Fixed MIDI note number */
  GEN_VELOCITY,			/**< Fixed MIDI velocity value */
  GEN_ATTENUATION,		/**< Initial volume attenuation */
  GEN_RESERVED2,		      /**< Reserved */
  GEN_ENDLOOPADDRCOARSEOFS,	/**< Sample end loop address coarse offset (X 32768) */
  GEN_COARSETUNE,		      /**< Coarse tuning */
  GEN_FINETUNE,			/**< Fine tuning */
  GEN_SAMPLEID,			/**< Sample ID (shouldn't be set by user) */
  GEN_SAMPLEMODE,		      /**< Sample mode flags */
  GEN_RESERVED3,		      /**< Reserved */
  GEN_SCALETUNE,		      /**< Scale tuning */
  GEN_EXCLUSIVECLASS,		/**< Exclusive class number */
  GEN_OVERRIDEROOTKEY,		/**< Sample root note override */

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,			/**< Pitch (NOTE: Not a real SoundFont generator) */
  GEN_LAST			      /**< Value defines the count of generators (#fluid_gen_type) */
      };


/**
 * SoundFont generator structure.
 */
struct Generator {
      unsigned char flags; /**< Is the generator set or not (#fluid_gen_flags) */
      double val;          /**< The nominal value           */
      double mod;          /**< Change by modulators        */
      double nrpn;         /**< Change by NRPN messages     */
      };

/**
 * Enum value for 'flags' field of #_Generator (not really flags).
 */
enum fluid_gen_flags {
      GEN_UNUSED,		/**< Generator value is not set */
      GEN_SET,		/**< Generator value is set     */
      GEN_ABS_NRPN	/**< DOCME                      */
      };

int fluid_gen_set_default_values(Generator* gen);
  /*
   *  The interface to the synthesizer's voices
   *  Examples on using them can be found in fluid_defsfont.c
   */

  /* for fluid_voice_add_mod */
enum fluid_voice_add_mod {
      FLUID_VOICE_OVERWRITE,
      FLUID_VOICE_ADD,
      FLUID_VOICE_DEFAULT
      };

  /* Add a modulator to a voice (SF2.1 only). */
void fluid_voice_add_mod(Voice* voice, Mod* mod, int mode);

/* Disable FPE exception check */
#define fluid_check_fpe(expl)

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);

enum fluid_midi_control_change {
      BANK_SELECT_MSB = 0x00,
      MODULATION_MSB = 0x01,
      BREATH_MSB = 0x02,
      FOOT_MSB = 0x04,
      PORTAMENTO_TIME_MSB = 0x05,
      DATA_ENTRY_MSB = 0x06,
      VOLUME_MSB = 0x07,
      BALANCE_MSB = 0x08,
      PAN_MSB = 0x0A,
      EXPRESSION_MSB = 0x0B,
      EFFECTS1_MSB = 0x0C,
      EFFECTS2_MSB = 0x0D,
      GPC1_MSB = 0x10, /* general purpose controller */
      GPC2_MSB = 0x11,
      GPC3_MSB = 0x12,
      GPC4_MSB = 0x13,
      BANK_SELECT_LSB = 0x20,
      MODULATION_WHEEL_LSB = 0x21,
      BREATH_LSB = 0x22,
      FOOT_LSB = 0x24,
      PORTAMENTO_TIME_LSB = 0x25,
      DATA_ENTRY_LSB = 0x26,
      VOLUME_LSB = 0x27,
      BALANCE_LSB = 0x28,
      PAN_LSB = 0x2A,
      EXPRESSION_LSB = 0x2B,
      EFFECTS1_LSB = 0x2C,
      EFFECTS2_LSB = 0x2D,
      GPC1_LSB = 0x30,
      GPC2_LSB = 0x31,
      GPC3_LSB = 0x32,
      GPC4_LSB = 0x33,
      SUSTAIN_SWITCH = 0x40,
      PORTAMENTO_SWITCH = 0x41,
      SOSTENUTO_SWITCH = 0x42,
      SOFT_PEDAL_SWITCH = 0x43,
      LEGATO_SWITCH = 0x45,
      HOLD2_SWITCH = 0x45,
      SOUND_CTRL1 = 0x46,
      SOUND_CTRL2 = 0x47,
      SOUND_CTRL3 = 0x48,
      SOUND_CTRL4 = 0x49,
      SOUND_CTRL5 = 0x4A,
      SOUND_CTRL6 = 0x4B,
      SOUND_CTRL7 = 0x4C,
      SOUND_CTRL8 = 0x4D,
      SOUND_CTRL9 = 0x4E,
      SOUND_CTRL10 = 0x4F,
      GPC5 = 0x50,
      GPC6 = 0x51,
      GPC7 = 0x52,
      GPC8 = 0x53,
      PORTAMENTO_CTRL = 0x54,
      EFFECTS_DEPTH1 = 0x5B,
      EFFECTS_DEPTH2 = 0x5C,
      EFFECTS_DEPTH3 = 0x5D,
      EFFECTS_DEPTH4 = 0x5E,
      EFFECTS_DEPTH5 = 0x5F,
      DATA_ENTRY_INCR = 0x60,
      DATA_ENTRY_DECR = 0x61,
      NRPN_LSB = 0x62,
      NRPN_MSB = 0x63,
      RPN_LSB = 0x64,
      RPN_MSB = 0x65,
      ALL_SOUND_OFF = 0x78,
      ALL_CTRL_OFF = 0x79,
      LOCAL_CONTROL = 0x7A,
      ALL_NOTES_OFF = 0x7B,
      OMNI_OFF = 0x7C,
      OMNI_ON = 0x7D,
      POLY_OFF = 0x7E,
      POLY_ON = 0x7F
      };

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

class Channel
      {
      unsigned int sfontnum;
      unsigned int banknum;
      unsigned int prognum;
      Preset* _preset;

   public:
      int channum;
      Fluid* synth;
      short key_pressure;
      short channel_pressure;
      short pitch_bend;
      short pitch_wheel_sensitivity;

      short cc[128];          // controller values

      /* cached values of last MSB values of MSB/LSB controllers */
      unsigned char bank_msb;
      int interp_method;

      /* the micro-tuning */
      Tuning* tuning;

      /* NRPN system */
      short nrpn_select;

      /* The values of the generators, set by NRPN messages, or by
       * fluid_synth_set_gen(), are cached in the channel so they can be
       * applied to future notes. They are copied to a voice's generators
       * in fluid_voice_init(), wihich calls fluid_gen_init().  */

      fluid_real_t gen[GEN_LAST];

      /* By default, the NRPN values are relative to the values of the
       * generators set in the SoundFont. For example, if the NRPN
       * specifies an attack of 100 msec then 100 msec will be added to the
       * combined attack time of the sound font and the modulators.
       *
       * However, it is useful to be able to specify the generator value
       * absolutely, completely ignoring the generators of the sound font
       * and the values of modulators. The gen_abs field, is a boolean
       * flag indicating whether the NRPN value is absolute or not.
       */
      char gen_abs[GEN_LAST];

   public:
      Channel(Fluid* synth, int num);

      void setTuning(Tuning* t)           { tuning = t; }
      bool hasTuning() const              { return tuning != 0; }
      Tuning* getTuning() const           { return tuning; }
      bool sustained() const              { return cc[SUSTAIN_SWITCH] >= 64; }
      void setGen(int n, float v, char a) { gen[n] = v; gen_abs[n] = a; }
      float getGen(int n) const           { return gen[n]; }
      char getGenAbs(int n) const         { return gen_abs[n]; }
      void init();
      void initCtrl();
      void setCC(int n, int val)          { cc[n] = val; }
      void reset();
      void setPreset(Preset* p);
      Preset* preset() const              { return _preset;  }
      unsigned int getSfontnum() const    { return sfontnum; }
      void setSfontnum(unsigned int s)    { sfontnum = s;    }
      unsigned int getBanknum() const     { return banknum;  }
      void setBanknum(unsigned int b)     { banknum = b;     }
      void setPrognum(int p)              { prognum = p;     }
      int getPrognum() const              { return prognum;  }
      int setcc(int ctrl, int val);
      int pitchBend(int val);
      int getPitchBend() const            { return pitch_bend; }
      int pitchWheelSens(int val);
      int getCC(int num);
      int getNum() const                  { return channum;    }
      void setInterpMethod(int m)         { interp_method = m; }
      int getInterpMethod() const         { return interp_method; }
      };

#define NO_CHANNEL      0xff

enum fluid_voice_status {
	FLUID_VOICE_CLEAN,
	FLUID_VOICE_ON,
	FLUID_VOICE_SUSTAINED,
	FLUID_VOICE_OFF
      };

/*
 * envelope data
 */
struct fluid_env_data_t {
	unsigned int count;
	fluid_real_t coeff;
	fluid_real_t incr;
	fluid_real_t min;
	fluid_real_t max;
      };

/* Indices for envelope tables */
enum fluid_voice_envelope_index_t {
	FLUID_VOICE_ENVDELAY,
	FLUID_VOICE_ENVATTACK,
	FLUID_VOICE_ENVHOLD,
	FLUID_VOICE_ENVDECAY,
	FLUID_VOICE_ENVSUSTAIN,
	FLUID_VOICE_ENVRELEASE,
	FLUID_VOICE_ENVFINISHED,
	FLUID_VOICE_ENVLAST
      };

/*
 * interpolation data
 */
struct fluid_interp_coeff_t {
      fluid_real_t a0, a1, a2, a3;
      };

//---------------------------------------------------------
//   Mod
//---------------------------------------------------------

struct Mod
      {
      unsigned char dest;
      unsigned char src1;
      unsigned char flags1;
      unsigned char src2;
      unsigned char flags2;
      double amount;
      /* The 'next' field allows to link modulators into a list.  It is
       * not used in fluid_voice.c, there each voice allocates memory for a
       * fixed number of modulators.  Since there may be a huge number of
       * different zones, this is more efficient.
       */
      Mod * next;

      void clone(Mod* mod) const;
      };

/* Flags telling the polarity of a modulator.  Compare with SF2.01
   section 8.2. Note: The numbers of the bits are different!  (for
   example: in the flags of a SF modulator, the polarity bit is bit
   nr. 9) */

enum fluid_mod_flags {
      FLUID_MOD_POSITIVE = 0,
      FLUID_MOD_NEGATIVE = 1,
      FLUID_MOD_UNIPOLAR = 0,
      FLUID_MOD_BIPOLAR  = 2,
      FLUID_MOD_LINEAR   = 0,
      FLUID_MOD_CONCAVE  = 4,
      FLUID_MOD_CONVEX   = 8,
      FLUID_MOD_SWITCH   = 12,
      FLUID_MOD_GC       = 0,
      FLUID_MOD_CC       = 16
      };

/* Flags telling the source of a modulator.  This corresponds to
 * SF2.01 section 8.2.1 */

enum fluid_mod_src {
      FLUID_MOD_NONE             = 0,
      FLUID_MOD_VELOCITY         = 2,
      FLUID_MOD_KEY              = 3,
      FLUID_MOD_KEYPRESSURE      = 10,
      FLUID_MOD_CHANNELPRESSURE  = 13,
      FLUID_MOD_PITCHWHEEL       = 14,
      FLUID_MOD_PITCHWHEELSENS   = 16
      };

/* Allocates memory for a new modulator */
Mod * fluid_mod_new(void);

/* Frees the modulator */
void fluid_mod_delete(Mod * mod);

void fluid_mod_set_source1(Mod* mod, int src, int flags);
void fluid_mod_set_source2(Mod* mod, int src, int flags);
void fluid_mod_set_dest(Mod* mod, int dst);
void fluid_mod_set_amount(Mod* mod, double amount);

int fluid_mod_get_source1(Mod* mod);
int fluid_mod_get_flags1(Mod* mod);
int fluid_mod_get_source2(Mod* mod);
int fluid_mod_get_flags2(Mod* mod);
int fluid_mod_get_dest(Mod* mod);
double fluid_mod_get_amount(Mod* mod);


/* Determines, if two modulators are 'identical' (all parameters
   except the amount match) */
int Modest_identity(Mod * mod1, Mod * mod2);

fluid_real_t fluid_mod_get_value(Mod* mod, Channel* chan, Voice* voice);
void fluid_dump_modulator(Mod * mod);

#define fluid_mod_has_source(mod,cc,ctrl)  \
( ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) != 0) && (cc != 0)) \
   || ((((mod)->src1 == ctrl) && (((mod)->flags1 & FLUID_MOD_CC) == 0) && (cc == 0)))) \
|| ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) != 0) && (cc != 0)) \
    || ((((mod)->src2 == ctrl) && (((mod)->flags2 & FLUID_MOD_CC) == 0) && (cc == 0)))))

#define fluid_mod_has_dest(mod,gen)  ((mod)->dest == gen)

/*
 *  phase
 */

#define FLUID_INTERP_BITS        8
#define FLUID_INTERP_BITS_MASK   0xff000000
#define FLUID_INTERP_BITS_SHIFT  24
#define FLUID_INTERP_MAX         256

#define FLUID_FRACT_MAX ((double)4294967296.0)

/* fluid_phase_t
* Purpose:
* Playing pointer for voice playback
*
* When a sample is played back at a different pitch, the playing pointer in the
* source sample will not advance exactly one sample per output sample.
* This playing pointer is implemented using fluid_phase_t.
* It is a 64 bit number. The higher 32 bits contain the 'index' (number of
* the current sample), the lower 32 bits the fractional part.
* Access is possible in two ways:
* -through the 64 bit part 'b64', if the architecture supports 64 bit integers
* -through 'index' and 'fract'
* Note: b64 and index / fract share the same memory location!
*/
typedef union {
    struct{
	/* Note, that the two 32-bit ints form a 64-bit int! */
#ifdef WORDS_BIGENDIAN
	sint32 index;
	uint32 fract;
#else
	uint32 fract;
	sint32 index;
#endif
    } b32;
#ifdef USE_LONGLONG
    long long b64;
#endif
} fluid_phase_t;

/* Purpose:
 * Set a to b.
 * a: fluid_phase_t
 * b: fluid_phase_t
 */
#ifdef USE_LONGLONG
#define fluid_phase_set(a,b) a=b;
#else
#define fluid_phase_set(a, b) { \
  (a).b32.fract = (b).b32.fract; \
  (a).b32.index = (b).b32.index; \
}
#endif

#define fluid_phase_set_int(a, b)   { \
  (a).b32.index = (sint32) (b); \
  (a).b32.fract = 0; \
}

/* Purpose:
 * Sets the phase a to a phase increment given in b.
 * For example, assume b is 0.9. After setting a to it, adding a to
 * the playing pointer will advance it by 0.9 samples. */
#define fluid_phase_set_float(a, b)   { \
  (a).b32.index = (sint32) (b); \
  (a).b32.fract = (uint32) (((double)(b) - (double)((a).b32.index)) * (double)FLUID_FRACT_MAX); \
}

/* Purpose:
 * Return the index and the fractional part, respectively. */
#define fluid_phase_index(_x) \
  ((int)(_x).b32.index)
#define fluid_phase_fract(_x) \
  ((_x).b32.fract)

/* Purpose:
 * Takes the fractional part of the argument phase and
 * calculates the corresponding position in the interpolation table.
 * The fractional position of the playing pointer is calculated with a quite high
 * resolution (32 bits). It would be unpractical to keep a set of interpolation
 * coefficients for each possible fractional part...
 */
#define fluid_phase_fract_to_tablerow(_x) \
  ((int)(((_x).b32.fract & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT))

#define fluid_phase_double(_x) \
  ((double)((_x).b32.index) + ((double)((_x).b32.fract) / FLUID_FRACT_MAX))

/* Purpose:
 * Advance a by a step of b (both are fluid_phase_t).
 */
#ifdef USE_LONGLONG
#define fluid_phase_incr(a, b) (a).b64 += (b).b64;
#else
/* The idea to use (a).index += (b).index + ((a).fract < (b).fract) to
   handle wrap-arounds comes from Mozilla's macros to handle 64-bit
   integer on 32-bit platforms. Header prlong.h in the NSPR
   library. www.mozilla.org. */
#define fluid_phase_incr(a, b)  { \
  (a).b32.fract += (b).b32.fract; \
  (a).b32.index += (b).b32.index + ((a).b32.fract < (b).b32.fract); \
}
#endif

/* Purpose:
 * Subtract b from a (both are fluid_phase_t).
 */
#ifdef USE_LONGLONG
#define fluid_phase_decr(a, b) a-=b;
#else
#define fluid_phase_decr(a, b) { \
  (a).b32.index -= b.b32.index - ((a).b32.fract < (b).b32.fract); \
  (a).b32.fract -= b.b32.fract; \
}
#endif

/* Purpose:
 * Subtract b samples from a.
 */
#define fluid_phase_sub_int(a, b) { (a).b32.index -= b; }

/* Purpose:
 * The playing pointer is _phase. How many output samples are produced, until the point _p1 in the sample is reached,
 * if _phase advances in steps of _incr?
 */
#define fluid_phase_steps(_phase,_index,_incr) \
  (int)(((double)(_index) - fluid_phase_double(_phase)) / (double)_incr)

/* Purpose:
 * Creates the expression a.index++.
 * It is slightly different, when USE_LONGLONG is turned on. */
#define fluid_phase_index_plusplus(a) (((a).b32.index)++)

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice
      {
   public:
	unsigned int id;                /* the id is incremented for every new noteon.
					           it's used for noteoff's  */
	unsigned char status;
	unsigned char chan;             /* the channel number, quick access for channel messages */
	unsigned char key;              /* the key, quick acces for noteoff */
	unsigned char vel;              /* the velocity */
	Channel* channel;
	Generator gen[GEN_LAST];
	Mod mod[FLUID_NUM_MOD];
	int mod_count;
	int has_looped;                 /* Flag that is set as soon as the first loop is completed. */
	Sample* sample;
	int check_sample_sanity_flag;   /* Flag that initiates, that sample-related parameters
					           have to be checked. */
	/* basic parameters */
	fluid_real_t output_rate;        /* the sample rate of the synthesizer */

	unsigned int start_time;
	unsigned int ticks;

	fluid_real_t amp;                /* the linear amplitude */
	fluid_phase_t phase;             /* the phase of the sample wave */

	/* basic parameters */
	fluid_real_t pitch;              /* the pitch in midicents */
	fluid_real_t attenuation;        /* the attenuation in centibels */
	fluid_real_t min_attenuation_cB; /* Estimate on the smallest possible attenuation
					  * during the lifetime of the voice */
	fluid_real_t root_pitch;

	/* sample and loop start and end points (offset in sample memory).  */
	int start;
	int end;
	int loopstart;
	int loopend;

	/* master gain */
	fluid_real_t synth_gain;

	/* vol env */
	fluid_env_data_t volenv_data[FLUID_VOICE_ENVLAST];
	unsigned int volenv_count;
	int volenv_section;
	fluid_real_t volenv_val;
	fluid_real_t amplitude_that_reaches_noise_floor_nonloop;
	fluid_real_t amplitude_that_reaches_noise_floor_loop;

	/* mod env */
	fluid_env_data_t modenv_data[FLUID_VOICE_ENVLAST];
	unsigned int modenv_count;
	int modenv_section;
	fluid_real_t modenv_val;         /* the value of the modulation envelope */
	fluid_real_t modenv_to_fc;
	fluid_real_t modenv_to_pitch;

	/* mod lfo */
	fluid_real_t modlfo_val;          /* the value of the modulation LFO */
	unsigned int modlfo_delay;       /* the delay of the lfo in samples */
	fluid_real_t modlfo_incr;         /* the lfo frequency is converted to a per-buffer increment */
	fluid_real_t modlfo_to_fc;
	fluid_real_t modlfo_to_pitch;
	fluid_real_t modlfo_to_vol;

	/* vib lfo */
	fluid_real_t viblfo_val;        /* the value of the vibrato LFO */
	unsigned int viblfo_delay;      /* the delay of the lfo in samples */
	fluid_real_t viblfo_incr;       /* the lfo frequency is converted to a per-buffer increment */
	fluid_real_t viblfo_to_pitch;

	/* resonant filter */
	fluid_real_t fres;              /* the resonance frequency, in cents (not absolute cents) */
	fluid_real_t last_fres;         /* Current resonance frequency of the IIR filter */
	/* Serves as a flag: A deviation between fres and last_fres */
	/* indicates, that the filter has to be recalculated. */
	fluid_real_t q_lin;             /* the q-factor on a linear scale */
	fluid_real_t filter_gain;       /* Gain correction factor, depends on q */
	fluid_real_t hist1, hist2;      /* Sample history for the IIR filter */
	int filter_startup;             /* Flag: If set, the filter will be set directly.
					   Else it changes smoothly. */

	/* filter coefficients */
	/* The coefficients are normalized to a0. */
	/* b0 and b2 are identical => b02 */
	fluid_real_t b02;              /* b0 / a0 */
	fluid_real_t b1;              /* b1 / a0 */
	fluid_real_t a1;              /* a0 / a0 */
	fluid_real_t a2;              /* a1 / a0 */

	fluid_real_t b02_incr;
	fluid_real_t b1_incr;
	fluid_real_t a1_incr;
	fluid_real_t a2_incr;
	int filter_coeff_incr_count;

	/* pan */
	fluid_real_t pan;
	fluid_real_t amp_left;
	fluid_real_t amp_right;

	/* reverb */
	fluid_real_t reverb_send;
	fluid_real_t amp_reverb;

	/* chorus */
	fluid_real_t chorus_send;
	fluid_real_t amp_chorus;

	/* interpolation method, as in fluid_interp in fluidsynth.h */
	int interp_method;

	/* for debugging */
	int debug;
	double ref;

   public:
      Voice(fluid_real_t output_rate);
      Channel* get_channel() const    { return channel; }
      void voice_start();
      int voice_off();
      void init(Sample*, Channel*, int key, int vel, unsigned int id,
         unsigned int start_time, fluid_real_t _gain);
      void gen_incr(int i, float val);
      void gen_set(int i, float val);
      float gen_get(int gen);
      unsigned int get_id() const { return id; }
      bool isPlaying()            { return ((status == FLUID_VOICE_ON) || (status == FLUID_VOICE_SUSTAINED)); }
      void set_gain(fluid_real_t gain);
      void set_param(int gen, fluid_real_t nrpn_value, int abs);

      // Update all the synthesis parameters, which depend on generator
      // 'gen'. This is only necessary after changing a generator of an
      // already operating voice.  Most applications will not need this
      // function.

      void update_param(int gen);

      fluid_real_t GEN(int n) {
         return fluid_real_t(gen[n].val) + fluid_real_t(gen[n].mod)
            + fluid_real_t(gen[n].nrpn);
         }
      void modulate_all();
      void modulate(int _cc, int _ctrl);
      fluid_real_t get_lower_boundary_for_attenuation();
      void check_sample_sanity();
      void noteoff();
      void kill_excl();
      int calculate_hold_decay_buffers(int gen_base, int gen_key2base, int is_decay);
      void calculate_runtime_synthesis_parameters();


      /* A voice is 'ON', if it has not yet received a noteoff
       * event. Sending a noteoff event will advance the envelopes to
       * section 5 (release).
       */
      bool RELEASED() const    { return chan == NO_CHANNEL; }
      bool SUSTAINED() const   { return status == FLUID_VOICE_SUSTAINED; }
      bool ON() const          { return (status == FLUID_VOICE_ON) && (volenv_section < FLUID_VOICE_ENVRELEASE); }
      bool AVAILABLE() const   { return (status == FLUID_VOICE_CLEAN) || (status == FLUID_VOICE_OFF); }
      int SAMPLEMODE() const   { return ((int)gen[GEN_SAMPLEMODE].val); }

      void write(fluid_real_t* l, fluid_real_t* r, fluid_real_t* reverb_buf, fluid_real_t* chorus_buf);
      };

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)
extern void fluid_voice_config();

}  // namespace Fluid

#endif  // __FLUID_S_H__
