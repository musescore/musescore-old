//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __DRIVER_H__
#define __DRIVER_H__

#include "event.h"

class Seq;
class MidiPatch;

//---------------------------------------------------------
//   MidiOutEvent
//---------------------------------------------------------

struct MidiOutEvent {
      double time;
      char port;
      char type;        // midi event type and channel
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

//---------------------------------------------------------
//   Driver
//---------------------------------------------------------

class Driver {

   protected:
      Seq* seq;

   public:
      Driver(Seq* s)    { seq = s;}
      virtual ~Driver() {}
      virtual bool init() = 0;
      virtual bool start() = 0;
      virtual bool stop() = 0;
      virtual QList<QString> inputPorts() = 0;
      virtual void stopTransport() = 0;
      virtual void startTransport() = 0;
      virtual int getState() = 0;
      virtual int sampleRate() const = 0;

      virtual void putEvent(const MidiOutEvent&) = 0;
      virtual void process(int, float*, float*, int) = 0;
      virtual void midiRead() {}
      virtual void heartBeat() {}
      virtual const MidiPatch* getPatchInfo(int, const MidiPatch*) { return 0; }
      };

#endif

