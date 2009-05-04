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

#define numcombs 8
#define numallpasses 4

#define stereospread 23

/*
 These values assume 44.1KHz sample rate
 they will probably be OK for 48KHz sample rate
 but would need scaling for 96KHz (or other) sample rates.
 The values were obtained by listening tests.
*/
#define combtuningL1 1116
#define combtuningR1 1116 + stereospread
#define combtuningL2 1188
#define combtuningR2 1188 + stereospread
#define combtuningL3 1277
#define combtuningR3 1277 + stereospread
#define combtuningL4 1356
#define combtuningR4 1356 + stereospread
#define combtuningL5 1422
#define combtuningR5 1422 + stereospread
#define combtuningL6 1491
#define combtuningR6 1491 + stereospread
#define combtuningL7 1557
#define combtuningR7 1557 + stereospread
#define combtuningL8 1617
#define combtuningR8 1617 + stereospread
#define allpasstuningL1 556
#define allpasstuningR1 556 + stereospread
#define allpasstuningL2 441
#define allpasstuningR2 441 + stereospread
#define allpasstuningL3 341
#define allpasstuningR3 341 + stereospread
#define allpasstuningL4 225
#define allpasstuningR4 225 + stereospread


struct fluid_allpass {
      fluid_real_t feedback;
      fluid_real_t *buffer;
      int bufsize;
      int bufidx;
      };

struct fluid_comb {
      fluid_real_t feedback;
      fluid_real_t filterstore;
      fluid_real_t damp1;
      fluid_real_t damp2;
      fluid_real_t *buffer;
      int bufsize;
      int bufidx;
      };

//---------------------------------------------------------
//   Reverb
//---------------------------------------------------------

class Reverb {

      void update();
      void init();

   public:
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
      /* Comb filters */
      fluid_comb combL[numcombs];
      fluid_comb combR[numcombs];
      /* Allpass filters */
      fluid_allpass allpassL[numallpasses];
      fluid_allpass allpassR[numallpasses];
      /* Buffers for the combs */
      fluid_real_t bufcombL1[combtuningL1];
      fluid_real_t bufcombR1[combtuningR1];
      fluid_real_t bufcombL2[combtuningL2];
      fluid_real_t bufcombR2[combtuningR2];
      fluid_real_t bufcombL3[combtuningL3];
      fluid_real_t bufcombR3[combtuningR3];
      fluid_real_t bufcombL4[combtuningL4];
      fluid_real_t bufcombR4[combtuningR4];
      fluid_real_t bufcombL5[combtuningL5];
      fluid_real_t bufcombR5[combtuningR5];
      fluid_real_t bufcombL6[combtuningL6];
      fluid_real_t bufcombR6[combtuningR6];
      fluid_real_t bufcombL7[combtuningL7];
      fluid_real_t bufcombR7[combtuningR7];
      fluid_real_t bufcombL8[combtuningL8];
      fluid_real_t bufcombR8[combtuningR8];
      /* Buffers for the allpasses */
      fluid_real_t bufallpassL1[allpasstuningL1];
      fluid_real_t bufallpassR1[allpasstuningR1];
      fluid_real_t bufallpassL2[allpasstuningL2];
      fluid_real_t bufallpassR2[allpasstuningR2];
      fluid_real_t bufallpassL3[allpasstuningL3];
      fluid_real_t bufallpassR3[allpasstuningR3];
      fluid_real_t bufallpassL4[allpasstuningL4];
      fluid_real_t bufallpassR4[allpasstuningR4];

      Reverb();
      void processmix(fluid_real_t *in, fluid_real_t *left_out, fluid_real_t *right_out);

      void reset() { init(); }

      void setroomsize(fluid_real_t value);
      void setdamp(fluid_real_t value);
      void setlevel(fluid_real_t value);
      void setwidth(fluid_real_t value);
      void setmode(fluid_real_t value);

      fluid_real_t getroomsize();
      fluid_real_t getdamp();
      fluid_real_t getlevel();
      fluid_real_t getwidth() { return width; }
      };

/*
 * reverb preset
 */
struct fluid_revmodel_presets_t {
      const char* name;
      fluid_real_t roomsize;
      fluid_real_t damp;
      fluid_real_t width;
      fluid_real_t level;
      };

}

#endif /* _FLUID_REV_H */
