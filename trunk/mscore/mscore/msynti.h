//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: synti.h 2099 2009-09-15 17:09:49Z wschweer $
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

#ifndef __MSYNTI_H__
#define __MSYNTI_H__

#include "synti.h"

//---------------------------------------------------------
//   MSynth
//---------------------------------------------------------

class MSynth {
      QList<Synti*> syntis;

   public:
      MSynth() {}
      virtual ~MSynth() {}
      void init(int sampleRate);

      void setMasterTuning(int, double);
      double masterTuning(int) const;

      bool loadSoundFont(int, const QString&);
      QString soundFont(int) const;

      void process(unsigned, float*, float*, int);
      void play(const Event&);

      const QList<MidiPatch*>& getPatchInfo() const;

      double masterGain(int) const;
      void setMasterGain(int, double);

      double effectParameter(int, int /*effect*/, int /*parameter*/);
      void setEffectParameter(int, int /*effect*/, int /*parameter*/, double /*value*/ );
      };

#endif

