//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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

#include "instrument.h"
#include "xml.h"
#include "drumset.h"
#include "articulation.h"
#include "utils.h"

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void NamedEventList::write(Xml& xml, const QString& n) const
      {
      xml.stag(QString("%1 name=\"%2\"").arg(n).arg(name));
      foreach(Event* e, events)
            e->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void NamedEventList::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "program") {
                  Event* ev = new Event(ME_CONTROLLER);
                  ev->setController(CTRL_PROGRAM);
                  ev->setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else if (tag == "controller") {
                  Event* ev = new Event(ME_CONTROLLER);
                  ev->setController(e.attribute("ctrl", "0").toInt());
                  ev->setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   operator
//---------------------------------------------------------

bool MidiArticulation::operator==(const MidiArticulation& i) const
      {
      return (i.name == name) && (i.velocity == velocity) && (i.gateTime == gateTime);
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      Channel a;
      a.name  = "normal";
      _channel.append(a);

      _minPitchA          = 0;
      _maxPitchA          = 127;
      _minPitchP          = 0;
      _maxPitchP          = 127;
      _drumset            = 0;
      _useDrumset         = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Instrument::write(Xml& xml) const
      {
      xml.stag("Instrument");
      if (_minPitchP > 0)
            xml.tag("minPitchP", _minPitchP);
      if (_maxPitchP < 127)
            xml.tag("maxPitchP", _maxPitchP);
      if (_minPitchA > 0)
            xml.tag("minPitchA", _minPitchA);
      if (_maxPitchA < 127)
            xml.tag("maxPitchA", _maxPitchA);
      if (_transpose.diatonic)
            xml.tag("transposeDiatonic", _transpose.diatonic);
      if (_transpose.chromatic)
            xml.tag("transposeChromatic", _transpose.chromatic);
      if (_useDrumset) {
            xml.tag("useDrumset", _useDrumset);
            _drumset->save(xml);
            }
      if (!_trackName.isEmpty())
            xml.tag("trackName", _trackName);
//      if (!_longName.isEmpty())
//            xml.tag("longName", _longName);
//      if (!_shortName.isEmpty())
//            xml.tag("shortName", _shortName);
      foreach(const NamedEventList& a, _midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel& a, _channel)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(QDomElement e)
      {
      int program = -1;
      int bank    = 0;
      int chorus = 30;
      int reverb = 30;
      int volume = 100;
      int pan    = 60;
      bool customDrumset = false;

      _channel.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "minPitch") {      // obsolete
                  _minPitchP = i;
                  _minPitchA = i;
                  }
            else if (tag == "maxPitch") {       // obsolete
                  _maxPitchP = i;
                  _maxPitchA = i;
                  }
            else if (tag == "minPitchA")
                  _minPitchA = i;
            else if (tag == "minPitchP")
                  _minPitchP = i;
            else if (tag == "maxPitchA")
                  _maxPitchA = i;
            else if (tag == "maxPitchP")
                  _maxPitchP = i;
            else if (tag == "trackName")
                  _trackName = val;
//            else if (tag == "longName")
//                  _longName = val;
//            else if (tag == "shortName")
//                  _shortName = val;
            else if (tag == "transposition") {    // obsolete
                  _transpose.chromatic = i;
                  _transpose.diatonic = chromatic2diatonic(i);
                  }
            else if (tag == "transposeChromatic")
                  _transpose.chromatic = i;
            else if (tag == "transposeDiatonic")
                  _transpose.diatonic = i;
            else if (tag == "useDrumset") {
                  _useDrumset = i;
                  if (_useDrumset)
                        _drumset = new Drumset(*smDrumset);
                  }
            else if (tag == "Drum") {
                  // if we see on of this tags, a custom drumset will
                  // be created
                  if (_drumset == 0)
                        _drumset = new Drumset(*smDrumset);
                  if (!customDrumset) {
                        _drumset->clear();
                        customDrumset = true;
                        }
                  _drumset->load(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  _midiActions.append(a);
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  _articulation.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel a;
                  a.read(e);
                  _channel.append(a);
                  }
            else if (tag == "chorus")     // obsolete
                  chorus = i;
            else if (tag == "reverb")     // obsolete
                  reverb = i;
            else if (tag == "midiProgram")  // obsolete
                  program = i;
            else if (tag == "volume")     // obsolete
                  volume = i;
            else if (tag == "pan")        // obsolete
                  pan = i;
            else if (tag == "midiChannel")      // obsolete
                  ;
            else
                  domError(e);
            }
      if (_channel.isEmpty()) {      // for backward compatibility
            Channel a;
            a.chorus  = chorus;
            a.reverb  = reverb;
            a.name    = "normal";
            a.program = program;
            a.bank    = bank;
            a.volume  = volume;
            a.pan     = pan;
            _channel.append(a);
            }
      if (_useDrumset) {
            if (_channel[0].bank == 0)
                  _channel[0].bank = 128;
            _channel[0].updateInitList();
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const QString& s) const
      {
      foreach(const NamedEventList& a, _midiActions) {
            if (s == a.name)
                  return const_cast<NamedEventList*>(&a);
            }
      return 0;
      }

//---------------------------------------------------------
//   Channel
//---------------------------------------------------------

Channel::Channel()
      {
      for(int i = 0; i < A_INIT_COUNT; ++i)
            init.append(0);
      channel  = -1;
      program  = -1;
      bank     = 0;
      volume   = 100;
      pan      = 64;
      chorus   = 30;
      reverb   = 30;

      mute     = false;
      solo     = false;
      soloMute = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Channel::write(Xml& xml) const
      {
      if (name.isEmpty())
            xml.stag("Channel");
      else
            xml.stag(QString("Channel name=\"%1\"").arg(name));
      updateInitList();
      foreach(Event* e, init) {
            if (e)
                  e->write(xml);
            }
      if (mute)
            xml.tag("mute", mute);
      if (solo)
            xml.tag("solo", solo);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Channel::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "program") {
                  program = e.attribute("value", "-1").toInt();
                  if (program == -1)
                        program = val.toInt();
                  }
            else if (tag == "controller") {
                  int value = e.attribute("value", "0").toInt();
                  int ctrl  = e.attribute("ctrl", "0").toInt();
                  switch(ctrl) {
                        case CTRL_HBANK:
                              bank = (value << 7) + (bank & 0x7f);
                              break;
                        case CTRL_LBANK:
                              bank = (bank & ~0x7f) + (value & 0x7f);
                              break;
                        case CTRL_VOLUME:
                              volume = value;
                              break;
                        case CTRL_PANPOT:
                              pan = value;
                              break;
                        case CTRL_CHORUS_SEND:
                              chorus = value;
                              break;
                        case CTRL_REVERB_SEND:
                              reverb = value;
                              break;
                        default:
                              {
                              Event* e = new Event(ME_CONTROLLER);
                              e->setController(ctrl);
                              e->setValue(value);
                              init.append(e);
                              }
                              break;
                        }
                  }
            else if (tag == "Articulation") {
                  MidiArticulation a;
                  a.read(e);
                  articulation.append(a);
                  }
            else
                  domError(e);
            }
      updateInitList();
      }

//---------------------------------------------------------
//   updateInitList
//---------------------------------------------------------

void Channel::updateInitList() const
      {
      for (int i = 0; i < A_INIT_COUNT; ++i) {
            // delete init[i];            // memory leak
            init[i] = 0;
            }
      Event* e;
      if (program != -1) {
            e = new Event(ME_CONTROLLER);
            e->setController(CTRL_PROGRAM);
            e->setValue(program);
            init[A_PROGRAM] = e;
            }

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_HBANK);
      e->setValue((bank >> 7) & 0x7f);
      init[A_HBANK] = e;

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_LBANK);
      e->setValue(bank & 0x7f);
      init[A_LBANK] = e;

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_VOLUME);
      e->setValue(volume);
      init[A_VOLUME] = e;

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_PANPOT);
      e->setValue(pan);
      init[A_PAN] = e;

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_CHORUS_SEND);
      e->setValue(chorus);
      init[A_CHORUS] = e;

      e = new Event(ME_CONTROLLER);
      e->setController(CTRL_REVERB_SEND);
      e->setValue(reverb);
      init[A_REVERB] = e;
      }

