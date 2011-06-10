//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: sig.cpp 3485 2010-09-21 09:08:48Z lasconic $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "al/al.h"

//---------------------------------------------------------
//   ticks_beat
//---------------------------------------------------------

static int ticks_beat(int n)
      {
      int m = (AL::division * 4) / n;
      return m;
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool SigEvent::operator==(const SigEvent& e) const
      {
      return (_timesig.identical(e._timesig));
      }

//---------------------------------------------------------
//   TimeSigMap
//---------------------------------------------------------

TimeSigMap::TimeSigMap()
      {
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void TimeSigMap::add(int tick, const Fraction& f)
      {
      if (!f.isValid()) {
            ; // printf("illegal signature %d/%d\n", f.numerator(), f.denominator());
            }
      (*this)[tick] = SigEvent(f);
      normalize();
      }

void TimeSigMap::add(int tick, const SigEvent& ev)
      {
      (*this)[tick] = ev;
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
//      int z    = 4;
//      int n    = 4;
      int tick = 0;
      int bar  = 0;

      for (iSigEvent i = begin(); i != end(); ++i) {
            SigEvent& e  = i->second;
            int tm = 1; // TODO
            e.setBar(bar + (i->first - tick) / tm);
            bar  = e.bar();
            tick = i->first;
            }
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
      if (empty()) {
            *bar = 0;
            *beat = 0;
            *tick = 0;
            return;
            }
      ciSigEvent e = upper_bound(t);
      if (empty() || e == begin()) {
            // fprintf(stderr, "tickValue(0x%x) not found\n", t);
            return;
            }
      --e;
      int delta  = t - e->first;
      int ticksB = ticks_beat(e->second.timesig().denominator());
      int ticksM = ticksB * e->second.timesig().numerator();
      if (ticksM == 0) {
            *bar = 0;
            *beat = 0;
            *tick = 0;
            return;
            }
      *bar       = e->second.bar() + delta / ticksM;
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
            if (bar < e->second.bar())
                  break;
            }
      if (empty() || e == begin()) {
//            fprintf(stderr, "TimeSigMap::bar2tick(): not found(%d,%d,%d) not found\n",
//               bar, beat, tick);
//            if (empty())
//                  fprintf(stderr, "   list is empty\n");
            // abort();
            return 0;
            }
      --e;
      int ticksB = ticks_beat(e->second.timesig().denominator());
      int ticksM = ticksB * e->second.timesig().numerator();
      return e->first + (bar - e->second.bar()) * ticksM + ticksB * beat + tick;
      }

//---------------------------------------------------------
//   TimeSigMap::read
//---------------------------------------------------------

void TimeSigMap::read(XmlReader* r, int fileDivision)
      {
      while (r->readElement()) {
            if (r->tag() == "sig") {
                  SigEvent t;
                  int tick = t.read(r, fileDivision);
                  (*this)[tick] = t;
                  }
            else
                  r->unknown();
            }
      normalize();
      }

//---------------------------------------------------------
//   SigEvent::read
//---------------------------------------------------------

int SigEvent::read(XmlReader* r, int fileDivision)
      {
      int tick = 0;
      while (r->readAttribute()) {
            if (r->tag() == "tick")
                  tick = r->intValue();
            }
      tick = tick * AL::division / fileDivision;

      int nominator;
      int denominator;
      int denominator2 = -1;
      int nominator2   = -1;

      while (r->readElement()) {
            if (r->readInt("nom", &nominator))
                  ;
            else if (r->readInt("denom", &denominator))
                  ;
            else if (r->readInt("nom2", &nominator2))
                  ;
            else if (r->readInt("denom2", &denominator2))
                  ;
            else
                  r->unknown();
            }
      if ((nominator2 == -1) || (denominator2 == -1)) {
            nominator2   = nominator;
            denominator2 = denominator;
            }
      _timesig = Fraction(nominator, denominator);
      _nominal = Fraction(nominator2, denominator2);
      return tick;
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
//            printf("TimeSigMap::raster(%x,)\n", t);
            // abort();
            return t;
            }
      int delta  = t - e->first;
      int ticksM = ticks_beat(e->second.timesig().denominator()) * e->second.timesig().numerator();
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
      int ticksM = ticks_beat(e->second.timesig().denominator()) * e->second.timesig().numerator();
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
      int ticksM = ticks_beat(e->second.timesig().denominator()) * e->second.timesig().numerator();
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
            return ticks_beat(e->second.timesig().denominator()) * e->second.timesig().numerator();
            }
      return raster;
      }

