#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

namespace FluidS {

/***************************************************************
 *
 *         BASIC TYPES
 */

typedef float fluid_real_t;


enum fluid_status {
      FLUID_OK = 0,
      FLUID_FAILED = -1
      };

typedef int32_t  sint32;
typedef uint32_t uint32;

/***************************************************************
 *
 *                      CONSTANTS
 */

#define FLUID_BUFSIZE                256  // 64

#ifndef PI
#define PI                          3.141592654
#endif

/***************************************************************
 *
 *                      SYSTEM INTERFACE
 */
typedef FILE*  fluid_file;

#define FLUID_MALLOC(_n)             malloc(_n)
#define FLUID_REALLOC(_p,_n)         realloc(_p,_n)
#define FLUID_NEW(_t)                (_t*)malloc(sizeof(_t))
#define FLUID_ARRAY(_t,_n)           (_t*)malloc((_n)*sizeof(_t))
#define FLUID_FREE(_p)               free(_p)
#define FLUID_FOPEN(_f,_m)           fopen(_f,_m)
#define FLUID_FCLOSE(_f)             fclose(_f)
#define FLUID_FREAD(_p,_s,_n,_f)     fread(_p,_s,_n,_f)
#define FLUID_FSEEK(_f,_n,_set)      fseek(_f,_n,_set)
#define FLUID_MEMCPY(_dst,_src,_n)   memcpy(_dst,_src,_n)
#define FLUID_MEMSET(_s,_c,_n)       memset(_s,_c,_n)
#define FLUID_STRLEN(_s)             strlen(_s)
#define FLUID_STRCMP(_s,_t)          strcmp(_s,_t)
#define FLUID_STRNCMP(_s,_t,_n)      strncmp(_s,_t,_n)
#define FLUID_STRCPY(_dst,_src)      strcpy(_dst,_src)
#define FLUID_STRCHR(_s,_c)          strchr(_s,_c)

#define FLUID_STRDUP(s)              strdup(s)
#define FLUID_SPRINTF                sprintf
#define FLUID_FPRINTF                fprintf

#define fluid_clip(_val, _min, _max) \
{ (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

/* Purpose:
 * Some commands (SSE extensions on Pentium) need aligned data(
 * The address must be ...xxx0.
 * Take a pointer, and round it up to the next suitable address.
 * Obviously, one has to allocate 15 bytes of additional memory.
 * As soon as proper alignment is supported by the compiler, this
 * can be removed.
 */
#define FLUID_ALIGN16BYTE(ptr) ptr

#define FLUID_LOG                    fluid_log

#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

extern char* fluid_error();
}


#endif /* _FLUIDSYNTH_PRIV_H */
