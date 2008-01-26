//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
//
//  This file is in part based on code from KGuitar, a KDE tabulature editor
//     * copyright (C) 2002-2003 the KGuitar development team
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

#include "chordinfo.h"

//---------------------------------------------------------
//   ChordInfo
//---------------------------------------------------------

ChordInfo::ChordInfo(int t, int b, int s3, int s5, int s7, int s9, int s11, int s13)
      {
      static int toneshift[6] = { 3, 7, 10, 2, 5, 9 };
      QString flat("+");
      QString sharp("-");
      QString maj7("maj7");

      _tonic = t;
      _bass  = b;
      s[0]   = s3;
      s[1]   = s5;
      s[2]   = s7;
      s[3]   = s9;
      s[4]   = s11;
      s[5]   = s13;
      for (int i = 0; i < 6; i++) {
            if (s[i] == -1)
                  s[i] = 0;
            else
                  s[i] -= toneshift[i] + 2;
            }

      _name = ""; // Settings::noteName(_tonic);
      // Special cases
      if ((s3 == -1) && (s5 == 7) && (s7 == -1) && (s9 == -1) && (s11 == -1) && (s13 == -1)) {
            _name += "5";
            }
      else if ((s3 == 4) && (s5 == 8) && (s7 == -1) && (s9 == -1) && (s11 == -1) && (s13 == -1)) {
            _name += "aug";
            }
      else {
            if ((s3 == 3) && (s5 == 6) && (s7 == 9)) {
                  _name += "dim";
                  }
            else {
                  if (s3 == 3)
                        _name += "m";
                  if (s5 == 6)
                        _name += "/5" + flat;
                  if (s5 == 8)
                        _name += "/5" + sharp;
                  if (((s5 == 6) || (s5 == 8)) && ((s7 != -1) || (s9 != -1) || (s11 != -1) || (s13 != -1)))
                        _name += "/";
                  if ((s7 == 10) && (s9 == -1))
                        _name += "7";
                  if (s7 == 11)
                        _name += maj7;
                  if (s7 == 9)
                        _name += "6";
                  if (((s7 == 11) || (s7 == 9)) && ((s9 != -1) || (s11 != -1) || (s13 != -1)))
                        _name += "/";
                  }

            if ((s7 == -1) && (s9 != -1))
                  _name += "add";
            if ((s9 == 2) && (s11 == -1))
                  _name += "9";
            if (s9 == 1)
                  _name += "9" + flat;
            if (s9 == 3)
                  _name += "9" + sharp;
            if (((s9 == 1) || (s9 == 3)) && ((s11 != -1) || (s13 != -1)))
                  _name += "/";

            if ((s9 == -1) && (s11 != -1))
                  _name += "add";
            if ((s11 == 5) && (s13 == -1))
                  _name += "11";
            if (s11 == 4)
                  _name += "11" + flat;
            if (s11 == 6)
                  _name += "11" + sharp;
            if (((s11 == 4) || (s11 == 6)) && (s13 != -1))
                  _name += "/";

            if ((s11 == -1) && (s13 != -1))
                  _name += "add";
            if (s13 == 9)
                  _name += "13";
            if (s13 == 8)
                  _name += "13" + flat;
            if (s13 == 10)
                  _name += "13" + sharp;

            if (s3 == 2)
                  _name += "sus2";
            if (s3 == 5)
                  _name += "sus4";

            if ((s3 == -1) && (s5 == -1)) {
                  _name += " (no3no5)";
                  }
            else {
                  if (s3 == -1)
                        _name += " (no3)";
                  if (s5 == -1)
                        _name += " (no5)";
                  }
            }
      }

