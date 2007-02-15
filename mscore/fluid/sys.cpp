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


#include "sys.h"

static char fluid_errbuf[512];  /* buffer for error message */

static fluid_log_function_t fluid_log_function[LAST_LOG_LEVEL];
static void* fluid_log_user_data[LAST_LOG_LEVEL];
static int fluid_log_initialized = 0;

static char* fluid_libname = "fluidsynth";


void fluid_sys_config()
{
  fluid_log_config();
  fluid_time_config();
}


unsigned int fluid_debug_flags = 0;

/**
 * Installs a new log function for a specified log level.
 * @param level Log level to install handler for.
 * @param fun Callback function handler to call for logged messages
 * @param data User supplied data pointer to pass to log function
 * @return The previously installed function.
 */
fluid_log_function_t
fluid_set_log_function(int level, fluid_log_function_t fun, void* data)
{
  fluid_log_function_t old = 0;

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    old = fluid_log_function[level];
    fluid_log_function[level] = fun;
    fluid_log_user_data[level] = data;
  }
  return old;
}

/**
 * Default log function which prints to the stderr.
 * @param level Log level
 * @param message Log message
 * @param data User supplied data (not used)
 */
void
fluid_default_log_function(int level, char* message, void* data)
{
  FILE* out;

#if defined(WIN32)
  out = stdout;
#else
  out = stderr;
#endif

  if (fluid_log_initialized == 0) {
    fluid_log_config();
  }

  switch (level) {
  case FLUID_PANIC:
    FLUID_FPRINTF(out, "%s: panic: %s\n", fluid_libname, message);
    break;
  case FLUID_ERR:
    FLUID_FPRINTF(out, "%s: error: %s\n", fluid_libname, message);
    break;
  case FLUID_WARN:
    FLUID_FPRINTF(out, "%s: warning: %s\n", fluid_libname, message);
    break;
  case FLUID_INFO:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  case FLUID_DBG:
    break;
  default:
    FLUID_FPRINTF(out, "%s: %s\n", fluid_libname, message);
    break;
  }
  fflush(out);
}

/*
 * fluid_init_log
 */
void
fluid_log_config(void)
{
  if (fluid_log_initialized == 0) {

    fluid_log_initialized = 1;

    if (fluid_log_function[FLUID_PANIC] == 0) {
      fluid_set_log_function(FLUID_PANIC, fluid_default_log_function, 0);
    }

    if (fluid_log_function[FLUID_ERR] == 0) {
      fluid_set_log_function(FLUID_ERR, fluid_default_log_function, 0);
    }

    if (fluid_log_function[FLUID_WARN] == 0) {
      fluid_set_log_function(FLUID_WARN, fluid_default_log_function, 0);
    }

    if (fluid_log_function[FLUID_INFO] == 0) {
      fluid_set_log_function(FLUID_INFO, fluid_default_log_function, 0);
    }

    if (fluid_log_function[FLUID_DBG] == 0) {
      fluid_set_log_function(FLUID_DBG, fluid_default_log_function, 0);
    }
  }
}

/**
 * Print a message to the log.
 * @param level Log level (#fluid_log_level).
 * @param fmt Printf style format string for log message
 * @param ... Arguments for printf 'fmt' message string
 * @return Always returns -1
 */
int
fluid_log(int level, char* fmt, ...)
{
  fluid_log_function_t fun = 0;

  va_list args;
  va_start (args, fmt);
  vsnprintf(fluid_errbuf, sizeof (fluid_errbuf), fmt, args);
  va_end (args);

  if ((level >= 0) && (level < LAST_LOG_LEVEL)) {
    fun = fluid_log_function[level];
    if (fun != 0) {
      (*fun)(level, fluid_errbuf, fluid_log_user_data[level]);
    }
  }
  return FLUID_FAILED;
}

