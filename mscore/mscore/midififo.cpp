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

#include "midififo.h"

#if 0
//---------------------------------------------------------
//   put
//    return true on fifo overflow
//---------------------------------------------------------

bool MidiFifo::put(const MidiEvent& event)
      {
      if (size < MIDI_FIFO_SIZE) {
            fifo[wIndex] = event;
            wIndex = (wIndex + 1) % MIDI_FIFO_SIZE;
            size.ref();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   get
//---------------------------------------------------------

MidiEvent MidiFifo::get()
      {
      MidiEvent event(fifo[rIndex]);
      rIndex = (rIndex + 1) % MIDI_FIFO_SIZE;
      size.deref();
      return event;
      }

//---------------------------------------------------------
//   peek
//---------------------------------------------------------

const MidiEvent& MidiFifo::peek(int n)
      {
      int idx = (rIndex + n) % MIDI_FIFO_SIZE;
      return fifo[idx];
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void MidiFifo::remove()
      {
      rIndex = (rIndex + 1) % MIDI_FIFO_SIZE;
      size.deref();
      }
#endif


//---------------------------------------------------------
//   put
//    return true on fifo overflow
//---------------------------------------------------------

bool MidiOutFifo::put(const MidiOutEvent& event)
      {
      if (int(size) < MIDI_FIFO_SIZE) {
            fifo[wIndex] = event;
            wIndex = (wIndex + 1) % MIDI_FIFO_SIZE;
            size.ref();
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   get
//---------------------------------------------------------

MidiOutEvent MidiOutFifo::get()
      {
      MidiOutEvent event(fifo[rIndex]);
      rIndex = (rIndex + 1) % MIDI_FIFO_SIZE;
      size.deref();
      return event;
      }

