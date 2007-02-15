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


#include "defsfont.h"
/* Todo: Get rid of that 'include' */
// #include "sys.h"

/***************************************************************
 *
 *                           SFONT LOADER
 */

fluid_sfloader_t* new_fluid_defsfloader()
{
  fluid_sfloader_t* loader;

  loader = FLUID_NEW(fluid_sfloader_t);
  if (loader == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  loader->data = 0;
  loader->free = delete_fluid_defsfloader;
  loader->load = fluid_defsfloader_load;

  return loader;
}

int delete_fluid_defsfloader(fluid_sfloader_t* loader)
{
  if (loader) {
    FLUID_FREE(loader);
  }
  return FLUID_OK;
}

fluid_sfont_t* fluid_defsfloader_load(fluid_sfloader_t* loader, const QString& filename)
{
  fluid_defsfont_t* defsfont;
  fluid_sfont_t* sfont;

  defsfont = new_fluid_defsfont();

  if (defsfont == 0) {
    return 0;
  }

  if (fluid_defsfont_load(defsfont, filename) == FLUID_FAILED) {
    delete_fluid_defsfont(defsfont);
    return 0;
  }

  sfont = FLUID_NEW(fluid_sfont_t);
  if (sfont == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  sfont->data = defsfont;
  sfont->free = fluid_defsfont_sfont_delete;
  sfont->get_name = fluid_defsfont_sfont_get_name;
  sfont->get_preset = fluid_defsfont_sfont_get_preset;
  sfont->iteration_start = fluid_defsfont_sfont_iteration_start;
  sfont->iteration_next = fluid_defsfont_sfont_iteration_next;

  return sfont;
}



/***************************************************************
 *
 *                           PUBLIC INTERFACE
 */

int fluid_defsfont_sfont_delete(fluid_sfont_t* sfont)
{
  if (delete_fluid_defsfont((fluid_defsfont_t*)sfont->data) != 0) {
    return -1;
  }
  FLUID_FREE(sfont);
  return 0;
}

char* fluid_defsfont_sfont_get_name(fluid_sfont_t* sfont)
{
  return fluid_defsfont_get_name((fluid_defsfont_t*) sfont->data);
}

fluid_preset_t* fluid_defsfont_sfont_get_preset(fluid_sfont_t* sfont, unsigned int bank, unsigned int prenum)
{
  fluid_preset_t* preset;
  fluid_defpreset_t* defpreset;

  defpreset = fluid_defsfont_get_preset((fluid_defsfont_t*) sfont->data, bank, prenum);

  if (defpreset == 0) {
    return 0;
  }

  preset = FLUID_NEW(fluid_preset_t);
  if (preset == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  preset->sfont = sfont;
  preset->data = defpreset;
  preset->free = fluid_defpreset_preset_delete;
  preset->get_name = fluid_defpreset_preset_get_name;
  preset->get_banknum = fluid_defpreset_preset_get_banknum;
  preset->get_num = fluid_defpreset_preset_get_num;
  preset->noteon = fluid_defpreset_preset_noteon;
  preset->notify = 0;

  return preset;
}

void fluid_defsfont_sfont_iteration_start(fluid_sfont_t* sfont)
{
  fluid_defsfont_iteration_start((fluid_defsfont_t*) sfont->data);
}

int fluid_defsfont_sfont_iteration_next(fluid_sfont_t* sfont, fluid_preset_t* preset)
{
  preset->free = fluid_defpreset_preset_delete;
  preset->get_name = fluid_defpreset_preset_get_name;
  preset->get_banknum = fluid_defpreset_preset_get_banknum;
  preset->get_num = fluid_defpreset_preset_get_num;
  preset->noteon = fluid_defpreset_preset_noteon;
  preset->notify = 0;

  return fluid_defsfont_iteration_next((fluid_defsfont_t*) sfont->data, preset);
}

int fluid_defpreset_preset_delete(fluid_preset_t* preset)
{
  FLUID_FREE(preset);
/*   printf("TODO: free modulators\n");  */
  return 0;
}

char* fluid_defpreset_preset_get_name(fluid_preset_t* preset)
{
  return fluid_defpreset_get_name((fluid_defpreset_t*) preset->data);
}

int fluid_defpreset_preset_get_banknum(fluid_preset_t* preset)
{
  return fluid_defpreset_get_banknum((fluid_defpreset_t*) preset->data);
}

int fluid_defpreset_preset_get_num(fluid_preset_t* preset)
{
  return fluid_defpreset_get_num((fluid_defpreset_t*) preset->data);
}

int fluid_defpreset_preset_noteon(fluid_preset_t* preset, fluid_synth_t* synth,
				 int chan, int key, int vel)
{
  return fluid_defpreset_noteon((fluid_defpreset_t*) preset->data, synth, chan, key, vel);
}




/***************************************************************
 *
 *                           SFONT
 */

/*
 * new_fluid_defsfont
 */
fluid_defsfont_t* new_fluid_defsfont()
      {
      fluid_defsfont_t* sfont = FLUID_NEW(fluid_defsfont_t);

      if (sfont == 0) {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            return 0;
            }

      sfont->filename = 0;
      sfont->samplepos = 0;
      sfont->samplesize = 0;
      sfont->sample = 0;
      sfont->sampledata = 0;
      sfont->preset = 0;

      return sfont;
      }

/*
 * delete_fluid_defsfont
 */
int delete_fluid_defsfont(fluid_defsfont_t* sfont)
{
  fluid_list_t *list;
  fluid_defpreset_t* preset;
  fluid_sample_t* sample;

  /* Check that no samples are currently used */
  for (list = sfont->sample; list; list = fluid_list_next(list)) {
    sample = (fluid_sample_t*) fluid_list_get(list);
    if (fluid_sample_refcount(sample) != 0) {
      return -1;
    }
  }

  if (sfont->filename != 0) {
    FLUID_FREE(sfont->filename);
  }

  for (list = sfont->sample; list; list = fluid_list_next(list)) {
    delete_fluid_sample((fluid_sample_t*) fluid_list_get(list));
  }

  if (sfont->sample) {
    delete_fluid_list(sfont->sample);
  }

  if (sfont->sampledata != 0) {
//    fluid_munlock(sfont->sampledata, sfont->samplesize);
    FLUID_FREE(sfont->sampledata);
  }

  preset = sfont->preset;
  while (preset != 0) {
    sfont->preset = preset->next;
    delete_fluid_defpreset(preset);
    preset = sfont->preset;
  }

  FLUID_FREE(sfont);
  return FLUID_OK;
}

/*
 * fluid_defsfont_get_name
 */
char* fluid_defsfont_get_name(fluid_defsfont_t* sfont)
{
  return sfont->filename;
}


/*
 * fluid_defsfont_load
 */
int fluid_defsfont_load(fluid_defsfont_t* sfont, const QString& file)
{
  SFData* sfdata;
  fluid_list_t *p;
  SFPreset* sfpreset;
  SFSample* sfsample;
  fluid_sample_t* sample;
  fluid_defpreset_t* preset;

  sfont->filename = (char*)FLUID_MALLOC(1 + file.size());
  FLUID_STRCPY(sfont->filename, file.toLatin1().data());

  /* The actual loading is done in the sfont and sffile files */
  sfdata = sfload_file(file);
  if (sfdata == 0) {
    FLUID_LOG(FLUID_ERR, "Couldn't load soundfont file");
    return FLUID_FAILED;
  }

  /* Keep track of the position and size of the sample data because
     it's loaded separately (and might be unloaded/reloaded in future) */
  sfont->samplepos = sfdata->samplepos;
  sfont->samplesize = sfdata->samplesize;

  /* load sample data in one block */
  if (fluid_defsfont_load_sampledata(sfont) != FLUID_OK)
    goto err_exit;

  /* Create all the sample headers */
  p = sfdata->sample;
  while (p != 0) {
    sfsample = (SFSample *) p->data;

    sample = new_fluid_sample();
    if (sample == 0) goto err_exit;

    if (fluid_sample_import_sfont(sample, sfsample, sfont) != FLUID_OK)
      goto err_exit;

    fluid_defsfont_add_sample(sfont, sample);
    fluid_voice_optimize_sample(sample);
    p = fluid_list_next(p);
  }

  /* Load all the presets */
  p = sfdata->preset;
  while (p != 0) {
    sfpreset = (SFPreset *) p->data;
    preset = new_fluid_defpreset(sfont);
    if (preset == 0) goto err_exit;

    if (fluid_defpreset_import_sfont(preset, sfpreset, sfont) != FLUID_OK)
      goto err_exit;

    fluid_defsfont_add_preset(sfont, preset);
    p = fluid_list_next(p);
  }
  delete sfdata;

  return FLUID_OK;

err_exit:
  delete sfdata;
  return FLUID_FAILED;
}

/* fluid_defsfont_add_sample
 *
 * Add a sample to the SoundFont
 */
int fluid_defsfont_add_sample(fluid_defsfont_t* sfont, fluid_sample_t* sample)
{
  sfont->sample = fluid_list_append(sfont->sample, sample);
  return FLUID_OK;
}

/* fluid_defsfont_add_preset
 *
 * Add a preset to the SoundFont
 */
int fluid_defsfont_add_preset(fluid_defsfont_t* sfont, fluid_defpreset_t* preset)
{
  fluid_defpreset_t *cur, *prev;
  if (sfont->preset == 0) {
    preset->next = 0;
    sfont->preset = preset;
  } else {
    /* sort them as we go along. very basic sorting trick. */
    cur = sfont->preset;
    prev = 0;
    while (cur != 0) {
      if ((preset->bank < cur->bank)
	  || ((preset->bank == cur->bank) && (preset->num < cur->num))) {
	if (prev == 0) {
	  preset->next = cur;
	  sfont->preset = preset;
	} else {
	  preset->next = cur;
	  prev->next = preset;
	}
	return FLUID_OK;
      }
      prev = cur;
      cur = cur->next;
    }
    preset->next = 0;
    prev->next = preset;
  }
  return FLUID_OK;
}

/*
 * fluid_defsfont_load_sampledata
 */
int
fluid_defsfont_load_sampledata(fluid_defsfont_t* sfont)
      {
      QFile fd(sfont->filename);
      unsigned short endian;

      if (!fd.open(QIODevice::ReadOnly)) {
            FLUID_LOG(FLUID_ERR, "Can't open soundfont file");
            return FLUID_FAILED;
            }

      if (!fd.seek(sfont->samplepos)) {
            perror("error");
            FLUID_LOG(FLUID_ERR, "Failed to seek position in data file");
            fd.close();
            return FLUID_FAILED;
            }
      sfont->sampledata = (short*) FLUID_MALLOC(sfont->samplesize);
      if (sfont->sampledata == 0) {
            FLUID_LOG(FLUID_ERR, "Out of memory");
            fd.close();
            return FLUID_FAILED;
            }
      if (fd.read((char*)sfont->sampledata, sfont->samplesize) != sfont->samplesize) {
            FLUID_LOG(FLUID_ERR, "Failed to read sample data");
            fd.close();
            return FLUID_FAILED;
            }
      fd.close();

      /* Lock the memory to disable paging. It's okay if this fails. It
         probably means that the user doesn't have to required permission.
      */
//      if (fluid_mlock(sfont->sampledata, sfont->samplesize) != 0) {
//            FLUID_LOG(FLUID_WARN, "Failed to pin the sample data to RAM; swapping is possible.");
//            }

      /* I'm not sure this endian test is waterproof...  */
      endian = 0x0100;

      /* If this machine is big endian, the sample have to byte swapped  */
      if (((char *) &endian)[0]) {
            unsigned char* cbuf;
            unsigned char hi, lo;
            unsigned int i, j;
            short s;
            cbuf = (unsigned char*) sfont->sampledata;
            for (i = 0, j = 0; j < sfont->samplesize; i++) {
                  lo = cbuf[j++];
                  hi = cbuf[j++];
                  s = (hi << 8) | lo;
                  sfont->sampledata[i] = s;
                  }
            }
      return FLUID_OK;
      }

/*
 * fluid_defsfont_get_sample
 */
fluid_sample_t* fluid_defsfont_get_sample(fluid_defsfont_t* sfont, char *s)
{
  fluid_list_t* list;
  fluid_sample_t* sample;

  for (list = sfont->sample; list; list = fluid_list_next(list)) {

    sample = (fluid_sample_t*) fluid_list_get(list);

    if (FLUID_STRCMP(sample->name, s) == 0) {
      return sample;
    }
  }

  return 0;
}

/*
 * fluid_defsfont_get_preset
 */
fluid_defpreset_t* fluid_defsfont_get_preset(fluid_defsfont_t* sfont, unsigned int bank, unsigned int num)
{
  fluid_defpreset_t* preset = sfont->preset;
  while (preset != 0) {
    if ((preset->bank == bank) && ((preset->num == num))) {
      return preset;
    }
    preset = preset->next;
  }
  return 0;
}

/*
 * fluid_defsfont_iteration_start
 */
void fluid_defsfont_iteration_start(fluid_defsfont_t* sfont)
{
  sfont->iter_cur = sfont->preset;
}

/*
 * fluid_defsfont_iteration_next
 */
int fluid_defsfont_iteration_next(fluid_defsfont_t* sfont, fluid_preset_t* preset)
{
  if (sfont->iter_cur == 0) {
    return 0;
  }

  preset->data = (void*) sfont->iter_cur;
  sfont->iter_cur = fluid_defpreset_next(sfont->iter_cur);
  return 1;
}

/***************************************************************
 *
 *                           PRESET
 */

/*
 * new_fluid_defpreset
 */
fluid_defpreset_t*
new_fluid_defpreset(fluid_defsfont_t* sfont)
{
  fluid_defpreset_t* preset = FLUID_NEW(fluid_defpreset_t);
  if (preset == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }
  preset->next = 0;
  preset->sfont = sfont;
  preset->name[0] = 0;
  preset->bank = 0;
  preset->num = 0;
  preset->global_zone = 0;
  preset->zone = 0;
  return preset;
}

/*
 * delete_fluid_defpreset
 */
int
delete_fluid_defpreset(fluid_defpreset_t* preset)
{
  int err = FLUID_OK;
  fluid_preset_zone_t* zone;
  if (preset->global_zone != 0) {
    if (delete_fluid_preset_zone(preset->global_zone) != FLUID_OK) {
      err = FLUID_FAILED;
    }
    preset->global_zone = 0;
  }
  zone = preset->zone;
  while (zone != 0) {
    preset->zone = zone->next;
    if (delete_fluid_preset_zone(zone) != FLUID_OK) {
      err = FLUID_FAILED;
    }
    zone = preset->zone;
  }
  FLUID_FREE(preset);
  return err;
}

int fluid_defpreset_get_banknum(fluid_defpreset_t* preset)
{
  return preset->bank;
}

int
fluid_defpreset_get_num(fluid_defpreset_t* preset)
{
  return preset->num;
}

char*
fluid_defpreset_get_name(fluid_defpreset_t* preset)
{
  return preset->name;
}

/*
 * fluid_defpreset_next
 */
fluid_defpreset_t*
fluid_defpreset_next(fluid_defpreset_t* preset)
{
  return preset->next;
}


/*
 * fluid_defpreset_noteon
 */
int
fluid_defpreset_noteon(fluid_defpreset_t* preset, fluid_synth_t* synth, int chan, int key, int vel)
{
  fluid_preset_zone_t *preset_zone, *global_preset_zone;
  fluid_inst_t* inst;
  fluid_inst_zone_t *inst_zone, *global_inst_zone, *z;
  fluid_sample_t* sample;
  fluid_voice_t* voice;
  fluid_mod_t * mod;
  fluid_mod_t * mod_list[FLUID_NUM_MOD]; /* list for 'sorting' preset modulators */
  int mod_list_count;
  int i;

  global_preset_zone = fluid_defpreset_get_global_zone(preset);

  /* run thru all the zones of this preset */
  preset_zone = fluid_defpreset_get_zone(preset);
  while (preset_zone != 0) {

    /* check if the note falls into the key and velocity range of this
       preset */
    if (fluid_preset_zone_inside_range(preset_zone, key, vel)) {

      inst = fluid_preset_zone_get_inst(preset_zone);
      global_inst_zone = fluid_inst_get_global_zone(inst);

      /* run thru all the zones of this instrument */
      inst_zone = fluid_inst_get_zone(inst);
	  while (inst_zone != 0) {

	/* make sure this instrument zone has a valid sample */
	sample = fluid_inst_zone_get_sample(inst_zone);
	if (fluid_sample_in_rom(sample) || (sample == 0)) {
	  inst_zone = fluid_inst_zone_next(inst_zone);
	  continue;
	}

	/* check if the note falls into the key and velocity range of this
	   instrument */

	if (fluid_inst_zone_inside_range(inst_zone, key, vel) && (sample != 0)) {

	  /* this is a good zone. allocate a new synthesis process and
             initialize it */

	  voice = fluid_synth_alloc_voice(synth, sample, chan, key, vel);
	  if (voice == 0) {
	    return FLUID_FAILED;
	  }


	  z = inst_zone;

	  /* Instrument level, generators */

	  for (i = 0; i < GEN_LAST; i++) {

	    /* SF 2.01 section 9.4 'bullet' 4:
	     *
	     * A generator in a local instrument zone supersedes a
	     * global instrument zone generator.  Both cases supersede
	     * the default generator -> voice_gen_set */

	    if (inst_zone->gen[i].flags){
	      fluid_voice_gen_set(voice, i, inst_zone->gen[i].val);

	    } else if ((global_inst_zone != 0) && (global_inst_zone->gen[i].flags)) {
	      fluid_voice_gen_set(voice, i, global_inst_zone->gen[i].val);

	    } else {
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

	  while (mod){

	    /* 'Identical' modulators will be deleted by setting their
	     *  list entry to 0.  The list length is known, 0
	     *  entries will be ignored later.  SF2.01 section 9.5.1
	     *  page 69, 'bullet' 3 defines 'identical'.  */

	    for (i = 0; i < mod_list_count; i++){
	      if (fluid_mod_test_identity(mod,mod_list[i])){
		mod_list[i] = 0;
	      }
	    }

	    /* Finally add the new modulator to to the list. */
	    mod_list[mod_list_count++] = mod;
	    mod = mod->next;
	  }

	  /* Add instrument modulators (global / local) to the voice. */
	  for (i = 0; i < mod_list_count; i++){

	    mod = mod_list[i];

	    if (mod != 0){ /* disabled modulators CANNOT be skipped. */

	      /* Instrument modulators -supersede- existing (default)
	       * modulators.  SF 2.01 page 69, 'bullet' 6 */
	      fluid_voice_add_mod(voice, mod, FLUID_VOICE_OVERWRITE);
	    }
	  }

	  /* Preset level, generators */

	  for (i = 0; i < GEN_LAST; i++) {

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
		fluid_voice_gen_incr(voice, i, preset_zone->gen[i].val);
	      } else if ((global_preset_zone != 0) && global_preset_zone->gen[i].flags) {
		fluid_voice_gen_incr(voice, i, global_preset_zone->gen[i].val);
	      } else {
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
	    while (mod){
	      mod_list[mod_list_count++] = mod;
	      mod = mod->next;
	    }
	  }

	  /* Process the modulators of the local preset zone.  Kick
	   * out all identical modulators from the global preset zone
	   * (SF 2.01 page 69, second-last bullet) */

	  mod = preset_zone->mod;
	  while (mod){
	    for (i = 0; i < mod_list_count; i++){
	      if (fluid_mod_test_identity(mod,mod_list[i])){
		mod_list[i] = 0;
	      }
	    }

	    /* Finally add the new modulator to the list. */
	    mod_list[mod_list_count++] = mod;
	    mod = mod->next;
	  }

	  /* Add preset modulators (global / local) to the voice. */
	  for (i = 0; i < mod_list_count; i++){
	    mod = mod_list[i];
	    if ((mod != 0) && (mod->amount != 0)) { /* disabled modulators can be skipped. */

	      /* Preset modulators -add- to existing instrument /
	       * default modulators.  SF2.01 page 70 first bullet on
	       * page */
	      fluid_voice_add_mod(voice, mod, FLUID_VOICE_ADD);
	    }
	  }

	  /* add the synthesis process to the synthesis loop. */
	  fluid_synth_start_voice(synth, voice);

	  /* Store the ID of the first voice that was created by this noteon event.
	   * Exclusive class may only terminate older voices.
	   * That avoids killing voices, which have just been created.
	   * (a noteon event can create several voice processes with the same exclusive
	   * class - for example when using stereo samples)
	   */
	}

	inst_zone = fluid_inst_zone_next(inst_zone);
      }
	}
    preset_zone = fluid_preset_zone_next(preset_zone);
  }

  return FLUID_OK;
}

/*
 * fluid_defpreset_set_global_zone
 */
int
fluid_defpreset_set_global_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone)
{
  preset->global_zone = zone;
  return FLUID_OK;
}

/*
 * fluid_defpreset_import_sfont
 */
int
fluid_defpreset_import_sfont(fluid_defpreset_t* preset,
			     SFPreset* sfpreset,
			     fluid_defsfont_t* sfont)
{
  fluid_list_t *p;
  SFZone* sfzone;
  fluid_preset_zone_t* zone;
  int count;
  char zone_name[256];
  if ((sfpreset->name != 0) && (FLUID_STRLEN(sfpreset->name) > 0)) {
    FLUID_STRCPY(preset->name, sfpreset->name);
  } else {
    FLUID_SPRINTF(preset->name, "Bank%d,Preset%d", sfpreset->bank, sfpreset->prenum);
  }
  preset->bank = sfpreset->bank;
  preset->num = sfpreset->prenum;
  p = sfpreset->zone;
  count = 0;
  while (p != 0) {
    sfzone = (SFZone *) p->data;
    FLUID_SPRINTF(zone_name, "%s/%d", preset->name, count);
    zone = new_fluid_preset_zone(zone_name);
    if (zone == 0) {
      return FLUID_FAILED;
    }
    if (fluid_preset_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
      return FLUID_FAILED;
    }
    if ((count == 0) && (fluid_preset_zone_get_inst(zone) == 0)) {
      fluid_defpreset_set_global_zone(preset, zone);
    } else if (fluid_defpreset_add_zone(preset, zone) != FLUID_OK) {
      return FLUID_FAILED;
    }
    p = fluid_list_next(p);
    count++;
  }
  return FLUID_OK;
}

/*
 * fluid_defpreset_add_zone
 */
int
fluid_defpreset_add_zone(fluid_defpreset_t* preset, fluid_preset_zone_t* zone)
{
  if (preset->zone == 0) {
    zone->next = 0;
    preset->zone = zone;
  } else {
    zone->next = preset->zone;
    preset->zone = zone;
  }
  return FLUID_OK;
}

/*
 * fluid_defpreset_get_zone
 */
fluid_preset_zone_t*
fluid_defpreset_get_zone(fluid_defpreset_t* preset)
{
  return preset->zone;
}

/*
 * fluid_defpreset_get_global_zone
 */
fluid_preset_zone_t*
fluid_defpreset_get_global_zone(fluid_defpreset_t* preset)
{
  return preset->global_zone;
}

/*
 * fluid_preset_zone_next
 */
fluid_preset_zone_t*
fluid_preset_zone_next(fluid_preset_zone_t* preset)
{
  return preset->next;
}

/*
 * new_fluid_preset_zone
 */
fluid_preset_zone_t*
new_fluid_preset_zone(char *name)
{
  int size;
  fluid_preset_zone_t* zone = 0;
  zone = FLUID_NEW(fluid_preset_zone_t);
  if (zone == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }
  zone->next = 0;
  size = 1 + FLUID_STRLEN(name);
  zone->name = (char*)FLUID_MALLOC(size);
  if (zone->name == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(zone);
    return 0;
  }
  FLUID_STRCPY(zone->name, name);
  zone->inst = 0;
  zone->keylo = 0;
  zone->keyhi = 128;
  zone->vello = 0;
  zone->velhi = 128;

  /* Flag all generators as unused (default, they will be set when they are found
   * in the sound font).
   * This also sets the generator values to default, but that is of no concern here.*/
  fluid_gen_set_default_values(&zone->gen[0]);
  zone->mod = 0; /* list of modulators */
  return zone;
}

/***************************************************************
 *
 *                           PRESET_ZONE
 */

/*
 * delete_fluid_preset_zone
 */
int
delete_fluid_preset_zone(fluid_preset_zone_t* zone)
{
  fluid_mod_t *mod, *tmp;

  mod = zone->mod;
  while (mod)	/* delete the modulators */
    {
      tmp = mod;
      mod = mod->next;
      fluid_mod_delete (tmp);
    }

  if (zone->name) FLUID_FREE (zone->name);
  if (zone->inst) delete_fluid_inst (zone->inst);
  FLUID_FREE(zone);
  return FLUID_OK;
}

/*
 * fluid_preset_zone_import_sfont
 */
int
fluid_preset_zone_import_sfont(fluid_preset_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont)
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
      /* FIXME: some generators have an unsigne word amount value but i don't know which ones */
      zone->gen[sfgen->id].val = (fluid_real_t) sfgen->amount.sword;
      zone->gen[sfgen->id].flags = GEN_SET;
      break;
    }
    r = fluid_list_next(r);
  }
  if ((sfzone->instsamp != 0) && (sfzone->instsamp->data != 0)) {
    zone->inst = (fluid_inst_t*) new_fluid_inst();
    if (zone->inst == 0) {
      FLUID_LOG(FLUID_ERR, "Out of memory");
      return FLUID_FAILED;
    }
    if (fluid_inst_import_sfont(zone->inst, (SFInst *) sfzone->instsamp->data, sfont) != FLUID_OK) {
      return FLUID_FAILED;
    }
  }

  /* Import the modulators (only SF2.1 and higher) */
  for (count = 0, r = sfzone->mod; r != 0; count++) {

    SFMod* mod_src = (SFMod *)r->data;
    fluid_mod_t * mod_dest = fluid_mod_new();
    int type;

    if (mod_dest == 0){
      return FLUID_FAILED;
    }
    mod_dest->next = 0; /* pointer to next modulator, this is the end of the list now.*/

    /* *** Amount *** */
    mod_dest->amount = mod_src->amount;

    /* *** Source *** */
    mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
    mod_dest->flags1 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->src & (1<<7)){
      mod_dest->flags1 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags1 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->src & (1<<8)){
      mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags1 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->src & (1<<9)){
      mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type=(mod_src->src) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags1 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags1 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags1 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags1 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount=0;
    }

    /* *** Dest *** */
    mod_dest->dest = mod_src->dest; /* index of controlled generator */

    /* *** Amount source *** */
    mod_dest->src2 = mod_src->amtsrc & 127; /* index of source 2, seven-bit value, SF2.01 section 8.2, p.50 */
    mod_dest->flags2 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->amtsrc & (1<<7)){
      mod_dest->flags2 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags2 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->amtsrc & (1<<8)){
      mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags2 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->amtsrc & (1<<9)){
      mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = (mod_src->amtsrc) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags2 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags2 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags2 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags2 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount=0;
    }

    /* *** Transform *** */
    /* SF2.01 only uses the 'linear' transform (0).
     * Deactivate the modulator by setting the amount to 0 in any other case.
     */
    if (mod_src->trans !=0){
      mod_dest->amount = 0;
    }

    /* Store the new modulator in the zone The order of modulators
     * will make a difference, at least in an instrument context: The
     * second modulator overwrites the first one, if they only differ
     * in amount. */
    if (count == 0){
      zone->mod = mod_dest;
    } else {
      fluid_mod_t * last_mod = zone->mod;

      /* Find the end of the list */
      while (last_mod->next != 0){
	last_mod=last_mod->next;
      }

      last_mod->next = mod_dest;
    }

    r = fluid_list_next(r);
  } /* foreach modulator */

  return FLUID_OK;
}

