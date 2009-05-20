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

//---------------------------------------------------------
//   JackAudio
//---------------------------------------------------------

JackAudio::JackAudio(Seq* s)
   : Driver(s)
      {
      client = 0;
      synth  = 0;
      }

//---------------------------------------------------------
//   ~JackAudio
//---------------------------------------------------------

JackAudio::~JackAudio()
      {
      if (client) {
            if (jack_client_close(client)) {
                  fprintf(stderr, "jack_client_close() failed: %s\n",
                     strerror(errno));
                  }
            }
      if (synth)
            delete synth;
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void* JackAudio::registerPort(const char* name)
      {
      return jack_port_register(client, name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void JackAudio::unregisterPort(void* p)
      {
      jack_port_unregister(client, (jack_port_t*)p);
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
      /* connect the ports. Note: you can't do this before
         the client is activated, because we can't allow
         connections to be made to clients that aren't
         running.
       */
      QString lport = preferences.lPort;
      QString rport = preferences.rPort;

      const char* src = jack_port_name(portL);
      int rv;
      rv = jack_connect(client, src, lport.toLatin1().data());
      if (rv) {
            fprintf(stderr, "jack connect <%s> - <%s> failed: %d\n",
               src, lport.toLatin1().data(), rv);
            }
      src = jack_port_name(portR);
      if (!rport.isEmpty()) {
            if (jack_connect(client, src, rport.toLatin1().data())) {
                  fprintf(stderr, "jack connect <%s> - <%s> failed\n",
                     src, rport.toLatin1().data());
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
      jack_client_close(client);
#if 0
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
      float* lbuffer = audio->getLBuffer(frames);
      float* rbuffer = audio->getRBuffer(frames);
      audio->seq->process((unsigned)frames, lbuffer, rbuffer, 1);
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
      jack_set_error_function(noJackError);

      portL  = 0;
      portR  = 0;
      client = 0;
      int i  = 0;
      for (i = 0; i < 5; ++i) {
            sprintf(_jackName, "Mscore%d", i+1);
            client = jack_client_new(_jackName);
            if (client)
                  break;
            }
      if (client == 0)
            return false;
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
      portL = jack_port_register(client, "left",  JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
      portR = jack_port_register(client, "right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

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

#ifdef USE_GLOBAL_FLUID
      synth = new Fluid();
#else
      synth = new FluidS::Fluid();
#endif
      synth->init(_sampleRate, preferences.midiPorts * 16);
      midiDriver = new AlsaMidiDriver(seq);
      if (!midiDriver->init()) {
            delete midiDriver;
            midiDriver = 0;
            return false;
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
      synth->play(e);
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void JackAudio::process(int n, float* l, float* r, int stride)
      {
      synth->process(n, l, r, stride);
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const MidiPatch* JackAudio::getPatchInfo(bool onlyDrums, const MidiPatch* p)
      {
      if (synth)
            return synth->getPatchInfo(onlyDrums, p);
      return 0;
      }

//---------------------------------------------------------
//   midiRead
//---------------------------------------------------------

void JackAudio::midiRead()
      {
      midiDriver->read();
      }

