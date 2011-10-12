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
#include "scoreview.h"
#include "audio.h"

Seq* seq;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      synti    = new FluidS::Fluid();
      driver   = 0;
      running  = false;
      playPos  = events.constBegin();
      endTick  = 0;
      cs       = 0;
      state    = TRANSPORT_STOP;
      playlistChanged = false;
      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));
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
      driver = 0; // new Portaudio(this);
/*      if (!driver->init()) {
            printf("init audio queue failed\n");
            delete driver;
            driver = 0;
            }
      int sampleRate = driver->sampleRate();
      synti->init(sampleRate);
      if (!driver->start())
            return false;
      running = true;
      */
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
//   setView
//---------------------------------------------------------

void Seq::setView(ScoreView* v)
      {
      view = v;
      }

//---------------------------------------------------------
//   start
//---------------------------------------------------------

void Seq::start()
      {
      if (playlistChanged)
            collectEvents();
      if (!cs)
            return;
      seek(cs->playPos());
      state = TRANSPORT_PLAY;
      if (!heartBeatTimer->isActive())
            heartBeatTimer->start(1000/10);
      }

//---------------------------------------------------------
//   stop
//---------------------------------------------------------

void Seq::stop()
      {
      state = TRANSPORT_STOP;
      cs->setPlayPos(playPos.key());
      heartBeatTimer->stop();
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void Seq::setRelTempo(qreal val)
      {
      SeqMsg msg;
      msg.data.realVal = val;
      msg.id = SEQ_TEMPO_CHANGE;
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
                              int tick = cs->utime2utick(qreal(playTime) / qreal(MScore::sampleRate));
                              cs->tempomap()->setRelTempo(msg.data.realVal);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(tick) * MScore::sampleRate;
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.data.realVal);
                        break;
                  case SEQ_PLAY:
                        synti->play(msg.event);
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data.intVal);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, float* p)
      {
      unsigned frames = n;
      processMessages();
      if (state == TRANSPORT_PLAY) {
            unsigned framePos = 0;
            int endTime = playTime + frames;
            for (; playPos != events.constEnd(); ++playPos) {
                  int f = cs->utick2utime(playPos.key()) * MScore::sampleRate;
                  if (f >= endTime)
                        break;
                  int n = f - playTime;
                  synti->process(n, p);
                  p += 2 * n;
                  playTime += n;
                  frames    -= n;
                  framePos  += n;
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
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int utick)
      {
      stopNotes();

      playTime = cs->utick2utime(utick) * MScore::sampleRate;
      playPos  = events.lowerBound(utick);
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int tick)
      {
      if (cs == 0)
            return;
      cs->setPlayPos(tick);
      tick = cs->repeatList()->tick2utick(tick);

      SeqMsg msg;
      msg.data.intVal = tick;
      msg.id          = SEQ_SEEK;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      events.clear();

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

            if (!mute)
                  synti->play(event);
            }
      else if (type == ME_CONTROLLER)
            synti->play(event);
      }

//---------------------------------------------------------
//   stopNotes
//    called from GUI context
//---------------------------------------------------------

void Seq::stopNotes()
      {
      synti->allNotesOff();
      }

//---------------------------------------------------------
//   heartBeat
//    update GUI
//---------------------------------------------------------

void Seq::heartBeat()
      {
      if (state != TRANSPORT_PLAY)
            return;
      view->moveCursor(cs->repeatList()->utick2tick(playPos.key()));
      }

//---------------------------------------------------------
//   startStop
//---------------------------------------------------------

void Seq::startStop()
      {
      if (isPlaying())
            stop();
      else
            start();
      }

