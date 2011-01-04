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


#ifndef _FLUID_CHORUS_H
#define _FLUID_CHORUS_H

namespace FluidS {

#define MAX_CHORUS	99

/* Interpolate how many steps between samples? Must be power of two
   For example: 8 => use a resolution of 256 steps between any two
   samples
*/
#define INTERPOLATION_SUBSAMPLES_LN2 8
#define INTERPOLATION_SUBSAMPLES (1 << (INTERPOLATION_SUBSAMPLES_LN2-1))
#define INTERPOLATION_SUBSAMPLES_ANDMASK (INTERPOLATION_SUBSAMPLES-1)

/* Use how many samples for interpolation? Must be odd.  '7' sounds
   relatively clean, when listening to the modulated delay signal
   alone.  For a demo on aliasing try '1' With '3', the aliasing is
   still quite pronounced for some input frequencies
*/
#define INTERPOLATION_SAMPLES 5


//---------------------------------------------------------
//   Chorus
//---------------------------------------------------------

class Chorus {
      static void triangle(int *buf, int len, int depth);
      static void sine(int *buf, int len, int depth);

      /* Store the values between fluid_chorus_set_xxx and fluid_chorus_update
      * Logic behind this:
      * - both 'parameter' and 'new_parameter' hold the same value.
      * - To change the chorus settings, 'new_parameter' is modified and
      *   fluid_chorus_update is called.
      * - If the new value is valid, it is copied to 'parameter'.
      * - If it is invalid, 'new_parameter' is restored to 'parameter'.
      */

      int type;                     // current value
      int new_type;                 // next value, if parameter check is OK
      float depth_ms;               // current value
      float new_depth_ms;           // next value, if parameter check is OK
      float level;                  // current value
      float new_level;              // next value, if parameter check is OK
      float speed_Hz;               // current value
      float new_speed_Hz;           // next value, if parameter check is OK
      int number_blocks;            // current value
      int new_number_blocks;        // next value, if parameter check is OK

      float *chorusbuf;
      int counter;
      long phase[MAX_CHORUS];
      long modulation_period_samples;
      int* lookup_tab;
      float sample_rate;

      /* sinc lookup table */
      float sinc_table[INTERPOLATION_SAMPLES][INTERPOLATION_SUBSAMPLES];

   public:
      Chorus(float sample_rate);
      ~Chorus();

      void update();
#if 0
      void set_nr(int nr)                      { new_number_blocks = nr;       }
      void set_level(float level)              { new_level         = level;    }
      void set_speed_Hz(float speed_Hz)        { new_speed_Hz      = speed_Hz; }
      void set_depth_ms(float depth_ms)        { new_depth_ms      = depth_ms; }
      void set_type(int type)                  { new_type          = type;     }
#endif
      void process(int, float *in, float *left_out, float *right_out);
      void reset();

      int get_nr() const         { return number_blocks; }
      float get_level() const    { return level;    }
      float get_speed_Hz() const { return speed_Hz; }
      float get_depth_ms() const { return depth_ms; }
      int get_type() const       { return type;     }

      void setParameter(int parameter, double value);
      double parameter(int idx) const;
      };

}
#endif /* _FLUID_CHORUS_H */
