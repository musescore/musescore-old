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
#include "driver.h"

typedef std::multiset<MidiOutEvent, std::less<MidiOutEvent> > MidiOutEventList;
typedef MidiOutEventList::iterator iMidiOutEvent;
typedef MidiOutEventList::const_iterator ciMidiOutEvent;

//---------------------------------------------------------
//   MidiOutFifo
//---------------------------------------------------------

class MidiOutFifo {
      MidiOutEvent fifo[MIDI_FIFO_SIZE];
      QAtomicInt size;
      int wIndex;
      int rIndex;

   public:
      MidiOutFifo()  { clear(); }
      bool put(const MidiOutEvent& event);   // returns true on fifo overflow
      MidiOutEvent get();
      bool isEmpty() const { return int(size) == 0; }
      void clear()         { size = 0, wIndex = 0, rIndex = 0; }
      };

#endif

