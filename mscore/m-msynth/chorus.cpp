/*
 * August 24, 1998
 * Copyright (C) 1998 Juergen Mueller And Sundry Contributors
 * This source code is freely redistributable and may be used for
 * any purpose.  This copyright notice must be maintained.
 * Juergen Mueller And Sundry Contributors are not responsible for
 * the consequences of using this software.
 */


/*

  CHANGES

  - Adapted for fluidsynth, Peter Hanappe, March 2002

  - Variable delay line implementation using bandlimited
    interpolation, code reorganization: Markus Nentwig May 2002

 */


/*
 * 	Chorus effect.
 *
 * Flow diagram scheme for n delays ( 1 <= n <= MAX_CHORUS ):
 *
 *        * gain-in                                           ___
 * ibuff -----+--------------------------------------------->|   |
 *            |      _________                               |   |
 *            |     |         |                   * level 1  |   |
 *            +---->| delay 1 |----------------------------->|   |
 *            |     |_________|                              |   |
 *            |        /|\                                   |   |
 *            :         |                                    |   |
 *            : +-----------------+   +--------------+       | + |
 *            : | Delay control 1 |<--| mod. speed 1 |       |   |
 *            : +-----------------+   +--------------+       |   |
 *            |      _________                               |   |
 *            |     |         |                   * level n  |   |
 *            +---->| delay n |----------------------------->|   |
 *                  |_________|                              |   |
 *                     /|\                                   |___|
 *                      |                                      |
 *              +-----------------+   +--------------+         | * gain-out
 *              | Delay control n |<--| mod. speed n |         |
 *              +-----------------+   +--------------+         +----->obuff
 *
 *
 * The delay i is controlled by a sine or triangle modulation i ( 1 <= i <= n).
 *
 * The delay of each block is modulated between 0..depth ms
 *
 */


/* Variable delay line implementation
 * ==================================
 *
 * The modulated delay needs the value of the delayed signal between
 * samples.  A lowpass filter is used to obtain intermediate values
 * between samples (bandlimited interpolation).  The sample pulse
 * train is convoluted with the impulse response of the low pass
 * filter (sinc function).  To make it work with a small number of
 * samples, the sinc function is windowed (Hamming window).
 *
 */

#include "chorus.h"
#include "fluid.h"

