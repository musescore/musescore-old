//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: seq.h,v 1.20 2006/03/02 17:08:43 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __EVENT_H__
#define __EVENT_H__

class Note;

//---------------------------------------------------------
//   Midi Events
//---------------------------------------------------------

enum {
      ME_NOTEOFF    = 0x80,
      ME_NOTEON     = 0x90,
      ME_POLYAFTER  = 0xa0,
      ME_CONTROLLER = 0xb0,
      ME_PROGRAM    = 0xc0,
      ME_AFTERTOUCH = 0xd0,
      ME_PITCHBEND  = 0xe0,
      ME_SYSEX      = 0xf0,
      ME_META       = 0xff,
      ME_SONGPOS    = 0xf2,
      ME_ENDSYSEX   = 0xf7,
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,

      ME_NOTE       = 0x100,
      ME_CHORD      = 0x101,
      };

//---------------------------------------------------------
//   Midi Meta Events
//---------------------------------------------------------

enum {
      META_SEQUENCE_NUMBER = 0,
      META_TEXT            = 1,
      META_COPYRIGHT       = 2,
      META_TRACK_NAME      = 3,
      META_INSTRUMENT_NAME = 4,
      META_LYRIC           = 5,
      META_MARKER          = 6,
      META_CUE_POINT       = 7,
      META_TITLE           = 8,     // mscore extension
      META_SUBTITLE        = 9,     // mscore extension
      META_COMPOSER        = 0xa,   // mscore extension
      META_TRANSLATOR      = 0xb,   // mscore extension
      META_POET            = 0xc,   // mscore extension
      META_TRACK_COMMENT   = 0xf,
      META_PORT_CHANGE     = 0x21,
      META_CHANNEL_PREFIX  = 0x22,
      META_TEMPO           = 0x51,
      META_TIME_SIGNATURE  = 0x58,
      META_KEY_SIGNATURE   = 0x59,
      };

//---------------------------------------------------------
//   Event
//    sequencer event type (note on, note off)
//---------------------------------------------------------

struct Event {
      int type;
      int channel;
      int val1;         // pitch
      int val2;         // velocity
      Note* note;       // used to mark the currently played notes
      };

#endif

