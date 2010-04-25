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

#ifdef __linux__
static const char *options = "htuAJBM:N:S:I:W:d:r:p:n:s:";
#else
static const char *options = "htuJBM:N:S:I:W:s:";
#endif
static char  optline [1024];
static bool  t_opt = false;
static bool  u_opt = false;
static bool  A_opt = false;
static bool  B_opt = false;
static int   r_val = 48000;
static int   p_val = 1024;
static int   n_val = 2;
static const char *N_val = "aeolus";
static const char *I_val = "Aeolus";
static const char *W_val = "waves";
static const char *d_val = "default";
static const char *s_val = 0;
static Lfq_u32  comm_queue (256);
static Iface   *iface = 0;

//---------------------------------------------------------
//   procoptions
//---------------------------------------------------------

static void procoptions (int ac, char *av [], const char *where)
      {
      int k;

      optind = 1;
      opterr = 0;
      while ((k = getopt (ac, av, options)) != -1) {
            if (optarg && (*optarg == '-')) {
                  fprintf (stderr, "\n%s\n", where);
                  fprintf (stderr, "  Missing argument for '-%c' option.\n", k);
                  fprintf (stderr, "  Use '-h' to see all options.\n");
                  exit (1);
                  }
            switch (k) {
 	            case 't' : t_opt = true;  break;
 	            case 'u' : u_opt = true;  break;
 	            case 'A' : A_opt = true;  break;
	            case 'J' : A_opt = false; break;
	            case 'B' : B_opt = true; break;
                  case 'r' : r_val = atoi (optarg); break;
                  case 'p' : p_val = atoi (optarg); break;
                  case 'n' : n_val = atoi (optarg); break;
                  case 'N' : N_val = optarg; break;
                  case 'I' : I_val = optarg; break;
                  case 'W' : W_val = optarg; break;
                  case 'd' : d_val = optarg; break;
                  case 's' : s_val = optarg; break;
                  case '?':
                        fprintf (stderr, "\n%s\n", where);
                        if (optopt != ':' && strchr (options, optopt))
                              fprintf (stderr, "  Missing argument for '-%c' option.\n", optopt);
                        else if (isprint (optopt))
                              fprintf (stderr, "  Unknown option '-%c'.\n", optopt);
                        else
                              fprintf (stderr, "  Unknown option character '0x%02x'.\n", optopt & 255);
                        fprintf (stderr, "  Use '-h' to see all options.\n");
                        exit (1);
                  default:
                        abort ();
                  }
            }
      }

//---------------------------------------------------------
//   readconfig
//---------------------------------------------------------

static int readconfig (const char *path)
      {
      FILE *F;
      char  s [1024];
      char *p;
      char *av [30];
      int   ac;

      av [0] = 0;
      ac = 1;
      if ((F = fopen (path, "r"))) {
            while (fgets (optline, 1024, F) && (ac == 1)) {
                  p = strtok (optline, " \t\n");
                  if (*p == '#')
                        continue;
                  while (p && (ac < 30)) {
		            av [ac++] = p;
                        p = strtok (0, " \t\n");
	                  }
	            }
            fclose (F);
            sprintf (s, "In file '%s':", path);
            procoptions (ac, av, s);
            return 0;
            }
      return 1;
      }

extern Iface *create_iface (int ac, char *av []);

//---------------------------------------------------------
//   main
//---------------------------------------------------------

void Aeolus::init(int /* samplerate */)
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
      char     s [1024];

      //TODO: not crossplatform
      char* p = getenv("HOME");
      if (p)
            sprintf(s, "%s/.aeolusrc", p);
      else
            strcpy(s, ".aeolusrc");
      if (readconfig(s))
            readconfig("/etc/aeolus.conf");
      procoptions (ac, av, "On command line:");
      if (mlockall (MCL_CURRENT | MCL_FUTURE))
            fprintf (stderr, "Warning: memory lock failed.\n");

      audio = new Audio (N_val, note_queue, &comm_queue);
      audio->init_jack (midi_queue);
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
      note_queue = new Lfq_u32(256);
      model = 0;
      slave = 0;
      iface = 0;
      MidiPatch* patch = new MidiPatch;
      patch->drum = false;
      patch->bank = 0;
      patch->prog = 0;
      patch->name = "Aeolus";
      patchList.append(patch);
      }

Aeolus::~Aeolus()
      {
      delete midi_queue;
      delete note_queue;
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
      if (type == ME_NOTEON) {
            int n = event.dataA();
            int v = event.dataB();
            if (v == 0) {   // note off
                  if (midi_queue->write_avail () > 0) {
	                  note_queue->write(0, ((n - 36) << 8) | m);
                        note_queue->write_commit(1);
	                  }
                  }
            else {            // note on
                  if (midi_queue->write_avail () > 0) {
                        note_queue->write(0, (1 << 24) | ((n - 36) << 8) | m);
                        note_queue->write_commit(1);
                        }
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

