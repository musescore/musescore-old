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

#include "editstaff.h"
#include "staff.h"
#include "part.h"
#include "editdrumset.h"
#include "score.h"
#include "measure.h"

//---------------------------------------------------------
//   EditStaff
//---------------------------------------------------------

EditStaff::EditStaff(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      setupUi(this);

      Part* part = staff->part();

      useDrumset->setChecked(part->useDrumset());
      editDrumset->setEnabled(part->useDrumset());
      lines->setValue(staff->lines());
      small->setChecked(staff->small());
      transposition->setValue(part->pitchOffset());
      shortName->setHtml(part->shortNameHtml());
      longName->setHtml(part->longNameHtml());
      slashStyle->setChecked(staff->slashStyle());

      connect(buttonBox, SIGNAL(clicked(QAbstractButton*)), this, SLOT(bboxClicked(QAbstractButton*)));
      connect(editDrumset, SIGNAL(clicked()), SLOT(editDrumsetClicked()));
      }

//---------------------------------------------------------
//   bboxClicked
//---------------------------------------------------------

void EditStaff::bboxClicked(QAbstractButton* button)
      {
      QDialogButtonBox::ButtonRole br = buttonBox->buttonRole(button);
      switch(br) {
            case QDialogButtonBox::ApplyRole:
                  apply();
                  break;

            case QDialogButtonBox::AcceptRole:
                  apply();
                  // fall through

            case QDialogButtonBox::RejectRole:
                  close();
                  break;

            default:
                  printf("EditStaff: unknown button %d\n", int(br));
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void EditStaff::apply()
      {
      Part* part = staff->part();

      part->setUseDrumset(useDrumset->isChecked());
      part->setPitchOffset(transposition->value());
      staff->setLines(lines->value());
      staff->setSmall(small->isChecked());
      staff->setSlashStyle(slashStyle->isChecked());
      part->setShortName(*shortName->document());
      part->setLongName(*longName->document());
      Score* score = staff->score();
      score->setInstrumentNames();
      MeasureBaseList* ml = score->measures();
      int staffIdx = score->staffIdx(staff);
      for (MeasureBase* mb = ml->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            MStaff* mstaff = static_cast<Measure*>(mb)->mstaff(staffIdx);
            mstaff->lines->setLines(staff->lines());
            }

      }

//---------------------------------------------------------
//   editDrumsetClicked
//---------------------------------------------------------

void EditStaff::editDrumsetClicked()
      {
      EditDrumset dse(staff->part()->drumset(), this);
      dse.exec();
      }

