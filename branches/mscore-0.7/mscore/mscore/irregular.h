//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __IRREGULAR_H__
#define __IRREGULAR_H__

#include "ui_irregular.h"
#include "sig.h"

class Measure;

//---------------------------------------------------------
//   IrregularMeasureDialog
//---------------------------------------------------------

class IrregularMeasureDialog : public QDialog, private Ui::IrregularMeasureDialogBase {
      Q_OBJECT
      Measure* m;

   public:
      IrregularMeasureDialog(Measure*, QWidget* parent = 0);
      SigEvent sig() const;
      bool isIrregular() const;
      };

#endif

