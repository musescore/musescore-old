//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: padstate.h,v 1.3 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __PADSTATE_H__
#define __PADSTATE_H__

#include "globals.h"

struct Drumset;

//---------------------------------------------------------
//   PadState
//    keyboard & pad note entry state
//---------------------------------------------------------

struct PadState {
      int dots;
      int len;
      int tickLen;  // len + len * (dot ? .5 : 0)
      bool rest;
      int pad;
      int voice;
      int pitch;
      int prefix;
      NoteType noteType;
      BeamMode beamMode;
      int drumNote;
      Drumset* drumset;

      PadState() {
            dots     = 0;
            len      = division;
            tickLen  = division;
            rest     = false;
            pad      = 0;
            voice    = 0;
            pitch    = 60;
            prefix   = 0;
            noteType = NOTE_NORMAL;
            beamMode = BEAM_AUTO;
            drumNote = -1;
            drumset  = 0;
            }
      };

#endif

