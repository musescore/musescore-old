//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id:$
//
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

#ifndef __MIDIFIFO_H__
#define __MIDIFIFO_H__

#define MIDI_FIFO_SIZE    512

#include <set>
#include "event.h"

//---------------------------------------------------------
//   MidiOutEvent
//---------------------------------------------------------

struct MidiOutEvent {
      unsigned time;
      char port;
      char type;
      char a;
      char b;

      MidiOutEvent() {}

      bool operator<(const MidiOutEvent& e) const {
            if (time != e.time)
                  return time < e.time;
            // play note off events first to prevent overlapping
            // notes

            int channel = type & 0xf;
            if (channel == e.type & 0xf) {
                  int t = type & 0xf0;
                  return t == ME_NOTEOFF || (t == ME_NOTEON && b == 0);
                  }
            int map[16] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 10, 11, 12, 13, 14, 15 };
            return map[channel] < map[e.type & 0xf];
            }
      };

typedef std::multiset<MidiOutEvent, std::less<MidiOutEvent> > MidiOutEventList;
typedef MidiOutEventList::iterator iMidiOutEvent;
typedef MidiOutEventList::const_iterator ciMidiOutEvent;

//---------------------------------------------------------
//   MidiOutFifo
//---------------------------------------------------------

class MidiOutFifo {
      MidiOutEvent fifo[MIDI_FIFO_SIZE];
      volatile int size;
      int wIndex;
      int rIndex;

   public:
      MidiOutFifo()  { clear(); }
      bool put(const MidiOutEvent& event);   // returns true on fifo overflow
      MidiOutEvent get();
      bool isEmpty() const { return size == 0; }
      void clear()         { size = 0, wIndex = 0, rIndex = 0; }
      };

#endif

