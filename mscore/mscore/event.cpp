//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
#include "moevent.h"

//---------------------------------------------------------
//   NoteEvent::write
//---------------------------------------------------------

void NoteEvent::write(Xml& xml) const
      {
      xml.tagE(QString("note  tick=\"%1\" channel=\"%2\" len=\"%3\" pitch=\"%4\" velo=\"%5\"")
         .arg(ontime()).arg(channel()).arg(duration()).arg(pitch()).arg(velo()));
      }

//---------------------------------------------------------
//   NoteOn::write
//---------------------------------------------------------

void NoteOn::write(Xml& xml) const
      {
      xml.tagE(QString("note-on  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
            .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
      }

//---------------------------------------------------------
//   NoteOff::write
//---------------------------------------------------------

void NoteOff::write(Xml& xml) const
      {
      xml.tagE(QString("note-off  tick=\"%1\" channel=\"%2\" pitch=\"%3\" velo=\"%4\"")
            .arg(ontime()).arg(channel()).arg(pitch()).arg(velo()));
      }

//---------------------------------------------------------
//   ControllerEvent::write
//---------------------------------------------------------

void ControllerEvent::write(Xml& xml) const
      {
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
      }

//---------------------------------------------------------
//   SysexEvent::write
//---------------------------------------------------------

void SysexEvent::write(Xml& xml) const
      {
      xml.stag(QString("sysex tick=\"%1\" len=\"%2\"").arg(ontime()).arg(_len));
      xml.dump(_len, _data);
      xml.etag();
      }

//---------------------------------------------------------
//   MetaEvent::write
//---------------------------------------------------------

void MetaEvent::write(Xml& xml) const
      {
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
      }

//---------------------------------------------------------
//   NoteOn::write
//---------------------------------------------------------

void NoteOn::write(MidiFile* mf) const
      {
      mf->writeStatus(ME_NOTEON, channel());
      mf->put(pitch());
      mf->put(velo());
      }

//---------------------------------------------------------
//   NoteOff::write
//---------------------------------------------------------

void NoteOff::write(MidiFile* mf) const
      {
      mf->writeStatus(ME_NOTEOFF, channel());
      mf->put(pitch());
      mf->put(velo());
      }

//---------------------------------------------------------
//   ControllerEvent::write
//---------------------------------------------------------

void ControllerEvent::write(MidiFile* mf) const
      {
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
      }

//---------------------------------------------------------
//   midiOutEvent
//---------------------------------------------------------

bool ControllerEvent::midiOutEvent(QList<MidiOutEvent>* el, Score* cs) const
      {
      int port = cs->midiPort(channel());
      int ch   = cs->midiChannel(channel());
      switch(controller()) {
            case CTRL_PROGRAM:
                  {
                  int hb = (value() >> 16) & 0xff;
                  int lb = (value() >> 8) & 0xff;
                  int pr = value() & 0xff;
                  if (hb != 0xff) {
                        MidiOutEvent e;
                        e.port = port;
                        e.type = ME_CONTROLLER | ch;
                        e.a    = CTRL_HBANK;
                        e.b    = hb;
                        el->append(e);
                        }
                  if (lb != 0xff) {
                        MidiOutEvent e;
                        e.port = port;
                        e.type = ME_CONTROLLER | ch;
                        e.a    = CTRL_LBANK;
                        e.b    = lb;
                        el->append(e);
                        }
                  MidiOutEvent e;
                  e.port = port;
                  e.type = ME_PROGRAM | ch;
                  e.a    = pr;
                  e.b    = cs->midiMapping()->at(channel()).part->useDrumset();
                  el->append(e);
                  }
                  return true;
            case CTRL_PITCH:
                  {
                  MidiOutEvent e;
                  e.port = port;
                  e.type = ME_PITCHBEND | ch;
                  int v  = value() + 8192;
                  e.a    = v & 0x7f;
                  e.b    = (v >> 7) & 0x7f;
                  el->append(e);
                  }
                  return true;
            case CTRL_PRESS:
                  {
                  MidiOutEvent e;
                  e.port = port;
                  e.type = ME_AFTERTOUCH | ch;
                  e.a    = value();
                  el->append(e);
                  }
                  return true;
            default:
                  {
                  MidiOutEvent e;
                  e.port = port;
                  e.type = ME_CONTROLLER | ch;
                  e.a    = controller();
                  e.b    = value();
                  el->append(e);
                  }
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   MetaEvent::write
//---------------------------------------------------------

void MetaEvent::write(MidiFile* mf) const
      {
      mf->put(ME_META);
      mf->put(_metaType);
      mf->putvl(len());
      mf->write(data(), len());
      mf->resetRunningStatus();     // really ?!
      }

//---------------------------------------------------------
//   SysexEvent::write
//---------------------------------------------------------

void SysexEvent::write(MidiFile* mf) const
      {
      mf->put(ME_SYSEX);
      mf->putvl(len() + 1);  // including 0xf7
      mf->write(data(), len());
      mf->put(ME_ENDSYSEX);
      mf->resetRunningStatus();
      }


