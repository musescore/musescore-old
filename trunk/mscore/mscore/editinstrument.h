//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: editinstrument.h,v 1.4 2006/03/02 17:08:33 wschweer Exp $
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

#ifndef __EDITINSTRUMENT_H__
#define __EDITINSTRUMENT_H__

#include "ui_editinstrument.h"

class InstrumentTemplate;

//---------------------------------------------------------
//   EditInstrument
//---------------------------------------------------------

class EditInstrument : public QDialog, private Ui::EditInstrumentBase
      {
      Q_OBJECT

      QList<InstrumentTemplate*> templates;

   private slots:
      void on_instrumentList_itemSelectionChanged();
      void on_buttonCancel_pressed();
      void on_buttonOk_pressed();
      void on_buttonNew_pressed();

   public:
      EditInstrument(QWidget* parent = 0);
      ~EditInstrument();
      void setCurrentInstrument(const InstrumentTemplate* instr);
      void update();
      };

#endif

