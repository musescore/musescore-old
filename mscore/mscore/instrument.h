//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: part.h,v 1.8 2006/03/13 21:35:59 wschweer Exp $
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

#ifndef __INSTRUMENT_H__
#define __INSTRUMENT_H__

#include "globals.h"

class Instrument;
class Xml;
class Staff;
class Score;
class Drumset;

//---------------------------------------------------------
//   MidiAction
//---------------------------------------------------------

class MidiAction {
   public:
      enum MidiActionType {
            ACTION_NO,               // no midi action
            ACTION_PROGRAM_CHANGE,   // send midi program change message
            ACTION_CONTROLLER,       // send midi controller message
            };

   private:
      QString _name;
      MidiActionType _type;
      union {
            struct {
                  int hbank, lbank, program;
                  };
            struct {
                  int ctrl, ctrlValue;
                  };
            };
   public:
      MidiAction();
      void setProgram(int hb, int lb, int pr);
      void setController(int ctrl, int value);
      bool programChange(int* hbank, int* lbank, int* program);
      bool controller(int* controller, int* controllerValue);
      void write(Xml&) const;
      void read(QDomElement);
      MidiActionType type()          { return _type; }
      QString name() const           { return _name; }
      void setName(const QString& n) { _name = n; }
      };

//---------------------------------------------------------
//   Instrument
//---------------------------------------------------------

struct Instrument {
      char midiChannel;
      char midiPort;
      char midiProgram;
      char midiBankSelectH;
      char midiBankSelectL;
      char volume;
      char pan;
      char chorus;
      char reverb;
      char minPitch;
      char maxPitch;
      char pitchOffset;

      bool mute;
      bool solo;
      bool soloMute;
      Drumset* drumset;
      bool useDrumset;

      QList<MidiAction> midiActions;

      Instrument();
      void read(QDomElement);
      void write(Xml& xml) const;
      MidiAction midiAction(const QString& s) const;
      };

#endif

