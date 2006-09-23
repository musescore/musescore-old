//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: alsa.cpp,v 1.14 2006/03/02 17:08:30 wschweer Exp $
//
//  AlsaDriver based on code from Fons Adriaensen (clalsadr.cc)
//    Copyright (C) 2003 Fons Adriaensen
//  partly based on original work from Paul Davis
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include <sys/time.h>

#include "config.h"
#include "alsa.h"
#include "score.h"
#include "mscore.h"
#include "preferences.h"
#include "seq.h"

#ifdef USE_ALSA

static snd_seq_t* alsaSeq;
static int alsaSeqFdi = -1;
static int alsaSeqFdo = -1;
static snd_seq_addr_t musePort;

//---------------------------------------------------------
//   AlsaDriver
//---------------------------------------------------------

AlsaDriver::AlsaDriver(QString s, unsigned rate,
   snd_pcm_uframes_t frsize, unsigned nfrags)
      {
      _name        = s;
      _play_handle = 0;
      _play_hwpar  = 0;
      _play_swpar  = 0;
      _play_npfd   = 0;
      _rate        = rate;
      _frsize      = frsize;
      _nfrags      = nfrags;
      _stat        = -1;
      _play_nchan  = 2;
      }

//---------------------------------------------------------
//   init
//    return true on success
//---------------------------------------------------------

bool AlsaDriver::init()
      {
      if (snd_pcm_open (&_play_handle, _name.toLatin1().data(), SND_PCM_STREAM_PLAYBACK, 0) < 0) {
            _play_handle = 0;
            fprintf (stderr, "Alsa_driver: Cannot open PCM device %s for playback.\n",
               _name.toLatin1().data());
            return false;
            }

      // check capabilities here

      if (snd_pcm_hw_params_malloc (&_play_hwpar) < 0) {
            fprintf (stderr, "Alsa_driver: can't allocate playback hw params\n");
            return false;
            }

      if (snd_pcm_sw_params_malloc (&_play_swpar) < 0) {
            fprintf (stderr, "Alsa_driver: can't allocate playback sw params\n");
            return false;
            }
      if (setHwpar(_play_handle, _play_hwpar) < 0)
            return false;
      if (setSwpar(_play_handle, _play_swpar) < 0)
            return false;
      int dir;
      unsigned rate = _rate;
      if (snd_pcm_hw_params_get_rate (_play_hwpar, &_rate, &dir) || (rate != _rate) || dir) {
            fprintf (stderr, "Alsa_driver: can't get requested sample rate for playback.\n");
            return false;
            }

      unsigned frsize = _frsize;
      if (snd_pcm_hw_params_get_period_size (_play_hwpar, &_frsize, &dir) || (frsize != _frsize) || dir) {
            fprintf (stderr, "Alsa_driver: can't get requested period size for playback.\n");
            return false;
            }

      unsigned nfrags = _nfrags;
      if (snd_pcm_hw_params_get_periods (_play_hwpar, &_nfrags, &dir) || (nfrags != _nfrags) || dir) {
            fprintf (stderr, "Alsa_driver: can't get requested number of periods for playback.\n");
            return false;
            }

      snd_pcm_hw_params_get_format (_play_hwpar, &_play_format);
      snd_pcm_hw_params_get_access (_play_hwpar, &_play_access);

      switch (_play_format) {
            case SND_PCM_FORMAT_S32_LE:
                  _clear_func = clear_32le;
                  _play_func  = play_32le;
                  break;
            case SND_PCM_FORMAT_S24_3LE:
                  _clear_func = clear_24le;
                  _play_func  = play_24le;
                  break;
            case SND_PCM_FORMAT_S16_LE:
                  _clear_func = clear_16le;
                  _play_func  = play_16le;
                  break;
            default:
                  fprintf (stderr, "Alsa_driver: can't handle playback sample format.\n");
                  return false;
            }
      _play_npfd = snd_pcm_poll_descriptors_count (_play_handle);
      if (_play_npfd > MAXPFD) {
            fprintf (stderr, "Alsa_driver: interface requires more than %d pollfd\n", MAXPFD);
            return false;
            }
      _stat = 0;
      return true;
      }

//---------------------------------------------------------
//   ~AlsaDriver
//---------------------------------------------------------

AlsaDriver::~AlsaDriver()
      {
      snd_pcm_sw_params_free (_play_swpar);
      snd_pcm_hw_params_free (_play_hwpar);

      if (_play_handle)
            snd_pcm_close(_play_handle);
      }

