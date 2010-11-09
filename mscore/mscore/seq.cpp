//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "config.h"
#include "seq.h"
#include "mscore.h"

#ifdef USE_ALSA
#include "alsa.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

#include "msynth/synti.h"
#include "slur.h"
#include "score.h"
#include "segment.h"
#include "note.h"
#include "chord.h"
#include "al/tempo.h"
#include "scoreview.h"
#include "playpanel.h"
#include "staff.h"
#include "measure.h"
#include "preferences.h"
#include "part.h"
#include "ottava.h"
#include "utils.h"
#include "repeatlist.h"
#include "synthcontrol.h"
#include "pianoroll.h"

#ifdef USE_JACK
#include "jackaudio.h"
#endif

#ifdef AEOLUS
#include "aeolus/aeolus/aeolus.h"
#endif

#include "fluid.h"

Seq* seq;

static const int guiRefresh   = 10;       // Hz
static const int peakHoldTime = 1400;     // msec
static const int peakHold     = (peakHoldTime * guiRefresh) / 1000;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      synti = new MasterSynth();

      running         = false;
      playlistChanged = false;
      cs              = 0;
      cv              = 0;

      endTick  = 0;
      state    = STOP;
      driver   = 0;
      playPos  = events.constBegin();

      playTime = 0.0;
      startTime = 0.0;
      curTick   = 0;
      curUtick  = 0;

      meterValue[0]     = 0.0;
      meterValue[1]     = 0.0;
      meterPeakValue[0] = 0.0;
      meterPeakValue[1] = 0.0;
      peakTimer[0]       = 0;
      peakTimer[1]       = 0;

      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));
      heartBeatTimer->start(1000/guiRefresh);
//      heartBeatTimer->stop();

      noteTimer = new QTimer(this);
      noteTimer->setSingleShot(true);
      connect(noteTimer, SIGNAL(timeout()), this, SLOT(stopNotes()));
      noteTimer->stop();

      connect(this, SIGNAL(toGui(int)), this, SLOT(seqMessage(int)), Qt::QueuedConnection);
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
      delete synti;
      delete driver;
      }

//---------------------------------------------------------
//   setScoreView
//---------------------------------------------------------

