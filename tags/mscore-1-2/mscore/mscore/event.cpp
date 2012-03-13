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

//---------------------------------------------------------
//   Event::Event
//---------------------------------------------------------

Event::Event()
      {
      _data   = 0;
      _note   = 0;
      _tuning = 0.0;
      _ontime = -1;
      _channel = 0;
      }

Event::Event(int t)
      {
      _type   = t;
      _data   = 0;
      _note   = 0;
      _tuning = 0.0;
      _ontime = -1;
      _channel = 0;
      }


Event::Event(const Event& e)
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

Event::~Event()
      {
      delete[] _data;
      }

//---------------------------------------------------------
//   isChannelEvent
//---------------------------------------------------------

bool Event::isChannelEvent() const
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
//   Event::write
//---------------------------------------------------------

void Event::write(Xml& xml) const
      {
      switch(_type) {
            case ME_NOTE:
                  xml.tagE(QString("note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
                     .arg(ontime()).arg(channel()).arg(duration()).arg(pitch()).arg(velo()));
                  break;

            case ME_NOTEON:
                  xml.tagE(QString("note-on  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
                  break;

            case ME_NOTEOFF:
                  xml.tagE(QString("note-off  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
                     .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
                  break;

            case ME_CONTROLLER:
                  if (controller() == CTRL_PROGRAM) {
                        if ((ontime() == -1) && (channel() == 0)) {
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

//---------------------------------------------------------
//   Event::write
//---------------------------------------------------------

void Event::write(MidiFile* mf) const
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
                              mf->put(value());
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
                              mf->put(value());
                              break;
                        default:
                              mf->writeStatus(ME_CONTROLLER, channel());
                              mf->put(controller());
                              mf->put(value());
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

