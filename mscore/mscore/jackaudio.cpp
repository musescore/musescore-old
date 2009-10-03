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

#include "jackaudio.h"

#include "preferences.h"
#include "synti.h"
#include "score.h"
#include "mscore.h"
#include "seq.h"
#include "fluid.h"
#include "alsa.h"
#include "alsamidi.h"

#include <jack/midiport.h>

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

JackAudio::JackAudio(Seq* s)
   : Driver(s)
      {
printf("JackAudio() midi %d audio %d\n", preferences.useJackMidi, preferences.useJackAudio);
      client        = 0;
      synth         = 0;
      _frameCounter = 0;
      }

//---------------------------------------------------------
//   ~JackAudio
//---------------------------------------------------------

JackAudio::~JackAudio()
      {
printf("~JackAudio()\n");
      if (client) {
            if (jack_client_close(client)) {
                  fprintf(stderr, "jack_client_close() failed: %s\n",
                     strerror(errno));
                  }
            }
      delete synth;
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

int JackAudio::registerPort(const QString& name, bool input, bool midi)
      {
printf("JackAudio:: register port input %d midi %d\n", input, midi);
      int portFlag         = input ? JackPortIsInput : JackPortIsOutput;
      const char* portType = midi ? JACK_DEFAULT_MIDI_TYPE : JACK_DEFAULT_AUDIO_TYPE;
      jack_port_t* port = jack_port_register(client, qPrintable(name), portType, portFlag, 0);
      if (port == 0) {
            printf("JackAudio:registerPort(%s) failed\n", qPrintable(name));
            return -1;
            }
      if (midi) {
            midiPorts.append(port);
            return midiPorts.size() - 1;
            }
      else {
            ports.append(port);
            return ports.size() - 1;
            }
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void JackAudio::unregisterPort(int port)
      {
      jack_port_unregister(client, ports[port]);
      ports[port] = 0;
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> JackAudio::inputPorts()
      {
      const char** ports = jack_get_ports(client, 0, 0, 0);
      QList<QString> clientList;
      for (const char** p = ports; p && *p; ++p) {
            jack_port_t* port = jack_port_by_name(client, *p);
            int flags = jack_port_flags(port);
            if (!(flags & JackPortIsInput))
                  continue;
            char buffer[128];
            strncpy(buffer, *p, 128);
            if (strncmp(buffer, "Mscore", 6) == 0)
                  continue;
            clientList.append(QString(buffer));
            }
      return clientList;
      }

//---------------------------------------------------------
//   connect
//---------------------------------------------------------

void JackAudio::connect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);

      fprintf(stderr, "jack connect <%s>%p - <%s>%p\n",
               sn, src, dn, dst);

      if (sn == 0 || dn == 0) {
            fprintf(stderr, "JackAudio::connect: unknown jack ports\n");
            return;
            }
      if (jack_connect(client, sn, dn)) {
            fprintf(stderr, "jack connect <%s>%p - <%s>%p failed\n",
               sn, src, dn, dst);
            }
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void JackAudio::disconnect(void* src, void* dst)
      {
      const char* sn = jack_port_name((jack_port_t*) src);
      const char* dn = jack_port_name((jack_port_t*) dst);
      if (sn == 0 || dn == 0) {
            fprintf(stderr, "JackAudio::disconnect: unknown jack ports\n");
            return;
            }
      if (jack_disconnect(client, sn, dn)) {
            fprintf(stderr, "jack disconnect <%s> - <%s> failed\n",
               sn, dn);
            }
      }

//---------------------------------------------------------
//   start
//    return false on error
//---------------------------------------------------------

bool JackAudio::start()
      {
      if (jack_activate(client)) {
            fprintf (stderr, "JACK: cannot activate client\n");
            return false;
            }
      if (preferences.useJackAudio) {
            /* connect the ports. Note: you can't do this before
               the client is activated, because we can't allow
               connections to be made to clients that aren't
               running.
             */
            QString lport = preferences.lPort;
            QString rport = preferences.rPort;

            const char* src = jack_port_name(ports[0]);
            int rv;
            rv = jack_connect(client, src, qPrintable(lport));
            if (rv) {
                  fprintf(stderr, "jack connect <%s> - <%s> failed: %d\n",
                     src, qPrintable(lport), rv);
                  }
            src = jack_port_name(ports[1]);
            if (!rport.isEmpty()) {
                  if (jack_connect(client, src, qPrintable(rport))) {
                        fprintf(stderr, "jack connect <%s> - <%s> failed: %d\n",
                           src, qPrintable(rport), rv);
                        }
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   stop
//    return false on error
//---------------------------------------------------------

bool JackAudio::stop()
      {
printf("JackAudio::stop\n");
//      jack_client_close(client);
#if 1
      if (jack_deactivate(client)) {
            fprintf (stderr, "cannot deactivate client");
            return false;
            }
#endif
      return true;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int JackAudio::framePos() const
      {
      jack_nframes_t n = jack_frame_time(client);
      return (int)n;
      }


static int bufsize_callback(jack_nframes_t n, void*)
      {
      printf("JACK: buffersize changed %d\n", n);
      return 0;
      }

//---------------------------------------------------------
//   freewheel_callback
//---------------------------------------------------------

static void freewheel_callback(int /*starting*/, void*)
      {
      }

static int srate_callback(jack_nframes_t, void*)
      {
//      printf("JACK: sample rate changed: %d\n", n);
      return 0;
      }

static void registration_callback(jack_port_id_t, int, void*)
      {
      printf("JACK: registration changed\n");
      }

static int graph_callback(void*)
      {
//      printf("JACK: graph changed\n");
      return 0;
      }

//---------------------------------------------------------
//   processAudio
//    JACK callback
//---------------------------------------------------------

int JackAudio::processAudio(jack_nframes_t frames, void* p)
      {
      JackAudio* audio = (JackAudio*)p;
      float* l;
      float* r;
      if (preferences.useJackAudio) {
            l = (float*)jack_port_get_buffer(audio->ports[0], frames);
            r = (float*)jack_port_get_buffer(audio->ports[1], frames);
            }
      else {
            l = 0;
            r = 0;
            }
      if (preferences.useJackMidi) {
            foreach(jack_port_t* port, audio->midiPorts) {
                  void* portBuffer = jack_port_get_buffer(port, frames);
                  jack_midi_clear_buffer(portBuffer);
                  }
            }
      audio->seq->process((unsigned)frames, l, r, 1);
      audio->_frameCounter += frames;
      return 0;
      }

//---------------------------------------------------------
//   processShutdown
//---------------------------------------------------------

static void processShutdown(void*)
      {
      fprintf(stderr, "JACK SHUTDOWN\n");
      }

//---------------------------------------------------------
//   jackError
//---------------------------------------------------------

static void jackError(const char *s)
      {
      fprintf(stderr, "JACK ERROR: %s\n", s);
      }

//---------------------------------------------------------
//   noJackError
//---------------------------------------------------------

static void noJackError(const char* /* s */)
      {
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool JackAudio::init()
      {
printf("JackAudio()::init()\n");
      jack_set_error_function(noJackError);

      client = 0;
      int i  = 0;
      for (i = 0; i < 5; ++i) {
            sprintf(_jackName, "Mscore%d", i+1);
            client = jack_client_new(_jackName);
            if (client)
                  break;
            }
      if (client == 0) {
            printf("JackAudio()::init(): failed\n");
            return false;
            }
      jack_set_error_function(jackError);
      jack_set_process_callback(client, processAudio, this);
      jack_on_shutdown(client, processShutdown, this);
      jack_set_buffer_size_callback(client, bufsize_callback, this);
      jack_set_sample_rate_callback(client, srate_callback, this);
      jack_set_port_registration_callback(client, registration_callback, this);
      jack_set_graph_order_callback(client, graph_callback, this);
      jack_set_freewheel_callback (client, freewheel_callback, this);
      _sampleRate   = jack_get_sample_rate(client);
      _segmentSize  = jack_get_buffer_size(client);

      // register mscore left/right output ports
      if (preferences.useJackAudio) {
            registerPort("left", false, false);
            registerPort("right", false, false);

            // connect mscore output ports to jack input ports
            QString lport = preferences.lPort;
            QString rport = preferences.rPort;
            QList<QString> ports = inputPorts();
            QList<QString>::iterator pi = ports.begin();
            if (lport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.lPort = *pi;
                        ++pi;
                        }
                  else {
                        fprintf(stderr, "no jack ports found\n");
                        jack_client_close(client);
                        client = 0;
                        return false;
                        }
                  }
            if (rport.isEmpty()) {
                  if (pi != ports.end()) {
                        preferences.rPort = *pi;
                        }
                  else {
                        fprintf(stderr, "no jack port for right channel found!\n");
                        }
                  }

            synth = new FluidS::Fluid();
            synth->init(_sampleRate);
            }

      if (preferences.useJackMidi) {
            for (int i = 0; i < preferences.midiPorts; ++i)
                  registerPort(QString("mscore-midi-%1").arg(i+1), false, true);
            }
      return true;
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void JackAudio::startTransport()
      {
      jack_transport_start(client);
      }

//---------------------------------------------------------
//   stopTrasnport
//---------------------------------------------------------

void JackAudio::stopTransport()
      {
      jack_transport_stop(client);
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

int JackAudio::getState()
      {
      jack_position_t pos;
      int transportState = jack_transport_query(client, &pos);
      switch (transportState) {
            case JackTransportStopped:  return Seq::STOP;
            case JackTransportLooping:
            case JackTransportRolling:  return Seq::PLAY;
            case JackTransportStarting: return Seq::START_PLAY;
            default:
                  return Seq::STOP;
            }
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void JackAudio::putEvent(const Event& e)
      {
      if (preferences.useJackAudio)
            synth->play(e);
      if (!preferences.useJackMidi)
            return;

      int portIdx = e.channel() / 16;
      int chan    = e.channel() % 16;

printf("JackAudio::putEvent %d:%d\n", portIdx, chan);
      if (portIdx < 0 || portIdx >= midiPorts.size()) {
            printf("JackAudio::putEvent: invalid port %d\n", portIdx);
            return;
            }
      jack_port_t* port = midiPorts[portIdx];
      if (midiOutputTrace) {
            const char* portName = jack_port_name(port);
            printf("MidiOut<%s>: jackMidi: ", portName);
            // e.dump();
            }
      void* pb = jack_port_get_buffer(port, _segmentSize);
#if 0
      int ft   = e.ontime() - _frameCounter;
      if (ft < 0)
            ft = 0;
      if (ft >= int(_segmentSize)) {
            printf("JackAudio::putEvent: time out of range %d(seg=%d)\n", ft, _segmentSize);
            if (ft > int(_segmentSize))
                  ft = _segmentSize - 1;
            }
#endif
      int ft = 0;

      switch(e.type()) {
            case ME_NOTEON:
            case ME_NOTEOFF:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PITCHBEND:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, ft, 3);
                  if (p == 0) {
                        fprintf(stderr, "JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  p[2] = e.dataB();
                  }
                  break;

            case ME_PROGRAM:
            case ME_AFTERTOUCH:
                  {
                  unsigned char* p = jack_midi_event_reserve(pb, ft, 2);
                  if (p == 0) {
                        fprintf(stderr, "JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0] = e.type() | chan;
                  p[1] = e.dataA();
                  }
                  break;
            case ME_SYSEX:
                  {
                  const unsigned char* data = e.data();
                  int len = e.len();
                  unsigned char* p = jack_midi_event_reserve(pb, ft, len+2);
                  if (p == 0) {
                        fprintf(stderr, "JackMidi: buffer overflow, event lost\n");
                        return;
                        }
                  p[0]     = 0xf0;
                  p[len+1] = 0xf7;
                  memcpy(p+1, data, len);
                  }
                  break;
            case ME_SONGPOS:
            case ME_CLOCK:
            case ME_START:
            case ME_CONTINUE:
            case ME_STOP:
                  printf("JackMidi: event type %x not supported\n", e.type());
                  break;
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void JackAudio::process(int n, float* l, float* r, int stride)
      {
      if (preferences.useJackAudio)
            synth->process(n, l, r, stride);
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void JackAudio::midiRead()
      {
//      midiDriver->read();
      }

