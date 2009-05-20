//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

// #include "event.h"

class Seq;
class MidiPatch;
class Synth;
class Event;

//---------------------------------------------------------
//   Driver
//---------------------------------------------------------

class Driver {

   protected:
      Seq* seq;
      Synth* synth;

   public:
      Driver(Seq* s)    { seq = s; synth = 0; }
      virtual ~Driver() {}
      virtual bool init() = 0;
      virtual bool start() = 0;
      virtual bool stop() = 0;
      virtual QList<QString> inputPorts() = 0;
      virtual void stopTransport() = 0;
      virtual void startTransport() = 0;
      virtual int getState() = 0;
      virtual int sampleRate() const = 0;

      virtual void putEvent(const Event&) = 0;
      virtual void process(int, float*, float*, int) = 0;
      virtual void midiRead() {}
      virtual const MidiPatch* getPatchInfo(bool /*onlyDrums*/, const MidiPatch*) { return 0; }

      Synth* getSynth() const { return synth; }
      };

#endif

