//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

namespace AL {
class Xml;

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      double tempo;     // beats per second
      double time;      // precomputed time for tick in sec

      int read(QDomElement);
      void write(Xml&, int) const;

      TEvent() {
            tempo = 0.0;
            }
      TEvent(const TEvent& e) {
            tempo = e.tempo;
            time  = e.time;
            }
      TEvent(double t) {
            tempo = t;
            time = 0.0;
            }
      bool valid() const { return tempo != 0.0; }
      };

//---------------------------------------------------------
//   TempoList
//---------------------------------------------------------

typedef std::map<int, TEvent>::iterator iTEvent;
typedef std::map<int, TEvent>::const_iterator ciTEvent;
typedef std::map<int, TEvent>::reverse_iterator riTEvent;
typedef std::map<int, TEvent>::const_reverse_iterator criTEvent;

class TempoList : public std::map<int, TEvent> {
      int _tempoSN;           // serial no to track tempo changes
      bool useList;
      double _tempo;          // tempo if not using tempo list (beats per second)
      int _relTempo;          // rel. tempo (100 == 1.0)

      void normalize();
      void add(int tick, double tempo);
      void change(int tick, double newTempo);
      void del(iTEvent);
      void del(int tick);
      void add(int t, const TEvent&);

   public:
      TempoList();
      void clear();

      void read(QDomElement, int sourceDivision);
      void write(Xml&) const;
      void dump() const;

      double tempo(int tick) const;

      double tick2time(int tick, int* sn = 0) const;
      double tick2timeLC(int tick, int* sn) const;
      double tick2time(int tick, double time, int* sn) const;
      int time2tick(double time, int* sn = 0) const;
      int time2tick(double time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }
      void setTempo(int tick, double newTempo);
      void addTempo(int t, double tempo);
      void addTempo(int tick, const TEvent& ev);
      void delTempo(int tick);
      void changeTempo(int tick, double newTempo);
      bool setMasterFlag(int tick, bool val);
      int tick2samples(int tick);
      int samples2tick(int samples);
      void setRelTempo(int val);
      int relTempo() const { return _relTempo; }
      void removeTime(int start, int len);
      void insertTime(int start, int len);
      };

}     // namespace AL
#endif
