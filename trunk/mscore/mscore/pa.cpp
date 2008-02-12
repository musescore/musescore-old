//=============================================================================
//  MusE Score
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

#include "preferences.h"
#include "synti.h"
#include "score.h"
#include "mscore.h"
#include "seq.h"
#include "pa.h"
#include <portaudio.h>

PaStream* stream;

//---------------------------------------------------------
//   paCallback
//---------------------------------------------------------

int paCallback(const void*, void* out, long unsigned frames,
   const PaStreamCallbackTimeInfo*, PaStreamCallbackFlags, void *)
      {
      float* op = (float*)out;
      seq->process((unsigned)frames, op, op+1, 2);
      return 0;
      }

//---------------------------------------------------------
//   Portaudio
//---------------------------------------------------------

Portaudio::Portaudio()
   : Audio()
      {
      _sampleRate = 44100;
      initialized = false;
      state       = Seq::STOP;
      seekflag    = false;
      }

//---------------------------------------------------------
//   ~Portaudio
//---------------------------------------------------------

Portaudio::~Portaudio()
      {
      if (initialized)
            Pa_Terminate();
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Portaudio::init()
      {
      PaError err = Pa_Initialize();
      if (err != paNoError) {
            printf("Portaudio initialize failed: %s\n", Pa_GetErrorText(err));
            return false;
            }

      initialized = true;

      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      /* Open an audio I/O stream. */
      struct PaStreamParameters out;

      out.device = idx;
      out.channelCount = 2;
      out.sampleFormat = paFloat32;
      out.suggestedLatency = 0.100;
      out.hostApiSpecificStreamInfo = 0;

      err = Pa_OpenStream(&stream, 0, &out,
            double(_sampleRate),
            0,
            0,
            paCallback,
            (void*)this);

      if (err != paNoError) {
            printf("Portaudio open stream %d failed: %s\n", idx, Pa_GetErrorText(err));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   apiList
//---------------------------------------------------------

QStringList Portaudio::apiList() const
      {
      QStringList al;

      PaHostApiIndex apis = Pa_GetHostApiCount();
      for (PaHostApiIndex i = 0; i < apis; ++i) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(i);
            al.append(info->name);
            }
      return al;
      }

//---------------------------------------------------------
//   deviceList
//---------------------------------------------------------

QStringList Portaudio::deviceList(int apiIdx)
      {
      QStringList dl;
      const PaHostApiInfo* info = Pa_GetHostApiInfo(apiIdx);
      for (int i = 0; i < info->deviceCount; ++i) {
            PaDeviceIndex idx = Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, i);
            const PaDeviceInfo* di = Pa_GetDeviceInfo(idx);
            dl.append(di->name);
            }
      return dl;
      }

//---------------------------------------------------------
//   deviceIndex
//---------------------------------------------------------

int Portaudio::deviceIndex(int apiIdx, int apiDevIdx)
      {
      return Pa_HostApiDeviceIndexToDeviceIndex(apiIdx, apiDevIdx);
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void* Portaudio::registerPort(const char*)
      {
      return 0;
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void Portaudio::unregisterPort(void*)
      {
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> Portaudio::inputPorts()
      {
      QList<QString> clientList;
      return clientList;
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void Portaudio::connect(void* /*src*/, void* /*dst*/)
      {
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void Portaudio::disconnect(void* /*src*/, void* /*dst*/)
      {
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool Portaudio::start()
      {
      PaError err = Pa_StartStream(stream);
      if (err != paNoError) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool Portaudio::stop()
      {
      PaError err = Pa_StopStream(stream);
      if (err != paNoError) {
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int Portaudio::framePos() const
      {
      return 0;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void Portaudio::startTransport()
      {
      state = Seq::PLAY;
      }

//---------------------------------------------------------
//   stopTransport
//---------------------------------------------------------

void Portaudio::stopTransport()
      {
      state = Seq::STOP;
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

int Portaudio::getState()
      {
      return state;
      }

