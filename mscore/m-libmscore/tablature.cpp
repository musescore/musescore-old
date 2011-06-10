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
#include "m-al/xml.h"

static int guitarStrings[6] = { 40, 45, 50, 55, 59, 64 };

Tablature guitarTablature(13, 6, guitarStrings);

//---------------------------------------------------------
//   Tablature
//---------------------------------------------------------

Tablature::Tablature(int numFrets, int numStrings, int strings[])
      {
      _frets = numFrets;
      for (int i = 0; i < numStrings; ++i)
            stringTable.append(strings[i]);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tablature::read(XmlReader* r)
      {
      while (r->readElement()) {
            int v;
            if (r->readInt("frets", &_frets))
                  ;
            else if (r->readInt("string", &v))
                  stringTable.append(v);
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   convertPitch
//---------------------------------------------------------

bool Tablature::convertPitch(int pitch, int* string, int* fret) const
      {
      int strings = stringTable.size();

      for (int i = 0; i < strings; ++i) {
            int min = stringTable[i];
            int max;
            if (i + 1 == strings)
                  max = min + _frets;
            else
                  max = stringTable[i+1] - 1;

            if (pitch >= min && pitch <= max) {
                  *string = strings - i - 1;
                  *fret   = pitch - min;
                  return true;
                  }
            }
      *string = 0;
      *fret   = 0;
      return false;
      }

//---------------------------------------------------------
//   getPitch
//---------------------------------------------------------

int Tablature::getPitch(int string, int fret) const
      {
      int strings = stringTable.size();
      return stringTable[strings - string - 1] + fret;
      }

//---------------------------------------------------------
//   fret
//    return fret for given pitch and string
//    return -1 if not possible
//---------------------------------------------------------

int Tablature::fret(int pitch, int string) const
      {
      int strings = stringTable.size();

      if (string < 0 || string >= strings)
            return -1;
      int fret = pitch - stringTable[strings - string - 1];
      if (fret < 0 || fret >= _frets)
            return -1;
      return fret;
      }

