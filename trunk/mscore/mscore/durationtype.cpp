//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "durationtype.h"

extern int division;

//---------------------------------------------------------
//   dots
//---------------------------------------------------------

static int getDots(int base, int rest, int* dots)
      {
      *dots = 0;
      if (rest >= base / 2) {
            *dots = *dots + 1;
            rest -= base / 2;
            }
      if (rest >= base / 4) {
            *dots = *dots + 1;
            rest -= base / 4;
            }
      return rest;
      }

#if 0
//---------------------------------------------------------
//   headType
//    return for a given tickLen the baselen of a note
//    (which determines the head symbol) and a number of
//    dots (<= 2)
//
//    return the remaining ticks if any
//---------------------------------------------------------

static int headType(int tickLen, Duration* type)
      {
      if (tickLen == 0) {
            *type = Duration(Duration::V_MEASURE);
            return 0;
            }
      Duration dt;
      for (int i = 0; i < Duration::V_ZERO; ++i) {
            dt.setType(Duration::DurationType(i));
            int ticks = dt.ticks();
            if (tickLen / ticks) {
                  int remain = tickLen % ticks;
                  if ((ticks - remain) < (ticks/4)) {
                        *type = Duration(Duration::DurationType(i-1));
                        return 0;
                        }
                  *type = dt;
                  int dots;
                  int rest = getDots(ticks, remain, &dots);
                  dt.setDots(dots);
                  return rest;
                  }
            }
printf("1: no duration type for ticks %d\n", tickLen);
      *type = Duration(Duration::V_QUARTER);
      return 0;
      }
#endif

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void Duration::setVal(int ticks)
      {
      if (ticks == 0)
            _val = V_MEASURE;
      else {
            Duration dt;
            for (int i = 0; i < Duration::V_ZERO; ++i) {
                  dt.setType(Duration::DurationType(i));
                  int t = dt.ticks();
                  if (ticks / t) {
                        int remain = ticks % t;
                        if ((t - remain) < (t/4)) {
                              _val = DurationType(i - 1);
                              return;
                              }
                        _val = DurationType(i);
                        getDots(t, remain, &_dots);
                        return;
                        }
                  }
            printf("2: no duration type for ticks %d\n", ticks);
            _val = V_QUARTER;       // fallback default value
            }
      }

//---------------------------------------------------------
//   ticks
//---------------------------------------------------------

int Duration::ticks() const
      {
      int t;
      switch(_val) {
            case V_QUARTER:   t = division;        break;
            case V_EIGHT:     t = division / 2;    break;
            case V_256TH:     t = division / 64;   break;
            case V_128TH:     t = division / 32;   break;
            case V_64TH:      t = division / 16;   break;
            case V_32ND:      t = division / 8;    break;
            case V_16TH:      t = division / 4;    break;
            case V_HALF:      t = division * 2;    break;
            case V_WHOLE:     t = division * 4;    break;
            case V_BREVE:     t = division * 8;    break;
            case V_LONG:      t = division * 16;   break;
            case V_ZERO:
            case V_MEASURE:
                  return 0;
            default:
            case V_INVALID:
                  return -1;
            }
      int tmp = t;
      for (int i = 0; i < _dots; ++i)
            tmp += (t >> (i+1));
      return tmp;
      }

//---------------------------------------------------------
//   name
//---------------------------------------------------------

QString Duration::name() const
      {
      switch(_val) {
            case V_QUARTER:   return "quarter";
            case V_EIGHT:     return "eighth";
            case V_256TH:     return "256th";
            case V_128TH:     return "128th";
            case V_64TH:      return "64th";
            case V_32ND:      return "32nd";
            case V_16TH:      return "16th";
            case V_HALF:      return "half";
            case V_WHOLE:     return "whole";
            case V_MEASURE:   return "measure";
            case V_BREVE:     return "breve";
            case V_LONG:      return "long";
            default:
printf("Duration::name(): invalid duration type %d\n", _val);
            case V_ZERO:
            case V_INVALID:   return "";
            }
      }

