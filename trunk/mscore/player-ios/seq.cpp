//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp 3693 2010-11-09 17:23:35Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "seq.h"
#include "libmscore/score.h"
#include "libmscore/repeatlist.h"
#include "m-msynth/fluid.h"
#include "libmscore/score.h"
#include "libmscore/note.h"
#include "libmscore/staff.h"
#include "libmscore/part.h"
#include "libmscore/chord.h"
#include "libmscore/segment.h"
#include "libmscore/measure.h"
#include "libmscore/system.h"
#include "libmscore/tempo.h"
#include "audioqueue.h"

Seq* seq;

//---------------------------------------------------------
//   curTime
//---------------------------------------------------------

static qreal curTime()
      {
      struct timeval t;
      gettimeofday(&t, 0);
      return (qreal)((qreal)t.tv_sec + (t.tv_usec / 1000000.0));
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      synti    = new FluidS::Fluid();
      driver   = 0;
      running  = false;
      playPos  = events.constBegin();
      playTime = 0;
      cs       = 0;
      state    = TRANSPORT_STOP;
      playlistChanged = false;
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
      delete synti;
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init()
      {
      driver = new QueueAudio(this);
      if (!driver->init()) {
            delete driver;
            driver = 0;
            }
      int sampleRate = driver->sampleRate();
      synti->init(sampleRate);
      if (!driver->start()) {
            return false;
            }
      running = true;
      return true;
      }

//---------------------------------------------------------
//   exit
//---------------------------------------------------------

void Seq::exit()
      {
      if (driver) {
            driver->stop();
            delete driver;
            driver = 0;
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Seq::setScore(Score* s)
      {
      cs = s;
      playlistChanged = true;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void Seq::start()
      {
      if (playlistChanged)
            collectEvents();
      seek(cs->playPos());
      state = TRANSPORT_PLAY;
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void Seq::stop()
      {
      state    = TRANSPORT_STOP;
      int tick = 0;
      if (playPos != events.constEnd())
            tick = playPos.key();
      cs->setPlayPos(tick);
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void Seq::setRelTempo(float val)
      {
      SeqMsg msg;
      msg.data = lrint(val);
      msg.id   = SEQ_TEMPO_CHANGE;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   processMessages
//---------------------------------------------------------

void Seq::processMessages()
      {
      for (;;) {
            if (toSeq.isEmpty())
                  break;
            SeqMsg msg = toSeq.dequeue();
            switch(msg.id) {
                  case SEQ_TEMPO_CHANGE:
                        if (playTime != 0) {
                              int tick = cs->utime2utick(playTime / MScore::sampleRate);
                              cs->tempomap()->setRelTempo(msg.data);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(tick) * MScore::sampleRate;
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.data);
                        break;
                  case SEQ_PLAY:
                        putEvent(msg.event);
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, short* p)
      {
      unsigned frames = n;

      processMessages();
      if (state == TRANSPORT_PLAY) {
            int endTime = playTime + frames;
            for (; playPos != events.constEnd(); ++playPos) {
                  int f = cs->utick2utime(playPos.key()) * MScore::sampleRate;
                  if (f >= endTime)
                        break;
                  int n = f - playTime;
                  if (n < 0) {
                        printf("n < 0\n");
                        break;
                        }
                  synti->process(n, p);
                  p += 2 * n;
                  playTime  += n;
                  frames    -= n;
                  playEvent(playPos.value());
                  }
            if (frames) {
                  synti->process(frames, p);
                  playTime += frames;
                  }
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
                  seek(0);
                  state = TRANSPORT_STOP;
                  }
            }
      else {
            synti->process(frames, p);
            }
      }

//---------------------------------------------------------
//   sendEvent
//    called from GUI context to send a midi event to
//    midi out or synthesizer
//---------------------------------------------------------

void Seq::sendEvent(const SeqEvent& ev)
      {
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.event = ev;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

SeqMsgFifo::SeqMsgFifo()
      {
      maxCount = SEQ_MSG_FIFO_SIZE;
      clear();
      }

//---------------------------------------------------------
//   enqueue
//---------------------------------------------------------

void SeqMsgFifo::enqueue(const SeqMsg& msg)
      {
      if (isFull()) {
            printf("SeqMsgFifo: overflow\n");
            return;
            }
      messages[widx] = msg;
      push();
      }

//---------------------------------------------------------
//   dequeue
//---------------------------------------------------------

SeqMsg SeqMsgFifo::dequeue()
      {
      SeqMsg msg = messages[ridx];
      pop();
      return msg;
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void Seq::putEvent(const SeqEvent& event)
      {
      synti->play(event);
      }

//---------------------------------------------------------
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int utick)
      {
      // send note off events
      foreach(SeqEvent n, activeNotes) {
            if (n.type() != ME_NOTEON)
                  continue;
            n.setVelo(0);
            putEvent(n);
            }
      activeNotes.clear();

      playTime  = cs->utick2utime(utick) * MScore::sampleRate;
      playPos   = events.lowerBound(utick);
      guiPos    = playPos;
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int tick)
      {
      if (cs == 0)
            return;
//      Segment* seg = cs->tick2segment(tick);
//      if (seg) {
//            mscore->currentScoreView()->moveCursor(seg, -1);
//            }
      cs->setPlayPos(tick);

      tick = cs->repeatList()->tick2utick(tick);

      SeqMsg msg;
      msg.data = tick;
      msg.id   = SEQ_SEEK;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      events.clear();
      activeNotes.clear();

      cs->toEList(&events);
      endTick = 0;
      if (!events.empty()) {
            EventMap::const_iterator e = events.constEnd();
            --e;
            endTick = e.key();
            }
      playlistChanged = false;
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const SeqEvent& event)
      {
      int type = event.type();
      if (type == ME_NOTEON) {
            bool mute;
            const Note* note = event.note();

            if (note) {
                  Instrument* instr = note->staff()->part()->instr();
                  const Channel& a = instr->channel(note->subchannel());
                  mute = a.mute || a.soloMute;
                  }
            else
                  mute = false;

            if (event.velo()) {
                  if (!mute) {
                        putEvent(event);
                        activeNotes.append(event);
                        }
                  }
            else {
                  for (QList<SeqEvent>::iterator k = activeNotes.begin(); k != activeNotes.end(); ++k) {
                        SeqEvent l = *k;
                        if (l.channel() == event.channel() && l.pitch() == event.pitch()) {
                              l.setVelo(0);
                              activeNotes.erase(k);
                              putEvent(l);
                              break;
                              }
                        }
                  }
            }
      else if (type == ME_CONTROLLER)
            putEvent(event);
      }

//---------------------------------------------------------
//   heartBeat
//    update GUI
//---------------------------------------------------------

QRectF Seq::heartBeat(int* pageIdx, bool* stopped)
      {
      QRectF r;
      if (state != TRANSPORT_PLAY) {
            *stopped = true;
            r.setWidth(0.0);
            return r;
            }
      *stopped = false;
#if 0
      qreal endTime = curTime() - startTime;
      const Note* note = 0;
      for (; guiPos != events.constEnd(); ++guiPos) {
            qreal f = cs->utick2utime(guiPos.key());
            if (f >= endTime)
                  break;
            if (guiPos.value().type() == ME_NOTEON) {
                  SeqEvent n = guiPos.value();
                  const Note* note1 = n.note();
                  if (n.velo()) {
                        note = note1;
                        }
                  }
            }
      if (note) {
            Page* page = note->chord()->segment()->measure()->system()->page();
            *pageIdx = cs->pageIdx(page);
//            r = cs->moveCursor(note->chord()->segment());
            }
      else
            r.setWidth(.0);
#endif
      return r;
      }

