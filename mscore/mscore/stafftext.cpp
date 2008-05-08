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
#include "system.h"

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

StaffText::StaffText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_STAFF);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void StaffText::write(Xml& xml) const
      {
      xml.stag("StaffText");
      if (!_instrumentActionName.isEmpty())
            xml.tagE(QString("midiInstrumentAction name=\"%1\"").arg(_instrumentActionName));
      else
            _midiAction.write(xml);
      Text::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffText::read(QDomElement e)
      {
      _midiAction = MidiAction();
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "MidiAction")
                  _midiAction.read(e);
            else if (tag == "midiInstrumentAction")
                  _instrumentActionName = e.attribute("name");
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
      if (!st->instrumentActionName().isEmpty()) {
            instrumentDefinedAction->setChecked(true);
            midiActionName->setText(st->instrumentActionName());
            }
      else {
            instrumentDefinedAction->setChecked(false);
            switch (ma.type()) {
                  case MidiAction::ACTION_PROGRAM_CHANGE:
                        {
                        sendProgramChange->setChecked(true);
                        sendControllerValue->setChecked(false);
                        int hbank, lbank, program;
                        ma.programChange(&hbank, &lbank, &program);
                        midiBankSelectH->setValue(hbank + 1);
                        midiBankSelectL->setValue(lbank + 1);
                        midiProgram->setValue(program + 1);
                        }
                        break;
                  case MidiAction::ACTION_CONTROLLER:
                        {
                        sendControllerValue->setChecked(true);
                        sendProgramChange->setChecked(false);
                        int controller, controllerValue;
                        ma.controller(&controller, &controllerValue);
                        midiController->setValue(controller + 1);
                        midiControllerValue->setValue(controllerValue);
                        }
                        break;
                  default:
                        sendControllerValue->setChecked(false);
                        sendProgramChange->setChecked(false);
                        break;
                  }
            }

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
      if (sendProgramChange->isChecked()) {
            staffText->setMidiProgram(
               midiBankSelectH->value() - 1,
               midiBankSelectL->value() - 1,
               midiProgram->value() - 1
               );
            }
      else if (sendControllerValue->isChecked()) {
            staffText->setMidiController(
               midiController->value() - 1,
               midiControllerValue->value()
               );
            }
      else if (instrumentDefinedAction->isChecked()) {
            staffText->setInstrumentActionName(midiActionName->text());
            }
      else
            staffText->setMidiAction(MidiAction());
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

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffText::layout(ScoreLayout* l)
      {
      Text::layout(l);
      Measure* m = (Measure*)parent();
      double y = track() != -1 ? m->system()->staff(track() / VOICES)->y() : 0.0;
      double x = (tick() != -1) ? m->tick2pos(tick()) : 0.0;
      setPos(ipos() + QPointF(x, y));
      }


