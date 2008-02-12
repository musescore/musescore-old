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
#define __MIDIDRIVER_H

#include "audio.h"

class Event;
class MidiOutEvent;
class MidiSeq;

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

//---------------------------------------------------------
//   MidiDriver
//---------------------------------------------------------

class MidiDriver {

   public:
      MidiDriver() {}
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
//   DummyAudio
//---------------------------------------------------------

class DummyAudio : public Audio {
      float* buffer;
      pthread_t dummyThread;
      int realTimePriority;
      MidiSeq* midiSeq;

      static void* loop(void* pa);

   public:
      int state;
      bool seekflag;
      unsigned pos;

      DummyAudio();
      virtual ~DummyAudio();

      virtual bool init();
      virtual void start();
      virtual void stop();
      virtual QList<QString> inputPorts();
      virtual void startTransport();
      virtual void stopTransport();

      virtual int getState()          { return state; }
      virtual int sampleRate() const  { return 44100; }
      virtual bool isRealtime() const { return false; }
      virtual void putEvent(const MidiOutEvent&);
#if 0
      unsigned frameTime() const;
      unsigned lastFrameTime() const;
      unsigned curFrame() const { return pos; }
      QList<PortName> outputPorts(bool midi = false);
      virtual void registerClient();
      virtual Port registerOutPort(const QString& s, bool);
      virtual Port registerInPort(const QString& s, bool);
      virtual void unregisterPort(Port);
      virtual bool connect(Port, Port);
      virtual bool disconnect(Port, Port);
      virtual void setPortName(Port, const QString&);
      virtual Port findPort(const QString& s);
      virtual QString portName(Port port);
      virtual int realtimePriority() const { return 40; }
      virtual void seekTransport(unsigned n);
#endif
      };

extern MidiDriver* midiDriver;
#endif