/**
 * An improved strtok, still trashes the input string, but is portable and
 * thread safe.  Also skips token chars at beginning of token string and never
 * returns an empty token (will return 0 if source ends in token chars though).
 * NOTE: NOT part of public API
 * @internal
 * @param str Pointer to a string pointer of source to tokenize.  Pointer gets
 *   updated on each invocation to point to beginning of next token.  Note that
 *   token char get's overwritten with a 0 byte.  String pointer is set to 0
 *   when final token is returned.
 * @param delim String of delimiter chars.
 * @return Pointer to the next token or 0 if no more tokens.
 */
char *fluid_strtok (char **str, char *delim)
{
  char *s, *d, *token;
  char c;

  if (str == 0 || delim == 0 || !*delim)
  {
    FLUID_LOG(FLUID_ERR, "Null pointer");
    return 0;
  }

  s = *str;
  if (!s) return 0;	/* str points to a 0 pointer? (tokenize already ended) */

  /* skip delimiter chars at beginning of token */
  do
  {
    c = *s;
    if (!c)	/* end of source string? */
    {
      *str = 0;
      return 0;
    }

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	s++;		/* advance to next source char */
	break;
      }
    }
  } while (*d);		/* while token char match */

  token = s;		/* start of token found */

  /* search for next token char or end of source string */
  for (s = s+1; *s; s++)
  {
    c = *s;

    for (d = delim; *d; d++)	/* is source char a token char? */
    {
      if (c == *d)	/* token char match? */
      {
	*s = '\0';	/* overwrite token char with zero byte to terminate token */
	*str = s+1;	/* update str to point to beginning of next token */
	return token;
      }
    }
  }

  /* we get here only if source string ended */
  *str = 0;
  return token;
}

/*
 * fluid_error
 */
char*
fluid_error()
{
  return fluid_errbuf;
}


/*
 *
 *  fluid_is_midifile
 */
int
fluid_is_midifile(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == 0) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "MThd", 4) == 0;
}

/*
 *  fluid_is_soundfont
 *
 */
int
fluid_is_soundfont(char* filename)
{
  FILE* fp = fopen(filename, "rb");
  char id[4];

  if (fp == 0) {
    return 0;
  }
  if (fread((void*) id, 1, 4, fp) != 4) {
    fclose(fp);
    return 0;
  }
  fclose(fp);

  return strncmp(id, "RIFF", 4) == 0;
}

#if defined(WIN32)

/*=============================================================*/
/*                                                             */
/*                           Win32                             */
/*                                                             */
/*=============================================================*/

/***************************************************************
 *
 *               Timer
 *
 */

struct _fluid_timer_t
{
  long msec;
  fluid_timer_callback_t callback;
  void* data;
  HANDLE thread;
  DWORD thread_id;
  int cont;
  int auto_destroy;
};

static int fluid_timer_count = 0;
DWORD WINAPI fluid_timer_run(LPVOID data);

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

  timer->cont = 1;
  timer->msec = msec;
  timer->callback = callback;
  timer->data = data;
  timer->thread = 0;
  timer->auto_destroy = auto_destroy;

  if (new_thread) {
    timer->thread = CreateThread(0, 0, fluid_timer_run, (LPVOID) timer, 0, &timer->thread_id);
    if (timer->thread == 0) {
      FLUID_LOG(FLUID_ERR, "Couldn't create timer thread");
      FLUID_FREE(timer);
      return 0;
    }
    SetThreadPriority(timer->thread, THREAD_PRIORITY_TIME_CRITICAL);
  } else {
    fluid_timer_run((LPVOID) timer);
  }
  return timer;
}

