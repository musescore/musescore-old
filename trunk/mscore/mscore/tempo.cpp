//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: tempo.cpp,v 1.9 2006/03/13 21:35:59 wschweer Exp $
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

#include "mtime.h"
#include "tempo.h"
#include "xml.h"
#include "score.h"

//---------------------------------------------------------
//   TempoList
//---------------------------------------------------------

TempoList::TempoList()
      {
      _tempo    = 500000;
      insert(std::pair<const int, TEvent*> (MAX_TICK, new TEvent(_tempo, 0)));
      _tempoSN  = 1;
      _relTempo = 100;
      useList   = true;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TempoList::add(int tick, int tempo)
      {
      iTEvent e = upper_bound(tick);
      if (tick == e->second->tick)
            e->second->tempo = tempo;
      else {
            TEvent* ne = e->second;
            TEvent* ev = new TEvent(ne->tempo, ne->tick);
            ne->tempo  = tempo;
            ne->tick   = tick;
            insert(std::pair<const int, TEvent*> (tick, ev));
            }
      normalize();
      }

//---------------------------------------------------------
//   TempoList::normalize
//---------------------------------------------------------

void TempoList::normalize()
      {
      double time = 0;
      for (iTEvent e = begin(); e != end(); ++e) {
            e->second->time = time;
            int delta = e->first - e->second->tick;
            time += double(delta) / (division * _relTempo * 10000.0/e->second->tempo);
            }
      }

//---------------------------------------------------------
//   TempoList::dump
//---------------------------------------------------------

void TempoList::dump() const
      {
      printf("\nTempoList:\n");
      for (ciTEvent i = begin(); i != end(); ++i) {
            printf("%6d %06d Tempo %6d %f\n",
               i->first, i->second->tick, i->second->tempo,
               i->second->time);
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void TempoList::clear()
      {
      for (iTEvent i = begin(); i != end(); ++i)
            delete i->second;
      pstl::ipmap<TEvent* >::clear();
      insert(std::pair<const int, TEvent*> (MAX_TICK, new TEvent(500000, 0)));
      ++_tempoSN;
      }

//---------------------------------------------------------
//   tempo
//---------------------------------------------------------

int TempoList::tempo(int tick) const
      {
      if (useList) {
            ciTEvent i = upper_bound(tick);
            if (i == end()) {
                  printf("no TEMPO\n");
                  return 1000;
                  }
            return i->second->tempo;
            }
      else
            return _tempo;
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TempoList::del(int tick)
      {
// printf("TempoList::del(%d)\n", tick);
      iTEvent e = find(tick);
      if (e == end()) {
            printf("TempoList::del(%d): not found\n", tick);
            return;
            }
      del(e);
      ++_tempoSN;
      }

void TempoList::del(iTEvent e)
      {
      iTEvent ne = e;
      ++ne;
      if (ne == end()) {
            printf("TempoList::del() HALLO\n");
            return;
            }
      ne->second->tempo = e->second->tempo;
      ne->second->tick  = e->second->tick;
      erase(e);
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void TempoList::change(int tick, int newTempo)
      {
      iTEvent e = find(tick);
      e->second->tempo = newTempo;
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   setTempo
//    called from transport window
//    & slave mode tempo changes
//---------------------------------------------------------

void TempoList::setTempo(int tick, int newTempo)
      {
// printf("set tempo useList:%d %d, diff %3d\n", useList, newTempo, newTempo-_tempo);
      if (useList)
            add(tick, newTempo);
      else
            _tempo = newTempo;
      ++_tempoSN;
      }

//---------------------------------------------------------
//   setRelTempo
//---------------------------------------------------------

void TempoList::setRelTempo(int val)
      {
      _relTempo = val;
      ++_tempoSN;
      normalize();
      }

//---------------------------------------------------------
//   addTempo
//---------------------------------------------------------

void TempoList::addTempo(int t, int tempo)
      {
      add(t, tempo);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   delTempo
//---------------------------------------------------------

void TempoList::delTempo(int tick)
      {
      del(tick);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   changeTempo
//---------------------------------------------------------

void TempoList::changeTempo(int tick, int newTempo)
      {
      change(tick, newTempo);
      ++_tempoSN;
      }

//---------------------------------------------------------
//   setMasterFlag
//---------------------------------------------------------

bool TempoList::setMasterFlag(int /*tick*/, bool val)
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

double TempoList::tick2time(int tick, double time, int* sn) const
      {
      return (*sn == _tempoSN) ? time : tick2time(tick, sn);
      }

//---------------------------------------------------------
//   time2tick
//    return cached value t if list did not change
//---------------------------------------------------------

int TempoList::time2tick(double time, int t, int* sn) const
      {
      return (*sn == _tempoSN) ? t : time2tick(time, sn);
      }

//---------------------------------------------------------
//   tick2time
//---------------------------------------------------------

double TempoList::tick2time(int tick, int* sn) const
      {
      double t;
      if (useList) {
            ciTEvent i = upper_bound(tick);
            int delta = tick - i->second->tick;
            t = i->second->time + double(delta) / (division * _relTempo * 10000.0 / i->second->tempo);
            }
      else
            t = (double(tick) * double(_tempo)) / (double(division) * _relTempo * 10000.0);
      if (sn)
            *sn = _tempoSN;
      return t;
      }

//---------------------------------------------------------
//   time2tick
//---------------------------------------------------------

int TempoList::time2tick(double time, int* sn) const
      {
      int tick;
      if (useList) {
            ciTEvent e;
            for (e = begin(); e != end();) {
                  ciTEvent ee = e;
                  ++ee;
                  if (ee == end())
                        break;
                  if (time < ee->second->time)
                        break;
                  e = ee;
                  }
            int te = e->second->tempo;
            double dtime = time - e->second->time;
            tick = e->second->tick + lrint(dtime * _relTempo * division * 10000.0 / te);
            }
      else
            tick = lrint(time * _relTempo * division * 10000.0 / double(_tempo));
      if (sn)
            *sn = _tempoSN;
      return tick;
      }

//---------------------------------------------------------
//   TempoList::write
//---------------------------------------------------------

void TempoList::write(Xml& xml) const
      {
      xml.stag(QString("tempolist fix=\"%1\"").arg(_tempo));
      if (_relTempo != 100)
            xml.tag("relTempo", _relTempo);
      for (ciTEvent i = begin(); i != end(); ++i)
            i->second->write(xml, i->first);
      xml.etag();
      }

//---------------------------------------------------------
//   TempoList::read
//---------------------------------------------------------

void TempoList::read(QDomNode node, Score* cs)
      {
      QDomElement e = node.toElement();
      _tempo = e.attribute("fix","500000").toInt();

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            e = node.toElement();
            if (e.isNull())
                  continue;
            if (e.tagName() == "tempo") {
                  TEvent* t = new TEvent();
                  unsigned tick = t->read(node, cs);
                  tick = cs->fileDivision(tick);
                  iTEvent pos = find(tick);
                  if (pos != end())
                        erase(pos);
                  insert(std::pair<const int, TEvent*> (tick, t));
                  }
            else if (e.tagName() == "relTempo")
                  _relTempo = e.text().toInt();
            else
                  printf("MuseScore:Tempolist: unknown tag %s\n", e.tagName().toLatin1().data());
            }
      normalize();
      ++_tempoSN;
      }

//---------------------------------------------------------
//   TEvent::write
//---------------------------------------------------------

void TEvent::write(Xml& xml, int at) const
      {
      xml.stag(QString("tempo at=\"%1\"").arg(at));
      xml.tag("tick", tick);
      xml.tag("val", tempo);
      xml.etag();
      }

//---------------------------------------------------------
//   TEvent::read
//---------------------------------------------------------

int TEvent::read(QDomNode node, Score* cs)
      {
      QDomElement e = node.toElement();
      int at = e.attribute("tick", "0").toInt();

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "tick")
                  tick = cs->fileDivision(i);
            else if (tag == "val")
                  tempo = i;
            else
                  printf("Mscore:TEvent: unknown tag %s\n",
                     tag.toLatin1().data());
            }
      return at;
      }
