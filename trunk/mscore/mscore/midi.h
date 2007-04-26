//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: midi.h,v 1.9 2006/03/02 17:08:36 wschweer Exp $
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

#ifndef __MIDI_H__
#define __MIDI_H__

const int MIDI_PORTS = 16;
// const int MIDI_CHANNEL = 16;
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
      ME_CLOCK      = 0xf8,
      ME_START      = 0xfa,
      ME_CONTINUE   = 0xfb,
      ME_STOP       = 0xfc,
      };

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
      };

//---------------------------------------------------------
//   MidiEvent
//---------------------------------------------------------

struct MidiEvent {
      unsigned char* data;
      int dataLen;
      int tick;
      int len;
      int type;
      int channel;
      int port;
      int dataA, dataB, dataC;

      MidiEvent();
      ~MidiEvent();
      bool isNote() const { return type == ME_NOTEON; }
      bool isNoteOff() const {
            return (type == ME_NOTEOFF) || ((type == ME_NOTEON) && (dataB == 0));
            }
      bool isNoteOff(MidiEvent* e) {
            return (e->isNoteOff() && (e->dataA == dataA));
            }
      int pitch() const { return dataA; }
      int velo() const  { return dataB; }
      };

typedef QMultiMap<int, MidiEvent*> EventList;
typedef EventList::iterator iEvent;
typedef EventList::const_iterator ciEvent;

//---------------------------------------------------------
//   MidiTrack
//---------------------------------------------------------

class MidiTrack {
      EventList* _events;
      int _outChannel;
      int _outPort;
      QString _name;
      QString _comment;
      bool _drumTrack;
      bool _minor;
      int _key;

   public:
      MidiTrack();
      ~MidiTrack();

      int hbank, lbank, program;
      int minPitch, maxPitch, medPitch;

      bool empty() const;
      EventList* events() const         { return _events;     }
      void setEvents(EventList* el)     { _events = el;       }
      int outChannel() const            { return _outChannel; }
      void setOutChannel(int n)         { _outChannel = n;    }
      int outPort() const               { return _outPort;    }
      void setOutPort(int n)            { _outPort = n;       }
      QString name() const              { return _name;       }
      void setName(const QString& s)    { _name = s;          }
      QString comment() const           { return _comment;    }
      void setComment(const QString& s) { _comment = s;       }
      bool drumTrack() const            { return _drumTrack;  }
      void setDrumTrack(bool f)         { _drumTrack = f;     }
      QString instrName(int type) const;
      void setMinor(bool v)             { _minor = v;         }
#undef minor    // who sets this macro?
      bool minor() const                { return _minor;      }
      void setKey(int k)                { _key = k;           }
      int key() const                   { return _key;        }
      };

typedef QList<MidiTrack*> MidiTrackList;

//---------------------------------------------------------
//   MidiType
//---------------------------------------------------------

enum MidiType {
      MT_UNKNOWN = 0, MT_GM = 1, MT_GS = 2, MT_XG = 4
      };

//---------------------------------------------------------
//   MidiFile
//---------------------------------------------------------

class MidiFile {
      Score* cs;
      MidiTrackList _tracks;
      int timesig_z, timesig_n;
      int status, click;
      int sstatus;
      int channelprefix;
      QFile* fp;
      int fileDivision;
      int curPos;

      MidiType _midiType;
      char errorBuffer[512];

      bool read(void*, qint64);
      bool write(const void*, qint64);
      void put(unsigned char c) { write(&c, 1); }
      bool skip(qint64);
      int readShort();
      void writeShort(int);
      int readLong();
      void writeLong(int);
      MidiEvent* readEvent();
      bool readTrack(bool);
      bool writeTrack(const MidiTrack*);
      int getvl();
      void putvl(unsigned);
      void writeEvent(int channel, const MidiEvent*);
      void processTrack(MidiTrack* track);
      bool checkSysex(MidiTrack*, unsigned, unsigned char*);

   public:
      MidiFile(Score* s);
      void setFp(QFile* fp);
      bool read();
      bool write();
      const char* error;
      MidiTrackList* tracks()   { return &_tracks; }
      MidiType midiType() const { return _midiType; }

      QString title, subTitle, composer, translator, poet;
      };

#define XCHG_SHORT(x) ((((x)&0xFF)<<8) | (((x)>>8)&0xFF))
#ifdef __i486__
#define XCHG_LONG(x) \
     ({ int __value; \
        asm ("bswap %1; movl %1,%0" : "=g" (__value) : "r" (x)); \
       __value; })
#else
#define XCHG_LONG(x) ((((x)&0xFF)<<24) | \
		      (((x)&0xFF00)<<8) | \
		      (((x)&0xFF0000)>>8) | \
		      (((x)>>24)&0xFF))
#endif

#define BE_SHORT(x) XCHG_SHORT(x)
#define BE_LONG(x)  XCHG_LONG(x)


//---------------------------------------------------------
//   MidiInstrument
//---------------------------------------------------------

struct MidiInstrument {
      int type;
      int hbank, lbank, patch;
      int split;
      char* name;
      };

#endif

