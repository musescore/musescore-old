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

#include "repeat2.h"

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
      if (driver)
            seek(0);
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (mode != SEL_SINGLE || state != STOP || cs == 0 || driver == 0)
            return;
      int tick = cs->pos();
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
            driver = new DummyAudio(this);
            if (!driver->init()) {
                  printf("init DummyAudio failed\n");
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
            printf("no audio driver\n");
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
//   frame2tick
//---------------------------------------------------------

int Seq::frame2tick(int frame) const
      {
      return cs->tempomap->time2tick(double(frame) / sampleRate());
      }

//---------------------------------------------------------
//   tick2frame
//---------------------------------------------------------

int Seq::tick2frame(int tick) const
      {
      return int(cs->tempomap->tick2time(tick) * sampleRate());
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
            seek(cs->playPos());
            heartBeatTimer->start(100);
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
      if (state == PLAY && pstate)  {
            pauseState = pstate;
            driver->stopTransport();
            }
      else if (state == STOP && pauseState && playState) {
            driver->startTransport();
            }
      pauseState = pstate;
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      setState(STATE_PLAY);
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(true);
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      setState(STATE_NORMAL);
      foreach(Viewer* v, cs->getViewer())
            v->setCursorOn(false);
      cs->start();
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
            heartBeatTimer->stop();
            QAction* a = getAction("play");
            a->setChecked(false);
            }

      // code from heart beat:
      Note* note = 0;
      for (EventMap::const_iterator i = guiPos; i != playPos; ++i) {
            const Event* e = i.value();
            if (e->type() == ME_NOTEON) {
                  NoteOn* no = (NoteOn*)e;
                  no->note()->setSelected(no->velo() != 0);
                  cs->addRefresh(no->note()->abbox());
                  if (no->velo())
                        note = no->note();
                  }
            }
      if (note == 0) {
            for (EventMap::const_iterator i = playPos; i != events.constEnd(); ++i) {
                  const Event* e = i.value();
                  if (e->type() == ME_NOTEON) {
                        note = ((NoteOn*)e)->note();
                        break;
                        }
                  }
            }
      if (note) {
            cs->select(note, 0, 0);
            cs->setPlayPos(note->chord()->tick());
            }
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
                  break;

            case '1':         // PLAY
                  emit started();
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
      foreach(const Event* e, _activeNotes) {
            if (e->type() != ME_NOTEON)
                  continue;
            NoteOn* no = (NoteOn*)e;
            MidiOutEvent e;
            e.time = 0;
            e.port = no->port();
            e.type = ME_NOTEON | no->channel();
            e.a    = no->pitch();
            e.b    = 0;
            driver->putEvent(e);
            no->note()->setSelected(false);
            }
      _activeNotes.clear();
      if (!pauseState)
            emit toGui('0');
      state = STOP;
      }

//---------------------------------------------------------
//   startTranspor
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
      state     = PLAY;
      startTime = curTime();
      }

//---------------------------------------------------------
//   playEvent
//    send one event to the synthesizer
//---------------------------------------------------------

void Seq::playEvent(double t, const Event* event)
      {
      int type = event->type();
      if (type == ME_NOTEON) {
            NoteOn* n = (NoteOn*) event;
            int channel = n->channel();
            MidiOutEvent e;
            e.time = t;
            e.port = n->port();
            e.type = ME_NOTEON | channel;
            e.a    = n->pitch();
            e.b    = n->velo();
            driver->putEvent(e);

            if (n->velo())
                  _activeNotes.append(n);
            else {
                  for (QList<NoteOn*>::iterator k = _activeNotes.begin(); k != _activeNotes.end(); ++k) {
                        NoteOn* l = *k;
                        if (l->channel() == channel && l->pitch() == n->pitch()) {
                              _activeNotes.erase(k);
                              break;
                              }
                        }
                  }
            }
      else if (type == ME_CONTROLLER)  {
            ControllerEvent* c = (ControllerEvent*)event;
            MidiOutEvent e;
            e.time = t;
            e.port = c->port();
            e.type = ME_CONTROLLER | c->channel();
            e.a    = c->controller();
            e.b    = c->value();
            driver->putEvent(e);
            }
      else {
            printf("bad event type %x\n", type);
            }
      }

//---------------------------------------------------------
//   processMidi
//---------------------------------------------------------