void Seq::setScoreView(ScoreView* v)
      {
      if (cv !=v && cs) {
            disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
            markedNotes.clear();
            stop();
            cs = v ? v->score() : 0;
#ifndef __MINGW32__
            while (state != STOP)
                  usleep(100000);
#endif
            }
      cv = v;
      cs = cv ? cv->score() : 0;
      playlistChanged = true;
      if (cs) {
            connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
            initInstruments();
            }
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (cs == 0 || driver == 0)
            return;

      int tick = cs->pos();
      if (tick == -1)
            return;

      if ((mode != SEL_LIST) || (state == STOP))
            cs->setPlayPos(tick);
      else {
            seek(tick);
            }
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init()
      {
      driver = 0;

      bool useJackFlag      = preferences.useJackAudio || preferences.useJackMidi;
      bool useAlsaFlag      = preferences.useAlsaAudio;
      bool usePortaudioFlag = preferences.usePortaudioAudio;

#ifdef USE_JACK
      if (useJackFlag) {
            useAlsaFlag      = false;
            usePortaudioFlag = false;
            driver = new JackAudio(this);
            if (!driver->init()) {
                  printf("no JACK server found\n");
                  delete driver;
                  driver = 0;
                  }
            else
                  useJACK = true;
            }
#endif
#ifdef USE_ALSA
      if (driver == 0 && useAlsaFlag) {
            driver = new AlsaAudio(this);
            if (!driver->init()) {
                  printf("init ALSA driver failed\n");
                  delete driver;
                  driver = 0;
                  }
            else {
                  useALSA = true;
                  }
            }
#endif
#ifdef USE_PORTAUDIO
      if (usePortaudioFlag) {
            driver = new Portaudio(this);
            if (!driver->init()) {
                  printf("no audio output found\n");
                  delete driver;
                  driver = 0;
                  }
            else
                  usePortaudio = true;
            }
#endif
      if (driver == 0) {
#if 0
            QString s = tr("Init audio driver failed.\n"
                                "Sequencer will be disabled.");
            QMessageBox::critical(0, "MuseScore: Init Audio Driver", s);
#endif
            printf("init audio driver failed\n");
            return false;
            }
      AL::sampleRate = driver->sampleRate();
      synti->init(AL::sampleRate);

      if (!driver->start()) {
            printf("Cannot start I/O\n");
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
            if (debugMode)
                  printf("Stop I/O\n");
            driver->stop();
            delete driver;
            driver = 0;
            }
      }

//---------------------------------------------------------
//   inputPorts
//---------------------------------------------------------

QList<QString> Seq::inputPorts()
      {
      if (driver)
            return driver->inputPorts();
      QList<QString> a;
      return a;
      }

//---------------------------------------------------------
//   rewindStart
//---------------------------------------------------------

void Seq::rewindStart()
      {
      seek(0);
      }

//---------------------------------------------------------
//   canStart
//    return true if sequencer can be started
//---------------------------------------------------------

bool Seq::canStart()
      {
      if (!driver)
            return false;
      if (events.empty() || cs->playlistDirty() || playlistChanged)
            collectEvents();
      return (!events.empty() && endTick != 0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (events.empty() || cs->playlistDirty() || playlistChanged)
            collectEvents();
      seek(cs->playPos());
      driver->startTransport();
      }

//---------------------------------------------------------
//   stop
//    called from gui thread
//---------------------------------------------------------

void Seq::stop()
      {
      if (state == STOP)
            return;
      if (!driver)
            return;
      driver->stopTransport();
      if (cv)
            cv->setCursorOn(false);
      if (cs) {
            cs->setLayoutAll(false);
            cs->setUpdateAll();
            cs->end();
            }
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      cv->setCursorOn(true);
      cs->end();
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      cv->setCursorOn(false);
#if 0
      cs->setLayoutAll(false);
      cs->setUpdateAll();
      cs->end();
#endif
      }

//---------------------------------------------------------
//   guiStop
//---------------------------------------------------------

void Seq::guiStop()
      {
      QAction* a = getAction("play");
      a->setChecked(false);

      if (!cs) {
            markedNotes.clear();
            return;
            }

      //
      // deselect all selected notes
      //
      foreach(const Note* n, markedNotes) {
            ((Note*)n)->setSelected(false);     //HACK
            cs->addRefresh(n->abbox());
            }
      markedNotes.clear();
      cs->setPlayPos(cs->utime2utick(playTime));
      cs->end();
      emit stopped();
      }

//---------------------------------------------------------
//   seqSignal
//    sequencer message to GUI
//    execution environment: gui thread
//---------------------------------------------------------

void Seq::seqMessage(int msg)
      {
      switch(msg) {
            case '0':         // STOP
                  guiStop();
//                  heartBeatTimer->stop();
                  if (driver && mscore->getSynthControl()) {
                        meterValue[0]     = .0f;
                        meterValue[1]     = .0f;
                        meterPeakValue[0] = .0f;
                        meterPeakValue[1] = .0f;
                        peakTimer[0]       = 0;
                        peakTimer[1]       = 0;
                        mscore->getSynthControl()->setMeter(0.0, 0.0, 0.0, 0.0);
                        }
                  break;

            case '1':         // PLAY
                  emit started();
//                  heartBeatTimer->start(1000/guiRefresh);
                  break;

            default:
                  printf("MScore::Seq:: unknown seq msg %d\n", msg);
                  break;
            }
      }

//---------------------------------------------------------
//   stopTransport
//    JACK has stopped
//    executed in realtime environment
//---------------------------------------------------------

void Seq::stopTransport()
      {
      // send note off events
      foreach(Event e, activeNotes) {
            if (e.type() != ME_NOTEON)
                  continue;
            e.setVelo(0);
            putEvent(e);
            }
      // send sustain off
      Event e;
      e.setType(ME_CONTROLLER);
      e.setController(CTRL_SUSTAIN);
      e.setValue(0);
      putEvent(e);

      activeNotes.clear();
      emit toGui('0');
      state = STOP;
      }

//---------------------------------------------------------
//   startTransport
//    JACK has started
//    executed in realtime environment
//---------------------------------------------------------

void Seq::startTransport()
      {
      emit toGui('1');
      startTime  = curTime() - playTime;
      state      = PLAY;
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const Event& event)
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
                  for (QList<Event>::iterator k = activeNotes.begin(); k != activeNotes.end(); ++k) {
                        Event l = *k;
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
                        {
                        if (playTime != 0) {
                              int tick = cs->utime2utick(playTime);
                              cs->tempomap()->setRelTempo(msg.data);
                              cs->repeatList()->update();
                              playTime = cs->utick2utime(tick);
                              startTime = curTime() - playTime;
                              }
                        else
                              cs->tempomap()->setRelTempo(msg.data);
                        }
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

void Seq::process(unsigned n, float* lbuffer, float* rbuffer, int stride)
      {
      unsigned frames = n;
      int driverState = driver->getState();

      if (driverState != state) {
            if ((state == STOP || state == START_PLAY) && driverState == PLAY)
                  startTransport();
            else if ((state == PLAY || state == START_PLAY) && driverState == STOP)
                  stopTransport();
            else if (state != driverState)
                  printf("Seq: state transition %d -> %d ?\n",
                     state, driverState);
            }

      float* l = lbuffer;
      float* r = rbuffer;
      for (unsigned i = 0; i < n; ++i) {
            *l = 0;
            *r = 0;
            l += stride;
            r += stride;
            }
      l = lbuffer;
      r = rbuffer;

      processMessages();

      if (state == PLAY) {
            //
            // play events for one segment
            //
            unsigned framePos = 0;
            double endTime = playTime + double(frames)/double(AL::sampleRate);
            for (; playPos != events.constEnd(); ++playPos) {
                  double f = cs->utick2utime(playPos.key());
                  if (f >= endTime)
                        break;
                  int n = lrint((f - playTime) * AL::sampleRate);

                  if (n < 0) {
                        printf("%d:  %f - %f\n", playPos.key(), f, playTime);
						n=0;
                        }
                  synti->process(n, l, r, stride);
                  l += n * stride;
                  r += n * stride;
                  playTime += double(n)/double(AL::sampleRate);

                  frames    -= n;
                  framePos  += n;
                  playEvent(playPos.value());
                  }
            if (frames) {
                  synti->process(frames, l, r, stride);
                  playTime += double(frames)/double(AL::sampleRate);
                  }
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
                  rewindStart();
                  }
            }
      else {
            synti->process(frames, l, r, stride);
            }

      if (lbuffer == 0 || rbuffer == 0)   // midi only?
            return;
      //
      // metering
      //
      int k = 0;
      float lv = 0.0f;
      float rv = 0.0f;
      for (unsigned i = 0; i < n; ++i) {
            float val = fabs(lbuffer[k]);
            if (lv < val)
                  lv = val;
            val = fabs(rbuffer[k]);
            if (rv < val)
                  rv = val;
            k += stride;
            }
      meterValue[0] = lv;
      meterValue[1] = rv;
      if (meterPeakValue[0] < lv) {
            meterPeakValue[0] = lv;
            peakTimer[0] = 0;
            }
      if (meterPeakValue[1] < rv) {
            meterPeakValue[1] = rv;
            peakTimer[1] = 0;
            }
      }

//---------------------------------------------------------
//   initInstruments
//---------------------------------------------------------

void Seq::initInstruments()
      {
      foreach(const MidiMapping& mm, *cs->midiMapping()) {
            Channel* channel = mm.articulation;
            foreach(Event e, channel->init) {
                  if (e.type() == ME_INVALID)
                        continue;
                  e.setChannel(channel->channel);
                  sendEvent(e);
                  }
            }
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

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->setEndpos(endTick);
      playlistChanged = false;
      cs->setPlaylistDirty(false);
      }

//---------------------------------------------------------
//   getCurTime
//---------------------------------------------------------

int Seq::getCurTime()
      {
      return (startTime > 0? lrint(curTime() - startTime) :0);
      }

//---------------------------------------------------------
//   getCurTick
//---------------------------------------------------------

void Seq::getCurTick(int* tick, int* utick)
      {
      *tick  = curTick;
      *utick = curUtick;
      }

int Seq::getCurTick()
      {
      return cs->utime2utick(curTime() - startTime);
      }

//---------------------------------------------------------
//   heartBeat
//    update GUI
//---------------------------------------------------------

void Seq::heartBeat()
      {
      SynthControl* sc = mscore->getSynthControl();
      if (sc && driver) {
            if (++peakTimer[0] >= peakHold)
                  meterPeakValue[0] *= .7f;
            if (++peakTimer[1] >= peakHold)
                  meterPeakValue[1] *= .7f;
            sc->setMeter(meterValue[0], meterValue[1], meterPeakValue[0], meterPeakValue[1]);
            }
      if (state != PLAY)
            return;
      PlayPanel* pp = mscore->getPlayPanel();
      double endTime = curTime() - startTime;
      if (pp)
            pp->heartBeat2(lrint(endTime));
      const Note* note = 0;
      for (; guiPos != events.constEnd(); ++guiPos) {
            double f = cs->utick2utime(guiPos.key());
            if (f >= endTime)
                  break;
            if (guiPos.value().type() == ME_NOTEON) {
                  Event n = guiPos.value();
                  const Note* note1 = n.note();
                  if (n.velo()) {
                        note = note1;
                        while (note1) {
                              ((Note*)note1)->setSelected(true);  // HACK
                              markedNotes.append(note1);
                              cs->addRefresh(note1->abbox());
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }

                        }
                  else {
                        while (note1) {
                              ((Note*)note1)->setSelected(false);       // HACK
                              cs->addRefresh(note1->abbox());
                              markedNotes.removeOne(note1);
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }
                        }
                  }
            }
      if (note) {
            mscore->currentScoreView()->moveCursor(note->chord()->segment(), -1);
            cv->adjustCanvasPosition(note, true);
            curTick  = note->chord()->tick();
            curUtick = guiPos.key();
            if (pp)
                  pp->heartBeat(curTick, curUtick);
            mscore->setPos(curTick);
            }
      PianorollEditor* pre = mscore->getPianorollEditor();
      if (pre && pre->isVisible())
            pre->heartBeat(this);
      cs->end();
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void Seq::setRelTempo(double relTempo)
      {
      SeqMsg msg;
      msg.data = lrint(relTempo);
      msg.id   = SEQ_TEMPO_CHANGE;
      guiToSeq(msg);

      double t = cs->tempomap()->tempo(playPos.key()) * relTempo * 0.01;

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            pp->setTempo(t);
            pp->setRelTempo(lrint(relTempo));
            }
      }

//---------------------------------------------------------
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int utick)
      {
      // send note off events
      foreach(Event n, activeNotes) {
            if (n.type() != ME_NOTEON)
                  continue;
            n.setVelo(0);
            putEvent(n);
            }
      activeNotes.clear();

      playTime  = cs->utick2utime(utick);
      startTime = curTime() - playTime;
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
      Segment* seg = cs->tick2segment(tick);
      if (seg) {
            mscore->currentScoreView()->moveCursor(seg, -1);
            }
      cs->setPlayPos(tick);
      cs->end();

      tick = cs->repeatList()->tick2utick(tick);

      SeqMsg msg;
      msg.data = tick;
      msg.id   = SEQ_SEEK;
      guiToSeq(msg);
      mscore->setPos(tick);
      foreach(const Note* n, markedNotes) {
            ((Note*)n)->setSelected(false);     // HACK
            cs->addRefresh(n->abbox());
            }
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(const Channel& a, int pitch, int velo, double nt)
      {
      if (state != STOP)
            return;

      bool active = false;

      //
      // Check if there is already a note sounding
      // for channel/pitch. If found, stop note by
      // sending a note off event
      //
      foreach(const Event& event, eventList) {
            if ((event.channel() == a.channel) && (event.pitch() == pitch)) {
                  sendEvent(event);
                  active = true;
                  break;
                  }
            }

      Event ev(ME_NOTEON);
      ev.setChannel(a.channel);
      ev.setPitch(pitch);
      ev.setTuning(nt);
      ev.setVelo(velo);
      sendEvent(ev);

      if (!active) {
            Event e(ME_NOTEON);
            e.setChannel(a.channel);
            e.setPitch(pitch);
            e.setVelo(0);
            eventList.append(e);
            }
      }

void Seq::startNote(const Channel& a, int pitch, int velo, int duration, double nt)
      {
      stopNotes();
      startNote(a, pitch, velo, nt);
      if (duration) {
            noteTimer->setInterval(duration);
            noteTimer->start();
            }
      }

//---------------------------------------------------------
//   startNoteTimer
//---------------------------------------------------------

void Seq::startNoteTimer(int duration)
      {
      if (duration) {
            noteTimer->setInterval(duration);
            noteTimer->start();
            }
      }

//---------------------------------------------------------
//   stopNotes
//    called from GUI context
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event& event, eventList)
            sendEvent(event);
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data)
      {
      Event event(ME_CONTROLLER);
      event.setChannel(channel);
      event.setController(ctrl);
      event.setValue(data);
      sendEvent(event);
      }

