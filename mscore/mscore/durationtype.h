//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

//---------------------------------------------------------
//   Duration
//---------------------------------------------------------

class Duration {
   public:
      enum DurationType {
            V_LONG, V_BREVE, V_WHOLE, V_HALF, V_QUARTER, V_EIGHT, V_16TH,
            V_32ND, V_64TH, V_128TH, V_256TH, V_MEASURE, V_INVALID
            };
   private:
      DurationType _val;

   public:
      Duration() : _val(V_INVALID) {}
      Duration(DurationType t) : _val(t)    {};
      DurationType val() const              { return _val; }
      bool isValid() const                  { return _val != V_INVALID; }
      void setVal(int tick);
      void setVal(const QString&);
      void setType(DurationType t)          { _val = t; }

      int ticks() const;
      bool operator==(const Duration& t) const { return t._val == _val; }
      bool operator!=(const Duration& t) const { return t._val != _val; }
      static const int types = 12;
      QString name() const;
      int headType() const;               // note head type
      int hooks() const;
      bool hasStem() const;
      Duration shift(int val);
      };

extern int headType(int tickLen, Duration* type, int* dots);

#endif

