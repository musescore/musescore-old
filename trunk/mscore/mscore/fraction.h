//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __FRACTION_H__
#define __FRACTION_H__

#include "al/al.h"

//---------------------------------------------------------
//   Fraction
//---------------------------------------------------------

class Fraction {
      int _numerator;
      int _denominator;

      void reduce();

   public:
      Fraction(int z = 0, int n = 1);
      int numerator() const      { return _numerator;           }
      int denominator() const    { return _denominator;            }
      void setNumerator(int v)   { _numerator = v;              }
      void setDenominator(int v) { _denominator = v;               }
      void set(int z, int n)     { _numerator = z; _denominator = n; }
      bool isZero() const        { return _numerator == 0;      }

      int ticks() const          { return (_numerator * AL::division * 4 + (_denominator/2)) / _denominator; }
      static Fraction fromTicks(int v);

      Fraction& operator+=(const Fraction&);
      Fraction& operator-=(const Fraction&);
      Fraction& operator*=(const Fraction&);
      Fraction& operator*=(int);
      Fraction& operator/=(const Fraction&);

      Fraction operator+(const Fraction& v) const { return Fraction(*this) += v; }
      Fraction operator-(const Fraction& v) const { return Fraction(*this) -= v; }
      Fraction operator*(const Fraction& v) const { return Fraction(*this) *= v; }
      Fraction operator*(int v)             const { return Fraction(*this) *= v; }
      Fraction operator/(const Fraction& v) const { return Fraction(*this) /= v; }

      bool operator<(const Fraction&) const;
      bool operator<=(const Fraction&) const;
      bool operator>=(const Fraction&) const;
      bool operator>(const Fraction&) const;
      bool operator==(const Fraction&) const;
      bool operator!=(const Fraction&) const;
      };

#endif

