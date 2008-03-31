//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#ifndef __STAFFTEXT_H__
#define __STAFFTEXT_H__

#include "text.h"
#include "ui_stafftext.h"

//---------------------------------------------------------
//   MidiAction
//---------------------------------------------------------

enum MidiAction {
      MIDI_ACTION_NO,               // no midi action
      MIDI_ACTION_PROGRAM_CHANGE,   // send midi program change message
      MIDI_ACTION_CONTROLLER,       // send midi controller message
      MIDI_ACTION_INSTRUMENT        // instrument defines action
      };

//---------------------------------------------------------
//   StaffText
//---------------------------------------------------------

class StaffText : public Text  {
      enum MidiAction _midiAction;
      int _hbank, _lbank, _program;
      int _controller, _controllerValue;
      QString _instrumentActionName;

   public:
      StaffText(Score*);
      virtual StaffText* clone() const { return new StaffText(*this); }
      virtual ElementType type() const { return STAFF_TEXT; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      virtual bool genPropertyMenu(QMenu*) const;
      virtual void propertyAction(const QString&);
      void setMidiProgram(int hb, int lb, int pr) {
            _hbank = hb;
            _lbank = lb;
            _program = pr;
            _midiAction = MIDI_ACTION_PROGRAM_CHANGE;
            }
      void setMidiController(int ctrl, int value) {
            _controller = ctrl;
            _controllerValue = value;
            _midiAction = MIDI_ACTION_CONTROLLER;
            }
      MidiAction midiAction() const               { return _midiAction; }
      void setMidiAction(MidiAction a)            { _midiAction = a; }
      void setInstrumentActionName(const QString& s) {
            _instrumentActionName = s;
            _midiAction = MIDI_ACTION_INSTRUMENT;
            }
      QString instrumentActionName() const        { return _instrumentActionName; }
      void midiProgramChange(int* hb, int* lb, int* pr) const {
            *hb = _hbank;
            *lb = _lbank;
            *pr = _program;
            }
      void midiController(int* ctrl, int* value) {
            *ctrl = _controller;
            *value = _controllerValue;
            }
      };

//---------------------------------------------------------
//   StaffTextProperties
//    Dialog
//---------------------------------------------------------

class StaffTextProperties : public QDialog, public Ui::StaffTextProperties {
      Q_OBJECT

      StaffText* staffText;

   private slots:
      void saveValues();
      void typeProgramChanged(bool);
      void typeControllerChanged(bool);
      void typeInstrumentChanged(bool);

   public:
      StaffTextProperties(StaffText*, QWidget* parent = 0);
      };

#endif
