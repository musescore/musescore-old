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
      Articulation* a = new Articulation();
      a->name         = "normal";
      articulations.append(a);

      minPitch        = 0;
      maxPitch        = 127;
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
      if (minPitch > 0)
            xml.tag("minPitch", minPitch);
      if (maxPitch < 127)
            xml.tag("maxPitch", maxPitch);
      if (pitchOffset)
            xml.tag("transposition", pitchOffset);
      if (useDrumset) {
            xml.tag("useDrumset", useDrumset);
            drumset->save(xml);
            }
      foreach(const NamedEventList& a, midiActions)
            a.write(xml, "MidiAction");
      foreach(const Articulation* a, articulations)
            a->write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(QDomElement e)
      {
      foreach(Articulation* a, articulations)
            delete a;
      articulations.clear();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "minPitch")
                  minPitch = i;
            else if (tag == "maxPitch")
                  maxPitch = i;
            else if (tag == "transposition")
                  pitchOffset = i;
            else if (tag == "useDrumset") {
                  useDrumset = i;
                  if (useDrumset)
                        drumset = new Drumset(*smDrumset);
                  }
            else if (tag == "Drum") {
                  if (drumset == 0)
                        drumset = new Drumset(*smDrumset);
                  drumset->load(e);
                  }
            else if (tag == "MidiAction") {
                  NamedEventList a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else if (tag == "Articulation") {
                  Articulation* a = new Articulation();
                  a->read(e);
                  articulations.append(a);
                  }
            else if (tag == "chorus")     // obsolete
                  ;
            else if (tag == "reverb")     // obsolete
                  ;
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

NamedEventList Instrument::midiAction(const QString& s) const
      {
      foreach(const NamedEventList& a, midiActions) {
            if (s == a.name)
                  return a;
            }
      return NamedEventList();
      }

//---------------------------------------------------------
//   Articulation
//---------------------------------------------------------

Articulation::Articulation()
      {
      for(int i = 0; i < A_INIT_COUNT; ++i)
            init.append(0);
      channel  = -1;
      program  = 0;
      hbank    = -1;
      lbank    = -1;
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

void Articulation::write(Xml& xml) const
      {
      if (name.isEmpty())
            xml.stag("Articulation");
      else
            xml.stag(QString("Articulation name=\"%1\"").arg(name));
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

void Articulation::read(QDomElement e)
      {
      name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "program") {
                  program = e.attribute("value", "-1").toInt();
                  if (program == -1)
                        program = val.toInt();
                  ControllerEvent* e = new ControllerEvent();
                  e->setController(CTRL_PROGRAM);
                  e->setValue(program);
                  init[A_PROGRAM] = e;
                  }
            else if (tag == "controller") {
                  int value = e.attribute("value", "0").toInt();
                  int ctrl  = e.attribute("ctrl", "0").toInt();
                  ControllerEvent* e = new ControllerEvent();
                  e->setController(ctrl);
                  e->setValue(value);
                  switch(ctrl) {
                        case CTRL_HBANK:
                              init[A_HBANK] = e;
                              hbank = value;
                              break;
                        case CTRL_LBANK:
                              init[A_LBANK] = e;
                              lbank = value;
                              break;
                        case CTRL_VOLUME:
                              init[A_VOLUME] = e;
                              volume = value;
                              break;
                        case CTRL_PANPOT:
                              pan = value;
                              init[A_PAN] = e;
                              break;
                        case CTRL_CHORUS_SEND:
                              chorus = value;
                              init[A_CHORUS] = e;
                              break;
                        case CTRL_REVERB_SEND:
                              reverb = value;
                              init[A_REVERB] = e;
                              break;
                        default:
                              init.append(e);
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

void Articulation::updateInitList() const
      {
      for (int i = 0; i < A_INIT_COUNT; ++i) {
            if (init[i]) {
                  delete init[i];
                  init[i] = 0;
                  }
            }
      if (program) {
            ControllerEvent* e = new ControllerEvent();
            e->setController(CTRL_PROGRAM);
            e->setValue(program);
            init[A_PROGRAM] = e;
            }
      if (hbank != -1) {
            ControllerEvent* e = new ControllerEvent();
            e->setController(CTRL_HBANK);
            e->setValue(hbank);
            init[A_HBANK] = e;
            }
      if (lbank != -1) {
            ControllerEvent* e = new ControllerEvent();
            e->setController(CTRL_LBANK);
            e->setValue(lbank);
            init[A_LBANK] = e;
            }
      ControllerEvent* e = new ControllerEvent();
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

