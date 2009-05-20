//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id$
//
//  (C) Copyright 2004 Robert Jonsson (rj@spamatica.se)
//  Copyright (C) 2002-2006 by Werner Schweer and others
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

// #include <linux/version.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <poll.h>

#include "rtctimer.h"
#include "globals.h"
#include "preferences.h"

//---------------------------------------------------------
//   RtcTimer
//---------------------------------------------------------

RtcTimer::RtcTimer()
      {
      timerFd = -1;
      }

//---------------------------------------------------------
//   ~RtcTimer
//---------------------------------------------------------
RtcTimer::~RtcTimer()
    {
    if (timerFd != -1)
      close(timerFd);
    }

//---------------------------------------------------------
//   initTimer
//    return false on error
//---------------------------------------------------------

bool RtcTimer::initTimer()
      {
      if(TIMER_DEBUG)
            printf("RtcTimer::initTimer()\n");
      if (timerFd != -1) {
            fprintf(stderr,"RtcTimer::initTimer(): called on initialised timer!\n");
            return false;
            }
      timerFd = ::open("/dev/rtc", O_RDONLY);
      if (timerFd == -1) {
            fprintf(stderr, "RtcTimer:: fatal error: open /dev/rtc failed: %s\n",
               strerror(errno));
            return false;
            }
      if (!setTimerFreq(preferences.rtcTicks)) {
            // unable to set timer frequency
            timerFd = -1;
            return false;
            }
      // check if timer really works, start and stop it once.
      if (!startTimer()) {
            timerFd = -1;
            return false;
            }
      if (!stopTimer()) {
            timerFd = -1;
            return false;
            }
      // stop has closed the device, open it again
      timerFd = ::open("/dev/rtc", O_RDONLY);
      return true;
      }


//---------------------------------------------------------
//   setTimerFreq
//    return false on error
//---------------------------------------------------------

bool RtcTimer::setTimerFreq(unsigned int tick)
    {
    int rc = ioctl(timerFd, RTC_IRQP_SET, tick);
    if (rc == -1) {
            fprintf(stderr, "RtcTimer::setTimerFreq(): cannot set ticks %d on /dev/rtc: %s\n",
               preferences.rtcTicks, strerror(errno));
            fprintf(stderr, "  precise timer not available\n");
            close(timerFd);
            //if (!debugMode)
            //      fatalError("set timer ticks failed");
            timerFd = -1;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   getTimerResolution
//---------------------------------------------------------

int RtcTimer::getTimerResolution()
      {
      /* The RTC doesn't really work with a set resolution as such.
       * Not sure how this fits into things yet.
       */
      return 0;
      }

//---------------------------------------------------------
//   getTimerFreq
//---------------------------------------------------------

unsigned int RtcTimer::getTimerFreq()
    {
    unsigned int freq;
    int rv = ioctl(timerFd, RTC_IRQP_READ, &freq);
    if (rv < 1)
      return 0;
    return freq;
    }
//---------------------------------------------------------
//   startTimer
//---------------------------------------------------------

bool RtcTimer::startTimer()
      {
      if (TIMER_DEBUG)
            printf("RtcTimer::startTimer\n");
      if (timerFd == -1) {
            if (!debugMode)
                  fprintf(stderr, "RtcTimer::startTimer(): no timer to start.\n");
            return false;
            }
      if (ioctl(timerFd, RTC_PIE_ON, 0) == -1) {
            perror("RtcTimer::startTimer(): start: RTC_PIE_ON failed");
            ::close(timerFd);
            timerFd = -1;
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   stopTimer
//---------------------------------------------------------

bool RtcTimer::stopTimer()
      {
      if (TIMER_DEBUG)
            printf("RtcTimer::stopTimer\n");
      if (timerFd != -1) {
          ioctl(timerFd, RTC_PIE_OFF, 0);
          ::close(timerFd);
          timerFd = -1;
          }
      return true;
      }

//---------------------------------------------------------
//   getTimerTicks
//---------------------------------------------------------

unsigned long RtcTimer::getTimerTicks()
      {
      if (TIMER_DEBUG)
            printf("getTimerTicks()\n");
      if (timerFd==-1) {
          fprintf(stderr,"RtcTimer::getTimerTicks(): no RTC open to read!\n");
          return 0;
      }
      unsigned long nn;
      if (read(timerFd, &nn, sizeof(unsigned long)) != sizeof(unsigned long)) {
            perror("rtc timer read error");
            // fatalError("RtcTimer::getTimerTicks(): error reading RTC");
            }
      return nn;
      }

