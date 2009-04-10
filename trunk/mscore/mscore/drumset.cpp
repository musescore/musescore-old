//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "drumset.h"
#include "xml.h"
#include "note.h"

Drumset* smDrumset;           // standard midi drumset

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void Drumset::save(Xml& xml)
      {
      for (int i = 0; i < 128; ++i) {
            if (!isValid(i))
                  continue;
            xml.stag(QString("Drum pitch=\"%1\"").arg(i));
            xml.tag("head", noteHead(i));
            xml.tag("line", line(i));
            xml.tag("voice", voice(i));
            xml.tag("name", name(i));
            xml.tag("stem", int(stemDirection(i)));
            if (shortcut(i)) {
                  switch (shortcut(i)) {
                        case 'C':
                        case 'D':
                        case 'E':
                        case 'F':
                        case 'G':
                        case 'A':
                        case 'B':
                              {
                              char a[2];
                              a[0] = shortcut(i);
                              a[1] = 0;
                              xml.tag("shortcut", a);
                              }
                              break;
                        default:
                              printf("illegal drum shortcut\n");
                              break;
                        }
                  }
            xml.etag();
            }
      }

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::load(QDomElement e)
      {
      int pitch = e.attribute("pitch", "-1").toInt();
      if (pitch < 0 || pitch > 127) {
            printf("load drumset: invalid pitch %d\n", pitch);
            return;
            }
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            bool isNum;
            int i = val.toInt(&isNum);

            if (tag == "head")
                  drum[pitch].notehead = i;
            else if (tag == "line")
                  drum[pitch].line = i;
            else if (tag == "voice")
                  drum[pitch].voice = i;
            else if (tag == "name")
                  drum[pitch].name = val;
            else if (tag == "stem")
                  drum[pitch].stemDirection = Direction(i);
            else if (tag == "shortcut")
                  drum[pitch].shortcut = isNum ? i : toupper(val[0].toAscii());
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Drumset::clear()
      {
      for (int i = 0; i < 128; ++i) {
            drum[i].name = "";
            drum[i].notehead = -1;
            drum[i].shortcut = 0;
            }
      }

//---------------------------------------------------------
//   nextPitch
//---------------------------------------------------------

int Drumset::nextPitch(int ii)
      {
      for (int i = ii + 1; i < 127; ++i) {
            if (isValid(i))
                  return i;
            }
      for (int i = 0; i <= ii; ++i) {
            if (isValid(i))
                  return i;
            }
      return 0;
      }

//---------------------------------------------------------
//   prevPitch
//---------------------------------------------------------

int Drumset::prevPitch(int ii)
      {
      for (int i = ii - 1; i >= 0; --i) {
            if (isValid(i))
                  return i;
            }
      for (int i = 127; i >= ii; --i) {
            if (isValid(i))
                  return i;
            }
      return 0;
      }

//---------------------------------------------------------
//   initDrumset
//    initialize standard midi drumset
//---------------------------------------------------------

void initDrumset()
      {
      smDrumset = new Drumset;
      for (int i = 0; i < 128; ++i) {
            smDrumset->drum[i].notehead = -1;   // invalid entry
            smDrumset->drum[i].line     = 0;
            smDrumset->drum[i].shortcut = 0;
            }
      smDrumset->drum[35].name     = "Acoustic Bass Drum";
      smDrumset->drum[35].notehead = HEAD_NORMAL;
      smDrumset->drum[35].line     = 7;
      smDrumset->drum[35].stemDirection = DOWN;
      smDrumset->drum[35].voice    = 1;

      smDrumset->drum[36].name     = "Bass Drum";
      smDrumset->drum[36].notehead = HEAD_NORMAL;
      smDrumset->drum[36].line     = 7;
      smDrumset->drum[36].stemDirection = DOWN;
      smDrumset->drum[36].voice    = 1;
      smDrumset->drum[36].shortcut = Qt::Key_C;

      smDrumset->drum[37].name     = "Side Stick";
      smDrumset->drum[37].notehead = HEAD_CROSS;   // Side Stick
      smDrumset->drum[37].line     = 3;
      smDrumset->drum[37].stemDirection = UP;
      smDrumset->drum[37].voice    = 0;

      smDrumset->drum[38].name     = "Snare (Acoustic)";
      smDrumset->drum[38].notehead = HEAD_NORMAL;   // Snare (Acoustic)
      smDrumset->drum[38].line     = 3;
      smDrumset->drum[38].stemDirection = UP;
      smDrumset->drum[38].voice    = 0;

      smDrumset->drum[40].name     = "Snare (Electric)";
      smDrumset->drum[40].notehead = HEAD_NORMAL;   // Snare (Electric)
      smDrumset->drum[40].line     = 3;
      smDrumset->drum[40].stemDirection = UP;
      smDrumset->drum[40].voice    = 0;

      smDrumset->drum[41].name     = "Tom 5";
      smDrumset->drum[41].notehead = HEAD_NORMAL;   // Tom 5
      smDrumset->drum[41].line     = 5;
      smDrumset->drum[41].stemDirection = UP;
      smDrumset->drum[41].voice    = 0;

      smDrumset->drum[42].name     = "Hi-Hat Closed";
      smDrumset->drum[42].notehead = HEAD_CROSS;   // Hi-Hat Closed
      smDrumset->drum[42].line     = -1;
      smDrumset->drum[42].stemDirection = UP;
      smDrumset->drum[42].voice    = 0;

      smDrumset->drum[43].name     = "Tom 4";
      smDrumset->drum[43].notehead = HEAD_NORMAL;   // Tom 4
      smDrumset->drum[43].line     = 5;
      smDrumset->drum[43].stemDirection = DOWN;
      smDrumset->drum[43].voice    = 1;

      smDrumset->drum[44].name     = "Hi-Hat Pedal";
      smDrumset->drum[44].notehead = HEAD_CROSS;   // Hi-Hat Pedal
      smDrumset->drum[44].line     = 9;
      smDrumset->drum[44].stemDirection = DOWN;
      smDrumset->drum[44].voice    = 1;

      smDrumset->drum[45].name     = "Tom 3";
      smDrumset->drum[45].notehead = HEAD_NORMAL;   // Tom 3
      smDrumset->drum[45].line     = 2;
      smDrumset->drum[45].stemDirection = UP;
      smDrumset->drum[45].voice    = 0;

      smDrumset->drum[46].name     = "Hi-Hat Open";
      smDrumset->drum[46].notehead = HEAD_CROSS;   // Hi-Hat Open
      smDrumset->drum[46].line     = 1;
      smDrumset->drum[46].stemDirection = UP;
      smDrumset->drum[46].voice    = 0;

      smDrumset->drum[47].name     = "Tom 2";
      smDrumset->drum[47].notehead = HEAD_NORMAL;   // Tom 2
      smDrumset->drum[47].line     = 1;
      smDrumset->drum[47].stemDirection = UP;
      smDrumset->drum[47].voice    = 0;

      smDrumset->drum[48].name     = "Tom 1";
      smDrumset->drum[48].notehead = HEAD_NORMAL;   // Tom 1
      smDrumset->drum[48].line     = 0;
      smDrumset->drum[48].stemDirection = UP;
      smDrumset->drum[48].voice    = 0;

      smDrumset->drum[49].name     = "Crash 1";
      smDrumset->drum[49].notehead = HEAD_CROSS;   // Crash 1
      smDrumset->drum[49].line     = -2;
      smDrumset->drum[49].stemDirection = UP;
      smDrumset->drum[49].voice    = 0;

      smDrumset->drum[50].name     = "Tom";
      smDrumset->drum[50].notehead = HEAD_NORMAL;   // Tom
      smDrumset->drum[50].line     = 0;
      smDrumset->drum[50].stemDirection = UP;
      smDrumset->drum[50].voice    = 0;

      smDrumset->drum[51].name     = "Ride";
      smDrumset->drum[51].notehead = HEAD_CROSS;   // Ride
      smDrumset->drum[51].line     = 0;
      smDrumset->drum[51].stemDirection = UP;
      smDrumset->drum[51].voice    = 0;
      smDrumset->drum[51].shortcut = Qt::Key_D;

      smDrumset->drum[52].name     = "China";
      smDrumset->drum[52].notehead = HEAD_CROSS;   // China
      smDrumset->drum[52].line     = -3;
      smDrumset->drum[52].stemDirection = UP;
      smDrumset->drum[52].voice    = 0;

      smDrumset->drum[53].name     = "Ride (Bell)";
      smDrumset->drum[53].notehead = HEAD_DIAMOND;   // Ride (Bell)
      smDrumset->drum[53].line     = 0;
      smDrumset->drum[53].stemDirection = UP;
      smDrumset->drum[53].voice    = 0;

      smDrumset->drum[54].name     = "Tambourine";
      smDrumset->drum[54].notehead = HEAD_DIAMOND;   // Tambourine
      smDrumset->drum[54].line     = 2;
      smDrumset->drum[54].stemDirection = UP;
      smDrumset->drum[54].voice    = 0;

      smDrumset->drum[55].name     = "Ride (Bell)";
      smDrumset->drum[55].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[55].line     = -3;
      smDrumset->drum[55].stemDirection = UP;
      smDrumset->drum[55].voice    = 0;

      smDrumset->drum[56].name     = "Ride (Bell)";
      smDrumset->drum[56].notehead = HEAD_TRIANGLE;   // Ride (Bell)
      smDrumset->drum[56].line     = 1;
      smDrumset->drum[56].stemDirection = UP;
      smDrumset->drum[56].voice    = 0;

      smDrumset->drum[57].name     = "Ride (Bell)";
      smDrumset->drum[57].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[57].line     = -3;
      smDrumset->drum[57].stemDirection = UP;
      smDrumset->drum[57].voice    = 0;

      smDrumset->drum[59].name     = "Ride (Bell)";
      smDrumset->drum[59].notehead = HEAD_CROSS;   // Ride (Bell)
      smDrumset->drum[59].line     = 2;
      smDrumset->drum[59].stemDirection = UP;
      smDrumset->drum[59].voice    = 0;

      smDrumset->drum[63].name     = "open high conga";
      smDrumset->drum[63].notehead = HEAD_CROSS;   // open high conga
      smDrumset->drum[63].line     = 4;
      smDrumset->drum[63].stemDirection = UP;
      smDrumset->drum[63].voice    = 0;

      smDrumset->drum[64].name     = "low conga";
      smDrumset->drum[64].notehead = HEAD_CROSS;   // low conga
      smDrumset->drum[64].line     = 6;
      smDrumset->drum[64].stemDirection = UP;
      smDrumset->drum[64].voice    = 0;
      }


