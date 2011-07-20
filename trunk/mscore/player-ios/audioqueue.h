//=============================================================================
//  MscorePlayer
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __AUDIOQUEUE_H__
#define __AUDIOQUEUE_H__

#include "driver.h"

class Synth;
class Seq;
class MidiDriver;
class QueueAudioData;

//---------------------------------------------------------
//   QueueAudio
//---------------------------------------------------------

class QueueAudio : public Driver {

      QueueAudioData* d;
      int state;

   public:
      QueueAudio(Seq* s);
      virtual ~QueueAudio() {}
      virtual bool init();
      virtual bool start();
      virtual bool stop();
      virtual void startTransport();
      virtual void stopTransport();
      virtual int getState()          { return state;      }
      virtual int sampleRate() const  { return 44100;      }
      virtual void putEvent(const Event&, unsigned /*framePos*/) {}
      virtual void midiRead() {}

      virtual void registerPort(const QString& /*name*/, bool /*input*/) {}
      virtual void unregisterPort(int){}
      virtual QList<QString> inputPorts() { QList<QString> a; return a; }
      };

#endif

