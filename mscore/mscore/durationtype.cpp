//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: utils.cpp,v 1.24 2006/03/02 17:08:43 wschweer Exp $
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

//---------------------------------------------------------
//   headType
//    return for a given tickLen the baselen of a note
//    (which determines the head symbol) and a number of
//    dots (<= 2)
//
//    return the remaining ticks if any
//---------------------------------------------------------

int headType(int tickLen, Duration* type, int* dots)
      {
      if (tickLen == 0) {
            *type = Duration(Duration::V_MEASURE);
            *dots = 0;
            return 0;
            }
      Duration dt;
      for (int i = 0; i < Duration::types - 1; ++i) {
            dt.setType(Duration::DurationType(i));
            int ticks = dt.ticks();
            if (tickLen / ticks) {
                  int remain = tickLen % ticks;
                  if ((ticks - remain) < (ticks/4)) {
                        *dots = 0;
                        *type = Duration(Duration::DurationType(i-1));
                        return 0;
                        }
                  *type = dt;
                  return getDots(ticks, remain, dots);
                  }
            }
printf("1: no duration type for ticks %d\n", tickLen);
      *type = Duration(Duration::V_QUARTER);
      *dots = 0;
      return 0;
      }

//---------------------------------------------------------
//   setVal
//---------------------------------------------------------

void Duration::setVal(int ticks)
      {
      if (ticks == 0) {
            _val = V_MEASURE;
            }
      else {
            Duration dt;
            for (int i = 0; i < Duration::types - 1; ++i) {
                  dt.setType(Duration::DurationType(i));
                  int t = dt.ticks();
                  if (ticks / t) {
                        int remain = ticks % t;
                        if ((t - remain) < (t/4))
                              _val = DurationType(i - 1);
                        else
                              _val = DurationType(i);
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
      switch(_val) {
            case V_QUARTER:   return division;
            case V_EIGHT:     return division / 2;
            case V_256TH:     return division / 64;
            case V_128TH:     return division / 32;
            case V_64TH:      return division / 16;
            case V_32ND:      return division / 8;
            case V_16TH:      return division / 4;
            case V_HALF:      return division * 2;
            case V_WHOLE:     return division * 4;
            case V_BREVE:     return division * 8;
            case V_LONG:      return division * 16;
            case V_MEASURE:   return 0;
            default:
            case V_INVALID:   return -1;
            }
      }

int Duration::ticks(int dots) const
      {
      int t = ticks();
      for (int i = 0; i < dots; ++i) {
            t += (t >> (i+1));
            }
      return t;
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
            case V_WHOLE:
            case V_MEASURE:   return "whole";
            case V_BREVE:     return "breve";
            case V_LONG:      return "long";
            default:
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
         // V_32ND, V_64TH, V_128TH, V_256TH, V_MEASURE, V_INVALID
            3,      4,       5,       6,      0,         0
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

void Duration::setVal(const QString& s)
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
      else
            printf("unknown duration type <%s>\n", qPrintable(s));
      }

//---------------------------------------------------------
//   shift
//---------------------------------------------------------

Duration Duration::shift(int v)
      {
      if (_val == V_MEASURE || _val == V_INVALID)
            return Duration();
      int newValue = _val + v;
      if ((newValue < 0) || (newValue > V_256TH))
            return Duration();
      return Duration(DurationType(newValue));
      }

