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
//   MidiAction
//---------------------------------------------------------

MidiAction::MidiAction()
      {
      _type = ACTION_NO;
      }

//---------------------------------------------------------
//   setProgram
//---------------------------------------------------------

void MidiAction::setProgram(int hb, int lb, int pr)
      {
      hbank   = hb;
      lbank   = lb;
      program = pr;
      _type    = ACTION_PROGRAM_CHANGE;
      }

//---------------------------------------------------------
//   setController
//---------------------------------------------------------

void MidiAction::setController(int c, int v)
      {
      ctrl      = c;
      ctrlValue = v;
      _type     = ACTION_CONTROLLER;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void MidiAction::write(Xml& xml) const
      {
      if (_type == ACTION_NO)
            return;

      if (!_name.isEmpty())
            xml.stag(QString("MidiAction name=\"%1\"").arg(_name));
      else
            xml.stag("MidiAction");
      switch(_type) {
            case ACTION_PROGRAM_CHANGE:
                  xml.tagE(QString("programChange hbank=\"%1\" lbank=\"%2\" program=\"%3\"")
                     .arg(hbank).arg(lbank).arg(program));
                  break;
            case ACTION_CONTROLLER:
                  xml.tagE(QString("controller controller=\"%1\" value=\"%2\"")
                     .arg(ctrl).arg(ctrlValue));
                  break;
            case ACTION_NO:
                  break;
            }
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void MidiAction::read(QDomElement e)
      {
      _type = ACTION_NO;
      _name = e.attribute("name");
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "programChange") {
                  _type = ACTION_PROGRAM_CHANGE;
                  hbank   = e.attribute("hbank", "-1").toInt();
                  lbank   = e.attribute("lbank", "-1").toInt();
                  program = e.attribute("program", "0").toInt();
                  }
            else if (tag == "controller") {
                  _type = ACTION_CONTROLLER;
                  ctrl  = e.attribute("controller", "0").toInt();
                  ctrlValue = e.attribute("controller", "0").toInt();
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   programChange
//---------------------------------------------------------

bool MidiAction::programChange(int* hb, int* lb, int* pr)
      {
      if (_type != ACTION_PROGRAM_CHANGE)
            return false;
      *hb = hbank;
      *lb = lbank;
      *pr = program;
      return true;
      }

//---------------------------------------------------------
//   controller
//---------------------------------------------------------

bool MidiAction::controller(int* c, int* v)
      {
      if (_type != ACTION_CONTROLLER)
            return false;
      *c  = ctrl;
      *v  = ctrlValue;
      return true;
      }

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

Instrument::Instrument()
      {
      midiChannel     = 0;
      midiPort        = 0;
      midiProgram     = 0;
      midiBankSelectH = -1;
      midiBankSelectL = -1;
      volume          = 100;
      pan             = 64;
      chorus          = 30;
      reverb          = 30;
      mute            = false;
      solo            = false;
      soloMute        = false;
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
      if (midiChannel != 0)
            xml.tag("midiChannel", midiChannel);
      if (midiPort != 0)
            xml.tag("midiPort", midiPort);
      if (midiProgram != 0)
            xml.tag("midiProgram", midiProgram);
      if (midiBankSelectH != -1)
            xml.tag("midiBankSelectH", midiBankSelectH);
      if (midiBankSelectL != -1)
            xml.tag("midiBankSelectL", midiBankSelectL);
      if (volume != 100)
            xml.tag("volume", volume);
      if (pan != 64)
            xml.tag("pan", pan);
      if (chorus)
            xml.tag("chorus", chorus);
      if (reverb)
            xml.tag("reverb", reverb);
      if (mute)
            xml.tag("mute", mute);
      if (solo)
            xml.tag("solo", solo);
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
      foreach(const MidiAction& a, midiActions)
            a.write(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Instrument::read
//---------------------------------------------------------

void Instrument::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "midiChannel")
                  midiChannel = i;
            else if (tag == "midiPort")
                  midiPort = i;
            else if (tag == "midiProgram")
                  midiProgram = i;
            else if (tag == "midiBankSelectH")
                  midiBankSelectH = i;
            else if (tag == "midiBankSelectL")
                  midiBankSelectL = i;
            else if (tag == "volume")
                  volume = i;
            else if (tag == "pan")
                  pan = i;
            else if (tag == "chorus")
                  chorus = i;
            else if (tag == "reverb")
                  reverb = i;
            else if (tag == "mute")
                  mute = i;
            else if (tag == "solo")
                  solo = i;
            else if (tag == "minPitch")
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
                  MidiAction a;
                  a.read(e);
                  midiActions.append(a);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   action
//---------------------------------------------------------

MidiAction Instrument::midiAction(const QString& s) const
      {
      foreach(const MidiAction& a, midiActions) {
            if (s == a.name())
                  return a;
            }
      return MidiAction();
      }

