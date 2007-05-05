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

#ifndef __KEYFINDER_H__
#define __KEYFINDER_H__

struct note_struct {
      int ontime;
      int offtime;
      int duration;
      int pitch;
      int tpc;
      };

struct Beat {
      int time;
      int level;
      };

struct SBeat {
      int time;
      };

struct MidiSegment {
      int start;
      int end;
      struct note_struct snote[100];
      int numnotes;           /* number of notes in the segment */
      double average_dur;     /* average input vector value (needed for K-S algorithm) */
      };

class MidiTrack;
class SigList;
extern int findKey(MidiTrack*, SigList*);
#endif

