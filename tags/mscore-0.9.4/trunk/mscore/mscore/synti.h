//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synti.h,v 1.12 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __SYNTI_H__
#define __SYNTI_H__

struct MidiPatch;
struct MidiOutEvent;

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
      virtual void play(const MidiOutEvent&) = 0;
      virtual const MidiPatch* getPatchInfo(bool onlyDrums, const MidiPatch* p) const = 0;
      };

#endif

