//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#ifndef __EDITSTYLE_H__
#define __EDITSTYLE_H__

#include "ui_editstyle.h"
#include "globals.h"
#include "style.h"

class Score;
class Style;

//---------------------------------------------------------
//   EditStyle
//---------------------------------------------------------

class EditStyle : public QDialog, private Ui::EditStyleBase {
      Q_OBJECT

      Score* cs;
      Style lstyle;    // local copy of style

      QButtonGroup* stemGroups[VOICES];

      void getValues();
      void setValues();

      void apply();

   private slots:
      void selectChordDescriptionFile();
      void buttonClicked(QAbstractButton*);
      void editEvenHeaderClicked();
      void editOddHeaderClicked();
      void editEvenFooterClicked();
      void editOddFooterClicked();

   public:
      EditStyle(Score*, QWidget*);
      };

#endif


