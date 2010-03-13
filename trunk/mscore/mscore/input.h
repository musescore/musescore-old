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
class Drumset;
class Segment;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState {
      Duration _duration;      // currently duration

   public:
      bool rest;
      int pad;
      int pitch;
      NoteType noteType;
      BeamMode beamMode;
      int drumNote;
      Drumset* drumset;
      int track;
      Segment* _segment;
      bool noteEntryMode;
      Slur* slur;

      InputState();
      int voice() const                   { return track % VOICES;   }
      int tickLen() const                 { return _duration.ticks(); }
      ChordRest* cr() const;
      int tick() const;
      void setDuration(const Duration& d) { _duration = d; }
      Duration duration() const           { return _duration; }
      void setDots(int n)                 { _duration.setDots(n); }
      Segment* segment() const            { return _segment; }
      };

#endif