/*
 * fluid_preset_zone_get_inst
 */
fluid_inst_t*
fluid_preset_zone_get_inst(fluid_preset_zone_t* zone)
{
  return zone->inst;
}

/*
 * fluid_preset_zone_inside_range
 */
int
fluid_preset_zone_inside_range(fluid_preset_zone_t* zone, int key, int vel)
{
  return ((zone->keylo <= key) &&
	  (zone->keyhi >= key) &&
	  (zone->vello <= vel) &&
	  (zone->velhi >= vel));
}

/***************************************************************
 *
 *                           INST
 */

/*
 * new_fluid_inst
 */
fluid_inst_t*
new_fluid_inst()
{
  fluid_inst_t* inst = FLUID_NEW(fluid_inst_t);
  if (inst == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }
  inst->name[0] = 0;
  inst->global_zone = 0;
  inst->zone = 0;
  return inst;
}

/*
 * delete_fluid_inst
 */
int
delete_fluid_inst(fluid_inst_t* inst)
{
  fluid_inst_zone_t* zone;
  int err = FLUID_OK;
  if (inst->global_zone != 0) {
    if (delete_fluid_inst_zone(inst->global_zone) != FLUID_OK) {
      err = FLUID_FAILED;
    }
    inst->global_zone = 0;
  }
  zone = inst->zone;
  while (zone != 0) {
    inst->zone = zone->next;
    if (delete_fluid_inst_zone(zone) != FLUID_OK) {
      err = FLUID_FAILED;
    }
    zone = inst->zone;
  }
  FLUID_FREE(inst);
  return err;
}

