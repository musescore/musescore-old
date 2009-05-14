//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: spatium.h,v 1.4 2006/03/02 17:08:43 wschweer Exp $
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

extern double _spatium; // spatium value in points
extern double _spatiumMag;

//---------------------------------------------------------
//   Spatium
//    - a unit of measure
//    - the distance between two note lines
//    - used for many layout items
//---------------------------------------------------------

class Spatium {
      double _val;

   public:
      Spatium()                        { _val = 0.0; }
      explicit Spatium(double v)       { _val = v; }
      double point() const             { return _val * _spatium; }
      double val() const               { return _val; }
      void set(double v)               { _val = v/_spatium; }
      bool operator>(Spatium a) const  { return _val > a._val; }
      bool operator<(Spatium a) const  { return _val < a._val; }
      bool operator==(Spatium a) const { return _val == a._val; }
      bool operator!=(Spatium a) const { return _val != a._val; }

      Spatium& operator+=(Spatium a) {
            _val += a._val;
            return *this;
            }
      Spatium& operator+=(double a) {
            _val += (a / _spatium);
            return *this;
            }
      Spatium& operator-=(Spatium a) {
            _val -= a._val;
            return *this;
            }
      Spatium& operator-=(double a) {
            _val -= (a / _spatium);
            return *this;
            }
      Spatium& operator/=(double d) {
            _val /= d;
            return *this;
            }
      double operator/(Spatium b) {
            return _val / b._val;
            }
      Spatium& operator*=(int d) {
            _val *= d;
            return *this;
            }
      Spatium& operator*=(double d) {
            _val *= d;
            return *this;
            }
      Spatium operator-() const { return Spatium(-_val); }

      friend Spatium spatium(double v);
      friend double point(Spatium s);
      };

inline Spatium operator+(Spatium a, Spatium b)
      {
      Spatium r(a);
      r += b;
      return r;
      }
inline Spatium operator-(Spatium a, Spatium b)
      {
      Spatium r(a);
      r -= b;
      return r;
      }
inline Spatium operator/(Spatium a, double b)
      {
      Spatium r(a);
      r /= b;
      return r;
      }
inline Spatium operator*(Spatium a, int b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(int a, Spatium b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }
inline Spatium operator*(Spatium a, double b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(double a, Spatium b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }

inline Spatium spatium(double v) { return Spatium(v / _spatium); }
inline double point(Spatium s)   { return s._val * _spatium; }

#endif

