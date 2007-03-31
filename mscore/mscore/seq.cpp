//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: seq.cpp,v 1.46 2006/03/02 17:08:43 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "jackaudio.h"
#include "alsa.h"
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

enum {
      ME_NOTEOFF    = 0x80,
      ME_NOTEON     = 0x90,
      ME_POLYAFTER  = 0xa0,
      ME_CONTROLLER = 0xb0,
      ME_PROGRAM    = 0xc0,
      ME_AFTERTOUCH = 0xd0,
      ME_PITCHBEND  = 0xe0,
      ME_SYSEX      = 0xf0,
      ME_META       = 0xff,
      ME_SONGPOS    = 0xf2,
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,
      };


Seq* seq;

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::Seq()
      {
      running  = false;
      cs = 0;

#ifndef __MINGW32__
      endTick  = 0;
      state    = STOP;
      synti    = new ISynth();
      audio    = 0;
      _volume  = 1.0;
      playPos  = events.begin();

      //---------------------------------------------------
      //  pipe for asynchronous gui to sequencer
      //  messages
      //---------------------------------------------------

      int filedes[2];         // 0 - reading   1 - writing
      if (pipe(filedes) == -1) {
            perror("creating pipe0");
            ::exit(-1);
            }
      // pipe for gui -> sequencer
      fromThreadFdr = filedes[0];
      fromThreadFdw = filedes[1];
      int rv = fcntl(fromThreadFdw, F_SETFL, O_NONBLOCK);
      if (rv == -1)
            perror("set pipe write O_NONBLOCK");
      rv = fcntl(fromThreadFdr, F_SETFL, O_NONBLOCK);
      if (rv == -1)
            perror("set pipe read O_NONBLOCK");

      //
      // pipe for asynchronous sequencer to gui messages
      //
      if (pipe(filedes) == -1) {
            perror("creating pipe1");
            ::exit(-1);
            }
      sigFd = filedes[1];
      QSocketNotifier* ss = new QSocketNotifier(filedes[0], QSocketNotifier::Read);
      connect(ss, SIGNAL(activated(int)), this, SLOT(seqMessage(int)));
      heartBeatTimer = new QTimer(this);
      connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(heartBeat()));
      heartBeatTimer->stop();
#endif
      }

//---------------------------------------------------------
//   Seq
//---------------------------------------------------------