/*
 * fluid_inst_set_global_zone
 */
int
fluid_inst_set_global_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
  inst->global_zone = zone;
  return FLUID_OK;
}

/*
 * fluid_inst_import_sfont
 */
int
fluid_inst_import_sfont(fluid_inst_t* inst, SFInst *sfinst, fluid_defsfont_t* sfont)
{
  fluid_list_t *p;
  SFZone* sfzone;
  fluid_inst_zone_t* zone;
  char zone_name[256];
  int count;

  p = sfinst->zone;
  if ((sfinst->name != 0) && (FLUID_STRLEN(sfinst->name) > 0)) {
    FLUID_STRCPY(inst->name, sfinst->name);
  } else {
    FLUID_STRCPY(inst->name, "<untitled>");
  }

  count = 0;
  while (p != 0) {

    sfzone = (SFZone *) p->data;
    FLUID_SPRINTF(zone_name, "%s/%d", inst->name, count);

    zone = new_fluid_inst_zone(zone_name);
    if (zone == 0) {
      return FLUID_FAILED;
    }

    if (fluid_inst_zone_import_sfont(zone, sfzone, sfont) != FLUID_OK) {
      return FLUID_FAILED;
    }

    if ((count == 0) && (fluid_inst_zone_get_sample(zone) == 0)) {
      fluid_inst_set_global_zone(inst, zone);

    } else if (fluid_inst_add_zone(inst, zone) != FLUID_OK) {
      return FLUID_FAILED;
    }

    p = fluid_list_next(p);
    count++;
  }
  return FLUID_OK;
}

