//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.h,v 1.8 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __PART_H__
#define __PART_H__

#include "plist.h"

class Instrument;
class Xml;
class StaffList;
class Staff;
class Score;

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
      int  midiChannel;
      int  midiProgram;
      int  volume;
      int  pan;
      int  chorus;
      int  reverb;
      bool mute;
      bool solo;
      bool show;
      int  minPitch;
      int  maxPitch;

      Instrument();
      void read(QDomNode);
      void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part {
      Score* cs;
      QString _trackName;           // used in tracklist
      QString _longName;            // shown on first system
      QString _shortName;           // shown on followup systems
      Instrument _instrument;
      StaffList* _staves;
      QString _id;                  // used for MusicXml import

   public:
      Part(Score*);
      ~Part();
      void read(Score*, QDomNode);
      void write(Xml& xml) const;
      int nstaves() const;
      StaffList* staves() const           { return _staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)        { _id = s; }
      QString id() const                  { return _id; }
      QString shortName() const           { return _shortName; }
      QString longName() const            { return _longName;  }
      QString trackName() const           { return _trackName; }
      void setLongName(const QString& s)  { _longName = s;  }
      void setShortName(const QString& s) { _shortName = s; }
      void setTrackName(const QString& s) { _trackName = s; }
      void setStaves(int);
      void setMidiChannel(int val)        { _instrument.midiChannel = val;  }
      void setMidiProgram(int val)        { _instrument.midiProgram = val;  }
      void setMinPitch(int val)           { _instrument.minPitch = val;     }
      void setMaxPitch(int val)           { _instrument.maxPitch = val;     }
      int midiChannel() const             { return _instrument.midiChannel; }
      int midiProgram() const             { return _instrument.midiProgram; }
      int minPitch() const                { return _instrument.minPitch;    }
      int maxPitch() const                { return _instrument.maxPitch;    }
      int volume() const                  { return _instrument.volume;      }
      int reverb() const                  { return _instrument.reverb;      }
      int chorus() const                  { return _instrument.chorus;      }
      void insertStaff(Staff*);
      void removeStaff(Staff*);
      const Instrument* instrument() const { return &_instrument; }
      Instrument* instrument() { return &_instrument; }
      };

class PartList : public pstl::plist<Part*> {
      };
typedef PartList::iterator iPart;
typedef PartList::reverse_iterator riPart;
typedef PartList::const_iterator ciPart;
#endif

