//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: exportly.cpp,v 1.71 2006/03/28 14:58:58 wschweer Exp $
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "config.h"

#ifdef HAS_AUDIOFILE

#include <sndfile.h>
#include "score.h"
#include "fluid.h"
#include "tempo.h"
#include "note.h"
#include "mscore.h"
#include "part.h"
#include "preferences.h"

//---------------------------------------------------------
//   saveAudio
//---------------------------------------------------------

bool Score::saveAudio(const QString& name, int format)
      {
      int sampleRate = 44100;

#ifdef USE_GLOBAL_FLUID
      Synth* synth = new Fluid();
#else
      Synth* synth = new FluidS::Fluid();
#endif

      synth->init(sampleRate, preferences.midiPorts * 16);
      QString p;
      if (!preferences.soundFont.isEmpty())
            p = preferences.soundFont;
      else
            p = QString(getenv("DEFAULT_SOUNDFONT"));
      if (p.isEmpty()) {
            //
            // fallback to integrated soundfont
            //
            p = ":/data/piano1.sf2";
            }
      bool rv = synth->loadSoundFont(p);
      if (!rv) {
            fprintf(stderr, "loading sound font <%s> failed\n", qPrintable(p));
            delete synth;
            return false;
            }
      EventMap events;
      toEList(&events);

      EventMap::const_iterator playPos;
      playPos = events.constBegin();

      SF_INFO info;
      memset(&info, 0, sizeof(info));
      info.channels   = 2;
      info.samplerate = sampleRate;
      info.format     = format;
      SNDFILE* sf     = sf_open(qPrintable(name), SFM_WRITE, &info);
      if (sf == 0) {
            fprintf(stderr, "open soundfile failed: %s\n", sf_strerror(sf));
            return false;
            }

      QProgressBar* pBar = mscore->showProgressBar();
      pBar->reset();
      EventMap::const_iterator endPos = events.constEnd();
      --endPos;
      double et = utick2utime(endPos.key());
      et += 1.0;   // add trailer (sec)
      pBar->setRange(0, int(et));

      //
      // init instruments
      //
      foreach(const Part* part, _parts) {
            const Instrument* instr = part->instrument();

            foreach(const Channel* a, instr->channel) {
                  foreach(Event* e, a->init) {
                        if (e == 0)
                              continue;
                        e->setChannel(a->channel);
                        synth->play(*e);
                        }
                  }
            }

      static const unsigned int FRAMES = 512;
      float buffer[FRAMES * 2];
      int stride      = 2;
      double playTime = 0.0;

      for (;;) {
            unsigned int frames = FRAMES;
            //
            // collect events for one segment
            //
            double endTime = playTime + double(frames)/double(sampleRate);
            float* l = buffer;
            float* r = buffer + 1;
            for (; playPos != events.constEnd(); ++playPos) {
                  double f = utick2utime(playPos.key());
                  if (f >= endTime)
                        break;
                  int n = lrint((f - playTime) * sampleRate);
                  synth->process(n, l, r, stride);

                  l         += n * stride;
                  r         += n * stride;
                  playTime += double(n)/double(sampleRate);
                  frames    -= n;
                  synth->play(*(playPos.value()));
                  }
            if (frames) {
                  synth->process(frames, l, r, stride);
                  playTime += double(frames)/double(sampleRate);
                  }
            sf_writef_float(sf, buffer, FRAMES);
            playTime = endTime;
            pBar->setValue(int(playTime));
            if (playTime > et)
                  break;
            }

      mscore->hideProgressBar();

      if (sf_close(sf)) {
            fprintf(stderr, "close soundfile failed\n");
            return false;
            }

      return true;
      }

//---------------------------------------------------------
//   saveWav
//---------------------------------------------------------

bool Score::saveWav(const QString& name)
      {
      return saveAudio(name, SF_FORMAT_WAV | SF_FORMAT_PCM_16);
      }

//---------------------------------------------------------
//   saveOgg
//---------------------------------------------------------

bool Score::saveOgg(const QString& name)
      {
      return saveAudio(name, SF_FORMAT_OGG | SF_FORMAT_VORBIS);
      }

//---------------------------------------------------------
//   saveFlac
//---------------------------------------------------------

bool Score::saveFlac(const QString& name)
      {
      return saveAudio(name, SF_FORMAT_FLAC | SF_FORMAT_PCM_16);
      }

#endif // HAS_AUDIOFILE

