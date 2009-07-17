//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __INPUT_H__
#define __INPUT_H__

#include "globals.h"
#include "durationtype.h"

class Slur;
class ChordRest;
struct Drumset;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState {
   public:
      int dots;
//      int len;
      Duration duration;      // currently selected duration
      bool rest;
      int pad;
      int pitch;
      int prefix;
      NoteType noteType;
      BeamMode beamMode;
      int drumNote;
      Drumset* drumset;
      int track;
      bool noteEntryMode;
      Slur* slur;
      ChordRest* cr;

      InputState();
      int pos() const;
      void setPos(ChordRest* cr);

      int voice() const { return track % VOICES; }
      int tickLen() { return duration.ticks(dots); }
      };

#endif

