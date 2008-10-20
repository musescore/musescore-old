//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp,v 1.46 2006/03/02 17:08:43 wschweer Exp $
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

#include "seq.h"
#include "mscore.h"
#include "fluid.h"
#ifdef USE_JACK
#include "jackaudio.h"
#endif
#ifdef USE_ALSA
#include "alsa.h"
#include "mididriver.h"
#include "midiseq.h"
#endif
#ifdef USE_PORTAUDIO
#include "pa.h"
#endif

#include "slur.h"
#include "score.h"
#include "segment.h"
#include "note.h"
#include "chord.h"
#include "tempo.h"
#include "canvas.h"
#include "playpanel.h"
#include "staff.h"
#include "measure.h"
#include "layout.h"
#include "preferences.h"
#include "part.h"
#include "ottava.h"
#include "utils.h"

Seq* seq;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      running    = false;
      pauseState = false;

      playlistChanged = false;
      cs = 0;

      endTick  = 0;
      state    = STOP;
      driver   = 0;
      _volume  = 1.0;
      playPos  = events.constBegin();

      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));
      heartBeatTimer->stop();
      playTimer = new QTimer(this);
      playTimer->setSingleShot(true);
      connect(playTimer, SIGNAL(timeout()), this, SLOT(stopNotes()));
      connect(this, SIGNAL(toGui(int)), this, SLOT(seqMessage(int)), Qt::QueuedConnection);
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
      if (driver)
            delete driver;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Seq::setScore(Score* s)
      {
      if (cs) {
            disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
            stop();
#ifndef __MINGW32__
            while (state != STOP)
                  usleep(100000);
#endif
            }
      cs = s;
      playlistChanged = true;
      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      int tick = cs->pos();
      if (mode != SEL_SINGLE || state == STOP || cs == 0 || driver == 0) {
            cs->setPlayPos(tick);
            return;
            }
      if (tick != -1)
            seek(tick);
      }

//---------------------------------------------------------
//   init
//    return false on error
//---------------------------------------------------------