//---------------------------------------------------------
//   pcmStart
//---------------------------------------------------------

int AlsaDriver::pcmStart()
      {
      int err;
      unsigned n = snd_pcm_avail_update (_play_handle);
      if (n != _frsize * _nfrags) {
            fprintf  (stderr, "Alsa_driver: full buffer not available at start.\n");
            return -1;
            }
      for (unsigned i = 0; i < _nfrags; i++) {
            playInit (_frsize);
            for (unsigned j = 0; j < _play_nchan; j++)
                  clearChan (j, _frsize);
            playDone (_frsize);
            }
      if ((err = snd_pcm_start (_play_handle)) < 0) {
            fprintf (stderr, "Alsa_driver: pcm_start(play): %s.\n", snd_strerror (err));
            return -1;
            }
      return 0;
      }

//---------------------------------------------------------
//   pcmStop
//---------------------------------------------------------

int AlsaDriver::pcmStop()
      {
      int err;

      if (_play_handle && ((err = snd_pcm_drop (_play_handle)) < 0)) {
            fprintf (stderr, "Alsa_driver: pcm_drop(play): %s\n", snd_strerror (err));
            return -1;
            }
      return 0;
      }

//---------------------------------------------------------
//   pcmWait
//---------------------------------------------------------

snd_pcm_sframes_t AlsaDriver::pcmWait()
      {
      _pcnt = 0;
      _stat = 0;
      _xrun = false;
      bool need_play = true;

      while (need_play) {
            _pcnt++;
            int n = 0;
            if (need_play) {
                  snd_pcm_poll_descriptors(_play_handle, &_pfd[0], _play_npfd);
                  n += _play_npfd;
                  }

            for (int i = 0; i < n; i++)
                  _pfd[i].events |= POLLERR;

            errno = 0;
            if (poll(_pfd, n, 1000000) < 0) {
                  if (errno == EINTR) {
                        _stat = 1;
                        return 0;
                        }
                  fprintf (stderr, "Alsa_driver: poll(): %s\n.", strerror (errno));
                  _stat = 2;
                  return 0;
                  }

            int i = 0;
            int play_to = 0;

            if (need_play) {
                  for (; i < _play_npfd; i++) {
                        if (_pfd[i].revents & POLLERR) {
                              _xrun = true;
                              _stat |= 4;
                              }
                        if (_pfd[i].revents == 0)
                              play_to++;
                        }
                  if (!play_to)
                        need_play = false;
                  }

            if ((play_to && (play_to == _play_npfd))) {
                  fprintf (stderr, "Alsa_driver: poll timed out\n.");
                  _stat |= 16;
                  return 0;
                  }
            }
      snd_pcm_sframes_t play_av = snd_pcm_avail_update(_play_handle);
      if (play_av < 0) {
            _xrun = true;
            _stat |= 64;
            }
      if (_xrun) {
            recover();
            return 0;
            }
      return play_av;
      }

//---------------------------------------------------------
//   pcmIdle
//---------------------------------------------------------

