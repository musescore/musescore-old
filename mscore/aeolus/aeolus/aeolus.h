//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __AEOLUS_H__
#define __AEOLUS_H__

struct MidiPatch;
class Event;

#include "stdint.h"
#include "synti.h"

class Audio;
class Model;
class Slave;
class Lfq_u8;
class Lfq_u32;

//---------------------------------------------------------
//   Synth
//---------------------------------------------------------

class Aeolus : public Synth {
      Lfq_u8* midi_queue;
      Lfq_u32*  note_queue;

      Audio* audio;
      Model* model;
      Slave* slave;
      QList<MidiPatch*> patchList;
      uint16_t _midimap [16];

   public:
      Aeolus();
      virtual ~Aeolus();
      virtual void init(int sampleRate);

      virtual void setMasterTuning(double);
      virtual double masterTuning() const;

      virtual bool loadSoundFont(const QString&);
      virtual QString soundFont() const;

      virtual void process(unsigned, float*, float*, int);
      virtual void play(const Event&);

      virtual const QList<MidiPatch*>& getPatchInfo() const;

      virtual double masterGain() const;
      virtual void setMasterGain(double);

      virtual double effectParameter(int /*effect*/, int /*parameter*/);
      virtual void setEffectParameter(int /*effect*/, int /*parameter*/, double /*value*/ );
      };

#endif


