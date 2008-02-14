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

#ifndef __MIDISEQ_H__
#define __MIDISEQ_H__

#include "thread.h"
#include "timerdev.h"
#include "midififo.h"

class MidiDriver;

//---------------------------------------------------------
//   MidiSeq
//---------------------------------------------------------

class MidiSeq : public Thread {
      int realRtcTicks;
      Timer* timer;
      static MidiDriver* midiDriver;

      MidiOutFifo fifo;
      MidiOutEventList playEvents;

      static void midiTick(void* p, void*);
      int getTimerTicks() { return timer->getTimerTicks(); }

   public:
      MidiSeq(MidiDriver*, const char* name);
      bool start(int);
      virtual void threadStop();
      virtual void threadStart(void*);
      void updatePollFd();
      bool initRealtimeTimer();
      void putEvent(const MidiOutEvent& e) {
            fifo.put(e);
            }
      };

#endif

