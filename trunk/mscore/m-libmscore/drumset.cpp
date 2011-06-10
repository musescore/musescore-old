//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: drumset.cpp 2992 2010-04-22 14:42:39Z lasconic $
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

#include <ctype.h>
#include "drumset.h"
#include "m-al/xml.h"
#include "note.h"

Drumset* smDrumset;           // standard midi drumset

//---------------------------------------------------------
//   load
//---------------------------------------------------------

void Drumset::load(XmlReader* r)
      {
      int pitch = -1;
      while (r->readAttribute()) {
            if (r->tag() == "pitch")
                  pitch = r->intValue();
            }
      if (pitch < 0 || pitch > 127) {
            printf("load drumset: invalid pitch %d\n", pitch);
            pitch = 0;
            }
      while (r->readElement()) {
            int i;
            QString val;

            if (r->readInt("head", &i))
                  _drum[pitch].notehead = i;
            else if (r->readInt("line", &i))
                  _drum[pitch].line = i;
            else if (r->readInt("voice", &i))
                  _drum[pitch].voice = i;
            else if (r->readString("name", &_drum[pitch].name))
                  ;
            else if (r->readInt("stem", &i))
                  _drum[pitch].stemDirection = Direction(i);
            else if (r->readString("shortcut", &val)) {
                  bool isNum;
                  i = val.toInt(&isNum);
                  _drum[pitch].shortcut = isNum ? i : toupper(val[0].toAscii());
                  }
            else
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Drumset::clear()
      {
      for (int i = 0; i < 128; ++i) {
            _drum[i].name = "";
            _drum[i].notehead = -1;
            _drum[i].shortcut = 0;
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
            smDrumset->drum(i).notehead = -1;   // invalid entry
            smDrumset->drum(i).line     = 0;
            smDrumset->drum(i).shortcut = 0;
            }
      smDrumset->drum(35) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Acoustic Bass Drum"), HEAD_NORMAL,   7, DOWN, 1);
      smDrumset->drum(36) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Bass Drum"),          HEAD_NORMAL,   7, DOWN, 1, Qt::Key_C);
      smDrumset->drum(37) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Side Stick"),         HEAD_CROSS,    3, UP);
      smDrumset->drum(38) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Snare (Acoustic)"),   HEAD_NORMAL,   3, UP);
      smDrumset->drum(40) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Snare (Electric)"),   HEAD_NORMAL,   3, UP);
      smDrumset->drum(41) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 5"),              HEAD_NORMAL,   5, UP);
      smDrumset->drum(42) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Closed"),      HEAD_CROSS,   -1, UP);
      smDrumset->drum(43) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 4"),              HEAD_NORMAL,   5, DOWN, 1);
      smDrumset->drum(44) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Pedal"),       HEAD_CROSS,    9, DOWN, 1);
      smDrumset->drum(45) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 3"),              HEAD_NORMAL,   2, UP);
      smDrumset->drum(46) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Hi-Hat Open"),        HEAD_CROSS,    1, UP);
      smDrumset->drum(47) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 2"),              HEAD_NORMAL,   1, UP);
      smDrumset->drum(48) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom 1"),              HEAD_NORMAL,   0, UP);
      smDrumset->drum(49) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Crash 1"),            HEAD_CROSS,   -2, UP);
      smDrumset->drum(50) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tom"),                HEAD_NORMAL,   0, UP);
      smDrumset->drum(51) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride"),               HEAD_CROSS,    0, UP, 0, Qt::Key_D);
      smDrumset->drum(52) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "China"),              HEAD_CROSS,   -3, UP);
      smDrumset->drum(53) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_DIAMOND,  0, UP);
      smDrumset->drum(54) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Tambourine"),         HEAD_DIAMOND,  2, UP);
      smDrumset->drum(55) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,   -3, UP);
      smDrumset->drum(56) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_TRIANGLE, 1, UP);
      smDrumset->drum(57) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,   -3, UP);
      smDrumset->drum(59) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "Ride (Bell)"),        HEAD_CROSS,    2, UP);
      smDrumset->drum(63) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "open high conga"),    HEAD_CROSS,    4, UP);
      smDrumset->drum(64) = DrumInstrument(QT_TRANSLATE_NOOP("drumset", "low conga"),          HEAD_CROSS,    6, UP);
      }

