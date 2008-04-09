//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  AlsaDriver based on code from Fons Adriaensen (clalsadr.cc)
//  Copyright (C) 2003 Fons Adriaensen
//  partly based on original work from Paul Davis
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

#ifndef __MIDIDRIVER_H__
#define __MIDIDRIVER_H__

#include "driver.h"

class Event;
class MidiOutEvent;
class MidiSeq;
class Seq;

//---------------------------------------------------------
//    Port
//---------------------------------------------------------

class Port {
      enum { ALSA_TYPE, ZERO_TYPE } type;
      unsigned char _alsaPort;
      unsigned char _alsaClient;

   protected:
      unsigned char alsaPort() const   { return _alsaPort; }
      unsigned char alsaClient() const { return _alsaClient; }

   public:
      Port();
      Port(unsigned char client, unsigned char port);
      void setZero();
      bool isZero() const;
      bool operator==(const Port& p) const;
      bool operator<(const Port& p) const;
      friend class MidiDriver;
      friend class AlsaMidiDriver;
      };

#ifdef USE_ALSA
//---------------------------------------------------------
//   MidiDriver
//---------------------------------------------------------

class MidiDriver {
   protected:
      Port midiInPort;
      Port* midiOutPorts;
      Seq* seq;

   public:
      MidiDriver(Seq* s) { seq = s; }
      virtual ~MidiDriver() {}
      virtual bool init() = 0;
      virtual Port registerOutPort(const QString& name) = 0;
      virtual Port registerInPort(const QString& name) = 0;
      virtual void getInputPollFd(struct pollfd**, int* n) = 0;
      virtual void getOutputPollFd(struct pollfd**, int* n) = 0;
      virtual void read() = 0;
      virtual void write(const MidiOutEvent&) = 0;
      };

//---------------------------------------------------------
//   AlsaMidi
//---------------------------------------------------------

class AlsaMidi : public Driver {
      float* buffer;
      int realTimePriority;
      MidiSeq* midiSeq;
      MidiDriver* midiDriver;

      static void* loop(void* pa);

   public:
      int state;
      bool seekflag;
      unsigned pos;

      AlsaMidi(Seq*);
      virtual ~AlsaMidi() {}

      virtual bool init();
      virtual bool start();
      virtual bool stop();
      virtual QList<QString> inputPorts();
      virtual void startTransport();
      virtual void stopTransport();

      virtual int getState()          { return state; }
      virtual int sampleRate() const  { return 10000; }
      virtual bool isRealtime() const { return false; }
      virtual void putEvent(const MidiOutEvent&);
      virtual void process(int, float*, float*, int) {}
      virtual void midiRead();
      };

#endif

#endif

