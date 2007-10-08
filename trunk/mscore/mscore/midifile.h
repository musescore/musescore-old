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
//   Midi Controller
//---------------------------------------------------------

enum {
      CTRL_HBANK = 0x00,
      CTRL_LBANK = 0x20,

      CTRL_HDATA = 0x06,
      CTRL_LDATA = 0x26,

      CTRL_HNRPN = 0x63,
      CTRL_LNRPN = 0x62,

      CTRL_HRPN  = 0x65,
      CTRL_LRPN  = 0x64,

      CTRL_MODULATION         = 0x01,
      CTRL_PORTAMENTO_TIME    = 0x05,
      CTRL_VOLUME             = 0x07,
      CTRL_PANPOT             = 0x0a,
      CTRL_EXPRESSION         = 0x0b,
      CTRL_SUSTAIN            = 0x40,
      CTRL_PORTAMENTO         = 0x41,
      CTRL_SOSTENUTO          = 0x42,
      CTRL_SOFT_PEDAL         = 0x43,
      CTRL_HARMONIC_CONTENT   = 0x47,
      CTRL_RELEASE_TIME       = 0x48,
      CTRL_ATTACK_TIME        = 0x49,

      CTRL_BRIGHTNESS         = 0x4a,
      CTRL_PORTAMENTO_CONTROL = 0x54,
      CTRL_REVERB_SEND        = 0x5b,
      CTRL_CHORUS_SEND        = 0x5d,
      CTRL_VARIATION_SEND     = 0x5e,

      CTRL_ALL_SOUNDS_OFF     = 0x78, // 120
      CTRL_RESET_ALL_CTRL     = 0x79, // 121
      CTRL_LOCAL_OFF          = 0x7a, // 122

      // special midi events are mapped to internal
      // controller
      //
      CTRL_PROGRAM   = 0x40001,
      CTRL_PITCH     = 0x40002,
      CTRL_PRESS     = 0x40003,
      CTRL_POLYAFTER = 0x40004
      };

//---------------------------------------------------------
//   MidiType
//---------------------------------------------------------

enum MidiType {
      MT_UNKNOWN = 0, MT_GM = 1, MT_GS = 2, MT_XG = 4
      };

class MidiFile;
class Xml;

//---------------------------------------------------------
//   MidiEvent
//---------------------------------------------------------

class MidiEvent {
      int _ontime;

   public:
      MidiEvent()           { _ontime = -1; }
      MidiEvent(int t)      { _ontime = t;  }
      virtual ~MidiEvent()  {}
      virtual int type() const = 0;

      int ontime() const    { return _ontime; }
      void setOntime(int v) { _ontime = v; }

      virtual bool isChannelEvent() const = 0;
      virtual void write(MidiFile*) const {}
      virtual void dump(Xml&) const {}
      };

//---------------------------------------------------------
//   MidiChannelEvent
//---------------------------------------------------------

class MidiChannelEvent : public MidiEvent {
      char _channel;

   public:
      MidiChannelEvent()          { _channel = 0;   }
      MidiChannelEvent(int t, int c) : MidiEvent(t), _channel(c) {}
      virtual int type() const = 0;

      bool isChannelEvent() const { return true;     }
      int channel() const         { return _channel; }
      void setChannel(int c)      { _channel = c;    }
      };

//---------------------------------------------------------
//   MidiNoteOnOff
//---------------------------------------------------------

class MidiNoteOnOff : public MidiChannelEvent {
   protected:
      char _pitch;
      char _velo;

   public:
      MidiNoteOnOff()          { _pitch = -1; _velo = -1; }
      MidiNoteOnOff(int t, int c, int p, int v)
         : MidiChannelEvent(t, c), _pitch(p), _velo(v) {}
      virtual int type() const = 0;

      int pitch() const        { return _pitch; }
      void setPitch(int p)     { _pitch = p; }
      int velo() const         { return _velo; }
      void setVelo(int v)      { _velo = v; }
      };

//---------------------------------------------------------
//   MidiNoteOn
//---------------------------------------------------------

class MidiNoteOn : public MidiNoteOnOff {
   public:
      MidiNoteOn()             {}
      MidiNoteOn(int t, int c, int p, int v) : MidiNoteOnOff(t, c, p, v) {}
      virtual int type() const { return ME_NOTEON; }

      virtual void write(MidiFile*) const;
      virtual void dump(Xml&) const;
      };

//---------------------------------------------------------
//   MidiNoteOff
//---------------------------------------------------------

class MidiNoteOff : public MidiNoteOnOff {
      char _pitch;
      char _velo;

   public:
      MidiNoteOff()            {}
      MidiNoteOff(int t, int c, int p, int v) : MidiNoteOnOff(t, c, p, v) {}
      virtual int type() const { return ME_NOTEOFF; }

      int pitch() const        { return _pitch; }
      void setPitch(int p)     { _pitch = p; }
      int velo() const         { return _velo; }
      void setVelo(int v)      { _velo = v; }
      virtual void write(MidiFile*) const;
      virtual void dump(Xml&) const;
      };

//---------------------------------------------------------
//   MidiNote
//---------------------------------------------------------

class MidiNote : public MidiNoteOnOff {
      int _duration;
      int _tpc;               // tonal pitch class

