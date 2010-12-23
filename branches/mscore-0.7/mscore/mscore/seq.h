//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: seq.h,v 1.20 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SEQ_H__
#define __SEQ_H__

class Synth;
class Audio;
class Note;
class QTimer;
class Score;
class Painter;

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum { SEQ_TEMPO_CHANGE, SEQ_PLAY, SEQ_SEEK };

struct SeqMsg {
      int id;
      int data1;
      int data2;
      int data3;
      };

//---------------------------------------------------------
//   Event
//    sequencer event type (note on, note off)
//---------------------------------------------------------

struct Event {
      int type;
      int channel;
      int val1;         // pitch
      int val2;         // velocity
      Note* note;       // used to mark the currently played notes
      };

typedef std::multimap<int, Event, std::less<int> > EList;
typedef EList::iterator iEvent;
typedef EList::const_iterator ciEvent;
typedef std::pair <iEvent, iEvent> EventRange;

//---------------------------------------------------------
//   Seq
//    sequencer
//---------------------------------------------------------

class Seq : public QObject {
      Q_OBJECT

      Score* cs;
      bool running;                       // true if sequencer is available
      int state;                          // STOP, PLAY, START_PLAY
      bool playlistChanged;
      bool pauseState;

      Synth* synti;
      Audio* audio;
      int fromThreadFdw, fromThreadFdr;   // message pipe
      int sigFd;                          // pipe fd for messages to gui

      EList events;                       // playlist
      QList<Event> _activeNotes;          // currently sounding notes
      int playFrame;
      ciEvent playPos, guiPos;

      int endTick;

      float _volume;

      QTimer* heartBeatTimer;

      QList<Event> eventList;

      int toGui(char c) { return write(sigFd, &c, 1); }
      void collectEvents();
      void stopTransport();
      void startTransport();
      int frame2tick(int frame) const;
      int tick2frame(int tick) const;
      void setPos(int);
      void playEvent(const Event& event);
      void guiStop();

   private slots:
      void seqMessage(int fd);
      void heartBeat();
      void selectionChanged(int);

   public slots:
      void setVolume(float);
      void setRelTempo(int);
      void seek(int);

   signals:
      void started();
      void stopped();

   public:
      enum { STOP, PLAY, START_PLAY };

      Seq();
      ~Seq();
      void start();
      void stop();
      void pause();
      void rewindStart();
      void seekEnd();
      void nextMeasure();
      void nextChord();
      void prevMeasure();
      void prevChord();

      bool loadSoundFont(const QString&);
      bool init();
      void exit();
      bool isRunning() const    { return running; }
      bool isPlaying() const    { return state == PLAY; }
      bool isStopped() const    { return state == STOP; }
      void process(unsigned, float*, float*);
      std::list<QString> inputPorts();
      int sampleRate() const;
      int getEndTick() const    { return endTick; }
      float volume() const      {  return _volume; }
      bool isRealtime() const;
      void sendMessage(SeqMsg&) const;
      void startNote(int, int, int);
      void stopNotes();
      void setController(int, int, int) const;
      void setScore(Score* s);
      Synth* synth() const  { return synti; }
      };

extern Seq* seq;
extern void initSequencer();
#endif
