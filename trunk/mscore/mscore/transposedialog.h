//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2009 Werner Schweer and others
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

#ifndef __TRANSPOSEDIALOG_H__
#define __TRANSPOSEDIALOG_H__

#include "ui_transposedialog.h"

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

class TransposeDialog : public QDialog, Ui::TransposeDialogBase {
      Q_OBJECT

   public:
      TransposeDialog(QWidget* parent = 0);
      void enableTransposeKeys(bool val)  { transposeKeys->setEnabled(val);       }
      bool getTransposeKeys() const       { return transposeKeys->isChecked();    }
      int getSemitones() const            { return semitones->value();            }
      bool getTransposeChordNames() const { return transposeChordNames->isChecked(); }
      };
#endif

