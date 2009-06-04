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


/*
  More information about micro tuning can be found at:

  http://www.midi.org/about-midi/tuning.htm
  http://www.midi.org/about-midi/tuning-scale.htm
  http://www.midi.org/about-midi/tuning_extens.htm
*/

#ifndef _FLUID_TUNING_H
#define _FLUID_TUNING_H

namespace FluidS {

//---------------------------------------------------------
//   Tuning
//---------------------------------------------------------

class Tuning {
      char* _name;
      int bank;
      int prog;
      double pitch[128];  /* the pitch of every key, in cents */

    public:
      Tuning(char* name, int bank, int prog);
      ~Tuning();
      int getBank() const          { return bank; }
      int getProg() const          { return prog; }
      double getPitch(int k) const { return pitch[k]; }
      char* name() const           { return _name; }
      void setName(char* n);
      void setKey(int k, double p) { pitch[k] = p; }

      void setPitch(int key, double pitch);
      void setOctave(double* pitch_deriv);
      void setAll(double* pitch);
      };
}

#endif /* _FLUID_TUNING_H */
