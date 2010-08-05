//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "synti.h"
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

Seq* seq;

static const int guiRefresh   = 20;         // Hz
static const int peakHoldTime = 1400;     // msec
static const int peakHold     = (peakHoldTime * guiRefresh) / 1000;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      running         = false;
      playlistChanged = false;
      cs              = 0;

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
      heartBeatTimer->stop();

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
      else
            seek(tick);
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
      Synth* synth = driver->getSynth();
      if (synth) {
            QString p;
            if (!preferences.soundFont.isEmpty())
                  p = preferences.soundFont;
            else
                  p = QString(getenv("DEFAULT_SOUNDFONT"));
            if (p.isEmpty()) {
                  QMessageBox::critical(0, tr("MuseScore: Load SoundFont"),
                     tr("No SoundFont configured\n"
                     "Playback will be disabled."));
                  }
            else {
                  if (debugMode)
                        printf("load soundfont <%s>\n", qPrintable(p));
                  bool rv = synth->loadSoundFont(p);
                  if (!rv) {
                        QString s = tr("Loading SoundFont\n"
                           "\"%1\"\n"
                           "failed. Playback will be disabled.\n\n"
                           "Go to Display > Synthesizer \n"
                           "and check that the file location is correct").arg(p);
                        QMessageBox::critical(0, tr("MuseScore: Load SoundFont"), s);
                        }
                  }
            synth->setMasterTuning(preferences.tuning);
            synth->setMasterGain(preferences.masterGain);
            synth->setEffectParameter(0, 0, preferences.reverbRoomSize);
            synth->setEffectParameter(0, 1, preferences.reverbDamp);
            synth->setEffectParameter(0, 2, preferences.reverbWidth);
            synth->setEffectParameter(0, 3, preferences.reverbGain);

            synth->setEffectParameter(1, 4, preferences.chorusGain);
            }

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
      foreach(Note* n, markedNotes) {
            n->setSelected(false);
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
                  heartBeatTimer->stop();
                  if (driver && driver->getSynth() && mscore->getSynthControl()) {
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
                  heartBeatTimer->start(1000/guiRefresh);
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
      foreach(const Event* e, activeNotes) {
            if (e->type() != ME_NOTEON)
                  continue;
            Event ee(*e);
            ee.setVelo(0);
            driver->putEvent(ee, 0);
            }
      // send sustain off
      Event e;
      e.setType(ME_CONTROLLER);
      e.setController(CTRL_SUSTAIN);
      e.setValue(0);
      driver->putEvent(e, 0);

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

void Seq::playEvent(const Event* event, unsigned framePos)
      {
      int type = event->type();
      if (type == ME_NOTEON) {
            bool mute;
            Note* note = event->note();

            if (note) {
                  Part* instr = note->staff()->part();
                  const Channel& a = instr->channel(note->subchannel());
                  mute = a.mute || a.soloMute;
                  }
            else
                  mute = false;

            if (event->velo()) {
                  if (!mute) {
                        driver->putEvent(*event, framePos);
                        activeNotes.append(event);
                        }
                  }
            else {
                  for (QList<const Event*>::iterator k = activeNotes.begin(); k != activeNotes.end(); ++k) {
                        const Event* l = *k;
                        if (l->channel() == event->channel() && l->pitch() == event->pitch()) {
                              Event ee(*l);
                              ee.setVelo(0);
                              activeNotes.erase(k);
                              driver->putEvent(ee, framePos);
                              break;
                              }
                        }
                  }
            }
      else if (type == ME_CONTROLLER)
            driver->putEvent(*event, framePos);
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
                        driver->putEvent(msg.event, 0);
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
                  driver->process(n, l, r, stride);
                  l         += n * stride;
                  r         += n * stride;
                  playTime += double(n)/double(AL::sampleRate);

                  frames    -= n;
                  framePos  += n;
                  playEvent(playPos.value(), framePos);
                  }
            if (frames) {
                  driver->process(frames, l, r, stride);
                  playTime += double(frames)/double(AL::sampleRate);
                  }
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
                  rewindStart();
                  }
            }
      else
            driver->process(frames, l, r, stride);

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
      foreach(const Part* part, *cs->parts()) {
            foreach(const Channel& a, part->channel()) {
                  foreach(Event* e, a.init) {
                        if (e == 0)
                              continue;
                        Event ee(*e);
                        ee.setChannel(a.channel);
                        sendEvent(ee);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      foreach(Event* e, events)
            delete e;
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
      if (state != PLAY)
            return;

      SynthControl* sc = mscore->getSynthControl();
      if (sc && driver && driver->getSynth()) {
            if (++peakTimer[0] >= peakHold)
                  meterPeakValue[0] *= .7f;
            if (++peakTimer[1] >= peakHold)
                  meterPeakValue[1] *= .7f;
            sc->setMeter(meterValue[0], meterValue[1], meterPeakValue[0], meterPeakValue[1]);
            }

      PlayPanel* pp = mscore->getPlayPanel();
      double endTime = curTime() - startTime;
      if (pp)
            pp->heartBeat2(lrint(endTime));
      Note* note = 0;
      for (; guiPos != events.constEnd(); ++guiPos) {
            double f = cs->utick2utime(guiPos.key());
            if (f >= endTime)
                  break;
            if (guiPos.value()->type() == ME_NOTEON) {
                  Event* n = guiPos.value();
                  Note* note1 = n->note();
                  if (n->velo()) {
                        note = note1;
                        while (note1) {
                              note1->setSelected(true);
                              markedNotes.append(note1);
                              cs->addRefresh(note1->abbox());
                              note1 = note1->tieFor() ? note1->tieFor()->endNote() : 0;
                              }

                        }
                  else {
                        while (note1) {
                              note1->setSelected(false);
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
      foreach(const Event* n, activeNotes) {
            if (n->type() != ME_NOTEON)
                  continue;
            Event e(*n);
            e.setVelo(0);
            driver->putEvent(e, 0);
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

      SeqMsg msg;
      msg.data = tick;
      msg.id   = SEQ_SEEK;
      guiToSeq(msg);
      mscore->setPos(tick);
      foreach(Note* n, markedNotes) {
            n->setSelected(false);
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
      foreach(const Event* event, eventList) {
            if (event->channel() == a.channel && event->pitch() == pitch) {
                  sendEvent(*event);
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
            Event* e = new Event(ME_NOTEON);
            e->setChannel(a.channel);
            e->setPitch(pitch);
            e->setVelo(0);
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
//   stopNotes
//    called from GUI context
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event* event, eventList) {
            sendEvent(*event);
            delete event;
            }
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int idx, int ctrl, int data)
      {
      Event event(ME_CONTROLLER);
      event.setChannel(idx);
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
      Note* note = 0;
      for (;;) {
            if (i.value()->type() == ME_NOTEON) {
                  Event* n = i.value();
                  note = n->note();
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
            if (i.value()->type() != ME_NOTEON)
                  continue;
            Event* n = i.value();
            if (i.key() > tick && n->velo()) {
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
      Note* note = 0;
      for (;;) {
            if (i.value()->type() == ME_NOTEON) {
                  note = i.value()->note();
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
            if (i.value()->type() == ME_NOTEON) {
                  Event* n = i.value();
                  if (i.key() < tick && n->velo()) {
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
                  if (i.value()->type() == ME_NOTEON) {
                        Event* n = i.value();
                        if (i.key() < tick && n->velo()) {
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

const QList<MidiPatch*>& Seq::getPatchInfo() const
      {
      static QList<MidiPatch*> pl;
      if (driver && driver->getSynth())
            return driver->getSynth()->getPatchInfo();
      return pl;
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
//   setMasterVolume
//---------------------------------------------------------

void Seq::setMasterVolume(float gain)
      {
      if (driver && driver->getSynth()) {
            emit masterVolumeChanged(gain);
            return driver->getSynth()->setMasterGain(gain);
            }
      }

//---------------------------------------------------------
//   masterVolume
//---------------------------------------------------------

float Seq::masterVolume() const
      {
      if (driver && driver->getSynth())
            return driver->getSynth()->masterGain();
      return 0;
      }


