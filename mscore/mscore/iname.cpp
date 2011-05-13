//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "score.h"
#include "iname.h"
#include "editstaff.h"

//---------------------------------------------------------
//   InstrumentName
//---------------------------------------------------------

InstrumentName::InstrumentName(Score* s)
   : Text(s)
      {
      _layoutPos = 0;
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool InstrumentName::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Staff Properties..."));
      a->setData("sprops");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void InstrumentName::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "sprops") {
            EditStaff editStaff(staff(), 0);
            editStaff.exec();
            }
      else
            Text::propertyAction(viewer, s);
      }

