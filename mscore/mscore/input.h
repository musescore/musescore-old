//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: input.h,v 1.4 2006/03/02 17:08:34 wschweer Exp $
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

#ifndef __INPUT_H__
#define __INPUT_H__

class Slur;
class ChordRest;

//---------------------------------------------------------
//   InputState
//---------------------------------------------------------

struct InputState {
      int track;
      int pos;
      bool noteEntryMode;
      Slur* slur;
      ChordRest* cr;

      InputState() {
            track         = 0;
            pos           = 0;
            noteEntryMode = false;
            slur          = 0;
            cr            = 0;
            }
      };

#endif

