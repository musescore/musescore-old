//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "al/tempo.h"
#include "note.h"
#include "mscore.h"
#include "part.h"
#include "preferences.h"

//---------------------------------------------------------
//   saveAudio
//---------------------------------------------------------

bool Score::saveAudio(const QString& name, const QString& ext, QString soundFont)
      {
      int format;
      if (ext == "wav")
            format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
      else if (ext == "ogg")
            format = SF_FORMAT_OGG | SF_FORMAT_VORBIS;
      else if (ext == "flac")
            format = SF_FORMAT_FLAC | SF_FORMAT_PCM_16;
      else {
            fprintf(stderr, "unknown audio file type <%s>\n", qPrintable(ext));
            return false;
            }
      static const int sampleRate = 44100;

      EventMap events;
      toEList(&events);

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

      double peak = 0.0;
      double gain = 1.0;
      for (int pass = 0; pass < 2; ++pass) {
            Synth* synth = new FluidS::Fluid();
            synth->init(sampleRate);

            if (soundFont.isEmpty()) {
                  if (!preferences.soundFont.isEmpty())
                        soundFont = preferences.soundFont;
                  else
                        soundFont = QString(getenv("DEFAULT_SOUNDFONT"));
                  if (soundFont.isEmpty()) {
                        fprintf(stderr, "MuseScore: error: no soundfont configured\n");
                        delete synth;
                        return false;
                        }
                  }
            bool rv = synth->loadSoundFont(soundFont);
            if (!rv) {
                  fprintf(stderr, "MuseScore: error: loading sound font <%s> failed\n", qPrintable(soundFont));
                  delete synth;
                  return false;
                  }
            EventMap::const_iterator playPos;
            playPos = events.constBegin();
            EventMap::const_iterator endPos = events.constEnd();
            --endPos;
            double et = utick2utime(endPos.key());
            et += 1.0;   // add trailer (sec)
            pBar->setRange(0, int(et));

            //
            // init instruments
            //
            foreach(const Part* part, _parts) {
                  foreach(const Channel& a, part->channel()) {
                        a.updateInitList();
                        foreach(Event* e, a.init) {
                              if (e == 0)
                                    continue;
                              e->setChannel(a.channel);
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
                        const Event* e = playPos.value();
                        if (e->isChannelEvent()) {
                              int channelIdx = e->channel();
                              Channel* c = _midiMapping[channelIdx].articulation;
                              if (!c->mute)
                                    synth->play(*e);
                              }
                        }
                  if (frames) {
                        synth->process(frames, l, r, stride);
                        playTime += double(frames)/double(sampleRate);
                        }
                  if (pass == 1) {
                        for (unsigned i = 0; i < FRAMES * 2; ++i)
                              buffer[i] *= gain;
                        sf_writef_float(sf, buffer, FRAMES);
                        }
                  else {
                        for (unsigned i = 0; i < FRAMES * 2; ++i) {
                              if (qAbs(buffer[i]) > peak)
                                    peak = qAbs(buffer[i]);
                              }
                        }
                  playTime = endTime;
                  pBar->setValue(int(playTime));
                  if (playTime > et)
                        break;
                  }
            delete synth;
            gain = 0.99 / peak;
            }

      mscore->hideProgressBar();

      if (sf_close(sf)) {
            fprintf(stderr, "close soundfile failed\n");
            return false;
            }

      return true;
      }

#endif // HAS_AUDIOFILE

