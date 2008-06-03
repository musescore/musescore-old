//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __EVENT_H__
#define __EVENT_H__

class Note;
class MidiFile;
class Xml;
class MidiOutEvent;

//---------------------------------------------------------
//   Midi Events
//---------------------------------------------------------

enum {
      ME_NOTEOFF    = 0x80,
      ME_NOTEON     = 0x90,
      ME_POLYAFTER  = 0xa0,
      ME_CONTROLLER = 0xb0,
      ME_PROGRAM    = 0xc0,
      ME_AFTERTOUCH = 0xd0,
      ME_PITCHBEND  = 0xe0,
      ME_SYSEX      = 0xf0,
      ME_META       = 0xff,
      ME_SONGPOS    = 0xf2,
      ME_ENDSYSEX   = 0xf7,
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,

      ME_NOTE       = 0x100,
      ME_CHORD      = 0x101,
      };

//---------------------------------------------------------
//   Midi Meta Events
//---------------------------------------------------------

enum {
      META_SEQUENCE_NUMBER = 0,
      META_TEXT            = 1,
      META_COPYRIGHT       = 2,
      META_TRACK_NAME      = 3,
      META_INSTRUMENT_NAME = 4,
      META_LYRIC           = 5,
      META_MARKER          = 6,
      META_CUE_POINT       = 7,
      META_TITLE           = 8,     // mscore extension
      META_SUBTITLE        = 9,     // mscore extension
      META_COMPOSER        = 0xa,   // mscore extension
      META_TRANSLATOR      = 0xb,   // mscore extension
      META_POET            = 0xc,   // mscore extension
      META_TRACK_COMMENT   = 0xf,
      META_PORT_CHANGE     = 0x21,
      META_CHANNEL_PREFIX  = 0x22,
      META_TEMPO           = 0x51,
      META_TIME_SIGNATURE  = 0x58,
      META_KEY_SIGNATURE   = 0x59,
      };

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
//   Event
//---------------------------------------------------------

class Event {
      int _port;
      int _ontime;

   public:
      Event()           { _ontime = -1; _port = 0; }
      Event(int t)      { _ontime = t;  }
      virtual ~Event()  {}
      virtual int type() const = 0;

      int ontime() const    { return _ontime; }
      void setOntime(int v) { _ontime = v;    }
      int port() const      { return _port;   }
      void setPort(int v)   { _port = v;      }

      virtual bool isChannelEvent() const = 0;
      virtual void write(MidiFile*) const {}
      virtual void write(Xml&) const {}
      virtual void read(QDomElement) {}
      virtual bool midiOutEvent(MidiOutEvent*) { return false; }
      };

//---------------------------------------------------------
//   MidiChannelEvent
//---------------------------------------------------------

class ChannelEvent : public Event {
      int _channel;

   protected:
      int _a;
      int _b;

   public:
      ChannelEvent()          { _channel = 0; _a = -1; _b = -1;   }
      ChannelEvent(int t, int c) : Event(t), _channel(c) {}
      ChannelEvent(int t, int c, int a, int b)
         : Event(t), _channel(c), _a(a), _b(b) {}
      virtual int type() const = 0;

      bool isChannelEvent() const { return true;     }
      int channel() const         { return _channel; }
      void setChannel(int c)      { _channel = c;    }
      int dataA() const           { return _a;       }
      int dataB() const           { return _b;       }
      void setDataA(int v)        { _a = v;          }
      void setDataB(int v)        { _b = v;          }
      };

//---------------------------------------------------------
//   NoteOnOff
//---------------------------------------------------------

class NoteOnOff : public ChannelEvent {
      Note* _note;

   public:
      NoteOnOff()          { _note = 0; }
      NoteOnOff(int t, int c, int p, int v)
         : ChannelEvent(t, c, p, v) { _note = 0; }
      virtual int type() const = 0;

      int pitch() const        { return _a; }
      void setPitch(int p)     { _a = p;    }
      int velo() const         { return _b; }
      void setVelo(int v)      { _b = v;    }
      Note* note() const       { return _note; }
      void setNote(Note* n)    { _note = n; }
      };

//---------------------------------------------------------
//   NoteOn
//---------------------------------------------------------

class NoteOn : public NoteOnOff {
   public:
      NoteOn()             {}
      NoteOn(int t, int c, int p, int v) : NoteOnOff(t, c, p, v) {}
      virtual int type() const { return ME_NOTEON; }

