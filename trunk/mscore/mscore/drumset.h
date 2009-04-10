//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __DRUMSET_H__
#define __DRUMSET_H__

#include "globals.h"

class Xml;

//---------------------------------------------------------
//   DrumInstrument
//---------------------------------------------------------

struct DrumInstrument {
      QString name;
      int notehead;           ///< notehead symbol set
      int line;               ///< place notehead onto this line
      int voice;
      Direction stemDirection;
      char shortcut;          ///< accelerator key (CDEFGAB)
      };

static const int DRUM_INSTRUMENTS = 128;

//---------------------------------------------------------
//   Drumset
//    defines note heads and line position for all
//    possible midi notes in a drumset
//---------------------------------------------------------

struct Drumset {
      DrumInstrument drum[DRUM_INSTRUMENTS];

      bool isValid(int pitch) const            { return drum[pitch].notehead != -1; }
      int noteHead(int pitch) const            { return drum[pitch].notehead;       }
      int line(int pitch) const                { return drum[pitch].line;           }
      int voice(int pitch) const               { return drum[pitch].voice;          }
      Direction stemDirection(int pitch) const { return drum[pitch].stemDirection;  }
      const QString& name(int pitch) const     { return drum[pitch].name;           }
      int shortcut(int pitch) const            { return drum[pitch].shortcut;       }
      void save(Xml&);
      void load(QDomElement);
      void clear();
      int nextPitch(int);
      int prevPitch(int);
      };

extern Drumset* smDrumset;
extern void initDrumset();

#endif

