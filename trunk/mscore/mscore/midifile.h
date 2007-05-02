//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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

const int MIDI_CHANNEL = 16;

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
      char dataA, dataB, dataC;

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
      bool isChannelEvent() const {
            return (type == ME_NOTEOFF
                   || type == ME_NOTEON
                   || type == ME_POLYAFTER
                   || type == ME_CONTROLLER
                   || type == ME_PROGRAM
                   || type == ME_AFTERTOUCH
                   || type == ME_PITCHBEND);
            }
      };

typedef QMultiMap<int, MidiEvent*> EventList;
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
      void setOutChannel(int n)         { _outChannel = n;    }
      int outPort() const               { return _outPort;    }
      void setOutPort(int n)            { _outPort = n;       }
      QString name() const              { return _name;       }
      void setName(const QString& s)    { _name = s;          }
      QString comment() const           { return _comment;    }
      void setComment(const QString& s) { _comment = s;       }
      void insert(MidiEvent* e)         { _events.insert(e->tick, e); }
      void mergeNoteOnOff();
      void cleanup();
      void changeDivision(int newDivision);
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
      QIODevice* fp;
      MidiTrackList _tracks;
      int timesig_z, timesig_n;
      int status, click;
      int sstatus;
      int _division;
      int curPos;
      int _format;

      MidiType _midiType;
      char errorBuffer[512];

      // write
      bool write(const void*, qint64);
      void put(unsigned char c) { write(&c, 1); }
      bool skip(qint64);
      void writeShort(int);
      void writeLong(int);
      void putvl(unsigned);
      void writeEvent(int channel, const MidiEvent*);
      bool writeTrack(const MidiTrack*);

      // read
      bool read(void*, qint64);
      int getvl();
      int readShort();
      int readLong();
      MidiEvent* readEvent();
      bool readTrack(bool);

   public:
      MidiFile();
      bool read(QIODevice*);
      bool write(QIODevice*);

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

#endif

