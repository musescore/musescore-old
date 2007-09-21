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

#ifndef _FLUIDSYNTH_H
#define _FLUIDSYNTH_H

#include <stdio.h>

#define FLUIDSYNTH_API

/*

  Forward declarations

*/
typedef struct _fluid_hashtable_t fluid_settings_t;
typedef struct _fluid_synth_t fluid_synth_t;
typedef struct _fluid_voice_t fluid_voice_t;
typedef struct _fluid_sfloader_t fluid_sfloader_t;
typedef struct _fluid_sfont_t fluid_sfont_t;
typedef struct _fluid_preset_t fluid_preset_t;
typedef struct _fluid_sample_t fluid_sample_t;
typedef struct _fluid_mod_t fluid_mod_t;
typedef struct _fluid_audio_driver_t fluid_audio_driver_t;
typedef struct _fluid_player_t fluid_player_t;
typedef struct _fluid_midi_event_t fluid_midi_event_t;
typedef struct _fluid_midi_driver_t fluid_midi_driver_t;
typedef struct _fluid_midi_router_t fluid_midi_router_t;
typedef struct _fluid_midi_router_rule_t fluid_midi_router_rule_t;
typedef struct _fluid_hashtable_t fluid_cmd_handler_t;
typedef struct _fluid_shell_t fluid_shell_t;
typedef struct _fluid_server_t fluid_server_t;
typedef struct _fluid_event_t fluid_event_t;
typedef struct _fluid_sequencer_t fluid_sequencer_t;
typedef struct _fluid_ramsfont_t fluid_ramsfont_t;
typedef struct _fluid_rampreset_t fluid_rampreset_t;

typedef int fluid_istream_t;
typedef int fluid_ostream_t;
  /**
   *
   *    Synthesizer settings
   *
   *
   *     The create a synthesizer object you will have to specify its
   *     settings. These settings are stored in the structure below.

   *     void my_synthesizer()
   *     {
   *       fluid_settings_t* settings;
   *       fluid_synth_t* synth;
   *       fluid_audio_driver_t* adriver;
   *
   *
   *       settings = new_fluid_settings();
   *       fluid_settings_setstr(settings, "audio.driver", "alsa");
   *       // ... change settings ...
   *       synth = new_fluid_synth(settings);
   *       adriver = new_fluid_audio_driver(settings, synth);
   *
   *       ...
   *
   *     }
   *
   *
   */




/* Hint FLUID_HINT_BOUNDED_BELOW indicates that the LowerBound field
   of the FLUID_PortRangeHint should be considered meaningful. The
   value in this field should be considered the (inclusive) lower
   bound of the valid range. If FLUID_HINT_SAMPLE_RATE is also
   specified then the value of LowerBound should be multiplied by the
   sample rate. */
#define FLUID_HINT_BOUNDED_BELOW   0x1

/* Hint FLUID_HINT_BOUNDED_ABOVE indicates that the UpperBound field
   of the FLUID_PortRangeHint should be considered meaningful. The
   value in this field should be considered the (inclusive) upper
   bound of the valid range. If FLUID_HINT_SAMPLE_RATE is also
   specified then the value of UpperBound should be multiplied by the
   sample rate. */
#define FLUID_HINT_BOUNDED_ABOVE   0x2

/* Hint FLUID_HINT_TOGGLED indicates that the data item should be
   considered a Boolean toggle. Data less than or equal to zero should
   be considered `off' or `false,' and data above zero should be
   considered `on' or `true.' FLUID_HINT_TOGGLED may not be used in
   conjunction with any other hint except FLUID_HINT_DEFAULT_0 or
   FLUID_HINT_DEFAULT_1. */
#define FLUID_HINT_TOGGLED         0x4

/* Hint FLUID_HINT_SAMPLE_RATE indicates that any bounds specified
   should be interpreted as multiples of the sample rate. For
   instance, a frequency range from 0Hz to the Nyquist frequency (half
   the sample rate) could be requested by this hint in conjunction
   with LowerBound = 0 and UpperBound = 0.5. Hosts that support bounds
   at all must support this hint to retain meaning. */
#define FLUID_HINT_SAMPLE_RATE     0x8

/* Hint FLUID_HINT_LOGARITHMIC indicates that it is likely that the
   user will find it more intuitive to view values using a logarithmic
   scale. This is particularly useful for frequencies and gains. */
#define FLUID_HINT_LOGARITHMIC     0x10

/* Hint FLUID_HINT_INTEGER indicates that a user interface would
   probably wish to provide a stepped control taking only integer
   values. Any bounds set should be slightly wider than the actual
   integer range required to avoid floating point rounding errors. For
   instance, the integer set {0,1,2,3} might be described as [-0.1,
   3.1]. */
#define FLUID_HINT_INTEGER         0x20


#define FLUID_HINT_FILENAME        0x01
#define FLUID_HINT_OPTIONLIST      0x02



enum fluid_types_enum {
  FLUID_NO_TYPE = -1,
  FLUID_NUM_TYPE,
  FLUID_INT_TYPE,
  FLUID_STR_TYPE,
  FLUID_SET_TYPE
};


FLUIDSYNTH_API fluid_settings_t* new_fluid_settings(void);
FLUIDSYNTH_API void delete_fluid_settings(fluid_settings_t* settings);



FLUIDSYNTH_API
int fluid_settings_get_type(fluid_settings_t* settings, char* name);

FLUIDSYNTH_API
int fluid_settings_get_hints(fluid_settings_t* settings, char* name);

/** Returns whether the setting is changeable in real-time. */
FLUIDSYNTH_API int fluid_settings_is_realtime(fluid_settings_t* settings, char* name);


/** returns 1 if the value has been set, 0 otherwise */
FLUIDSYNTH_API
int fluid_settings_setstr(fluid_settings_t* settings, char* name, char* str);

/**
    Get the value of a string setting. If the value does not exists,
    'str' is set to NULL. Otherwise, 'str' will point to the
    value. The application does not own the returned value. Instead,
    the application should make a copy of the value if it needs it
    later.

   \returns 1 if the value exists, 0 otherwise
*/
FLUIDSYNTH_API
int fluid_settings_getstr(fluid_settings_t* settings, char* name, char** str);

/** Get the default value of a string setting. */
FLUIDSYNTH_API
char* fluid_settings_getstr_default(fluid_settings_t* settings, char* name);

/** Get the value of a numeric setting.

   \returns 1 if the value exists and is equal to 'value', 0
    otherwise
*/
FLUIDSYNTH_API
int fluid_settings_str_equal(fluid_settings_t* settings, char* name, char* value);


/** returns 1 if the value has been set, 0 otherwise */
FLUIDSYNTH_API
int fluid_settings_setnum(fluid_settings_t* settings, char* name, double val);

/** returns 1 if the value exists, 0 otherwise */
FLUIDSYNTH_API
int fluid_settings_getnum(fluid_settings_t* settings, char* name, double* val);

/** Get the default value of a string setting. */
FLUIDSYNTH_API
double fluid_settings_getnum_default(fluid_settings_t* settings, char* name);

/** Get the range of values of a numeric settings. */
FLUIDSYNTH_API
void fluid_settings_getnum_range(fluid_settings_t* settings, char* name,
				double* min, double* max);


