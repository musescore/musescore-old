/*
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
#include "rev.h"

namespace FluidS {

class Voice;
class SFont;
class Preset;
class Sample;
class Channel;
class Mod;
class Reverb;
class Chorus;

#define FLUID_BUFSIZE           256  // 64
#define FLUID_NUM_PROGRAMS      129

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

//---------------------------------------------------------
//   BankOffset
//---------------------------------------------------------

struct BankOffset {
      int sfont_id;
      int offset;
      };

//---------------------------------------------------------
//   Tuning
//---------------------------------------------------------

class Tuning {
      QString _name;
      int bank;
      int prog;
      double pitch[128];      // the pitch of every key, in cents

    public:
      Tuning(const QString& name, int bank, int prog);
      int getBank() const            { return bank; }
      int getProg() const            { return prog; }
      double getPitch(int k) const   { return pitch[k]; }
      QString name() const           { return _name; }
      void setName(const QString& p) { _name = p;  }
      void setKey(int k, double p)   { pitch[k] = p; }

      void setPitch(int key, double pitch);
      void setOctave(double* pitch_deriv);
      void setAll(double* pitch);
      };

//---------------------------------------------------------
//   Fluid
//---------------------------------------------------------

class Fluid : public Synth {
      static const int SILENT_BLOCKS = 32;
      int silentBlocks;

      QList<SFont*> sfonts;               // the loaded soundfonts
      QList<BankOffset*> bank_offsets;    // the offsets of the soundfont banks
      QList<Voice*> freeVoices;           // unused synthesis processes
      QList<Voice*> activeVoices;         // active synthesis processes
      QString _error;                     // last error message

      static bool initialized;
      static void init();


   protected:
      double sample_rate;                 // The sample rate
      int state;                          // the synthesizer state

      unsigned int sfont_id;

      double gain;                        // master gain
      QList<Channel*> channel;            // the channels

      unsigned int noteid;                // the id is incremented for every new note. it's used for noteoff's

      float* left_buf;
      float* right_buf;
      float* fx_buf[2];

      Reverb* reverb;
      Chorus* chorus;
      int cur;                            // the current sample in the audio buffers to be output

      Tuning*** tuning;                   // 128 banks of 128 programs for the tunings
      Tuning* cur_tuning;                 // current tuning in the iteration

   public:
      Fluid();
      ~Fluid();
      virtual void init(int sr);
      virtual bool loadSoundFont(const QString& s) { return sfload(s, true); }
      virtual void play(const Event&);
      virtual const MidiPatch* getPatchInfo(bool onlyDrums, const MidiPatch*) const;

      bool log(const char* fmt, ...);

      bool set_reverb_preset(int num);
      void one_block();

      Preset* get_preset(unsigned int sfontnum, unsigned int banknum, unsigned int prognum);
      Preset* find_preset(unsigned int banknum, unsigned int prognum);
      void all_notes_off(int chan);
      void all_sounds_off(int chan);
      void modulate_voices(int chan, int is_cc, int ctrl);
      void modulate_voices_all(int chan);
      void damp_voices(int chan);
      int kill_voice(Voice * voice);
      void print_voice();

      /** This function assures that every MIDI channels has a valid preset
       *  (NULL is okay). This function is called after a SoundFont is
       *  unloaded or reloaded. */
      void update_presets();

      BankOffset* get_bank_offset0(int sfont_id) const;
      void remove_bank_offset(int sfont_id);

      int get_cc(int chan, int num);
      void system_reset();
      void program_change(int chan, int prognum);
      int tuning_iteration_next(int* bank, int* prog);
      void tuning_iteration_start();

      int get_bank_offset(int sfont_id);
      int set_bank_offset(int sfont_id, int offset);
//      int stop(unsigned int id);
      void set_gen2(int chan, int param, float value, int absolute, int normalized);
      float get_gen(int chan, int param);
      void set_gen(int chan, int param, float value);
      void set_interp_method(int chan, int interp_method);

      Preset* get_channel_preset(int chan);
      SFont* get_sfont_by_name(const QString& name);
      SFont* get_sfont_by_id(unsigned int id);
      SFont* get_sfont(unsigned int num)  { return sfonts[num];   }
      int sfcount() const                 { return sfonts.size(); }
      void remove_sfont(SFont* sf);
      int add_sfont(SFont* sf);
      int sfreload(unsigned int id);
      bool sfunload(unsigned int id, int reset_presets);
      int sfload(const QString& filename, int reset_presets);

      void start_voice(Voice* voice);
      Voice* alloc_voice(unsigned id, Sample* sample, int chan, int key, int vel, double vt);
      Voice* free_voice_by_kill();

      virtual void process(unsigned len, float* lout, float* rout, int stride);

      void set_chorus(int nr, double level, double speed, double depth_ms, int type);
      void set_reverb(double roomsize, double damping, double width, double level);
      void program_reset();

      float get_gain()          { return gain;  }
      void set_gain(float g);
      bool program_select2(int chan, char* sfont_name, unsigned bank_num, unsigned preset_num);
      bool program_select(int chan, unsigned sfont_id, unsigned bank_num, unsigned preset_num);
      void get_program(int chan, unsigned* sfont_id, unsigned* bank_num, unsigned* preset_num);
      void sfont_select(int chan, unsigned int sfont_id);
      void bank_select(int chan, unsigned int bank);
      Preset* get_preset(char* sfont_name, unsigned banknum, unsigned prognum);
      void get_pitch_wheel_sens(int chan, int* pval);
      void pitch_wheel_sens(int chan, int val);
      void get_pitch_bend(int chan, int* ppitch_bend);

      void freeVoice(Voice* v);

      void reset_tuning(int chan);
      bool select_tuning(int chan, int bank, int prog);
      bool tune_notes(int bank, int prog, int len, int *key, double* pitch);
      Tuning* get_tuning(int bank, int prog);
      void create_octave_tuning(int bank, int prog, char* name, double* pitch);
      void create_key_tuning(int bank, int prog, char* name, double* pitch);
      Tuning* create_tuning(int bank, int prog, char* name);

      QString error() const { return _error; }

      friend class Voice;
      };

  /*
   *
   * Low level access
   *
   */

