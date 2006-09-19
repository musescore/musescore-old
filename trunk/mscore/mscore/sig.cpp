//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: sig.cpp,v 1.11 2006/03/28 14:58:58 wschweer Exp $
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

#include "sig.h"
#include "xml.h"

const int division = 384;

//---------------------------------------------------------
//   ticks_beat
//---------------------------------------------------------

static int ticks_beat(int N)
      {
      int m = division;
      switch (N) {
            case  1:  m <<= 2; break;           // 1536
            case  2:  m <<= 1; break;           // 768
            case  4:  break;                    // 384
            case  8:  m >>= 1; break;           // 192
            case 16:  m >>= 2; break;           // 96
            case 32:  m >>= 3; break;           // 48
            case 64:  m >>= 4; break;           // 24
            case 128: m >>= 5; break;           // 12
            default:
                  fprintf(stderr, "bad divisor %d\n", N);
                  assert(false);
                  break;
            }
      return m;
      }

//---------------------------------------------------------
//   ticks_measure
//---------------------------------------------------------

int ticks_measure(int Z, int N)
      {
      return ticks_beat(N) * Z;
      }

//---------------------------------------------------------
//   SigEvent
//---------------------------------------------------------

SigEvent::SigEvent(int Z, int N)
      {
      irregular = false;
      z   = Z;
      n   = N;
      bar = 0;
      ticks = ticks_measure(z, n);
      }

SigEvent::SigEvent(int Z, int N, int t)
      {
      irregular = true;
      ticks = t;
      z   = Z;
      n   = N;
      bar = 0;
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

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void SigList::add(int tick, int ticks, int z, int n)
      {
      if (z == 0 || n == 0) {
            printf("illegal signature %d/%d\n", z, n);
            }
      (*this)[tick] = SigEvent(z, n, ticks);
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

      for (iSigEvent e = begin(); e != end(); ++e) {
            e->second.bar = bar + (e->first - tick) / tm;
            bar           = e->second.bar;
            tick          = e->first;
            if (!e->second.irregular) {
                  tm = ticks_measure(e->second.z, e->second.n);
                  e->second.ticks = tm;
                  }
            }
      }

//---------------------------------------------------------
//   ticksMeasure
//---------------------------------------------------------

int SigList::ticksMeasure(int tick) const
      {
      ciSigEvent i = upper_bound(tick);
      if (empty() || i == begin()) {
            printf("timesig(%d): not found\n", tick);
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
      ciSigEvent i = upper_bound(tick);
      if (empty() || i == begin()) {
            z = 4;
            n = 4;
            printf("timesig(%d): not found\n", tick);
            return;
            }
      --i;
      z = i->second.z;
      n = i->second.n;
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
      int ticksB = ticks_beat(e->second.n);
      int ticksM = ticksB * e->second.z;
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
            abort();
            }
      --e;
      int ticksB = ticks_beat(e->second.n);
      int ticksM = ticksB * e->second.z;
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
      xml.etag("siglist");
      }

//---------------------------------------------------------
//   SigList::read
//---------------------------------------------------------

void SigList::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            if (tag == "sig") {
                  SigEvent t;
                  int tick = t.read(node);
                  (*this)[tick] = t;
                  }
            else
                  printf("Mscore:SigList: unknown tag %s\n",
                     tag.toLatin1().data());
            }
      normalize();
      }

//---------------------------------------------------------
//   SigEvent::write
//---------------------------------------------------------

void SigEvent::write(Xml& xml, int tick) const
      {
      xml.stag("sig tick=\"%d\"", tick);
      if (irregular)
            xml.tag("ticks", ticks);
      xml.tag("nom", z);
      xml.tag("denom", n);
      xml.etag("sig");
      }

//---------------------------------------------------------
//   SigEvent::read
//---------------------------------------------------------

int SigEvent::read(QDomNode node)
      {
      irregular = false;

      QDomElement e = node.toElement();
      int tick = e.attribute("tick", "0").toInt();

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "nom")
                  z = i;
            else if (tag == "denom")
                  n = i;
            else if (tag == "ticks") {
                  ticks = i;
                  irregular = true;
                  }
            else
                  printf("Mscore:SigEvent: unknown tag %s\n",
                     tag.toLatin1().data());
            }
      if (!irregular)
            ticks = ticks_measure(z, n);
      return tick;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void SigList::removeTime(int, int)
      {
      printf("SigList::removeTime(): not impl.\n");
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void SigList::insertTime(int, int)
      {
      printf("SigList::insertTime(): not impl.\n");
      }