/** returns 1 if the value has been set, 0 otherwise */
FLUIDSYNTH_API
int fluid_settings_setint(fluid_settings_t* settings, char* name, int val);

/** returns 1 if the value exists, 0 otherwise */
FLUIDSYNTH_API
int fluid_settings_getint(fluid_settings_t* settings, char* name, int* val);

/** Get the default value of a string setting. */
FLUIDSYNTH_API
int fluid_settings_getint_default(fluid_settings_t* settings, char* name);

/** Get the range of values of a numeric settings. */
FLUIDSYNTH_API
void fluid_settings_getint_range(fluid_settings_t* settings, char* name,
				int* min, int* max);



typedef void (*fluid_settings_foreach_option_t)(void* data, char* name, char* option);



FLUIDSYNTH_API
void fluid_settings_foreach_option(fluid_settings_t* settings,
				  char* name, void* data,
				  fluid_settings_foreach_option_t func);


typedef void (*fluid_settings_foreach_t)(void* data, char* s, int type);

FLUIDSYNTH_API
void fluid_settings_foreach(fluid_settings_t* settings, void* data,
			   fluid_settings_foreach_t func);
  /**   Embedded synthesizer
   *
   *    You create a new synthesizer with new_fluid_synth() and you destroy
   *    if with delete_fluid_synth(). Use the settings structure to specify
   *    the synthesizer characteristics.
   *
   *    You have to load a SoundFont in order to hear any sound. For that
   *    you use the fluid_synth_sfload() function.
   *
   *    You can use the audio driver functions described below to open
   *    the audio device and create a background audio thread.
   *
   *    The API for sending MIDI events is probably what you expect:
   *    fluid_synth_noteon(), fluid_synth_noteoff(), ...
   *
   */


  /** Creates a new synthesizer object.
   *
   *  Creates a new synthesizer object. As soon as the synthesizer is
   *  created, it will start playing.
   *
   * \param settings a pointer to a settings structure
   * \return a newly allocated synthesizer or NULL in case of error
   */
FLUIDSYNTH_API fluid_synth_t* new_fluid_synth(fluid_settings_t* settings);


  /**
   * Deletes the synthesizer previously created with new_fluid_synth.
   *
   * \param synth the synthesizer object
   * \return 0 if no error occured, -1 otherwise
   */
FLUIDSYNTH_API int delete_fluid_synth(fluid_synth_t* synth);


  /** Get a reference to the settings of the synthesizer.
   *
   * \param synth the synthesizer object
   * \return pointer to the settings
   */
FLUIDSYNTH_API fluid_settings_t* fluid_synth_get_settings(fluid_synth_t* synth);


  /*
   *
   * MIDI channel messages
   *
   */

  /** Send a noteon message. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_noteon(fluid_synth_t* synth, int chan, int key, int vel);

  /** Send a noteoff message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_noteoff(fluid_synth_t* synth, int chan, int key);

  /** Send a control change message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_cc(fluid_synth_t* synth, int chan, int ctrl, int val);

  /** Get a control value. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_get_cc(fluid_synth_t* synth, int chan, int ctrl, int* pval);

  /** Send a pitch bend message. Returns 0 if no error occurred, -1 otherwise.  */
FLUIDSYNTH_API int fluid_synth_pitch_bend(fluid_synth_t* synth, int chan, int val);

  /** Get the pitch bend value. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API
int fluid_synth_get_pitch_bend(fluid_synth_t* synth, int chan, int* ppitch_bend);

  /** Set the pitch wheel sensitivity. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_pitch_wheel_sens(fluid_synth_t* synth, int chan, int val);

  /** Get the pitch wheel sensitivity. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_get_pitch_wheel_sens(fluid_synth_t* synth, int chan, int* pval);

  /** Send a program change message. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_program_change(fluid_synth_t* synth, int chan, int program);

  /** Select a bank. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API
int fluid_synth_bank_select(fluid_synth_t* synth, int chan, unsigned int bank);

  /** Select a sfont. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API
int fluid_synth_sfont_select(fluid_synth_t* synth, int chan, unsigned int sfont_id);

  /** Select a preset for a channel. The preset is specified by the
      SoundFont ID, the bank number, and the preset number. This
      allows any preset to be selected and circumvents preset masking
      due to previously loaded SoundFonts on the SoundFont stack.

      \param synth The synthesizer
      \param chan The channel on which to set the preset
      \param sfont_id The ID of the SoundFont
      \param bank_num The bank number
      \param preset_num The preset number
      \return 0 if no errors occured, -1 otherwise
  */
FLUIDSYNTH_API
int fluid_synth_program_select(fluid_synth_t* synth, int chan,
			      unsigned int sfont_id,
			      unsigned int bank_num,
			      unsigned int preset_num);

  /** Returns the program, bank, and SoundFont number of the preset on
      a given channel. Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API
int fluid_synth_get_program(fluid_synth_t* synth, int chan,
			   unsigned int* sfont_id,
			   unsigned int* bank_num,
			   unsigned int* preset_num);

  /** Send a bank select and a program change to every channel to
   *  reinitialize the preset of the channel. This function is useful
   *  mainly after a SoundFont has been loaded, unloaded or
   *  reloaded. . Returns 0 if no error occurred, -1 otherwise. */
FLUIDSYNTH_API int fluid_synth_program_reset(fluid_synth_t* synth);

  /** Send a reset. A reset turns all the notes off and resets the
      controller values. */
FLUIDSYNTH_API int fluid_synth_system_reset(fluid_synth_t* synth);


  /*
   *
   * Low level access
   *
   */

  /** Create and start voices using a preset. The id passed as
   * argument will be used as the voice group id.  */
FLUIDSYNTH_API int fluid_synth_start(fluid_synth_t* synth, unsigned int id,
				     fluid_preset_t* preset, int audio_chan,
				     int midi_chan, int key, int vel);

  /** Stop the voices in the voice group defined by id. */
FLUIDSYNTH_API int fluid_synth_stop(fluid_synth_t* synth, unsigned int id);

  /** Change the value of a generator of the voices in the voice group
   * defined by id. */
/* FLUIDSYNTH_API int fluid_synth_ctrl(fluid_synth_t* synth, int id,  */
/* 				    int gen, float value,  */
/* 				    int absolute, int normalized); */


  /*
   *
   * SoundFont management
   *
   */

  /** Loads a SoundFont file and creates a new SoundFont. The newly
      loaded SoundFont will be put on top of the SoundFont
      stack. Presets are searched starting from the SoundFont on the
      top of the stack, working the way down the stack until a preset
      is found.

      \param synth The synthesizer object
      \param filename The file name
      \param reset_presets If non-zero, the presets on the channels will be reset
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API
int fluid_synth_sfload(fluid_synth_t* synth, const QString& filename, int reset_presets);

  /** Reload a SoundFont. The reloaded SoundFont retains its ID and
      index on the stack.

      \param synth The synthesizer object
      \param id The id of the SoundFont
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API int fluid_synth_sfreload(fluid_synth_t* synth, unsigned int id);

  /** Removes a SoundFont from the stack and deallocates it.

      \param synth The synthesizer object
      \param id The id of the SoundFont
      \param reset_presets If TRUE then presets will be reset for all channels
      \returns 0 if no error, -1 otherwise
  */
