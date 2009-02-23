//=============================================================================
//  MuseScore
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

#include "chordproperties.h"
#include "chord.h"

//---------------------------------------------------------
//   ChordProperties
//---------------------------------------------------------

ChordProperties::ChordProperties(Chord* c, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      chord = c;
      smallCheckBox->setChecked(chord->small());
      noStemCheckBox->setChecked(chord->noStem());
      extraLeadingSpace->setValue(chord->extraLeadingSpace().val());
      extraTrailingSpace->setValue(chord->extraTrailingSpace().val());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void ChordProperties::accept()
      {
      chord->setSmall(smallCheckBox->isChecked());
      chord->setNoStem(noStemCheckBox->isChecked());
      chord->setExtraLeadingSpace(Spatium(extraLeadingSpace->value()));
      chord->setExtraTrailingSpace(Spatium(extraTrailingSpace->value()));
      QDialog::accept();
      }