DWORD WINAPI
fluid_timer_run(LPVOID data)
{
  int count = 0;
  int cont = 1;
  long start;
  long delay;
  fluid_timer_t* timer;
  timer = (fluid_timer_t*) data;

  if ((timer == 0) || (timer->callback == 0)) {
    return 0;
  }

  SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);

  /* keep track of the start time for absolute positioning */
  start = fluid_curtime();

  while (cont) {

    /* do whatever we have to do */
    cont = (*timer->callback)(timer->data, fluid_curtime() - start);

    count++;

    /* to avoid incremental time errors, I calculate the delay between
       two callbacks bringing in the "absolute" time (count *
       timer->msec) */
    delay = (count * timer->msec) - (fluid_curtime() - start);
    if (delay > 0) {
      Sleep(delay);
    }

    cont &= timer->cont;
  }

  FLUID_LOG(FLUID_DBG, "Timer thread finished");

  if (timer->auto_destroy) {
    FLUID_FREE(timer);
  }

  ExitThread(0);
  return 0;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
  timer->cont = 0;
  fluid_timer_join(timer);
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
  DWORD wait_result;
  if (timer->thread == 0) {
    return FLUID_OK;
  }
  wait_result = WaitForSingleObject(timer->thread, INFINITE);
  return (wait_result == WAIT_OBJECT_0)? FLUID_OK : FLUID_FAILED;
}


/***************************************************************
 *
 *               Time
 */

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

static double fluid_cpu_frequency = -1.0;

void fluid_time_config(void)
{
  if (fluid_cpu_frequency < 0.0) {
    fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
  }
}

double fluid_utime(void)
{
  return (rdtsc() / fluid_cpu_frequency);
}

double rdtsc(void)
{
  LARGE_INTEGER t;
  QueryPerformanceCounter(&t);
  return (double) t.QuadPart;
}

double fluid_estimate_cpu_frequency(void)
{
#if 0
  LONGLONG start, stop, ticks;
  unsigned int before, after, delta;
  double freq;

  start = rdtsc();
  stop = start;
  before = fluid_curtime();
  after = before;

  while (1) {
    if (after - before > 1000) {
	break;
    }
    after = fluid_curtime();
    stop = rdtsc();
  }

  delta = after - before;
  ticks = stop - start;

  freq = 1000 * ticks / delta;

  return freq;

#else
  unsigned int before, after;
  LARGE_INTEGER start, stop;

  before = fluid_curtime();
  QueryPerformanceCounter(&start);

  Sleep(1000);

  after = fluid_curtime();
  QueryPerformanceCounter(&stop);

  return (double) 1000 * (stop.QuadPart - start.QuadPart) / (after - before);
#endif
}



#elif defined(MACOS9)
/*=============================================================*/
/*                                                             */
/*                           MacOS 9                           */
/*                                                             */
/*=============================================================*/


/***************************************************************
 *
 *               Timer
 */

struct _fluid_timer_t
{
	TMTask myTmTask;
  long msec;
  unsigned int start;
  unsigned int count;
  int isInstalled;
  fluid_timer_callback_t callback;
  void* data;
  int auto_destroy;
};

static TimerUPP	myTimerUPP;

void
_timerCallback(fluid_timer_t *timer)
{
	int cont;
  cont = (*timer->callback)(timer->data, fluid_curtime() - timer->start);
  if (cont) {
  	PrimeTime((QElemPtr)timer, timer->msec);
	} else {
		timer->isInstalled = 0;
	}
  timer->count++;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }

	if (!myTimerUPP)
		myTimerUPP = NewTimerProc(_timerCallback);

  /* setup tmtask */
	timer->myTmTask.tmAddr = myTimerUPP;
	timer->myTmTask.qLink = 0;
	timer->myTmTask.qType = 0;
	timer->myTmTask.tmCount = 0L;
	timer->myTmTask.tmWakeUp = 0L;
	timer->myTmTask.tmReserved = 0L;

  timer->callback = callback;

  timer->msec = msec;
  timer->data = data;
  timer->start = fluid_curtime();
  timer->isInstalled = 1;
  timer->count = 0;
  timer->auto_destroy = auto_destroy;

  InsXTime((QElemPtr)timer);
  PrimeTime((QElemPtr)timer, msec);

  return timer;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		RmvTime((QElemPtr)timer);
	}
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
	if (timer->isInstalled) {
		int count = timer->count;
		/* wait until count has incremented */
		while (count == timer->count) {}
	}
  return FLUID_OK;
}

/***************************************************************
 *
 *               Time
 */
#define kTwoPower32 (4294967296.0)      /* 2^32 */

void fluid_time_config(void)
{
}