FLUIDSYNTH_API int fluid_synth_sfunload(fluid_synth_t* synth, unsigned int id, int reset_presets);

  /** Add a SoundFont. The SoundFont will be put on top of
      the SoundFont stack.

      \param synth The synthesizer object
      \param sfont The SoundFont
      \returns The ID of the loaded SoundFont, or -1 in case of error
  */
FLUIDSYNTH_API int fluid_synth_add_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

  /** Remove a SoundFont that was previously added using
   *  fluid_synth_add_sfont(). The synthesizer does not delete the
   *  SoundFont; this is responsability of the caller.

      \param synth The synthesizer object
      \param sfont The SoundFont
  */
FLUIDSYNTH_API void fluid_synth_remove_sfont(fluid_synth_t* synth, fluid_sfont_t* sfont);

  /** Count the number of loaded SoundFonts.

      \param synth The synthesizer object
      \returns The number of loaded SoundFonts
  */
FLUIDSYNTH_API int fluid_synth_sfcount(fluid_synth_t* synth);

  /** Get a SoundFont. The SoundFont is specified by its index on the
      stack. The top of the stack has index zero.

      \param synth The synthesizer object
      \param num The number of the SoundFont (0 <= num < sfcount)
      \returns A pointer to the SoundFont
  */
FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont(fluid_synth_t* synth, unsigned int num);

  /** Get a SoundFont. The SoundFont is specified by its ID.

      \param synth The synthesizer object
      \param id The id of the sfont
      \returns A pointer to the SoundFont
  */
FLUIDSYNTH_API fluid_sfont_t* fluid_synth_get_sfont_by_id(fluid_synth_t* synth, unsigned int id);


  /** Get the preset of a channel */
FLUIDSYNTH_API fluid_preset_t* fluid_synth_get_channel_preset(fluid_synth_t* synth, int chan);

  /** Offset the bank numbers in a SoundFont. Returns -1 if an error
   * occured (out of memory or negative offset) */
FLUIDSYNTH_API int fluid_synth_set_bank_offset(fluid_synth_t* synth, int sfont_id, int offset);

  /** Get the offset of the bank numbers in a SoundFont. */
FLUIDSYNTH_API int fluid_synth_get_bank_offset(fluid_synth_t* synth, int sfont_id);



  /*
   *
   * Reverb
   *
   */

  /** Set the parameters for the built-in reverb unit */
FLUIDSYNTH_API void fluid_synth_set_reverb(fluid_synth_t* synth, double roomsize,
					 double damping, double width, double level);

  /** Turn on (1) / off (0) the built-in reverb unit */
FLUIDSYNTH_API void fluid_synth_set_reverb_on(fluid_synth_t* synth, int on);


  /** Query the current state of the reverb. */
