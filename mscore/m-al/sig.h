//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sig.h 3549 2010-10-04 10:51:28Z wschweer $
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

#ifndef __AL_SIG_H__
#define __AL_SIG_H__

#include <map>
#include "fraction.h"

class XmlReader;

//-------------------------------------------------------------------
//   Time Signature Event
//    Incomplete measures as for example pickup measures have
//    a nominal duration different from actual duration.
//-------------------------------------------------------------------

class SigEvent {
      Fraction _timesig;
      Fraction _nominal;
      int _bar;               ///< precomputed value

   public:
      int read(XmlReader*, int fileDivision);
      // void write(Xml&, int) const;

      SigEvent() : _timesig(0, 0) {}       ///< default SigEvent is invalid
      SigEvent(const Fraction& s, int bar = 0) : _timesig(s), _bar(bar) {}

      bool operator==(const SigEvent& e) const;
      bool valid() const       { return _timesig.isValid(); }
      Fraction timesig() const { return _timesig;          }
      Fraction nominal() const { return _nominal;          }
      void setNominal(const Fraction& f) { _nominal = f;  }
      int bar() const          { return _bar;              }
      void setBar(int val)     { _bar = val;               }
      };

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

typedef std::map<const int, SigEvent>::iterator iSigEvent;
typedef std::map<const int, SigEvent>::const_iterator ciSigEvent;

class TimeSigMap : public std::map<const int, SigEvent > {
      void normalize();

   public:
      TimeSigMap();

      void add(int tick, const Fraction&);
      void add(int tick, const SigEvent& ev);

      void del(int tick);

      void read(XmlReader*, int fileDiv);
//      void write(Xml&) const;

      const SigEvent& timesig(int tick) const;

      void tickValues(int t, int* bar, int* beat, int* tick) const;
      int bar2tick(int bar, int beat, int tick) const;

      unsigned raster(unsigned tick, int raster) const;
      unsigned raster1(unsigned tick, int raster) const;    // round down
      unsigned raster2(unsigned tick, int raster) const;    // round up
      int rasterStep(unsigned tick, int raster) const;
      };

#endif