unsigned int fluid_curtime()
{
	/* could be optimized by not going though a double */
	UnsignedWide    uS;
	double mSf;
	unsigned int ms;

	Microseconds(&uS);

  mSf = ((((double) uS.hi) * kTwoPower32) + uS.lo)/1000.0f;

  ms = mSf;

  return (ms);
}



#else

/*=============================================================*/
/*                                                             */
/*                           POSIX                             */
/*                                                             */
/*=============================================================*/


/***************************************************************
 *
 *               Timer
 */

struct _fluid_timer_t
{
  long msec;
  fluid_timer_callback_t callback;
  void* data;
  pthread_t thread;
  int cont;
  int auto_destroy;
};

void*
fluid_timer_start(void *data)
{
  int count = 0;
  int cont = 1;
  long start;
  long delay;
  fluid_timer_t* timer;
  timer = (fluid_timer_t*) data;

  /* keep track of the start time for absolute positioning */
  start = fluid_curtime();

  while (cont) {

    /* do whatever we have to do */
    cont = (*timer->callback)(timer->data, fluid_curtime() - start);

    count++;

    /* to avoid incremental time errors, calculate the delay between
       two callbacks bringing in the "absolute" time (count *
       timer->msec) */
    delay = (count * timer->msec) - (fluid_curtime() - start);
    if (delay > 0) {
      usleep(delay * 1000);
    }

    cont &= timer->cont;
  }

  FLUID_LOG(FLUID_DBG, "Timer thread finished");
  if (timer->thread != 0) {
    pthread_exit(0);
  }

  if (timer->auto_destroy) {
    FLUID_FREE(timer);
  }

  return 0;
}

fluid_timer_t*
new_fluid_timer(int msec, fluid_timer_callback_t callback, void* data,
	       int new_thread, int auto_destroy)
{
  pthread_attr_t *attr = 0;
  pthread_attr_t rt_attr;
  int sched = SCHED_FIFO;
  struct sched_param priority;
  int err;

  fluid_timer_t* timer = FLUID_NEW(fluid_timer_t);
  if (timer == 0) {
    FLUID_LOG(FLUID_ERR, "Out of memory");
    return 0;
  }
  timer->msec = msec;
  timer->callback = callback;
  timer->data = data;
  timer->cont = 1;
  timer->thread = 0;
  timer->auto_destroy = auto_destroy;

  err = pthread_attr_init(&rt_attr);
  if (err == 0) {
	  err = pthread_attr_setschedpolicy(&rt_attr, SCHED_FIFO);
	  if (err == 0) {
		  priority.sched_priority = 10;
		  err = pthread_attr_setschedparam(&rt_attr, &priority);
		  if (err == 0) {
			  attr = &rt_attr;
		  }
	  }
  }

  if (new_thread) {
	  err = pthread_create(&timer->thread, attr, fluid_timer_start, (void*) timer);
	  if (err == 0) {
		  FLUID_LOG(FLUID_DBG, "The timer thread was created with real-time priority");
	  } else {
		  /* Create the thread with default attributes */
		  err = pthread_create(&timer->thread, 0, fluid_timer_start, (void*) timer);
		  if (err != 0) {
			  FLUID_LOG(FLUID_ERR, "Failed to create the timer thread");
			  FLUID_FREE(timer);
			  return 0;
		  } else {
			  FLUID_LOG(FLUID_DBG, "The timer thread does not have real-time priority");
		  }
	  }
  } else {
    fluid_timer_start((void*) timer);
  }
  return timer;
}

int
delete_fluid_timer(fluid_timer_t* timer)
{
  timer->cont = 0;
  fluid_timer_join(timer);
  FLUID_LOG(FLUID_DBG, "Joined player thread");
  FLUID_FREE(timer);
  return FLUID_OK;
}

int
fluid_timer_join(fluid_timer_t* timer)
{
  int err = 0;

  if (timer->thread != 0) {
    err = pthread_join(timer->thread, 0);
  }
  FLUID_LOG(FLUID_DBG, "Joined player thread");
  return (err == 0)? FLUID_OK : FLUID_FAILED;
}


