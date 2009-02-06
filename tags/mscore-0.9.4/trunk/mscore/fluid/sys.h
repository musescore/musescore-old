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


/**

   This header contains a bunch of (mostly) system and machine
   dependent functions:

   - timers
   - current time in milliseconds and microseconds
   - debug logging
   - profiling
   - memory locking
   - checking for floating point exceptions

 */

#ifndef _FLUID_SYS_H
#define _FLUID_SYS_H

#include "priv.h"


void fluid_sys_config(void);
void fluid_log_config(void);
void fluid_time_config(void);


/*
 * Utility functions
 */
char *fluid_strtok (char **str, const char *delim);


/**

  Additional debugging system, separate from the log system. This
  allows to print selected debug messages of a specific subsystem.

 */

extern unsigned int fluid_debug_flags;

#if DEBUG

enum fluid_debug_level {
  FLUID_DBG_DRIVER = 1
};

int fluid_debug(int level, char * fmt, ...);

#else
#define fluid_debug
#endif


/** fluid_curtime() returns the current time in milliseconds. This time
    should only be used in relative time measurements.  */

/** fluid_utime() returns the time in micro seconds. this time should
    only be used to measure duration (relative times). */

#if defined(WIN32)
#define fluid_curtime()   GetTickCount()

double fluid_utime(void);

#elif defined(MACOS9)
#include <OSUtils.h>
#include <Timer.h>

unsigned int fluid_curtime();
#define fluid_utime()  0.0

#else

unsigned int fluid_curtime(void);
double fluid_utime(void);

#endif



/**
    Timers

 */

/* if the callback function returns 1 the timer will continue; if it
   returns 0 it will stop */
typedef int (*fluid_timer_callback_t)(void* data, unsigned int msec);

typedef struct _fluid_timer_t fluid_timer_t;

fluid_timer_t* new_fluid_timer(int msec, fluid_timer_callback_t callback,
					    void* data, int new_thread, int auto_destroy);

int delete_fluid_timer(fluid_timer_t* timer);
int fluid_timer_join(fluid_timer_t* timer);
int fluid_timer_stop(fluid_timer_t* timer);

/**

    Profiling
 */


/**
    Profile numbers. List all the pieces of code you want to profile
    here. Be sure to add an entry in the fluid_profile_data table in
    fluid_sys.c
*/
enum {
  FLUID_PROF_WRITE_S16,
  FLUID_PROF_ONE_BLOCK,
  FLUID_PROF_ONE_BLOCK_CLEAR,
  FLUID_PROF_ONE_BLOCK_VOICE,
  FLUID_PROF_ONE_BLOCK_VOICES,
  FLUID_PROF_ONE_BLOCK_REVERB,
  FLUID_PROF_ONE_BLOCK_CHORUS,
  FLUID_PROF_VOICE_NOTE,
  FLUID_PROF_VOICE_RELEASE,
  FLUID_PROF_LAST
};


/* No profiling */
#define fluid_profiling_print()
#define fluid_profile_ref()  0
#define fluid_profile(_num,_ref)


/**

    Memory locking

    Memory locking is used to avoid swapping of the large block of
    sample data.
 */

#if HAVE_SYS_MMAN_H
#define fluid_mlock(_p,_n)      mlock(_p, _n)
#define fluid_munlock(_p,_n)    munlock(_p,_n)
#else
#define fluid_mlock(_p,_n)      0
#define fluid_munlock(_p,_n)
#endif


/**

    Floating point exceptions

    fluid_check_fpe() checks for "unnormalized numbers" and other
    exceptions of the floating point processsor.
*/
#if 0
/* Enable FPE exception check */
#define fluid_check_fpe(expl) fluid_check_fpe_i386(expl)
#else
/* Disable FPE exception check */
#define fluid_check_fpe(expl)
#endif

unsigned int fluid_check_fpe_i386(char * explanation_in_case_of_fpe);

#endif /* _FLUID_SYS_H */
