//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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

#include "boxproperties.h"
#include "box.h"

//---------------------------------------------------------
//   BoxProperties
//---------------------------------------------------------

BoxProperties::BoxProperties(Box* b, QWidget* parent)
   : QDialog(parent)
      {
      _box = b;
      setupUi(this);

      leftMargin->setValue(b->leftMargin());
      rightMargin->setValue(b->rightMargin());
      topMargin->setValue(b->topMargin());
      bottomMargin->setValue(b->bottomMargin());
      connect(dialogButtonBox, SIGNAL(accepted()), SLOT(ok()));
      }

//---------------------------------------------------------
//   ok
//---------------------------------------------------------

void BoxProperties::ok()
      {
      _box->setLeftMargin(leftMargin->value());
      _box->setRightMargin(rightMargin->value());
      _box->setTopMargin(topMargin->value());
      _box->setBottomMargin(bottomMargin->value());
      }

