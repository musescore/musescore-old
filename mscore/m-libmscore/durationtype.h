//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2008-2011 Werner Schweer
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

#ifndef __DURATIONTYPE_H__
#define __DURATIONTYPE_H__

#include <QtCore/QString>
#include <QtCore/QList>

#include "m-al/fraction.h"

//---------------------------------------------------------
//   TimeDuration
//---------------------------------------------------------

class TimeDuration {
   public:
      enum DurationType {
            V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            V_32ND, V_64TH, V_128TH, V_256TH, V_ZERO, V_MEASURE,  V_INVALID
            };
   private:
      DurationType _val;
      int _dots;

   public:
      TimeDuration() : _val(V_INVALID), _dots(0) {}
      TimeDuration(const Fraction&);
      TimeDuration(const QString&);
      TimeDuration(DurationType t) : _val(t), _dots(0) {};
      DurationType type() const             { return _val; }
      bool isValid() const                  { return _val != V_INVALID; }
      bool isZero() const                   { return _val == V_ZERO; }
      void setVal(int tick);
      void setType(DurationType t)          { _val = t; }
      void setType(const QString&);

      int ticks() const;
      bool operator==(const TimeDuration& t) const     { return t._val == _val && t._dots == _dots; }
      bool operator==(const DurationType& t) const { return t == _val; }
      bool operator!=(const TimeDuration& t) const     { return t._val != _val || t._dots != _dots; }
      bool operator<(const TimeDuration& t) const;
      bool operator>(const TimeDuration& t) const;
      bool operator>=(const TimeDuration& t) const;
      bool operator<=(const TimeDuration& t) const;
      TimeDuration& operator-=(const TimeDuration& t);
      TimeDuration operator-(const TimeDuration& t) const { return TimeDuration(*this) -= t; }
      TimeDuration& operator+=(const TimeDuration& t);
      TimeDuration operator+(const TimeDuration& t) const { return TimeDuration(*this) += t; }

      QString name() const;
      int headType() const;               // note head type
      int hooks() const;
      bool hasStem() const;
      TimeDuration shift(int val) const;
      int dots() const    { return _dots; }
      void setDots(int v) { _dots = v; }
      Fraction fraction() const;
      };

extern QList<TimeDuration> toDurationList(Fraction, bool useDottedValues);
#endif

