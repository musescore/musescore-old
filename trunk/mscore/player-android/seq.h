//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#include "m-msynth/seq_event.h"
#include "libmscore/fifo.h"
// #include "libmscore/painter.h"

class Synti;
class Driver;
class Score;
class ScoreView;

enum { TRANSPORT_STOP=0, TRANSPORT_PLAY=1, TRANSPORT_STARTING=3,
       TRANSPORT_NET_STARTING=4 };

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum { SEQ_NO_MESSAGE, SEQ_TEMPO_CHANGE, SEQ_PLAY, SEQ_SEEK };

struct SeqMsg {
      int id;
      union {
            int intVal;
            qreal realVal;
            } data;
      SeqEvent event;
      };

//---------------------------------------------------------
//   SeqMsgFifo
//---------------------------------------------------------

static const int SEQ_MSG_FIFO_SIZE = 256;

class SeqMsgFifo : public FifoBase {
      SeqMsg messages[SEQ_MSG_FIFO_SIZE];

   public:
      SeqMsgFifo();
      virtual ~SeqMsgFifo()     {}
      void enqueue(const SeqMsg&);        // put object on fifo
      SeqMsg dequeue();                   // remove object from fifo
      };

//---------------------------------------------------------
//   Seq
//    sequencer
//---------------------------------------------------------

class Seq : public QObject {
      Q_OBJECT

      Score* cs;
      ScoreView* view;
      bool running;                       // true if sequencer is available
      int state;                          // STOP, PLAY, START_PLAY

      bool playlistChanged;

      QTimer* heartBeatTimer;

      SeqMsgFifo toSeq;

      Synti* synti;
      Driver* driver;

      int playTime;
      int endTick;

      EventMap::const_iterator playPos;   // moved in real time thread

      EventMap events;
      QList<SeqEvent> eventList;
      void sendMessage(SeqMsg&) const;
      void processMessages();
      void playEvent(const SeqEvent&);
      void collectEvents();
      void setPos(int utick);

      void stopNotes();

   private slots:
      void heartBeat();

   public:
      Seq();
      ~Seq();

      bool init();
      void exit();
      void sendEvent(const SeqEvent&);

      void process(unsigned, short*);
      void setScore(Score*);
      void setView(ScoreView*);
      void start();
      void stop();
      bool isPlaying() const { return state == TRANSPORT_PLAY; }
      void startStop();
      void seek(int);
      void setRelTempo(qreal);
      };

extern Seq* seq;
#endif

