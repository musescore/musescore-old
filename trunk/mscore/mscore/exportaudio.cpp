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
#include "libmscore/score.h"
#include "fluid.h"
// #include "libmscore/tempo.h"
#include "libmscore/note.h"
#include "musescore.h"
#include "libmscore/part.h"
#include "preferences.h"
#include "seq.h"
#include "libmscore/mscore.h"

//---------------------------------------------------------
//   saveAudio
//---------------------------------------------------------

bool MuseScore::saveAudio(Score* score, const QString& name, const QString& ext)
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
      int sampleRate = preferences.exportAudioSampleRate;

      MasterSynth* synti = new MasterSynth();
      synti->init(sampleRate);
      synti->setState(score->syntiState());

      EventMap events;
      score->toEList(&events);

      SF_INFO info;
      memset(&info, 0, sizeof(info));
      info.channels   = 2;
      info.samplerate = sampleRate;
      info.format     = format;
      SNDFILE* sf     = sf_open(qPrintable(name), SFM_WRITE, &info);
      if (sf == 0) {
            fprintf(stderr, "open soundfile failed: %s\n", sf_strerror(sf));
            delete synti;
            return false;
            }

      QProgressBar* pBar = showProgressBar();
      pBar->reset();

      double peak = 0.0;
      double gain = 1.0;
      for (int pass = 0; pass < 2; ++pass) {
            EventMap::const_iterator playPos;
            playPos = events.constBegin();
            EventMap::const_iterator endPos = events.constEnd();
            --endPos;
            double et = score->utick2utime(endPos.key());
            et += 1.0;   // add trailer (sec)
            pBar->setRange(0, int(et));

            //
            // init instruments
            //
            foreach(const Part* part, *score->parts()) {
                  foreach(const Channel& a, part->instr()->channel()) {
                        a.updateInitList();
                        foreach(Event e, a.init) {
                              if (e.type() == ME_INVALID)
                                    continue;
                              e.setChannel(a.channel);
                              int syntiIdx= score->midiMapping(a.channel)->articulation->synti;
                              synti->play(e, syntiIdx);
                              }
                        }
                  }

            static const unsigned FRAMES = 512;
            float buffer[FRAMES * 2];
            double playTime = 0.0;
            synti->setGain(gain);

            for (;;) {
                  unsigned frames = FRAMES;
                  //
                  // collect events for one segment
                  //
                  memset(buffer, 0, sizeof(float) * FRAMES * 2);
                  double endTime = playTime + double(frames)/double(sampleRate);
                  float* l = buffer;
                  float* r = buffer + 1;
                  for (; playPos != events.constEnd(); ++playPos) {
                        double f = score->utick2utime(playPos.key());
                        if (f >= endTime)
                              break;
                        int n = lrint((f - playTime) * sampleRate);
                        synti->process(n, l, r);

                        l         += n;
                        r         += n;
                        playTime += double(n)/double(sampleRate);
                        frames    -= n;
                        const Event& e = playPos.value();
                        if (e.isChannelEvent()) {
                              int channelIdx = e.channel();
                              Channel* c = score->midiMapping(channelIdx)->articulation;
                              if (!c->mute) {
                                    synti->play(e, c->synti);
                                    }
                              }
                        }
                  if (frames) {
                        synti->process(frames, l, r);
                        playTime += double(frames)/double(sampleRate);
                        }
                  if (pass == 1) {
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
            gain = 0.99 / peak;
            }

      hideProgressBar();

      delete synti;
      if (sf_close(sf)) {
            fprintf(stderr, "close soundfile failed\n");
            return false;
            }

      return true;
      }

#endif // HAS_AUDIOFILE

