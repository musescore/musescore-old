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

#ifndef __PART_H__
#define __PART_H__

#include "globals.h"
#include "instrument.h"
#include "text.h"

class Xml;
class Staff;
class Score;
class Drumset;
class InstrumentTemplate;

//---------------------------------------------------------
//   Part
//---------------------------------------------------------

class Part {
      Score* _score;
      QString _trackName;           ///< used in tracklist

      TextC* _longName;
      TextC* _shortName;

      Instrument _instrument;
      QList<Staff*> _staves;
      QString _id;                  ///< used for MusicXml import
      bool _show;                   ///< show part in partitur if true

   public:
      Part(Score*);
      ~Part();
      void initFromInstrTemplate(const InstrumentTemplate*);

      void read(QDomElement);
      void write(Xml& xml) const;
      int nstaves() const                       { return _staves.size(); }
      QList<Staff*>* staves()                   { return &_staves; }
      const QList<Staff*>* staves() const       { return &_staves; }
      Staff* staff(int idx) const;
      void setId(const QString& s)              { _id = s; }
      QString id() const                        { return _id; }
      QString trackName() const                 { return _trackName;  }
      void setTrackName(const QString& s)       { _trackName = s; }

      QString shortNameHtml() const;
      QString longNameHtml()  const;

      TextC* longName()                         { return _longName; }
      TextC* shortName()                        { return _shortName; }
      void setLongName(const QString& s);
      void setLongNameEncoded(const QString& s);
      void setShortNameEncoded(const QString& s);
      void setShortName(const QString& s);
      void setLongNameHtml(const QString& s);
      void setShortNameHtml(const QString& s);
      void setLongName(const QTextDocument& s);
      void setShortName(const QTextDocument& s);

      void setStaves(int);

      void setAmateurPitchRange(int a, int b)  {
            _instrument.minPitchA = a;
            _instrument.maxPitchA = b;
            }
      void setProfessionalPitchRange(int a, int b)  {
            _instrument.minPitchP = a;
            _instrument.maxPitchP = b;
            }

      void setDrumset(Drumset* ds)             { _instrument.drumset = ds;      }
      Drumset* drumset() const                 { return _instrument.drumset;    }
      bool useDrumset() const                  { return _instrument.useDrumset; }
      void setUseDrumset(bool val);

      int minPitchP() const                    { return _instrument.minPitchP;  }
      int maxPitchP() const                    { return _instrument.maxPitchP;  }
      int minPitchA() const                    { return _instrument.minPitchA;  }
      int maxPitchA() const                    { return _instrument.maxPitchA;  }
      void setMinPitchP(int v)                 { _instrument.minPitchP = v;     }
      void setMaxPitchP(int v)                 { _instrument.maxPitchP = v;     }
      void setMinPitchA(int v)                 { _instrument.minPitchA = v;     }
      void setMaxPitchA(int v)                 { _instrument.maxPitchA = v;     }
      int transposeChromatic() const           { return _instrument.transposeChromatic; }
      int transposeDiatonic() const            { return _instrument.transposeDiatonic; }
      void setTransposeChromatic(int v)        { _instrument.transposeChromatic = v; }
      void setTransposeDiatonic(int v)         { _instrument.transposeDiatonic = v; }

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
      const Instrument* instrument() const     { return &_instrument; }
      Instrument* instrument()                 { return &_instrument; }
      void setInstrument(const Instrument& i)  { _instrument = i;     }
      bool show() const                        { return _show;        }
      void setShow(bool val);
      Score* score() const                     { return _score; }
      };

#endif

