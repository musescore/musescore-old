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

#include "mididriver.h"
#include "alsa.h"
#include "preferences.h"
#include "mscore.h"
#include "midifile.h"
#include "globals.h"
#include "midififo.h"

Port midiInPort;
Port* midiOutPorts;
MidiDriver* midiDriver;

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

#ifdef USE_ALSA

static const unsigned int inCap  = SND_SEQ_PORT_CAP_SUBS_READ;
static const unsigned int outCap = SND_SEQ_PORT_CAP_SUBS_WRITE;

//---------------------------------------------------------
//   AlsaMidiDriver
//---------------------------------------------------------

class AlsaMidiDriver : public MidiDriver {
      snd_seq_t* alsaSeq;

      bool putEvent(snd_seq_event_t* event);

   public:
      AlsaMidiDriver();
      virtual ~AlsaMidiDriver() {}
      virtual bool init();
      virtual Port registerOutPort(const QString& name);
      virtual Port registerInPort(const QString& name);
      virtual void getInputPollFd(struct pollfd**, int* n);
      virtual void getOutputPollFd(struct pollfd**, int* n);
      virtual void read();
      virtual void write(const MidiOutEvent&);
      };

//---------------------------------------------------------
//   AlsaMidiDriver
//---------------------------------------------------------

AlsaMidiDriver::AlsaMidiDriver()
   : MidiDriver()
      {
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool AlsaMidiDriver::init()
      {
      int error = snd_seq_open(&alsaSeq, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
      if (error < 0) {
            if (error == ENOENT)
                  fprintf(stderr, "open ALSA sequencer failed: %s\n", snd_strerror(error));
            return false;
            }

      snd_seq_set_client_name(alsaSeq, "MuseScore");

      //-----------------------------------------
      //    subscribe to "Announce"
      //    this enables callbacks for any
      //    alsa port changes
      //-----------------------------------------

      snd_seq_addr_t src, dst;
      int rv = snd_seq_create_simple_port(alsaSeq, "MusE Port 0",
         inCap | outCap | SND_SEQ_PORT_CAP_READ
         | SND_SEQ_PORT_CAP_WRITE
         | SND_SEQ_PORT_CAP_NO_EXPORT,
         SND_SEQ_PORT_TYPE_APPLICATION);
      if (rv < 0) {
            fprintf(stderr, "Alsa: create MuseScore port failed: %s\n", snd_strerror(error));
            return false;
            }
      dst.port   = rv;
      dst.client = snd_seq_client_id(alsaSeq);
      src.port   = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
      src.client = SND_SEQ_CLIENT_SYSTEM;

      snd_seq_port_subscribe_t* subs;
      snd_seq_port_subscribe_alloca(&subs);
      snd_seq_port_subscribe_set_dest(subs, &dst);
      snd_seq_port_subscribe_set_sender(subs, &src);
      error = snd_seq_subscribe_port(alsaSeq, subs);
      if (error < 0) {
            fprintf(stderr, "Alsa: Subscribe System failed: %s\n", snd_strerror(error));
            return false;
            }
      return true;
      }

//---------------------------------------------------------
//   registerOutPort
//---------------------------------------------------------

Port AlsaMidiDriver::registerOutPort(const QString& name)
      {
      int alsaPort  = snd_seq_create_simple_port(alsaSeq, name.toLatin1().data(),
         outCap | SND_SEQ_PORT_CAP_WRITE, SND_SEQ_PORT_TYPE_APPLICATION);
      if (alsaPort < 0) {
            perror("cannot create alsa out port");
            return Port();
            }
      return Port(snd_seq_client_id(alsaSeq), alsaPort);
      }

//---------------------------------------------------------
//   registerInPort
//---------------------------------------------------------

Port AlsaMidiDriver::registerInPort(const QString& name)
      {
      int alsaPort  = snd_seq_create_simple_port(alsaSeq, name.toLatin1().data(),
         inCap | SND_SEQ_PORT_CAP_READ, SND_SEQ_PORT_TYPE_APPLICATION);
      if (alsaPort < 0) {
            perror("cannot create alsa in port");
            return Port();
            }
      return Port(snd_seq_client_id(alsaSeq), alsaPort);
      }

//---------------------------------------------------------
//   getInputPollFd
//---------------------------------------------------------

void AlsaMidiDriver::getInputPollFd(struct pollfd** p, int* n)
      {
      int npfdi = snd_seq_poll_descriptors_count(alsaSeq, POLLIN);
      struct pollfd* pfdi  = new struct pollfd[npfdi];
      snd_seq_poll_descriptors(alsaSeq, pfdi, npfdi, POLLIN);
      *p = pfdi;
      *n = npfdi;
      }

//---------------------------------------------------------
//   getOutputPollFd
//---------------------------------------------------------

void AlsaMidiDriver::getOutputPollFd(struct pollfd** p, int* n)
      {
      int npfdo = snd_seq_poll_descriptors_count(alsaSeq, POLLOUT);
      struct pollfd* pfdo  = new struct pollfd[npfdo];
      snd_seq_poll_descriptors(alsaSeq, pfdo, npfdo, POLLOUT);
      *p = pfdo;
      *n = npfdo;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void AlsaMidiDriver::read()
      {
      static int active;

      snd_seq_event_t* ev;
      int rv = snd_seq_event_input(alsaSeq, &ev);
      if (rv < 0)
            return;

      if (!mscore->midiinEnabled()) {
            snd_seq_free_event(ev);
            active = 0;
            return;
            }

      if (ev->type == SND_SEQ_EVENT_NOTEON) {
            int pitch   = ev->data.note.note;
            int velo    = ev->data.note.velocity;
            if (velo) {
                  mscore->midiNoteReceived(pitch, active);
                  ++active;
                  }
            else
                  --active;
            }
      else if (ev->type == SND_SEQ_EVENT_NOTEOFF)     // "Virtual Keyboard" sends this
            --active;

      if (midiInputTrace) {
            printf("MidiIn: ");
            switch(ev->type) {
                  case SND_SEQ_EVENT_NOTEON:
                        printf("noteOn ch:%2d 0x%02x 0x%02x\n",
                           ev->data.note.channel,
                           ev->data.note.note,
                           ev->data.note.velocity);
                        break;
                  case SND_SEQ_EVENT_SYSEX:
                        printf("sysEx len %d", ev->data.ext.len);
                        break;
                  case SND_SEQ_EVENT_CONTROLLER:
                        printf("ctrl 0x%02x 0x%02x",
                           ev->data.control.param,
                           ev->data.control.value);
                        break;
                  case SND_SEQ_EVENT_PITCHBEND:
                        printf("pitchbend 0x%04x", ev->data.control.value);
                        break;
                  case SND_SEQ_EVENT_PGMCHANGE:
                        printf("pgmChange 0x%02x", ev->data.control.value);
                        break;
                  case SND_SEQ_EVENT_CHANPRESS:
                        printf("channelPress 0x%02x", ev->data.control.value);
                        break;
                  case SND_SEQ_EVENT_START:
                        printf("start");
                        break;
                  case SND_SEQ_EVENT_CONTINUE:
                        printf("continue");
                        break;
                  case SND_SEQ_EVENT_STOP:
                        printf("stop");
                        break;
                  case SND_SEQ_EVENT_SONGPOS:
                        printf("songpos");
                        break;
                  default:
                        printf("type 0x%02x", ev->type);
                        break;
                  case SND_SEQ_EVENT_PORT_SUBSCRIBED:
                  case SND_SEQ_EVENT_SENSING:
                        break;
                  }
            printf("\n");
            }
      snd_seq_free_event(ev);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void AlsaMidiDriver::write(const MidiOutEvent& e)
      {
      if (midiOutputTrace) {
            // TODO
            }

      int chn = e.type & 0xf;
      int a   = e.a;
      int b   = e.b;

      snd_seq_event_t event;
      memset(&event, 0, sizeof(event));
      snd_seq_ev_set_direct(&event);
      int port = e.port;
      if (port >= preferences.midiPorts)
            port = 0;
      snd_seq_ev_set_source(&event, midiOutPorts[port].alsaPort());
      snd_seq_ev_set_dest(&event, SND_SEQ_ADDRESS_SUBSCRIBERS, 0);

      switch(e.type & 0xf0) {
            case ME_NOTEON:
                  snd_seq_ev_set_noteon(&event, chn, a, b);
                  break;
            case ME_NOTEOFF:
                  snd_seq_ev_set_noteoff(&event, chn, a, 0);
                  break;
            case ME_PROGRAM:
                  snd_seq_ev_set_pgmchange(&event, chn, a);
                  break;
            case ME_CONTROLLER:
                  snd_seq_ev_set_controller(&event, chn, a, b);
                  break;
            case ME_PITCHBEND:
                  snd_seq_ev_set_pitchbend(&event, chn, a);
                  break;
            case ME_POLYAFTER:
                  // chnEvent2(chn, 0xa0, a, b);
                  break;
            case ME_AFTERTOUCH:
                  snd_seq_ev_set_chanpress(&event, chn, a);
                  break;
            case ME_SYSEX:
                  {
#if 0
                  SysexEvent* se         = (SysexEvent*) e;
                  const unsigned char* p = se->data();
                  int n                  = se->len();
                  int len                = n + sizeof(event) + 2;
                  char buf[len];
                  event.type             = SND_SEQ_EVENT_SYSEX;
                  event.flags            = SND_SEQ_EVENT_LENGTH_VARIABLE;
                  event.data.ext.len     = n + 2;
                  event.data.ext.ptr  = (void*)(buf + sizeof(event));
                  memcpy(buf, &event, sizeof(event));
                  char* pp = buf + sizeof(event);
                  *pp++ = 0xf0;
                  memcpy(pp, p, n);
                  pp += n;
                  *pp = 0xf7;
                  putEvent(&event);
#endif
                  return;
                  }
            default:
                  printf("MidiAlsaDriver::putEvent(): event type 0x%02x not implemented\n",
                     e.type & 0xf0);
                  return;
            }
      putEvent(&event);
      }

//---------------------------------------------------------
//   putEvent
//    return false if event is delivered
//---------------------------------------------------------

bool AlsaMidiDriver::putEvent(snd_seq_event_t* event)
      {
      int error;

      do {
            error   = snd_seq_event_output_direct(alsaSeq, event);
            int len = snd_seq_event_length(event);
            if (error == len) {
                  return false;
                  }
            if (error < 0) {
                  if (error == -12) {
                        return true;
                        }
                  else {
                        fprintf(stderr, "MidiAlsaDevice::%p putEvent(): midi write error: %s\n",
                           this, snd_strerror(error));
                        //exit(-1);
                        }
                  }
            else
                  fprintf(stderr, "MidiAlsaDevice::putEvent(): midi write returns %d, expected %d: %s\n",
                     error, len, snd_strerror(error));
            } while (error == -12);
      return true;
      }

#endif /* USE_ALSA */

//---------------------------------------------------------
//   initMidi
//    return true on error
//---------------------------------------------------------

bool initMidi()
      {
#ifdef USE_ALSA
      midiDriver = new AlsaMidiDriver();
      if (!midiDriver->init())
            return true;
#else
      return true;
#endif
      midiOutPorts = new Port[preferences.midiPorts];
      midiInPort = midiDriver->registerOutPort("MuseScore Port-0");
      for (int i = 0; i < preferences.midiPorts; ++i)
            midiOutPorts[i] = midiDriver->registerInPort(QString("MuseScore Port-%1").arg(i));
      return false;
      }

