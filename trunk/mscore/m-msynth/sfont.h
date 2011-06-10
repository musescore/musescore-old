/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont loading code borrowed from Smurf SoundFont Editor by Josh Green
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

#ifndef _FLUID_DEFSFONT_H
#define _FLUID_DEFSFONT_H

#include "fluid.h"


namespace FluidS {

class Preset;
class Sample;
class Instrument;
struct SFGen;
struct SFMod;

struct SFChunk;

//---------------------------------------------------------
//   SFVersion
//---------------------------------------------------------

struct SFVersion {            // version structure
      unsigned short major, minor;
      SFVersion();
      };

//---------------------------------------------------------
//   SFont
//---------------------------------------------------------

class SFont {
      int _nPresets;
      Preset** presets;

   public:
      SFont(int n, Preset** p) : _nPresets(n), presets(p) {}
      Preset* preset(int bank, int prenum);
      friend class Preset;
      };

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

class Sample {

   public:
      uint start;
      uint end;
      uint loopstart;
      uint loopend;
      uint samplerate;
      int origpitch;
      int pitchadj;
      int sampletype;

      const short* data;

      bool amplitude_that_reaches_noise_floor_is_valid;
      float amplitude_that_reaches_noise_floor;

      void optimize();
      Sample(uint a, uint b, uint c, uint d, uint e, int f, int g, int h,
         const short* k)
         : start(a), end(b), loopstart(c), loopend(d),
           samplerate(e), origpitch(f), pitchadj(g), sampletype(h),
          data(k)
            {
            amplitude_that_reaches_noise_floor_is_valid = false;
            amplitude_that_reaches_noise_floor = 0.0;
            }
      };

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

class Zone {
      int keylo, keyhi, vello, velhi;

   public:
      Generator   genlist[GEN_LAST];
      QList<Mod*> modlist;        // List of modulators

   public:
      Zone(int a, int b, int c, int d, int nGen, Generator** g);
      bool inside_range(int key, int vel) const {
            return ((keylo <= key) && (keyhi >= key) && (vello <= vel) && (velhi >= vel));
            }
      };

//---------------------------------------------------------
//   PZone
//---------------------------------------------------------

class PZone : public Zone {
      Instrument* instrument;

   public:
      PZone(int a, int b, int c, int d, Instrument* i)
         : Zone(a, b, c, d, 0, 0), instrument(i) {}
      Instrument* get_inst() const { return instrument; }
      };

//---------------------------------------------------------
//   IZone
//---------------------------------------------------------

class IZone : public Zone {
      Sample* sample;

   public:
      IZone(int a, int b, int c, int d, Sample* s, int nGen, Generator** generators)
         : Zone(a, b, c, d, nGen, generators), sample(s) {}
      Sample* get_sample() const { return sample; }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

class Instrument {
      IZone* _globalZone;
      int _nZones;
      IZone** _zones;

   public:
      Instrument(IZone* g, int n, IZone** z) : _globalZone(g), _nZones(n), _zones(z) {}
      IZone* globalZone() const { return _globalZone; }
      IZone* zone(int k)        { return _zones[k];   }
      int nZones() const        { return _nZones;     }
      };

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

class Preset {
      int bank, num;
      PZone* _global_zone;           // the global zone of the preset
      int _nZones;
      PZone** zones;

   public:
      Preset(int a, int b, PZone* g, int n, PZone** z)
         : bank(a), num(b), _global_zone(g), _nZones(n), zones(z) {}

      int get_banknum() const      { return bank; }
      int get_num() const          { return num;  }
      bool noteon(Fluid*, int chan, int key, int vel, float nt);
      };
}

#endif  /* _FLUID_SFONT_H */
