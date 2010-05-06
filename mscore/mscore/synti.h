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

   public:
      Synth() {}
      virtual ~Synth() {}
      virtual void init(int sampleRate) = 0;

      virtual const char* name() const = 0;

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFont(const QString&) = 0;
      virtual QString soundFont() const = 0;

      virtual void process(unsigned, float*, float*, int, float) = 0;
      virtual void play(const Event&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      virtual double effectParameter(int /*effect*/, int /*param*/)  { return 0.0; }
      virtual void setEffectParameter(int /*effect*/, int /*param*/, double /*val*/ ) { }
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

      bool loadSoundFont(const QString&);
      QString soundFont() const;

      int synthNameToIndex(const QString&) const;
      QString synthIndexToName(int) const;

      QList<MidiPatch*> getPatchInfo() const;
      Synth* getSynth(int n);
      const QList<Synth*>& getSyntis() const;
      };

#endif

