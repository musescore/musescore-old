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

//---------------------------------------------------------
//   ChordProperties
//---------------------------------------------------------

ChordProperties::ChordProperties(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   small
//---------------------------------------------------------

bool ChordProperties::small() const
      {
      return smallCheckBox->isChecked();
      }

//---------------------------------------------------------
//   setSmall
//---------------------------------------------------------

void ChordProperties::setSmall(bool val)
      {
      smallCheckBox->setChecked(val);
      }

//---------------------------------------------------------
//   noStem
//---------------------------------------------------------

bool ChordProperties::noStem() const
      {
      return noStemCheckBox->isChecked();
      }

//---------------------------------------------------------
//   setNoStem
//---------------------------------------------------------

void ChordProperties::setNoStem(bool val)
      {
      noStemCheckBox->setChecked(val);
      }
