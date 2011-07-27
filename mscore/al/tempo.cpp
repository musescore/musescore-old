//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "tempo.h"
#include "xml.h"
#include "al.h"

namespace AL {

//---------------------------------------------------------
//   TempoMap
//---------------------------------------------------------

TempoMap::TempoMap()
      {
      _tempo    = 2.0;
      _tempoSN  = 1;
      _relTempo = 100;
      useList   = true;
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
            e->second.time = time;
            tick = e->first;
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
      if (!useList)
            return _tempo;
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
      if (!useList)
            return TEvent(_tempo);
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
//   setTempo
//    called from transport window
//    & slave mode tempo changes
//---------------------------------------------------------

void TempoMap::setTempo(int tick, double newTempo)
      {
      if (useList)
            add(tick, newTempo);
      else
            _tempo = newTempo;
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
//   setMasterFlag
//---------------------------------------------------------

bool TempoMap::setMasterFlag(int /*tick*/, bool val)
      {
      if (useList != val) {
            useList = val;
            ++_tempoSN;
            return true;
            }
      return false;
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

      if (useList) {
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
            }
      else
            tempo = _tempo;
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

      if (useList) {
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
            }
      tick += lrint(delta * _relTempo * 0.01 * division * tempo);
      if (sn)
            *sn = _tempoSN;
      return tick;
      }

//---------------------------------------------------------
//   TempoMap::write
//---------------------------------------------------------

void TempoMap::write(Xml& xml) const
      {
      xml.stag(QString("tempolist fix=\"%1\"").arg(_tempo));
      if (_relTempo != 100)
            xml.tag("relTempo", _relTempo);
      for (ciTEvent i = begin(); i != end(); ++i)
            i->second.write(xml, i->first);
      xml.etag();
      }

//---------------------------------------------------------
//   TempoMap::read
//---------------------------------------------------------

void TempoMap::read(QDomElement e, int sourceDivision)
      {
      _tempo = e.attribute("fix","2.0").toDouble();

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() == "tempo") {
                  TEvent t;
                  unsigned tick = t.read(e);
                  tick = (tick * division + sourceDivision/2) / sourceDivision;
                  iTEvent pos = find(tick);
                  if (pos != end())
                        erase(pos);
                  insert(std::pair<const int, TEvent> (tick, t));
                  }
            else if (e.tagName() == "relTempo")
                  _relTempo = e.text().toInt();
            else
                  domError(e);
            }
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   TEvent::write
//---------------------------------------------------------

void TEvent::write(Xml& xml, int at) const
      {
      xml.tag(QString("tempo tick=\"%1\"").arg(at), QVariant(tempo));
      }

//---------------------------------------------------------
//   TEvent::read
//---------------------------------------------------------

int TEvent::read(QDomElement e)
      {
      int at = e.attribute("tick", "0").toInt();
      tempo  = e.text().toDouble();
      return at;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TempoMap::removeTime(int tick, int len)
      {
      TempoMap tmp;
      for (ciTEvent i = begin(); i != end(); ++i) {
            if (i->first >= tick) {
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
            if (i->first >= tick)
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


