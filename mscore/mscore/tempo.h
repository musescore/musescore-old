//=============================================================================
//  MusE
//  Linux Music Editor
//  $Id: tempo.h,v 1.5 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __TEMPO_H__
#define __TEMPO_H__

#include "ipmap.h"

#ifndef MAX_TICK
#define MAX_TICK (0x7fffffff/100)
#endif

class Xml;
class Score;

//---------------------------------------------------------
//   Tempo Event
//---------------------------------------------------------

struct TEvent {
      int tempo;
      int tick;            // new tempo at tick
      double time;         // precomputed time for tick in sec

      int read(QDomElement, Score*);
      void write(Xml&, int) const;

      TEvent() { }
      TEvent(int t, int tk) {
            tempo = t;
            tick = tk;
            time = 0.0;
            }
      };

//---------------------------------------------------------
//   TempoList
//---------------------------------------------------------

typedef pstl::ipmap<TEvent* >::iterator iTEvent;
typedef pstl::ipmap<TEvent* >::const_iterator ciTEvent;
typedef pstl::ipmap<TEvent* >::reverse_iterator riTEvent;
typedef pstl::ipmap<TEvent* >::const_reverse_iterator criTEvent;

class TempoList : public pstl::ipmap<TEvent* > {
      int _tempoSN;           // serial no to track tempo changes
      bool useList;
      int _tempo;             // tempo if not using tempo list
      int _relTempo;          // rel. tempo (100 == 1.0)

      void normalize();
      void add(int tick, int tempo);
      void change(int tick, int newTempo);
      void del(iTEvent);
      void del(int tick);

   public:
      TempoList();
      void clear();

      void read(QDomElement, Score*);
      void write(Xml&) const;
      void dump() const;

      int tempo(int tick) const;
      double tick2time(int tick, int* sn = 0) const;
      double tick2timeLC(int tick, int* sn) const;
      double tick2time(int tick, double time, int* sn) const;
      int time2tick(double time, int* sn = 0) const;
      int time2tick(double time, int tick, int* sn) const;
      int tempoSN() const { return _tempoSN; }
      void setTempo(int tick, int newTempo);
      void addTempo(int t, int tempo);
      void delTempo(int tick);
      void changeTempo(int tick, int newTempo);
      bool setMasterFlag(int tick, bool val);
      int tick2samples(int tick);
      int samples2tick(int samples);
      void setRelTempo(int val);
      int relTempo() const { return _relTempo; }
      };

extern TempoList tempomap;
#endif
