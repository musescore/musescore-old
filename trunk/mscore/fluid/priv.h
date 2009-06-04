#ifndef _FLUIDSYNTH_PRIV_H
#define _FLUIDSYNTH_PRIV_H

namespace FluidS {

enum fluid_status {
      FLUID_OK     = 0,
      FLUID_FAILED = -1
      };

typedef float    fluid_real_t;
typedef int32_t  sint32;
typedef uint32_t uint32;

#define FLUID_BUFSIZE                256  // 64

#define FLUID_NEW(_t)                (_t*)malloc(sizeof(_t))
#define FLUID_ARRAY(_t,_n)           (_t*)malloc((_n)*sizeof(_t))

#define fluid_clip(_val, _min, _max) \
{ (_val) = ((_val) < (_min))? (_min) : (((_val) > (_max))? (_max) : (_val)); }

#define FLUID_LOG                    fluid_log

extern char* fluid_error();
}


#endif /* _FLUIDSYNTH_PRIV_H */
