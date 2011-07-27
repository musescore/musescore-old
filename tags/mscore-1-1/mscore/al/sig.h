//=============================================================================
//  MusE Score
//  Linux Music Score Editor
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

#ifndef __AL_SIG_H__
#define __AL_SIG_H__

#include "fraction.h"

namespace AL {
class Xml;

//-------------------------------------------------------------------
//   Time Signature Event
//    Incomplete measures as for example pickup measures have
//    a nominal duration different from actual duration.
//-------------------------------------------------------------------

struct SigEvent {
      Fraction actual;
      Fraction nominal;
      int bar;                ///< precomputed value
      int ticks;              ///< ticks per measure, precomputed value

      int read(QDomElement, int fileDivision);
      void write(Xml&, int) const;

      SigEvent() : actual(0, 0) {}                    ///< default SigEvent is invalid
      SigEvent(const Fraction&);                      ///< set regular event
      SigEvent(const Fraction&, const Fraction&);     ///< set irregular event

      bool operator==(const SigEvent& e) const;
      bool valid() const { return actual.isValid(); }
      QString print() const {
            return valid() ? actual.print() : QString("void");
            }
      bool nominalEqual(const SigEvent& e) const {
            return nominal.identical(e.nominal);
            }
      bool nominalEqualActual() const { return actual.identical(nominal); }
      Fraction fraction() const       { return actual;            }
      Fraction getNominal() const     { return nominal;           }
      };

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

typedef std::map<const int, SigEvent>::iterator iSigEvent;
typedef std::map<const int, SigEvent>::const_iterator ciSigEvent;

class TimeSigMap : public std::map<const int, SigEvent > {
      unsigned _serial;
      void normalize();

   public:
      TimeSigMap();
      void add(int tick, const Fraction&);
      void add(int tick, const Fraction& a, const Fraction& n);
      void add(int tick, int ticks, const Fraction& nominal);
      void add(int tick, const SigEvent& ev);

      void del(int tick);

      void read(QDomElement, int fileDiv);
      void write(Xml&) const;
      void dump() const;

      const SigEvent& timesig(int tick) const;

      void tickValues(int t, int* bar, int* beat, int* tick) const;
      int bar2tick(int bar, int beat, int tick) const;

      int ticksMeasure(int tick) const;

      void removeTime(int start, int len);
      void insertTime(int start, int len);
      int serial() const { return _serial; }
      unsigned raster(unsigned tick, int raster) const;
      unsigned raster1(unsigned tick, int raster) const;    // round down
      unsigned raster2(unsigned tick, int raster) const;    // round up
      int rasterStep(unsigned tick, int raster) const;
      Fraction measureRest(unsigned tick) const;
      };

extern int ticks_measure(const Fraction&);

}     // namespace AL
#endif
