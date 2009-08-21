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

#include "fraction.h"

//---------------------------------------------------------
//   ggT
//    greatest common divisor
//---------------------------------------------------------

static int ggT(int a, int b)
      {
      if (b == 0)
            return a < 0 ? -a : a;
      return ggT(b, a % b);
      }

//---------------------------------------------------------
//   kgV
//    least common divisor
//---------------------------------------------------------

static unsigned kgV(int a, int b)
      {
      return a * b / ggT(a, b);
      }

//---------------------------------------------------------
//   Fraction
//---------------------------------------------------------

Fraction::Fraction(int z, int n)
   : _zaehler(z), _nenner(n)
      {
      kuerzen();
      }

//---------------------------------------------------------
//   kuerzen
//---------------------------------------------------------

void Fraction::kuerzen()
      {
      int tmp = ggT(_zaehler, _nenner);
      _zaehler /= tmp;
      _nenner  /= tmp;
      }

//---------------------------------------------------------
//   operator+=
//---------------------------------------------------------

Fraction& Fraction::operator+=(const Fraction& val)
      {
      const unsigned tmp = kgV(_nenner, val._nenner);
      _zaehler = _zaehler * (tmp / _nenner) + val._zaehler * (tmp / val._nenner);
      _nenner  = tmp;
      kuerzen();
      return *this;
      }

bool Fraction::operator<(const Fraction& val) const
      {
      const unsigned v = kgV(_nenner, val._nenner);
      return _zaehler * (v / _nenner) < val._zaehler * (v / val._nenner);
      }

bool Fraction::operator<=(const Fraction& val) const
      {
      const unsigned v = kgV(_nenner, val._nenner);
      return _zaehler * (v / _nenner) <= val._zaehler * (v / val._nenner);
      }

bool Fraction::operator>=(const Fraction& val) const
      {
      const unsigned v = kgV(_nenner, val._nenner);
      return _zaehler * (v / _nenner) >= val._zaehler * (v / val._nenner);
      }

bool Fraction::operator>(const Fraction& val) const
      {
      const unsigned v = kgV(_nenner, val._nenner);
      return _zaehler * (v / _nenner) > val._zaehler * (v / val._nenner);
      }

bool Fraction::operator==(const Fraction& val) const
      {
      return (_zaehler == val._zaehler) && (_nenner == val._nenner);
      }

bool Fraction::operator!=(const Fraction& val) const
      {
      return (_zaehler != val._zaehler) || (_nenner != val._nenner);
      }

//---------------------------------------------------------
//   operator-=
//---------------------------------------------------------

Fraction& Fraction::operator-=(const Fraction& val)
      {
      const unsigned tmp = kgV(_nenner, val._nenner);
      _zaehler = _zaehler * (tmp / _nenner) - val._zaehler * (tmp / val._nenner);
      _nenner  = tmp;
      kuerzen();
      return *this;
      }

//---------------------------------------------------------
//   operator*=
//---------------------------------------------------------

Fraction& Fraction::operator*=(const Fraction& val)
      {
      _zaehler *= val._zaehler;
      _nenner  *= val._nenner;
      kuerzen();
      return *this;
      }

Fraction& Fraction::operator*=(int val)
      {
      _zaehler *= val;
      kuerzen();
      return *this;
      }

//---------------------------------------------------------
//   operator/=
//---------------------------------------------------------

Fraction& Fraction::operator/=(const Fraction& val)
      {
      _zaehler *= val._nenner;
      _nenner  *= val._zaehler;
      kuerzen();
      return *this;
      }

//---------------------------------------------------------
//   fromTicks
//---------------------------------------------------------

Fraction Fraction::fromTicks(int ticks)
      {
      return Fraction(ticks, 1) / Fraction(division * 4, 1);
      }

