//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer and others (ws@seh.de)
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

#ifndef __PITCHSPELLING_H__
#define __PITCHSPELLING_H__

class MidiNote;

//---------------------------------------------------------
//   tpc2line
//---------------------------------------------------------

inline static int tpc2line(int tpc)
      {
      static const int lines[7] = { 3, 0, 4, 1, 5, 2, 6 };
      return lines[(tpc+1) % 7];
      }

void spell(QList<MidiNote*>& notes, int);

#endif


