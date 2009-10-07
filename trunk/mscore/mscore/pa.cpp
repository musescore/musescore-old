//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "fluid.h"

#ifdef USE_ALSA
#include "alsa.h"
#include "alsamidi.h"
#endif

#include <portaudio.h>
#include "mididriver.h"
#include "pm.h"

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

Portaudio::Portaudio(Seq* s)
   : Driver(s)
      {
      _sampleRate = 44100;
      initialized = false;
      state       = Seq::STOP;
      seekflag    = false;
      midiDriver  = 0;
      }

//---------------------------------------------------------
//   ~Portaudio
//---------------------------------------------------------

Portaudio::~Portaudio()
      {
      if (initialized) {
            // Pa_Terminate();      // DEBUG: crashes
            }
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
      if (debugMode)
            printf("using PortAudio Version: %s\n", Pa_GetVersionText());

      PaDeviceIndex idx = preferences.portaudioDevice;

      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      /* Open an audio I/O stream. */
      struct PaStreamParameters out;

      out.device           = idx;
      out.channelCount     = 2;
      out.sampleFormat     = paFloat32;
      out.suggestedLatency = 0.100;
      out.hostApiSpecificStreamInfo = 0;

      err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
      if (err != paNoError) {
            // fall back to default device:
            out.device = Pa_GetDefaultOutputDevice();
            err = Pa_OpenStream(&stream, 0, &out, double(_sampleRate), 0, 0, paCallback, (void*)this);
            if (err != paNoError)
                  printf("Portaudio open stream %d failed: %s\n", idx, Pa_GetErrorText(err));
            return false;
            }
      synth = new FluidS::Fluid();
      synth->init(_sampleRate);

#ifdef USE_ALSA
      midiDriver = new AlsaMidiDriver(seq);
#endif
#ifdef USE_PORTMIDI
      midiDriver = new PortMidiDriver(seq);
#endif
      if (midiDriver && !midiDriver->init()) {
            delete midiDriver;
            midiDriver = 0;
#ifdef USE_PORTMIDI
            return true;                  // return OK for audio driver; midi is only input
#else
            return false;
#endif
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

int Portaudio::registerPort(const QString&, bool, bool)
      {
      return -1;
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void Portaudio::unregisterPort(int)
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
            printf("Portaudio: start stream failed: %s\n", Pa_GetErrorText(err));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool Portaudio::stop()
      {
      PaError err = Pa_StopStream(stream);
      if (err != paNoError) {
            printf("Portaudio: stop failed: %s\n", Pa_GetErrorText(err));
            return false;
            }
      return true;
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

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void Portaudio::putEvent(const Event& e, unsigned /* framePos*/)
      {
      synth->play(e);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Portaudio::process(int n, float* l, float* r, int stride)
      {
      synth->process(n, l, r, stride);
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void Portaudio::midiRead()
      {
      if (midiDriver)
            midiDriver->read();
      }

//---------------------------------------------------------
//   currentApi
//---------------------------------------------------------

int Portaudio::currentApi() const
      {
      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            for (int k = 0; k < info->deviceCount; ++k) {
                  PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                  if (i == idx)
                        return api;
                  }
            }
      printf("Portaudio: no current api found for device %d\n", idx);
      return -1;
      }

//---------------------------------------------------------
//   currentDevice
//---------------------------------------------------------

int Portaudio::currentDevice() const
      {
      PaDeviceIndex idx = preferences.portaudioDevice;
      if (idx < 0)
            idx = Pa_GetDefaultOutputDevice();

      for (int api = 0; api < Pa_GetHostApiCount(); ++api) {
            const PaHostApiInfo* info = Pa_GetHostApiInfo(api);
            for (int k = 0; k < info->deviceCount; ++k) {
                  PaDeviceIndex i = Pa_HostApiDeviceIndexToDeviceIndex(api, k);
                  if (i == idx)
                        return k;
                  }
            }
      printf("Portaudio: no current ApiDevice found for device %d\n", idx);
      return -1;
      }

