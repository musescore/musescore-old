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

//---------------------------------------------------------
//   Signature Event
//    for regular measures: ticks = ticks_measure(z, n)
//    for irregular measures z/n has no meaning
//---------------------------------------------------------

struct SigEvent {
      bool irregular;
      int ticks;
      int z, n;            // measure signature
      int bar;             // precomputed

      int read(QDomNode);
      void write(Xml&, int) const;

      SigEvent() { }
      SigEvent(int Z, int N);
      SigEvent(int, int, int);
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
      void add(int tick, int ticks, int z, int n);
      void del(int tick);

      void read(QDomNode);
      void write(Xml&) const;

      void timesig(int tick, int& z, int& n) const;
      void tickValues(int t, int* bar, int* beat, int* tick) const;
      int bar2tick(int bar, int beat, int tick) const;

      int ticksMeasure(int tick) const;

      void removeTime(int start, int len);
      void insertTime(int start, int len);
      };

extern int ticks_measure(int Z, int N);

#endif