//---------------------------------------------------------
//   sendEvent
//    called from GUI context to send a midi event to
//    midi out or synthesizer
//---------------------------------------------------------

void Seq::sendEvent(const Event& ev)
      {
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.event = ev;
      guiToSeq(msg);
      }

//---------------------------------------------------------
//   nextMeasure
//---------------------------------------------------------

void Seq::nextMeasure()
      {
      EventMap::const_iterator i = playPos;
      const Note* note = 0;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  const Event& n = i.value();
                  note = n.note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->measure();
      m = m->nextMeasure();
      if (m) {
            int rtick = m->tick() - note->chord()->tick();
            seek(playPos.key() + rtick);
            }
      }

//---------------------------------------------------------
//   nextChord
//---------------------------------------------------------

void Seq::nextChord()
      {
      int tick = playPos.key();
      for (EventMap::const_iterator i = playPos; i != events.constEnd(); ++i) {
            if (i.value().type() != ME_NOTEON)
                  continue;
            const Event& n = i.value();
            if (i.key() > tick && n.velo()) {
                  seek(i.key());
                  break;
                  }
            }
      }

//---------------------------------------------------------
//   prevMeasure
//---------------------------------------------------------

void Seq::prevMeasure()
      {
      EventMap::const_iterator i = playPos;
      const Note* note = 0;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  note = i.value().note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      Measure* m = note->chord()->measure();
      m = m->prevMeasure();

      if (m) {
            int rtick = note->chord()->tick() - m->tick();
            seek(playPos.key() - rtick);
            }
      else
            seek(0);
      }