/*
 * fluid_inst_add_zone
 */
int fluid_inst_add_zone(fluid_inst_t* inst, fluid_inst_zone_t* zone)
{
  if (inst->zone == 0) {
    zone->next = 0;
    inst->zone = zone;
  } else {
    zone->next = inst->zone;
    inst->zone = zone;
  }
  return FLUID_OK;
}

/*
 * fluid_inst_get_zone
 */
fluid_inst_zone_t* fluid_inst_get_zone(fluid_inst_t* inst)
{
  return inst->zone;
}

/*
 * fluid_inst_get_global_zone
 */
fluid_inst_zone_t* fluid_inst_get_global_zone(fluid_inst_t* inst)
{
  return inst->global_zone;
}

/***************************************************************
 *
 *                           INST_ZONE
 */

/*
 * new_fluid_inst_zone
 */
fluid_inst_zone_t*
new_fluid_inst_zone(char* name)
{
  int size;
  fluid_inst_zone_t* zone = 0;
  zone = FLUID_NEW(fluid_inst_zone_t);
  if (zone == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }
  zone->next = 0;
  size = 1 + FLUID_STRLEN(name);
  zone->name = (char*)FLUID_MALLOC(size);
  if (zone->name == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    FLUID_FREE(zone);
    return 0;
  }
  FLUID_STRCPY(zone->name, name);
  zone->sample = 0;
  zone->keylo = 0;
  zone->keyhi = 128;
  zone->vello = 0;
  zone->velhi = 128;

  /* Flag the generators as unused.
   * This also sets the generator values to default, but they will be overwritten anyway, if used.*/
  fluid_gen_set_default_values(&zone->gen[0]);
  zone->mod=0; /* list of modulators */
  return zone;
}

/*
 * delete_fluid_inst_zone
 */
int
delete_fluid_inst_zone(fluid_inst_zone_t* zone)
{
  fluid_mod_t *mod, *tmp;

  mod = zone->mod;
  while (mod)	/* delete the modulators */
    {
      tmp = mod;
      mod = mod->next;
      fluid_mod_delete (tmp);
    }

  if (zone->name) FLUID_FREE (zone->name);
  FLUID_FREE(zone);
  return FLUID_OK;
}

/*
 * fluid_inst_zone_next
 */
fluid_inst_zone_t*
fluid_inst_zone_next(fluid_inst_zone_t* zone)
{
  return zone->next;
}

/*
 * fluid_inst_zone_import_sfont
 */
int
fluid_inst_zone_import_sfont(fluid_inst_zone_t* zone, SFZone *sfzone, fluid_defsfont_t* sfont)
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
	 i don't know which ones */
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

  if ((sfzone->instsamp != 0) && (sfzone->instsamp->data != 0)) {
    zone->sample = fluid_defsfont_get_sample(sfont, ((SFSample *) sfzone->instsamp->data)->name);
    if (zone->sample == 0) {
      FLUID_LOG(FLUID_ERR, "Couldn't find sample name");
      return FLUID_FAILED;
    }
  }

  /* Import the modulators (only SF2.1 and higher) */
  for (count = 0, r = sfzone->mod; r != 0; count++) {
    SFMod* mod_src = (SFMod *) r->data;
    int type;
    fluid_mod_t* mod_dest;

    mod_dest = fluid_mod_new();
    if (mod_dest == 0){
      return FLUID_FAILED;
    }

    mod_dest->next = 0; /* pointer to next modulator, this is the end of the list now.*/

    /* *** Amount *** */
    mod_dest->amount = mod_src->amount;

    /* *** Source *** */
    mod_dest->src1 = mod_src->src & 127; /* index of source 1, seven-bit value, SF2.01 section 8.2, page 50 */
    mod_dest->flags1 = 0;

    /* Bit 7: CC flag SF 2.01 section 8.2.1 page 50*/
    if (mod_src->src & (1<<7)){
      mod_dest->flags1 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags1 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->src & (1<<8)){
      mod_dest->flags1 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags1 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->src & (1<<9)){
      mod_dest->flags1 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags1 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type = (mod_src->src) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags1 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags1 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags1 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags1 |= FLUID_MOD_SWITCH;
    } else {
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
    if (mod_src->amtsrc & (1<<7)){
      mod_dest->flags2 |= FLUID_MOD_CC;
    } else {
      mod_dest->flags2 |= FLUID_MOD_GC;
    }

    /* Bit 8: D flag SF 2.01 section 8.2.2 page 51*/
    if (mod_src->amtsrc & (1<<8)){
      mod_dest->flags2 |= FLUID_MOD_NEGATIVE;
    } else {
      mod_dest->flags2 |= FLUID_MOD_POSITIVE;
    }

    /* Bit 9: P flag SF 2.01 section 8.2.3 page 51*/
    if (mod_src->amtsrc & (1<<9)){
      mod_dest->flags2 |= FLUID_MOD_BIPOLAR;
    } else {
      mod_dest->flags2 |= FLUID_MOD_UNIPOLAR;
    }

    /* modulator source types: SF2.01 section 8.2.1 page 52 */
    type=(mod_src->amtsrc) >> 10;
    type &= 63; /* type is a 6-bit value */
    if (type == 0){
      mod_dest->flags2 |= FLUID_MOD_LINEAR;
    } else if (type == 1){
      mod_dest->flags2 |= FLUID_MOD_CONCAVE;
    } else if (type == 2){
      mod_dest->flags2 |= FLUID_MOD_CONVEX;
    } else if (type == 3){
      mod_dest->flags2 |= FLUID_MOD_SWITCH;
    } else {
      /* This shouldn't happen - unknown type!
       * Deactivate the modulator by setting the amount to 0. */
      mod_dest->amount = 0;
    }

    /* *** Transform *** */
    /* SF2.01 only uses the 'linear' transform (0).
     * Deactivate the modulator by setting the amount to 0 in any other case.
     */
    if (mod_src->trans !=0){
      mod_dest->amount = 0;
    }

    /* Store the new modulator in the zone
     * The order of modulators will make a difference, at least in an instrument context:
     * The second modulator overwrites the first one, if they only differ in amount. */
    if (count == 0){
      zone->mod=mod_dest;
    } else {
      fluid_mod_t * last_mod=zone->mod;
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
fluid_sample_t*
fluid_inst_zone_get_sample(fluid_inst_zone_t* zone)
{
  return zone->sample;
}

/*
 * fluid_inst_zone_inside_range
 */
int
fluid_inst_zone_inside_range(fluid_inst_zone_t* zone, int key, int vel)
{
  return ((zone->keylo <= key) &&
	  (zone->keyhi >= key) &&
	  (zone->vello <= vel) &&
	  (zone->velhi >= vel));
}

/***************************************************************
 *
 *                           SAMPLE
 */

/*
 * new_fluid_sample
 */
fluid_sample_t*
new_fluid_sample()
{
  fluid_sample_t* sample = 0;

  sample = FLUID_NEW(fluid_sample_t);
  if (sample == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  memset(sample, 0, sizeof(fluid_sample_t));
  sample->valid = 1;

  return sample;
}

/*
 * delete_fluid_sample
 */
int
delete_fluid_sample(fluid_sample_t* sample)
{
  FLUID_FREE(sample);
  return FLUID_OK;
}

/*
 * fluid_sample_in_rom
 */
int
fluid_sample_in_rom(fluid_sample_t* sample)
{
  return (sample->sampletype & FLUID_SAMPLETYPE_ROM);
}

/*
 * fluid_sample_import_sfont
 */
int
fluid_sample_import_sfont(fluid_sample_t* sample, SFSample* sfsample, fluid_defsfont_t* sfont)
{
  FLUID_STRCPY(sample->name, sfsample->name);
  sample->data = sfont->sampledata;
  sample->start = sfsample->start;
  sample->end = sfsample->start + sfsample->end;
  sample->loopstart = sfsample->start + sfsample->loopstart;
  sample->loopend = sfsample->start + sfsample->loopend;
  sample->samplerate = sfsample->samplerate;
  sample->origpitch = sfsample->origpitch;
  sample->pitchadj = sfsample->pitchadj;
  sample->sampletype = sfsample->sampletype;

  if (sample->sampletype & FLUID_SAMPLETYPE_ROM) {
    sample->valid = 0;
    FLUID_LOG(FLUID_WARN, "Ignoring sample %s: can't use ROM samples", sample->name);
  }
  if (sample->end - sample->start < 8) {
    sample->valid = 0;
    FLUID_LOG(FLUID_WARN, "Ignoring sample %s: too few sample data points", sample->name);
  } else {
/*      if (sample->loopstart < sample->start + 8) { */
/*        FLUID_LOG(FLUID_WARN, "Fixing sample %s: at least 8 data points required before loop start", sample->name);     */
/*        sample->loopstart = sample->start + 8; */
/*      } */
/*      if (sample->loopend > sample->end - 8) { */
/*        FLUID_LOG(FLUID_WARN, "Fixing sample %s: at least 8 data points required after loop end", sample->name);     */
/*        sample->loopend = sample->end - 8; */
/*      } */
  }
  return FLUID_OK;
}



/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/



/*=================================sfload.c========================
  Borrowed from Smurf SoundFont Editor by Josh Green
  =================================================================*/

/*
   functions for loading data from sfont files, with appropriate byte swapping
   on big endian machines. Sfont IDs are not swapped because the ID read is
   equivalent to the matching ID list in memory regardless of LE/BE machine
*/

#define READID(var,fd)		G_STMT_START {		\
    if (!fd->safe_fread(var, 4))			\
	return(FAIL);					\
} G_STMT_END

#define READSTR(var,fd)		G_STMT_START {		\
    if (!fd->safe_fread(var, 20))			\
	return(FAIL);					\
    (*var)[20] = '\0';					\
} G_STMT_END

#ifdef WORDS_BIGENDIAN
#define READD(var,fd)		G_STMT_START {		\
	unsigned int _temp;					\
	if (!fd->safe_fread(&_temp, 4))			\
	return(FAIL);					\
	var = GINT32_FROM_BE(_temp);			\
} G_STMT_END
#else
#define READD(var,fd)		G_STMT_START {		\
    unsigned int _temp;					\
    if (!fd->safe_fread(&_temp, 4))			\
	return(FAIL);					\
    var = GINT32_FROM_LE(_temp);			\
} G_STMT_END
#endif