void Seq::processMidi()
      {
      int driverState = driver->getState();
      if (state == START_PLAY && driverState == PLAY)
            startTransport();
      else if (state == PLAY && driverState == STOP)
            stopTransport();
      else if (state == START_PLAY && driverState == STOP)
            stopTransport();
      else if (state == STOP && driverState == PLAY)
            startTransport();
      else if (state != driverState)
            printf("Driver: state transition %d -> %d ?\n",
               state, driverState);

      QMutexLocker locker(&mutex);
      while (!toSeq.isEmpty()) {
            SeqMsg msg = toSeq.dequeue();
            switch(msg.id) {
                  case SEQ_TEMPO_CHANGE:
                        {
                        int tick = frame2tick(playFrame);
                        cs->tempomap->setRelTempo(msg.data1);
                        playFrame = tick2frame(tick);
                        }
                        break;

                  case SEQ_PLAY:
                        {
                        int channel = msg.data1 & 0xf;
                        int type    = msg.data1 & 0xf0;
                        if (type == ME_NOTEON) {
                              MidiOutEvent e;
                              e.time = 0;
                              e.port = 0;
                              e.type = ME_NOTEON | channel;
                              e.a    = msg.data2;
                              e.b    = msg.data3;
                              driver->putEvent(e);
                              }
                        else if (type == ME_CONTROLLER) {
                              MidiOutEvent e;
                              e.time = 0;
                              e.port = 0;
                              e.type = ME_CONTROLLER | channel;
                              e.a    = msg.data2;
                              e.b    = msg.data3;
                              driver->putEvent(e);
                              }
                        }
                        break;
                  case SEQ_SEEK:
                        setPos(msg.data1);
                        break;
                  }
            }
      if (state == PLAY) {
            //
            // collect events for one segment
            //
            double endTime = curTime() + (512.0 / preferences.rtcTicks);

            for (; playPos != events.constEnd(); ++playPos) {
                  double t = startTime + tick2time(playPos.key());
                  if (t >= endTime)
                        break;
                  playEvent(t, playPos.value());
                  }
            if (playPos == events.constEnd())
                  driver->stopTransport();
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned n, float* lbuffer, float* rbuffer, int stride)
      {
      int frames = n;
      int jackState = driver->getState();

      if (state == START_PLAY && jackState == PLAY)
            startTransport();
      else if (state == PLAY && jackState == STOP)
            stopTransport();
      else if (state == START_PLAY && jackState == STOP)
            stopTransport();
      else if (state == STOP && jackState == PLAY)
            startTransport();
      else if (state != jackState)
            printf("JACK: state transition %d -> %d ?\n",
               state, jackState);

      float* l = lbuffer;
      float* r = rbuffer;

      QMutexLocker locker(&mutex);
      while (!toSeq.isEmpty()) {
            SeqMsg msg = toSeq.dequeue();
            switch(msg.id) {
                  case SEQ_TEMPO_CHANGE:
                        {
                        int tick = frame2tick(playFrame);
                        cs->tempomap->setRelTempo(msg.data1);
                        playFrame = tick2frame(tick);
                        }
                        break;

                  case SEQ_PLAY:
                        {
                        int channel = msg.data1 & 0xf;
                        int type    = msg.data1 & 0xf0;
                        if (type == ME_NOTEON) {
                              MidiOutEvent e;
                              e.time = 0;
                              e.port = 0;
                              e.type = ME_NOTEON | channel;
                              e.a    = msg.data2;
                              e.b    = msg.data3;
                              driver->putEvent(e);
                              }
                        else if (type == ME_CONTROLLER) {
                              MidiOutEvent e;
                              e.time = 0;
                              e.port = 0;
                              e.type = ME_CONTROLLER | channel;
                              e.a    = msg.data2;
                              e.b    = msg.data3;
                              driver->putEvent(e);
                              }
                        }
                  case SEQ_SEEK:
                        setPos(msg.data1);
                        break;
                  }
            }

      if (state == PLAY) {
            //
            // collect events for one segment
            //
            int endFrame = playFrame + frames;
            for (; playPos != events.constEnd(); ++playPos) {
                  int f = tick2frame(playPos.key());
                  if (f >= endFrame)
                        break;

                  int n = f - playFrame;
                  if (n < 0 || n > int(frames)) {
                        printf("Seq: at %d bad n %d(>%d) = %d - %d\n",
                           playPos.key(), n, frames, f, playFrame);
                        break;
                        }
                  driver->process(n, l, r, stride);
                  l         += n * stride;
                  r         += n * stride;
                  playFrame += n;
                  frames    -= n;
                  playEvent(0, playPos.value());
                  }
            if (frames) {
                  driver->process(frames, l, r, stride);
                  playFrame += frames;
                  }
            if (playPos == events.constEnd()) {
                  driver->stopTransport();
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
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      foreach(Event* e, events)
            delete e;

      events.clear();

      foreach(Part* part, *cs->parts()) {
            int channel = part->midiChannel();
            setController(channel, CTRL_PROGRAM, part->midiProgram());
            setController(channel, CTRL_VOLUME, part->volume());
            setController(channel, CTRL_REVERB_SEND, part->reverb());
            setController(channel, CTRL_CHORUS_SEND, part->chorus());
            setController(channel, CTRL_PANPOT, part->pan());
            }

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
      driver->heartBeat();

      if (guiPos == playPos)
            return;
      cs->start();
      Note* note = 0;
      if (guiPos == events.constEnd()) {
            // special case seek:
            guiPos = playPos;
            if (guiPos.value()->type() == ME_NOTEON) {
                  NoteOn* n = (NoteOn*)guiPos.value();
                  note = n->note();
                  foreach(Viewer* v, cs->getViewer())
                        v->moveCursor(note->chord()->segment());
                  }
            }
      else {
            EventMap::const_iterator e = playPos;
            for (EventMap::const_iterator i = guiPos; i != e; ++i) {
                  if (i.value()->type() == ME_NOTEON) {
                        NoteOn* n = (NoteOn*)i.value();
                        n->note()->setSelected(n->velo());
                        cs->addRefresh(n->note()->abbox());
                        if (n->velo())
                              note = n->note();
                        }
                  }
            }
      if (note) {
            foreach(Viewer* v, cs->getViewer())
                  v->moveCursor(note->chord()->segment());
            PlayPanel* pp = mscore->getPlayPanel();
            if (pp)
                  pp->heartBeat(note->chord()->tick(), guiPos.key());
            }
      cs->setLayoutAll(false);      // DEBUG
      cs->end();
      guiPos = playPos;
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
      msg.data1 = relTempo;
      msg.id    = SEQ_TEMPO_CHANGE;
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
//---------------------------------------------------------

void Seq::setPos(int tick)
      {
      // send note off events
      foreach(const NoteOn* n, _activeNotes) {
            MidiOutEvent e;
            e.time = 0;
            e.port = 0;
            e.type = ME_NOTEON | n->channel();
            e.a    = n->pitch();
            e.b    = 0;
            driver->putEvent(e);
            n->note()->setSelected(false);
            }
      _activeNotes.clear();
      playFrame = tick2frame(tick);
      playPos   = events.lowerBound(tick);
      guiPos    = events.end();     // special case so signal heartBeat a seek
      }

//---------------------------------------------------------
//   seek
//---------------------------------------------------------

void Seq::seek(int tick)
      {
      cs->setPlayPos(tick);
      SeqMsg msg;
      msg.data1 = tick;
      msg.id    = SEQ_SEEK;
      guiToSeq(msg);
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(int channel, int pitch, int velo)
      {
      if (state != STOP)
            return;
      SeqMsg msg;
      msg.data1 = ME_NOTEON | channel;
      msg.data2 = pitch;
      msg.data3 = velo;
      msg.id    = SEQ_PLAY;
      guiToSeq(msg);

      NoteOn* e = new NoteOn;
      e->setChannel(channel);
      e->setPitch(pitch);
      e->setVelo(velo);
      eventList.append(e);
      }

void Seq::startNote(int channel, int pitch, int velo, int duration)
      {
      startNote(channel, pitch, velo);
      playTimer->start(duration);
      }

//---------------------------------------------------------
//   stopNotes
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event* event, eventList) {
            if (event->type() == ME_NOTEON) {
                  NoteOn* n = (NoteOn*)event;
                  SeqMsg msg;
                  msg.data1 = ME_NOTEON | n->channel();
                  msg.data2 = n->pitch();
                  msg.data3 = 0;
                  msg.id    = SEQ_PLAY;
                  guiToSeq(msg);
                  }
            delete event;
            }
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data)
      {
      SeqMsg msg;
      msg.data1 = 0xb0 | channel;
      msg.data2 = ctrl;
      msg.data3 = data;
      msg.id    = SEQ_PLAY;
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
      QMutexLocker locker(&mutex);
      toSeq.enqueue(msg);
      }

//---------------------------------------------------------
//   getPatchInfo
//---------------------------------------------------------

const MidiPatch* Seq::getPatchInfo(int /*ch*/, const MidiPatch* /*p*/)
      {
      return 0;
      }

//---------------------------------------------------------
//   midiInputReady
//---------------------------------------------------------

void Seq::midiInputReady()
      {
      driver->midiRead();
      }

