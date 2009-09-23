//=============================================================================
//  AL
//  Audio Utility Library
//  $Id:$
//
//  Copyright (C) 2002-2009 by Werner Schweer and others
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

#ifndef __POS_H__
#define __POS_H__

#include "sig.h"

namespace AL {

class TempoList;
class SigList;
class Xml;

enum TType { TICKS, FRAMES };

//---------------------------------------------------------
//   Pos
//    depending on type _tick or _frame is a cached
//    value. When the tempomap changes, all cached values
//    are invalid. Sn is used to check for tempomap
//    changes.
//---------------------------------------------------------

class Pos {
      TType _type;
      mutable int sn;
      mutable unsigned _tick;
      mutable unsigned _frame;

   protected:
      TempoList* tempo;
      SigList*  sig;

   public:
      Pos(TempoList*, SigList*);
      Pos(TempoList*, SigList*, int measure, int beat, int tick);
      Pos(TempoList*, SigList*, int minute, int sec, int frame, int subframe);
      Pos(TempoList*, SigList*, unsigned, TType type = TICKS);
      Pos(TempoList*, SigList*, const QString&);

      void dump(int n = 0) const;

      unsigned time(TType t) const { return t == TICKS ? tick() : frame(); }
      void mbt(int* measure, int* beat, int* tick) const;
      void msf(int* minute, int* sec, int* frame, int* subframe) const;
      SigEvent timesig() const;
      void snap(int);
      void upSnap(int);
      void downSnap(int);
      Pos snaped(int) const;
      Pos upSnaped(int) const;
      Pos downSnaped(int) const;

      void invalidSn()  { sn = -1; }

      TType  type() const     { return _type; }
      void   setType(TType t);

      Pos& operator+=(const Pos& a);
      Pos& operator+=(int a);
      Pos& operator-=(const Pos& a);
      Pos& operator-=(int a);

      bool operator>=(const Pos& s) const;
      bool operator>(const Pos& s) const;
      bool operator<(const Pos& s) const;
      bool operator<=(const Pos& s) const;
      bool operator==(const Pos& s) const;
      bool operator!=(const Pos& s) const;

      friend Pos operator+(const Pos& a, const Pos& b);
      friend Pos operator-(const Pos& a, const Pos& b);
      friend Pos operator+(const Pos& a, int b);
      friend Pos operator-(const Pos& a, int b);

      unsigned tick() const;
      unsigned frame() const;
      void setTick(unsigned);
      void setFrame(unsigned);

      void write(Xml&, const char*) const;
      void read(QDomNode);
      bool isValid() const { return true; }
      };

//---------------------------------------------------------
//   PosLen
//---------------------------------------------------------

class PosLen : public Pos {
      mutable unsigned _lenTick;
      mutable unsigned _lenFrame;
      mutable int sn;

   public:
      PosLen(TempoList*, SigList*);
      PosLen(const PosLen&);
      void dump(int n = 0) const;

      void write(Xml&, const char*) const;
      void read(QDomNode);
      void setLenTick(unsigned);
      void setLenFrame(unsigned);
      unsigned lenTick() const;
      unsigned lenFrame() const;
      Pos end() const;
      unsigned endTick() const    { return end().tick(); }
      unsigned endFrame() const   { return end().frame(); }
      void setPos(const Pos&);

      bool operator==(const PosLen& s) const;
      };

}     // namespace AL
#endif
