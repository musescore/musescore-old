//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: select.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

/**
 \file
 Implementation of class Selection plus other selection related functions.
*/

#include "libmscore/select.h"
#include "selectdialog.h"
#include "globals.h"
#include "scoreview.h"
#include "libmscore/note.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/score.h"
#include "libmscore/slur.h"
#include "libmscore/system.h"
#include "al/sig.h"
#include "libmscore/utils.h"
#include "libmscore/text.h"
#include "libmscore/segment.h"
#include "input.h"
#include "libmscore/measure.h"
#include "libmscore/page.h"
#include "libmscore/barline.h"
#include "libmscore/xml.h"
#include "libmscore/lyrics.h"
#include "limits.h"
#include "libmscore/tuplet.h"
#include "libmscore/beam.h"
#include "libmscore/textline.h"

//---------------------------------------------------------
//   SelectDialog
//---------------------------------------------------------

SelectDialog::SelectDialog(const Element* _e, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      e = _e;
      type->setText(e->name());
      }

//---------------------------------------------------------
//   setPattern
//---------------------------------------------------------

void SelectDialog::setPattern(ElementPattern* p)
      {
      p->type    = e->type();
      p->subtype = e->subtype();
      p->staff   = sameStaff->isChecked() ? e->staffIdx() : -1;
      p->voice   = sameVoice->isChecked() ? e->voice() : -1;
      p->system  = 0;
      if (sameSystem->isChecked()) {
            do {
                  if (e->type() == SYSTEM) {
                        p->system = static_cast<const System*>(e);
                        break;
                        }
                  e = e->parent();
                  } while (e);
            }
      }

