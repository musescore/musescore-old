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

#if defined(Q_WS_WIN)
  #include <windows.h>
  #include <mmsystem.h>
#endif

#include "portmidi/porttime/porttime.h"

#include "pm.h"
#include "mscore.h"
#include "seq.h"

//---------------------------------------------------------
//   Port
//---------------------------------------------------------

Port::Port()
      {
      type = ZERO_TYPE;
      }

Port::Port(unsigned char client, unsigned char port)
      {
      _alsaPort = port;
      _alsaClient = client;
      type = ALSA_TYPE;
      }

//---------------------------------------------------------
//   setZero
//---------------------------------------------------------

void Port::setZero()
      {
      type = ZERO_TYPE;
      }

//---------------------------------------------------------
//   isZero
//---------------------------------------------------------

bool Port::isZero()  const
      {
      return type == ZERO_TYPE;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Port::operator==(const Port& p) const
      {
      if (type == ALSA_TYPE)
            return _alsaPort == p._alsaPort && _alsaClient == p._alsaClient;
      else
            return true;
      }

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool Port::operator<(const Port& p) const
      {
      if (type == ALSA_TYPE) {
            if (_alsaPort != p._alsaPort)
                  return _alsaPort < p._alsaPort;
            return _alsaClient < p._alsaClient;
            }
      return false;
      }

//---------------------------------------------------------
//   PortMidiDriver
//---------------------------------------------------------

PortMidiDriver::PortMidiDriver(Seq* s)
  : MidiDriver(s)
      {
      inputStream = 0;
      timer = 0;
      }

PortMidiDriver::~PortMidiDriver()
      {
      if (inputStream) {
            Pt_Stop();
            Pm_Close(inputStream);
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool PortMidiDriver::init()
      {
      inputId  = Pm_GetDefaultInputDeviceID();
      outputId = Pm_GetDefaultOutputDeviceID();

      if (inputId == pmNoDevice)
            return false;

      static const int INPUT_BUFFER_SIZE = 100;
      static const int DRIVER_INFO = 0;
      static const int TIME_INFO = 0;

      Pt_Start(20, 0, 0);      // timer started, 20 millisecond accuracy

      PmError error = Pm_OpenInput(&inputStream,
         inputId,
         DRIVER_INFO, INPUT_BUFFER_SIZE,
         ((long (*)(void*)) Pt_Time),
         TIME_INFO);
      if (error != pmNoError) {
            const char* p = Pm_GetErrorText(error);
            printf("PortMidi: open input (id=%d) failed: %s\n", int(inputId), p);
            Pt_Stop();
            return false;
            }

      Pm_SetFilter(inputStream, PM_FILT_ACTIVE | PM_FILT_CLOCK | PM_FILT_SYSEX);

      PmEvent buffer[1];
      while (Pm_Poll(inputStream))
            Pm_Read(inputStream, buffer, 1);

      timer = new QTimer();
      timer->setInterval(20);       // 20 msec
      timer->start();
      timer->connect(timer, SIGNAL(timeout()), seq, SLOT(midiInputReady()));
      return true;
      }

//---------------------------------------------------------
//   registerOutPort
//---------------------------------------------------------

Port PortMidiDriver::registerOutPort(const QString&)
      {
      return Port();
      }

//---------------------------------------------------------
//   registerInPort
//---------------------------------------------------------

Port PortMidiDriver::registerInPort(const QString&)
      {
      return Port();
      }

//---------------------------------------------------------
//   getInputPollFd
//---------------------------------------------------------

void PortMidiDriver::getInputPollFd(struct pollfd**, int* n)
      {
      *n = 0;
      }

//---------------------------------------------------------
//   getOutputPollFd
//---------------------------------------------------------

void PortMidiDriver::getOutputPollFd(struct pollfd**, int* n)
      {
      *n = 0;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void PortMidiDriver::read()
      {
      static int active = 0;

      if (!inputStream)
            return;
      PmEvent buffer[1];
      while (Pm_Poll(inputStream)) {
            int n = Pm_Read(inputStream, buffer, 1);
            if (n > 0) {
                  if (mscore->midiinEnabled()) {
                        int type = Pm_MessageStatus(buffer[0].message);
                        if (type == ME_NOTEON) {
                              int pitch = Pm_MessageData1(buffer[0].message);
                              int velo = Pm_MessageData2(buffer[0].message);
                              if (velo) {
                                    mscore->midiNoteReceived(pitch, active);
                                    ++active;
                                    }
                              else
                                    --active;
                              }
                        else if (type == ME_NOTEOFF)
                              --active;
                        }
                  else
                        active = 0;
                  }
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void PortMidiDriver::write(const Event&)
      {
      }

//---------------------------------------------------------
//   deviceInList
//---------------------------------------------------------

QStringList PortMidiDriver::deviceInList() const
      {
      QStringList il;
      int interf = Pm_CountDevices();
      for (PmDeviceID id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if(info->input)
                il.append(QString(info->interf) +","+ QString(info->name));
            }
      return il;
      }

//---------------------------------------------------------
//   getDeviceIn
//---------------------------------------------------------

int PortMidiDriver::getDeviceIn(char* name)
      {

      int interf = Pm_CountDevices();
      for (int id = 0; id < interf; id++) {
            const PmDeviceInfo* info = Pm_GetDeviceInfo((PmDeviceID)id);
            if(info->input){
              QString n = QString(info->interf) +","+ QString(info->name);
              if (n == name){
                 return id;
              }
            }
      }
      return -1;
      }