//---------------------------------------------------------
//   channelIdx
//---------------------------------------------------------

int Instrument::channelIdx(const QString& s) const
      {
      int idx = 0;
      foreach(const Channel& a, _channel) {
            if (a.name.isEmpty() && s == "normal")
                  return idx;
            if (s == a.name)
                  return idx;
            ++idx;
            }
      return -1;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiArticulation::write(Xml& xml) const
      {
      xml.stag(QString("Articulation name=\"%1\"").arg(name));
      xml.tag("velocity", velocity);
      xml.tag("gateTime", gateTime);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString text(e.text());
            if (tag == "velocity") {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  velocity = text.toInt();
                  }
            else if (tag == "gateTime") {
                  if (text.endsWith("%"))
                        text = text.left(text.size()-1);
                  gateTime = text.toInt();
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   updateVelocity
//---------------------------------------------------------

void Instrument::updateVelocity(int* velocity, int channelIdx, const QString& name)
      {
      const Channel& c = _channel[channelIdx];
      foreach(const MidiArticulation& a, c.articulation) {
            if (a.name == name) {
                  *velocity = *velocity * a.velocity / 100;
                  return;
                  }
            }
      foreach(const MidiArticulation& a, _articulation) {
            if (a.name == name) {
                  *velocity = *velocity * a.velocity / 100;
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   operator==
//---------------------------------------------------------

bool Instrument::operator==(const Instrument& i) const
      {
      return i._minPitchA == _minPitchA
         &&  i._maxPitchA == _maxPitchA
         &&  i._minPitchP == _minPitchP
         &&  i._maxPitchP == _maxPitchP
         &&  i._useDrumset == _useDrumset
         &&  i._midiActions == _midiActions
         &&  i._channel == _channel
         &&  i._articulation == _articulation
         &&  i._transpose.diatonic == _transpose.diatonic
         &&  i._transpose.chromatic == _transpose.chromatic
         ;
      }

//---------------------------------------------------------
//   setUseDrumset
//---------------------------------------------------------

void Instrument::setUseDrumset(bool val)
      {
      _useDrumset = val;
      if (val && _drumset == 0) {
            _drumset = new Drumset(*smDrumset);
            }
      }

//---------------------------------------------------------
//   setDrumset
//---------------------------------------------------------

void Instrument::setDrumset(Drumset* ds)
      {
      if (ds != _drumset) {
            _drumset = ds;
            }
      }