   public:
      MidiNote()               { _pitch = -1; _velo = -1; _duration = -1; }
      virtual int type() const { return ME_NOTE; }

      int duration() const     { return _duration; }
      void setDuration(int v)  { _duration = v; }
      int offtime() const      { return ontime() + _duration; }
      int tpc() const          { return _tpc;    }
      void setTpc(int v)       { _tpc = v;       }
      virtual void dump(Xml&) const;
      };

//---------------------------------------------------------
//   MidiChord
//---------------------------------------------------------

class MidiChord : public MidiEvent {
      int _duration;
      int _voice;
      QList<MidiNote*> _notes;

   public:
      MidiChord()                   {}
      virtual int type() const      { return ME_CHORD;  }

      int duration() const          { return _duration; }
      void setDuration(int v)       { _duration = v;    }
      int voice() const             { return _voice;    }
      void setVoice(int val)        { _voice = val;     }
      int offtime() const           { return ontime() + _duration; }
      QList<MidiNote*>& notes()     { return _notes;    }
      bool isChannelEvent() const   { return true;      }
      virtual void dump(Xml&) const {}
      };

//---------------------------------------------------------
//   MidiController
//    the following midi events are stored as special
//    controller:
//      ME_POLYAFTER  =
//      ME_PROGRAM    =
//      ME_AFTERTOUCH =
//      ME_PITCHBEND  =
//---------------------------------------------------------

class MidiController : public MidiChannelEvent {
      int _controller;
      int _value;

   public:
      MidiController()                    { _controller = -1; _value = 0; }
      MidiController(int t, int ch, int c, int v)
         : MidiChannelEvent(t, ch), _controller(c), _value(v) {}
      virtual int type() const            { return ME_CONTROLLER; }

      virtual bool isChannelEvent() const { return true; }
      int controller() const              { return _controller; }
      void setController(int val)         { _controller = val; }
      int value() const                   { return _value; }
      void setValue(int v)                { _value = v; }
      virtual void write(MidiFile*) const;
      virtual void dump(Xml&) const;
      };

//---------------------------------------------------------
//   MidiData
//---------------------------------------------------------

class MidiData : public MidiEvent {
   protected:
      int _len;
      unsigned char* _data;

   public:
      MidiData() { _data = 0; _len = 0; }
      MidiData(int t, int l, unsigned char* d) : MidiEvent(t), _len(l), _data(d) {}
      virtual int type() const = 0;

      ~MidiData() {
            if (_data)
                  delete _data;
            }
      virtual bool isChannelEvent() const { return false; }
      unsigned char* data() const         { return _data; }
      void setData(unsigned char* d)      { _data = d;    }
      int len() const                     { return _len;  }
      void setLen(int l)                  { _len = l;     }
      };

//---------------------------------------------------------
//   MidiSysex
//---------------------------------------------------------

class MidiSysex : public MidiData {
   public:
      MidiSysex()              {}
      MidiSysex(int t, int l, unsigned char* d) : MidiData(t, l, d) {}
      virtual int type() const { return ME_SYSEX; }

      virtual void write(MidiFile*) const;
      virtual void dump(Xml&) const;
      };

//---------------------------------------------------------
//   MidiMeta
//---------------------------------------------------------

class MidiMeta : public MidiData {
      int _metaType;

   public:
      MidiMeta()               { _metaType = -1; }
      MidiMeta(int t, int mt, int l, unsigned char* d)
         : MidiData(t, l, d), _metaType(mt) {}
      virtual int type() const { return ME_META;   }

      int metaType() const     { return _metaType; }
      void setMetaType(int v)  { _metaType = v;    }
      virtual void write(MidiFile*) const;
      virtual void dump(Xml&) const;
      };

class EventList : public QList<MidiEvent*> {
   public:
      void insert(MidiEvent*);
      void insert(int,int);
      };

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

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
      void insert(MidiEvent* e)         { _events.insert(e);  }
      void append(MidiEvent* e)         { _events.append(e);  }
      void mergeNoteOnOff();
      void cleanup();
      void changeDivision(int newDivision);
      void move(int ticks);
      bool isDrumTrack() const;
      void extractTimeSig(SigList* sig);
      void quantize(int startTick, int endTick, EventList* dst);
      int getInitProgram();
      void findChords();
      int separateVoices(int);

      friend class MidiFile;
      };

typedef QList<MidiTrack*> MidiTrackList;

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile {
      SigList _siglist;
      QIODevice* fp;
      MidiTrackList _tracks;
      int _division;
      int _format;               ///< midi file format (0-2)
      bool _noRunningStatus;     ///< do not use running status on output
      MidiType _midiType;

      // values used during read()
      int status;                ///< running status
      int sstatus;               ///< running status (not reset after meta or sysex events)
      int click;                 ///< current tick position in file
      qint64 curPos;             ///< current file byte position

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
      MidiEvent* readEvent();
      bool readTrack();

      void resetRunningStatus() { status = -1; }

   public:
      MidiFile();
      bool read(QIODevice*);
      bool write(QIODevice*);
      void readXml(QDomElement);

      MidiTrackList* tracks()       { return &_tracks;  }
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

      friend class MidiNoteOn;
      friend class MidiNoteOff;
      friend class MidiMeta;
      friend class MidiSysex;
      friend class MidiController;
      friend class MidiTrack;
      };

extern QString midiMetaName(int meta);

#endif

