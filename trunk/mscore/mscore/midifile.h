//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __MIDIFILE_H__
#define __MIDIFILE_H__

#include "sig.h"
#include "event.h"

const int MIDI_CHANNEL = 16;

//---------------------------------------------------------
//   MidiType
//---------------------------------------------------------

enum MidiType {
      MT_UNKNOWN = 0, MT_GM = 1, MT_GS = 2, MT_XG = 4
      };

class MidiFile;
class Xml;

class MidiFile;

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

class MidiTrack {
      MidiFile* mf;
      EventList _events;
      int _outChannel;
      int _outPort;
      QString _name;
      QString _comment;
      bool _drumTrack;
      bool _hasKey;
      int _staffIdx;

   protected:
      void readXml(QDomElement);

   public:
      int maxPitch;
      int minPitch;
      int medPitch;
      int program;

      MidiTrack(MidiFile*);
      ~MidiTrack();

      bool empty() const;
      const EventList events() const    { return _events;     }
      EventList& events()               { return _events;     }
      int outChannel() const            { return _outChannel; }
      void setOutChannel(int n);
      int outPort() const               { return _outPort;    }
      void setOutPort(int n)            { _outPort = n;       }
      QString name() const              { return _name;       }
      void setName(const QString& s)    { _name = s;          }
      QString comment() const           { return _comment;    }
      void setComment(const QString& s) { _comment = s;       }
      void insert(Event* e)             { _events.insert(e);  }
      void append(Event* e)             { _events.append(e);  }
      void mergeNoteOnOff();
      void cleanup();
      inline int division() const;
      void changeDivision(int newDivision);
      void move(int ticks);
      bool isDrumTrack() const;
      void extractTimeSig(SigList* sig);
      void quantize(int startTick, int endTick, EventList* dst);
      int getInitProgram();
      void findChords();
      int separateVoices(int);
      void setHasKey(bool val) { _hasKey = val;    }
      bool hasKey() const      { return _hasKey;   }
      int staffIdx() const     { return _staffIdx; }
      void setStaffIdx(int v)  { _staffIdx = v;    }

      friend class MidiFile;
      };

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile {
      SigList _siglist;
      QIODevice* fp;
      QList<MidiTrack*> _tracks;
      int _division;
      int _format;               ///< midi file format (0-2)
      bool _noRunningStatus;     ///< do not use running status on output
      MidiType _midiType;

      // values used during read()
      int status;                ///< running status
      int sstatus;               ///< running status (not reset after meta or sysex events)
      int click;                 ///< current tick position in file
      qint64 curPos;             ///< current file byte position
      int _shortestNote;

   protected:
      // write
      bool write(const void*, qint64);
      bool skip(qint64);
      void writeShort(int);
      void writeLong(int);
      bool writeTrack(const MidiTrack*);
      void putvl(unsigned);
      void put(unsigned char c) { write(&c, 1); }
      void writeStatus(int type, int channel);

      // read
      bool read(void*, qint64);
      int getvl();
      int readShort();
      int readLong();
      Event* readEvent();
      bool readTrack();
      QString _error;

      void resetRunningStatus() { status = -1; }

   public:
      MidiFile();
      bool read(QIODevice*);
      bool write(QIODevice*);
      void readXml(QDomElement);

      QList<MidiTrack*>* tracks()   { return &_tracks;  }
      MidiType midiType() const     { return _midiType; }
      void setMidiType(MidiType mt) { _midiType = mt;   }
      int format() const            { return _format;   }
      void setFormat(int fmt)       { _format = fmt;    }
      int division() const          { return _division; }
      void setDivision(int val)     { _division = val;  }
      void changeDivision(int val);
      void process1();
      void sortTracks();
      void separateChannel();
      void move(int ticks);
      SigList siglist() const         { return _siglist;         }
      int noRunningStatus() const     { return _noRunningStatus; }
      void setNoRunningStatus(bool v) { _noRunningStatus = v;    }
      void processMeta(Score*, MidiTrack* track, int staffIdx, MetaEvent* e);
      const QString& error() const    { return _error; }
      void setShortestNote(int v)     { _shortestNote = v;    }
      int shortestNote() const        { return _shortestNote; }

      friend class NoteOn;
      friend class NoteOff;
      friend class MetaEvent;
      friend class SysexEvent;
      friend class ControllerEvent;
      friend class MidiTrack;
      };

int MidiTrack::division() const { return mf->division(); }

extern QString midiMetaName(int meta);

#endif

