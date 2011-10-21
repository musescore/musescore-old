//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include "restproperties.h"
#include "libmscore/rest.h"

//---------------------------------------------------------
//   RestProperties
//---------------------------------------------------------

RestProperties::RestProperties(Rest* r, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      rest = r;
      smallCheckBox->setChecked(r->small());
      extraLeadingSpace->setValue(rest->extraLeadingSpace().val());
      extraTrailingSpace->setValue(rest->extraTrailingSpace().val());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void RestProperties::accept()
      {
qDebug("rest accept %f %f\n", extraLeadingSpace->value(), extraTrailingSpace->value());
      rest->setSmall(smallCheckBox->isChecked());
      rest->setExtraLeadingSpace(Spatium(extraLeadingSpace->value()));
      rest->setExtraTrailingSpace(Spatium(extraTrailingSpace->value()));
      QDialog::accept();
      }