#ifdef WORDS_BIGENDIAN
#define READW(var,fd)		G_STMT_START {		\
	unsigned short _temp;					\
	if (!fd->safe_fread(&_temp, 2))			\
	return(FAIL);					\
var = GINT16_FROM_BE(_temp);			\
} G_STMT_END
#else
#define READW(var,fd)		G_STMT_START {		\
    unsigned short _temp;					\
    if (!fd->safe_fread(&_temp, 2))			\
	return(FAIL);					\
    var = GINT16_FROM_LE(_temp);			\
} G_STMT_END
#endif


#define READB(var,fd)		G_STMT_START {		\
    if (!fd->safe_fread(&var, 1))			\
	return(FAIL);					\
} G_STMT_END
#define FSKIP(size,fd)		G_STMT_START {		\
    if (!fd->safe_fseek(size, SEEK_CUR))		\
	return(FAIL);					\
} G_STMT_END
#define FSKIPW(fd)		G_STMT_START {		\
    if (!fd->safe_fseek(2, SEEK_CUR))			\
	return(FAIL);					\
} G_STMT_END

/* removes and advances a fluid_list_t pointer */
#define SLADVREM(list, item)	G_STMT_START {		\
    fluid_list_t *_temp = item;				\
    item = fluid_list_next(item);				\
    list = fluid_list_remove_link(list, _temp);		\
    delete1_fluid_list(_temp);				\
} G_STMT_END

char idlist[] = {
      "RIFFLISTsfbkINFOsdtapdtaifilisngINAMiromiverICRDIENGIPRD"
      "ICOPICMTISFTsnamsmplphdrpbagpmodpgeninstibagimodigenshdr"
      };

static unsigned int sdtachunk_size;

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
      return (UNKN_ID);
      }

//---------------------------------------------------------
//   SFData
//---------------------------------------------------------

SFData::SFData(const QString& s)
   : QFile(s)
      {
      samplepos     = 0;
      samplesize    = 0;
      info          = 0;
      preset        = 0;
      inst          = 0;
      sample        = 0;
      }

//---------------------------------------------------------
//   sfload_file
//---------------------------------------------------------

SFData* sfload_file(const QString& fname)
      {
      SFData* sf = new SFData(fname);
      if (!(sf->open(QIODevice::ReadOnly))) {
            FLUID_LOG (FLUID_ERR, _("Unable to open file \"%s\""), fname.toLatin1().data());
            delete sf;
            return 0;
            }

      if (!sf->load_body()) {
            delete sf;
            return 0;
            }
      return sf;
      }

//---------------------------------------------------------
//   READCHUNK
//---------------------------------------------------------

bool SFData::READCHUNK(SFChunk* var)
      {
	if (!safe_fread(var, 8))
	      return false;
#ifdef WORDS_BIGENDIAN
      var->size = GUINT32_FROM_BE(var->size);
#else
      var->size = GUINT32_FROM_LE(var->size);
#endif
      return true;
      }

//---------------------------------------------------------
//   load_body
//    return true on success
//---------------------------------------------------------

bool SFData::load_body()
      {
      SFChunk chunk;

      if (!READCHUNK(&chunk) || (chunkid(chunk.id) != RIFF_ID)) {
            FLUID_LOG (FLUID_ERR, _("Not a RIFF file"));
            return false;
            }

      READID (&chunk.id, this);	/* load file ID */
      if (chunkid (chunk.id) != SFBK_ID) {	/* error if not SFBK_ID */
            FLUID_LOG (FLUID_ERR, _("Not a sound font file"));
            return false;
            }
      if (chunk.size != size() - 8) {
            gerr (ErrCorr, _("Sound font file size mismatch"));
            return false;
            }

      /* Process INFO block */
      if (!read_listchunk (&chunk))
            return false;
      if (chunkid(chunk.id) != INFO_ID)
            return (gerr (ErrCorr, _("Invalid ID found when expecting INFO chunk")));
      if (!process_info(chunk.size))
            return false;

      /* Process sample chunk */
      if (!read_listchunk (&chunk))
            return false;
      if (chunkid (chunk.id) != SDTA_ID)
            return (gerr (ErrCorr, _("Invalid ID found when expecting SAMPLE chunk")));
      if (!process_sdta(chunk.size))
            return false;

      /* process HYDRA chunk */
      if (!read_listchunk (&chunk))
            return false;
      if (chunkid (chunk.id) != PDTA_ID)
            return (gerr (ErrCorr, _("Invalid ID found when expecting HYDRA chunk")));
      if (!process_pdta (chunk.size))
            return false;

      if (!fixup_pgen())
            return false;
      if (!fixup_igen())
            return false;
      if (!fixup_sample())
            return false;

      /* sort preset list by bank, preset # */
      preset = fluid_list_sort(preset, (fluid_compare_func_t) sfont_preset_compare_func);

      return true;
      }

int SFData::read_listchunk (SFChunk* chunk)
      {
      SFData* fd = this;
      if (!fd->READCHUNK (chunk) || (chunkid (chunk->id) != LIST_ID))	{
            return (gerr (ErrCorr, _("Invalid chunk id in level 0 parse")));
            }
      READID (&chunk->id, fd);	/* read id string */
      chunk->size -= 4;
      return (OK);
      }

int SFData::process_info (int size)
{
      SFData * sf = this;
      SFData* fd  = this;

  SFChunk chunk;
  unsigned char id;
  char *item;
  unsigned short ver;

  while (size > 0)
    {
      if (!fd->READCHUNK (&chunk))
            return false;
      size -= 8;

      id = chunkid (chunk.id);

      if (id == IFIL_ID)
	{			/* sound font version chunk? */
	  if (chunk.size != 4)
	    return (gerr (ErrCorr,
		_("Sound font version info chunk has invalid size")));

	  READW (ver, fd);
	  sf->version.major = ver;
	  READW (ver, fd);
	  sf->version.minor = ver;

	  if (sf->version.major < 2) {
	    FLUID_LOG (FLUID_ERR,
		      _("Sound font version is %d.%d which is not"
			" supported, convert to version 2.0x"),
		      sf->version.major,
		      sf->version.minor);
	    return (FAIL);
	  }

	  if (sf->version.major > 2) {
	    FLUID_LOG (FLUID_WARN,
		      _("Sound font version is %d.%d which is newer than"
			" what this version of FLUID Synth was designed for (v2.0x)"),
		      sf->version.major,
		      sf->version.minor);
	    return (FAIL);
	  }
	}
      else if (id == IVER_ID)
	{			/* ROM version chunk? */
	  if (chunk.size != 4)
	    return (gerr (ErrCorr,
		_("ROM version info chunk has invalid size")));

	  READW (ver, fd);
	  sf->romver.major = ver;
	  READW (ver, fd);
	  sf->romver.minor = ver;
	}
      else if (id != UNKN_ID)
	{
	  if ((id != ICMT_ID && chunk.size > 256) || (chunk.size > 65536)
	    || (chunk.size % 2))
	    return (gerr (ErrCorr,
		_("INFO sub chunk %.4s has invalid chunk size"
		  " of %d bytes"), &chunk.id, chunk.size));

	  /* alloc for chunk id and da chunk */
	  if (!(item = (char*)FLUID_MALLOC (chunk.size + 1)))
	    {
	      FLUID_LOG(FLUID_ERR, "Out of memory");
	      return (FAIL);
	    }

	  /* attach to INFO list, sfont_close will cleanup if FAIL occurs */
	  sf->info = fluid_list_append (sf->info, item);

	  *(unsigned char *) item = id;
	  if (!fd->safe_fread (&item[1], chunk.size))
	    return (FAIL);

	  /* force terminate info item (don't forget uint8 info ID) */
	  *(item + chunk.size) = '\0';
	}
      else
	return (gerr (ErrCorr, _("Invalid chunk id in INFO chunk")));
      size -= chunk.size;
    }

  if (size < 0)
    return (gerr (ErrCorr, _("INFO chunk size mismatch")));

  return (OK);
}

int SFData::process_sdta (int size)
{
      SFData * sf = this;
      SFData* fd  = this;

  SFChunk chunk;

  if (size == 0)
    return (OK);		/* no sample data? */

  /* read sub chunk */
  if (!fd->READCHUNK (&chunk))
      return false;
  size -= 8;

  if (chunkid (chunk.id) != SMPL_ID)
    return (gerr (ErrCorr,
	_("Expected SMPL chunk found invalid id instead")));

  if ((size - chunk.size) != 0)
    return (gerr (ErrCorr, _("SDTA chunk size mismatch")));

  /* sample data follows */
  sf->samplepos = fd->pos();

  /* used in fixup_sample() to check validity of sample headers */
  sdtachunk_size = chunk.size;
  sf->samplesize = chunk.size;

  FSKIP (chunk.size, fd);

  return (OK);
}

int SFData::pdtahelper (unsigned int expid, unsigned int reclen,
   SFChunk * chunk, int * size)
      {
      SFData* fd = this;

  unsigned int id;
  char *expstr;

  expstr = CHNKIDSTR (expid);	/* in case we need it */

  if (!fd->READCHUNK (chunk))
      return false;
  *size -= 8;

  if ((id = chunkid (chunk->id)) != expid)
    return (gerr (ErrCorr, _("Expected"
	  " PDTA sub-chunk \"%.4s\" found invalid id instead"), expstr));

  if (chunk->size % reclen)	/* valid chunk size? */
    return (gerr (ErrCorr,
	_("\"%.4s\" chunk size is not a multiple of %d bytes"), expstr,
	reclen));
  if ((*size -= chunk->size) < 0)
    return (gerr (ErrCorr,
	_("\"%.4s\" chunk size exceeds remaining PDTA chunk size"), expstr));
  return (OK);
}