Seq::~Seq()
      {
//      delete synti;
//      delete audio;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Seq::setScore(Score* s)
      {
      if (cs) {
            disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
            stop();
            while (state != STOP)
                  usleep(100000);
            }
      cs = s;
      connect(cs, SIGNAL(selectionChanged(int)), SLOT(selectionChanged(int)));
      if (audio)
            setPos(0);
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void Seq::selectionChanged(int mode)
      {
      if (mode != SEL_SINGLE || state != STOP || cs == 0 || audio == 0)
            return;
      int tick = cs->pos();
      if (tick != -1)
            setPos(tick);
      }

//---------------------------------------------------------
//   isRealtime
//---------------------------------------------------------

bool Seq::isRealtime() const
      {
      return audio->isRealtime();
      }

//---------------------------------------------------------
//   init
//    return true on error
//---------------------------------------------------------

bool Seq::init()
      {
      audio = 0;
      if (preferences.useJackAudio) {
            audio = new JackAudio;
            if (audio->init()) {
                  printf("no JACK server found\n");
                  delete audio;
                  audio = 0;
                  }
            }
      if (audio == 0 && preferences.useAlsaAudio) {
            audio = new AlsaAudio;
            if (audio->init()) {
                  printf("no ALSA audio found\n");
                  delete audio;
                  audio = 0;
                  }
            }
      if (audio == 0)
            return true;
      if (synti->init(audio->sampleRate())) {
            printf("Synti init failed\n");
            return true;
            }
      audio->start();
      running = true;
      return false;
      }

//---------------------------------------------------------
//   exit
//---------------------------------------------------------

void Seq::exit()
      {
      if (audio) {
            audio->stop();
            }
      }

//---------------------------------------------------------
//   sampleRate
//---------------------------------------------------------

int Seq::sampleRate() const
      {
      return audio->sampleRate();
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
//   inputPorts
//---------------------------------------------------------

std::list<QString> Seq::inputPorts()
      {
      return audio->inputPorts();
      }

//---------------------------------------------------------
//   loadSoundFont
//---------------------------------------------------------

bool Seq::loadSoundFont(const QString& s)
      {
      return synti->loadSoundFont(s);
      }

//---------------------------------------------------------
//   rewindStart
//---------------------------------------------------------

void Seq::rewindStart()
      {
      setPos(0);
      }

//---------------------------------------------------------
//   start
//    called from gui thread
//---------------------------------------------------------

void Seq::start()
      {
      if (events.empty() || cs->playlistDirty()) {
            collectEvents();
            setPos(playTick);
            }
      audio->startTransport();
      heartBeatTimer->start(1000/20);
      }

//---------------------------------------------------------
//   stop
//    called from gui thread
//---------------------------------------------------------

void Seq::stop()
      {
      audio->stopTransport();
      }

//---------------------------------------------------------
//   setStop
//    called from button toggled
//---------------------------------------------------------

void MuseScore::setStop(bool f)
      {
      if (!f) {
            getAction("stop")->setChecked(true);
            if (playPanel)
                  playPanel->setStop(true);
            }
      else {
            seq->stop();
            }
      }

//---------------------------------------------------------
//   setPlay
//    called from button toggled
//---------------------------------------------------------

void MuseScore::setPlay(bool f)
      {
      if (!f) {
            getAction("play")->setChecked(true);
            if (playPanel)
                  playPanel->setPlay(true);
            }
      else {
            seq->start();
            }
      }

//---------------------------------------------------------
//   seqStarted
//---------------------------------------------------------

void MuseScore::seqStarted()
      {
      disconnect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));

      QAction* a = getAction("stop");
      a->blockSignals(true);
      a->setChecked(false);
      a->blockSignals(false);

      a = getAction("play");
      a->blockSignals(true);
      a->setChecked(true);
      a->blockSignals(false);

      if (playPanel) {
            playPanel->setStop(false);
            playPanel->setPlay(true);
            }
      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->enableSeek(false);
      }

//---------------------------------------------------------
//   seqStopped
//    JACK has stopped
//    executed in gui environment
//---------------------------------------------------------

void MuseScore::seqStopped()
      {
      QAction* a = getAction("stop");
      a->blockSignals(true);
      a->setChecked(true);
      a->blockSignals(false);
      a = getAction("play");
      a->blockSignals(true);
      a->setChecked(false);
      a->blockSignals(false);
      if (playPanel) {
            playPanel->setStop(true);
            playPanel->setPlay(false);
            }
      seq->activeNotes().clear();
      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->enableSeek(true);
      connect(cs, SIGNAL(selectionChanged(int)), this, SLOT(selectionChanged(int)));
      }

//---------------------------------------------------------
//   seqSignal
//    sequencer message to GUI
//    execution environment: gui thread
//---------------------------------------------------------

void Seq::seqMessage(int fd)
      {
      char buffer[16];

      int n = ::read(fd, buffer, 16);
      if (n < 0) {
            printf("MScore::Seq: seqMessage(): READ PIPE failed: %s\n",
               strerror(errno));
            return;
            }
      for (int i = 0; i < n; ++i) {
            switch(buffer[i]) {
                  case '0':         // STOP
                        heartBeatTimer->stop();
                        if (!events.empty()) {
                              iEvent e = events.end();
                              --e;
                              cs->select(e->second.note, 0, 0);
                              }
                        emit stopped();
                        break;
                  case '1':         // PLAY
                        emit started();
                        break;
                  default:
                        printf("MScore::Seq:: unknown seq msg %d\n", buffer[i]);
                        break;
                  }
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
      for (iEvent i = _activeNotes.begin(); i != _activeNotes.end(); ++i) {
//            printf("stop note %d\n", i->second.val1);
            synti->playNote(i->second.channel, i->second.val1, 0);
            }
      toGui('0');
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
      if (endTick) {
            toGui('1');
            state = PLAY;
            }
      }

//---------------------------------------------------------
//   process
//---------------------------------------------------------

void Seq::process(unsigned frames, float* lbuffer, float* rbuffer)
      {
      int jackState = audio->getState();
      if (state == START_PLAY && jackState == PLAY) {
            startTransport();
            }
      else if (state == PLAY && jackState == STOP) {
            stopTransport();
            }
      else if (state == START_PLAY && jackState == STOP) {
            stopTransport();
            }
      else if (state == STOP && jackState == PLAY) {
            startTransport();
            }
      else if (state != jackState)
            printf("JACK: state transition %d -> %d ?\n",
               state, jackState);

      float* l = lbuffer;
      float* r = rbuffer;

      EList pe;   // events to play in this cycle

      //
      // read messages from gui
      //
      SeqMsg msg;
      while (read(fromThreadFdr, &msg, sizeof(SeqMsg)) == sizeof(SeqMsg)) {
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
                        Event ev;
                        ev.type    = msg.data1 & 0xf0;
                        ev.channel = msg.data1 & 0xf;
                        ev.val1    = msg.data2;
                        ev.val2    = msg.data3;
                        ev.note    = 0;
                        pe.insert(std::pair<const unsigned, Event> (0, ev));
                        }
                        break;
                  default:
                        printf("unknown seq msg %d\n", msg.id);
                        break;
                  }
            }

      bool eof = false;
      if (state == PLAY) {
            //
            // collect events
            //
            ciEvent s        = playPos;
            int endFrameTick = frame2tick(playFrame + frames);
            ciEvent e        = events.lower_bound(endFrameTick);
            for (ciEvent i = s; i != e; ++i)
                  pe.insert(*i);
            eof = (e == events.end());
            playPos = e;

            int cpos  = playFrame;
            int ctick = frame2tick(cpos);
            for (ciEvent i = pe.begin(); i != pe.end(); ++i) {
                  if (ctick < i->first) {
                        int f = tick2frame(i->first);
                        int n = f - cpos;
                        synti->process(n, l, r);
                        l += n;
                        r += n;
                        cpos = f;
                        }
                  int channel = i->second.channel;
                  int type    = i->second.type;
                  if (type == 0x90) {
                        int key     = (channel << 8) | i->second.val1;
                        if (i->second.val2) {
                              // note on:
                              _activeNotes.insert(std::pair<const int, Event> (key, i->second));
                              }
                        else {
                              // note off; dont "play" note off until a note on was send
                              aEvent ia = _activeNotes.find(key);
                              if (ia == _activeNotes.end()) {
                                    printf("note %d not found\n", key & 0xff);
                                    continue;
                                    }
                              _activeNotes.erase(ia);
                              }
                        synti->playNote(channel, i->second.val1, i->second.val2);
                        }
                  else if (type == 0xb0)
                        synti->setController(channel, i->second.val1, i->second.val2);
                  else
                        printf("bad event type %x\n", type);

                  }
            playFrame += frames;
            if (cpos < playFrame)
                  synti->process(playFrame - cpos, l, r);
            if (eof)
                  audio->stopTransport();
            }
      else {
            for (ciEvent i = pe.begin(); i != pe.end(); ++i) {
                  int channel = i->second.channel;
                  int type    = i->second.type;
                  if (type == 0x90)
                        synti->playNote(channel, i->second.val1, i->second.val2);
                  else if (type == 0xb0)
                        synti->setController(channel, i->second.val1, i->second.val2);
                  }
            synti->process(frames, l, r);
            }

      // apply volume:
      for (unsigned i = 0; i < frames; ++i) {
            lbuffer[i] *= _volume;
            rbuffer[i] *= _volume;
            }
      }