FLUIDSYNTH_API double fluid_synth_get_reverb_roomsize(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_damp(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_reverb_width(fluid_synth_t* synth);

  /* Those are the default settings for the reverb */
#define FLUID_REVERB_DEFAULT_ROOMSIZE 0.2f
#define FLUID_REVERB_DEFAULT_DAMP 0.0f
#define FLUID_REVERB_DEFAULT_WIDTH 0.5f
#define FLUID_REVERB_DEFAULT_LEVEL 0.9f



  /*
   *
   * Chorus
   *
   */

enum fluid_chorus_mod {
  FLUID_CHORUS_MOD_SINE = 0,
  FLUID_CHORUS_MOD_TRIANGLE = 1
};

  /** Set up the chorus. It should be turned on with fluid_synth_set_chorus_on.
   * If faulty parameters are given, all new settings are discarded.
   * Keep in mind, that the needed CPU time is proportional to 'nr'.
   */
FLUIDSYNTH_API void fluid_synth_set_chorus(fluid_synth_t* synth, int nr, double level,
					 double speed, double depth_ms, int type);

  /** Turn on (1) / off (0) the built-in chorus unit */
FLUIDSYNTH_API void fluid_synth_set_chorus_on(fluid_synth_t* synth, int on);

  /** Query the current state of the chorus. */
FLUIDSYNTH_API int fluid_synth_get_chorus_nr(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_level(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_speed_Hz(fluid_synth_t* synth);
FLUIDSYNTH_API double fluid_synth_get_chorus_depth_ms(fluid_synth_t* synth);
FLUIDSYNTH_API int fluid_synth_get_chorus_type(fluid_synth_t* synth); /* see fluid_chorus_mod */

  /* Those are the default settings for the chorus. */
#define FLUID_CHORUS_DEFAULT_N 3
#define FLUID_CHORUS_DEFAULT_LEVEL 2.0f
#define FLUID_CHORUS_DEFAULT_SPEED 0.3f
#define FLUID_CHORUS_DEFAULT_DEPTH 8.0f
#define FLUID_CHORUS_DEFAULT_TYPE FLUID_CHORUS_MOD_SINE



  /*
   *
   * Audio and MIDI channels
   *
   */

  /** Returns the number of MIDI channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_midi_channels(fluid_synth_t* synth);

  /** Returns the number of audio channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_audio_channels(fluid_synth_t* synth);

  /** Returns the number of audio groups that the synthesizer uses
      internally. This is usually identical to audio_channels. */
FLUIDSYNTH_API int fluid_synth_count_audio_groups(fluid_synth_t* synth);

  /** Returns the number of effects channels that the synthesizer uses
      internally */
FLUIDSYNTH_API int fluid_synth_count_effects_channels(fluid_synth_t* synth);



  /*
   *
   * Synthesis parameters
   *
   */

  /** Set the master gain */
FLUIDSYNTH_API void fluid_synth_set_gain(fluid_synth_t* synth, float gain);

  /** Get the master gain */
FLUIDSYNTH_API float fluid_synth_get_gain(fluid_synth_t* synth);

  /** Set the polyphony limit (FluidSynth >= 1.0.6) */
FLUIDSYNTH_API int fluid_synth_set_polyphony(fluid_synth_t* synth, int polyphony);

  /** Get the polyphony limit (FluidSynth >= 1.0.6) */
FLUIDSYNTH_API int fluid_synth_get_polyphony(fluid_synth_t* synth);

  /** Get the internal buffer size. The internal buffer size if not the
      same thing as the buffer size specified in the
      settings. Internally, the synth *always* uses a specific buffer
      size independent of the buffer size used by the audio driver. The
      internal buffer size is normally 64 samples. The reason why it
      uses an internal buffer size is to allow audio drivers to call the
      synthesizer with a variable buffer length. The internal buffer
      size is useful for client who want to optimize their buffer sizes.
  */
FLUIDSYNTH_API int fluid_synth_get_internal_bufsize(fluid_synth_t* synth);

  /** Set the interpolation method for one channel or all channels (chan = -1) */
FLUIDSYNTH_API
int fluid_synth_set_interp_method(fluid_synth_t* synth, int chan, int interp_method);

  /* Flags to choose the interpolation method */
enum fluid_interp {
  /* no interpolation: Fastest, but questionable audio quality */
  FLUID_INTERP_NONE = 0,
  /* Straight-line interpolation: A bit slower, reasonable audio quality */
  FLUID_INTERP_LINEAR = 1,
  /* Fourth-order interpolation: Requires 50 % of the whole DSP processing time, good quality
   * Default. */
  FLUID_INTERP_DEFAULT = 4,
  FLUID_INTERP_4THORDER = 4,
  FLUID_INTERP_7THORDER = 7,
  FLUID_INTERP_HIGHEST=7
};




  /*
   *
   * Generator interface
   *
   */

  /** Change the value of a generator. This function allows to control
      all synthesis parameters in real-time. The changes are additive,
      i.e. they add up to the existing parameter value. This function is
      similar to sending an NRPN message to the synthesizer. The
      function accepts a float as the value of the parameter. The
      parameter numbers and ranges are described in the SoundFont 2.01
      specification, paragraph 8.1.3, page 48. See also 'fluid_gen_type'.

      \param synth The synthesizer object.
      \param chan The MIDI channel number.
      \param param The parameter number.
      \param value The parameter value.
      \returns Your favorite dish.
  */
FLUIDSYNTH_API
int fluid_synth_set_gen(fluid_synth_t* synth, int chan, int param, float value);


  /** Retreive the value of a generator. This function returns the value
      set by a previous call 'fluid_synth_set_gen' or by an NRPN message.

      \param synth The synthesizer object.
      \param chan The MIDI channel number.
      \param param The generator number.
      \returns The value of the generator.
  */
FLUIDSYNTH_API float fluid_synth_get_gen(fluid_synth_t* synth, int chan, int param);




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
FLUIDSYNTH_API
int fluid_synth_create_key_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
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
FLUIDSYNTH_API
int fluid_synth_create_octave_tuning(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
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
FLUIDSYNTH_API
int fluid_synth_tune_notes(fluid_synth_t* synth, int tuning_bank, int tuning_prog,
			  int len, int *keys, double* pitch, int apply);

  /** Select a tuning for a channel.

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  \param tuning_bank The tuning bank number [0-127]
  \param tuning_prog The tuning program number [0-127]
  */
FLUIDSYNTH_API
int fluid_synth_select_tuning(fluid_synth_t* synth, int chan, int tuning_bank, int tuning_prog);

  /** Set the tuning to the default well-tempered tuning on a channel.

  \param synth The synthesizer object
  \param chan The channel number [0-max channels]
  */
FLUIDSYNTH_API int fluid_synth_reset_tuning(fluid_synth_t* synth, int chan);

  /** Start the iteration throught the list of available tunings.

  \param synth The synthesizer object
  */
FLUIDSYNTH_API void fluid_synth_tuning_iteration_start(fluid_synth_t* synth);


  /** Get the next tuning in the iteration. This functions stores the
      bank and program number of the next tuning in the pointers given as
      arguments.

      \param synth The synthesizer object
      \param bank Pointer to an int to store the bank number
      \param prog Pointer to an int to store the program number
      \returns 1 if there is a next tuning, 0 otherwise
  */
FLUIDSYNTH_API
int fluid_synth_tuning_iteration_next(fluid_synth_t* synth, int* bank, int* prog);


  /** Dump the data of a tuning. This functions stores the name and
      pitch values of a tuning in the pointers given as arguments. Both
      name and pitch can be NULL is the data is not needed.

      \param synth The synthesizer object
      \param bank The tuning bank number [0-127]
      \param prog The tuning program number [0-127]
      \param name Pointer to a buffer to store the name
      \param len The length of the name buffer
      \param pitch Pointer to buffer to store the pitch values
  */
FLUIDSYNTH_API int fluid_synth_tuning_dump(fluid_synth_t* synth, int bank, int prog,
					 char* name, int len, double* pitch);




  /*
   *
   * Misc
   *
   */

  /** Get an estimation of the CPU load due to the audio synthesis.
      Returns a percentage (0-100).

      \param synth The synthesizer object
  */
FLUIDSYNTH_API double fluid_synth_get_cpu_load(fluid_synth_t* synth);

  /** Get a textual representation of the last error */
FLUIDSYNTH_API char* fluid_synth_error(fluid_synth_t* synth);


  /*
   *
   *    Synthesizer plugin
   *
   *
   *    To create a synthesizer plugin, create the synthesizer as
   *    explained above. Once the synthesizer is created you can call
   *    any of the functions below to get the audio.
   *
   */

  /** Generate a number of samples. This function expects two signed
   *  16bits buffers (left and right channel) that will be filled with
   *  samples.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param lout The sample buffer for the left channel
   *  \param loff The offset, in samples, in the left buffer where the writing pointer starts
   *  \param lincr The increment, in samples, of the writing pointer in the left buffer
   *  \param rout The sample buffer for the right channel
   *  \param roff The offset, in samples, in the right buffer where the writing pointer starts
   *  \param rincr The increment, in samples, of the writing pointer in the right buffer
   *  \returns 0 if no error occured, non-zero otherwise
   */

FLUIDSYNTH_API int fluid_synth_write_s16(fluid_synth_t* synth, int len,
				       void* lout, int loff, int lincr,
				       void* rout, int roff, int rincr);


  /** Generate a number of samples. This function expects two floating
   *  point buffers (left and right channel) that will be filled with
   *  samples.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param lout The sample buffer for the left channel
   *  \param loff The offset, in samples, in the left buffer where the writing pointer starts
   *  \param lincr The increment, in samples, of the writing pointer in the left buffer
   *  \param rout The sample buffer for the right channel
   *  \param roff The offset, in samples, in the right buffer where the writing pointer starts
   *  \param rincr The increment, in samples, of the writing pointer in the right buffer
   *  \returns 0 if no error occured, non-zero otherwise
   */

void fluid_synth_write_float(fluid_synth_t* synth, int len,
   float* lout, float* rout, int stride);

#if 0
  /** Generate a number of samples. This function implements the
   *  default interface defined in fluidsynth/audio.h. This function
   *  ignores the input buffers and expects at least two output
   *  buffer.
   *
   *  \param synth The synthesizer
   *  \param len The number of samples to generate
   *  \param nin The number of input buffers
   *  \param in The array of input buffers
   *  \param nout The number of output buffers
   *  \param out The array of output buffers
   *  \returns 0 if no error occured, non-zero otherwise
   */

FLUIDSYNTH_API int fluid_synth_process(fluid_synth_t* synth, int len,
				     int nin, float** in,
				     int nout, float** out);



  /* Type definition of the synthesizer's audio callback function. */
typedef int (*fluid_audio_callback_t)(fluid_synth_t* synth, int len,
				     void* out1, int loff, int lincr,
				     void* out2, int roff, int rincr);



#endif

  /*
   *  Synthesizer's interface to handle SoundFont loaders
   */


  /** Add a SoundFont loader to the synthesizer. Note that SoundFont
      loader don't necessarily load SoundFonts. They can load any type
      of wavetable data but export a SoundFont interface. */
FLUIDSYNTH_API void fluid_synth_add_sfloader(fluid_synth_t* synth, fluid_sfloader_t* loader);

  /** Allocate a synthesis voice. This function is called by a
      soundfont's preset in response to a noteon event.
      The returned voice comes with default modulators installed (velocity-to-attenuation,
      velocity to filter, ...)
      Note: A single noteon event may create any number of voices, when the preset is layered.
      Typically 1 (mono) or 2 (stereo).*/
FLUIDSYNTH_API fluid_voice_t* fluid_synth_alloc_voice(fluid_synth_t* synth, fluid_sample_t* sample,
						   int channum, int key, int vel);

  /** Start a synthesis voice. This function is called by a
      soundfont's preset in response to a noteon event after the voice
      has been allocated with fluid_synth_alloc_voice() and
      initialized.
      Exclusive classes are processed here.*/
FLUIDSYNTH_API void fluid_synth_start_voice(fluid_synth_t* synth, fluid_voice_t* voice);


  /** Write a list of all voices matching ID into buf, but not more than bufsize voices.
   * If ID <0, return all voices. */
FLUIDSYNTH_API void fluid_synth_get_voicelist(fluid_synth_t* synth,
					    fluid_voice_t* buf[], int bufsize, int ID);


  /** Callback function for the MIDI router. Any event goes through this. */
FLUIDSYNTH_API int fluid_synth_handle_midi_event(void* data, fluid_midi_event_t* event);


  /** This is a hack to get command handlers working */
FLUIDSYNTH_API void fluid_synth_set_midi_router(fluid_synth_t* synth,
					      fluid_midi_router_t* router);

  /**
   *
   *   SoundFont plugins
   *
   *    It is possible to add new SoundFont loaders to the
   *    synthesizer. The API uses a couple of "interfaces" (structures
   *    with callback functions): fluid_sfloader_t, fluid_sfont_t, and
   *    fluid_preset_t.
   *
   *    To add a new SoundFont loader to the synthesizer, call
   *    fluid_synth_add_sfloader() and pass a pointer to an
   *    fluid_sfloader_t structure. The important callback function in
   *    this structure is "load", which should try to load a file and
   *    returns a fluid_sfont_t structure, or NULL if it fails.
   *
   *    The fluid_sfont_t structure contains a callback to obtain the
   *    name of the soundfont. It contains two functions to iterate
   *    though the contained presets, and one function to obtain a
   *    preset corresponding to a bank and preset number. This
   *    function should return an fluid_preset_t structure.
   *
   *    The fluid_preset_t structure contains some functions to obtain
   *    information from the preset (name, bank, number). The most
   *    important callback is the noteon function. The noteon function
   *    should call fluid_synth_alloc_voice() for every sample that has
   *    to be played. fluid_synth_alloc_voice() expects a pointer to a
   *    fluid_sample_t structure and returns a pointer to the opaque
   *    fluid_voice_t structure. To set or increments the values of a
   *    generator, use fluid_voice_gen_{set,incr}. When you are
   *    finished initializing the voice call fluid_voice_start() to
   *    start playing the synthesis voice.
   * */

  enum {
    FLUID_PRESET_SELECTED,
    FLUID_PRESET_UNSELECTED,
    FLUID_SAMPLE_DONE
  };


/*
 * fluid_sfloader_t
 */

struct _fluid_sfloader_t {
  /** Private data */
  void* data;

  /** The free must free the memory allocated for the loader in
   * addition to any private data. It should return 0 if no error
   * occured, non-zero otherwise.*/
  int (*free)(fluid_sfloader_t* loader);

  /** Load a file. Returns NULL if an error occured. */
  fluid_sfont_t* (*load)(fluid_sfloader_t* loader, const QString& filename);
};


/*
 * fluid_sfont_t
 */

struct _fluid_sfont_t {
  void* data;
  unsigned int id;

  /** The 'free' callback function should return 0 when it was able to
      free all resources. It should return a non-zero value if some of
      the samples could not be freed because they are still in use. */
  int (*free)(fluid_sfont_t* sfont);

  /** Return the name of the sfont */
  char* (*get_name)(fluid_sfont_t* sfont);

  /** Return the preset with the specified bank and preset number. All
   *  the fields, including the 'sfont' field, should * be filled
   *  in. If the preset cannot be found, the function returns NULL. */
  fluid_preset_t* (*get_preset)(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum);

  void (*iteration_start)(fluid_sfont_t* sfont);

  /* return 0 when no more presets are available, 1 otherwise */
  int (*iteration_next)(fluid_sfont_t* sfont, fluid_preset_t* preset);
};

#define fluid_sfont_get_id(_sf) ((_sf)->id)


/*
 * fluid_preset_t
 */

struct _fluid_preset_t {
  void* data;
  fluid_sfont_t* sfont;
  int (*free)(fluid_preset_t* preset);
  char* (*get_name)(fluid_preset_t* preset);
  int (*get_banknum)(fluid_preset_t* preset);
  int (*get_num)(fluid_preset_t* preset);

  /** handle a noteon event. Returns 0 if no error occured. */
  int (*noteon)(fluid_preset_t* preset, fluid_synth_t* synth, int chan, int key, int vel);

  /** Implement this function if the preset needs to be notified about
      preset select and unselect events. */
  int (*notify)(fluid_preset_t* preset, int reason, int chan);
};


/*
 * fluid_sample_t
 */

struct _fluid_sample_t
{
  char name[21];
  unsigned int start;
  unsigned int end;
  unsigned int loopstart;
  unsigned int loopend;
  unsigned int samplerate;
  int origpitch;
  int pitchadj;
  int sampletype;
  int valid;
  short* data;

  /** The amplitude, that will lower the level of the sample's loop to
      the noise floor. Needed for note turnoff optimization, will be
      filled out automatically */
  /* Set this to zero, when submitting a new sample. */
  int amplitude_that_reaches_noise_floor_is_valid;
  double amplitude_that_reaches_noise_floor;

  /** Count the number of playing voices that use this sample. */
  unsigned int refcount;

  /** Implement this function if the sample or SoundFont needs to be
      notified when the sample is no longer used. */
  int (*notify)(fluid_sample_t* sample, int reason);

  /** Pointer to SoundFont specific data */
  void* userdata;
};


#define fluid_sample_refcount(_sample) ((_sample)->refcount)


/** Sample types */

#define FLUID_SAMPLETYPE_MONO	1
#define FLUID_SAMPLETYPE_RIGHT	2
#define FLUID_SAMPLETYPE_LEFT	4
#define FLUID_SAMPLETYPE_LINKED	8
#define FLUID_SAMPLETYPE_ROM	0x8000
/********************************************************************************/
/********************************************************************************/
/* ram soundfonts:
	October 2002 - Antoine Schmitt

	ram soundfonts live in ram. The samples are loaded from files
	or from RAM.  A minimal API manages a soundFont structure,
	with presets, each preset having only one preset-zone, which
	instrument has potentially many instrument-zones.  No global
	zones, and nor generator nor modulator other than the default
	ones are permitted.  This may be extensible in the future.
*/
/********************************************************************************/
/********************************************************************************/

/*
   We are not using the sfloader protocol, as we need more arguments
   than what it provides.
*/

/** Creates a fluid_sfont_t wrapping an fluid_ramsfont_t */
FLUIDSYNTH_API fluid_sfont_t* fluid_ramsfont_create_sfont(void);

/***********************
 * ramsfont specific API
 ***********************/
FLUIDSYNTH_API int fluid_ramsfont_set_name(fluid_ramsfont_t* sfont, char * name);

/* Creates one instrument zone for the sample inside the preset defined
 *     by bank/num
 *     \returns 0 if success
 */
FLUIDSYNTH_API
int fluid_ramsfont_add_izone(fluid_ramsfont_t* sfont,
				unsigned int bank, unsigned int num, fluid_sample_t* sample,
				int lokey, int hikey);

/* Removes the instrument zone corresponding to bank/num and to the sample
 *     \returns 0 if success
 */
FLUIDSYNTH_API
int fluid_ramsfont_remove_izone(fluid_ramsfont_t* sfont,
				unsigned int bank, unsigned int num, fluid_sample_t* sample);

/* Sets a generator on an instrument zone
 *     \returns 0 if success
 */
FLUIDSYNTH_API
int fluid_ramsfont_izone_set_gen(fluid_ramsfont_t* sfont,
				unsigned int bank, unsigned int num, fluid_sample_t* sample,
				int gen_type, float value);

/* Utility : sets the loop start/end values
 *     \on = 0 or 1; if 0, loopstart and loopend are not used
 *     \loopstart and loopend are floats, in frames
 *     \loopstart is counted from frame 0
 *     \loopend is counted from the last frame, thus is < 0
 *     \returns 0 if success
 */
FLUIDSYNTH_API
int fluid_ramsfont_izone_set_loop(fluid_ramsfont_t* sfont,
				unsigned int bank, unsigned int num, fluid_sample_t* sample,
				int on, float loopstart, float loopend);

/***************************************
 * sample_t specific API for ramsfont
 ***************************************/
FLUIDSYNTH_API fluid_sample_t* new_fluid_ramsample(void);
FLUIDSYNTH_API int delete_fluid_ramsample(fluid_sample_t* sample);
FLUIDSYNTH_API int fluid_sample_set_name(fluid_sample_t* sample, char * name);

/* Sets the sound data of the sample
 *     Warning : if copy_data is FALSE, data should have 8 unused frames at start
 *     and 8 unused frames at the end.
 */
FLUIDSYNTH_API
int fluid_sample_set_sound_data(fluid_sample_t* sample, short *data,
			       unsigned int nbframes, short copy_data, int rootkey);
/**
 * @file midi.h
 * @brief Functions for MIDI events, drivers and MIDI file playback.
 */

FLUIDSYNTH_API fluid_midi_event_t* new_fluid_midi_event(void);
FLUIDSYNTH_API int delete_fluid_midi_event(fluid_midi_event_t* event);

FLUIDSYNTH_API int fluid_midi_event_set_type(fluid_midi_event_t* evt, int type);
FLUIDSYNTH_API int fluid_midi_event_get_type(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_channel(fluid_midi_event_t* evt, int chan);
FLUIDSYNTH_API int fluid_midi_event_get_channel(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_get_key(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_key(fluid_midi_event_t* evt, int key);
FLUIDSYNTH_API int fluid_midi_event_get_velocity(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_velocity(fluid_midi_event_t* evt, int vel);
FLUIDSYNTH_API int fluid_midi_event_get_control(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_control(fluid_midi_event_t* evt, int ctrl);
FLUIDSYNTH_API int fluid_midi_event_get_value(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_value(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_program(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_program(fluid_midi_event_t* evt, int val);
FLUIDSYNTH_API int fluid_midi_event_get_pitch(fluid_midi_event_t* evt);
FLUIDSYNTH_API int fluid_midi_event_set_pitch(fluid_midi_event_t* evt, int val);


/**
 * Generic callback function for MIDI events.
 * @param data User defined data pointer
 * @param event The MIDI event
 * @return DOCME
 *
 * Will be used between
 * - MIDI driver and MIDI router
 * - MIDI router and synth
 * to communicate events.
 * In the not-so-far future...
 */
typedef int (*handle_midi_event_func_t)(void* data, fluid_midi_event_t* event);

/*
 *  MIDI router
 *
 *  The MIDI handler forwards incoming MIDI events to the synthesizer
 */

FLUIDSYNTH_API fluid_midi_router_t* new_fluid_midi_router(fluid_settings_t* settings,
						       handle_midi_event_func_t handler,
						       void* event_handler_data);

FLUIDSYNTH_API int delete_fluid_midi_router(fluid_midi_router_t* handler);
FLUIDSYNTH_API int fluid_midi_router_handle_midi_event(void* data, fluid_midi_event_t* event);
FLUIDSYNTH_API int fluid_midi_dump_prerouter(void* data, fluid_midi_event_t* event);
FLUIDSYNTH_API int fluid_midi_dump_postrouter(void* data, fluid_midi_event_t* event);

/*
 *  MIDI driver
 *
 *  The MIDI handler forwards incoming MIDI events to the synthesizer
 */

FLUIDSYNTH_API
fluid_midi_driver_t* new_fluid_midi_driver(fluid_settings_t* settings,
					 handle_midi_event_func_t handler,
					 void* event_handler_data);

FLUIDSYNTH_API void delete_fluid_midi_driver(fluid_midi_driver_t* driver);



/*
 *  MIDI file player
 *
 *  The MIDI player allows you to play MIDI files with the FLUID Synth
 */

FLUIDSYNTH_API fluid_player_t* new_fluid_player(fluid_synth_t* synth);
FLUIDSYNTH_API int delete_fluid_player(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_add(fluid_player_t* player, char* midifile);
FLUIDSYNTH_API int fluid_player_play(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_stop(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_join(fluid_player_t* player);
FLUIDSYNTH_API int fluid_player_set_loop(fluid_player_t* player, int loop);
FLUIDSYNTH_API int fluid_player_set_midi_tempo(fluid_player_t* player, int tempo);
FLUIDSYNTH_API int fluid_player_set_bpm(fluid_player_t* player, int bpm);
typedef void (*fluid_event_callback_t)(unsigned int time, fluid_event_t* event,
				      fluid_sequencer_t* seq, void* data);


/** Allocate a new sequencer structure */
FLUIDSYNTH_API fluid_sequencer_t* new_fluid_sequencer(void);

/** Free the sequencer structure */
FLUIDSYNTH_API void delete_fluid_sequencer(fluid_sequencer_t* seq);

/** clients can be sources or destinations of events. These functions ensure a unique ID for any
source or dest, for filtering purposes.
sources only dont need to register a callback.
*/

/** Register a client. The registration returns a unique client ID (-1 if error) */
FLUIDSYNTH_API
short fluid_sequencer_register_client(fluid_sequencer_t* seq, char* name,
				     fluid_event_callback_t callback, void* data);

/** Unregister a previously registered client. */
FLUIDSYNTH_API void fluid_sequencer_unregister_client(fluid_sequencer_t* seq, short id);

/** Returns the number of register clients. */
FLUIDSYNTH_API int fluid_sequencer_count_clients(fluid_sequencer_t* seq);

/** Returns the id of a registered client (-1 if non existing) */
FLUIDSYNTH_API short fluid_sequencer_get_client_id(fluid_sequencer_t* seq, int index);

/** Returns the name of a registered client, given its id. */
FLUIDSYNTH_API char* fluid_sequencer_get_client_name(fluid_sequencer_t* seq, int id);

/** Returns 1 if client is a destination (has a callback) */
FLUIDSYNTH_API int fluid_sequencer_client_is_dest(fluid_sequencer_t* seq, int id);



/** Sending an event immediately. */
FLUIDSYNTH_API void fluid_sequencer_send_now(fluid_sequencer_t* seq, fluid_event_t* evt);


/** Schedule an event for later sending. If absolute is 0, the time of
    the event will be offset with the current tick of the
    sequencer. If absolute is different from 0, the time will assumed
    to be absolute (starting from the creation of the sequencer).
    MAKES A COPY */
FLUIDSYNTH_API
int fluid_sequencer_send_at(fluid_sequencer_t* seq, fluid_event_t* evt,
			   unsigned int time, int absolute);

/** Remove events from the event queue. The events can be filtered on
    the source, the destination, and the type of the event. To avoid
    filtering, set either source, dest, or type to -1.  */
FLUIDSYNTH_API
void fluid_sequencer_remove_events(fluid_sequencer_t* seq, short source, short dest, int type);


/** Get the current tick */
FLUIDSYNTH_API unsigned int fluid_sequencer_get_tick(fluid_sequencer_t* seq);

/** Set the conversion from tick to absolute time. scale should be
    expressed as ticks per second. */
FLUIDSYNTH_API void fluid_sequencer_set_time_scale(fluid_sequencer_t* seq, double scale);

/** Set the conversion from tick to absolute time (ticks per
    second). */
FLUIDSYNTH_API double fluid_sequencer_get_time_scale(fluid_sequencer_t* seq);

// compile in internal traceing functions
#define FLUID_SEQ_WITH_TRACE 0

#if FLUID_SEQ_WITH_TRACE
FLUIDSYNTH_API char * fluid_seq_gettrace(fluid_sequencer_t* seq);
FLUIDSYNTH_API void fluid_seq_cleartrace(fluid_sequencer_t* seq);
#endif
  /** registers fluidsynth as a client of the given sequencer.
      The fluidsynth is registered with the name "fluidsynth".

      \returns the fluidsynth destID.
  */
FLUIDSYNTH_API
short fluid_sequencer_register_fluidsynth(fluid_sequencer_t* seq, fluid_synth_t* synth);
/**
 * @file log.h
 * @brief Logging interface
 *
 * The default logging function of the fluidsynth prints its messages
 * to the stderr. The synthesizer uses five level of messages: #FLUID_PANIC,
 * #FLUID_ERR, #FLUID_WARN, #FLUID_INFO, and #FLUID_DBG.
 *
 * A client application can install a new log function to handle the
 * messages differently. In the following example, the application
 * sets a callback function to display #FLUID_PANIC messages in a dialog,
 * and ignores all other messages by setting the log function to
 * NULL:
 *
 * DOCME (formatting)
 * fluid_set_log_function(FLUID_PANIC, show_dialog, (void*) root_window);
 * fluid_set_log_function(FLUID_ERR, NULL, NULL);
 * fluid_set_log_function(FLUID_WARN, NULL, NULL);
 * fluid_set_log_function(FLUID_DBG, NULL, NULL);
 */

/**
 * FluidSynth log levels.
 */
enum fluid_log_level {
  FLUID_PANIC,   /**< The synth can't function correctly any more */
  FLUID_ERR,     /**< Serious error occurred */
  FLUID_WARN,    /**< Warning */
  FLUID_INFO,    /**< Verbose informational messages */
  FLUID_DBG,     /**< Debugging messages */
  LAST_LOG_LEVEL
};

/**
 * Log function handler callback type used by fluid_set_log_function().
 * @param level Log level (#fluid_log_level)
 * @param message Log message text
 * @param data User data pointer supplied to fluid_set_log_function().
 */
typedef void (*fluid_log_function_t)(int level, char* message, void* data);

FLUIDSYNTH_API
fluid_log_function_t fluid_set_log_function(int level, fluid_log_function_t fun, void* data);

FLUIDSYNTH_API void fluid_default_log_function(int level, char* message, void* data);

FLUIDSYNTH_API int fluid_log(int level, char * fmt, ...);
/*
 *
 *  Utility functions
 */

/**
 * fluid_is_soundfont returns 1 if the specified filename is a
 * soundfont. It retuns 0 otherwise. The current implementation only
 * checks for the "RIFF" header in the file. It is useful only to
 * distinguish between SoundFonts and MIDI files.
 */
FLUIDSYNTH_API int fluid_is_soundfont(char* filename);

/**
 * fluid_is_midifile returns 1 if the specified filename is a MIDI
 * file. It retuns 0 otherwise. The current implementation only checks
 * for the "MThd" header in the file.
 */
FLUIDSYNTH_API int fluid_is_midifile(char* filename);




#ifdef WIN32
/** Set the handle to the instance of the application on the Windows
    platform. The handle is needed to open DirectSound. */
FLUIDSYNTH_API void* fluid_get_hinstance(void);
FLUIDSYNTH_API void fluid_set_hinstance(void* hinstance);
#endif
  /* Modulator-related definitions */

  /* Maximum number of modulators in a voice */
#define FLUID_NUM_MOD           64

  /*
   *  fluid_mod_t
   */
struct _fluid_mod_t
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
  fluid_mod_t * next;
};

/* Flags telling the polarity of a modulator.  Compare with SF2.01
   section 8.2. Note: The numbers of the bits are different!  (for
   example: in the flags of a SF modulator, the polarity bit is bit
   nr. 9) */
enum fluid_mod_flags
{
  FLUID_MOD_POSITIVE = 0,
  FLUID_MOD_NEGATIVE = 1,
  FLUID_MOD_UNIPOLAR = 0,
  FLUID_MOD_BIPOLAR = 2,
  FLUID_MOD_LINEAR = 0,
  FLUID_MOD_CONCAVE = 4,
  FLUID_MOD_CONVEX = 8,
  FLUID_MOD_SWITCH = 12,
  FLUID_MOD_GC = 0,
  FLUID_MOD_CC = 16
};

/* Flags telling the source of a modulator.  This corresponds to
 * SF2.01 section 8.2.1 */
enum fluid_mod_src
{
  FLUID_MOD_NONE = 0,
  FLUID_MOD_VELOCITY = 2,
  FLUID_MOD_KEY = 3,
  FLUID_MOD_KEYPRESSURE = 10,
  FLUID_MOD_CHANNELPRESSURE = 13,
  FLUID_MOD_PITCHWHEEL = 14,
  FLUID_MOD_PITCHWHEELSENS = 16
};

/* Allocates memory for a new modulator */
FLUIDSYNTH_API fluid_mod_t * fluid_mod_new(void);

/* Frees the modulator */
FLUIDSYNTH_API void fluid_mod_delete(fluid_mod_t * mod);


FLUIDSYNTH_API void fluid_mod_set_source1(fluid_mod_t* mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_source2(fluid_mod_t* mod, int src, int flags);
FLUIDSYNTH_API void fluid_mod_set_dest(fluid_mod_t* mod, int dst);
FLUIDSYNTH_API void fluid_mod_set_amount(fluid_mod_t* mod, double amount);

FLUIDSYNTH_API int fluid_mod_get_source1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags1(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_source2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_flags2(fluid_mod_t* mod);
FLUIDSYNTH_API int fluid_mod_get_dest(fluid_mod_t* mod);
FLUIDSYNTH_API double fluid_mod_get_amount(fluid_mod_t* mod);


/* Determines, if two modulators are 'identical' (all parameters
   except the amount match) */
FLUIDSYNTH_API int fluid_mod_test_identity(fluid_mod_t * mod1, fluid_mod_t * mod2);
/**
 * @file gen.h
 * @brief Functions and defines for SoundFont generator effects.
 */

/**
 * Generator (effect) numbers (Soundfont 2.01 specifications section 8.1.3)
 */
enum fluid_gen_type {
  GEN_STARTADDROFS,		/**< Sample start address offset (0-32767) */
  GEN_ENDADDROFS,		/**< Sample end address offset (-32767-0) */
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
  GEN_CHORUSSEND,		/**< Chorus send amount */
  GEN_REVERBSEND,		/**< Reverb send amount */
  GEN_PAN,			/**< Stereo panning */
  GEN_UNUSED2,			/**< Unused */
  GEN_UNUSED3,			/**< Unused */
  GEN_UNUSED4,			/**< Unused */
  GEN_MODLFODELAY,		/**< Modulation LFO delay */
  GEN_MODLFOFREQ,		/**< Modulation LFO frequency */
  GEN_VIBLFODELAY,		/**< Vibrato LFO delay */
  GEN_VIBLFOFREQ,		/**< Vibrato LFO frequency */
  GEN_MODENVDELAY,		/**< Modulation envelope delay */
  GEN_MODENVATTACK,		/**< Modulation envelope attack */
  GEN_MODENVHOLD,		/**< Modulation envelope hold */
  GEN_MODENVDECAY,		/**< Modulation envelope decay */
  GEN_MODENVSUSTAIN,		/**< Modulation envelope sustain */
  GEN_MODENVRELEASE,		/**< Modulation envelope release */
  GEN_KEYTOMODENVHOLD,		/**< Key to modulation envelope hold */
  GEN_KEYTOMODENVDECAY,		/**< Key to modulation envelope decay */
  GEN_VOLENVDELAY,		/**< Volume envelope delay */
  GEN_VOLENVATTACK,		/**< Volume envelope attack */
  GEN_VOLENVHOLD,		/**< Volume envelope hold */
  GEN_VOLENVDECAY,		/**< Volume envelope decay */
  GEN_VOLENVSUSTAIN,		/**< Volume envelope sustain */
  GEN_VOLENVRELEASE,		/**< Volume envelope release */
  GEN_KEYTOVOLENVHOLD,		/**< Key to volume envelope hold */
  GEN_KEYTOVOLENVDECAY,		/**< Key to volume envelope decay */
  GEN_INSTRUMENT,		/**< Instrument ID (shouldn't be set by user) */
  GEN_RESERVED1,		/**< Reserved */
  GEN_KEYRANGE,			/**< MIDI note range */
  GEN_VELRANGE,			/**< MIDI velocity range */
  GEN_STARTLOOPADDRCOARSEOFS,	/**< Sample start loop address coarse offset (X 32768) */
  GEN_KEYNUM,			/**< Fixed MIDI note number */
  GEN_VELOCITY,			/**< Fixed MIDI velocity value */
  GEN_ATTENUATION,		/**< Initial volume attenuation */
  GEN_RESERVED2,		/**< Reserved */
  GEN_ENDLOOPADDRCOARSEOFS,	/**< Sample end loop address coarse offset (X 32768) */
  GEN_COARSETUNE,		/**< Coarse tuning */
  GEN_FINETUNE,			/**< Fine tuning */
  GEN_SAMPLEID,			/**< Sample ID (shouldn't be set by user) */
  GEN_SAMPLEMODE,		/**< Sample mode flags */
  GEN_RESERVED3,		/**< Reserved */
  GEN_SCALETUNE,		/**< Scale tuning */
  GEN_EXCLUSIVECLASS,		/**< Exclusive class number */
  GEN_OVERRIDEROOTKEY,		/**< Sample root note override */

  /* the initial pitch is not a "standard" generator. It is not
   * mentioned in the list of generator in the SF2 specifications. It
   * is used, however, as the destination for the default pitch wheel
   * modulator. */
  GEN_PITCH,			/**< Pitch (NOTE: Not a real SoundFont generator) */
  GEN_LAST			/**< Value defines the count of generators (#fluid_gen_type) */
};


/**
 * SoundFont generator structure.
 */
typedef struct _fluid_gen_t
{
  unsigned char flags; /**< Is the generator set or not (#fluid_gen_flags) */
  double val;          /**< The nominal value */
  double mod;          /**< Change by modulators */
  double nrpn;         /**< Change by NRPN messages */
} fluid_gen_t;

/**
 * Enum value for 'flags' field of #_fluid_gen_t (not really flags).
 */
enum fluid_gen_flags
{
  GEN_UNUSED,		/**< Generator value is not set */
  GEN_SET,		/**< Generator value is set */
  GEN_ABS_NRPN		/**< DOCME */
};

FLUIDSYNTH_API int fluid_gen_set_default_values(fluid_gen_t* gen);
  /*
   *  The interface to the synthesizer's voices
   *  Examples on using them can be found in fluid_defsfont.c
   */

  /** Update all the synthesis parameters, which depend on generator gen.
      This is only necessary after changing a generator of an already operating voice.
      Most applications will not need this function.*/

FLUIDSYNTH_API void fluid_voice_update_param(fluid_voice_t* voice, int gen);


  /* for fluid_voice_add_mod */
enum fluid_voice_add_mod{
  FLUID_VOICE_OVERWRITE,
  FLUID_VOICE_ADD,
  FLUID_VOICE_DEFAULT
};

  /* Add a modulator to a voice (SF2.1 only). */
FLUIDSYNTH_API void fluid_voice_add_mod(fluid_voice_t* voice, fluid_mod_t* mod, int mode);

  /** Set the value of a generator */
FLUIDSYNTH_API void fluid_voice_gen_set(fluid_voice_t* voice, int gen, float val);

  /** Get the value of a generator */
FLUIDSYNTH_API float fluid_voice_gen_get(fluid_voice_t* voice, int gen);

  /** Modify the value of a generator by val */
FLUIDSYNTH_API void fluid_voice_gen_incr(fluid_voice_t* voice, int gen, float val);


  /** Return the unique ID of the noteon-event. A sound font loader
   *  may store the voice processes it has created for * real-time
   *  control during the operation of a voice (for example: parameter
   *  changes in sound font editor). The synth uses a pool of
   *  voices, which are 'recycled' and never deallocated.
   *
   * Before modifying an existing voice, check
   * - that its state is still 'playing'
   * - that the ID is still the same
   * Otherwise the voice has finished playing.
   */
FLUIDSYNTH_API unsigned int fluid_voice_get_id(fluid_voice_t* voice);


FLUIDSYNTH_API int fluid_voice_is_playing(fluid_voice_t* voice);

  /** If the peak volume during the loop is known, then the voice can
   * be released earlier during the release phase. Otherwise, the
   * voice will operate (inaudibly), until the envelope is at the
   * nominal turnoff point. In many cases the loop volume is many dB
   * below the maximum volume.  For example, the loop volume for a
   * typical acoustic piano is 20 dB below max.  Taking that into
   * account in the turn-off algorithm we can save 20 dB / 100 dB =>
   * 1/5 of the total release time.
   * So it's a good idea to call fluid_voice_optimize_sample
   * on each sample once.
   */

FLUIDSYNTH_API int fluid_voice_optimize_sample(fluid_sample_t* s);

#endif /* _FLUIDSYNTH_H */

