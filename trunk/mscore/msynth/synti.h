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

#ifndef __SYNTI_H__
#define __SYNTI_H__

struct MidiPatch;
class Event;
class Synth;

#include "sparm.h"

//---------------------------------------------------------
//   MidiPatch
//---------------------------------------------------------

struct MidiPatch {
      bool drum;
      int synti;
      int bank, prog;
      QString name;
      };

//---------------------------------------------------------
//   Synth
//---------------------------------------------------------

class Synth {

   protected:

   public:
      Synth() {}
      virtual ~Synth() {}
      virtual void init(int sampleRate) = 0;

      virtual const char* name() const = 0;

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFonts(const QStringList&) = 0;
      virtual bool addSoundFont(const QString&)    { return false; }
      virtual bool removeSoundFont(const QString&) { return false; }

      virtual QStringList soundFonts() const = 0;

      virtual void process(unsigned, float*, float*, int, float) = 0;
      virtual void play(const Event&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      // set/get a single parameter
      virtual SyntiParameter parameter(int /*id*/) const { return SyntiParameter(); }
      virtual void setParameter(int /*id*/, double /*val*/) {}
      virtual void setParameter(int /*id*/, const QString&) {}

      // get/set synthesizer state
      virtual SyntiState state() const = 0;
      virtual void setState(SyntiState&) {}
      };

//---------------------------------------------------------
//   MasterSynth
//---------------------------------------------------------

class MasterSynth {
      QList<Synth*> syntis;
      float _gain;

   public:
      MasterSynth();
      ~MasterSynth();
      void init(int sampleRate);

      void process(unsigned, float*, float*, int);
      void play(const Event&, int);

      double gain() const     { return _gain; }
      void setGain(float val) { _gain = val;  }

      void setMasterTuning(double) {}
      double masterTuning() const { return 440.0; }

      int synthNameToIndex(const QString&) const;
      QString synthIndexToName(int) const;

      QList<MidiPatch*> getPatchInfo() const;

      // set/get a single parameter
      SyntiParameter parameter(int id) const;
      void setParameter(int id, double val);

      // get/set synthesizer state
      SyntiState state() const;
      void setState(SyntiState&);

      Synth* synth(const QString& name);
      };

#endif

