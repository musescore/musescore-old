/*
    Copyright (C) 2003-2008 Fons Adriaensen <fons@kokkinizita.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "aeolus.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <sys/mman.h>
#include <signal.h>
#include <clthreads.h>
#include <dlfcn.h>
#include "audio.h"
#include "model.h"
#include "slave.h"
#include "iface.h"

// #include "mscore.h"
extern QString dataPath;
extern QString mscoreGlobalShare;

#include "event.h"

static bool  u_opt = false;
static const char *N_val = "aeolus";
static const char *I_val = "Aeolus";
static Lfq_u32 comm_queue (256);
static Iface *iface = 0;

extern Iface *create_iface (int ac, char *av []);

//---------------------------------------------------------
//   main
//---------------------------------------------------------

void Aeolus::init(int samplerate)
      {
      setlocale(LC_ALL, "C"); // scanf of floats does not work otherwise

      int ac = 1;
      char* av[2];
      av[0] = (char*)"aeolus";
      av[1] = 0;

      QString stops = mscoreGlobalShare + "/sound/aeolus/stops";
      int n = strlen(qPrintable(stops));
      char* stopsPath = new char[n+1];
      strcpy(stopsPath, qPrintable(stops));

      QDir dir;
      QString waves = dataPath + "/aeolus/waves";
      dir.mkpath(waves);
      n = strlen(qPrintable(waves));
      char* wavesPath = new char[n+1];
      strcpy(wavesPath, qPrintable(waves));

      ITC_ctrl itcc;

      if (mlockall (MCL_CURRENT | MCL_FUTURE))
            fprintf (stderr, "Warning: memory lock failed.\n");

      audio = new Audio (N_val, &comm_queue);
      audio->init(samplerate);
      model = new Model (&comm_queue, midi_queue, _midimap, audio->appname(),
         stopsPath, I_val, wavesPath, u_opt);

      slave = new Slave ();
      iface = create_iface (ac, av);

      ITC_ctrl::connect (audio, EV_EXIT,  &itcc, EV_EXIT);
      ITC_ctrl::connect (audio, EV_QMIDI, model, EV_QMIDI);
      ITC_ctrl::connect (audio, TO_MODEL, model, FM_AUDIO);

      ITC_ctrl::connect (model, EV_EXIT,  &itcc, EV_EXIT);
      ITC_ctrl::connect (model, TO_AUDIO, audio, FM_MODEL);
      ITC_ctrl::connect (model, TO_SLAVE, slave, FM_MODEL);
      ITC_ctrl::connect (model, TO_IFACE, iface, FM_MODEL);

      ITC_ctrl::connect (slave, EV_EXIT,  &itcc, EV_EXIT);
      ITC_ctrl::connect (slave, TO_AUDIO, audio, FM_SLAVE);
      ITC_ctrl::connect (slave, TO_MODEL, model, FM_SLAVE);

      ITC_ctrl::connect (iface, EV_EXIT,  &itcc, EV_EXIT);
      ITC_ctrl::connect (iface, TO_MODEL, model, FM_IFACE);

      audio->start();

      int audioPriority = 90;

      if (model->thr_start (SCHED_FIFO, audioPriority - 30, 0x00010000)) {
            fprintf (stderr, "Warning: can't run model thread in RT mode.\n");
            model->thr_start (SCHED_OTHER, 0, 0x00010000);
            }
      slave->thr_start (SCHED_OTHER, 0, 0x00010000);
      if (iface->thr_start (SCHED_OTHER, 0, 0x00010000)) {
            fprintf(stderr, "cannot start iface thread\n");
            return;
            }
      M_midi_info* M = new M_midi_info ();
      M->_client = 0;   // _client;
      M->_ipport = 0;   // _ipport;
      memcpy (M->_chbits, _midimap, 16);
      iface->send_event (TO_MODEL, M);
      }

//---------------------------------------------------------
//   Aeolus
//---------------------------------------------------------

Aeolus::Aeolus()
      {
      midi_queue = new Lfq_u8(1024);
      model = 0;
      slave = 0;
      iface = 0;
      MidiPatch* patch = new MidiPatch;
      patch->drum = false;
      patch->bank = 0;
      patch->prog = 0;
      patch->name = "Aeolus";
      patchList.append(patch);
      _sc_cmode = 0;
      _sc_group = 0;
      }

Aeolus::~Aeolus()
      {
      delete midi_queue;
      delete model;
      delete slave;
      delete iface;
      }

void Aeolus::setMasterTuning(double)
      {
      }

double Aeolus::masterTuning() const
      {
      return 440.0;
      }

bool Aeolus::loadSoundFont(const QString&)
      {
      return true;
      }

QString Aeolus::soundFont() const
      {
      return QString();
      }

void Aeolus::process(unsigned len, float* lout, float* rout, int stride)
      {
      audio->process(len, lout, rout, stride);
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void Aeolus::play(const Event& event)
      {
      int ch   = event.channel();
      int type = event.type();
      int m    = _midimap [ch] & 127;        // Keyboard and hold bits
      int f    = (_midimap [ch] >> 12) & 7;  // Control enabled if (f & 4)
      if (type == ME_NOTEON) {
            int n = event.dataA();
            int v = event.dataB();
            if (v == 0) {   // note off
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        audio->key_off(n - 36, m);
                  }
            else {            // note on
                  if (n < 36)
                        ;
                  else if (n <= 96)
                        audio->key_on(n - 36, m);
                  }
            }
      else if (type == ME_CONTROLLER) {
            int p = event.dataA();
            int v = event.dataB();
            switch(p) {
                  case MIDICTL_HOLD:
                  case MIDICTL_ASOFF:
                  case MIDICTL_ANOFF:
                        break;
                  case MIDICTL_BANK:
                        break;
                  case MIDICTL_IFELM:
                        if (!(f & 4))
                              break;
                        if (v & 64) {
                              // Set mode or clear group.
                              _sc_cmode = (v >> 4) & 3;
                              _sc_group = v & 7;
                              if (_sc_cmode == 0)
                                    model->clr_group(_sc_group);
                              }
                        else if (_sc_cmode) {
                              // Set, reset or toggle stop.
                              model->set_ifelm (_sc_group, v & 31, _sc_cmode - 1);
                              }
                        break;
                  }
            }
      }

const QList<MidiPatch*>& Aeolus::getPatchInfo() const
      {
      return patchList;
      }

double Aeolus::masterGain() const
      {
      return 1.0;
      }

void Aeolus::setMasterGain(double)
      {
      }

double Aeolus::effectParameter(int /*effect*/, int /*parameter*/)
      {
      return 0.0;
      }

void Aeolus::setEffectParameter(int /*effect*/, int /*parameter*/, double /*value*/ )
      {
      }

