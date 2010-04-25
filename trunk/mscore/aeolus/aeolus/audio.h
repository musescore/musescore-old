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


#ifndef __AUDIO_H
#define __AUDIO_H


#include <stdlib.h>
#include <clthreads.h>
#include "asection.h"
#include "division.h"
#include "lfqueue.h"
#include "reverb.h"
#include "global.h"

//---------------------------------------------------------
//   Audio
//---------------------------------------------------------

class Audio : public A_thread
      {
      enum { VOLUME, REVSIZE, REVTIME, STPOSIT };

      void init_audio();
      virtual void thr_main();
      void proc_queue(Lfq_u32 *);
      void proc_synth(int);
      void proc_keys1();
      void proc_keys2();
      void proc_mesg();

      void key_off (int n, int b) {
            _keymap [n] &= ~b;
            _keymap [n] |= 128;
            }

      void key_on (int n, int b) {
            _keymap [n] |= b | 128;
            }

      void cond_key_off (int m, int b) {
            unsigned char  *p;
            int i;
            for (i = 0, p = _keymap; i < NNOTES; i++, p++) {
                  if (*p & m) {
                        *p &= ~b;
                        *p |= 128;
                        }
                  }
            }

      void cond_key_on (int m, int b) {
            unsigned char  *p;
            int i;
            for (i = 0, p = _keymap; i < NNOTES; i++, p++) {
                  if (*p & m) {
                        *p |= b | 128;
                        }
                  }
            }

      const char     *_appname;
      uint16_t        _midimap [16];
      Lfq_u32        *_qnote;
      Lfq_u32        *_qcomm;
      Lfq_u8         *_qmidi;
      volatile bool   _running;
      int             _policy;
      int             _abspri;
      int             _relpri;
      int             _jmidi_count;
      int             _jmidi_index;
      void           *_jmidi_pdata;
      int             _hold;
      int             _nplay;
      unsigned int    _fsamp;
      int             _nasect;
      int             _ndivis;
      Asection       *_asectp [NASECT];
      Division       *_divisp [NDIVIS];
      Reverb          _reverb;
      unsigned char   _keymap [NNOTES];
      Fparm           _audiopar [4];
      float           _revsize;
      float           _revtime;

      int nout;
      float routb[PERIOD];
      float loutb[PERIOD];

   public:
      Audio(const char *jname, Lfq_u32 *qnote, Lfq_u32 *qcomm);
      virtual ~Audio();
      void  init_jack(Lfq_u8 *qmidi);
      void  start();

      const char  *appname() const { return _appname; }
      uint16_t    *midimap() const { return (uint16_t *) _midimap; }
      int  policy() const          { return _policy; }
      int  abspri() const          { return _abspri; }
      int  relpri() const          { return _relpri; }
      void process(unsigned len, float* lout, float* rout, int stride);
      };

#endif

