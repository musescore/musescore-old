//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: sig.h,v 1.3 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SIG_H__
#define __SIG_H__

class Xml;
class Score;

//---------------------------------------------------------
//   Time Signature Event
//    for irregular measures the nominal values
//    nominator2 denominator2 are also given
//---------------------------------------------------------

struct SigEvent {
      bool irregular;
      int nominator, denominator;   ///< actual values
      int nominator2, denominator2; ///< nominal values for irregular measures
      int bar;                      ///< precomputed value
      int ticks;                    ///< ticks per measure, precomputed value

      int read(QDomNode, Score*);
      void write(Xml&, int) const;

      SigEvent() { nominator = 0; }
      SigEvent(int, int);           ///< set regular event
      SigEvent(int, int, int, int); ///< set irregular event
      bool operator==(const SigEvent& e) const;
      bool valid() const { return nominator > 0; }
      };

//---------------------------------------------------------
//   SigList
//---------------------------------------------------------

typedef std::map<const int, SigEvent>::iterator iSigEvent;
typedef std::map<const int, SigEvent>::const_iterator ciSigEvent;

class SigList : public std::map<const int, SigEvent > {
      void normalize();

   public:
      SigList() {}
      void add(int tick, int z, int n);
      void add(int tick, int z, int n, int z2, int n2);
      void add(int tick, int ticks, int z, int n);
      void add(int tick, const SigEvent& ev);

      void del(int tick);

      void read(QDomNode, Score*);
      void write(Xml&) const;

      void timesig(int tick, int& z, int& n) const;
      SigEvent timesig(int tick) const;

      void tickValues(int t, int* bar, int* beat, int* tick) const;
      int bar2tick(int bar, int beat, int tick) const;

      int ticksMeasure(int tick) const;

      void removeTime(int start, int len);
      void insertTime(int start, int len);
      };

extern int ticks_measure(int Z, int N);

#endif
