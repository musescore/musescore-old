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

      virtual void setMasterTuning(double) {}
      virtual double masterTuning() const { return 440.0; }

      virtual bool loadSoundFont(const QString&) = 0;
      virtual QString soundFont() const = 0;

      virtual void process(unsigned, float*, float*, int) = 0;
      virtual void play(const Event&) = 0;

      virtual const QList<MidiPatch*>& getPatchInfo() const = 0;

      virtual double masterGain() const { return 1.0; }
      virtual void setMasterGain(double) {}

      virtual double effectParameter(int /*effect*/, int /*parameter*/)                { return 0.0; }
      virtual void setEffectParameter(int /*effect*/, int /*parameter*/, double /*value*/ ) { }
      };

#endif