/***************************************************************
 *
 *               Time
 */

static double fluid_cpu_frequency = -1.0;

double rdtsc(void);
double fluid_estimate_cpu_frequency(void);

void fluid_time_config(void)
{
  if (fluid_cpu_frequency < 0.0) {
    fluid_cpu_frequency = fluid_estimate_cpu_frequency() / 1000000.0;
  }
}

unsigned int fluid_curtime()
{
  struct timeval now;
  gettimeofday(&now, 0);
  return now.tv_sec * 1000 + now.tv_usec / 1000;
}

double fluid_utime(void)
{
  return (rdtsc() / fluid_cpu_frequency);
}

#if !defined(__i386__)

double rdtsc(void)
{
  return 0.0;
}

double fluid_estimate_cpu_frequency(void)
{
  return 1.0;
}

#else

double rdtsc(void)
{
  unsigned int a, b;

  __asm__ ("rdtsc" : "=a" (a), "=d" (b));
  return (double)b * (double)0x10000 * (double)0x10000 + a;
}

double fluid_estimate_cpu_frequency(void)
{
  double start, stop;
  unsigned int a0, b0, a1, b1;
  unsigned int before, after;

  before = fluid_curtime();
  __asm__ ("rdtsc" : "=a" (a0), "=d" (b0));

  sleep(1);

  after = fluid_curtime();
  __asm__ ("rdtsc" : "=a" (a1), "=d" (b1));


  start = (double)b0 * (double)0x10000 * (double)0x10000 + a0;
  stop = (double)b1 * (double)0x10000 * (double)0x10000 + a1;

  return 1000 * (stop - start) / (after - before);
}
#endif


#if 0

/***************************************************************
 *
 *               Floating point exceptions
 *
 *  The floating point exception functions were taken from Ircam's
 *  jMax source code. http://www.ircam.fr/jmax
 *
 *  FIXME: check in config for i386 machine
 *
 *  Currently not used. I leave the code here in case we want to pick
 *  this up again some time later.
 */

/* Exception flags */
#define _FPU_STATUS_IE    0x001  /* Invalid Operation */
#define _FPU_STATUS_DE    0x002  /* Denormalized Operand */
#define _FPU_STATUS_ZE    0x004  /* Zero Divide */
#define _FPU_STATUS_OE    0x008  /* Overflow */
#define _FPU_STATUS_UE    0x010  /* Underflow */
#define _FPU_STATUS_PE    0x020  /* Precision */
#define _FPU_STATUS_SF    0x040  /* Stack Fault */
#define _FPU_STATUS_ES    0x080  /* Error Summary Status */

/* Macros for accessing the FPU status word.  */

/* get the FPU status */
#define _FPU_GET_SW(sw) __asm__ ("fnstsw %0" : "=m" (*&sw))

/* clear the FPU status */
#define _FPU_CLR_SW() __asm__ ("fnclex" : : )

/* Purpose:
 * Checks, if the floating point unit has produced an exception in the meantime.
 */
unsigned int fluid_check_fpe_i386(char* explanation)
{
  unsigned int s;

  _FPU_GET_SW(s);
  _FPU_CLR_SW();

  if ((s & _FPU_STATUS_IE)
      || (s & _FPU_STATUS_DE)
      || (s & _FPU_STATUS_ZE)
      || (s & _FPU_STATUS_OE)
      || (s & _FPU_STATUS_UE)) {
      FLUID_LOG(FLUID_WARN, "FPE exception (before or in %s): %s%s%s%s%s", explanation,
	       (s & _FPU_STATUS_IE) ? "Invalid operation " : "",
	       (s & _FPU_STATUS_DE) ? "Denormal number " : "",
	       (s & _FPU_STATUS_ZE) ? "Zero divide " : "",
	       (s & _FPU_STATUS_OE) ? "Overflow " : "",
	       (s & _FPU_STATUS_UE) ? "Underflow " : "");
  }

  return s;
}

#endif


#endif