bool Seq::init()
      {
      driver = 0;

      bool useJackFlag      = preferences.useJackAudio;
      bool useAlsaFlag      = preferences.useAlsaAudio;
      bool usePortaudioFlag = preferences.usePortaudioAudio;
      bool useMidiOutFlag   = preferences.useMidiOutput;

      if (useMidiOutFlag) {
            useJackFlag      = false;
            useAlsaFlag      = false;
            usePortaudioFlag = false;
            }

#ifdef USE_JACK
      if (useJackFlag) {
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
            else
                  useALSA = true;
            }
      if (useMidiOutFlag) {
            driver = new AlsaMidi(this);
            if (!driver->init()) {
                  printf("init AlsaMidi failed\n");
                  delete driver;
                  driver = 0;
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
            QString s = QString("Init audio driver failed.\n"
                                "Sequencer will be disabled.");
            QMessageBox::critical(0, "MuseScore: Init Audio Driver", s);
#endif
            printf("init audio driver failed\n");
            return false;
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
            }
      }

//---------------------------------------------------------
//   sampleRate
//---------------------------------------------------------

int Seq::sampleRate() const
      {
      if (driver)
            return driver->sampleRate();
      return 44100;
      }

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int Seq::time2tick(double time) const
      {
      return cs->tempomap->time2tick(time);
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

double Seq::tick2time(int tick) const
      {
      return cs->tempomap->tick2time(tick);
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
//   loadSoundFont
//---------------------------------------------------------

bool Seq::loadSoundFont(const QString& /*s*/)
      {
      return true;      // TODO: synti->loadSoundFont(s);
      }

//---------------------------------------------------------
//   rewindStart
//---------------------------------------------------------

void Seq::rewindStart()
      {
      seek(0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (!driver)
            return;
      QAction* a = getAction("play");
      if (!a->isChecked()) {
            if (pauseState) {
                  guiStop();
                  QAction* a = getAction("pause");
                  a->setChecked(false);
                  pauseState = false;
                  state = STOP;
                  }
            else {
                  driver->stopTransport();
                  }
            }
      else {
            if (events.empty() || cs->playlistDirty() || playlistChanged)
                  collectEvents();
            if (events.empty()) {
                  a->setChecked(false);
                  return;
                  }
            seek(cs->playPos());
            if (!pauseState)
                  driver->startTransport();
            else
                  emit started();
            }
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
      }

//---------------------------------------------------------
//   pause
//    called from gui thread
//---------------------------------------------------------

void Seq::pause()
      {
      if (!driver)
            return;
      QAction* a = getAction("pause");
      int pstate = a->isChecked();
      a = getAction("play");
      int playState = a->isChecked();
      if (state == PLAY && pstate)
            driver->stopTransport();
      else if (state == STOP && pauseState && playState)
            driver->startTransport();
      pauseState = pstate;
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      if (cs->state() != STATE_PLAY)  // don't get stuck in play mode
           cs->setPrevState(cs->state());
      cs->setState(STATE_PLAY);
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(true);
      cs->end();
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      cs->setState(cs->prevState());
      // TODO: there should really be some sort of signal to the viewers
      // instead of the state change being handled here
      bool cursorOn = false;
      if (cs->state() == STATE_NOTE_ENTRY)
            cursorOn = true;
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(cursorOn);
      cs->setLayoutAll(false);
      cs->setUpdateAll();
      cs->end();
      }

//---------------------------------------------------------
//   guiStop
//---------------------------------------------------------

void Seq::guiStop()
      {
      if (!pauseState) {
            QAction* a = getAction("play");
            a->setChecked(false);
            }

      //
      // deselect all selected notes
      //
      foreach(const NoteOn* n, markedNotes) {
            n->note()->setSelected(false);
            cs->addRefresh(n->note()->abbox());
            }
      markedNotes.clear();
      cs->setPlayPos(time2tick(playTime));
      cs->end();
      if (!pauseState)
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
                  break;

            case '1':         // PLAY
                  emit started();
                  heartBeatTimer->start(100);
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
            NoteOn* no = (NoteOn*)e;
            MidiOutEvent e;
            e.port = no->port();
            e.type = ME_NOTEON | no->channel();
            e.a    = no->pitch();
            e.b    = 0;
            driver->putEvent(e);
            }
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
      // dont start transport, if we have nothing to play
      //
      if (endTick == 0)
            return;
      if (!pauseState)
            emit toGui('1');
      startTime = curTime() - playTime;
      state     = PLAY;
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(const Event* event)
      {
      int type = event->type();
      if (type == ME_NOTEON) {
            NoteOn* n = (NoteOn*) event;
            int channel = n->channel();
            MidiOutEvent e;
            e.port = n->port();
            e.type = ME_NOTEON | channel;
            e.a    = n->pitch();
            e.b    = n->velo();

            bool mute;
            Note* note = n->note();
            if (note) {
                  Instrument* instr = note->staff()->part()->instrument();
                  Channel* a = instr->channel[note->subchannel()];
                  mute = a->mute || a->soloMute;
                  }
            else {
                  mute = false;
                  }

            if (n->velo()) {
                  if (!mute) {
                        driver->putEvent(e);
                        activeNotes.append(n);
                        }
                  }
            else {
                  for (QList<NoteOn*>::iterator k = activeNotes.begin(); k != activeNotes.end(); ++k) {
                        NoteOn* l = *k;
                        if (l->channel() == channel && l->pitch() == n->pitch()) {
                              activeNotes.erase(k);
                              driver->putEvent(e);
                              break;
                              }
                        }
                  }
            }
      else if (type == ME_CONTROLLER)  {
            ControllerEvent* c = (ControllerEvent*)event;
            if (c->controller() == CTRL_PROGRAM) {
                  int hb = (c->value() >> 16) & 0xff;
                  int lb = (c->value() >> 8) & 0xff;
                  int pr = c->value() & 0xff;
                  if (hb != 0xff) {
                        MidiOutEvent e;
                        e.port = c->port();
                        e.type = ME_CONTROLLER | c->channel();
                        e.a    = CTRL_HBANK;
                        e.b    = hb;
                        driver->putEvent(e);
                        }
                  if (lb != 0xff) {
                        MidiOutEvent e;
                        e.port = c->port();
                        e.type = ME_CONTROLLER | c->channel();
                        e.a    = CTRL_LBANK;
                        e.b    = lb;
                        driver->putEvent(e);
                        }
                  MidiOutEvent e;
                  e.port = c->port();
                  e.type = ME_PROGRAM | c->channel();
                  e.a    = pr;
                  driver->putEvent(e);
                  }
            else {
                  MidiOutEvent e;
                  e.port = c->port();
                  e.type = ME_CONTROLLER | c->channel();
                  e.a    = c->controller();
                  e.b    = c->value();
                  driver->putEvent(e);
                  }
            }
      else {
            printf("bad event type %x\n", type);
            }
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
                        int tick = time2tick(playTime);
                        cs->tempomap->setRelTempo(msg.data);
                        playTime = tick2time(tick);
                        startTime = curTime() - playTime;
                        }
                        break;
                  case SEQ_PLAY:
                        driver->putEvent(msg.midiOutEvent);
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data);
                        break;
                  }
            }
      }

//---------------------------------------------------------
//   processMidi
//---------------------------------------------------------