      virtual void write(MidiFile*) const;
      virtual void write(Xml&) const;
      };

//---------------------------------------------------------
//   MidiNoteOff
//---------------------------------------------------------

class NoteOff : public NoteOnOff {
   public:
      NoteOff()            {}
      NoteOff(int t, int c, int p, int v) : NoteOnOff(t, c, p, v) {}
      virtual int type() const { return ME_NOTEOFF; }

      virtual void write(MidiFile*) const;
      virtual void write(Xml&) const;
      };

//---------------------------------------------------------
//   NoteEvent
//---------------------------------------------------------

class NoteEvent : public NoteOnOff {
      int _duration;
      int _tpc;               // tonal pitch class

   public:
      NoteEvent()               { _a = -1; _b = -1; _duration = -1; }
      virtual int type() const { return ME_NOTE; }

      int duration() const     { return _duration; }
      void setDuration(int v)  { _duration = v; }
      int offtime() const      { return ontime() + _duration; }
      int tpc() const          { return _tpc;    }
      void setTpc(int v)       { _tpc = v;       }
      virtual void write(Xml&) const;
      };

//---------------------------------------------------------
//   ChordEvent
//---------------------------------------------------------

class ChordEvent : public Event {
      int _duration;
      int _voice;
      QList<NoteEvent*> _notes;

   public:
      ChordEvent()                  {}
      virtual int type() const      { return ME_CHORD;  }

      int duration() const          { return _duration; }
      void setDuration(int v)       { _duration = v;    }
      int voice() const             { return _voice;    }
      void setVoice(int val)        { _voice = val;     }
      int offtime() const           { return ontime() + _duration; }
      QList<NoteEvent*>& notes()    { return _notes;    }
      bool isChannelEvent() const   { return true;      }
      virtual void write(Xml&) const {}
      };

//---------------------------------------------------------
//   ControllerEvent
//    the following midi events are stored as special
//    controller:
//      ME_POLYAFTER  =
//      ME_PROGRAM    =
//      ME_AFTERTOUCH =
//      ME_PITCHBEND  =
//---------------------------------------------------------

class ControllerEvent : public ChannelEvent {
   public:
      ControllerEvent()                    { _a = -1; _b = 0; }
      ControllerEvent(int t, int ch, int c, int v)
         : ChannelEvent(t, ch, c, v) {}
      virtual int type() const            { return ME_CONTROLLER; }

      virtual bool isChannelEvent() const { return true; }
      int controller() const              { return _a; }
      void setController(int val)         { _a = val; }
      int value() const                   { return _b; }
      void setValue(int v)                { _b = v; }
      virtual void write(MidiFile*) const;
      virtual void write(Xml&) const;
      virtual bool midiOutEvent(MidiOutEvent*);
      };

//---------------------------------------------------------
//   DataEvent
//---------------------------------------------------------

class DataEvent : public Event {
   protected:
      int _len;
      unsigned char* _data;

   public:
      DataEvent() { _data = 0; _len = 0; }
      DataEvent(int t, int l, unsigned char* d) : Event(t), _len(l), _data(d) {}
      virtual int type() const = 0;

      ~DataEvent() {
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
//   SysexEvent
//---------------------------------------------------------

class SysexEvent : public DataEvent {
   public:
      SysexEvent()              {}
      SysexEvent(int t, int l, unsigned char* d) : DataEvent(t, l, d) {}
      virtual int type() const { return ME_SYSEX; }

      virtual void write(MidiFile*) const;
      virtual void write(Xml&) const;
      };

//---------------------------------------------------------
//   MetaEvent
//---------------------------------------------------------

class MetaEvent : public DataEvent {
      int _metaType;

   public:
      MetaEvent()               { _metaType = -1; }
      MetaEvent(int t, int mt, int l, unsigned char* d)
         : DataEvent(t, l, d), _metaType(mt) {}
      virtual int type() const { return ME_META;   }

      int metaType() const     { return _metaType; }
      void setMetaType(int v)  { _metaType = v;    }
      virtual void write(MidiFile*) const;
      virtual void write(Xml&) const;
      };

//---------------------------------------------------------
//   EventList
//   EventMap
//---------------------------------------------------------

class EventList : public QList<Event*> {
   public:
      void insert(Event*);
      };

class EventMap : public QMap<int, Event*> {};

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

#endif

