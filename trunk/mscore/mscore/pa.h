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

#ifndef __PORTAUDIO_H__
#define __PORTAUDIO_H__

#include "config.h"
#include "audio.h"

//---------------------------------------------------------
//   Portaudio
//---------------------------------------------------------

class Portaudio : public Audio {
      bool initialized;
      int _sampleRate;

      int state;
      bool seekflag;
      unsigned pos;
      double startTime;

   public:
      Portaudio();
      virtual ~Portaudio();
      virtual bool init();
      void* registerPort(const char* name);
      void unregisterPort(void* p);
      virtual QList<QString> inputPorts();
      virtual void start();
      virtual void stop();
      int framePos() const;
      void connect(void*, void*);
      void disconnect(void* src, void* dst);
      float* getLBuffer(long n);
      float* getRBuffer(long n);
      virtual bool isRealtime() const   { return false; }
      virtual void startTransport();
      virtual void stopTransport();
      virtual int getState();
      virtual int sampleRate() const { return _sampleRate; }

      QStringList apiList() const;
      QStringList deviceList(int apiIdx);
      int deviceIndex(int apiIdx, int apiDevIdx);
      };

#endif


