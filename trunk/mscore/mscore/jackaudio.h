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

#ifndef __JACKAUDIO_H__
#define __JACKAUDIO_H__

#include "config.h"
#include "driver.h"
#include <jack/jack.h>

class Synth;
class Seq;
class MidiDriver;

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

class JackAudio : public Driver {
      int _sampleRate;
      int _segmentSize;

      jack_nframes_t _frameCounter;

      jack_client_t* client;
      char _jackName[8];

      QList<jack_port_t*> ports;
      QList<jack_port_t*> midiPorts;

      static int processAudio(jack_nframes_t, void*);

   public:
      JackAudio(Seq*);
      virtual ~JackAudio();
      virtual bool init();
      virtual QList<QString> inputPorts();
      virtual bool start();
      virtual bool stop();
      int framePos() const;
      void connect(void*, void*);
      void disconnect(void* src, void* dst);
      virtual bool isRealtime() const   { return jack_is_realtime(client); }
      virtual void startTransport();
      virtual void stopTransport();
      virtual int getState();
      virtual int sampleRate() const    { return _sampleRate; }
      virtual void putEvent(const Event&);
      virtual void process(int, float*, float*, int);
      virtual void midiRead();
      virtual unsigned frameTime() const { return _frameCounter; }

      virtual int registerPort(const QString& name, bool input, bool midi);
      virtual void unregisterPort(int);
      };

#endif

