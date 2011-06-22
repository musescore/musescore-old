//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
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
      double _val;

   public:
      Spatium()                        { _val = 0.0; }
      explicit Spatium(double v)       { _val = v; }
      double val() const               { return _val; }
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
      Spatium& operator/=(double d) {
            _val /= d;
            return *this;
            }
      double operator/(const Spatium& b) {
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
inline Spatium operator/(const Spatium& a, double b)
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
inline Spatium operator*(const Spatium& a, double b)
      {
      Spatium r(a);
      r *= b;
      return r;
      }
inline Spatium operator*(double a, const Spatium& b)
      {
      Spatium r(b);
      r *= a;
      return r;
      }

#endif

