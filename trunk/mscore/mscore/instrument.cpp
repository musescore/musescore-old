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

#include "instrument.h"
#include "xml.h"
#include "drumset.h"
#include "articulation.h"

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
                  ControllerEvent* ev = new ControllerEvent();
                  ev->setController(CTRL_PROGRAM);
                  ev->setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else if (tag == "controller") {
                  ControllerEvent* ev = new ControllerEvent();
                  ev->setController(e.attribute("ctrl", "0").toInt());
                  ev->setValue(e.attribute("value", "0").toInt());
                  events.append(ev);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      Channel* a      = new Channel();
      a->name         = "normal";
      channel.append(a);

      minPitchA        = 0;
      maxPitchA        = 127;
      minPitchP        = 0;
      maxPitchP        = 127;
      pitchOffset     = 0;
      drumset         = 0;
      useDrumset      = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Instrument::write(Xml& xml) const
      {
      xml.stag("Instrument");
      if (minPitchP > 0)
            xml.tag("minPitchP", minPitchP);
      if (maxPitchP < 127)
            xml.tag("maxPitchP", maxPitchP);
      if (minPitchA > 0)
            xml.tag("minPitchA", minPitchA);
      if (maxPitchA < 127)
            xml.tag("maxPitchA", maxPitchA);
      if (pitchOffset)
            xml.tag("transposition", pitchOffset);
      if (useDrumset) {
            xml.tag("useDrumset", useDrumset);
            drumset->save(xml);
            }
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Channel* a, channel)
            a->write(xml);
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

      foreach(Channel* a, channel)
            delete a;
      channel.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();

            if (tag == "minPitch") {      // obsolete
                  minPitchP = i;
                  minPitchA = i;
                  }
            else if (tag == "maxPitch") {       // obsolete
                  maxPitchP = i;
                  maxPitchA = i;
                  }
            else if (tag == "minPitchA")
                  minPitchA = i;
            else if (tag == "minPitchP")
                  minPitchP = i;
            else if (tag == "maxPitchA")
                  maxPitchA = i;
            else if (tag == "maxPitchP")
                  maxPitchP = i;
            else if (tag == "transposition")
                  pitchOffset = i;
            else if (tag == "useDrumset") {
                  useDrumset = i;
                  if (useDrumset)
                        drumset = new Drumset(*smDrumset);
                  }
            else if (tag == "Drum") {
                  // if we see on of this tags, a custom drumset will
                  // be created
                  if (drumset == 0)
                        drumset = new Drumset(*smDrumset);
                  if (!customDrumset) {
                        drumset->clear();
                        customDrumset = true;
                        }
                  drumset->load(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "Channel" || tag == "channel") {
                  Channel* a = new Channel();
                  a->read(e);
                  channel.append(a);
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
      if (channel.isEmpty()) {      // for backward compatibility
            Channel* a      = new Channel();
            a->chorus       = chorus;
            a->reverb       = reverb;
            a->name         = "normal";
            a->program      = program;
            a->bank         = bank;
            a->volume       = volume;
            a ->pan         = pan;
            channel.append(a);
            }
      if (useDrumset) {
            if (channel[0]->bank == 0)
                  channel[0]->bank = 128;
            channel[0]->updateInitList();
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList* Instrument::midiAction(const QString& s) const
      {
      foreach(const NamedEventList& a, midiActions) {
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
                              ControllerEvent* e = new ControllerEvent();
                              e->setController(ctrl);
                              e->setValue(value);
                              init.append(e);
                              }
                              break;
                        }
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
            if (init[i]) {
                  delete init[i];
                  init[i] = 0;
                  }
            }
      ControllerEvent* e;
      if (program != -1) {
            e = new ControllerEvent();
            e->setController(CTRL_PROGRAM);
            e->setValue(program);
            init[A_PROGRAM] = e;
            }

      e = new ControllerEvent();
      e->setController(CTRL_HBANK);
      e->setValue((bank >> 7) & 0x7f);
      init[A_HBANK] = e;

      e = new ControllerEvent();
      e->setController(CTRL_LBANK);
      e->setValue(bank & 0x7f);
      init[A_LBANK] = e;

      e = new ControllerEvent();
      e->setController(CTRL_VOLUME);
      e->setValue(volume);
      init[A_VOLUME] = e;

      e = new ControllerEvent();
      e->setController(CTRL_PANPOT);
      e->setValue(pan);
      init[A_PAN] = e;

      e = new ControllerEvent();
      e->setController(CTRL_CHORUS_SEND);
      e->setValue(chorus);
      init[A_CHORUS] = e;

      e = new ControllerEvent();
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
      foreach(const Channel* a, channel) {
            if (a->name.isEmpty() && s == "normal")
                  return idx;
            if (s == a->name)
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
      xml.stag(QString("Articulation name=\"%1\"").arg(Articulation::idx2name(idx)));
      xml.tag("velocity", velocity);
      xml.tag("gateTime", gateTime);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiArticulation::read(QDomElement e)
      {
      QString name = e.attribute("name");
      idx = Articulation::name2idx(name);
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "velocity")
                  velocity = e.text().toInt();
            else if (tag == "gateTime")
                  gateTime = e.text().toInt();
            else
                  domError(e);
            }
      }

