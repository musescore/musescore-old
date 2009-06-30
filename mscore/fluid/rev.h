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


#ifndef _FLUID_REV_H
#define _FLUID_REV_H

#include "priv.h"

namespace FluidS {

static const int numcombs = 8;
static const int numallpasses = 4;

//---------------------------------------------------------
//   Allpass
//---------------------------------------------------------

class Allpass {
      fluid_real_t feedback;
      fluid_real_t *buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void init();
      void setfeedback(fluid_real_t val) { feedback = val;  }
      fluid_real_t getfeedback() const   { return feedback; }

      fluid_real_t process(fluid_real_t _input) {
            fluid_real_t bufout = buffer[bufidx];
            fluid_real_t output = bufout - _input;
            buffer[bufidx] = _input + (bufout * feedback);
            if (++bufidx >= bufsize)
                  bufidx = 0;
            return output;
            }
      };

//---------------------------------------------------------
//   Comb
//---------------------------------------------------------

class Comb {
      fluid_real_t feedback;
      fluid_real_t filterstore;
      fluid_real_t damp1;
      fluid_real_t damp2;
      fluid_real_t *buffer;
      int bufsize;
      int bufidx;

   public:
      void setbuffer(int size);
      void init();
      void setdamp(fluid_real_t val);
      fluid_real_t getdamp() const       { return damp1;    }
      void setfeedback(fluid_real_t val) { feedback = val;  }
      fluid_real_t getfeedback() const   { return feedback; }

      fluid_real_t process(fluid_real_t input) {
            fluid_real_t tmp = buffer[bufidx];
            filterstore      = (tmp * damp2) + (filterstore * damp1);
            buffer[bufidx]   = input + (filterstore * feedback);
            if (++bufidx >= bufsize)
                  bufidx = 0;
            return tmp;
            }
      };

//---------------------------------------------------------
//   Reverb
//---------------------------------------------------------

class Reverb {
      static const float scaleroom = 0.28f;
      static const float offsetroom = 0.7f;
      static const float scalewet = 3.0f;
      static const float scaledamp = 0.4f;

      void update();
      void init();

      fluid_real_t roomsize;
      fluid_real_t damp;
      fluid_real_t wet, wet1, wet2;
      fluid_real_t width;
      fluid_real_t gain;
      /*
       The following are all declared inline
       to remove the need for dynamic allocation
       with its subsequent error-checking messiness
       */

      Comb combL[numcombs];
      Comb combR[numcombs];

      Allpass allpassL[numallpasses];
      Allpass allpassR[numallpasses];

   public:
      Reverb();
      void process(float* in, float* left_out, float* right_out);

      void reset() { init(); }

      void setroomsize(fluid_real_t value) { roomsize = (value * scaleroom) + offsetroom; }
      void setdamp(fluid_real_t value)     { damp = value * scaledamp; }
      void setlevel(fluid_real_t value)    { wet = value * scalewet;   }
      void setwidth(fluid_real_t value)    { width = value;            }
      void setmode(fluid_real_t value);
      fluid_real_t getroomsize() const     { return (roomsize - offsetroom) / scaleroom; }
      fluid_real_t getdamp() const         { return damp / scaledamp; }
      fluid_real_t getlevel() const        { return wet / scalewet;   }
      fluid_real_t getwidth()              { return width; }

      bool setPreset(int);
      };

//---------------------------------------------------------
//   ReverbPreset
//---------------------------------------------------------

struct ReverbPreset {
      const char* name;
      fluid_real_t roomsize;
      fluid_real_t damp;
      fluid_real_t width;
      fluid_real_t level;
      };
}
#endif /* _FLUID_REV_H */
