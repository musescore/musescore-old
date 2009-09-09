//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: select.h 2047 2009-08-26 18:33:38Z wschweer $
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SYNTHCONTROL_H__
#define __SYNTHCONTROL_H__

#include "ui_synthcontrol.h"

class Synth;

//---------------------------------------------------------
//   SynthControl
//---------------------------------------------------------

class SynthControl : public QWidget, Ui::SynthControl {
      Q_OBJECT

      Synth* synth;

      virtual void closeEvent(QCloseEvent*);

   private slots:
      void selectSoundFont();
      void masterGainChanged(double, int);
      void masterTuningChanged(double);
      void reverbValueChanged(double val, int idx);
      void chorusValueChanged(double val, int idx);

   signals:
      void closed();

   public:
      Synth* getSynth() const { return synth; }
      SynthControl(Synth*, QWidget* parent);
      void updatePreferences();
      void heartBeat(Synth*);
      void stop();
      };

#endif

