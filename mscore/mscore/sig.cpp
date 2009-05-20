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

#include "sig.h"
#include "xml.h"
#include "globals.h"

//---------------------------------------------------------
//   ticks_beat
//---------------------------------------------------------

static int ticks_beat(int n)
      {
      int m = (division * 4) / n;
      if ((division * 4) % n) {
            fprintf(stderr, "Mscore: ticks_beat(): bad divisor %d\n", n);
            abort();
            }
      return m;
      }

//---------------------------------------------------------
//   ticks_measure
//---------------------------------------------------------

int ticks_measure(int z, int n)
      {
      int m = (division * 4 * z) / n;
      if ((division * 4 * z) % n) {
            fprintf(stderr, "Mscore: ticks_measure(%d, %d): bad divisor\n", z, n);
            abort();
            }
      return m;
      }

//---------------------------------------------------------
//   SigEvent
//---------------------------------------------------------

SigEvent::SigEvent(int z, int n)
      {
      nominator    = z;
      denominator  = n;
      nominator2   = z;
      denominator2 = n;
      bar          = 0;
      ticks        = ticks_measure(z, n);
      }

SigEvent::SigEvent(int z1, int n1, int z2, int n2)
      {
      nominator    = z1;
      denominator  = n1;
      nominator2   = z2;
      denominator2 = n2;
      bar          = 0;
      ticks        = ticks_measure(z1, n1);
      }

bool SigEvent::operator==(const SigEvent& e) const
      {
      return e.nominator == nominator && e.denominator == denominator
         && e.nominator2 == nominator2 && e.denominator2 == denominator2;
      }

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

SigList::SigList()
      {
      _serial = 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SigList::add(int tick, int z, int n)
      {
      if (z == 0 || n == 0) {
            printf("illegal signature %d/%d\n", z, n);
            }
      (*this)[tick] = SigEvent(z, n);
      normalize();
      }

void SigList::add(int tick, int z, int n, int z2, int n2)
      {
      if (z == 0 || n == 0 || z2 == 0 || n2 == 0) {
            printf("illegal signature %d/%d\n", z, n);
            }
      (*this)[tick] = SigEvent(z, n, z2, n2);
      normalize();
      }

void SigList::add(int tick, const SigEvent& ev)
      {
      (*this)[tick] = ev;
      normalize();
      }

void SigList::add(int tick, int ticks, int z2, int n2)
      {
      if (z2 == 0 || n2 == 0) {
            printf("illegal signature %d/%d\n", z2, n2);
            }
      int z = 1, n = 4;
      if ((ticks % division) == 0) {
            z = ticks / division;
            n = 4;
            }
      else if ((ticks % (division/2)) == 0) {
            z = ticks / (division/2);
            n = 8;
            }
      else if ((ticks % (division/4)) == 0) {
            z = ticks / (division/4);
            n = 16;
            }
      else {
            printf("SigList::add(tick:%d, ticks:%d, z2:%d, n2:%d): irregular measure not supported\n",
               tick, ticks, z2, n2);
            if (debugMode)
                  abort();
            }

      (*this)[tick] = SigEvent(z, n, z2, n2);
      normalize();
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void SigList::del(int tick)
      {
      erase(tick);
      normalize();
      }

//---------------------------------------------------------
//   SigList::normalize
//---------------------------------------------------------

void SigList::normalize()
      {
      int z    = 4;
      int n    = 4;
      int tick = 0;
      int bar  = 0;
      int tm   = ticks_measure(z, n);

      for (iSigEvent i = begin(); i != end(); ++i) {
            SigEvent& e  = i->second;
            e.bar        = bar + (i->first - tick) / tm;
            bar          = e.bar;
            tick         = i->first;
            if (e.nominator == e.nominator2 && e.denominator == e.denominator2) {
                  tm      = ticks_measure(e.nominator, e.denominator);
                  e.ticks = tm;
                  }
            }
      ++_serial;
      }

//---------------------------------------------------------
//   ticksMeasure
//---------------------------------------------------------

int SigList::ticksMeasure(int tick) const
      {
      ciSigEvent i = upper_bound(tick);
      if (empty() || i == begin()) {
            printf("SigList::ticksMeasure(): timesig(%d): not found\n", tick);
            if (debugMode)
                  abort();
            return 4 * division;
            }
      --i;
      return i->second.ticks;
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

void SigList::timesig(int tick, int& z, int& n) const
      {
      if (empty()) {
            z = 4;
            n = 4;
            return;
            }
      ciSigEvent i = upper_bound(tick);
      if (i != begin())
            --i;
      z = i->second.nominator;
      n = i->second.denominator;
      }

SigEvent SigList::timesig(int tick) const
      {
      if (empty())
            return SigEvent(4, 4);
      ciSigEvent i = upper_bound(tick);
      if (i != begin())
            --i;
      return i->second;
      }

//---------------------------------------------------------
//   tickValues
//---------------------------------------------------------

void SigList::tickValues(int t, int* bar, int* beat, int* tick) const
      {
      ciSigEvent e = upper_bound(t);
      if (empty() || e == begin()) {
            fprintf(stderr, "tickValue(0x%x) not found\n", t);
            abort();
            }
      --e;
      int delta  = t - e->first;
      int ticksB = ticks_beat(e->second.denominator);
      int ticksM = ticksB * e->second.nominator;
      *bar       = e->second.bar + delta / ticksM;
      int rest   = delta % ticksM;
      *beat      = rest / ticksB;
      *tick      = rest % ticksB;
      }

//---------------------------------------------------------
//   bar2tick
//---------------------------------------------------------

int SigList::bar2tick(int bar, int beat, int tick) const
      {
      ciSigEvent e;

      for (e = begin(); e != end(); ++e) {
            if (bar < e->second.bar)
                  break;
            }
      if (empty() || e == begin()) {
            fprintf(stderr, "SigList::bar2tick(): not found(%d,%d,%d) not found\n",
               bar, beat, tick);
            if (empty())
                  fprintf(stderr, "   list is empty\n");
            // abort();
            return 0;
            }
      --e;
      int ticksB = ticks_beat(e->second.denominator);
      int ticksM = ticksB * e->second.nominator;
      return e->first + (bar - e->second.bar) * ticksM + ticksB * beat + tick;
      }

//---------------------------------------------------------
//   SigList::write
//---------------------------------------------------------

void SigList::write(Xml& xml) const
      {
      xml.stag("siglist");
      for (ciSigEvent i = begin(); i != end(); ++i)
            i->second.write(xml, i->first);
      xml.etag();
      }

//---------------------------------------------------------
//   SigList::read
//---------------------------------------------------------

void SigList::read(QDomElement e, int division, int fileDivision)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "sig") {
                  SigEvent t;
                  int tick = t.read(e, division, fileDivision);
                  (*this)[tick] = t;
                  }
            else
                  domError(e);
            }
      normalize();
      }

