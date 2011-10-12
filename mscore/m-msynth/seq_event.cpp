//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: event.cpp 3568 2010-10-09 17:24:22Z wschweer $
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

#include "seq_event.h"
#include "event_p.h"

//---------------------------------------------------------
//   Event::Event
//---------------------------------------------------------

SeqEventData::SeqEventData()
      {
      _data   = 0;
      _note   = 0;
      _tuning = 0.0;
      _ontime = -1;
      _channel = 0;
      }

SeqEventData::SeqEventData(int t)
      {
      _type   = t;
      _data   = 0;
      _note   = 0;
      _tuning = 0.0;
      _ontime = -1;
      _channel = 0;
      }

SeqEventData::SeqEventData(const SeqEventData& e)
   : QSharedData(e)
      {
      _type       = e._type;
      _ontime     = e._ontime;
      _channel    = e._channel;
      _a          = e._a;
      _b          = e._b;
      _tuning     = e._tuning;
      _duration   = e._duration;
      _tpc        = e._tpc;
      _voice      = e._voice;
      _notes      = e._notes;
      _len        = e._len;
      if (e._data) {
            _data = new unsigned char[_len + 1];      // dont forget trailing zero
            memcpy(_data, e._data, _len+1);
            }
      else
            _data = 0;
      _metaType = e._metaType;
      _note     = e._note;
      }

SeqEventData::~SeqEventData()
      {
      delete[] _data;
      }

//---------------------------------------------------------
//   isChannelEvent
//---------------------------------------------------------

bool SeqEventData::isChannelEvent() const
      {
      switch(_type) {
            case ME_NOTEOFF:
            case ME_NOTEON:
            case ME_POLYAFTER:
            case ME_CONTROLLER:
            case ME_PROGRAM:
            case ME_AFTERTOUCH:
            case ME_PITCHBEND:
            case ME_NOTE:
            case ME_CHORD:
                  return true;
            default:
                  return false;
            }
      return false;
      }

bool SeqEventData::operator==(const SeqEventData& e) const
      {
      return false;           // TODO
      }

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

SeqEvent::SeqEvent()
      {
      d = new SeqEventData;
      }

SeqEvent::SeqEvent(const SeqEvent& s)
   : d(s.d)
      {
      }

SeqEvent::SeqEvent(int t)
      {
      d = new SeqEventData(t);
      }

SeqEvent::~SeqEvent()
      {
      }

SeqEvent& SeqEvent::operator=(const SeqEvent& s)
      {
      d = s.d;
      return *this;
      }

bool SeqEvent::isChannelEvent() const    { return d->isChannelEvent(); }

int SeqEvent::noquantOntime() const      { return d->_noquantOntime;       }
void SeqEvent::setNoquantOntime(int v)   { d->_noquantOntime = v;          }
int SeqEvent::noquantDuration() const    { return d->_noquantDuration;     }
void SeqEvent::setNoquantDuration(int v) { d->_noquantDuration = v;        }

int SeqEvent::type() const               { return d->_type;                }
void SeqEvent::setType(int v)            { d->_type = v;                   }
int SeqEvent::ontime() const             { return d->_ontime;              }
void SeqEvent::setOntime(int v)          { d->_ontime = v;                 }
int SeqEvent::channel() const            { return d->_channel;             }
void SeqEvent::setChannel(int c)         { d->_channel = c;                }
int SeqEvent::dataA() const              { return d->_a;                   }
int SeqEvent::dataB() const              { return d->_b;                   }
void SeqEvent::setDataA(int v)           { d->_a = v;                      }
void SeqEvent::setDataB(int v)           { d->_b = v;                      }
int SeqEvent::pitch() const              { return d->_a;                   }
void SeqEvent::setPitch(int v)           { d->_a = v;                      }
int SeqEvent::velo() const               { return d->_b;                   }
void SeqEvent::setVelo(int v)            { d->_b = v;                      }
int SeqEvent::controller() const         { return d->_a;                   }
void SeqEvent::setController(int val)    { d->_a = val;                    }
int SeqEvent::value() const              { return d->_b;                   }
void SeqEvent::setValue(int v)           { d->_b = v;                      }
int SeqEvent::duration() const           { return d->_duration;            }
void SeqEvent::setDuration(int v)        { d->_duration = v;               }
int SeqEvent::voice() const              { return d->_voice;               }
void SeqEvent::setVoice(int val)         { d->_voice = val;                }
int SeqEvent::offtime() const            { return d->ontime() + d->_duration; }
QList<SeqEvent>& SeqEvent::notes()          { return d->_notes;               }
const uchar* SeqEvent::data() const      { return d->_data;                }
void SeqEvent::setData(uchar* p)         { d->_data = p;                   }
int SeqEvent::len() const                { return d->_len;                 }
void SeqEvent::setLen(int l)             { d->_len = l;                    }
int SeqEvent::metaType() const           { return d->_metaType;            }
void SeqEvent::setMetaType(int v)        { d->_metaType = v;               }
int SeqEvent::tpc() const                { return d->_tpc;                 }
void SeqEvent::setTpc(int v)             { d->_tpc = v;                    }
const Note* SeqEvent::note() const       { return d->_note;                }
void SeqEvent::setNote(const Note* v)    { d->_note = v;                   }
float SeqEvent::tuning() const          { return d->_tuning;              }
void SeqEvent::setTuning(float v)       { d->_tuning = v;                 }
bool SeqEvent::operator==(const SeqEvent& e) const { return d->operator==(*e.d);   }

