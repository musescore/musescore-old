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

/* This routine will be called by the PortAudio engine when audio is needed.
** It may called at interrupt level on some machines so don't do anything
** that could mess up the system like calling malloc() or free().
*/

int paCallback(void* /*in*/, void* out, long unsigned frames, PaTimestamp, void*)
      {
      seq->process((unsigned)frames, (float*)out);
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
      state     = Seq::STOP;
      seekflag  = false;
//      startTime = curTime();
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
//    return true on error
//---------------------------------------------------------

bool Portaudio::init()
      {
      PaError err = Pa_Initialize();
      if (err != paNoError)
            goto error;

      initialized = true;

      /* Open an audio I/O stream. */
      err = Pa_OpenDefaultStream(&stream, 0, 2, paFloat32, double(_sampleRate), 256, 2,
              paCallback, (void*)this);

      if (err != paNoError)
            return true;

   error:
      printf("Portaudio initialize failed: %s\n", Pa_GetErrorText(err));
      return false;
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

std::list<QString> Portaudio::inputPorts()
      {
      std::list<QString> clientList;
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

//---------------------------------------------------------
//   readMidiEvent
//---------------------------------------------------------

void readMidiEvent()
      {
      }

//---------------------------------------------------------
//   initMidi
//    return true on error
//---------------------------------------------------------

bool initMidi()
      {
      return true;
      }

//---------------------------------------------------------
//   getMidiReadFd
//---------------------------------------------------------

int getMidiReadFd()
      {
      return -1;
      }

