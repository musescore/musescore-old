//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#ifndef __PITCHVALUE_H__
#define __PITCHVALUE_H__

//---------------------------------------------------------
//   PitchValue
//    used in class Bend, BendCanvas
//
//    - time is 0 - 60 for 0-100% of the chord duration the
//      bend is attached to
//    - pitch is 100 for one semitone
//---------------------------------------------------------

struct PitchValue {
      int time;
      int pitch;
      bool vibrato;
      PitchValue() {}
      PitchValue(int a, int b, bool c = false) : time(a), pitch(b), vibrato(c) {}
      };

#endif

