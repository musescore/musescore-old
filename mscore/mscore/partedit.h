//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: partedit.h,v 1.2 2006/03/02 17:08:40 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __ILEDIT_H__
#define __ILEDIT_H__

#include "ui_partedit.h"

class Score;
class Part;

//---------------------------------------------------------
//   PartEdit
//---------------------------------------------------------

class PartEdit : public QWidget, public Ui::PartEditBase {
      Q_OBJECT

      Part* part;

   private slots:
      void patchChanged(int);
      void volChanged(int);
      void panChanged(int);
      void reverbChanged(int);
      void chorusChanged(int);
      void muteChanged(bool);
      void soloChanged(bool);
      void showPartChanged(bool);
      void channelChanged(int);
      void minPitchChanged(int);
      void maxPitchChanged(int);
      void partNameChanged(const QString&);
      void shortNameChanged(const QString&);
      void longNameChanged(const QString&);

   public:
      PartEdit();
      void setPart(Part*);
      };

//---------------------------------------------------------
//   InstrumentListEditor
//---------------------------------------------------------

class InstrumentListEditor : public QScrollArea
      {
      Q_OBJECT
      QScrollArea* sa;
      QVBoxLayout* vb;

   public:
      InstrumentListEditor(QWidget* parent);
      void updateAll(Score*);
      };

#endif