int AlsaDriver::pcmIdle(snd_pcm_uframes_t len)
      {
      snd_pcm_uframes_t n, k;

      if (_play_handle) {
            n = len;
            while (n) {
                  k = playInit (n);
                  for (unsigned i = 0; i < _play_nchan; i++)
                        clearChan (i, k);
                  playDone (k);
                  n -= k;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   playInit
//---------------------------------------------------------

int AlsaDriver::playInit(snd_pcm_uframes_t len)
      {
      int err;
      const snd_pcm_channel_area_t* a;

      if ((err = snd_pcm_mmap_begin (_play_handle, &a, &_play_offs, &len)) < 0) {
            fprintf (stderr, "Alsa_driver: snd_pcm_mmap_begin(play): %s.\n", snd_strerror (err));
            return -1;
            }
      _play_step = (a->step) >> 3;
      for (unsigned i = 0; i < _play_nchan; i++, a++) {
            _play_ptr[i] = (char *)a->addr + ((a->first + a->step * _play_offs) >> 3);
            }
      return len;
      }

//---------------------------------------------------------
//   printinfo
//---------------------------------------------------------

void AlsaDriver::printinfo()
      {
      fprintf (stderr, "\n  nchan  : %d\n", _play_nchan);
      fprintf (stderr, "  rate   : %d\n", _rate);
      fprintf (stderr, "  frsize : %ld\n", _frsize);
      fprintf (stderr, "  nfrags : %d\n", _nfrags);
      fprintf (stderr, "  format : %s\n", snd_pcm_format_name (_play_format));
      }

//---------------------------------------------------------
//   setHwpar
//---------------------------------------------------------

int AlsaDriver::setHwpar(snd_pcm_t* handle, snd_pcm_hw_params_t* hwpar)
      {
      int err;

      if ((err = snd_pcm_hw_params_any(handle, hwpar)) < 0) {
            fprintf (stderr, "Alsa_driver: no hw configurations available: %s.\n", snd_strerror (err));
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_periods_integer (handle, hwpar)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set period size to integral value.\n");
            return -1;
            }

      if (((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_NONINTERLEAVED)) < 0)
         && ((err = snd_pcm_hw_params_set_access (handle, hwpar, SND_PCM_ACCESS_MMAP_INTERLEAVED)) < 0)) {
            fprintf (stderr, "Alsa_driver: the interface doesn't support mmap-based access.\n");
            return -1;
            }

      if (((err = snd_pcm_hw_params_set_format (handle, hwpar, SND_PCM_FORMAT_S32)) < 0)
         && ((err = snd_pcm_hw_params_set_format (handle, hwpar, SND_PCM_FORMAT_S24_3LE)) < 0)
         && ((err = snd_pcm_hw_params_set_format (handle, hwpar, SND_PCM_FORMAT_S16)) < 0)) {
            fprintf (stderr, "Alsa_driver: the interface doesn't support 32, 24 or 16 bit access.\n.");
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_rate (handle, hwpar, _rate, 0)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set sample rate to %u.\n", _rate);
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_channels (handle, hwpar, _play_nchan)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set channel count to %u.\n",
               _play_nchan);
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_period_size (handle, hwpar, _frsize, 0)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set period size to %lu: %s\n",
               _frsize, snd_strerror(err));
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_periods (handle, hwpar, _nfrags, 0)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set periods to %u.\n", _nfrags);
            return -1;
            }

      if ((err = snd_pcm_hw_params_set_buffer_size (handle, hwpar, _frsize * _nfrags)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set buffer length to %lu.\n", _frsize * _nfrags);
            return -1;
            }

      if ((err = snd_pcm_hw_params (handle, hwpar)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set hardware parameters.\n");
            return -1;
            }
      return 0;
      }

//---------------------------------------------------------
//   setSwpar
//---------------------------------------------------------

int AlsaDriver::setSwpar(snd_pcm_t *handle, snd_pcm_sw_params_t *swpar)
      {
      int err;

      snd_pcm_sw_params_current(handle, swpar);
      if ((err = snd_pcm_sw_params_set_silence_size(handle, swpar, 0)) < 0) {
            fprintf(stderr, "AlsaDriver: can't set timestamp mode to %u.\n",
               SND_PCM_TSTAMP_MMAP);
            return -1;
            }

      if ((err = snd_pcm_sw_params_set_avail_min(handle, swpar, _frsize)) < 0) {
            fprintf(stderr, "AlsaDriver: can't set availmin to %lu.\n", _frsize);
            return -1;
            }
      if ((err = snd_pcm_sw_params(handle, swpar)) < 0) {
            fprintf (stderr, "Alsa_driver: can't set software parameters.\n");
            return -1;
            }
      return 0;
      }

//---------------------------------------------------------
//   recover
//---------------------------------------------------------

int AlsaDriver::recover()
      {
      int                err;
      snd_pcm_status_t  *stat;

      snd_pcm_status_alloca (&stat);

      if ((err = snd_pcm_status (_play_handle, stat)) < 0) {
            fprintf (stderr, "Alsa_driver: pcm_status(): %s\n",  snd_strerror (err));
            }
      else if (snd_pcm_status_get_state (stat) == SND_PCM_STATE_XRUN) {
            struct timeval tnow, trig;

            gettimeofday (&tnow, 0);
            snd_pcm_status_get_trigger_tstamp (stat, &trig);
            fprintf (stderr, "Alsa_driver: stat = %02x, xrun of at least %8.3lf ms\n", _stat,
               1e3 * tnow.tv_sec - 1e3 * trig.tv_sec + 1e-3 * tnow.tv_usec - 1e-3 * trig.tv_usec);
            }
      if (pcmStop())
            return -1;
      if (_play_handle && ((err = snd_pcm_prepare (_play_handle)) < 0)) {
            fprintf (stderr, "Alsa_driver: pcm_prepare(play): %s\n", snd_strerror (err));
            return -1;
            }
      if (pcmStart ())
            return -1;
      return 0;
      }

//---------------------------------------------------------
//   clear_16le
//---------------------------------------------------------

char* AlsaDriver::clear_16le (char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            *((short int *) dst) = 0;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   play_16le
//---------------------------------------------------------

char* AlsaDriver::play_16le (const float* src, char* dst, int step, int nfrm)
      {
      float     s;
      short int d;

      while (nfrm--) {
            s = *src++;
            if (s >  1)
                  d = 0x7fff;
            else if (s < -1)
                  d = 0x8001;
            else
                  d = (short int)(0x7fff * s);
            *((short int *) dst) = d;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   clear_24le
//---------------------------------------------------------

char* AlsaDriver::clear_24le(char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            dst [0] = 0;
            dst [1] = 0;
            dst [2] = 0;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   play_24le
//---------------------------------------------------------

char* AlsaDriver::play_24le(const float* src, char* dst, int step, int nfrm)
      {
      float   s;
      int     d;

      while (nfrm--) {
            s = *src++;
            if (s >  1)
                  d = 0x007fffff;
            else if (s < -1)
                  d = 0x00800001;
            else
                  d = (int)(0x007fffff * s);
            dst [0] = d;
            dst [1] = d >> 8;
            dst [2] = d >> 16;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   clear_32le
//---------------------------------------------------------

char* AlsaDriver::clear_32le(char* dst, int step, int nfrm)
      {
      while (nfrm--) {
            *((int *) dst) = 0;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   play_32le
//---------------------------------------------------------

char* AlsaDriver::play_32le(const float* src, char* dst, int step, int nfrm)
      {
      float   s;
      int     d;

      while (nfrm--) {
            s = *src++;
            if (s >  1)
                  d = 0x007fffff;
            else if (s < -1)
                  d = 0x00800001;
            else
                  d = (int)(0x007fffff * s);
            *((int *) dst) = d << 8;
            dst += step;
            }
      return dst;
      }

//---------------------------------------------------------
//   curTime
//---------------------------------------------------------

static double curTime()
      {
      struct timeval t;
      gettimeofday(&t, 0);
      return (double)((double)t.tv_sec + (t.tv_usec / 1000000.0));
      }

//---------------------------------------------------------
//   AlsaAudio
//---------------------------------------------------------

AlsaAudio::AlsaAudio()
      {
      alsa      = 0;
      state     = Seq::STOP;
      seekflag  = false;
      startTime = curTime();
      }

//---------------------------------------------------------
//   sampleRate
//---------------------------------------------------------

int AlsaAudio::sampleRate() const
      {
      if (alsa)
            return alsa->sampleRate();
      else
            return preferences.alsaSampleRate;
      }

//---------------------------------------------------------
//   AlsaAudio
//---------------------------------------------------------

AlsaAudio::~AlsaAudio()
      {
      }

//---------------------------------------------------------
//   init
//    return true on error
//---------------------------------------------------------

bool AlsaAudio::init()
      {
printf("AlsaAudio init\n");
      alsa = new AlsaDriver(
         preferences.alsaDevice,
         preferences.alsaSampleRate,
         preferences.alsaPeriodSize,
         preferences.alsaFragments);
      if (!alsa->init()) {
            delete alsa;
            alsa = 0;
            fprintf(stderr, "init ALSA audio driver failed\n");
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   alsaLoop
//---------------------------------------------------------

static void* alsaLoop(void* alsa)
      {
      ((AlsaAudio*)alsa)->alsaLoop();
      return 0;
      }

//---------------------------------------------------------
//   alsaLoop
//---------------------------------------------------------

void AlsaAudio::alsaLoop()
      {
      //
      // try to get realtime priviledges
      //
      struct sched_param rt_param;
      memset(&rt_param, 0, sizeof(rt_param));
      rt_param.sched_priority = 50;
      int rv = pthread_setschedparam(pthread_self(), SCHED_FIFO, &rt_param);
      if (rv == -1)
            perror("MuseScore: set realtime scheduler failed");
      else
            printf("MuseScore: running SCHED_FIFO\n");

      alsa->pcmStart();
      snd_pcm_uframes_t size = alsa->fsize();
      float lbuffer[size];
      float rbuffer[size];
      while (runAlsa) {
            snd_pcm_sframes_t k = alsa->pcmWait();
            while (k >= int(size)) {
                  alsa->playInit(size);
                  seq->process(size, lbuffer, rbuffer);
                  alsa->playChan(0, lbuffer, size);
                  alsa->playChan(1, rbuffer, size);
                  alsa->playDone(size);
                  k -= size;
                  }
            }
      alsa->pcmStop();
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

bool AlsaAudio::start()
      {
      runAlsa = true;
      pthread_attr_t* attributes = (pthread_attr_t*) malloc(sizeof(pthread_attr_t));
      pthread_attr_init(attributes);
      if (pthread_create(&thread, attributes, ::alsaLoop, this))
            perror("creating thread failed:");
      pthread_attr_destroy(attributes);
      return false;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

bool AlsaAudio::stop()
      {
      runAlsa = false;
      sleep(1);
      pthread_cancel(thread);
      pthread_join(thread, 0);
      return false;
      }

//---------------------------------------------------------
//   registerPort
//---------------------------------------------------------

void* AlsaAudio::registerPort(const char* /*name*/)
      {
      return 0;
      }

//---------------------------------------------------------
//   unregisterPort
//---------------------------------------------------------

void AlsaAudio::unregisterPort(void*)
      {
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

std::list<QString> AlsaAudio::inputPorts()
      {
      std::list<QString> l;
      return l;
      }

//---------------------------------------------------------
//   framePos
//---------------------------------------------------------

int AlsaAudio::framePos() const
      {
      return lrint((curTime()-startTime) * sampleRate());
      }

//---------------------------------------------------------
//   disconnect
//---------------------------------------------------------

void AlsaAudio::disconnect(void* /*src*/, void* /*dst*/)
      {
      }

//---------------------------------------------------------
//   startTransport
//---------------------------------------------------------

void AlsaAudio::startTransport()
      {
      state = Seq::PLAY;
      }

//---------------------------------------------------------
//   stopTransport
//---------------------------------------------------------

void AlsaAudio::stopTransport()
      {
      state = Seq::STOP;
      }

//---------------------------------------------------------
//   getState
//---------------------------------------------------------

int AlsaAudio::getState()
      {
      return state;
      }

//---------------------------------------------------------
//   MidiDevice
//---------------------------------------------------------

class MidiDevice {
      snd_seq_addr_t adr;
      QString name;
      int flags;

   public:
      MidiDevice(snd_seq_addr_t adr, const QString& name);
      void setrwFlags(int f)  { flags = f; }
      bool hasInput() const   { return (flags & 2); }
      void openInput();
      };

typedef std::list<MidiDevice*> MidiDeviceList;
typedef MidiDeviceList::iterator iMidiDevice;

MidiDeviceList midiDevices;

//---------------------------------------------------------
//   MidiDevice
//---------------------------------------------------------

MidiDevice::MidiDevice(snd_seq_addr_t a, const QString& s)
   : name(s)
      {
      flags = 0;
      adr   = a;
      }

//---------------------------------------------------------
//   openInput
//---------------------------------------------------------

void MidiDevice::openInput()
      {
      if (hasInput()) {
            snd_seq_port_subscribe_t* subs;
            snd_seq_port_subscribe_alloca(&subs);
            snd_seq_port_subscribe_set_dest(subs, &musePort);
            snd_seq_port_subscribe_set_sender(subs, &adr);
            snd_seq_subscribe_port(alsaSeq, subs);
            }
      }

//---------------------------------------------------------
//   getMidiReadFd
//---------------------------------------------------------

int getMidiReadFd()
      {
      return alsaSeqFdi;
      }

//---------------------------------------------------------
//   initMidi
//    return true on error
//---------------------------------------------------------

bool initMidi()
      {
      int error = snd_seq_open(&alsaSeq, "hw", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK);
      if (error < 0) {
            fprintf(stderr, "Could not open ALSA sequencer: %s\n",
               snd_strerror(error));
            return true;
            }
      const int inCap  = SND_SEQ_PORT_CAP_SUBS_READ;
      const int outCap = SND_SEQ_PORT_CAP_SUBS_WRITE;

      snd_seq_client_info_t *cinfo;
      snd_seq_client_info_alloca(&cinfo);
      snd_seq_client_info_set_client(cinfo, -1);

      while (snd_seq_query_next_client(alsaSeq, cinfo) >= 0) {
            snd_seq_port_info_t *pinfo;
            snd_seq_port_info_alloca(&pinfo);
            snd_seq_port_info_set_client(pinfo, snd_seq_client_info_get_client(cinfo));
            snd_seq_port_info_set_port(pinfo, -1);

            while (snd_seq_query_next_port(alsaSeq, pinfo) >= 0) {
                  unsigned capability = snd_seq_port_info_get_capability(pinfo);
                  if ((capability & outCap) == 0)
                        continue;
                  snd_seq_addr_t adr = *snd_seq_port_info_get_addr(pinfo);
                  MidiDevice* dev = new MidiDevice(adr, QString(snd_seq_port_info_get_name(pinfo)));
                  int flags = 0;
                  if (capability & outCap)
                        flags |= 1;
                  if (capability & inCap)
                        flags |= 2;
                  dev->setrwFlags(flags);
                  midiDevices.push_back(dev);
                  }
            }
      snd_seq_set_client_name(alsaSeq, "MuseScore");
      int ci = snd_seq_poll_descriptors_count(alsaSeq, POLLIN);
      int co = snd_seq_poll_descriptors_count(alsaSeq, POLLOUT);

      if (ci > 1 || co > 1) {
            printf("ALSA midi: cannot handle more than one poll fd\n");
            abort();
            }

      struct pollfd pfdi[ci];
      struct pollfd pfdo[co];
      snd_seq_poll_descriptors(alsaSeq, pfdi, ci, POLLIN);
      snd_seq_poll_descriptors(alsaSeq, pfdo, co, POLLOUT);
      alsaSeqFdo = pfdo[0].fd;
      alsaSeqFdi = pfdi[0].fd;

      int port  = snd_seq_create_simple_port(alsaSeq, "MuseScore Port 0",
         inCap | outCap | SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_WRITE,
         SND_SEQ_PORT_TYPE_APPLICATION);
      if (port < 0) {
            perror("create port");
            exit(1);
            }
      musePort.port   = port;
      musePort.client = snd_seq_client_id(alsaSeq);

      //-----------------------------------------
      //    subscribe to "Announce"
      //    this enables callbacks for any
      //    alsa port changes
      //-----------------------------------------

      snd_seq_addr_t aadr;
      aadr.client = SND_SEQ_CLIENT_SYSTEM;
      aadr.port   = SND_SEQ_PORT_SYSTEM_ANNOUNCE;

      snd_seq_port_subscribe_t* subs;
      snd_seq_port_subscribe_alloca(&subs);
      snd_seq_port_subscribe_set_dest(subs, &musePort);
      snd_seq_port_subscribe_set_sender(subs, &aadr);
      error = snd_seq_subscribe_port(alsaSeq, subs);
      if (error < 0) {
            printf("Alsa: Subscribe System failed: %s", snd_strerror(error));
            return true;
            }

      // subscribe all devices for reading
      for (iMidiDevice i = midiDevices.begin(); i != midiDevices.end(); ++i) {
            MidiDevice* dev = *i;
            dev->openInput();
            }

      return false;
      }

//---------------------------------------------------------
//   midiReadEvent
//---------------------------------------------------------

snd_seq_event* midiReadEvent()
      {
      snd_seq_event_t* ev;
      int rv = snd_seq_event_input(alsaSeq, &ev);
      if (rv < 0)
            return 0;
      return ev;
      }

//---------------------------------------------------------
//   midiFreeEvent
//---------------------------------------------------------

void midiFreeEvent(snd_seq_event* ev)
      {
      snd_seq_free_event(ev);
      }

//---------------------------------------------------------
//    readMidiEvent
//---------------------------------------------------------

void readMidiEvent()
      {
      static int active;
      snd_seq_event_t* ev = midiReadEvent();
      if (ev && mscore->midiinEnabled()) {
            if (ev->type == SND_SEQ_EVENT_NOTEON) {
//                  int channel = ev->data.note.channel;
                  int pitch   = ev->data.note.note;
                  int velo    = ev->data.note.velocity;
                  if (velo) {
                        mscore->midiNoteReceived(pitch, active);
                        ++active;
                        }
                  else
                        --active;
                  }
            midiFreeEvent(ev);
            }
      }

#else
//
// some dummy Routines
//
bool initMidi()      { return true; }
int getMidiReadFd()  { return -1; }
void readMidiEvent() {}

#endif

