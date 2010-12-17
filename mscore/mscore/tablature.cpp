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

Tablature::Tablature(int numFrets, QList<int>& strings)
      {
      _frets = numFrets;
      // DEEP COPY!
      foreach(int i, strings)
            stringTable.append(i);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Tablature::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            int v = e.text().toInt();
            if (tag == "frets")
                  _frets = v;
            else if (tag == "string")
                  stringTable.append(v);
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Tablature::write(Xml& xml) const
      {
      xml.stag("Tablature");
      xml.tag("frets", _frets);
      foreach(int pitch, stringTable)
            xml.tag("string", pitch);
      xml.etag();
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


