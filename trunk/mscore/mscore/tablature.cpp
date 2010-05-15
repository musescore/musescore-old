//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "tablature.h"

Tablature guitarTablature;

//---------------------------------------------------------
//   convertPitch
//---------------------------------------------------------

bool Tablature::convertPitch(int pitch, int* string, int* fret)
      {
      static int stringTable[6] = { 40, 45, 50, 55, 59, 64 };
      // static int frets = 13;
      static int frets = 4;
      static int strings = 6;

      for (int i = 0; i < strings; ++i) {
            int min = stringTable[i];
            int max = min + frets;

            if (pitch >= min && pitch <= max) {
                  *string = i;
                  *fret   = pitch - min;
                  return true;
                  }
            }
      return false;
      }



