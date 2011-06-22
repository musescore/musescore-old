//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __INPUT_H__
#define __INPUT_H__

#include "libmscore/mscore.h"
#include "libmscore/durationtype.h"

class Slur;
class ChordRest;
class Drumset;
class Segment;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

class InputState {
      Duration _duration;      // currently duration
      int _drumNote;
      Drumset* _drumset;
      int _track;
      Segment* _segment;
      bool _repitchMode;

   public:
      bool rest;
      int pad;
      int pitch;
      NoteType noteType;
      BeamMode beamMode;
      bool noteEntryMode;
      Slur* slur;

      InputState();
      int ticks() const                   { return _duration.ticks(); }
      ChordRest* cr() const;

      int tick() const;
      void setDuration(const Duration& d) { _duration = d;          }
      Duration duration() const           { return _duration;       }
      void setDots(int n)                 { _duration.setDots(n);   }

      Segment* segment() const            { return _segment;        }
      void setSegment(Segment* s)         { _segment = s;           }

      Drumset* drumset() const;

      int drumNote() const                { return _drumNote;       }
      void setDrumNote(int v)             { _drumNote = v;          }

      int voice() const                   { return _track % VOICES; }
      int track() const                   { return _track;          }
      void setTrack(int v)                { _track = v;             }

      bool repitchMode() const            { return _repitchMode;    }
      void setRepitchMode(bool val)       { _repitchMode = val;     }
      };

#endif