//---------------------------------------------------------
//   collectEvents
//---------------------------------------------------------

void Seq::collectEvents()
      {
      events.clear();

      int staffIdx = 0;
      int gateTime = 80;  // 100 - legato (100%)

      foreach(Part* part, *cs->parts()) {
            int channel = part->midiChannel();
            setController(channel, CTRL_PROGRAM, part->midiProgram());
            setController(channel, CTRL_VOLUME, part->volume());
            setController(channel, CTRL_REVERB_SEND, part->reverb());
            setController(channel, CTRL_CHORUS_SEND, part->chorus());
            setController(channel, CTRL_PAN, part->pan());

            for (int i = 0; i < part->staves()->size(); ++i) {
                  QList<OttavaE> ol;
                  for (Measure* m = cs->mainLayout()->first(); m; m = m->next()) {
                        foreach(Element* e, *m->el()) {
                              if (e->type() == OTTAVA) {
                                    Ottava* ottava = (Ottava*)e;
                                    OttavaE oe;
                                    oe.offset = ottava->pitchShift();
                                    oe.start  = ottava->tick();
                                    oe.end    = ottava->tick2();
                                    ol.append(oe);
                                    }
                              }
                        }
                  for (Measure* m = cs->mainLayout()->first(); m; m = m->next()) {
                        for (int voice = 0; voice < VOICES; ++voice) {
                              for (Segment* seg = m->first(); seg; seg = seg->next()) {
                                    Element* el = seg->element(staffIdx * VOICES + voice);
                                    if (!el || el->type() != CHORD)
                                          continue;
                                    Chord* chord = (Chord*)el;
                                    NoteList* nl = chord->noteList();

                                    for (iNote in = nl->begin(); in != nl->end(); ++in) {
                                          Note* note = in->second;
                                          if (note->tieBack())
                                                continue;
                                          unsigned len = 0;
                                          while (note->tieFor()) {
                                                if (note->tieFor()->endNote() == 0)
                                                      break;
                                                len += note->chord()->tickLen();
                                                note = note->tieFor()->endNote();
                                                }
                                          len += (note->chord()->tickLen() * gateTime / 100);

                                          unsigned tick = chord->tick();
                                          Event ev;
                                          ev.type       = 0x90; // note on
                                          ev.val1       = note->pitch();
                                          foreach(OttavaE o, ol) {
                                                if (tick >= o.start && tick <= o.end) {
                                                      ev.val1 += o.offset;
                                                      break;
                                                      }
                                                }
                                          ev.val2       = 60;
                                          ev.note       = note;
                                          ev.channel    = channel;
                                          events.insert(std::pair<const unsigned, Event> (tick, ev));

                                          ev.val2 = 0;
                                          events.insert(std::pair<const unsigned, Event> (tick+len, ev));
                                          }
                                    }
                              }
                        }
                  ++staffIdx;
                  }
            }
      Measure* lm = cs->mainLayout()->last();
      if (lm) {
            endTick   = lm->tick() + lm->tickLen();
            PlayPanel* pp = mscore->getPlayPanel();
            if (pp)
                  pp->setEndpos(endTick);
            }
      }

