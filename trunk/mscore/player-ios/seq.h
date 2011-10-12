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

#include "m-msynth/event.h"
#include "libmscore/fifo.h"
// #include "libmscore/painter.h"

class Synti;
class Driver;
class Score;

enum { TRANSPORT_STOP=0, TRANSPORT_PLAY=1, TRANSPORT_STARTING=3,
       TRANSPORT_NET_STARTING=4 };

//---------------------------------------------------------
//   SeqMsg
//    message format for gui -> sequencer messages
//---------------------------------------------------------

enum { SEQ_NO_MESSAGE, SEQ_TEMPO_CHANGE, SEQ_PLAY, SEQ_SEEK };

struct SeqMsg {
      int id;
      int data;
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

class Seq {
      bool running;                       // true if sequencer is available
      int  state;                         // STOP, PLAY, START_PLAY
      bool playlistChanged;
      Score* cs;

      SeqMsgFifo toSeq;

      Synti* synti;
      Driver* driver;

      int playTime;           // sample count
      int endTick;

      EventMap::const_iterator playPos;   // moved in real time thread
      EventMap::const_iterator guiPos;
      QList<SeqEvent> activeNotes;        // notes sounding

      EventMap events;
      QList<SeqEvent> eventList;
      void sendMessage(SeqMsg&) const;
      void processMessages();
      void putEvent(const SeqEvent&);  // send event to synthesizer in rt thread
      void playEvent(const SeqEvent&);
      void collectEvents();
      void setPos(int utick);

   public:
      Seq();
      ~Seq();

      bool init();
      void exit();
      void sendEvent(const SeqEvent&);

      void process(unsigned, float*);
      void setScore(Score* s);
      void start();
      void stop();
      bool isPlaying() const { return state == TRANSPORT_PLAY; }
      void startStop() {
            if (isPlaying())
                  stop();
            else
                  start();
            }
      void seek(int);
      QRectF heartBeat(int* pageIdx, bool* stopped);
      void setRelTempo(float);
      };

extern Seq* seq;
#endif