void Seq::processMidi()
      {
      int driverState = driver->getState();
      if (driverState != state) {
            if (state == START_PLAY && driverState == PLAY)
                  startTransport();
            else if (state == PLAY && driverState == STOP)
                  stopTransport();
            else if (state == START_PLAY && driverState == STOP)
                  stopTransport();
            else if (state == STOP && driverState == PLAY)
                  startTransport();
            else if (state != driverState)
                  printf("Seq: state transition %d -> %d ?\n",
                     state, driverState);
            }

      processMessages();

      if (state == PLAY) {
            double endTime = curTime();
            for (; playPos != events.constEnd(); ++playPos) {
                  playTime = tick2time(playPos.key());
                  double t = startTime + playTime;
                  if (t >= endTime)
                        break;
                  playEvent(playPos.value());
                  }
            if (playPos == events.constEnd())
                  driver->stopTransport();
                  rewindStart();
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, float* lbuffer, float* rbuffer, int stride)
      {
      int frames = n;
      int driverState = driver->getState();

      if (driverState != state) {
            if (state == START_PLAY && driverState == PLAY)
                  startTransport();
            else if (state == PLAY && driverState == STOP)
                  stopTransport();
            else if (state == START_PLAY && driverState == STOP)
                  stopTransport();
            else if (state == STOP && driverState == PLAY)
                  startTransport();
            else if (state != driverState)
                  printf("Seq: state transition %d -> %d ?\n",
                     state, driverState);
            }

      float* l = lbuffer;
      float* r = rbuffer;

      processMessages();

      if (state == PLAY) {
            //
            // collect events for one segment
            //
            double endTime = playTime + double(frames)/double(sampleRate());
            for (; playPos != events.constEnd(); ++playPos) {
                  double f = tick2time(playPos.key());
                  if (f >= endTime)
                        break;
                  int n = lrint((f - playTime) * sampleRate());
                  driver->process(n, l, r, stride);
                  l         += n * stride;
                  r         += n * stride;
                  playTime += double(n)/double(sampleRate());
                  frames    -= n;
                  playEvent(playPos.value());
                  }
            if (frames) {
                  driver->process(frames, l, r, stride);
                  playTime += double(frames)/double(sampleRate());
                  }
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
                  rewindStart();
                  }
            }
      else
            driver->process(frames, l, r, stride);

      // apply volume:
      for (unsigned i = 0; i < n; ++i) {
            *lbuffer *= _volume;
            *rbuffer *= _volume;
            lbuffer += stride;
            rbuffer += stride;
            }
      }

