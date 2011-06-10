//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tempo.cpp 3174 2010-06-15 10:45:04Z wschweer $
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

#include <math.h>
#include "tempo.h"
#include "xml.h"
#include "al.h"

class XmlReader;

namespace AL {

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

TempoMap::TempoMap()
      {
      _tempo    = 2.0;
      _tempoSN  = 1;
      _relTempo = 100;
      }

//---------------------------------------------------------
//   addPause
//---------------------------------------------------------

void TempoMap::addP(int tick, double pause)
      {
      iTEvent e = find(tick);
      if (e != end())
            e->second.pause = pause;
      else {
            double t = tempo(tick);
            insert(std::pair<const int, TEvent> (tick, TEvent(t, pause)));
            }
      normalize();
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TempoMap::add(int tick, double tempo)
      {
      iTEvent e = find(tick);
      if (e != end())
            e->second.tempo = tempo;
      else
            insert(std::pair<const int, TEvent> (tick, TEvent(tempo)));
      normalize();
      }

void TempoMap::add(int tick, const TEvent& ev)
      {
      (*this)[tick] = ev;
      normalize();
      }

//---------------------------------------------------------
//   TempoMap::normalize
//---------------------------------------------------------

void TempoMap::normalize()
      {
      double time  = 0;
      int tick     = 0;
      double tempo = 2.0;
      for (iTEvent e = begin(); e != end(); ++e) {
            int delta = e->first - tick;
            time += double(delta) / (division * tempo * _relTempo * 0.01);
            time += e->second.pause;
            e->second.time = time;
            tick  = e->first;
            tempo = e->second.tempo;
            }
      }

//---------------------------------------------------------
//   TempoMap::dump
//---------------------------------------------------------

void TempoMap::dump() const
      {
      printf("\nTempoMap:\n");
      for (ciTEvent i = begin(); i != end(); ++i)
            printf("%6d tempo: %f time: %f\n",
               i->first, i->second.tempo, i->second.time);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void TempoMap::clear()
      {
      std::map<int,TEvent>::clear();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

double TempoMap::tempo(int tick) const
      {
      if (empty())
            return 2.0;
      ciTEvent i = lower_bound(tick);
      if (i == end()) {
            --i;
            return i->second.tempo;
            }
      if (i->first == tick)
            return i->second.tempo;
      if (i == begin())
            return 2.0;
      --i;
      return i->second.tempo;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

TEvent TempoMap::getTempo(int tick) const
      {
      if (empty())
            return TEvent(2.0);
      ciTEvent i = lower_bound(tick);
      if (i == end()) {
            --i;
            return i->second;
            }
      if (i->first == tick)
            return i->second;
      if (i == begin())
            return TEvent(2.0);
      --i;
      return i->second;
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TempoMap::del(int tick)
      {
      iTEvent e = find(tick);
      if (e == end()) {
            printf("TempoMap::del event at (%d): not found\n", tick);
            return;
            }
      del(e);
      }

void TempoMap::del(iTEvent e)
      {
      erase(e);
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void TempoMap::change(int tick, double newTempo)
      {
      iTEvent e = find(tick);
      e->second.tempo = newTempo;
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void TempoMap::setRelTempo(int val)
      {
      _relTempo = val;
      ++_tempoSN;
      normalize();
      }

//---------------------------------------------------------
//   addPause
//---------------------------------------------------------

void TempoMap::addPause(int t, double pause)
      {
      addP(t, pause);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void TempoMap::addTempo(int t, double tempo)
      {
      add(t, tempo);
      ++_tempoSN;
      }

void TempoMap::addTempo(int tick, const TEvent& ev)
      {
      add(tick, ev);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   delTempo
//---------------------------------------------------------

void TempoMap::delTempo(int tick)
      {
      del(tick);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   changeTempo
//---------------------------------------------------------

void TempoMap::changeTempo(int tick, double newTempo)
      {
      change(tick, newTempo);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

double TempoMap::tick2time(int tick, double time, int* sn) const
      {
      return (*sn == _tempoSN) ? time : tick2time(tick, sn);
      }

//---------------------------------------------------------
//   time2tick
//    return cached value t if list did not change
//---------------------------------------------------------

int TempoMap::time2tick(double time, int t, int* sn) const
      {
      return (*sn == _tempoSN) ? t : time2tick(time, sn);
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

double TempoMap::tick2time(int tick, int* sn) const
      {
      double time  = 0.0;
      double delta = double(tick);
      double tempo = 2.0;

      if (!empty()) {
            int ptick  = 0;
            ciTEvent e = lower_bound(tick);
            if (e == end()) {
                  ciTEvent pe = e;
                  --pe;
                  ptick = pe->first;
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            else if (e->first == tick) {
                  ptick = tick;
                  tempo = e->second.tempo;
                  time  = e->second.time;
                  }
            else if (e != begin()) {
                  ciTEvent pe = e;
                  --pe;
                  ptick = pe->first;
                  tempo = pe->second.tempo;
                  time  = pe->second.time;
                  }
            delta = double(tick - ptick);
            }
      if (sn)
            *sn = _tempoSN;
      time += delta / (division * tempo * _relTempo * 0.01);
      return time;
      }

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int TempoMap::time2tick(double time, int* sn) const
      {
      int tick     = 0;
      double delta = time;
      double tempo = _tempo;

      delta = 0.0;
      tempo = 2.0;
      for (ciTEvent e = begin(); e != end(); ++e) {
            if (e->second.time >= time)
                  break;
            delta = e->second.time;
            tempo = e->second.tempo;
            tick  = e->first;
            }
      delta = time - delta;
      tick += lrint(delta * _relTempo * 0.01 * division * tempo);
      if (sn)
            *sn = _tempoSN;
      return tick;
      }

//---------------------------------------------------------
//   TempoMap::read
//---------------------------------------------------------

void TempoMap::read(XmlReader* r, int sourceDivision)
      {
      _tempo = 2.0;
      while (r->readAttribute()) {
            if (r->tag() == "fix")
                  _tempo = r->realValue();
            }

      while (r->readElement()) {
            if (r->tag() == "tempo") {
                  TEvent t;
                  unsigned tick = t.read(r);
                  tick = (tick * division + sourceDivision/2) / sourceDivision;
                  iTEvent pos = find(tick);
                  if (pos != end())
                        erase(pos);
                  insert(std::pair<const int, TEvent> (tick, t));
                  }
            else if (r->readInt("relTempo", &_relTempo))
                  ;
            else
                  r->unknown();
            }
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   TEvent::read
//---------------------------------------------------------

int TEvent::read(XmlReader* r)
      {
      int at = 0;
      while (r->readAttribute()) {
            if (r->tag() == "tick")
                  at = r->intValue();
            }
      tempo  = r->readReal("tempo");
      return at;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TempoMap::removeTime(int tick, int len)
      {
      TempoMap tmp;
      for (ciTEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0)) {
                  if (i->first >= tick + len)
                        tmp.add(i->first - len, i->second);
                  else
                        printf("remove tempo event\n");
                  }
            else
                  tmp.add(i->first, i->second);
            }
      std::map<int,TEvent>::clear();
      insert(tmp.begin(), tmp.end());
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void TempoMap::insertTime(int tick, int len)
      {
      TempoMap tmp;
      for (ciTEvent i = begin(); i != end(); ++i) {
            if ((i->first >= tick) && (tick != 0))
                  tmp.add(i->first + len, i->second);
            else
                  tmp.add(i->first, i->second);
            }
      std::map<int,TEvent>::clear();
      insert(tmp.begin(), tmp.end());
      normalize();
      ++_tempoSN;
      }
}     // namespace al


