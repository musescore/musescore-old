//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id: tempo.h 3290 2010-07-16 18:42:57Z wschweer $
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

#include <map>

class XmlReader;

namespace AL {
class Xml;

enum TempoType { TEMPO_FIX, TEMPO_RAMP };

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      TempoType type;
      double tempo;     // beats per second
      double pause;     // pause in seconds
      double time;      // precomputed time for tick in sec

      int read(XmlReader*);

      TEvent() {
            tempo = 0.0;
            }
      TEvent(const TEvent& e) {
            tempo = e.tempo;
            pause = e.pause;
            time  = e.time;
            }
      TEvent(double t, double p = 0.0) {
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
      double _tempo;          // tempo if not using tempo list (beats per second)
      int _relTempo;          // rel. tempo (100 == 1.0)

      void normalize();
      void add(int tick, double);
      void addP(int tick, double);
      void change(int tick, double);
      void del(iTEvent);
      void del(int tick);
      void add(int t, const TEvent&);

   public:
      TempoMap();
      void clear();

      void read(XmlReader*, int sourceDivision);
      void write(Xml&) const;
      void dump() const;

      double tempo(int tick) const;

      double tick2time(int tick, int* sn = 0) const;
      double tick2timeLC(int tick, int* sn) const;
      double tick2time(int tick, double time, int* sn) const;
      int time2tick(double time, int* sn = 0) const;
      int time2tick(double time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }
      void addTempo(int t, double);
      void addPause(int t, double);
      void addTempo(int tick, const TEvent& ev);
      void delTempo(int tick);
      void changeTempo(int tick, double newTempo);
      int tick2samples(int tick);
      int samples2tick(int samples);
      void setRelTempo(int val);
      int relTempo() const { return _relTempo; }
      void removeTime(int start, int len);
      void insertTime(int start, int len);
      TEvent getTempo(int tick) const;
      };

}     // namespace AL
#endif