int SFData::process_pdta (int size)
{
      SFData * sf = this;
      SFData* fd = this;

  SFChunk chunk;

  if (!pdtahelper (PHDR_ID, SFPHDRSIZE, &chunk, &size))
    return (FAIL);
  if (!load_phdr (chunk.size))
    return (FAIL);

  if (!pdtahelper (PBAG_ID, SFBAGSIZE, &chunk, &size))
    return (FAIL);
  if (!load_pbag (chunk.size))
    return (FAIL);

  if (!pdtahelper (PMOD_ID, SFMODSIZE, &chunk, &size))
    return (FAIL);
  if (!load_pmod (chunk.size))
    return (FAIL);

  if (!pdtahelper (PGEN_ID, SFGENSIZE, &chunk, &size))
    return (FAIL);
  if (!load_pgen (chunk.size))
    return (FAIL);

  if (!pdtahelper (IHDR_ID, SFIHDRSIZE, &chunk, &size))
    return (FAIL);
  if (!load_ihdr (chunk.size))
    return (FAIL);

  if (!pdtahelper (IBAG_ID, SFBAGSIZE, &chunk, &size))
    return (FAIL);
  if (!load_ibag (chunk.size))
    return (FAIL);

  if (!pdtahelper (IMOD_ID, SFMODSIZE, &chunk, &size))
    return (FAIL);
  if (!load_imod (chunk.size))
    return (FAIL);

  if (!pdtahelper (IGEN_ID, SFGENSIZE, &chunk, &size))
    return (FAIL);
  if (!load_igen (chunk.size))
    return (FAIL);

  if (!pdtahelper (SHDR_ID, SFSHDRSIZE, &chunk, &size))
    return (FAIL);
  if (!load_shdr (chunk.size))
    return (FAIL);

  return (OK);
}

/* preset header loader */
int SFData::load_phdr (int size)
{
      SFData * sf = this;
      SFData* fd  = this;

  int i, i2;
  SFPreset *p, *pr = 0;	/* ptr to current & previous preset */
  unsigned short zndx, pzndx = 0;

  if (size % SFPHDRSIZE || size == 0)
    return (gerr (ErrCorr, _("Preset header chunk size is invalid")));

  i = size / SFPHDRSIZE - 1;
  if (i == 0)
    {				/* at least one preset + term record */
      FLUID_LOG (FLUID_WARN, _("File contains no presets"));
      FSKIP (SFPHDRSIZE, fd);
      return (OK);
    }

  for (; i > 0; i--)
    {				/* load all preset headers */
      p = FLUID_NEW (SFPreset);
      sf->preset = fluid_list_append (sf->preset, p);
      p->zone = 0;		/* In case of failure, sfont_close can cleanup */
      READSTR (&p->name, fd);	/* possible read failure ^ */
      READW (p->prenum, fd);
      READW (p->bank, fd);
      READW (zndx, fd);
      READD (p->libr, fd);
      READD (p->genre, fd);
      READD (p->morph, fd);

      if (pr)
	{			/* not first preset? */
	  if (zndx < pzndx)
	    return (gerr (ErrCorr, _("Preset header indices not monotonic")));
	  i2 = zndx - pzndx;
	  while (i2--)
	    {
	      pr->zone = fluid_list_prepend (pr->zone, 0);
	    }
	}
      else if (zndx > 0)	/* 1st preset, warn if ofs >0 */
	FLUID_LOG (FLUID_WARN, _("%d preset zones not referenced, discarding"), zndx);
      pr = p;			/* update preset ptr */
      pzndx = zndx;
    }

  FSKIP (24, fd);
  READW (zndx, fd);		/* Read terminal generator index */
  FSKIP (12, fd);

  if (zndx < pzndx)
    return (gerr (ErrCorr, _("Preset header indices not monotonic")));
  i2 = zndx - pzndx;
  while (i2--)
    {
      pr->zone = fluid_list_prepend (pr->zone, 0);
    }

  return (OK);
}