/** Get the offset of the bank numbers in a SoundFont. */
int fluid_synth_get_bank_offset(Fluid* synth, int sfont_id);

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

void fluid_gen_set_default_values(Generator* gen);
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

      float gen[GEN_LAST];

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
      void setcc(int ctrl, int val);
      void pitchBend(int val);
      int getPitchBend() const            { return pitch_bend; }
      void pitchWheelSens(int val);
      int getCC(int num);
      int getNum() const                  { return channum;    }
      void setInterpMethod(int m)         { interp_method = m; }
      int getInterpMethod() const         { return interp_method; }
      };

#define NO_CHANNEL      0xff

enum fluid_voice_status {
	FLUID_VOICE_OFF,
	FLUID_VOICE_ON,
	FLUID_VOICE_SUSTAINED
      };

/*
 * envelope data
 */
struct fluid_env_data_t {
	unsigned int count;
	float coeff;
	float incr;
	float min;
	float max;
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
      float a0, a1, a2, a3;
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

float fluid_mod_get_value(Mod* mod, Channel* chan, Voice* voice);
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

//---------------------------------------------------------
//   Phase
/* Purpose:
* Playing pointer for voice playback
*
* When a sample is played back at a different pitch, the playing pointer in the
* source sample will not advance exactly one sample per output sample.
* This playing pointer is implemented using Phase.
* It is a 64 bit number. The higher 32 bits contain the 'index' (number of
* the current sample), the lower 32 bits the fractional part.
* Access is possible in two ways:
* -through the 64 bit part 'b64', if the architecture supports 64 bit integers
* -through 'index' and 'fract'
* Note: b64 and index / fract share the same memory location!
*/

class Phase {
   public:
      union {
            struct {
#if Q_BYTE_ORDER == Q_BIG_ENDIAN
                  qint32 _index;
                  quint32 _fract;
#else
                  quint32 _fract;
                  qint32 _index;
#endif
                  };
            qint64 b64;
            };
      void operator+=(const Phase& p) { b64 += p.b64; }
      void operator-=(int b)          { _index -= b;  }

      void setInt(qint32 b)           { _index = b; _fract = 0; }
      int index() const               { return _index; }
      int fract() const               { return _fract; }
      void setFract(int v)            { _fract = v; }
      };

/* Purpose:
 * Sets the phase a to a phase increment given in b.
 * For example, assume b is 0.9. After setting a to it, adding a to
 * the playing pointer will advance it by 0.9 samples. */
#define fluid_phase_set_float(a, b)   { \
  (a)._index = (qint32) (b); \
  (a)._fract = (quint32) (((double)(b) - (double)((a)._index)) * (double)FLUID_FRACT_MAX); \
}

/* Purpose:
 * Takes the fractional part of the argument phase and
 * calculates the corresponding position in the interpolation table.
 * The fractional position of the playing pointer is calculated with a quite high
 * resolution (32 bits). It would be unpractical to keep a set of interpolation
 * coefficients for each possible fractional part...
 */
#define fluid_phase_fract_to_tablerow(_x) \
  ((int)(((_x).fract() & FLUID_INTERP_BITS_MASK) >> FLUID_INTERP_BITS_SHIFT))

#define fluid_phase_double(_x) \
  ((double)((_x).index()) + ((double)((_x).fract()) / FLUID_FRACT_MAX))

/* Purpose:
 * The playing pointer is _phase. How many output samples are produced, until the point _p1 in the sample is reached,
 * if _phase advances in steps of _incr?
 */
#define fluid_phase_steps(_phase,_idx,_incr) \
  (int)(((double)(_idx) - fluid_phase_double(_phase)) / (double)_incr)

/* Purpose:
 * Creates the expression a.index++.
*/
#define fluid_phase_index_plusplus(a) (((a)._index)++)

//---------------------------------------------------------
//   Voice
//---------------------------------------------------------

