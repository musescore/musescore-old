//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: event.h 3358 2010-08-07 16:40:56Z wschweer $
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

#ifndef __EVENT_P_H__
#define __EVENT_P_H__

class Note;
class MidiFile;

//---------------------------------------------------------
//   EventData
//---------------------------------------------------------

class SeqEventData : public QSharedData {
   protected:
      int _type;
      int _ontime;
      int _noquantOntime;
      int _noquantDuration;
      int _channel;           // mscore channel number, not midi channel
      int _a, _b;
      int _duration;
      int _tpc;               // tonal pitch class
      int _voice;
      QList<SeqEvent> _notes;
      uchar* _data;   // always zero terminated (_data[_len] == 0; )
      int _len;
      int _metaType;
      const Note* _note;
      float _tuning;

      int noquantOntime() const      { return  _noquantOntime;       }
      void setNoquantOntime(int v)   {  _noquantOntime = v;          }
      int noquantDuration() const    { return  _noquantDuration;     }
      void setNoquantDuration(int v) {  _noquantDuration = v;        }

      int type() const               { return  _type;                }
      void setType(int v)            {  _type = v;                   }
      int ontime() const             { return  _ontime;              }
      void setOntime(int v)          {  _ontime = v;                 }
      int channel() const            { return  _channel;             }
      void setChannel(int c)         {  _channel = c;                }
      int dataA() const              { return  _a;                   }
      int dataB() const              { return  _b;                   }
      void setDataA(int v)           {  _a = v;                      }
      void setDataB(int v)           {  _b = v;                      }
      int pitch() const              { return  _a;                   }
      void setPitch(int v)           {  _a = v;                      }
      int velo() const               { return  _b;                   }
      void setVelo(int v)            {  _b = v;                      }
      int controller() const         { return  _a;                   }
      void setController(int val)    {  _a = val;                    }
      int value() const              { return  _b;                   }
      void setValue(int v)           {  _b = v;                      }
      int duration() const           { return  _duration;            }
      void setDuration(int v)        {  _duration = v;               }
      int voice() const              { return  _voice;               }
      void setVoice(int val)         {  _voice = val;                }
      int offtime() const            { return  ontime() +  _duration; }
      QList<SeqEvent>& notes()          { return  _notes;               }
      const uchar* data() const      { return  _data;                }
      void setData(uchar* d)         {  _data = d;                   }
      int len() const                { return  _len;                 }
      void setLen(int l)             {  _len = l;                    }
      int metaType() const           { return  _metaType;            }
      void setMetaType(int v)        {  _metaType = v;               }
      int tpc() const                { return  _tpc;                 }
      void setTpc(int v)             {  _tpc = v;                    }
      const Note* note() const       { return  _note;                }
      void setNote(const Note* v)    {  _note = v;                   }
      float tuning() const           { return  _tuning;              }
      void setTuning(float v)        {  _tuning = v;                 }
      bool operator==(const SeqEventData& e) const;

   public:
      SeqEventData();
      SeqEventData(const SeqEventData&);
      SeqEventData(int t);
      ~SeqEventData();

      void write(MidiFile*) const;

      bool isChannelEvent() const;

      friend class SeqEvent;
      };

#endif

