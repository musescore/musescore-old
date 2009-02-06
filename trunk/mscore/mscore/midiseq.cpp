//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//=============================================================================

#include <poll.h>

#include "config.h"
#include "midiseq.h"
#include "rtctimer.h"
#include "preferences.h"
#include "mididriver.h"
#include "utils.h"
#include "seq.h"

//---------------------------------------------------------
//   MidiSeq
//---------------------------------------------------------

MidiSeq::MidiSeq(const char* name)
   : Thread(name)
      {
      timer = 0;
      }

//---------------------------------------------------------
//   threadStart
//    called from loop()
//---------------------------------------------------------

void MidiSeq::threadStart(void*)
      {
      struct sched_param rt_param;
      memset(&rt_param, 0, sizeof(rt_param));

      int prio_min = sched_get_priority_min(SCHED_FIFO);
      int prio_max = sched_get_priority_max(SCHED_FIFO);
	int prio     = realTimePriority;
      if (prio < prio_min)
            prio = prio_min;
      else if (prio > prio_max)
            prio = prio_max;
      rt_param.sched_priority = prio;

      pthread_t tid = pthread_self();
      int rv = pthread_setschedparam(tid, SCHED_FIFO, &rt_param);
      if (rv != 0)
            perror("set realtime scheduler");

      int policy;
      if ( (policy = sched_getscheduler (0)) < 0) {
            printf("cannot get current client scheduler for midi thread: %s!\n", strerror(errno));
            }
      else {
            if (policy != SCHED_FIFO)
                  printf("midi thread %d _NOT_ running SCHED_FIFO\n", getpid());
            else if (debugMode) {
            	struct sched_param rt_param;
            	memset(&rt_param, 0, sizeof(sched_param));
            	int type;
            	int rv = pthread_getschedparam(pthread_self(), &type, &rt_param);
            	if (rv == -1)
                  	perror("get scheduler parameter");
                  printf("midiseq thread running SCHED_FIFO priority %d\n",
                     rt_param.sched_priority);
                  }
            }
      if (!initRealtimeTimer())
            printf("RTC init failed\n");
      updatePollFd();
      }

//---------------------------------------------------------
//   updatePollFd
//---------------------------------------------------------

void MidiSeq::updatePollFd()
      {
      if (!isRunning())
            return;
      clearPollFd();

      if (timer) {
            int timerFd = timer->getFd();
            if (timerFd != -1)
                  addPollFd(timerFd, POLLIN, midiTick, this, 0);
            }
      }

//---------------------------------------------------------
//   threadStop
//    called from loop()
//---------------------------------------------------------

void MidiSeq::threadStop()
      {
      if (timer)
	      timer->stopTimer();
      }

//---------------------------------------------------------
//   setRtcTicks
//    return true on success
//---------------------------------------------------------

bool MidiSeq::initRealtimeTimer()
      {
      if (!timer)
            return false;
      if (!timer->setTimerFreq(preferences.rtcTicks))
            return false;
      if (!timer->startTimer())
            return false;
      realRtcTicks = preferences.rtcTicks;
      return true;
      }

//---------------------------------------------------------
//   start
//    return true on error
//---------------------------------------------------------

bool MidiSeq::start(int prio)
      {
      timer = new RtcTimer;
      if (!timer->initTimer()) {
            delete timer;
            timer = 0;
            fprintf(stderr, "MidiSeq:start(): no midi timer available\n");
            return true;
            }
      Thread::start(prio);
      return false;
      }

//---------------------------------------------------------
//   midiTick
//    schedule events in playEvents
//---------------------------------------------------------

void MidiSeq::midiTick(void* p, void*)
      {
      MidiSeq* at = (MidiSeq*)p;
      at->getTimerTicks();    // read elapsed rtc timer ticks
      seq->processMidi();
      }

