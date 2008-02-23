//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.h,v 1.8 2006/03/13 21:35:59 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "globals.h"

class Instrument;
class Xml;
class Staff;
class Score;
class Drumset;

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
      char midiChannel;
      char midiPort;
      char midiProgram;
      char midiBankSelectH;
      char midiBankSelectL;
      char volume;
      char pan;
      char chorus;
      char reverb;
      char minPitch;
      char maxPitch;
      char pitchOffset;

      bool mute;
      bool solo;
      Drumset* drumset;
      bool useDrumset;

      Instrument();
      void read(QDomElement);
      void write(Xml& xml) const;
      };

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part {
      Score* _score;
      QString _trackName;           ///< used in tracklist
      QTextDocument _longName;      ///< shown on first system
      QTextDocument _shortName;     ///< shown on followup systems
      Instrument _instrument;
      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score*);
      ~Part() {}
      void read(QDomElement);
      void write(Xml& xml) const;
      int nstaves() const;
      QList<Staff*>* staves()                  { return &_staves; }
      const QList<Staff*>* staves() const      { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)             { _id = s; }
      QString id() const                       { return _id; }
      const QTextDocument& shortName() const   { return _shortName; }
      const QTextDocument& longName()  const   { return _longName;  }
      QString trackName() const                { return _trackName;  }
      void setLongName(const QString& s);
      void setShortName(const QString& s);
      void setLongName(const QTextDocument& s);
      void setShortName(const QTextDocument& s);
      void setTrackName(const QString& s)      { _trackName = s; }
      void setStaves(int);
      void setMidiChannel(int val)             { _instrument.midiChannel = val;  }
      void setMidiProgram(int val)             { _instrument.midiProgram = val;  }
      void setMinPitch(int val)                { _instrument.minPitch = val;     }
      void setMaxPitch(int val)                { _instrument.maxPitch = val;     }

      void setDrumset(Drumset* ds)             { _instrument.drumset = ds;       }
      Drumset* drumset() const                 { return _instrument.drumset;     }
      bool useDrumset() const                  { return _instrument.useDrumset;  }
      void setUseDrumset(bool val);

      int midiChannel() const                  { return _instrument.midiChannel; }
      int midiPort() const                     { return _instrument.midiPort; }
      int midiProgram() const                  { return _instrument.midiProgram; }
      int minPitch() const                     { return _instrument.minPitch;    }
      int maxPitch() const                     { return _instrument.maxPitch;    }
      int volume() const                       { return _instrument.volume;      }
      int reverb() const                       { return _instrument.reverb;      }
      int chorus() const                       { return _instrument.chorus;      }
      int pan() const                          { return _instrument.chorus;      }
      void setPitchOffset(int val)             { _instrument.pitchOffset = val;  }
      int pitchOffset() const                  { return _instrument.pitchOffset; }
      void insertStaff(Staff*);
      void removeStaff(Staff*);
      const Instrument* instrument() const     { return &_instrument; }
      Instrument* instrument()                 { return &_instrument; }
      void setInstrument(const Instrument& i)  { _instrument = i;     }
      bool show() const                        { return _show;        }
      void setShow(bool val);
      Score* score() const                     { return _score; }
      };

#endif

