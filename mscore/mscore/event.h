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

#ifndef __EVENT_H__
#define __EVENT_H__

class Note;
class MidiFile;
class Xml;

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
      CTRL_HBANK              = 0x00,
      CTRL_LBANK              = 0x20,
      CTRL_HDATA              = 0x06,
      CTRL_LDATA              = 0x26,
      CTRL_HNRPN              = 0x63,
      CTRL_LNRPN              = 0x62,
      CTRL_HRPN               = 0x65,
      CTRL_LRPN               = 0x64,
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
      CTRL_POLYAFTER = 0x40004,
      };

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

class Event {
      int _type;

      int _ontime;
      int _noquantOntime;
      int _noquantDuration;

      int _channel;     // mscore channel number, not midi channel
      int _a, _b;
      double _tuning;
      int _duration;
      int _tpc;               // tonal pitch class

      int _voice;
      QList<Event*> _notes;

      unsigned char* _data;   // always zero terminated (_data[_len] == 0; )
      int _len;
      int _metaType;

      Note* _note;

   public:
      Event();
      Event(const Event&);
      Event(int t);
      ~Event();

      void write(MidiFile*) const;
      void write(Xml&) const;
      void read(QDomElement);

      bool isChannelEvent() const;

      int noquantOntime() const      { return _noquantOntime;       }
      void setNoquantOntime(int v)   { _noquantOntime = v;          }
      int noquantDuration() const    { return _noquantDuration;     }
      void setNoquantDuration(int v) { _noquantDuration = v;        }

      int type() const               { return _type;                }
      void setType(int v)            { _type = v;                   }
      int ontime() const             { return _ontime;              }
      void setOntime(int v)          { _ontime = v;                 }
      int channel() const            { return _channel;             }
      void setChannel(int c)         { _channel = c;                }
      int dataA() const              { return _a;                   }
      int dataB() const              { return _b;                   }
      void setDataA(int v)           { _a = v;                      }
      void setDataB(int v)           { _b = v;                      }
      int pitch() const              { return _a;                   }
      void setPitch(int v)           { _a = v;                      }
      int velo() const               { return _b;                   }
      void setVelo(int v)            { _b = v;                      }
      int controller() const         { return _a;                   }
      void setController(int val)    { _a = val;                    }
      int value() const              { return _b;                   }
      void setValue(int v)           { _b = v;                      }
      int duration() const           { return _duration;            }
      void setDuration(int v)        { _duration = v;               }
      int voice() const              { return _voice;               }
      void setVoice(int val)         { _voice = val;                }
      int offtime() const            { return ontime() + _duration; }
      QList<Event*>& notes()         { return _notes;               }
      unsigned char* data() const    { return _data;                }
      void setData(unsigned char* d) { _data = d;                   }
      int len() const                { return _len;                 }
      void setLen(int l)             { _len = l;                    }
      int metaType() const           { return _metaType;            }
      void setMetaType(int v)        { _metaType = v;               }
      int tpc() const                { return _tpc;                 }
      void setTpc(int v)             { _tpc = v;                    }
      Note* note() const             { return _note;                }
      void setNote(Note* v)          { _note = v;                   }
      double tuning() const          { return _tuning;              }
      void setTuning(double v)       { _tuning = v;                 }
      };

//---------------------------------------------------------
//   EventList
//   EventMap
//---------------------------------------------------------

class EventList : public QList<Event*> {
   public:
      EventList() : QList<Event*>() {}
      EventList(const EventList& e);
      void insert(Event*);
      };

class EventMap : public QMap<int, Event*> {};

typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

#endif