//---------------------------------------------------------
//   heartBeat
//    paint currently sounding notes
//---------------------------------------------------------

void Seq::heartBeat()
      {
      if (guiPos == playPos)
            return;

      cs->start();
      for (ciEvent i = guiPos; i != playPos; ++i) {
            if (i->second.val2 == 0)       // note off
                  cs->deselect(i->second.note);
            else
                  cs->select(i->second.note, Qt::ShiftModifier, 0);
            }
      cs->end1();
      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->heartBeat(frame2tick(playFrame));
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
      msg.id    = SEQ_TEMPO_CHANGE;
      msg.data1 = relTempo;
      sendMessage(msg);

      int tempo = cs->tempomap->tempo(playPos->first) * 100 / relTempo;

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
      if (state != STOP)
            return;
      playTick  = tick;
      playFrame = tick2frame(tick);
      playPos   = events.lower_bound(tick);
      guiPos    = playPos;
      PlayPanel* pp = mscore->getPlayPanel();
      if (pp)
            pp->heartBeat(tick);
      }

//---------------------------------------------------------
//   sendMessage
//    send Message to sequencer
//---------------------------------------------------------

void Seq::sendMessage(SeqMsg& msg) const
      {
      if (!running)
            return;
      if (write(fromThreadFdw, &msg, sizeof(SeqMsg)) != sizeof(SeqMsg)) {
            fprintf(stderr, "sendMessage to sequencer failed: %s\n",
               strerror(errno));
            }
      }

//---------------------------------------------------------
//   startNote
//---------------------------------------------------------

void Seq::startNote(int channel, int pitch, int velo)
      {
      if (state != STOP)
            return;
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.data1 = 0x90 | channel;
      msg.data2 = pitch;
      msg.data3 = velo;
      sendMessage(msg);
      Event event;
      event.type = ME_NOTEON;
      event.channel = channel;
      event.val1 = pitch;
      event.val2 = velo;
      eventList.append(event);
      }

//---------------------------------------------------------
//   stopNotes
//---------------------------------------------------------

void Seq::stopNotes()
      {
      foreach(const Event& event, eventList) {
            if (event.type == ME_NOTEON) {
                  SeqMsg msg;
                  msg.id    = SEQ_PLAY;
                  msg.data1 = event.type | event.channel;
                  msg.data2 = event.val1;
                  msg.data3 = 0;
                  sendMessage(msg);
                  }
            }
      eventList.clear();
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void Seq::setController(int channel, int ctrl, int data) const
      {
      SeqMsg msg;
      msg.id    = SEQ_PLAY;
      msg.data1 = 0xb0 | channel;
      msg.data2 = ctrl;
      msg.data3 = data;
      sendMessage(msg);
      }