//---------------------------------------------------------
//   prevChord
//---------------------------------------------------------

void Seq::prevChord()
      {
      int tick  = playPos.key();
      //find the chord just before playpos
      EventMap::const_iterator i = playPos;
      for (;;) {
            if (i.value().type() == ME_NOTEON) {
                  const Event& n = i.value();
                  if (i.key() < tick && n.velo()) {
                        tick = i.key();
                        break;
                        }
                  }
            if (i == events.constBegin())
                  break;
            --i;
            }
      //go the previous chord
      if (i != events.constBegin()) {
            i = playPos;
            for (;;) {
                  if (i.value().type() == ME_NOTEON) {
                        const Event& n = i.value();
                        if (i.key() < tick && n.velo()) {
                              seek(i.key());
                              break;
                              }
                        }
                  if (i == events.constBegin())
                        break;
                  --i;
                  }
            }
      }

//---------------------------------------------------------
//   seekEnd
//---------------------------------------------------------

void Seq::seekEnd()
      {
      printf("seek to end\n");
      }

//---------------------------------------------------------
//   guiToSeq
//---------------------------------------------------------

void Seq::guiToSeq(const SeqMsg& msg)
      {
      if (!driver || !running)
            return;
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

QList<MidiPatch*> Seq::getPatchInfo() const
      {
      return synti->getPatchInfo();
      }

//---------------------------------------------------------
//   midiInputReady
//---------------------------------------------------------

void Seq::midiInputReady()
      {
      if (driver)
            driver->midiRead();
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
//   setGain
//---------------------------------------------------------

void Seq::setGain(float gain)
      {
      synti->setGain(gain);
      emit gainChanged(gain);
      }

//---------------------------------------------------------
//   gain
//---------------------------------------------------------

float Seq::gain() const
      {
      return synti->gain();
      }

//---------------------------------------------------------
//   putEvent
//---------------------------------------------------------

void Seq::putEvent(const Event& event)
      {
      int channel = event.channel();
      int syntiIdx= cs->midiMapping(channel)->articulation->synti;
      synti->play(event, syntiIdx);
      }

//---------------------------------------------------------
//   synthNameToIndex
//---------------------------------------------------------

int Seq::synthNameToIndex(const QString& name) const
      {
      return synti->synthNameToIndex(name);
      }

//---------------------------------------------------------
//   synthIndexToName
//---------------------------------------------------------

QString Seq::synthIndexToName(int idx) const
      {
      return synti->synthIndexToName(idx);
      }

