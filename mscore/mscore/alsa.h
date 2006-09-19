//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: alsa.h,v 1.8 2006/03/03 21:47:11 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __ALSA_H__
#define __ALSA_H

#include "audio.h"

#ifdef USE_ALSA

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API

#include <alsa/asoundlib.h>

//---------------------------------------------------------
//   AlsaDriver
//---------------------------------------------------------

class AlsaDriver {
      QString _name;
      typedef char* (*clear_function)(char*, int, int);
      typedef char* (*play_function)(const float*, char*, int, int);

      enum { MAXPFD = 8, MAXPLAY = 32 };

      int setHwpar(snd_pcm_t* handle, snd_pcm_hw_params_t* hwpar);
      int setSwpar(snd_pcm_t* handle, snd_pcm_sw_params_t* swpar);
      int recover();

      unsigned int           _rate;
      snd_pcm_uframes_t      _frsize;
      unsigned int           _nfrags;
      snd_pcm_format_t       _play_format;
      snd_pcm_access_t       _play_access;
      snd_pcm_t*             _play_handle;
      snd_pcm_hw_params_t*   _play_hwpar;
      snd_pcm_sw_params_t*   _play_swpar;
      unsigned int           _play_nchan;
      int                    _play_npfd;
      struct pollfd          _pfd [MAXPFD];
      snd_pcm_uframes_t      _capt_offs;
      snd_pcm_uframes_t      _play_offs;
      int                    _play_step;
      char*                  _play_ptr [MAXPLAY];
      int                    _stat;
      int                    _pcnt;
      bool                   _xrun;
      clear_function         _clear_func;
      play_function          _play_func;

      static char* clear_32le(char* dst, int step, int nfrm);
      static char* clear_24le(char* dst, int step, int nfrm);
      static char* clear_16le(char* dst, int step, int nfrm);
      static char* play_32le(const float* src, char* dst, int step, int nfrm);
      static char* play_24le(const float* src, char* dst, int step, int nfrm);
      static char* play_16le(const float* src, char* dst, int step, int nfrm);

    public:
      AlsaDriver(QString, unsigned, snd_pcm_uframes_t, unsigned);
      ~AlsaDriver();
      bool init();
      void printinfo();
      int pcmStart();
      int pcmStop();
      snd_pcm_sframes_t pcmWait();
      int pcmIdle(snd_pcm_uframes_t len);
      int playInit(snd_pcm_uframes_t len);
      void playChan(int chan, const float* src, snd_pcm_uframes_t len) {
            _play_ptr[chan] = _play_func(src, _play_ptr[chan], _play_step, len);
            }
      void clearChan(int chan, snd_pcm_uframes_t len) {
            _play_ptr[chan] = _clear_func(_play_ptr[chan], _play_step, len);
            }
      int playDone(snd_pcm_uframes_t len) {
            return snd_pcm_mmap_commit(_play_handle, _play_offs, len);
            }
      int stat() const                { return _stat; }
      int nplay() const               { return _play_nchan; }
      snd_pcm_t* playHandle() const   { return _play_handle; }
      snd_pcm_uframes_t fsize() const { return _frsize;      }
      unsigned int sampleRate() const { return _rate; }
      };

//---------------------------------------------------------
//   AlsaAudio
//---------------------------------------------------------

class AlsaAudio : public Audio {
      pthread_t thread;
      AlsaDriver* alsa;
      bool runAlsa;
      int state;
      bool seekflag;
      unsigned pos;
      double startTime;

      void registerClient();

   public:
      AlsaAudio();
      virtual ~AlsaAudio();
      virtual bool init();
      void* registerPort(const char* name);
      void unregisterPort(void* p);
      virtual std::list<QString> inputPorts();
      virtual bool start();
      virtual bool stop();
      int framePos() const;
      void connect(void*, void*) {}
      void disconnect(void* src, void* dst);
      float* getLBuffer(long n);
      float* getRBuffer(long n);
      virtual bool isRealtime() const   { return false; }
      virtual void startTransport();
      virtual void stopTransport();
      virtual int getState();
      virtual int sampleRate() const;
      void alsaLoop();
      };

#else
//---------------------------------------------------------
//   AlsaAudio
//    dummy
//---------------------------------------------------------

class AlsaAudio : public Audio {

   public:
      AlsaAudio() {}
      virtual ~AlsaAudio() {}
      virtual bool init() { return true; }
      void* registerPort(const char*) { return 0; }
      void unregisterPort(void*) {}
      virtual std::list<QString> inputPorts() {
            std::list<QString> a;
            return a;
            }
      virtual bool start() { return false; }
      virtual bool stop()  { return false; }
      int framePos() const { return 0; }
      void connect(void*, void*) {}
      void disconnect(void*, void*) {}
      virtual bool isRealtime() const   { return false; }
      virtual void startTransport() {}
      virtual void stopTransport()  {}
      virtual int getState()        { return 0; }
      virtual int sampleRate() const { return 48000; }
      };


#endif

extern bool initMidi();
extern int getMidiReadFd();
void readMidiEvent();

#endif

