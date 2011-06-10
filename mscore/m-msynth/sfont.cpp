/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * SoundFont file loading code borrowed from Smurf SoundFont Editor
 * Copyright (C) 1999-2001 Josh Green
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

#include "sfont.h"
#include "fluid.h"
#include "voice.h"

namespace FluidS {

//---------------------------------------------------------
//   Zone
//---------------------------------------------------------

Zone::Zone(int a, int b, int c, int d, int nGen, Generator** g)
      {
      fluid_gen_set_default_values(&genlist[0]);
      keylo = a;
      keyhi = b;
      vello = c;
      velhi = d;
      for (int i = 0; i < nGen; ++i) {
            Generator* gen    = *g++;
            genlist[gen->idx] = *gen;
            }
      }

Generator::Generator(int i, int v)
      {
      idx   = i;
      val   = (float) v;
      flags = GEN_SET;
      }

//---------------------------------------------------------
//   get_preset
//---------------------------------------------------------

Preset* SFont::preset(int bank, int num)
      {
      for (int i = 0; i < _nPresets; ++i) {
            Preset* p = presets[i];
            if ((p->get_banknum() == bank) && (p->get_num() == num))
                  return p;
            }
      return 0;
      }

//---------------------------------------------------------
//   noteon
//---------------------------------------------------------

bool Preset::noteon(Fluid* synth, int chan, int key, int vel, float nt)
      {
      Mod* mod;
      Mod* mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */

      for (int i = 0; i < _nZones; ++i) {
            PZone* pz = zones[i];
            if (!pz->inside_range(key, vel))
                  continue;
            Instrument* inst        = pz->get_inst();
            IZone* global_inst_zone = inst->globalZone();

            for (int k = 0; k < inst->nZones(); ++k) {
                  IZone* inst_zone = inst->zone(k);
                  Sample* sample   = inst_zone->get_sample();
                  if (!(sample && inst_zone->inside_range(key, vel)))
                        continue;

                  Voice* voice = synth->alloc_voice(sample, chan, key, vel, nt);
                  if (voice == 0)
                        return false;

                  /* Instrumentrument level, generators */

                  for (int i = 0; i < GEN_LAST; i++) {
            	      /* SF 2.01 section 9.4 'bullet' 4:
            	       *
            	       * A generator in a local instrument zone supersedes a
            	       * global instrument zone generator.  Both cases supersede
            	       * the default generator -> voice_gen_set
                         */

	                  if (inst_zone->genlist[i].flags)
                              voice->gen_set(i, inst_zone->genlist[i].val);
                        else if ((global_inst_zone != 0) && (global_inst_zone->genlist[i].flags))
                              voice->gen_set(i, global_inst_zone->genlist[i].val);
                        else {
                              /* The generator has not been defined in this instrument.
                               * Do nothing, leave it at the default.
                               */
                              }

                        }

                  /* global instrument zone, modulators: Put them all into a
                     * list. */

                  int mod_list_count = 0;

                  if (global_inst_zone) {
                        foreach(Mod* mod, global_inst_zone->modlist)
                              mod_list[mod_list_count++] = mod;
                        }

                  /* local instrument zone, modulators.
                   * Replace modulators with the same definition in the list:
                   * SF 2.01 page 69, 'bullet' 8
                   */
                  foreach(Mod* mod, inst_zone->modlist) {
	                  /* 'Identical' modulators will be deleted by setting their
	                   *  list entry to 0.  The list length is known, 0
	                   *  entries will be ignored later.  SF2.01 section 9.5.1
                         *  page 69, 'bullet' 3 defines 'identical'.  */

                        for (int i = 0; i < mod_list_count; i++){
                              if (mod_list[i] && test_identity(mod, mod_list[i])){
                                    mod_list[i] = 0;
	                              }
	                        }

                        /* Finally add the new modulator to to the list. */
                        mod_list[mod_list_count++] = mod;
                        }

                  /* Add instrument modulators (global / local) to the voice. */
                  for (int i = 0; i < mod_list_count; i++){
                        mod = mod_list[i];
                        if (mod) {  // disabled modulators CANNOT be skipped.
                              /* Instrumentrument modulators -supersede- existing (default)
	                         * modulators.  SF 2.01 page 69, 'bullet' 6 */
                              voice->add_mod(mod, FLUID_VOICE_OVERWRITE);
                              }
                        }

                  /* Preset level, generators */

                  for (int i = 0; i < GEN_LAST; i++) {
                        /* SF 2.01 section 8.5 page 58: If some generators are
                         * encountered at preset level, they should be ignored */
                        if ((i != GEN_STARTADDROFS)
                           && (i != GEN_ENDADDROFS)
                           && (i != GEN_STARTLOOPADDROFS)
                           && (i != GEN_ENDLOOPADDROFS)
                           && (i != GEN_STARTADDRCOARSEOFS)
                           && (i != GEN_ENDADDRCOARSEOFS)
                           && (i != GEN_STARTLOOPADDRCOARSEOFS)
                           && (i != GEN_KEYNUM)
                           && (i != GEN_VELOCITY)
                           && (i != GEN_ENDLOOPADDRCOARSEOFS)
                           && (i != GEN_SAMPLEMODE)
                           && (i != GEN_EXCLUSIVECLASS)
                           && (i != GEN_OVERRIDEROOTKEY)) {

                              /* SF 2.01 section 9.4 'bullet' 9: A generator in a
	                         * local preset zone supersedes a global preset zone
	                         * generator.  The effect is -added- to the destination
	                         * summing node -> voice_gen_incr */

                              if (pz->genlist[i].flags) {
                                    voice->gen_incr(i, pz->genlist[i].val);
                                    }
                              else if (_global_zone && _global_zone->genlist[i].flags) {
                                    voice->gen_incr(i, _global_zone->genlist[i].val);
                                    }
                              else {
                                    /* The generator has not been defined in this preset
                                     * Do nothing, leave it unchanged.
                                     */
                                    }
                              } /* if available at preset level */
                        } /* for all generators */


                  /* Global preset zone, modulators: put them all into a
                   * list. */

                  mod_list_count = 0;
                  if (_global_zone) {
                        foreach(Mod* mod, _global_zone->modlist)
                              mod_list[mod_list_count++] = mod;
                        }

                  /* Process the modulators of the local preset zone.  Kick
                   * out all identical modulators from the global preset zone
                   */

                  foreach(Mod* mod, pz->modlist) {
                        for (int i = 0; i < mod_list_count; i++) {
                              if (mod_list[i] && test_identity(mod, mod_list[i]))
                                    mod_list[i] = 0;
                              }
                        mod_list[mod_list_count++] = mod;
                        }

                  /* Add preset modulators (global / local) to the voice. */
                  for (int i = 0; i < mod_list_count; i++) {
                        mod = mod_list[i];
                        if ((mod == 0) || (mod->amount == 0))
                              continue;
                        voice->add_mod(mod, FLUID_VOICE_ADD);
                        }
	            synth->start_voice(voice);
                  }
            }
      return true;
      }
}

