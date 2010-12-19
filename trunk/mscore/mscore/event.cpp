//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "midifile.h"
#include "xml.h"
#include "driver.h"
#include "score.h"
#include "part.h"
#include "event.h"
#include "event_p.h"

//---------------------------------------------------------
//   Event::Event
//---------------------------------------------------------

EventData::EventData()
      {
      _type            = 0;
      _ontime          = 0;
      _noquantOntime   = 0;
      _noquantDuration = 0;
      _channel         = 0;
      _a               = 0;
      _b               = 0;
      _duration        = 0;
      _tpc             = 0;
      _voice           = 0;
      _data            = 0;
      _len             = 0;
      _metaType        = 0;
      _note            = 0;
      _tuning          = 0.0;
      }

EventData::EventData(int t)
      {
      _type            = t;
      _ontime          = 0;
      _noquantOntime   = 0;
      _noquantDuration = 0;
      _channel         = 0;
      _a               = 0;
      _b               = 0;
      _duration        = 0;
      _tpc             = 0;
      _voice           = 0;
      _data            = 0;
      _len             = 0;
      _metaType        = 0;
      _note            = 0;
      _tuning          = 0.0;
      }

EventData::EventData(const EventData& e)
   : QSharedData(e)
      {
      _type       = e._type;
      _ontime     = e._ontime;
      _noquantOntime   = e._noquantOntime;
      _noquantDuration = e._noquantDuration;
      _channel    = e._channel;
      _a          = e._a;
      _b          = e._b;
      _duration   = e._duration;
      _tpc        = e._tpc;
      _voice      = e._voice;
      _notes      = e._notes;
      if (e._data) {
            _data = new unsigned char[e._len + 1];      // dont forget trailing zero
            memcpy(_data, e._data, e._len+1);
            }
      else
            _data = 0;
      _len        = e._len;
      _metaType   = e._metaType;
      _note       = e._note;
      _tuning     = e._tuning;
      }

EventData::~EventData()
      {
      delete[] _data;
      }

//---------------------------------------------------------
//   isChannelEvent
//---------------------------------------------------------

bool EventData::isChannelEvent() const
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

//---------------------------------------------------------
//   EventData::write
//---------------------------------------------------------

