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

//---------------------------------------------------------
//   Time Signature Event
//    for irregular measures the nominal values
//    nominator2 denominator2 are also given
//---------------------------------------------------------

struct SigEvent {
      int nominator, denominator;   ///< actual values
      int nominator2, denominator2; ///< nominal values for irregular measures
      int bar;                      ///< precomputed value
      int ticks;                    ///< ticks per measure, precomputed value

      int read(QDomElement, int fileDivision);
      void write(Xml&, int) const;

      SigEvent() { nominator = 0; }
      SigEvent(const Fraction&);                      ///< set regular event
      SigEvent(int z, int n);                         ///< set regular event
      SigEvent(int z1, int n1, int z2, int n2);       ///< set irregular event

      bool operator==(const SigEvent& e) const;
      bool valid() const { return nominator > 0; }
      QString print() const {
            if (valid())
                  return QString("%1/%2").arg(nominator).arg(denominator);
            else
                  return QString("void");
            }
      bool nominalEqual(const SigEvent& e) const {
            return (e.nominator2 == nominator2) && (e.denominator2 == denominator2);
            }
      bool nominalEqualActual() const {
            return (nominator == nominator2) && (denominator == denominator2);
            }
      Fraction fraction() const { return Fraction(nominator, denominator); }
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
      void add(int tick, int z, int n);
      void add(int tick, int z, int n, int z2, int n2);
      void add(int tick, int ticks, int z, int n);
      void add(int tick, const SigEvent& ev);

      void del(int tick);

      void read(QDomElement, int fileDiv);
      void write(Xml&) const;

//      SigEvent timesig(int tick) const;
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
      };

extern int ticks_measure(int nominator, int denominator);
extern int ticks_measure(const Fraction&);

}     // namespace AL
#endif
