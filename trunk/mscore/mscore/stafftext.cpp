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

#include "score.h"
#include "stafftext.h"

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : Text(s)
      {
      _midiAction           = MIDI_ACTION_NO;
      _hbank                = -1;
      _lbank                = -1;
      _program              = 0;
      _controller           = 0;
      _controllerValue      = 0;

      setSubtype(TEXT_STAFF);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(Xml& xml) const
      {
      xml.stag("StaffText");
      switch(_midiAction) {
            case MIDI_ACTION_NO:
                  break;
            case MIDI_ACTION_PROGRAM_CHANGE:
                  xml.tagE(QString("midiProgramChange hbank=\"%1\" lbank=\"%2\" program=\"%3\"")
                     .arg(_hbank).arg(_lbank).arg(_program));
                  break;
            case MIDI_ACTION_CONTROLLER:
                  xml.tagE(QString("midiController controller=\"%1\" value=\"%2\"")
                     .arg(_controller).arg(_controllerValue));
                  break;
            case MIDI_ACTION_INSTRUMENT:
                  xml.tagE(QString("midiInstrumentAction name=\"%1\"").arg(_instrumentActionName));
                  break;
            }
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      _midiAction = MIDI_ACTION_NO;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "midiProgramChange") {
                  _hbank   = e.attribute("hbank", "-1").toInt();
                  _lbank   = e.attribute("lbank", "-1").toInt();
                  _program = e.attribute("program", "0").toInt();
                  _midiAction = MIDI_ACTION_PROGRAM_CHANGE;
                  }
            else if (tag == "midiController") {
                  _controller      = e.attribute("controller", "-1").toInt();
                  _controllerValue = e.attribute("value", "0").toInt();
                  _midiAction = MIDI_ACTION_CONTROLLER;
                  }
            else if (tag == "midiInstrumentAction") {
                  _instrumentActionName = e.attribute("name");
                  _midiAction = MIDI_ACTION_INSTRUMENT;
                  }
            else if (!Text::readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool StaffText::genPropertyMenu(QMenu* popup) const
      {
      Element::genPropertyMenu(popup);
      QAction* a = popup->addAction(tr("Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void StaffText::propertyAction(const QString& s)
      {
      if (s == "props") {
            StaffTextProperties rp(this);
            rp.exec();
            }
      else
            Element::propertyAction(s);
      }

//---------------------------------------------------------
//   StaffTextProperties
//---------------------------------------------------------

StaffTextProperties::StaffTextProperties(StaffText* st, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      staffText = st;

      MidiAction ma = st->midiAction();

      sendProgramChange->setChecked(ma == MIDI_ACTION_PROGRAM_CHANGE);
      sendControllerValue->setChecked(ma == MIDI_ACTION_CONTROLLER);
      instrumentDefinedAction->setChecked(ma == MIDI_ACTION_INSTRUMENT);

      int hbank, lbank, program;
      st->midiProgramChange(&hbank, &lbank, &program);
      midiBankSelectH->setValue(hbank + 1);
      midiBankSelectL->setValue(lbank + 1);
      midiProgram->setValue(program + 1);

      int controller, controllerValue;
      st->midiController(&controller, &controllerValue);
      midiController->setValue(controller + 1);
      midiControllerValue->setValue(controllerValue);

      midiActionName->setText(st->instrumentActionName());

      connect(this, SIGNAL(accepted()), SLOT(saveValues()));
      connect(sendProgramChange, SIGNAL(toggled(bool)), SLOT(typeProgramChanged(bool)));
      connect(sendControllerValue, SIGNAL(toggled(bool)), SLOT(typeControllerChanged(bool)));
      connect(instrumentDefinedAction, SIGNAL(toggled(bool)), SLOT(typeInstrumentChanged(bool)));
      }

//---------------------------------------------------------
//   saveValues
//---------------------------------------------------------

void StaffTextProperties::saveValues()
      {
      staffText->setMidiProgram(
         midiBankSelectH->value() - 1,
         midiBankSelectL->value() - 1,
         midiProgram->value() - 1
         );
      staffText->setMidiController(
         midiController->value() - 1,
         midiControllerValue->value()
         );
      staffText->setInstrumentActionName(midiActionName->text());

      if (sendProgramChange->isChecked())
            staffText->setMidiAction(MIDI_ACTION_PROGRAM_CHANGE);
      else if (sendControllerValue->isChecked())
            staffText->setMidiAction(MIDI_ACTION_CONTROLLER);
      else if (instrumentDefinedAction->isChecked())
            staffText->setMidiAction(MIDI_ACTION_INSTRUMENT);
      else
            staffText->setMidiAction(MIDI_ACTION_NO);
      }

//---------------------------------------------------------
//   typeProgramChanged
//---------------------------------------------------------

void StaffTextProperties::typeProgramChanged(bool val)
      {
      if (val) {
            sendControllerValue->setChecked(false);
            instrumentDefinedAction->setChecked(false);
            }
      }

//---------------------------------------------------------
//   typeControllerChanged
//---------------------------------------------------------

void StaffTextProperties::typeControllerChanged(bool val)
      {
      if (val) {
            sendProgramChange->setChecked(false);
            instrumentDefinedAction->setChecked(false);
            }
      }

//---------------------------------------------------------
//   typeInstrumentChanged
//---------------------------------------------------------

void StaffTextProperties::typeInstrumentChanged(bool val)
      {
      if (val) {
            sendProgramChange->setChecked(false);
            sendControllerValue->setChecked(false);
            }
      }


