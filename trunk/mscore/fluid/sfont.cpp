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

namespace FluidS {

//---------------------------------------------------------
//   SFont
//---------------------------------------------------------

SFont::SFont()
      {
      samplepos  = 0;
      samplesize = 0;
      }

SFont::~SFont()
      {
      foreach(Sample* s, sample)
            delete s;
      foreach(Preset* p, presets)
            delete p;
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

bool SFont::load(const QString& s)
      {
      filename = s;

      /* The actual loading is done in the sfont and sffile files */

      SFData sfdata(filename, this);
      if (!(sfdata.open(QIODevice::ReadOnly))) {
            FLUID_LOG (FLUID_ERR, "Unable to open file \"%s\"", qPrintable(filename));
            return false;
            }
      if (!sfdata.load())
            return false;

      /* Create all the sample headers */
      sample = sfdata.sample;
      sfdata.sample.clear();

      /* Load all the presets */
      foreach(SFPreset* sfpreset, sfdata.preset) {
            Preset* preset = new Preset(this);
            if (preset->importSfont(sfpreset, this) != FLUID_OK)
                  return FLUID_FAILED;
            add_preset(preset);
            }
      return true;
      }

//---------------------------------------------------------
//   get_sample
//---------------------------------------------------------

Sample* SFont::get_sample(char *s)
      {
      foreach(Sample* sa, sample) {
            if (strcmp(sa->name, s) == 0)
                  return sa;
            }
      return 0;
      }

//---------------------------------------------------------
//   get_preset
//---------------------------------------------------------

Preset* SFont::get_preset(int bank, int num)
      {
      foreach(Preset* p, presets) {
            if ((p->get_banknum() == bank) && (p->get_num() == num))
                  return p;
            }
      return 0;
      }

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

Preset::Preset(SFont* s)
      {
      sfont        = s;
      bank         = 0;
      num          = 0;
      _global_zone = 0;
      }

//---------------------------------------------------------
//   Preset
//---------------------------------------------------------

Preset::~Preset()
      {
      if (_global_zone) {
            delete _global_zone;
            _global_zone = 0;
            }
      foreach(PresetZone* z, zones)
            delete z;
      }

//---------------------------------------------------------
//   loadSamples
//    this is called if the preset is associated with a
//    channel
//---------------------------------------------------------

void Preset::loadSamples()
      {
      if (_global_zone && _global_zone->inst) {
            if (_global_zone->inst->global_zone && _global_zone->inst->global_zone->sample)
                  _global_zone->inst->global_zone->sample->load();
            foreach(InstZone* iz, _global_zone->inst->zone)
                  iz->sample->load();
            }

      foreach(PresetZone* z, zones) {
            if (z->inst->global_zone && z->inst->global_zone->sample)
                  z->inst->global_zone->sample->load();
            foreach(InstZone* iz, z->inst->zone)
                  iz->sample->load();
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Sample::load()
      {
      if (!_valid || data)
            return;

      QFile fd(sf->get_name());

      if (!fd.open(QIODevice::ReadOnly))
            return;
      if (!fd.seek(sf->samplePos() + start * sizeof(short)))
            return;
      unsigned int size = end - start;
      data              = new short[size];
      size              *= sizeof(short);

      if (fd.read((char*)data, size) != size)
            return;

      /* I'm not sure this endian test is waterproof...  */
      unsigned short endian = 0x0100;

      /* If this machine is big endian, the sample have to byte swapped  */
      if (((char *) &endian)[0]) {
            unsigned char hi, lo;
            unsigned int i, j;
            short s;
            unsigned char* cbuf = (unsigned char*) data;
            for (i = 0, j = 0; j < size; i++) {
                  lo = cbuf[j++];
                  hi = cbuf[j++];
                  s = (hi << 8) | lo;
                  data[i] = s;
                  }
            }
      end       -= start - 1;       // marks last sample, contrary to SF spec.
      loopstart -= start;
      loopend   -= start;
      start      = 0;
      optimize();
      }

//---------------------------------------------------------
//   noteon
//---------------------------------------------------------

int Preset::noteon(Fluid* synth, unsigned id, int chan, int key, int vel, double nt)
      {
      InstZone *global_inst_zone, *z;
      Sample* sample;
      Voice* voice;
      Mod* mod;
      Mod* mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */
      int mod_list_count;

      PresetZone* global_preset_zone = global_zone();

      /* run thru all the zones of this preset */
      foreach (PresetZone* preset_zone, zones) {
            /* check if the note falls into the key and velocity range of this
               preset */
            if (fluid_preset_zone_inside_range(preset_zone, key, vel)) {

                  Inst* inst = fluid_preset_zone_get_inst(preset_zone);
                  global_inst_zone = inst->get_global_zone();

                  /* run thru all the zones of this instrument */
                  foreach(InstZone* inst_zone, inst->get_zone()) {
                        /* make sure this instrument zone has a valid sample */
                        sample = fluid_inst_zone_get_sample(inst_zone);
                        if (sample == 0 || sample->inRom()) {
                              inst_zone = fluid_inst_zone_next(inst_zone);
                              continue;
                              }
                        /* check if the note falls into the key and velocity range of this
                           instrument */
                        if (fluid_inst_zone_inside_range(inst_zone, key, vel) && (sample != 0)) {

                              /* this is a good zone. allocate a new synthesis process and
                                 initialize it */

                              voice = synth->alloc_voice(id, sample, chan, key, vel, nt);
                              if (voice == 0)
                                    return FLUID_FAILED;
                              z = inst_zone;

                              /* Instrument level, generators */

                              for (int i = 0; i < GEN_LAST; i++) {
                  	            /* SF 2.01 section 9.4 'bullet' 4:
                  	             *
                  	             * A generator in a local instrument zone supersedes a
                  	             * global instrument zone generator.  Both cases supersede
                  	             * the default generator -> voice_gen_set */

	                              if (inst_zone->gen[i].flags){
                                          voice->gen_set(i, inst_zone->gen[i].val);
                                          }
                                    else if ((global_inst_zone != 0) && (global_inst_zone->gen[i].flags)) {
                                          voice->gen_set(i, global_inst_zone->gen[i].val);
                                          }
                                    else {
                                          /* The generator has not been defined in this instrument.
                                           * Do nothing, leave it at the default.
                                           */
                                          }

                                    } /* for all generators */

                              /* global instrument zone, modulators: Put them all into a
                                 * list. */

                              mod_list_count = 0;

                              if (global_inst_zone){
                                    mod = global_inst_zone->mod;
                                    while (mod){
                                          mod_list[mod_list_count++] = mod;
                                          mod = mod->next;
                                          }
                                    }

                              /* local instrument zone, modulators.
                               * Replace modulators with the same definition in the list:
                               * SF 2.01 page 69, 'bullet' 8
                               */
                              mod = inst_zone->mod;
                              while (mod) {
	                              /* 'Identical' modulators will be deleted by setting their
	                               *  list entry to 0.  The list length is known, 0
	                               *  entries will be ignored later.  SF2.01 section 9.5.1
                                     *  page 69, 'bullet' 3 defines 'identical'.  */

                                    for (int i = 0; i < mod_list_count; i++){
                                          if (Modest_identity(mod,mod_list[i])){
                                                mod_list[i] = 0;
	                                          }
	                                    }

                                    /* Finally add the new modulator to to the list. */
                                    mod_list[mod_list_count++] = mod;
                                    mod = mod->next;
                                    }

                              /* Add instrument modulators (global / local) to the voice. */
                              for (int i = 0; i < mod_list_count; i++){
                                    mod = mod_list[i];
                                    if (mod) {  // disabled modulators CANNOT be skipped.
                                          /* Instrument modulators -supersede- existing (default)
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

                                          if (preset_zone->gen[i].flags) {
                                                voice->gen_incr(i, preset_zone->gen[i].val);
                                                }
                                          else if ((global_preset_zone != 0) && global_preset_zone->gen[i].flags) {
                                                voice->gen_incr(i, global_preset_zone->gen[i].val);
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
                              if (global_preset_zone){
                                    mod = global_preset_zone->mod;
                                    while (mod) {
                                          mod_list[mod_list_count++] = mod;
                                          mod = mod->next;
                                          }
                                    }

                              /* Process the modulators of the local preset zone.  Kick
                               * out all identical modulators from the global preset zone
                               * (SF 2.01 page 69, second-last bullet) */

                              mod = preset_zone->mod;
                              while (mod) {
                                    for (int i = 0; i < mod_list_count; i++) {
                                          if (Modest_identity(mod,mod_list[i])){
                                                mod_list[i] = 0;
                                                }
                                          }
                                    /* Finally add the new modulator to the list. */
                                    mod_list[mod_list_count++] = mod;
                                    mod = mod->next;
                                    }

                              /* Add preset modulators (global / local) to the voice. */
                              for (int i = 0; i < mod_list_count; i++){
                                    mod = mod_list[i];
                                    if ((mod != 0) && (mod->amount != 0)) { /* disabled modulators can be skipped. */
                                          /* Preset modulators -add- to existing instrument /
	                                     * default modulators.  SF2.01 page 70 first bullet on
	                                     * page */
                                          voice->add_mod(mod, FLUID_VOICE_ADD);
                                          }
                                    }

	                        /* add the synthesis process to the synthesis loop. */
	                        synth->start_voice(voice);

                              /* Store the ID of the first voice that was created by this noteon event.
                               * Exclusive class may only terminate older voices.
                               * That avoids killing voices, which have just been created.
                               * (a noteon event can create several voice processes with the same exclusive
                               * class - for example when using stereo samples)
                               */
                              }
                        }
                  }
            }

      return FLUID_OK;
      }

//---------------------------------------------------------
//   fluid_defpreset_import_sfont
//---------------------------------------------------------

int Preset::importSfont(SFPreset* sfpreset, SFont* sfont)
      {
      if (sfpreset->name && (strlen(sfpreset->name) > 0))
            name = sfpreset->name;
      else
            name = QString("Bank%1,Preset%2").arg(sfpreset->bank).arg(sfpreset->prenum);

      bank    = sfpreset->bank;
      num     = sfpreset->prenum;
      fluid_list_t* p = sfpreset->zone;
      int count       = 0;

      while (p != 0) {
            SFZone* sfzone = (SFZone *) p->data;
            QString zone_name = QString("%1/%2").arg(name).arg(count);
            PresetZone* zone = new PresetZone(QString("%1/%2").arg(name).arg(count));
            if (zone->importSfont(sfzone, sfont) != FLUID_OK)
                  return FLUID_FAILED;
            if ((count == 0) && (fluid_preset_zone_get_inst(zone) == 0))
                  setGlobalZone(zone);
            else
                  add_zone(zone);
            p = fluid_list_next(p);
            count++;
            }
      return FLUID_OK;
      }

//---------------------------------------------------------
//   PresetZone
//---------------------------------------------------------

PresetZone::PresetZone(const QString& s)
      {
      name  = s;
      inst  = 0;
      keylo = 0;
      keyhi = 128;
      vello = 0;
      velhi = 128;

      /* Flag all generators as unused (default, they will be set when they are found
       * in the sound font).
       * This also sets the generator values to default, but that is of no concern here.*/

      fluid_gen_set_default_values(&gen[0]);
      mod = 0;
      }

PresetZone::~PresetZone()
      {
      Mod* m = mod;
      while (m)	{     /* delete the modulators */
            Mod* tmp = m;
            m = m->next;
            fluid_mod_delete (tmp);
            }
      if (inst)
            delete inst;
      }

//---------------------------------------------------------
//   importSfont
//---------------------------------------------------------

int PresetZone::importSfont(SFZone *sfzone, SFont* sfont)
      {
      fluid_list_t* r;
      int count;
      for (count = 0, r = sfzone->gen; r != 0; count++) {
            SFGen* sfgen = (SFGen *) r->data;
            switch (sfgen->id) {
                  case GEN_KEYRANGE:
                        keylo = (int) sfgen->amount.range.lo;
                        keyhi = (int) sfgen->amount.range.hi;
                        break;
                  case GEN_VELRANGE:
                        vello = (int) sfgen->amount.range.lo;
                        velhi = (int) sfgen->amount.range.hi;
                        break;
                  default:
                        /* FIXME: some generators have an unsigned word amount value but i don't know which ones
                         */
                        gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
                        gen[sfgen->id].flags = GEN_SET;
                        break;
                  }
            r = fluid_list_next(r);
            }
      if (sfzone->inst) {
            inst = new Inst();
            if (!inst->import_sfont(sfzone->inst, sfont))
                  return FLUID_FAILED;
            }

      // Import the modulators (only SF2.1 and higher)
      for (count = 0, r = sfzone->mod; r != 0; count++) {
            SFMod* mod_src = (SFMod *)r->data;
            Mod * mod_dest = fluid_mod_new();
            int type;
            if (mod_dest == 0)
                  return FLUID_FAILED;
            mod_dest->next = 0; /* pointer to next modulator, this is the end of the list now.*/

            /* *** Amount *** */
            mod_dest->amount = mod_src->amount;

            /* *** Source *** */
            mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
            mod_dest->flags1 = 0;

            /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
            if (mod_src->src & (1<<7))
                  mod_dest->flags1 |= FLUID_MOD_CC;
            else
                  mod_dest->flags1 |= FLUID_MOD_GC;

            /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
            if (mod_src->src & (1<<8))
                  mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
            else
                  mod_dest->flags1 |= FLUID_MOD_POSITIVE;

            /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
            if (mod_src->src & (1<<9))
                  mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
            else
                  mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;

            /* modulator source types: SF2.01 section 8.2.1 page 52 */
            type = (mod_src->src) >> 10;
            type &= 63; /* type is a 6-bit value */
            if (type == 0)
                  mod_dest->flags1 |= FLUID_MOD_LINEAR;
            else if (type == 1)
                  mod_dest->flags1 |= FLUID_MOD_CONCAVE;
            else if (type == 2)
                  mod_dest->flags1 |= FLUID_MOD_CONVEX;
            else if (type == 3)
                  mod_dest->flags1 |= FLUID_MOD_SWITCH;
            else {
                  /* This shouldn't happen - unknown type!
                   * Deactivate the modulator by setting the amount to 0.
                   */
                  mod_dest->amount=0;
                  }

            /* *** Dest *** */
            mod_dest->dest = mod_src->dest; /* index of controlled generator */

            /* *** Amount source *** */
            mod_dest->src2 = mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01 section 8.2, p.50 */
            mod_dest->flags2 = 0;

            /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
            if (mod_src->amtsrc & (1<<7))
                  mod_dest->flags2 |= FLUID_MOD_CC;
            else
                  mod_dest->flags2 |= FLUID_MOD_GC;

            /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
            if (mod_src->amtsrc & (1<<8))
                  mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
            else
                  mod_dest->flags2 |= FLUID_MOD_POSITIVE;

            /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
            if (mod_src->amtsrc & (1<<9))
                  mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
            else
                  mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;

            /* modulator source types: SF2.01 section 8.2.1 page 52 */
            type = (mod_src->amtsrc) >> 10;
            type &= 63; /* type is a 6-bit value */
            if (type == 0)
                  mod_dest->flags2 |= FLUID_MOD_LINEAR;
            else if (type == 1)
                  mod_dest->flags2 |= FLUID_MOD_CONCAVE;
            else if (type == 2)
                  mod_dest->flags2 |= FLUID_MOD_CONVEX;
            else if (type == 3)
                  mod_dest->flags2 |= FLUID_MOD_SWITCH;
            else {
                  /* This shouldn't happen - unknown type!
                   * Deactivate the modulator by setting the amount to 0. */
                  mod_dest->amount=0;
                  }

            /* *** Transform *** */
            /* SF2.01 only uses the 'linear' transform (0).
             * Deactivate the modulator by setting the amount to 0 in any other case.
             */
            if (mod_src->trans !=0)
                  mod_dest->amount = 0;

            /* Store the new modulator in the zone The order of modulators
             * will make a difference, at least in an instrument context: The
             * second modulator overwrites the first one, if they only differ
             * in amount.
             */
            if (count == 0)
                  mod = mod_dest;
            else {
                  Mod * last_mod = mod;

                  /* Find the end of the list */
                  while (last_mod->next != 0)
                        last_mod=last_mod->next;
                  last_mod->next = mod_dest;
                  }

            r = fluid_list_next(r);
            } /* foreach modulator */
      return FLUID_OK;
      }

/*
 * fluid_preset_zone_get_inst
 */
Inst* fluid_preset_zone_get_inst(PresetZone* zone)
      {
      return zone->inst;
      }

/*
 * fluid_preset_zone_inside_range
 */
int fluid_preset_zone_inside_range(PresetZone* zone, int key, int vel)
      {
      return ((zone->keylo <= key) &&
	  (zone->keyhi >= key) &&
	  (zone->vello <= vel) &&
	  (zone->velhi >= vel));
      }

//---------------------------------------------------------
//   Inst
//---------------------------------------------------------

Inst::Inst()
      {
      name[0]     = 0;
      global_zone = 0;
      }

Inst::~Inst()
      {
      if (global_zone)
            delete_fluid_inst_zone(global_zone);
      foreach(InstZone* z, zone)
            delete z;
      }

/*
 * fluid_inst_import_sfont
 */
bool Inst::import_sfont(SFInst *sfinst, SFont* sfont)
      {
      strncpy(name, sfinst->name && sfinst->name[0] ? sfinst->name : "<untitled>", 21);

      int count = 0;
      fluid_list_t* p = sfinst->zone;
      while (p != 0) {
            SFZone* sfzone = (SFZone *) p->data;
            char zone_name[256];
            sprintf(zone_name, "%s/%d", name, count);
            InstZone* z = new_fluid_inst_zone(zone_name);
            if (fluid_inst_zone_import_sfont(z, sfzone, sfont) != FLUID_OK)
                  return false;
            if ((count == 0) && (fluid_inst_zone_get_sample(z) == 0))
                  set_global_zone(z);
            else
                  zone.append(z);
            p = fluid_list_next(p);
            count++;
            }
      return true;
      }

/***************************************************************
 *
 *                           INST_ZONE
 */

/*
 * new_fluid_inst_zone
 */
InstZone* new_fluid_inst_zone(const char* name)
      {
      InstZone* zone = FLUID_NEW(InstZone);
      zone->next     = 0;
      zone->name     = strdup(name);
      zone->sample   = 0;
      zone->keylo    = 0;
      zone->keyhi    = 128;
      zone->vello    = 0;
      zone->velhi    = 128;

      /* Flag the generators as unused.
       * This also sets the generator values to default, but they will be overwritten anyway, if used.
       */
      fluid_gen_set_default_values(&zone->gen[0]);
      zone->mod = 0; /* list of modulators */
      return zone;
      }

/*
 * delete_fluid_inst_zone
 */
int delete_fluid_inst_zone(InstZone* zone)
      {
      Mod *mod, *tmp;

      mod = zone->mod;
      while (mod)	{  /* delete the modulators */
            tmp = mod;
            mod = mod->next;
            fluid_mod_delete (tmp);
            }
      if (zone->name)
            free (zone->name);
      free(zone);
      return FLUID_OK;
      }

/*
 * fluid_inst_zone_next
 */
InstZone* fluid_inst_zone_next(InstZone* zone)
      {
      return zone->next;
      }

/*
 * fluid_inst_zone_import_sfont
 */
int fluid_inst_zone_import_sfont(InstZone* zone, SFZone *sfzone, SFont* sfont)
      {
      fluid_list_t *r;
      SFGen* sfgen;
      int count;

      for (count = 0, r = sfzone->gen; r != 0; count++) {
            sfgen = (SFGen *) r->data;
            switch (sfgen->id) {
                  case GEN_KEYRANGE:
                        zone->keylo = (int) sfgen->amount.range.lo;
                        zone->keyhi = (int) sfgen->amount.range.hi;
                        break;
                  case GEN_VELRANGE:
                        zone->vello = (int) sfgen->amount.range.lo;
                        zone->velhi = (int) sfgen->amount.range.hi;
                        break;
                  default:
                        /* FIXME: some generators have an unsigned word amount value but
                           i don't know which ones
                         */
                        zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
                        zone->gen[sfgen->id].flags = GEN_SET;
                        break;
                  }
            r = fluid_list_next(r);
            }

      /* FIXME */
      /*    if (zone->gen[GEN_EXCLUSIVECLASS].flags == GEN_SET) { */
      /*      FLUID_LOG(FLUID_DBG, "ExclusiveClass=%d\n", (int) zone->gen[GEN_EXCLUSIVECLASS].val); */
      /*    } */

      if (sfzone->samp) {
            zone->sample = sfont->get_sample(sfzone->samp->name);
            if (zone->sample == 0) {
                  FLUID_LOG(FLUID_ERR, "Couldn't find sample name");
                  return FLUID_FAILED;
                  }
            }

      /* Import the modulators (only SF2.1 and higher) */
      for (count = 0, r = sfzone->mod; r != 0; count++) {
            SFMod* mod_src = (SFMod *) r->data;
            int type;
            Mod* mod_dest;

            mod_dest = fluid_mod_new();
            if (mod_dest == 0)
                  return FLUID_FAILED;

            mod_dest->next = 0; /* pointer to next modulator, this is the end of the list now.*/

            /* *** Amount *** */
            mod_dest->amount = mod_src->amount;

            /* *** Source *** */
            mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
            mod_dest->flags1 = 0;

            /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
            if (mod_src->src & (1<<7))
                  mod_dest->flags1 |= FLUID_MOD_CC;
            else
                  mod_dest->flags1 |= FLUID_MOD_GC;

            /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
            if (mod_src->src & (1<<8))
                  mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
            else
                  mod_dest->flags1 |= FLUID_MOD_POSITIVE;

            /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
            if (mod_src->src & (1<<9))
                  mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
            else
                  mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;

            /* modulator source types: SF2.01 section 8.2.1 page 52 */
            type = (mod_src->src) >> 10;
            type &= 63; /* type is a 6-bit value */
            if (type == 0)
                  mod_dest->flags1 |= FLUID_MOD_LINEAR;
            else if (type == 1)
                  mod_dest->flags1 |= FLUID_MOD_CONCAVE;
            else if (type == 2)
                  mod_dest->flags1 |= FLUID_MOD_CONVEX;
            else if (type == 3)
                  mod_dest->flags1 |= FLUID_MOD_SWITCH;
            else {
                  /* This shouldn't happen - unknown type!
                   * Deactivate the modulator by setting the amount to 0. */
                  mod_dest->amount = 0;
                  }

            /* *** Dest *** */
            mod_dest->dest=mod_src->dest; /* index of controlled generator */

            /* *** Amount source *** */
            mod_dest->src2=mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01 section 8.2, page 50 */
            mod_dest->flags2 = 0;

            /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
            if (mod_src->amtsrc & (1<<7))
                  mod_dest->flags2 |= FLUID_MOD_CC;
            else
                  mod_dest->flags2 |= FLUID_MOD_GC;

            /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
            if (mod_src->amtsrc & (1<<8))
                  mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
            else
                  mod_dest->flags2 |= FLUID_MOD_POSITIVE;

            /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
            if (mod_src->amtsrc & (1<<9))
                  mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
            else
                  mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;

            /* modulator source types: SF2.01 section 8.2.1 page 52 */
            type=(mod_src->amtsrc) >> 10;
            type &= 63; /* type is a 6-bit value */
            if (type == 0)
                  mod_dest->flags2 |= FLUID_MOD_LINEAR;
            else if (type == 1)
                  mod_dest->flags2 |= FLUID_MOD_CONCAVE;
            else if (type == 2)
                  mod_dest->flags2 |= FLUID_MOD_CONVEX;
            else if (type == 3)
                  mod_dest->flags2 |= FLUID_MOD_SWITCH;
            else {
                  /* This shouldn't happen - unknown type!
                   * Deactivate the modulator by setting the amount to 0.
                   */
                  mod_dest->amount = 0;
                  }

            /* *** Transform *** */
            /* SF2.01 only uses the 'linear' transform (0).
             * Deactivate the modulator by setting the amount to 0 in any other case.
             */
            if (mod_src->trans !=0)
                  mod_dest->amount = 0;

            /* Store the new modulator in the zone
             * The order of modulators will make a difference, at least in an instrument context:
             * The second modulator overwrites the first one, if they only differ in amount.
             */
            if (count == 0)
                  zone->mod=mod_dest;
            else {
                  Mod * last_mod=zone->mod;
                  /* Find the end of the list */
                  while (last_mod->next != 0){
                        last_mod=last_mod->next;
                        }
                  last_mod->next=mod_dest;
                  }
            r = fluid_list_next(r);
            } /* foreach modulator */
      return FLUID_OK;
      }

/*
 * fluid_inst_zone_get_sample
 */
Sample* fluid_inst_zone_get_sample(InstZone* zone)
      {
      return zone->sample;
      }

/*
 * fluid_inst_zone_inside_range
 */
int fluid_inst_zone_inside_range(InstZone* zone, int key, int vel)
      {
      return ((zone->keylo <= key) &&
	  (zone->keyhi >= key) &&
	  (zone->vello <= vel) &&
	  (zone->velhi >= vel));
      }

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

Sample::Sample(SFont* s)
      {
      sf          = s;
      _valid      = false;
      name[0]     = 0;
      start       = 0;
      end         = 0;
      loopstart   = 0;
      loopend     = 0;
      samplerate  = 0;
      origpitch   = 0;
      pitchadj    = 0;
      sampletype  = 0;
      data        = 0;
      amplitude_that_reaches_noise_floor_is_valid = false;
      amplitude_that_reaches_noise_floor = 0.0;
      }

//---------------------------------------------------------
//   Sample
//---------------------------------------------------------

Sample::~Sample()
      {
      if (data)
            delete[] data;
      }

//---------------------------------------------------------
//   inRom
//---------------------------------------------------------

bool Sample::inRom() const
      {
      return sampletype & FLUID_SAMPLETYPE_ROM;
      }

/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/*
   functions for loading data from sfont files, with appropriate byte swapping
   on big endian machines. Sfont IDs are not swapped because the ID read is
   equivalent to the matching ID list in memory regardless of LE/BE machine
*/

/* removes and advances a fluid_list_t pointer */
inline void SLADVREM(fluid_list_t*& list, fluid_list_t*& item)
      {
      fluid_list_t *_temp = item;
      item = fluid_list_next(item);
      list = fluid_list_remove_link(list, _temp);
      delete1_fluid_list(_temp);
      }

char idlist[] = {
      "RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
      "ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdr"
      };

//---------------------------------------------------------
//   chunkid
//---------------------------------------------------------

static int chunkid (unsigned int id)
      {
      unsigned int* p = (unsigned int *) & idlist;
      for (unsigned i = 0; i < sizeof (idlist) / sizeof (int); i++, p++) {
            if (*p == id)
                  return (i + 1);
            }
      return UNKN_ID;
      }

//---------------------------------------------------------
//   SFData
//---------------------------------------------------------

SFData::SFData(const QString& s, SFont* sfont)
   : QFile(s)
      {
      sf    = sfont;
      info  = 0;
      }

//---------------------------------------------------------
//   load
//    return true on success
//---------------------------------------------------------

bool SFData::load()
      {
      SFChunk chunk;

      try {
            READCHUNK(&chunk);
            if (chunkid(chunk.id) != RIFF_ID)
                  throw(QString("Not a RIFF file"));

            safe_fread(&chunk.id, 4);
            if (chunkid (chunk.id) != SFBK_ID)        /* error if not SFBK_ID */
                  throw(QString("Not a sound font file"));
            if (chunk.size != size() - 8)
                  throw(QString("Sound font file size mismatch"));

            /* Process INFO block */
            read_listchunk(&chunk);
            if (chunkid(chunk.id) != INFO_ID)
                  throw(QString("Invalid ID found when expecting INFO chunk"));
            process_info(chunk.size);

            /* Process sample chunk */
            read_listchunk(&chunk);
            if (chunkid (chunk.id) != SDTA_ID)
                  throw(QString("Invalid ID found when expecting SAMPLE chunk"));
            process_sdta(chunk.size);

            /* process HYDRA chunk */
            read_listchunk(&chunk);
            if (chunkid (chunk.id) != PDTA_ID)
                  throw(QString("Invalid ID found when expecting HYDRA chunk"));
            process_pdta (chunk.size);

            fixup_pgen();
            fixup_igen();
            }
      catch (QString s) {
            printf("fluid: error loading sound font: %s\n", qPrintable(s));
            return false;
            }

      /* sort preset list by bank, preset # */
      qSort(preset.begin(), preset.end(), preset_compare);
//TODO      preset = fluid_list_sort(preset, (fluid_compare_func_t) sfont_preset_compare_func);
      return true;
      }

//---------------------------------------------------------
//   READCHUNK
//---------------------------------------------------------

void SFData::READCHUNK(SFChunk* var)
      {
	safe_fread(var, 8);
#ifdef WORDS_BIGENDIAN
      var->size = GUINT32_FROM_BE(var->size);
#else
      var->size = GUINT32_FROM_LE(var->size);
#endif
      }

//---------------------------------------------------------
//   READW
//---------------------------------------------------------

unsigned short SFData::READW()
      {
	unsigned short _temp;
	safe_fread(&_temp, 2);
#ifdef WORDS_BIGENDIAN
      return GINT16_FROM_BE(_temp);
#else
      return GINT16_FROM_LE(_temp);
#endif
      }

//---------------------------------------------------------
//   READD
//---------------------------------------------------------

void SFData::READD(unsigned int& var)
      {
      unsigned int _temp;
	safe_fread(&_temp, 4);
#ifdef WORDS_BIGENDIAN
	var = GINT32_FROM_BE(_temp);
#else
      var = GINT32_FROM_LE(_temp);
#endif
      }

//---------------------------------------------------------
//   READB
//---------------------------------------------------------

unsigned char SFData::READB()
      {
      unsigned char var;
      safe_fread(&var, 1);
      return var;
      }

//---------------------------------------------------------
//   FSKIPW
//---------------------------------------------------------

void SFData::FSKIPW()
      {
      safe_fseek(2, SEEK_CUR);
      }

void SFData::READSTR(char* name)
      {
      safe_fread(name, 20);
      name[20] = '\0';
      }

//---------------------------------------------------------
//   read_listchunk
//---------------------------------------------------------

void SFData::read_listchunk (SFChunk* chunk)
      {
      READCHUNK (chunk);
      if (chunkid (chunk->id) != LIST_ID)
            throw(QString("Invalid chunk id in level 0 parse"));
      safe_fread(&chunk->id, 4);
      chunk->size -= 4;
      }

//---------------------------------------------------------
//   process_info
//---------------------------------------------------------

void SFData::process_info(int size)
      {
      char *item;

      while (size > 0) {
            SFChunk chunk;
            READCHUNK (&chunk);
            size -= 8;

            unsigned char id = chunkid (chunk.id);

            if (id == IFIL_ID) {    // sound font version chunk?
                  if (chunk.size != 4)
                        throw(QString("Sound font version info chunk has invalid size"));

                  version.major = READW();
                  version.minor = READW();

                  if (version.major < 2 || version.major > 2)
                        throw(QString("Bad Sound font version"));
                  }
            else if (id == IVER_ID) {   /* ROM version chunk? */
                  if (chunk.size != 4)
                        throw(QString("ROM version info chunk has invalid size"));
                  romver.major = READW();
                  romver.minor = READW();
                  }
            else if (id != UNKN_ID) {
                  if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536) || (chunk.size % 2))
                        throw(QString("INFO sub chunk has invalid chunk size"));

                  /* alloc for chunk id and da chunk */
                  item = (char*)malloc(chunk.size + 1);

                  /* attach to INFO list, sfont_close will cleanup if FAIL occurs */
                  info = fluid_list_append (info, item);

                  *(unsigned char *) item = id;
                  safe_fread (&item[1], chunk.size);

                  /* force terminate info item (don't forget uint8 info ID) */
                  *(item + chunk.size) = '\0';
                  }
            else
                  throw(QString("Invalid chunk id in INFO chunk"));
            size -= chunk.size;
            }
      if (size < 0)
            throw(QString("INFO chunk size mismatch"));
      }

//---------------------------------------------------------
//   process_sdta
//    return true on success
//---------------------------------------------------------

void SFData::process_sdta (int size)
      {
      if (size == 0)
            return;		// no sample data?

      /* read sub chunk */
      SFChunk chunk;
      READCHUNK (&chunk);
      size -= 8;

      if (chunkid (chunk.id) != SMPL_ID)
            throw(QString("Expected SMPL chunk found invalid id instead"));

      if ((size - chunk.size) != 0)
            throw(QString("SDTA chunk size mismatch"));
      /* sample data follows */
      sf->setSamplepos(pos());
      sf->setSamplesize(chunk.size);
      FSKIP(chunk.size);
      }

//---------------------------------------------------------
//   pdtahelper
//---------------------------------------------------------

void SFData::pdtahelper (unsigned expid, unsigned reclen, SFChunk* chunk, int* size)
      {
      unsigned id;
      char* expstr = CHNKIDSTR (expid);	/* in case we need it */

      READCHUNK (chunk);
      *size -= 8;

      if ((id = chunkid (chunk->id)) != expid)
            throw(QString("Expected PDTA sub-chunk %1 found invalid id instead").arg(expstr));

      if (chunk->size % reclen)	/* valid chunk size? */
            throw(QString("chunk size is not a multiple of %1 bytes").arg(reclen));
      if ((*size -= chunk->size) < 0)
            throw(QString("%1 chunk size exceeds remaining PDTA chunk size").arg(expstr));
      }

//---------------------------------------------------------
//   process_pdta
//---------------------------------------------------------

void SFData::process_pdta (int size)
      {
      SFChunk chunk;
      pdtahelper (PHDR_ID, SFPHDRSIZE, &chunk, &size);
      load_phdr (chunk.size);
      pdtahelper (PBAG_ID, SFBAGSIZE, &chunk, &size);
      load_pbag (chunk.size);
      pdtahelper (PMOD_ID, SFMODSIZE, &chunk, &size);
      load_pmod (chunk.size);
      pdtahelper (PGEN_ID, SFGENSIZE, &chunk, &size);
      load_pgen (chunk.size);
      pdtahelper (IHDR_ID, SFIHDRSIZE, &chunk, &size);
      load_ihdr (chunk.size);
      pdtahelper (IBAG_ID, SFBAGSIZE, &chunk, &size);
      load_ibag (chunk.size);
      pdtahelper (IMOD_ID, SFMODSIZE, &chunk, &size);
      load_imod (chunk.size);
      pdtahelper (IGEN_ID, SFGENSIZE, &chunk, &size);
      load_igen (chunk.size);
      pdtahelper (SHDR_ID, SFSHDRSIZE, &chunk, &size);
      load_shdr (chunk.size);
      }

/* preset header loader */
void SFData::load_phdr (int size)
      {
      int i2;
      SFPreset *pr = 0;	/* ptr to current & previous preset */
      unsigned short zndx, pzndx = 0;

      if (size % SFPHDRSIZE || size == 0)
            throw(QString("Preset header chunk size is invalid"));

      int i = size / SFPHDRSIZE - 1;
      if (i == 0) {				/* at least one preset + term record */
            FLUID_LOG (FLUID_WARN, "File contains no presets");
            FSKIP (SFPHDRSIZE);
            return;
            }
      for (; i > 0; i--) {				/* load all preset headers */
            SFPreset* p = new SFPreset;
            preset.append(p);
            p->zone = 0;		      /* In case of failure, sfont_close can cleanup */
            READSTR (p->name);	/* possible read failure ^ */
            p->prenum = READW();
            p->bank   = READW();
            zndx      = READW();
            READD (p->libr);
            READD (p->genre);
            READD (p->morph);

            if (pr) {			/* not first preset? */
                  if (zndx < pzndx)
                        throw(QString("Preset header indices not monotonic"));
                  i2 = zndx - pzndx;
                  while (i2--)
                        pr->zone = fluid_list_prepend (pr->zone, 0);
                  }
            else if (zndx > 0)      /* 1st preset, warn if ofs >0 */
                  FLUID_LOG (FLUID_WARN, "%d preset zones not referenced, discarding", zndx);
            pr = p;			/* update preset ptr */
            pzndx = zndx;
            }
      FSKIP (24);
      zndx = READW();		/* Read terminal generator index */
      FSKIP (12);
      if (zndx < pzndx)
            throw(QString("Preset header indices not monotonic"));
      i2 = zndx - pzndx;
      while (i2--)
            pr->zone = fluid_list_prepend (pr->zone, 0);
      }

/* preset bag loader */
void SFData::load_pbag (int size)
      {
      fluid_list_t *p2;
      SFZone *z, *pz = 0;
      unsigned short genndx, modndx;
      unsigned short pgenndx = 0, pmodndx = 0;
      unsigned short i;

      if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
            throw(QString("Preset bag chunk size is invalid"));

      foreach(SFPreset* p, preset) {
            p2 = p->zone;
            while (p2) {			/* traverse preset's zones */
	            if ((size -= SFBAGSIZE) < 0)
	                  throw(QString("Preset bag chunk size mismatch"));
	            z = new SFZone;
	            p2->data = z;
	            z->gen = 0;	/* Init gen and mod before possible failure, */
	            z->mod = 0;	/* to ensure proper cleanup (sfont_close) */
	            genndx = READW();	/* possible read failure ^ */
	            modndx = READW();
	            z->samp = 0;

                  if (pz) {			/* if not first zone */
                        if (genndx < pgenndx)
                              throw(QString("Preset bag generator indices not monotonic"));
                        if (modndx < pmodndx)
                              throw(QString("Preset bag modulator indices not monotonic"));
                        i = genndx - pgenndx;
                        while (i--)
                              pz->gen = fluid_list_prepend (pz->gen, 0);
                        i = modndx - pmodndx;
                        while (i--)
                              pz->mod = fluid_list_prepend (pz->mod, 0);
                        }
                  pz = z;		/* update previous zone ptr */
                  pgenndx = genndx;	/* update previous zone gen index */
                  pmodndx = modndx;	/* update previous zone mod index */
                  p2 = fluid_list_next (p2);
                  }
            }
      size -= SFBAGSIZE;
      if (size != 0)
            throw(QString("Preset bag chunk size mismatch"));

      genndx = READW();
      modndx = READW();

      if (!pz) {
            if (genndx > 0)
                  FLUID_LOG (FLUID_WARN, "No preset generators and terminal index not 0");
            if (modndx > 0)
                  FLUID_LOG (FLUID_WARN, "No preset modulators and terminal index not 0");
            return;
            }
      if (genndx < pgenndx)
            throw(QString("Preset bag generator indices not monotonic"));
      if (modndx < pmodndx)
            throw(QString("Preset bag modulator indices not monotonic"));
      i = genndx - pgenndx;
      while (i--)
            pz->gen = fluid_list_prepend (pz->gen, 0);
      i = modndx - pmodndx;
      while (i--)
            pz->mod = fluid_list_prepend (pz->mod, 0);
      }

//---------------------------------------------------------
//   load_pmod
//    preset modulator loader
//---------------------------------------------------------

void SFData::load_pmod (int size)
      {
      fluid_list_t *p2, *p3;
      SFMod *m;

      foreach(SFPreset* p, preset) {
            p2 = p->zone;
            while (p2) {			/* traverse this preset's zones */
	            p3 = ((SFZone *) (p2->data))->mod;
	            while (p3) {			/* load zone's modulators */
	                  if ((size -= SFMODSIZE) < 0)
		                  throw(QString("Preset modulator chunk size mismatch"));
	                  m = FLUID_NEW (SFMod);
	                  p3->data  = m;
	                  m->src    = READW();
	                  m->dest   = READW();
	                  m->amount = READW();
	                  m->amtsrc = READW();
	                  m->trans  = READW();
	                  p3 = fluid_list_next (p3);
	                  }
                  p2 = fluid_list_next (p2);
                  }
            }

  /*
     If there isn't even a terminal record
     Hmmm, the specs say there should be one, but..
   */
      if (size == 0)
            return;

      size -= SFMODSIZE;
      if (size != 0)
            throw(QString("Preset modulator chunk size mismatch"));
      FSKIP (SFMODSIZE);	/* terminal mod */
      }

/* -------------------------------------------------------------------
 * preset generator loader
 * generator (per preset) loading rules:
 * Zones with no generators or modulators shall be annihilated
 * Global zone must be 1st zone, discard additional ones (instrumentless zones)
 *
 * generator (per zone) loading rules (in order of decreasing precedence):
 * KeyRange is 1st in list (if exists), else discard
 * if a VelRange exists only preceded by a KeyRange, else discard
 * if a generator follows an instrument discard it
 * if a duplicate generator exists replace previous one
 * ------------------------------------------------------------------- */

void SFData::load_pgen (int size)
      {
      fluid_list_t *p2, *p3, *dup, **hz = 0;
      SFZone *z;
      SFGen *g;
      SFGenAmount genval;
      unsigned short genid;
      int level, skip, drop, gzone, discarded;

      foreach(SFPreset* p, preset) {
            gzone = FALSE;
            discarded = FALSE;
            p2 = p->zone;
            if (p2)
	            hz = &p2;
            while (p2) {			/* traverse preset's zones */
	            level = 0;
	            z = (SFZone *) (p2->data);
	            p3 = z->gen;
	            while (p3) {      /* load zone's generators */
	                  dup = 0;
	                  skip = FALSE;
	                  drop = FALSE;
	                  if ((size -= SFGENSIZE) < 0)
		                  throw(QString("Preset generator chunk size mismatch"));

	                  genid = READW();
	                  if (genid == Gen_KeyRange) {  /* nothing precedes */
		                  if (level == 0) {
		                        level = 1;
		                        genval.range.lo = READB();
		                        genval.range.hi = READB();
		                        }
		                  else
		                        skip = TRUE;
		                  }
	                  else if (genid == Gen_VelRange) {   // only KeyRange precedes
		                  if (level <= 1) {
		                        level = 2;
		                        genval.range.lo = READB();
		                        genval.range.hi = READB();
		                        }
		                  else
		                        skip = TRUE;
		                  }
	                  else if (genid == Gen_Instrument) { /* inst is last gen */
		                  level = 3;
		                  genval.uword = READW();
		                  SFZone* z   = (SFZone*)(p2->data);
		                  z->instIdx  = genval.uword + 1;

		                  break;	/* break out of generator loop */
		                  }
	                  else {
		                  level = 2;
		                  if (gen_validp (genid)) {     /* generator valid? */
		                        genval.sword = READW();
		                        dup = gen_inlist (genid, z->gen);
		                        }
		                  else
		                        skip = TRUE;
		                  }
	                  if (!skip) {
		                  if (!dup) {		/* if gen ! dup alloc new */
		                        g = FLUID_NEW (SFGen);
		                        p3->data = g;
		                        g->id = genid;
		                        }
		                  else {
		                        g = (SFGen *) (dup->data);	/* ptr to orig gen */
		                        drop = TRUE;
		                        }
		                  g->amount = genval;
		                  }
	                  else {		/* Skip this generator */
		                  discarded = TRUE;
		                  drop = TRUE;
		                  FSKIPW ();
		                  }
	                  if (!drop)
		                  p3 = fluid_list_next (p3);	/* next gen */
	                  else
		                  SLADVREM (z->gen, p3);	/* drop place holder */
	                  }  /* generator loop */

                  if (level == 3)
                        SLADVREM (z->gen, p3);	/* zone has inst? */
	            else {			/* congratulations its a global zone */
	                  if (!gzone) {		/* Prior global zones? */
		                  gzone = TRUE;

		                  /* if global zone is not 1st zone, relocate */
		                  if (*hz != p2) {
		                        void* save = p2->data;
		                        FLUID_LOG (FLUID_WARN, "Preset \"%s\": Global zone is not first zone", p->name);
		                        SLADVREM (*hz, p2);
		                        *hz = fluid_list_prepend (*hz, save);
		                        continue;
		                        }
		                  }
	                  else {      // previous global zone exists, discard
		                  FLUID_LOG (FLUID_WARN, "Preset \"%s\": Discarding invalid global zone", p->name);
		                  sfont_zone_delete (this, hz, (SFZone *) (p2->data));
		                  }
	                  }
	            while (p3) {      /* Kill any zones following an instrument */
	                  discarded = TRUE;
	                  if ((size -= SFGENSIZE) < 0)
		                  throw(QString("Preset generator chunk size mismatch"));
	                  FSKIP (SFGENSIZE);
	                  SLADVREM (z->gen, p3);
	                  }
	            p2 = fluid_list_next (p2);	/* next zone */
	            }
            if (discarded)
	            FLUID_LOG(FLUID_WARN, "Preset \"%s\": Some invalid generators were discarded", p->name);
            }
      /* in case there isn't a terminal record */
      if (size == 0)
            return;

      size -= SFGENSIZE;
      if (size != 0)
            throw(QString("Preset generator chunk size mismatch"));
      FSKIP (SFGENSIZE);	/* terminal gen */
      }

/* instrument header loader */
void SFData::load_ihdr(int size)
      {
      SFData * sf = this;
      int i, i2;
      SFInst *p, *pr = 0;	/* ptr to current & previous instrument */
      unsigned short zndx, pzndx = 0;

      if (size % SFIHDRSIZE || size == 0)	/* chunk size is valid? */
            throw(QString("Instrument header has invalid size"));

      size = size / SFIHDRSIZE - 1;
      if (size == 0) {  /* at least one preset + term record */
            FLUID_LOG (FLUID_WARN, "File contains no instruments");
            FSKIP (SFIHDRSIZE);
            return;
            }
      for (i = 0; i < size; i++) {  /* load all instrument headers */
            p = new SFInst;
            sf->inst.append(p);
            p->zone = 0;		/* For proper cleanup if fail (sfont_close) */
            READSTR (p->name);	/* Possible read failure ^ */
            zndx = READW();

            if (pr) {   /* not first instrument? */
	            if (zndx < pzndx)
                        throw(QString("Instrument header indices not monotonic"));
	            i2 = zndx - pzndx;
	            while (i2--)
                        pr->zone = fluid_list_prepend (pr->zone, 0);
                  }
            else if (zndx > 0)	/* 1st inst, warn if ofs >0 */
	            FLUID_LOG (FLUID_WARN, "%d instrument zones not referenced, discarding", zndx);
            pzndx = zndx;
            pr = p;			/* update instrument ptr */
            }

      FSKIP (20);
      zndx = READW();

      if (zndx < pzndx)
            throw(QString("Instrument header indices not monotonic"));
      i2 = zndx - pzndx;
      while (i2--)
            pr->zone = fluid_list_prepend (pr->zone, 0);
      }

/* instrument bag loader */
void SFData::load_ibag(int size)
      {
      SFData * sf = this;

      fluid_list_t *p2;
      SFZone *z, *pz = 0;
      unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
      int i;

      if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
            throw(QString("Instrument bag chunk size is invalid"));

      foreach(SFInst* inst, sf->inst) {
            p2 = inst->zone;
            while (p2) {      /* load this inst's zones */
	            if ((size -= SFBAGSIZE) < 0)
	                  throw(QString("Instrument bag chunk size mismatch"));
	            z = new SFZone;
	            p2->data = z;
	            z->gen = 0;	/* In case of failure, */
	            z->mod = 0;	/* sfont_close can clean up */
	            genndx = READW();
	            modndx = READW();
	            z->samp = 0;

	            if (pz) {			/* if not first zone */
	                  if (genndx < pgenndx)
		                  throw(QString("Instrument generator indices not monotonic"));
	                  if (modndx < pmodndx)
		                  throw(QString("Instrument modulator indices not monotonic"));
	                  i = genndx - pgenndx;
	                  while (i--)
		                  pz->gen = fluid_list_prepend (pz->gen, 0);
	                  i = modndx - pmodndx;
	                  while (i--)
		                  pz->mod = fluid_list_prepend (pz->mod, 0);
	                  }
	            pz = z;		/* update previous zone ptr */
	            pgenndx = genndx;
	            pmodndx = modndx;
	            p2 = fluid_list_next (p2);
	            }
            }
      size -= SFBAGSIZE;
      if (size != 0)
            throw(QString("Instrument chunk size mismatch"));

      genndx = READW();
      modndx = READW();

      if (!pz) {  /* in case that all are no zoners */
            if (genndx > 0)
	            FLUID_LOG (FLUID_WARN, "No instrument generators and terminal index not 0");
            if (modndx > 0)
                  FLUID_LOG (FLUID_WARN, "No instrument modulators and terminal index not 0");
            return;
            }
      if (genndx < pgenndx)
            throw(QString("Instrument generator indices not monotonic"));
      if (modndx < pmodndx)
            throw(QString("Instrument modulator indices not monotonic"));
      i = genndx - pgenndx;
      while (i--)
            pz->gen = fluid_list_prepend (pz->gen, 0);
      i = modndx - pmodndx;
      while (i--)
            pz->mod = fluid_list_prepend (pz->mod, 0);
      }

/* instrument modulator loader */
void SFData::load_imod(int size)
      {
      foreach(SFInst* i, inst) {
            fluid_list_t* p2 = i->zone;
            while (p2) {			/* traverse this inst's zones */
                  fluid_list_t* p3 = ((SFZone *) (p2->data))->mod;
                  while (p3) {			/* load zone's modulators */
                        if ((size -= SFMODSIZE) < 0)
                              throw(QString("Instrument modulator chunk size mismatch"));
                        SFMod* m  = FLUID_NEW (SFMod);
	                  p3->data  = m;
	                  m->src    = READW();
	                  m->dest   = READW();
	                  m->amount = READW();
	                  m->amtsrc = READW();
	                  m->trans  = READW();
	                  p3        = fluid_list_next (p3);
	                  }
	            p2 = fluid_list_next (p2);
	            }
            }
      /*
        If there isn't even a terminal record
        Hmmm, the specs say there should be one, but..
      */
      if (size == 0)
            return;
      size -= SFMODSIZE;
      if (size != 0)
            throw(QString("Instrument modulator chunk size mismatch"));
      FSKIP (SFMODSIZE);	/* terminal mod */
      }

/* load instrument generators (see load_pgen for loading rules) */
void SFData::load_igen (int size)
      {
      SFData * sf = this;
      fluid_list_t *p2, *p3, *dup, **hz = 0;
      SFZone *z;
      SFGen *g;
      SFGenAmount genval;
      unsigned short genid;
      int level, skip, drop, gzone, discarded;

      foreach(SFInst* inst, sf->inst) {
            gzone = FALSE;
            discarded = FALSE;
            p2 = inst->zone;
            if (p2)
                  hz = &p2;
            while (p2) {      /* traverse this instrument's zones */
                  level = 0;
                  z = (SFZone *) (p2->data);
                  p3 = z->gen;
                  while (p3) {      /* load zone's generators */
                        dup = 0;
                        skip = FALSE;
                        drop = FALSE;
                        if ((size -= SFGENSIZE) < 0)
                              throw(QString("IGEN chunk size mismatch"));

                        genid = READW();

                        if (genid == Gen_KeyRange) {  /* nothing precedes */
                              if (level == 0) {
                                    level = 1;
		                        genval.range.lo = READB();
		                        genval.range.hi = READB();
		                        }
		                  else
		                        skip = TRUE;
		                  }
	                  else if (genid == Gen_VelRange) {   // only KeyRange precedes
		                  if (level <= 1) {
		                        level = 2;
		                        genval.range.lo = READB();
		                        genval.range.hi = READB();
		                        }
		                  else
		                        skip = TRUE;
		                  }
	                  else if (genid == Gen_SampleId) {   // sample is last gen
		                  level = 3;
		                  genval.uword = READW();
		                  SFZone* z   = (SFZone*)(p2->data);
		                  z->sampIdx  = genval.uword + 1;
		                  break;	/* break out of generator loop */
		                  }
	                  else {
		                  level = 2;
		                  if (gen_valid (genid)) {      // gen valid?
		                        genval.sword = READW();
		                        dup = gen_inlist (genid, z->gen);
		                        }
		                  else
		                        skip = TRUE;
		                  }
                        if (!skip) {
		                  if (!dup) {   /* if gen ! dup alloc new */
		                        g = FLUID_NEW (SFGen);
		                        p3->data = g;
		                        g->id = genid;
		                        }
		                  else {
		                        g = (SFGen *) (dup->data);
		                        drop = TRUE;
		                        }
		                  g->amount = genval;
		                  }
	                  else {		/* skip this generator */
		                  discarded = TRUE;
		                  drop = TRUE;
		                  FSKIPW ();
		                  }
	                  if (!drop)
		                  p3 = fluid_list_next (p3);	/* next gen */
	                  else
		                  SLADVREM (z->gen, p3);
	                  }   /* generator loop */
                  if (level == 3)
	                  SLADVREM (z->gen, p3);	/* zone has sample? */
	            else {      /* its a global zone */
	                  if (!gzone) {
		                  gzone = TRUE;

		                  /* if global zone is not 1st zone, relocate */
      		            if (*hz != p2) {
	      	                  void* save = p2->data;
		                        FLUID_LOG (FLUID_WARN,
			                     "Instrument \"%s\": Global zone is not first zone",
			                     inst->name);
      		                  SLADVREM (*hz, p2);
	      	                  *hz = fluid_list_prepend (*hz, save);
		                        continue;
		                        }
      		            }
	                  else {      /* previous global zone exists, discard */
		                  FLUID_LOG (FLUID_WARN,
      		               "Instrument \"%s\": Discarding invalid global zone",
	      	               inst->name);
		                  sfont_zone_delete (sf, hz, (SFZone *) (p2->data));
		                  }
      	            }

	            while (p3) {      /* Kill any zones following a sample */
	                  discarded = TRUE;
      	            if ((size -= SFGENSIZE) < 0)
	      	            throw(QString("Instrument generator chunk size mismatch"));
      	            FSKIP (SFGENSIZE);
	                  SLADVREM (z->gen, p3);
	                  }
                  p2 = fluid_list_next (p2);	/* next zone */
                  }
            if (discarded)
                  FLUID_LOG(FLUID_WARN, "Instrument \"%s\": Some invalid generators were discarded",
      	         inst->name);
            }
      /* for those non-terminal record cases, grr! */
      if (size == 0)
            return;

      size -= SFGENSIZE;
      if (size != 0)
            throw(QString("IGEN chunk size mismatch"));
      FSKIP (SFGENSIZE);	/* terminal gen */
      }

//---------------------------------------------------------
//   load_shdr
//    sample header loader
//---------------------------------------------------------

void SFData::load_shdr (unsigned int size)
      {
      if (size % SFSHDRSIZE || size == 0)	/* size is multiple of SHDR size? */
            throw(QString("Sample header has invalid size"));

      size = size / SFSHDRSIZE - 1;
      if (size == 0) {  /* at least one sample + term record? */
            FLUID_LOG (FLUID_WARN, "File contains no samples");
            FSKIP (SFSHDRSIZE);
            return;
            }

      /* load all sample headers */
      for (unsigned int i = 0; i < size; i++) {
            Sample* p = new Sample(sf);
            sample.append(p);
            READSTR (p->name);
            READD (p->start);
            READD (p->end);	      /* - end, loopstart and loopend */
            READD (p->loopstart);	/* - will be checked and turned into */
            READD (p->loopend);
            READD (p->samplerate);
            p->origpitch = READB();
            p->pitchadj  = READB();
            FSKIPW ();		      // skip sample link
            p->sampletype = READW();
            if (p->sampletype & FLUID_SAMPLETYPE_ROM) {
                  p->setValid(false);
                  continue;
                  }
            if ((p->end > sf->getSamplesize()) || (p->start > (p->end - 4))) {
                  FLUID_LOG (FLUID_WARN, "Sample '%s' start/end file positions are invalid, disabling", p->name);
                  p->setValid(false);
                  continue;
                  }
            p->setValid(true);
            if (p->loopend > p->end || p->loopstart >= p->loopend || p->loopstart <= p->start) {	 /* loop is fowled?? (cluck cluck :) */
                  /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4) */
                  if ((p->end - p->start) >= 20) {
                        p->loopstart = p->start + 8;
                        p->loopend   = p->end - 8;
                        }
                  else {      // loop is fowled, sample is tiny (can't pad 8 samples)
                        p->loopstart = p->start + 1;
                        p->loopend   = p->end - 1;
                        }
                  }
            if (p->end - p->start < 8)
                  p->setValid(false);
            }
      FSKIP (SFSHDRSIZE);	/* skip terminal shdr */
      }

//---------------------------------------------------------
//   fixup_pgen
//    "fixup" (inst # -> inst ptr) instrument references
//    in preset list
//---------------------------------------------------------

void SFData::fixup_pgen()
      {
      fluid_list_t *p2;
      SFZone *z;

      foreach(SFPreset* p, preset) {
            p2 = p->zone;
            while (p2) {			/* traverse this preset's zones */
                  z = (SFZone *) (p2->data);
                  int i = z->instIdx;        // load instrument #
                  if (i) {
                        SFInst* p3 = inst[i-1];
                        if (!p3)
                              throw(QString("Preset %1 %2: Invalid instrument reference").arg(p->bank).arg(p->prenum));
                        z->inst = p3;  // TEST1
                        }
                  else
                        z->inst = 0;
                  p2 = fluid_list_next (p2);
                  }
            }
      }

/* "fixup" (sample # -> sample ptr) sample references in instrument list */
void SFData::fixup_igen()
      {
      foreach(SFInst* p, inst) {
            fluid_list_t* p2 = p->zone;
            while (p2) {			/* traverse instrument's zones */
                  SFZone* z = (SFZone *) (p2->data);
                  int i = z->sampIdx;
                  if (i) {
                        Sample* p3 = sample[i - 1];
                        if (!p3)
                              throw(QString("Instrument <%1>: Invalid sample reference").arg(p->name));
                        z->samp = p3;
                        }
                  p2 = fluid_list_next (p2);
                  }
            }
      }

/* optimum chunk area sizes (could be more optimum) */
#define PRESET_CHUNK_OPTIMUM_AREA   256
#define INST_CHUNK_OPTIMUM_AREA	256
#define SAMPLE_CHUNK_OPTIMUM_AREA	256
#define ZONE_CHUNK_OPTIMUM_AREA	256
#define MOD_CHUNK_OPTIMUM_AREA	256
#define GEN_CHUNK_OPTIMUM_AREA	256

unsigned short badgen[] = {
      Gen_Unused1, Gen_Unused2, Gen_Unused3, Gen_Unused4,
      Gen_Reserved1, Gen_Reserved2, Gen_Reserved3, 0
      };

unsigned short badpgen[] = {
      Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
      Gen_EndLoopAddrOfs, Gen_StartAddrCoarseOfs, Gen_EndAddrCoarseOfs,
      Gen_StartLoopAddrCoarseOfs, Gen_Keynum, Gen_Velocity,
      Gen_EndLoopAddrCoarseOfs, Gen_SampleModes, Gen_ExclusiveClass,
      Gen_OverrideRootKey, 0
      };

//---------------------------------------------------------
//   close
//    close SoundFont file and delete a SoundFont structure
//---------------------------------------------------------

SFData::~SFData()
      {
      fluid_list_t* p = info;
      while (p) {
            free(p->data);
            p = fluid_list_next(p);
            }
      delete_fluid_list(info);

      foreach(SFPreset* p, preset) {
            fluid_list_t* p2 = p->zone;
            while (p2) { // loop over preset's zones
	            delete ((SFZone*)p2->data);
	            p2 = fluid_list_next (p2);
                  }			/* free preset's zone list */
            delete p->zone;
            delete p;
            }

      foreach(SFInst* i, inst) {
            fluid_list_t* p2 = i->zone;
            while (p2) {			/* loop over inst's zones */
                  delete ((SFZone*)p2->data);
                  p2 = fluid_list_next (p2);
                  }			/* free inst's zone list */
            delete i->zone;
            delete i;
            }
      foreach(Sample* s, sample)
            delete s;
      }

/* free all elements of a zone (Preset or Instrument) */
SFZone::~SFZone()
      {
      fluid_list_t* p = gen;
      while (p) {				/* Free gen chunks for this zone */
            if (p->data)
                  free (p->data);
            p = fluid_list_next (p);
            }
      delete_fluid_list (gen);	/* free genlist */

      p = mod;
      while (p) {				/* Free mod chunks for this zone */
            if (p->data)
                  free (p->data);
            p = fluid_list_next (p);
            }
      delete_fluid_list (mod);	/* free modlist */
      }

//---------------------------------------------------------
//   preset_compare
//---------------------------------------------------------

bool preset_compare (SFPreset* a, SFPreset* b)
      {
      int aval = (a->bank) << 16 | a->prenum;
      int bval = (b->bank) << 16 | b->prenum;
      return aval < bval;
      }

/* delete zone from zone list */
void sfont_zone_delete (SFData * /*sf*/, fluid_list_t ** zlist, SFZone * zone)
      {
      *zlist = fluid_list_remove (*zlist, (void*) zone);
      delete zone;
      }

/* Find generator in gen list */
fluid_list_t * gen_inlist (int gen, fluid_list_t * genlist)
      {				/* is generator in gen list? */
      fluid_list_t *p;

      p = genlist;
      while (p) {
            if (p->data == 0)
                  return 0;
            if (gen == ((SFGen *) p->data)->id)
	            break;
            p = fluid_list_next (p);
            }
      return (p);
      }

/* check validity of instrument generator */
int gen_valid (int gen)
      {				/* is generator id valid? */
      int i = 0;

      if (gen > Gen_MaxValid)
            return (FALSE);
      while (badgen[i] && badgen[i] != gen)
            i++;
      return (badgen[i] == 0);
      }

/* check validity of preset generator */
int gen_validp (int gen)
      {				/* is preset generator valid? */
      int i = 0;

      if (!gen_valid (gen))
            return (FALSE);
      while (badpgen[i] && badpgen[i] != (unsigned short) gen)
            i++;
      return (badpgen[i] == 0);
      }

/* Logging function, returns FAIL to use as a return value in calling funcs */
int gerr (int /*ev*/, const char * fmt, ...)
      {
      va_list args;

      va_start (args, fmt);
      vprintf(fmt, args);
      va_end (args);

      printf("\n");
      return FAIL;
      }

//---------------------------------------------------------
//   safe_fread
//---------------------------------------------------------

void SFData::safe_fread(void *buf, int count)
      {
      if (read((char*)buf, count) != count) {
            if (atEnd())
                  throw(QString("EOF while attemping to read %1 bytes").arg(count));
            else
                  throw(QString("File read failed"));
            }
      }

//---------------------------------------------------------
//   safe_fseek
//---------------------------------------------------------

void SFData::safe_fseek(long ofs, int whence)
      {
      qint64 newpos = ofs;
      if (whence == SEEK_CUR)
            newpos += pos();
      else if (whence == SEEK_SET)
            ;
      else
            abort();
      if (seek(newpos) == -1)
            throw(QString("File seek failed with offset = %1 and whence = %2").arg(ofs).arg(whence));
      }
}
