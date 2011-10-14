//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synti.h 3627 2010-10-26 08:32:55Z wschweer $
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

#ifndef __SYNTI_H__
#define __SYNTI_H__


struct MidiPatch;
class SeqEvent;
class Synth;

#include "sparm.h"

//---------------------------------------------------------
//   Synti
//---------------------------------------------------------

class Synti {

   protected:

   public:
      Synti() {}
      virtual ~Synti() {}
      virtual void init(int sampleRate) = 0;

      virtual void setMasterTuning(float) {}
      virtual float masterTuning() const { return 440.0; }

      virtual void process(unsigned, short*) = 0;
      virtual void play(const SeqEvent&) = 0;

      // set/get a single parameter
      virtual SyntiParameter parameter(int /*id*/) const { return SyntiParameter(); }
      virtual void setParameter(int /*id*/, float /*val*/) {}
      virtual void setParameter(int /*id*/, const QString&) {}

      // get/set synthesizer state
      virtual SyntiState state() const = 0;
      virtual void setState(const SyntiState&) {}

      virtual float gain() const = 0;
      virtual void setGain(float) = 0;
      virtual void allNotesOff() = 0;
      };

#endif

