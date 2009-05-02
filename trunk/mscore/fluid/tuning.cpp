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

#include "tuning.h"
#include "priv.h"

namespace FluidS {

Tuning::Tuning(char* n, int b, int p)
      {
      _name = 0;
      setName(n);
      bank = b;
      prog = p;

      for (int i = 0; i < 128; i++)
            pitch[i] = i * 100.0;
      }

Tuning::~Tuning()
      {
      if (_name)
            free(_name);
      }

void Tuning::setName(char* n)
      {
      if (_name)
            free(_name);
      _name = 0;
      if (n)
            _name = strdup(n);
      }

void Tuning::setOctave(double* pitch_deriv)
      {
      for (int i = 0; i < 128; i++)
            pitch[i] = i * 100.0 + pitch_deriv[i % 12];
      }

void Tuning::setAll(double* pitch)
      {
      for (int i = 0; i < 128; i++)
            pitch[i] = pitch[i];
      }

void Tuning::setPitch(int k, double p)
      {
      if ((k >= 0) && (k < 128))
            pitch[k] = p;
      }
}

