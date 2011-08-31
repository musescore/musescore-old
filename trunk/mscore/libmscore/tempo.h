//=============================================================================
//  MusE
//  Linux Music Editor
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

#ifndef __AL_TEMPO_H__
#define __AL_TEMPO_H__

class Xml;

enum TempoType { TEMPO_FIX, TEMPO_RAMP };

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      TempoType type;
      qreal tempo;     // beats per second
      qreal pause;     // pause in seconds
      qreal time;      // precomputed time for tick in sec

      int read(QDomElement);
      void write(Xml&, int) const;

      TEvent() {
            tempo = 0.0;
            }
      TEvent(const TEvent& e) {
            tempo = e.tempo;
            pause = e.pause;
            time  = e.time;
            }
      TEvent(qreal t, qreal p = 0.0) {
            tempo = t;
            pause = p;
            time  = 0.0;
            }
      bool valid() const { return tempo != 0.0; }
      };

//---------------------------------------------------------
//   Tempomap
//---------------------------------------------------------

typedef std::map<int, TEvent>::iterator iTEvent;
typedef std::map<int, TEvent>::const_iterator ciTEvent;
typedef std::map<int, TEvent>::reverse_iterator riTEvent;
typedef std::map<int, TEvent>::const_reverse_iterator criTEvent;

class TempoMap : public std::map<int, TEvent> {
      int _tempoSN;           // serial no to track tempo changes
      qreal _tempo;          // tempo if not using tempo list (beats per second)
      int _relTempo;          // rel. tempo (100 == 1.0)

      void normalize();
      void add(int tick, qreal);
      void addP(int tick, qreal);
      void change(int tick, qreal);
      void del(iTEvent);
      void del(int tick);
      void add(int t, const TEvent&);

   public:
      TempoMap();
      void clear();

      void read(QDomElement, int sourceDivision);
      void write(Xml&) const;
      void dump() const;

      qreal tempo(int tick) const;

      qreal tick2time(int tick, int* sn = 0) const;
      qreal tick2timeLC(int tick, int* sn) const;
      qreal tick2time(int tick, qreal time, int* sn) const;
      int time2tick(qreal time, int* sn = 0) const;
      int time2tick(qreal time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }
      void addTempo(int t, qreal);
      void addPause(int t, qreal);
      void addTempo(int tick, const TEvent& ev);
      void delTempo(int tick);
      void changeTempo(int tick, qreal newTempo);
      int tick2samples(int tick);
      int samples2tick(int samples);
      void setRelTempo(int val);
      int relTempo() const { return _relTempo; }
      void removeTime(int start, int len);
      void insertTime(int start, int len);
      TEvent getTempo(int tick) const;
      };

#endif