//---------------------------------------------------------
//   initInstruments
//---------------------------------------------------------
void Seq::initInstruments()
{
      foreach(Part* part, *cs->parts()) {
            const Instrument* instr = part->instrument();

            foreach(const Channel* a, instr->channel) {
                  int idx     = a->channel;
                  int channel = cs->midiChannel(idx);
                  int port    = cs->midiPort(idx);

                  foreach(Event* e, a->init) {
                        if (e == 0)
                              continue;
                        MidiOutEvent event;
                        if (e->midiOutEvent(&event)) {
                              event.port = port;
                              event.type |= channel;
                              sendEvent(event);
                              }
                        else
                              printf("unknown event\n");
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
      initInstruments();

      cs->toEList(&events, 0);

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            if (events.empty())
                  pp->setEndpos(0);
            else {
                  EventMap::const_iterator e = events.constEnd();
                  --e;
                  endTick = e.key();
                  pp->setEndpos(endTick);
                  }
            }
      }

//---------------------------------------------------------
//   heartBeat
//    paint currently sounding notes
//---------------------------------------------------------

void Seq::heartBeat()
      {
      if (state != PLAY)
            return;

      PlayPanel* pp = mscore->getPlayPanel();
      double endTime = curTime() - startTime;
      if (pp)
            pp->heartBeat2(lrint(endTime));
      Note* note = 0;
      for (; guiPos != events.constEnd(); ++guiPos) {
            double f = tick2time(guiPos.key());
            if (f >= endTime)
                  break;
            if (guiPos.value()->type() == ME_NOTEON) {
                  NoteOn* n = (NoteOn*)guiPos.value();
                  n->note()->setSelected(n->velo());
                  cs->addRefresh(n->note()->abbox());
                  if (n->velo()) {
                        markedNotes.append(n);
                        note = n->note();
                        }
                  else {
                        markedNotes.removeAll(n);
                        }
                  }
            }
      if (note) {
            foreach(Viewer* v, cs->getViewer())
                  v->moveCursor(note->chord()->segment());
            cs->adjustCanvasPosition(note, true);
            if (pp)
                  pp->heartBeat(note->chord()->tick(), guiPos.key());
            }
      cs->end();
      }

//---------------------------------------------------------
//   setVolume
//---------------------------------------------------------

void Seq::setVolume(float val)
      {
      _volume = val;
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void Seq::setRelTempo(int relTempo)
      {
      SeqMsg msg;
      msg.data = relTempo;
      msg.id   = SEQ_TEMPO_CHANGE;
      guiToSeq(msg);

      double tempo = cs->tempomap->tempo(playPos.key()) * relTempo * 0.01;

      PlayPanel* pp = mscore->getPlayPanel();
      if (pp) {
            pp->setTempo(tempo);
            pp->setRelTempo(relTempo);
            }
      }

//---------------------------------------------------------
//   setPos
//    seek
//    realtime environment
//---------------------------------------------------------

void Seq::setPos(int tick)
      {
      // send note off events
      foreach(const NoteOn* n, activeNotes) {
            MidiOutEvent e;
            e.port = 0;
            e.type = ME_NOTEON | n->channel();
            e.a    = n->pitch();
            e.b    = 0;
            driver->putEvent(e);
            }
      activeNotes.clear();
      playTime  = tick2time(tick);
      startTime = curTime() - playTime;
      playPos   = events.lowerBound(tick);
      guiPos    = playPos;
      }

//---------------------------------------------------------
//   seek
//    send seek message to sequencer
//---------------------------------------------------------

void Seq::seek(int tick)
      {
      Segment* seg = cs->tick2segment(tick);
      if (seg) {
            foreach(Viewer* v, cs->getViewer())
                  v->moveCursor(seg);
            }
      cs->setPlayPos(tick);
      cs->end();

      SeqMsg msg;
      msg.data = tick;
      msg.id   = SEQ_SEEK;
      guiToSeq(msg);
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(Channel* a, int pitch, int velo)
      {
      if (state != STOP)
            return;

      bool active = false;
      int port    = cs->midiPort(a->channel);
      int channel = cs->midiChannel(a->channel);

      foreach(const Event* event, eventList) {
            NoteOn* n = (NoteOn*)event;
            if (n->port() == port && n->channel() == channel && n->pitch() == pitch) {
                  MidiOutEvent ev;
                  ev.port = port;
                  ev.type = ME_NOTEON | channel;
                  ev.a    = n->pitch();
                  ev.b    = 0;
                  sendEvent(ev);
                  active = true;
                  break;
                  }
            }

      MidiOutEvent ev;
      ev.port = port;
      ev.type = ME_NOTEON | channel;
      ev.a    = pitch;
      ev.b    = velo;
      sendEvent(ev);

      if (!active) {
            NoteOn* e = new NoteOn;
            e->setChannel(channel);
            e->setPort(port);
            e->setPitch(pitch);
            e->setVelo(velo);
            eventList.append(e);
            }
      }

void Seq::startNote(Channel* a, int pitch, int velo, int duration)
      {
      startNote(a, pitch, velo);
      playTimer->start(duration);
      }

//---------------------------------------------------------
//   stopNotes
//    called from GUI context
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event* event, eventList) {
            NoteOn* n = (NoteOn*)event;
            MidiOutEvent ev;
            ev.port = n->port();
            ev.type = ME_NOTEON | n->channel();
            ev.a    = n->pitch();
            ev.b    = 0;
            sendEvent(ev);
            delete event;
            }
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int idx, int ctrl, int data)
      {
      MidiOutEvent event;
      event.port = cs->midiPort(idx);
      event.type = ME_CONTROLLER | cs->midiChannel(idx);
      event.a    = ctrl;
      event.b    = data;
      sendEvent(event);
      }

//---------------------------------------------------------
//   sendEvent
//    called from GUI context to send a midi event to
//    midi out or synthesizer
//---------------------------------------------------------

void Seq::sendEvent(const MidiOutEvent& ev)
      {
      SeqMsg msg;
      msg.id = SEQ_PLAY;
      msg.midiOutEvent = ev;
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
                  NoteOn* n = (NoteOn*)i.value();
                  note = n->note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      MeasureBase* m = note->chord()->segment()->measure();
      do {
            m = m->next();
            } while (m && m->type() != MEASURE);
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
            NoteOn* n = (NoteOn*)i.value();
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
                  note = ((NoteOn*)i.value())->note();
                  break;
                  }
            if (i == events.begin())
                  break;
            --i;
            }
      if (!note)
            return;
      MeasureBase* m = note->chord()->segment()->measure();
      do {
            m = m->prev();
            } while (m && m->type() != MEASURE);

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
      EventMap::const_iterator i = playPos;
      for (;;) {
            if (i.value()->type() == ME_NOTEON) {
                  NoteOn* n = (NoteOn*)i.value();
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

const MidiPatch* Seq::getPatchInfo(int port, int ch, const MidiPatch* p)
      {
      if (driver)
            return driver->getPatchInfo(port, ch, p);
      return 0;
      }

//---------------------------------------------------------
//   midiInputReady
//---------------------------------------------------------

void Seq::midiInputReady()
      {
      if (driver && cs) {
            driver->midiRead();
            }
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