//---------------------------------------------------------
//   SigEvent::write
//---------------------------------------------------------

void SigEvent::write(Xml& xml, int tick) const
      {
      xml.stag(QString("sig tick=\"%1\"").arg(tick));
      if ((nominator2 != nominator) || (denominator2 != denominator)) {
            xml.tag("nom2", nominator2);
            xml.tag("denom2", denominator2);
            }
      xml.tag("nom", nominator);
      xml.tag("denom", denominator);
      xml.etag();
      }

//---------------------------------------------------------
//   SigEvent::read
//---------------------------------------------------------

int SigEvent::read(QDomElement e, int division, int fileDivision)
      {
      int tick  = e.attribute("tick", "0").toInt();
      tick      = tick * division / fileDivision;

      denominator2 = -1;
      nominator2   = -1;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int i = e.text().toInt();

            if (tag == "nom")
                  nominator = i;
            else if (tag == "denom")
                  denominator = i;
            else if (tag == "nom2")
                  nominator2 = i;
            else if (tag == "denom2")
                  denominator2 = i;
            else
                  domError(e);
            }
      if ((nominator2 == -1) || (denominator2 == -1)) {
            nominator2   = nominator;
            denominator2 = denominator;
            }
      ticks = ticks_measure(nominator, denominator);
      return tick;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SigList::removeTime(int tick, int len)
      {
      SigList tmp;
      for (ciSigEvent i = begin(); i != end(); ++i) {
            if (i->first >= tick) {
                  if (i->first >= tick + len)
                        tmp.add(i->first - len, i->second);
                  else
                        printf("remove sig event\n");
                  }
            else
                  tmp.add(i->first, i->second);
            }
      clear();
      insert(tmp.begin(), tmp.end());
      normalize();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void SigList::insertTime(int tick, int len)
      {
      SigList tmp;
      for (ciSigEvent i = begin(); i != end(); ++i) {
            if (i->first >= tick)
                  tmp.add(i->first + len, i->second);
            else
                  tmp.add(i->first, i->second);
            }
      clear();
      insert(tmp.begin(), tmp.end());
      normalize();
      }