/* preset bag loader */
int SFData::load_pbag (int size)
{
      SFData * sf = this;
      QFile* fd = this;

  fluid_list_t *p, *p2;
  SFZone *z, *pz = 0;
  unsigned short genndx, modndx;
  unsigned short pgenndx = 0, pmodndx = 0;
  unsigned short i;

  if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
    return (gerr (ErrCorr, _("Preset bag chunk size is invalid")));

  p = sf->preset;
  while (p)
    {				/* traverse through presets */
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse preset's zones */
	  if ((size -= SFBAGSIZE) < 0)
	    return (gerr (ErrCorr, _("Preset bag chunk size mismatch")));
	  z = FLUID_NEW (SFZone);
	  p2->data = z;
	  z->gen = 0;	/* Init gen and mod before possible failure, */
	  z->mod = 0;	/* to ensure proper cleanup (sfont_close) */
	  READW (genndx, sf);	/* possible read failure ^ */
	  READW (modndx, sf);
	  z->instsamp = 0;

	  if (pz)
	    {			/* if not first zone */
	      if (genndx < pgenndx)
		return (gerr (ErrCorr,
		    _("Preset bag generator indices not monotonic")));
	      if (modndx < pmodndx)
		return (gerr (ErrCorr,
		    _("Preset bag modulator indices not monotonic")));
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
      p = fluid_list_next (p);
    }

  size -= SFBAGSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset bag chunk size mismatch")));

  READW (genndx, sf);
  READW (modndx, sf);

  if (!pz)
    {
      if (genndx > 0)
	FLUID_LOG (FLUID_WARN, _("No preset generators and terminal index not 0"));
      if (modndx > 0)
	FLUID_LOG (FLUID_WARN, _("No preset modulators and terminal index not 0"));
      return (OK);
    }

  if (genndx < pgenndx)
    return (gerr (ErrCorr, _("Preset bag generator indices not monotonic")));
  if (modndx < pmodndx)
    return (gerr (ErrCorr, _("Preset bag modulator indices not monotonic")));
  i = genndx - pgenndx;
  while (i--)
    pz->gen = fluid_list_prepend (pz->gen, 0);
  i = modndx - pmodndx;
  while (i--)
    pz->mod = fluid_list_prepend (pz->mod, 0);

  return (OK);
}

/* preset modulator loader */
int SFData::load_pmod (int size)
{
      SFData * sf = this;
      QFile* fd = this;

  fluid_list_t *p, *p2, *p3;
  SFMod *m;

  p = sf->preset;
  while (p)
    {				/* traverse through all presets */
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse this preset's zones */
	  p3 = ((SFZone *) (p2->data))->mod;
	  while (p3)
	    {			/* load zone's modulators */
	      if ((size -= SFMODSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset modulator chunk size mismatch")));
	      m = FLUID_NEW (SFMod);
	      p3->data = m;
	      READW (m->src, sf);
	      READW (m->dest, sf);
	      READW (m->amount, sf);
	      READW (m->amtsrc, sf);
	      READW (m->trans, sf);
	      p3 = fluid_list_next (p3);
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  /*
     If there isn't even a terminal record
     Hmmm, the specs say there should be one, but..
   */
  if (size == 0)
    return (OK);

  size -= SFMODSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset modulator chunk size mismatch")));
  FSKIP (SFMODSIZE, sf);	/* terminal mod */

  return (OK);
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

int SFData::load_pgen (int size)
{
      SFData * sf = this;
      QFile* fd = this;

  fluid_list_t *p, *p2, *p3, *dup, **hz = 0;
  SFZone *z;
  SFGen *g;
  SFGenAmount genval;
  unsigned short genid;
  int level, skip, drop, gzone, discarded;

  p = sf->preset;
  while (p)
    {				/* traverse through all presets */
      gzone = FALSE;
      discarded = FALSE;
      p2 = ((SFPreset *) (p->data))->zone;
      if (p2)
	hz = &p2;
      while (p2)
	{			/* traverse preset's zones */
	  level = 0;
	  z = (SFZone *) (p2->data);
	  p3 = z->gen;
	  while (p3)
	    {			/* load zone's generators */
	      dup = 0;
	      skip = FALSE;
	      drop = FALSE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset generator chunk size mismatch")));

	      READW (genid, sf);

	      if (genid == Gen_KeyRange)
		{		/* nothing precedes */
		  if (level == 0)
		    {
		      level = 1;
		      READB (genval.range.lo, sf);
		      READB (genval.range.hi, sf);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_VelRange)
		{		/* only KeyRange precedes */
		  if (level <= 1)
		    {
		      level = 2;
		      READB (genval.range.lo, sf);
		      READB (genval.range.hi, sf);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_Instrument)
		{		/* inst is last gen */
		  level = 3;
		  READW (genval.uword, sf);
		  ((SFZone *) (p2->data))->instsamp = (fluid_list_t*)GINT_TO_POINTER (genval.uword + 1);
		  break;	/* break out of generator loop */
		}
	      else
		{
		  level = 2;
		  if (gen_validp (genid))
		    {		/* generator valid? */
		      READW (genval.sword, sf);
		      dup = gen_inlist (genid, z->gen);
		    }
		  else
		    skip = TRUE;
		}

	      if (!skip)
		{
		  if (!dup)
		    {		/* if gen ! dup alloc new */
		      g = FLUID_NEW (SFGen);
		      p3->data = g;
		      g->id = genid;
		    }
		  else
		    {
		      g = (SFGen *) (dup->data);	/* ptr to orig gen */
		      drop = TRUE;
		    }
		  g->amount = genval;
		}
	      else
		{		/* Skip this generator */
		  discarded = TRUE;
		  drop = TRUE;
		  FSKIPW (sf);
		}

	      if (!drop)
		p3 = fluid_list_next (p3);	/* next gen */
	      else
		SLADVREM (z->gen, p3);	/* drop place holder */

	    }			/* generator loop */

	  if (level == 3)
	    SLADVREM (z->gen, p3);	/* zone has inst? */
	  else
	    {			/* congratulations its a global zone */
	      if (!gzone)
		{		/* Prior global zones? */
		  gzone = TRUE;

		  /* if global zone is not 1st zone, relocate */
		  if (*hz != p2)
		    {
		      void* save = p2->data;
		      FLUID_LOG (FLUID_WARN,
			_("Preset \"%s\": Global zone is not first zone"),
			((SFPreset *) (p->data))->name);
		      SLADVREM (*hz, p2);
		      *hz = fluid_list_prepend (*hz, save);
		      continue;
		    }
		}
	      else
		{		/* previous global zone exists, discard */
		  FLUID_LOG (FLUID_WARN,
		    _("Preset \"%s\": Discarding invalid global zone"),
		    ((SFPreset *) (p->data))->name);
		  sfont_zone_delete (sf, hz, (SFZone *) (p2->data));
		}
	    }

	  while (p3)
	    {			/* Kill any zones following an instrument */
	      discarded = TRUE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Preset generator chunk size mismatch")));
	      FSKIP (SFGENSIZE, sf);
	      SLADVREM (z->gen, p3);
	    }

	  p2 = fluid_list_next (p2);	/* next zone */
	}
      if (discarded)
	FLUID_LOG(FLUID_WARN,
	  _("Preset \"%s\": Some invalid generators were discarded"),
	  ((SFPreset *) (p->data))->name);
      p = fluid_list_next (p);
    }

  /* in case there isn't a terminal record */
  if (size == 0)
    return (OK);

  size -= SFGENSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Preset generator chunk size mismatch")));
  FSKIP (SFGENSIZE, sf);	/* terminal gen */

  return (OK);
}

/* instrument header loader */
int SFData::load_ihdr(int size)
      {
      SFData * sf = this;
      QFile* fd = this;

  int i, i2;
  SFInst *p, *pr = 0;	/* ptr to current & previous instrument */
  unsigned short zndx, pzndx = 0;

  if (size % SFIHDRSIZE || size == 0)	/* chunk size is valid? */
    return (gerr (ErrCorr, _("Instrument header has invalid size")));

  size = size / SFIHDRSIZE - 1;
  if (size == 0)
    {				/* at least one preset + term record */
      FLUID_LOG (FLUID_WARN, _("File contains no instruments"));
      FSKIP (SFIHDRSIZE, sf);
      return (OK);
    }

  for (i = 0; i < size; i++)
    {				/* load all instrument headers */
      p = FLUID_NEW (SFInst);
      sf->inst = fluid_list_append (sf->inst, p);
      p->zone = 0;		/* For proper cleanup if fail (sfont_close) */
      READSTR (&p->name, sf);	/* Possible read failure ^ */
      READW (zndx, sf);

      if (pr)
	{			/* not first instrument? */
	  if (zndx < pzndx)
	    return (gerr (ErrCorr,
		_("Instrument header indices not monotonic")));
	  i2 = zndx - pzndx;
	  while (i2--)
	    pr->zone = fluid_list_prepend (pr->zone, 0);
	}
      else if (zndx > 0)	/* 1st inst, warn if ofs >0 */
	FLUID_LOG (FLUID_WARN, _("%d instrument zones not referenced, discarding"),
	  zndx);
      pzndx = zndx;
      pr = p;			/* update instrument ptr */
    }

  FSKIP (20, sf);
  READW (zndx, sf);

  if (zndx < pzndx)
    return (gerr (ErrCorr, _("Instrument header indices not monotonic")));
  i2 = zndx - pzndx;
  while (i2--)
    pr->zone = fluid_list_prepend (pr->zone, 0);

  return (OK);
}

/* instrument bag loader */
int SFData::load_ibag(int size)
{
      SFData * sf = this;
      QFile* fd = this;

  fluid_list_t *p, *p2;
  SFZone *z, *pz = 0;
  unsigned short genndx, modndx, pgenndx = 0, pmodndx = 0;
  int i;

  if (size % SFBAGSIZE || size == 0)	/* size is multiple of SFBAGSIZE? */
    return (gerr (ErrCorr, _("Instrument bag chunk size is invalid")));

  p = sf->inst;
  while (p)
    {				/* traverse through inst */
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* load this inst's zones */
	  if ((size -= SFBAGSIZE) < 0)
	    return (gerr (ErrCorr, _("Instrument bag chunk size mismatch")));
	  z = FLUID_NEW (SFZone);
	  p2->data = z;
	  z->gen = 0;	/* In case of failure, */
	  z->mod = 0;	/* sfont_close can clean up */
	  READW (genndx, sf);	/* READW = possible read failure */
	  READW (modndx, sf);
	  z->instsamp = 0;

	  if (pz)
	    {			/* if not first zone */
	      if (genndx < pgenndx)
		return (gerr (ErrCorr,
		    _("Instrument generator indices not monotonic")));
	      if (modndx < pmodndx)
		return (gerr (ErrCorr,
		    _("Instrument modulator indices not monotonic")));
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
      p = fluid_list_next (p);
    }

  size -= SFBAGSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Instrument chunk size mismatch")));

  READW (genndx, sf);
  READW (modndx, sf);

  if (!pz)
    {				/* in case that all are no zoners */
      if (genndx > 0)
	FLUID_LOG (FLUID_WARN,
	  _("No instrument generators and terminal index not 0"));
      if (modndx > 0)
	FLUID_LOG (FLUID_WARN,
	  _("No instrument modulators and terminal index not 0"));
      return (OK);
    }

  if (genndx < pgenndx)
    return (gerr (ErrCorr, _("Instrument generator indices not monotonic")));
  if (modndx < pmodndx)
    return (gerr (ErrCorr, _("Instrument modulator indices not monotonic")));
  i = genndx - pgenndx;
  while (i--)
    pz->gen = fluid_list_prepend (pz->gen, 0);
  i = modndx - pmodndx;
  while (i--)
    pz->mod = fluid_list_prepend (pz->mod, 0);

  return (OK);
}

/* instrument modulator loader */
int SFData::load_imod(int size)
{
      SFData * sf = this;
      QFile* fd = this;

  fluid_list_t *p, *p2, *p3;
  SFMod *m;

  p = sf->inst;
  while (p)
    {				/* traverse through all inst */
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* traverse this inst's zones */
	  p3 = ((SFZone *) (p2->data))->mod;
	  while (p3)
	    {			/* load zone's modulators */
	      if ((size -= SFMODSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Instrument modulator chunk size mismatch")));
	      m = FLUID_NEW (SFMod);
	      p3->data = m;
	      READW (m->src, sf);
	      READW (m->dest, sf);
	      READW (m->amount, sf);
	      READW (m->amtsrc, sf);
	      READW (m->trans, sf);
	      p3 = fluid_list_next (p3);
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  /*
     If there isn't even a terminal record
     Hmmm, the specs say there should be one, but..
   */
  if (size == 0)
    return (OK);

  size -= SFMODSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("Instrument modulator chunk size mismatch")));
  FSKIP (SFMODSIZE, sf);	/* terminal mod */

  return (OK);
}

/* load instrument generators (see load_pgen for loading rules) */
int SFData::load_igen (int size)
      {
      SFData * sf = this;
      QFile* fd = this;
  fluid_list_t *p, *p2, *p3, *dup, **hz = 0;
  SFZone *z;
  SFGen *g;
  SFGenAmount genval;
  unsigned short genid;
  int level, skip, drop, gzone, discarded;

  p = sf->inst;
  while (p)
    {				/* traverse through all instruments */
      gzone = FALSE;
      discarded = FALSE;
      p2 = ((SFInst *) (p->data))->zone;
      if (p2)
	hz = &p2;
      while (p2)
	{			/* traverse this instrument's zones */
	  level = 0;
	  z = (SFZone *) (p2->data);
	  p3 = z->gen;
	  while (p3)
	    {			/* load zone's generators */
	      dup = 0;
	      skip = FALSE;
	      drop = FALSE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr, _("IGEN chunk size mismatch")));

	      READW (genid, sf);

	      if (genid == Gen_KeyRange)
		{		/* nothing precedes */
		  if (level == 0)
		    {
		      level = 1;
		      READB (genval.range.lo, sf);
		      READB (genval.range.hi, sf);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_VelRange)
		{		/* only KeyRange precedes */
		  if (level <= 1)
		    {
		      level = 2;
		      READB (genval.range.lo, sf);
		      READB (genval.range.hi, sf);
		    }
		  else
		    skip = TRUE;
		}
	      else if (genid == Gen_SampleId)
		{		/* sample is last gen */
		  level = 3;
		  READW (genval.uword, sf);
		  ((SFZone *) (p2->data))->instsamp = (fluid_list_t*)GINT_TO_POINTER (genval.uword + 1);
		  break;	/* break out of generator loop */
		}
	      else
		{
		  level = 2;
		  if (gen_valid (genid))
		    {		/* gen valid? */
		      READW (genval.sword, sf);
		      dup = gen_inlist (genid, z->gen);
		    }
		  else
		    skip = TRUE;
		}

	      if (!skip)
		{
		  if (!dup)
		    {		/* if gen ! dup alloc new */
		      g = FLUID_NEW (SFGen);
		      p3->data = g;
		      g->id = genid;
		    }
		  else
		    {
		      g = (SFGen *) (dup->data);
		      drop = TRUE;
		    }
		  g->amount = genval;
		}
	      else
		{		/* skip this generator */
		  discarded = TRUE;
		  drop = TRUE;
		  FSKIPW (sf);
		}

	      if (!drop)
		p3 = fluid_list_next (p3);	/* next gen */
	      else
		SLADVREM (z->gen, p3);

	    }			/* generator loop */

	  if (level == 3)
	    SLADVREM (z->gen, p3);	/* zone has sample? */
	  else
	    {			/* its a global zone */
	      if (!gzone)
		{
		  gzone = TRUE;

		  /* if global zone is not 1st zone, relocate */
		  if (*hz != p2)
		    {
		      void* save = p2->data;
		      FLUID_LOG (FLUID_WARN,
			_("Instrument \"%s\": Global zone is not first zone"),
			((SFPreset *) (p->data))->name);
		      SLADVREM (*hz, p2);
		      *hz = fluid_list_prepend (*hz, save);
		      continue;
		    }
		}
	      else
		{		/* previous global zone exists, discard */
		  FLUID_LOG (FLUID_WARN,
		    _("Instrument \"%s\": Discarding invalid global zone"),
		    ((SFInst *) (p->data))->name);
		  sfont_zone_delete (sf, hz, (SFZone *) (p2->data));
		}
	    }

	  while (p3)
	    {			/* Kill any zones following a sample */
	      discarded = TRUE;
	      if ((size -= SFGENSIZE) < 0)
		return (gerr (ErrCorr,
		    _("Instrument generator chunk size mismatch")));
	      FSKIP (SFGENSIZE, sf);
	      SLADVREM (z->gen, p3);
	    }

	  p2 = fluid_list_next (p2);	/* next zone */
	}
      if (discarded)
	FLUID_LOG(FLUID_WARN,
	  _("Instrument \"%s\": Some invalid generators were discarded"),
	  ((SFInst *) (p->data))->name);
      p = fluid_list_next (p);
    }

  /* for those non-terminal record cases, grr! */
  if (size == 0)
    return (OK);

  size -= SFGENSIZE;
  if (size != 0)
    return (gerr (ErrCorr, _("IGEN chunk size mismatch")));
  FSKIP (SFGENSIZE, sf);	/* terminal gen */

  return (OK);
}

/* sample header loader */
int SFData::load_shdr (unsigned int size)
{
      SFData* sf = this;
      QFile* fd = this;

  unsigned int i;
  SFSample *p;

  if (size % SFSHDRSIZE || size == 0)	/* size is multiple of SHDR size? */
    return (gerr (ErrCorr, _("Sample header has invalid size")));

  size = size / SFSHDRSIZE - 1;
  if (size == 0)
    {				/* at least one sample + term record? */
      FLUID_LOG (FLUID_WARN, _("File contains no samples"));
      FSKIP (SFSHDRSIZE, sf);
      return (OK);
    }

  /* load all sample headers */
  for (i = 0; i < size; i++)
    {
      p = FLUID_NEW (SFSample);
      sf->sample = fluid_list_append (sf->sample, p);
      READSTR (&p->name, sf);
      READD (p->start, sf);
      READD (p->end, sf);	/* - end, loopstart and loopend */
      READD (p->loopstart, sf);	/* - will be checked and turned into */
      READD (p->loopend, sf);	/* - offsets in fixup_sample() */
      READD (p->samplerate, sf);
      READB (p->origpitch, sf);
      READB (p->pitchadj, sf);
      FSKIPW (sf);		/* skip sample link */
      READW (p->sampletype, sf);
      p->samfile = 0;
    }

  FSKIP (SFSHDRSIZE, sf);	/* skip terminal shdr */

  return (OK);
}

/* "fixup" (inst # -> inst ptr) instrument references in preset list */
int SFData::fixup_pgen()
      {
      SFData* sf = this;

  fluid_list_t *p, *p2, *p3;
  SFZone *z;
  int i;

  p = sf->preset;
  while (p)
    {
      p2 = ((SFPreset *) (p->data))->zone;
      while (p2)
	{			/* traverse this preset's zones */
	  z = (SFZone *) (p2->data);
	  if ((i = GPOINTER_TO_INT (z->instsamp)))
	    {			/* load instrument # */
	      p3 = fluid_list_nth (sf->inst, i - 1);
	      if (!p3)
		return (gerr (ErrCorr,
		    _("Preset %03d %03d: Invalid instrument reference"),
		    ((SFPreset *) (p->data))->bank,
		    ((SFPreset *) (p->data))->prenum));
	      z->instsamp = p3;
	    }
	  else
	    z->instsamp = 0;
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  return (OK);
}

/* "fixup" (sample # -> sample ptr) sample references in instrument list */
int SFData::fixup_igen()
{
      SFData* sf = this;
  fluid_list_t *p, *p2, *p3;
  SFZone *z;
  int i;

  p = sf->inst;
  while (p)
    {
      p2 = ((SFInst *) (p->data))->zone;
      while (p2)
	{			/* traverse instrument's zones */
	  z = (SFZone *) (p2->data);
	  if ((i = GPOINTER_TO_INT (z->instsamp)))
	    {			/* load sample # */
	      p3 = fluid_list_nth (sf->sample, i - 1);
	      if (!p3)
		return (gerr (ErrCorr,
		    _("Instrument \"%s\": Invalid sample reference"),
		    ((SFInst *) (p->data))->name));
	      z->instsamp = p3;
	    }
	  p2 = fluid_list_next (p2);
	}
      p = fluid_list_next (p);
    }

  return (OK);
}

/* convert sample end, loopstart and loopend to offsets and check if valid */
int SFData::fixup_sample()
      {
      SFData* sf = this;

      fluid_list_t *p;
      SFSample *sam;

      p = sf->sample;
      while (p)
    {
      sam = (SFSample *) (p->data);

      /* if sample is not a ROM sample and end is over the sample data chunk
         or sam start is greater than 4 less than the end (at least 4 samples) */
      if ((!(sam->sampletype & FLUID_SAMPLETYPE_ROM)
	  && sam->end > sdtachunk_size) || sam->start > (sam->end - 4))
	{
	  FLUID_LOG (FLUID_WARN, _("Sample '%s' start/end file positions are invalid,"
	      " disabling and will not be saved"), sam->name);

	  /* disable sample by setting all sample markers to 0 */
	  sam->start = sam->end = sam->loopstart = sam->loopend = 0;

	  return (OK);
	}
      else if (sam->loopend > sam->end || sam->loopstart >= sam->loopend
	|| sam->loopstart <= sam->start)
	{			/* loop is fowled?? (cluck cluck :) */
	  /* can pad loop by 8 samples and ensure at least 4 for loop (2*8+4) */
	  if ((sam->end - sam->start) >= 20)
	    {
	      sam->loopstart = sam->start + 8;
	      sam->loopend = sam->end - 8;
	    }
	  else
	    {			/* loop is fowled, sample is tiny (can't pad 8 samples) */
	      sam->loopstart = sam->start + 1;
	      sam->loopend = sam->end - 1;
	    }
	}

      /* convert sample end, loopstart, loopend to offsets from sam->start */
      sam->end -= sam->start + 1;	/* marks last sample, contrary to SF spec. */
      sam->loopstart -= sam->start;
      sam->loopend -= sam->start;

      p = fluid_list_next (p);
    }

  return (OK);
}

/* optimum chunk area sizes (could be more optimum) */
#define PRESET_CHUNK_OPTIMUM_AREA	256
#define INST_CHUNK_OPTIMUM_AREA		256
#define SAMPLE_CHUNK_OPTIMUM_AREA	256
#define ZONE_CHUNK_OPTIMUM_AREA		256
#define MOD_CHUNK_OPTIMUM_AREA		256
#define GEN_CHUNK_OPTIMUM_AREA		256

unsigned short badgen[] = { Gen_Unused1, Gen_Unused2, Gen_Unused3, Gen_Unused4,
  Gen_Reserved1, Gen_Reserved2, Gen_Reserved3, 0
};

unsigned short badpgen[] = { Gen_StartAddrOfs, Gen_EndAddrOfs, Gen_StartLoopAddrOfs,
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

      p = preset;
      while (p)  {      // loop over presets
            fluid_list_t* p2 = ((SFPreset *) (p->data))->zone;
            while (p2) { // loop over preset's zones
	            sfont_free_zone ((SFZone*)p2->data);
	            p2 = fluid_list_next (p2);
                  }			/* free preset's zone list */
            delete_fluid_list (((SFPreset *) (p->data))->zone);
            FLUID_FREE (p->data);	/* free preset chunk */
            p = fluid_list_next (p);
            }
      delete_fluid_list (preset);

      p = inst;
      while (p) {				/* loop over instruments */
            fluid_list_t* p2 = ((SFInst *) (p->data))->zone;
            while (p2) {			/* loop over inst's zones */
                  sfont_free_zone ((SFZone*)p2->data);
                  p2 = fluid_list_next (p2);
                  }			/* free inst's zone list */
            delete_fluid_list (((SFInst *) (p->data))->zone);
            FLUID_FREE (p->data);
            p = fluid_list_next (p);
            }
      delete_fluid_list (inst);

      p = sample;
      while (p) {
            FLUID_FREE (p->data);
            p = fluid_list_next (p);
            }
      delete_fluid_list(sample);
      }

/* free all elements of a zone (Preset or Instrument) */
void sfont_free_zone (SFZone * zone)
{
  fluid_list_t *p;

  if (!zone)
    return;

  p = zone->gen;
  while (p)
    {				/* Free gen chunks for this zone */
      if (p->data)
	FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (zone->gen);	/* free genlist */

  p = zone->mod;
  while (p)
    {				/* Free mod chunks for this zone */
      if (p->data)
	FLUID_FREE (p->data);
      p = fluid_list_next (p);
    }
  delete_fluid_list (zone->mod);	/* free modlist */

  FLUID_FREE (zone);	/* free zone chunk */
}

/* preset sort function, first by bank, then by preset # */
int
sfont_preset_compare_func (void* a, void* b)
{
  int aval, bval;

  aval = (int) (((SFPreset *) a)->bank) << 16 | ((SFPreset *) a)->prenum;
  bval = (int) (((SFPreset *) b)->bank) << 16 | ((SFPreset *) b)->prenum;

  return (aval - bval);
}

/* delete zone from zone list */
void
sfont_zone_delete (SFData * sf, fluid_list_t ** zlist, SFZone * zone)
{
  *zlist = fluid_list_remove (*zlist, (void*) zone);
  sfont_free_zone (zone);
}

/* Find generator in gen list */
fluid_list_t * gen_inlist (int gen, fluid_list_t * genlist)
{				/* is generator in gen list? */
  fluid_list_t *p;

  p = genlist;
  while (p)
    {
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

/*================================util.c===========================*/

/* Logging function, returns FAIL to use as a return value in calling funcs */
int gerr (int ev, char * fmt, ...)
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

bool SFData::safe_fread(void *buf, int count)
      {
      if (read((char*)buf, count) != count) {
            if (atEnd())
                  gerr (ErrEof, _("EOF while attemping to read %d bytes"), count);
            else
                  FLUID_LOG (FLUID_ERR, _("File read failed"));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   safe_fseek
//---------------------------------------------------------

bool SFData::safe_fseek(long ofs, int whence)
      {
      qint64 newpos = ofs;
      if (whence == SEEK_CUR)
            newpos += pos();
      else if (whence == SEEK_SET)
            ;
      else
            abort();
      if (seek(newpos) == -1) {
            FLUID_LOG (FLUID_ERR, _("File seek failed with offset = %ld and whence = %d"), ofs, whence);
            return false;
            }
      return true;
      }