//---------------------------------------------------------
//   headType
//---------------------------------------------------------

int Duration::headType() const
      {
      int headType = 0;
      switch(_val) {
            case V_256TH:
            case V_128TH:
            case V_64TH:
            case V_32ND:
            case V_16TH:
            case V_EIGHT:
            case V_QUARTER:
                  headType = 2;
                  break;
            case V_HALF:
                  headType = 1;
                  break;
            case V_MEASURE:
            case V_WHOLE:
                  headType = 0;
                  break;
            case V_BREVE:
                  headType = 3;
                  break;
            case V_LONG:
                  headType = 3;
                  break;
            default:
            case V_INVALID:
            case V_ZERO:
                  headType = 2;
                  break;
            }
      return headType;
      }

//---------------------------------------------------------
//   hooks
//---------------------------------------------------------

int Duration::hooks() const
      {
      static const int table[] = {
         // V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            0,      0,       0,       0,      0,         1,       2,
         // V_32ND, V_64TH, V_128TH, V_256TH, V_MEASURE, V_ZERO, V_INVALID
            3,      4,       5,       6,      0,         0,       0
            };
      return table[_val];
      }

//---------------------------------------------------------
//   hasStem
//---------------------------------------------------------

bool Duration::hasStem() const
      {
      switch(_val) {
            case V_256TH:
            case V_128TH:
            case V_64TH:
            case V_32ND:
            case V_16TH:
            case V_EIGHT:
            case V_QUARTER:
            case V_HALF:
            case V_LONG:
                  return true;
            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

Duration::Duration(const QString& s)
      {
      setType(s);
      _dots = 0;
      }

//---------------------------------------------------------
//   setType
//---------------------------------------------------------

void Duration::setType(const QString& s)
      {
      if (s == "quarter")
            _val = V_QUARTER;
      else if (s == "eighth")
            _val = V_EIGHT;
      else if (s == "256th")
            _val = V_256TH;
      else if (s == "128th")
            _val = V_128TH;
      else if (s == "64th")
            _val = V_64TH;
      else if (s == "32nd")
            _val = V_32ND;
      else if (s == "16th")
            _val = V_16TH;
      else if (s == "half")
            _val = V_HALF;
      else if (s == "whole")
            _val = V_WHOLE;
      else if (s == "breve")
            _val = V_BREVE;
      else if (s == "long")
            _val = V_LONG;
      else if (s == "measure")
            _val = V_MEASURE;
      else {
            _val = V_INVALID;
            printf("Duration::setVal(%s): unknown\n", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   shift
//---------------------------------------------------------

Duration Duration::shift(int v) const
      {
      if (_val == V_MEASURE || _val == V_INVALID || _val == V_ZERO)
            return Duration();
      int newValue = _val + v;
      if ((newValue < 0) || (newValue > V_256TH))
            return Duration();
      return Duration(DurationType(newValue));
      }

//---------------------------------------------------------
//   operator<
//---------------------------------------------------------

bool Duration::operator<(const Duration& t) const
      {
      if (t._val < _val)
            return true;
      if (t._val == _val) {
            if (_dots < t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator>=
//---------------------------------------------------------

bool Duration::operator>=(const Duration& t) const
      {
      if (t._val > _val)
            return true;
      if (t._val == _val) {
            if (_dots >= t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator<=
//---------------------------------------------------------

bool Duration::operator<=(const Duration& t) const
      {
      if (t._val < _val)
            return true;
      if (t._val == _val) {
            if (_dots <= t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   operator>
//---------------------------------------------------------

bool Duration::operator>(const Duration& t) const
      {
      if (t._val > _val)
            return true;
      if (t._val == _val) {
            if (_dots > t._dots)
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   fraction
//---------------------------------------------------------

Fraction Duration::fraction() const
      {
      int z = 1;
      unsigned n;
      switch(_val) {
            case V_256TH:     n = 256;      break;
            case V_128TH:     n = 128;      break;
            case V_64TH:      n = 64;       break;
            case V_32ND:      n = 32;       break;
            case V_16TH:      n = 16;       break;
            case V_EIGHT:     n = 8;        break;
            case V_QUARTER:   n = 4;        break;
            case V_HALF:      n = 2;        break;
            case V_WHOLE:     n = 1;        break;
            case V_BREVE:     z = 2; n = 1; break;
            case V_LONG:      z = 4; n = 1; break;
            case V_ZERO:      z = 0; n = 1; break;
            default:          n = 0;        break;
            }
      Fraction a(z, n);
      for (int i = 0; i < _dots; ++i) {
            n *= 2;
            a += Fraction(1, n);
            }
      return a;
      }

Duration::Duration(const Fraction& f)
      {
      _dots = 0;
      if (f.zaehler() == 0) {
            _val  = V_ZERO;
            _dots = 0;
            return;
            }
      switch(f.nenner()) {
            case 1:     _val = V_WHOLE; break;
            case 2:     _val = V_HALF; break;
            case 4:     _val = V_QUARTER; break;
            case 8:     _val = V_EIGHT; break;
            case 16:    _val = V_16TH; break;
            case 32:    _val = V_32ND; break;
            case 64:    _val = V_64TH; break;
            case 128:   _val = V_128TH; break;
            case 256:   _val = V_256TH; break;
            default:    _val = V_INVALID; break;
            }
      if (f.zaehler() != 1) {
            switch(f.zaehler()) {
                  case 3:
                        _val = DurationType(_val - 1);
                        _dots = 1;
                        break;
                  case 7:
                        _val = DurationType(_val - 2);
                        _dots = 2;
                        break;
                  default:
                        printf("Duration(%d/%d): not implemented\n", f.zaehler(), f.nenner());
                        abort();
                  }
            }
      }

//---------------------------------------------------------
//   operator -=
//---------------------------------------------------------

Duration& Duration::operator-=(const Duration& t)
      {
      Fraction f1 = fraction() - t.fraction();
      Duration d(f1);
      _val  = d._val;
      _dots = d._dots;
      return *this;
      }

//---------------------------------------------------------
//   operator +=
//---------------------------------------------------------

Duration& Duration::operator+=(const Duration& t)
      {
      Fraction f1 = fraction() + t.fraction();
      Duration d(f1);
      _val  = d._val;
      _dots = d._dots;
      return *this;
      }


//---------------------------------------------------------
//   toDurationList
//---------------------------------------------------------

QList<Duration> toDurationList(Fraction l, bool useDottedValues)
      {
      QList<Duration> dList;
      if (useDottedValues) {
            for (Duration d = Duration(Duration::V_LONG); (d.type() != Duration::V_ZERO) && (l.zaehler() != 0);) {
                  d.setDots(2);
                  Fraction ff(l - d.fraction());
                  if (ff.zaehler() >= 0) {
                        dList.append(d);
                        l -= d.fraction();
                        continue;
                        }
                  d.setDots(1);
                  ff = l - d.fraction();
                  if (ff.zaehler() >= 0) {
                        dList.append(d);
                        l -= d.fraction();
                        continue;
                        }
                  d.setDots(0);
                  ff = l - d.fraction();
                  if (ff.zaehler() < 0) {
                        d = d.shift(1);
                        }
                  else {
                        l -= d.fraction();
                        dList.append(d);
                        }
                  }
            }
      else {
            for (Duration d = Duration(Duration::V_LONG); !d.isZero() && (l.zaehler() != 0);) {
                  Fraction ff(l - d.fraction());
                  if (ff.zaehler() < 0) {
                        d = d.shift(1);
                        continue;
                        }
                  l -= d.fraction();
                  dList.append(d);
                  }
            }
      return dList;
      }

