//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __SPATIUM_H__
#define __SPATIUM_H__

//---------------------------------------------------------
//   Spatium
//    - a unit of measure
//    - the distance between two note lines
//    - used for many layout items
//---------------------------------------------------------

class Spatium {
      qreal _val;

   public:
      Spatium()                        { _val = 0.0; }
      explicit Spatium(qreal v)       { _val = v; }
      qreal val() const               { return _val; }
      bool operator>(const Spatium& a) const  { return _val > a._val; }
      bool operator<(const Spatium& a) const  { return _val < a._val; }
      bool operator==(const Spatium& a) const { return _val == a._val; }
      bool operator!=(const Spatium& a) const { return _val != a._val; }

      Spatium& operator+=(const Spatium& a) {
            _val += a._val;
            return *this;
            }
      Spatium& operator-=(const Spatium& a) {
            _val -= a._val;
            return *this;
            }
      Spatium& operator/=(qreal d) {
            _val /= d;
            return *this;
            }
      qreal operator/(const Spatium& b) {
            return _val / b._val;
            }
      Spatium& operator*=(int d) {
            _val *= d;
            return *this;
            }
      Spatium& operator*=(qreal d) {
            _val *= d;
            return *this;
            }
      Spatium operator-() const { return Spatium(-_val); }
      };

inline Spatium operator+(const Spatium& a, const Spatium& b)
      {
      Spatium r(a);
      r += b;
      return r;
      }
inline Spatium operator-(const Spatium& a, const Spatium& b)
      {
      Spatium r(a);
      r -= b;
      return r;
      }
inline Spatium operator/(const Spatium& a, qreal b)
      {
      Spatium r(a);
      r /= b;
      return r;
      }
inline Spatium operator*(const Spatium& a, int b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(int a, const Spatium& b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }
inline Spatium operator*(const Spatium& a, qreal b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(qreal a, const Spatium& b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }

#endif

