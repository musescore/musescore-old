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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "globals.h"
#include "event.h"

class Instrument;
class Xml;
class Staff;
class Score;
class Drumset;

//---------------------------------------------------------
//   NamedEventList
//---------------------------------------------------------

struct NamedEventList {
      QString name;
      EventList events;

      void write(Xml&, const QString& name) const;
      void read(QDomElement);
      };

//---------------------------------------------------------
//   MidiArticulation
//---------------------------------------------------------

struct MidiArticulation {
      QString name;
      int velocity;           // velocity change: -100% - +100%
      int gateTime;           // gate time change: -100% - +100%
      void write(Xml&) const;
      void read(QDomElement);
      };

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

// this are the indexes of controllers which are always present in
// Channel init EventList (maybe zero)

enum {
      A_HBANK, A_LBANK, A_PROGRAM, A_VOLUME, A_PAN, A_CHORUS, A_REVERB,
      A_INIT_COUNT
      };

struct Channel {
      QString name;
      int channel;      // mscore channel number, mapped to midi port/channel
      mutable EventList init;

      int program;     // current values as shown in mixer
      int bank;        // initialized from "init"
      char volume;
      char pan;
      char chorus;
      char reverb;

      bool mute;
      bool solo;
      bool soloMute;

      QList<MidiArticulation*> articulation;

      Channel();
      void write(Xml&) const;
      void read(QDomElement);
      void updateInitList() const;
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
      char minPitchA, maxPitchA, minPitchP, maxPitchP;
      char transposeDiatonic;
      char transposeChromatic;

      bool useDrumset;
      Drumset* drumset;

      QList<NamedEventList> midiActions;
      QList<Channel*> channel;      // at least one entry
      QList<MidiArticulation*> articulation;

      Instrument();
      void read(QDomElement);
      void write(Xml& xml) const;
      NamedEventList* midiAction(const QString& s) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);
      };

#endif