namespace FluidS {

#define MAX_DELAY	      100
#define MAX_DEPTH	      10
#define MIN_SPEED_HZ	0.29
#define MAX_SPEED_HZ    5

/* Length of one delay line in samples:
 * Set through MAX_SAMPLES_LN2.
 * For example:
 * MAX_SAMPLES_LN2=12
 * => MAX_SAMPLES=pow(2,12)=4096
 * => MAX_SAMPLES_ANDMASK=4095
 */
#define MAX_SAMPLES_LN2 12

#define MAX_SAMPLES (1 << (MAX_SAMPLES_LN2-1))
#define MAX_SAMPLES_ANDMASK (MAX_SAMPLES-1)

#define fluid_log(a, ...)

//---------------------------------------------------------
//   Chorus
//---------------------------------------------------------

Chorus::Chorus(float sr)
      {
      Chorus* chorus = this;

      memset(this, 0, sizeof(Chorus));
      sample_rate = sr;

      /* Lookup table for the SI function (impulse response of an ideal low pass) */

      /* i: Offset in terms of whole samples */
      for (int i = 0; i < INTERPOLATION_SAMPLES; i++){

            /* ii: Offset in terms of fractional samples ('subsamples') */
            for (int ii = 0; ii < INTERPOLATION_SUBSAMPLES; ii++){
                  /* Move the origin into the center of the table */
                  float i_shifted = ((float) i- ((float) INTERPOLATION_SAMPLES) / 2.
			  + (float) ii / (float) INTERPOLATION_SUBSAMPLES);
                  if (fabs(i_shifted) < 0.000001) {
                        /* sinc(0) cannot be calculated straightforward (limit needed
                           for 0/0) */
                        sinc_table[i][ii] = (float)1.;
                        }
                  else {
                        sinc_table[i][ii] = (float)sin(i_shifted * M_PI) / (M_PI * i_shifted);
                        /* Hamming window */
                        sinc_table[i][ii] *= (float)0.5 * (1.0 + cos(2.0 * M_PI * i_shifted / (float)INTERPOLATION_SAMPLES));
                        }
                  }
            }

      lookup_tab = new int[(int) (chorus->sample_rate / MIN_SPEED_HZ)];
      chorusbuf  = new float[MAX_SAMPLES];
      reset();
      }

//---------------------------------------------------------
//   reset
//---------------------------------------------------------

void Chorus::reset()
      {
      memset(chorusbuf, 0, MAX_SAMPLES * sizeof(*chorusbuf));

      set_nr(FLUID_CHORUS_DEFAULT_N);
      set_level(FLUID_CHORUS_DEFAULT_LEVEL);
      set_speed_Hz(FLUID_CHORUS_DEFAULT_SPEED);
      set_depth_ms(FLUID_CHORUS_DEFAULT_DEPTH);
      set_type(FLUID_CHORUS_MOD_SINE);

      update();
      }

//---------------------------------------------------------
//   ~Chorus
//---------------------------------------------------------

Chorus::~Chorus()
      {
      delete[] chorusbuf;
      delete[] lookup_tab;
      }

//---------------------------------------------------------
//   update
//    Calculates the internal chorus parameters using the settings from
//    fluid_chorus_set_xxx.
//---------------------------------------------------------

void Chorus::update()
      {
      if (new_number_blocks < 0) {
            fluid_log(FLUID_WARN, "chorus: number blocks must be >=0! Setting value to 0.");
            new_number_blocks = 0;
            }
      else if (new_number_blocks > MAX_CHORUS) {
            fluid_log(FLUID_WARN, "chorus: number blocks larger than max. allowed! Setting value to %d.",
               MAX_CHORUS);
            new_number_blocks = MAX_CHORUS;
            }

      if (new_speed_Hz < MIN_SPEED_HZ) {
            fluid_log(FLUID_WARN, "chorus: speed is too low (min %f)! Setting value to min.",
               (float) MIN_SPEED_HZ);
            new_speed_Hz = MIN_SPEED_HZ;
            }
      else if (new_speed_Hz > MAX_SPEED_HZ) {
            fluid_log(FLUID_WARN, "chorus: speed must be below %f Hz! Setting value to max.",
               (float) MAX_SPEED_HZ);
            new_speed_Hz = MAX_SPEED_HZ;
            }
      if (new_depth_ms < 0.0) {
            fluid_log(FLUID_WARN, "chorus: depth must be positive! Setting value to 0.");
            new_depth_ms = 0.0;
            }
      /* Depth: Check for too high value through modulation_depth_samples. */

      if (new_level < 0.0) {
            fluid_log(FLUID_WARN, "chorus: level must be positive! Setting value to 0.");
            new_level = 0.0;
            }
      else if (new_level > 10) {
            fluid_log(FLUID_WARN, "chorus: level must be < 10. A reasonable level is << 1! "
               "Setting it to 0.1.");
            new_level = 0.1;
            }

      /* The modulating LFO goes through a full period every x samples: */
      modulation_period_samples = lrint(sample_rate / new_speed_Hz);

      /* The variation in delay time is x: */
      int modulation_depth_samples = (int)
         (new_depth_ms / 1000.0  /* convert modulation depth in ms to s*/
         * sample_rate);

      if (modulation_depth_samples > MAX_SAMPLES) {
            fluid_log(FLUID_WARN, "chorus: Too high depth. Setting it to max (%d).", MAX_SAMPLES);
            modulation_depth_samples = MAX_SAMPLES;
            }

      /* initialize LFO table */
      if (type == FLUID_CHORUS_MOD_SINE)
            sine(lookup_tab, modulation_period_samples, modulation_depth_samples);
      else if (type == FLUID_CHORUS_MOD_TRIANGLE)
            triangle(lookup_tab, modulation_period_samples, modulation_depth_samples);
      else {
            fluid_log(FLUID_WARN, "chorus: Unknown modulation type. Using sinewave.");
            type = FLUID_CHORUS_MOD_SINE;
            sine(lookup_tab, modulation_period_samples, modulation_depth_samples);
            }

      for (int i = 0; i < number_blocks; i++) {
            /* Set the phase of the chorus blocks equally spaced */
            phase[i] = (int) ((float) modulation_period_samples
               * (float) i / (float) number_blocks);
            }

      /* Start of the circular buffer */
      counter       = 0;
      type          = new_type;
      depth_ms      = new_depth_ms;
      level         = new_level;
      speed_Hz      = new_speed_Hz;
      number_blocks = new_number_blocks;
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Chorus::process(int n, float *in, float *left_out, float *right_out)
      {
      for (int sample_index = 0; sample_index < n; sample_index++) {
            float d_in = in[sample_index];
            float d_out = 0.0f;

            /* Write the current sample into the circular buffer */
            chorusbuf[counter] = d_in;

            for (int i = 0; i < number_blocks; i++) {
                  /* Calculate the delay in subsamples for the delay line of chorus block nr. */

                  /* The value in the lookup table is so, that this expression
                   * will always be positive.  It will always include a number of
                   * full periods of MAX_SAMPLES*INTERPOLATION_SUBSAMPLES to
                   * remain positive at all times.
                   */
                  int pos_subsamples = (INTERPOLATION_SUBSAMPLES * counter
                     - lookup_tab[phase[i]]);

                  int pos_samples = pos_subsamples/INTERPOLATION_SUBSAMPLES;

                  /* modulo divide by INTERPOLATION_SUBSAMPLES */
                  pos_subsamples &= INTERPOLATION_SUBSAMPLES_ANDMASK;

                  for (int ii = 0; ii < INTERPOLATION_SAMPLES; ii++) {
	                  /* Add the delayed signal to the chorus sum d_out Note: The
	                   * delay in the delay line moves backwards for increasing
	                   * delay!*/

	                  /* The & in chorusbuf[...] is equivalent to a division modulo
	                     MAX_SAMPLES, only faster.
                         */
                        d_out += (chorusbuf[pos_samples & MAX_SAMPLES_ANDMASK]
                           * sinc_table[ii][pos_subsamples]);
                        pos_samples--;
                        }
                  /* Cycle the phase of the modulating LFO */
                  phase[i]++;
                  phase[i] %= (modulation_period_samples);
                  } /* foreach chorus block */

            d_out *= level;

            /* Add the chorus sum d_out to output */
            left_out[sample_index]  += d_out;
            right_out[sample_index] += d_out;

            /* Move forward in circular buffer */
            counter++;
            counter %= MAX_SAMPLES;
            }
      }

/* Purpose:
 *
 * Calculates a modulation waveform (sine) Its value ( modulo
 * MAXSAMPLES) varies between 0 and depth*INTERPOLATION_SUBSAMPLES.
 * Its period length is len.  The waveform data will be used modulo
 * MAXSAMPLES only.  Since MAXSAMPLES is substracted from the waveform
 * a couple of times here, the resulting (current position in
 * buffer)-(waveform sample) will always be positive.
 */
void Chorus::sine(int *buf, int len, int depth)
      {
      for (int i = 0; i < len; i++) {
            float val = sin((float) i / (float)len * 2.0 * M_PI);
            buf[i] = (int) ((1.0 + val) * (float) depth / 2.0 * (float) INTERPOLATION_SUBSAMPLES);
            buf[i] -= 3* MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
            }
      }

/* Purpose:
 * Calculates a modulation waveform (triangle)
 * See fluid_chorus_sine for comments.
 */
void Chorus::triangle(int *buf, int len, int depth)
      {
      int i=0;
      int ii=len-1;
      float val;
      float val2;

      while (i <= ii){
            val = i * 2.0 / len * (float)depth * (float) INTERPOLATION_SUBSAMPLES;
            val2= (int) (val + 0.5) - 3 * MAX_SAMPLES * INTERPOLATION_SUBSAMPLES;
            buf[i++] = (int) val2;
            buf[ii--] = (int) val2;
            }
      }

//---------------------------------------------------------
//   setParameter
//---------------------------------------------------------

void Chorus::setParameter(int idx, float value)
      {
      switch (idx) {
            case 0: type = lrint(value); break;
            case 1: speed_Hz = value * MAX_SPEED_HZ + MIN_SPEED_HZ; break;
            case 2: depth_ms = value * MAX_DEPTH; break;
            case 3: number_blocks = lrint(value * 100.0); break;
            case 4: new_level = value; break;
            }
      }

//---------------------------------------------------------
//   parameter
//---------------------------------------------------------

float Chorus::parameter(int idx) const
      {
      switch (idx) {
            case 0: return type;
            case 1: return (speed_Hz-MIN_SPEED_HZ) / MAX_SPEED_HZ;
            case 2: return depth_ms / MAX_DEPTH;
            case 3: return number_blocks / 100.0;
            case 4: return level;
            }
      return 0.0;
      }

}
