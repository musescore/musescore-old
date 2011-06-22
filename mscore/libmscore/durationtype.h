//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2008-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __DURATIONTYPE_H__
#define __DURATIONTYPE_H__

#include "al/fraction.h"

//---------------------------------------------------------
//   Duration
//---------------------------------------------------------

class Duration {
   public:
      enum DurationType {
            V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            V_32ND, V_64TH, V_128TH, V_256TH, V_ZERO, V_MEASURE,  V_INVALID
            };
   private:
      DurationType _val;
      int _dots;

   public:
      Duration() : _val(V_INVALID), _dots(0) {}
      Duration(const Fraction&);
      Duration(const QString&);
      Duration(DurationType t) : _val(t), _dots(0) {};
      DurationType type() const             { return _val; }
      bool isValid() const                  { return _val != V_INVALID; }
      bool isZero() const                   { return _val == V_ZERO; }
      void setVal(int tick);
      void setType(DurationType t)          { _val = t; }
      void setType(const QString&);

      int ticks() const;
      bool operator==(const Duration& t) const     { return t._val == _val && t._dots == _dots; }
      bool operator==(const DurationType& t) const { return t == _val; }
      bool operator!=(const Duration& t) const     { return t._val != _val || t._dots != _dots; }
      bool operator<(const Duration& t) const;
      bool operator>(const Duration& t) const;
      bool operator>=(const Duration& t) const;
      bool operator<=(const Duration& t) const;
      Duration& operator-=(const Duration& t);
      Duration operator-(const Duration& t) const { return Duration(*this) -= t; }
      Duration& operator+=(const Duration& t);
      Duration operator+(const Duration& t) const { return Duration(*this) += t; }

      QString name() const;
      int headType() const;               // note head type
      int hooks() const;
      bool hasStem() const;
      Duration shift(int val) const;
      int dots() const    { return _dots; }
      void setDots(int v) { _dots = v; }
      Fraction fraction() const;
      void print() const;
      };

extern QList<Duration> toDurationList(Fraction, bool useDottedValues);
#endif

