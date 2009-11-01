//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __PIANOROLL_H__
#define __PIANOROLL_H__

class Score;
class Staff;
class PianoView;
class Note;
class Ruler;
class Seq;

#include "al/pos.h"

namespace Awl {
      class PitchEdit;
      class PosLabel;
      };

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

class PianorollEditor : public QDialog {
      Q_OBJECT

      PianoView* gv;
      Score* _score;
      Staff* staff;
      Awl::PitchEdit* pitch;
      QSpinBox* velocity;
      AL::Pos locator[3];
      QComboBox* veloType;
      Awl::PosLabel* pos;
      Ruler* ruler;

      void updateVelocity(Note* note);
      void updateSelection();

   private slots:
      void selectionChanged();
      void veloTypeChanged(int);
      void velocityChanged(int);
      void keyPressed(int);
      void keyReleased(int);
      void moveLocator(int);
      void cmd(QAction*);

   public slots:
      void changeSelection(int);

   public:
      PianorollEditor(QWidget* parent = 0);
      void setStaff(Staff* staff);
      Score* score() const { return _score; }
      void heartBeat(Seq*);
      };

#endif


