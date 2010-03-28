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
#include "interval.h"

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
      bool operator==(const NamedEventList& i) const { return i.name == name && i.events == events; }
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

      MidiArticulation() {}
      bool operator==(const MidiArticulation& i) const;
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

      QList<MidiArticulation> articulation;

      Channel();
      void write(Xml&) const;
      void read(QDomElement);
      void updateInitList() const;
      bool operator==(const Channel& c) { return (name == c.name) && (channel == c.channel); }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
      QString _trackName;           ///< used in tracklist
      QString _longName;
      QString _shortName;
      char _minPitchA, _maxPitchA, _minPitchP, _maxPitchP;
      Interval _transpose;

      bool _useDrumset;
      Drumset* _drumset;

      QList<NamedEventList> _midiActions;
      QList<MidiArticulation> _articulation;
      QList<Channel> _channel;      // at least one entry

   public:
      Instrument();
      void read(QDomElement);
      void write(Xml& xml) const;
      NamedEventList* midiAction(const QString& s) const;
      int channelIdx(const QString& s) const;
      void updateVelocity(int* velocity, int channel, const QString& name);

      bool operator==(const Instrument&) const;

      int minPitchP() const                    { return _minPitchP;  }
      int maxPitchP() const                    { return _maxPitchP;  }
      int minPitchA() const                    { return _minPitchA;  }
      int maxPitchA() const                    { return _maxPitchA;  }
      void setMinPitchP(int v)                 { _minPitchP = v;     }
      void setMaxPitchP(int v)                 { _maxPitchP = v;     }
      void setMinPitchA(int v)                 { _minPitchA = v;     }
      void setMaxPitchA(int v)                 { _maxPitchA = v;     }
      Interval transpose() const               { return _transpose; }
      void setTranspose(const Interval& v)     { _transpose = v; }

      QString trackName() const                { return _trackName;  }
      void setTrackName(const QString& s)      { _trackName = s; }
      QString longName() const                 { return _longName; }
      QString shortName() const                { return _shortName; }
      void setLongName(const QString& s)       { _longName = s; }
      void setShortName(const QString& s)      { _shortName = s; }

      void setDrumset(Drumset* ds);
      Drumset* drumset() const                 { return _drumset;    }
      bool useDrumset() const                  { return _useDrumset; }
      void setUseDrumset(bool val);
      void setAmateurPitchRange(int a, int b)       { _minPitchA = a; _maxPitchA = b; }
      void setProfessionalPitchRange(int a, int b)  { _minPitchP = a; _maxPitchP = b; }
      Channel& channel(int idx)                     { return _channel[idx]; }
      const Channel& channel(int idx) const         { return _channel[idx]; }

      const QList<NamedEventList>& midiActions() const       { return _midiActions; }
      const QList<MidiArticulation>& articulation() const    { return _articulation; }
      const QList<Channel>& channel() const                  { return _channel; }

      void setMidiActions(const QList<NamedEventList>& l)    { _midiActions = l;  }
      void setArticulation(const QList<MidiArticulation>& l) { _articulation = l; }
      void setChannel(const QList<Channel>& l)               { _channel = l;      }
      };

#endif

