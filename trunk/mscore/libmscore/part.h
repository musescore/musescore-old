//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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
#include "instrument.h"
#include "text.h"

class Xml;
class Staff;
class Score;
class InstrumentTemplate;

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part {
      Score* _score;

      QString _trackName;           ///< used in tracklist
      InstrumentList _instrList;

      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score*);

      void read(XmlReader*);
      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }

//      const QTextDocumentFragment& longName(int tick = 0) const  { return instr(tick)->longName();  }
//      const QTextDocumentFragment& shortName(int tick = 0) const { return instr(tick)->shortName(); }
//      QTextDocumentFragment& longName(int tick = 0)              { return instr(tick)->longName();  }
//      QTextDocumentFragment& shortName(int tick = 0)             { return instr(tick)->shortName(); }

      void setLongName(const QString& s);
      void setShortName(const QString& s);

      void setStaves(int);

      int volume() const;
      int reverb() const;
      int chorus() const;
      int pan() const;
      int midiProgram() const;
      void setMidiProgram(int);

      int midiChannel() const;
      void setMidiChannel(int) const;

      void insertStaff(Staff*);
      void removeStaff(Staff*);
      bool show() const                        { return _show;        }
      void setShow(bool val);
      Score* score() const                     { return _score; }

      Instrument* instr(int tick = 0);
      const Instrument* instr(int tick = 0) const;
      void setInstrument(const Instrument&, int tick = 0);
      void removeInstrument(int tick);

      QString trackName() const                { return _trackName; }
      void setTrackName(const QString& s)      { _trackName = s; }
      InstrumentList* instrList()              { return &_instrList;       }
      };

#endif

