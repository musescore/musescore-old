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
      QString _trackName;           ///< used in tracklist
      QTextDocument _longName;      ///< shown on first system
      QTextDocument _shortName;     ///< shown on followup systems
      Instrument _instrument;
      StaffList* _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score*);
      ~Part();
      void read(Score*, QDomNode);
      void write(Xml& xml) const;
      int nstaves() const;
      StaffList* staves() const                { return _staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)             { _id = s; }
      QString id() const                       { return _id; }
      const QTextDocument& shortName() const   { return _shortName; }
      const QTextDocument& longName() const    { return _longName;  }
      QString trackName() const                { return _trackName; }
      void setLongName(const QString& s);
      void setShortName(const QString& s);
      void setLongName(const QTextDocument& s);
      void setShortName(const QTextDocument& s);
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
      int pan() const                     { return _instrument.chorus;      }
      void insertStaff(Staff*);
      void removeStaff(Staff*);
      const Instrument* instrument() const { return &_instrument; }
      Instrument* instrument() { return &_instrument; }
      bool show() const        { return _show; }
      void setShow(bool val);
      Score* score() const     { return cs; }
      };

#endif