class Voice
      {
      Fluid* _fluid;
      double _noteTuning;             // +/- in midicent

   public:
	unsigned int id;                // the id is incremented for every new noteon.
					           // it's used for noteoff's
	unsigned char status;
	unsigned char chan;             // the channel number, quick access for channel messages
	unsigned char key;              // the key, quick acces for noteoff
	unsigned char vel;              // the velocity

	Channel* channel;
	Generator gen[GEN_LAST];
	Mod mod[FLUID_NUM_MOD];
	int mod_count;
	int has_looped;                 /* Flag that is set as soon as the first loop is completed. */
	Sample* sample;
	int check_sample_sanity_flag;   /* Flag that initiates, that sample-related parameters
					           have to be checked. */
	unsigned int ticks;

	float amp;                /* the linear amplitude */
	Phase phase;                     // the phase of the sample wave

	/* basic parameters */
	float pitch;              /* the pitch in midicents */
	float attenuation;        /* the attenuation in centibels */
	float min_attenuation_cB; /* Estimate on the smallest possible attenuation
					          * during the lifetime of the voice */
	float root_pitch;

	/* sample and loop start and end points (offset in sample memory).  */
	int start;
	int end;
	int loopstart;
	int loopend;

	/* master gain */
	float synth_gain;

	/* vol env */
	fluid_env_data_t volenv_data[FLUID_VOICE_ENVLAST];
	unsigned int volenv_count;
	int volenv_section;
	float volenv_val;
	float amplitude_that_reaches_noise_floor_nonloop;
	float amplitude_that_reaches_noise_floor_loop;

	/* mod env */
	fluid_env_data_t modenv_data[FLUID_VOICE_ENVLAST];
	unsigned int modenv_count;
	int modenv_section;
	float modenv_val;         /* the value of the modulation envelope */
	float modenv_to_fc;
	float modenv_to_pitch;

	/* mod lfo */
	float modlfo_val;          /* the value of the modulation LFO */
	unsigned int modlfo_delay;       /* the delay of the lfo in samples */
	float modlfo_incr;         /* the lfo frequency is converted to a per-buffer increment */
	float modlfo_to_fc;
	float modlfo_to_pitch;
	float modlfo_to_vol;

	/* vib lfo */
	float viblfo_val;        /* the value of the vibrato LFO */
	unsigned int viblfo_delay;      /* the delay of the lfo in samples */
	float viblfo_incr;       /* the lfo frequency is converted to a per-buffer increment */
	float viblfo_to_pitch;

	/* resonant filter */
	float fres;              /* the resonance frequency, in cents (not absolute cents) */
	float last_fres;         /* Current resonance frequency of the IIR filter */
	/* Serves as a flag: A deviation between fres and last_fres */
	/* indicates, that the filter has to be recalculated. */
	float q_lin;             /* the q-factor on a linear scale */
	float filter_gain;       /* Gain correction factor, depends on q */
	float hist1, hist2;      /* Sample history for the IIR filter */
	int filter_startup;             /* Flag: If set, the filter will be set directly.
					   Else it changes smoothly. */

	/* filter coefficients */
	/* The coefficients are normalized to a0. */
	/* b0 and b2 are identical => b02 */
	float b02;              /* b0 / a0 */
	float b1;              /* b1 / a0 */
	float a1;              /* a0 / a0 */
	float a2;              /* a1 / a0 */

	float b02_incr;
	float b1_incr;
	float a1_incr;
	float a2_incr;
	int filter_coeff_incr_count;

	/* pan */
	float pan;
	float amp_left;
	float amp_right;

	/* reverb */
	float reverb_send;
	float amp_reverb;

	/* chorus */
	float chorus_send;
	float amp_chorus;

	/* interpolation method, as in fluid_interp in fluidsynth.h */
	int interp_method;

	/* for debugging */
	int debug;
	double ref;

   public:
      Voice(Fluid*);
      Channel* get_channel() const    { return channel; }
      void voice_start();
      void voice_off();
      void init(Sample*, Channel*, int key, int vel, unsigned int id,
         float _gain, double tuning);
      void gen_incr(int i, float val);
      void gen_set(int i, float val);
      float gen_get(int gen);
      unsigned int get_id() const { return id; }
      bool isPlaying()            { return ((status == FLUID_VOICE_ON) || (status == FLUID_VOICE_SUSTAINED)); }
      void set_gain(float gain);
      void set_param(int gen, float nrpn_value, int abs);

      // Update all the synthesis parameters, which depend on generator
      // 'gen'. This is only necessary after changing a generator of an
      // already operating voice.  Most applications will not need this
      // function.

      void update_param(int gen);

      double GEN(int n) { return gen[n].val + gen[n].mod + gen[n].nrpn; }

      void modulate_all();
      void modulate(int _cc, int _ctrl);
      float get_lower_boundary_for_attenuation();
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
      int SAMPLEMODE() const   { return ((int)gen[GEN_SAMPLEMODE].val); }

      void write(float* l, float* r, float* reverb_buf, float* chorus_buf);
      void add_mod(Mod* mod, int mode);
      };

#define FLUID_SAMPLESANITY_CHECK (1 << 0)
#define FLUID_SAMPLESANITY_STARTUP (1 << 1)
extern void fluid_voice_config();

}  // namespace Fluid

#endif  // __FLUID_S_H__