void EventData::write(Xml& xml) const
      {
      switch(_type) {
            case ME_NOTE:
                  xml.tagE(QString("note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
                     .arg(_ontime).arg(_channel).arg(_duration).arg(pitch()).arg(velo()));
                  break;

            case ME_NOTEON:
                  xml.tagE(QString("note-on  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(_ontime).arg(_channel).arg(pitch()).arg(velo()));
                  break;

            case ME_NOTEOFF:
                  xml.tagE(QString("note-off  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(_ontime).arg(_channel).arg(pitch()).arg(velo()));
                  break;

            case ME_CONTROLLER:
                  if (controller() == CTRL_PROGRAM) {
                        if ((_ontime == -1) && (_channel == 0)) {
                              xml.tagE(QString("program value=\"%1\"").arg(value()));
                              }
                        else {
                              xml.tagE(QString("program tick=\"%1\" channel=\"%2\" value=\"%3\"")
                                 .arg(ontime()).arg(channel()).arg(value()));
                              }
                        }
                  else {
                        if ((ontime() == -1) && (channel() == 0)) {
                              xml.tagE(QString("controller ctrl=\"%1\" value=\"%2\"")
                                 .arg(controller()).arg(value()));
                              }
                        else {
                              xml.tagE(QString("controller tick=\"%1\" channel=\"%2\" ctrl=\"%3\" value=\"%4\"")
                                 .arg(ontime()).arg(channel()).arg(controller()).arg(value()));
                              }
                        }
                  break;

            case ME_SYSEX:
                  xml.stag(QString("sysex tick=\"%1\" len=\"%2\"").arg(ontime()).arg(_len));
                  xml.dump(_len, _data);
                  xml.etag();
                  break;

            case ME_META:
                  switch(metaType()) {
                        case META_TRACK_NAME:
                              xml.tag(QString("TrackName tick=\"%1\"").arg(ontime()), QString((char*)(data())));
                              break;

                        case META_LYRIC:
                              xml.tag(QString("Lyric tick=\"%1\"").arg(ontime()), QString((char*)(data())));
                              break;

                        case META_KEY_SIGNATURE:
                              {
                              const char* keyTable[] = {
                                    "Ces", "Ges", "Des", "As", "Es", "Bes", "F",
                                    "C",
                                    "G", "D", "A", "E", "B", "Fis", "Cis"
                                    };
                              int key = (char)(_data[0]) + 7;
                              if (key < 0 || key > 14) {
                                    printf("bad key signature %d\n", key);
                                    key = 0;
                                    }
                              QString sex(_data[1] ? "Minor" : "Major");
                              QString keyName(keyTable[key]);
                              xml.tag(QString("Key tick=\"%1\" key=\"%2\" sex=\"%3\"").arg(ontime()).arg(_data[0]).arg(_data[1]),
                                 QString("%1 %2").arg(keyName).arg(sex));
                              }
                              break;

                        case META_TIME_SIGNATURE:
                              xml.tagE(QString("TimeSig tick=\"%1\" num=\"%2\" denom=\"%3\" metro=\"%4\" quarter=\"%5\"")
                                 .arg(ontime())
                                 .arg(int(_data[0]))
                                 .arg(int(_data[1]))
                                 .arg(int(_data[2]))
                                 .arg(int(_data[3])));
                              break;

                        case META_TEMPO:
                              {
                              unsigned tempo = _data[2] + (_data[1] << 8) + (_data[0] << 16);
                              xml.tagE(QString("Tempo tick=\"%1\" value=\"%2\"").arg(ontime()).arg(tempo));
                              }
                              break;

                        default:
                              xml.stag(QString("Meta tick=\"%1\" type=\"%2\" len=\"%3\" name=\"%4\"")
                                 .arg(ontime()).arg(metaType()).arg(_len).arg(midiMetaName(metaType())));
                              xml.dump(_len, _data);
                              xml.etag();
                              break;
                        }
                  break;
            }
      }

bool EventData::operator==(const EventData& e) const
      {
      return false;           // TODO
      }

//---------------------------------------------------------
//   EventData::write
//---------------------------------------------------------

void EventData::write(MidiFile* mf) const
      {
      switch(_type) {
            case ME_NOTEON:
                  mf->writeStatus(ME_NOTEON, channel());
                  mf->put(pitch());
                  mf->put(velo());
                  break;

            case ME_NOTEOFF:
                  mf->writeStatus(ME_NOTEOFF, channel());
                  mf->put(pitch());
                  mf->put(velo());
                  break;

            case ME_CONTROLLER:
                  switch(controller()) {
                        case CTRL_PROGRAM:
                              mf->writeStatus(ME_PROGRAM, channel());
                              mf->put(value() & 0x7f);
                              break;
                        case CTRL_PITCH:
                              {
                              mf->writeStatus(ME_PITCHBEND, channel());
                              int v = value() + 8192;
                              mf->put(v & 0x7f);
                              mf->put((v >> 7) & 0x7f);
                              }
                              break;
                        case CTRL_PRESS:
                              mf->writeStatus(ME_AFTERTOUCH, channel());
                              mf->put(value() & 0x7f);
                              break;
                        default:
                              mf->writeStatus(ME_CONTROLLER, channel());
                              mf->put(controller());
                              mf->put(value() & 0x7f);
                              break;
                        }
                  break;

            case ME_META:
                  mf->put(ME_META);
                  mf->put(_metaType);
                  mf->putvl(len());
                  mf->write(data(), len());
                  mf->resetRunningStatus();     // really ?!
                  break;

            case ME_SYSEX:
                  mf->put(ME_SYSEX);
                  mf->putvl(len() + 1);  // including 0xf7
                  mf->write(data(), len());
                  mf->put(ME_ENDSYSEX);
                  mf->resetRunningStatus();
                  break;
            }
      }

//---------------------------------------------------------
//   Event
//---------------------------------------------------------

Event::Event()
      {
      d = new EventData;
      }

Event::Event(const Event& s)
   : d(s.d)
      {
      }

Event::Event(int t)
      {
      d = new EventData(t);
      }

Event::~Event()
      {
      }

Event& Event::operator=(const Event& s)
      {
      d = s.d;
      return *this;
      }

void Event::write(MidiFile* mf) const { d->write(mf);  }
void Event::write(Xml& xml) const     { d->write(xml); }
// void Event::read(QDomElement e)       { d->read(e);    }

bool Event::isChannelEvent() const    { return d->isChannelEvent(); }

int Event::noquantOntime() const      { return d->_noquantOntime;       }
void Event::setNoquantOntime(int v)   { d->_noquantOntime = v;          }
int Event::noquantDuration() const    { return d->_noquantDuration;     }
void Event::setNoquantDuration(int v) { d->_noquantDuration = v;        }

int Event::type() const               { return d->_type;                }
void Event::setType(int v)            { d->_type = v;                   }
int Event::ontime() const             { return d->_ontime;              }
void Event::setOntime(int v)          { d->_ontime = v;                 }
int Event::channel() const            { return d->_channel;             }
void Event::setChannel(int c)         { d->_channel = c;                }
int Event::dataA() const              { return d->_a;                   }
int Event::dataB() const              { return d->_b;                   }
void Event::setDataA(int v)           { d->_a = v;                      }
void Event::setDataB(int v)           { d->_b = v;                      }
int Event::pitch() const              { return d->_a;                   }
void Event::setPitch(int v)           { d->_a = v;                      }
int Event::velo() const               { return d->_b;                   }
void Event::setVelo(int v)            { d->_b = v;                      }
int Event::controller() const         { return d->_a;                   }
void Event::setController(int val)    { d->_a = val;                    }
int Event::value() const              { return d->_b;                   }
void Event::setValue(int v)           { d->_b = v;                      }
int Event::duration() const           { return d->_duration;            }
void Event::setDuration(int v)        { d->_duration = v;               }
int Event::voice() const              { return d->_voice;               }
void Event::setVoice(int val)         { d->_voice = val;                }
int Event::offtime() const            { return d->ontime() + d->_duration; }
QList<Event>& Event::notes()          { return d->_notes;               }
const uchar* Event::data() const      { return d->_data;                }
void Event::setData(uchar* p)         { d->_data = p;                   }
int Event::len() const                { return d->_len;                 }
void Event::setLen(int l)             { d->_len = l;                    }
int Event::metaType() const           { return d->_metaType;            }
void Event::setMetaType(int v)        { d->_metaType = v;               }
int Event::tpc() const                { return d->_tpc;                 }
void Event::setTpc(int v)             { d->_tpc = v;                    }
const Note* Event::note() const       { return d->_note;                }
void Event::setNote(const Note* v)    { d->_note = v;                   }
double Event::tuning() const          { return d->_tuning;              }
void Event::setTuning(double v)       { d->_tuning = v;                 }
bool Event::operator==(const Event& e) const { return d->operator==(*e.d);   }

