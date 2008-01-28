//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#ifndef _BB_H__
#define __BB_H__

#include "midifile.h"

class BBFile;

//---------------------------------------------------------
//   BBTrack
//---------------------------------------------------------

class BBTrack {
      BBFile* bb;
      EventList _events;
      int _outChannel;
      bool _drumTrack;

      void quantize(int startTick, int endTick, EventList* dst);

   public:
      BBTrack(BBFile*);
      ~BBTrack();
      bool empty() const;
      const EventList events() const    { return _events;     }
      EventList& events()               { return _events;     }
      int outChannel() const            { return _outChannel; }
      void setOutChannel(int n)         { _outChannel = n;    }
      void insert(MidiEvent* e)         { _events.insert(e);  }
      void append(MidiEvent* e)         { _events.append(e);  }

      void findChords();
      int separateVoices(int);
      void cleanup();

      friend class BBFile;
      };

//---------------------------------------------------------
//   BBFile
//---------------------------------------------------------

class BBFile {
      QString _path;
      unsigned char _version;
      char* _title;
      int _style, _key, _bpm;
      unsigned char _chordExt[1024];
      unsigned char _chordBase[1024];
      int _startChorus;
      int _endChorus;
      int _repeats;
      int _flags;
      char* _styleName;
      QList<BBTrack*> _tracks;
      int _measures;
      SigList _siglist;

      QByteArray ba;
      const unsigned char* a;
      int size;

   public:
      BBFile();
      ~BBFile();
      bool read(const QString&);
      QList<BBTrack*>* tracks()   { return &_tracks;  }
      int measures() const        { return _measures; }
      const char* title() const   { return _title;    }
      SigList siglist() const         { return _siglist;         }
      };

#endif

