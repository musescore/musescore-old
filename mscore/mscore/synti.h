//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synti.h,v 1.12 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SYNTI_H__
#define __SYNTI_H__

enum {
      CTRL_ALL_NOTES_OFF = 0x7b,
      CTRL_PROGRAM       = 0x10000,
      CTRL_PITCH         = 0x10001,
      CTRL_VOLUME        = 7,
      CTRL_PAN           = 0x0a,
      CTRL_SUSTAIN       = 0x40,
      CTRL_CHORUS_SEND   = 0x5d,
      CTRL_REVERB_SEND   = 0x5b
      };

//---------------------------------------------------------
//   MidiPatch
//---------------------------------------------------------

struct MidiPatch {
      signed char typ;                     // 1 - GM  2 - GS  4 - XG
      signed char hbank, lbank, prog;
      const char* name;
      };

//---------------------------------------------------------
//   Synth
//---------------------------------------------------------

class Synth {

   public:
      Synth() {}
      virtual ~Synth() {}
      virtual bool init(int sampleRate) = 0;
      virtual bool loadSoundFont(const QString&) = 0;
      virtual void process(unsigned, float*, float*, int) = 0;
      virtual void playNote(int channel, int pitch, int velo) = 0;
      virtual bool setController(int ch, int ctrl, int val) = 0;
      virtual const MidiPatch* getPatchInfo(int ch, const MidiPatch* p) const = 0;
      };

#endif

