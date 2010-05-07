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
#include "al.h"

namespace AL {

//---------------------------------------------------------
//   ticks_beat
//---------------------------------------------------------

static int ticks_beat(int n)
      {
      int m = (AL::division * 4) / n;
      if ((AL::division * 4) % n) {
            fprintf(stderr, "Mscore: ticks_beat(): bad divisor %d\n", n);
            abort();
            }
      return m;
      }

//---------------------------------------------------------
//   ticks_measure
//---------------------------------------------------------

int ticks_measure(const Fraction& f)
      {
      return (AL::division * 4 * f.numerator()) / f.denominator();
      }

//---------------------------------------------------------
//   SigEvent
//---------------------------------------------------------

SigEvent::SigEvent(const Fraction& f)
      {
      actual  = f;
      nominal = f;
      bar     = 0;
      ticks   = ticks_measure(f);
      }

SigEvent::SigEvent(const Fraction& a, const Fraction& n)
      {
      actual  = a;
      nominal = n;
      bar     = 0;
      ticks   = ticks_measure(a);
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SigEvent::operator==(const SigEvent& e) const
      {
      return (actual.identical(e.actual)) && (nominal.identical(e.nominal));
      }

//---------------------------------------------------------
//   TimeSigMap
//---------------------------------------------------------

TimeSigMap::TimeSigMap()
      {
      _serial = 0;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TimeSigMap::add(int tick, const Fraction& f)
      {
      if (!f.isValid()) {
            printf("illegal signature %d/%d\n", f.numerator(), f.denominator());
            }
      (*this)[tick] = SigEvent(f);
      normalize();
      }

void TimeSigMap::add(int tick, const Fraction& a, const Fraction& n)
      {
      (*this)[tick] = SigEvent(a, n);
      normalize();
      }

void TimeSigMap::add(int tick, const SigEvent& ev)
      {
      (*this)[tick] = ev;
      normalize();
      }

void TimeSigMap::add(int tick, int ticks, const Fraction& nominal)
      {
      Fraction actual(1, 4);
      if ((ticks % AL::division) == 0) {
            actual = Fraction(ticks / AL::division, 4);
            }
      else if ((ticks % (AL::division/2)) == 0) {
            actual = Fraction(ticks / (AL::division/2), 8);
            }
      else if ((ticks % (AL::division/4)) == 0) {
            actual = Fraction(ticks / (AL::division/4), 16);
            }
      else {
            printf("TimeSigMap::add(tick:%d, ticks:%d, z2:%d, n2:%d): irregular measure not supported\n",
               tick, ticks, nominal.numerator(), nominal.denominator());
            if (debugMsg)
                  abort();
            }

      (*this)[tick] = SigEvent(actual, nominal);
      normalize();
      }

//---------------------------------------------------------
//   del
//---------------------------------------------------------

void TimeSigMap::del(int tick)
      {
      erase(tick);
      normalize();
      }

//---------------------------------------------------------
//   TimeSigMap::normalize
//---------------------------------------------------------

void TimeSigMap::normalize()
      {
      int z    = 4;
      int n    = 4;
      int tick = 0;
      int bar  = 0;
      int tm   = ticks_measure(Fraction(z, n));

      for (iSigEvent i = begin(); i != end(); ++i) {
            SigEvent& e  = i->second;
            e.bar        = bar + (i->first - tick) / tm;
            bar          = e.bar;
            tick         = i->first;
            if (e.nominalEqualActual()) {
                  tm      = ticks_measure(e.fraction());
                  e.ticks = tm;
                  }
            }
      ++_serial;
      }

//---------------------------------------------------------
//   ticksMeasure
//---------------------------------------------------------

int TimeSigMap::ticksMeasure(int tick) const
      {
      return timesig(tick).ticks;
      }

//---------------------------------------------------------
//   timesig
//---------------------------------------------------------

const SigEvent& TimeSigMap::timesig(int tick) const
      {
      static const SigEvent ev(Fraction(4, 4));
      if (empty())
            return ev;
      ciSigEvent i = upper_bound(tick);
      if (i != begin())
            --i;
      return i->second;
      }

//---------------------------------------------------------
//   tickValues
//---------------------------------------------------------

void TimeSigMap::tickValues(int t, int* bar, int* beat, int* tick) const
      {
      ciSigEvent e = upper_bound(t);
      if (empty() || e == begin()) {
            fprintf(stderr, "tickValue(0x%x) not found\n", t);
            abort();
            }
      --e;
      int delta  = t - e->first;
      int ticksB = ticks_beat(e->second.fraction().denominator());
      int ticksM = ticksB * e->second.fraction().numerator();
      if (ticksM == 0) {
            printf("TimeSigMap::tickValues: at %d %s\n", t, qPrintable(e->second.fraction().print()));
            *bar = 0;
            *beat = 0;
            *tick = 0;
            return;
            }
      *bar       = e->second.bar + delta / ticksM;
      int rest   = delta % ticksM;
      *beat      = rest / ticksB;
      *tick      = rest % ticksB;
      }

//---------------------------------------------------------
//   bar2tick
//---------------------------------------------------------

int TimeSigMap::bar2tick(int bar, int beat, int tick) const
      {
      ciSigEvent e;

      for (e = begin(); e != end(); ++e) {
            if (bar < e->second.bar)
                  break;
            }
      if (empty() || e == begin()) {
            fprintf(stderr, "TimeSigMap::bar2tick(): not found(%d,%d,%d) not found\n",
               bar, beat, tick);
            if (empty())
                  fprintf(stderr, "   list is empty\n");
            // abort();
            return 0;
            }
      --e;
      int ticksB = ticks_beat(e->second.fraction().denominator());
      int ticksM = ticksB * e->second.fraction().numerator();
      return e->first + (bar - e->second.bar) * ticksM + ticksB * beat + tick;
      }

//---------------------------------------------------------
//   TimeSigMap::write
//---------------------------------------------------------

void TimeSigMap::write(Xml& xml) const
      {
      xml.stag("siglist");
      for (ciSigEvent i = begin(); i != end(); ++i)
            i->second.write(xml, i->first);
      xml.etag();
      }

//---------------------------------------------------------
//   TimeSigMap::read
//---------------------------------------------------------

void TimeSigMap::read(QDomElement e, int fileDivision)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "sig") {
                  SigEvent t;
                  int tick = t.read(e, fileDivision);
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
      if (!nominalEqualActual()) {
            xml.tag("nom2",   nominal.numerator());
            xml.tag("denom2", nominal.denominator());
            }
      xml.tag("nom",   actual.numerator());
      xml.tag("denom", actual.denominator());
      xml.etag();
      }

//---------------------------------------------------------
//   SigEvent::read
//---------------------------------------------------------

int SigEvent::read(QDomElement e, int fileDivision)
      {
      int tick  = e.attribute("tick", "0").toInt();
      tick      = tick * AL::division / fileDivision;

      int nominator;
      int denominator;
      int denominator2 = -1;
      int nominator2   = -1;
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
      actual = Fraction(nominator, denominator);
      nominal = Fraction(nominator2, denominator2);
      ticks = ticks_measure(actual);
      return tick;
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void TimeSigMap::removeTime(int tick, int len)
      {
// printf("TimeSigMap::removeTime %d len %d\n", tick, len);
      TimeSigMap tmp;
      for (ciSigEvent i = begin(); i != end(); ++i) {
            // entry at tick 0 is sticky
            if ((i->first >= tick) && (i->first != 0)) {
                  if (i->first >= tick + len)
                        tmp.add(i->first - len, i->second);
                  else
                        printf("TimeSigMap::remove sig event\n");
                  }
            else
                  tmp.add(i->first, i->second);
            }
      *this = tmp;
//      clear();
//      insert(tmp.begin(), tmp.end());
      normalize();
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void TimeSigMap::insertTime(int tick, int len)
      {
// printf("TimeSigMap::insertTime %d len %d\n", tick, len);
      TimeSigMap tmp;
      for (ciSigEvent i = begin(); i != end(); ++i) {
            // entry at tick 0 is sticky
            if (i->first >= tick && (i->first != 0))
                  tmp.add(i->first + len, i->second);
            else
                  tmp.add(i->first, i->second);
            }
      *this = tmp;
//      clear();
//      insert(tmp.begin(), tmp.end());
      normalize();
      }

//---------------------------------------------------------
//   raster
//---------------------------------------------------------

unsigned TimeSigMap::raster(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      ciSigEvent e = upper_bound(t);
      if (e == end()) {
            printf("TimeSigMap::raster(%x,)\n", t);
            // abort();
            return t;
            }
      int delta  = t - e->first;
      int ticksM = ticks_beat(e->second.fraction().denominator()) * e->second.fraction().numerator();
      if (raster == 0)
            raster = ticksM;
      int rest   = delta % ticksM;
      int bb     = (delta/ticksM)*ticksM;
      return  e->first + bb + ((rest + raster/2)/raster)*raster;
      }

//---------------------------------------------------------
//   raster1
//    round down
//---------------------------------------------------------

unsigned TimeSigMap::raster1(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      ciSigEvent e = upper_bound(t);

      int delta  = t - e->first;
      int ticksM = ticks_beat(e->second.fraction().denominator()) * e->second.fraction().numerator();
      if (raster == 0)
            raster = ticksM;
      int rest   = delta % ticksM;
      int bb     = (delta/ticksM)*ticksM;
      return  e->first + bb + (rest/raster)*raster;
      }

//---------------------------------------------------------
//   raster2
//    round up
//---------------------------------------------------------

unsigned TimeSigMap::raster2(unsigned t, int raster) const
      {
      if (raster == 1)
            return t;
      ciSigEvent e = upper_bound(t);

      int delta  = t - e->first;
      int ticksM = ticks_beat(e->second.fraction().denominator()) * e->second.fraction().numerator();
      if (raster == 0)
            raster = ticksM;
      int rest   = delta % ticksM;
      int bb     = (delta/ticksM)*ticksM;
      return  e->first + bb + ((rest+raster-1)/raster)*raster;
      }

//---------------------------------------------------------
//   rasterStep
//---------------------------------------------------------

int TimeSigMap::rasterStep(unsigned t, int raster) const
      {
      if (raster == 0) {
            ciSigEvent e = upper_bound(t);
            return ticks_beat(e->second.fraction().denominator()) * e->second.fraction().numerator();
            }
      return raster;
      }

//---------------------------------------------------------
//   measureRest
//    return (measure end - tick) as a fraction
//    (silly implementation)
//---------------------------------------------------------

Fraction TimeSigMap::measureRest(unsigned tick) const
      {
      const SigEvent& e = timesig(tick);
      int bar, beat, restTicks;
      tickValues(tick, &bar, &beat, &restTicks);
      int stick = bar2tick(bar, 0, 0);
      return e.fraction() - Fraction::fromTicks(tick - stick);
      }
}     // namespace AL

