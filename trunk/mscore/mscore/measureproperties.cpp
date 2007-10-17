//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer and others
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

#include "measureproperties.h"
#include "measure.h"
#include "sig.h"
#include "score.h"
#include "repeat.h"

//---------------------------------------------------------
//   MeasureProperties
//---------------------------------------------------------

MeasureProperties::MeasureProperties(Measure* _m, QWidget* parent)
   : QDialog(parent)
      {
      m = _m;
      setupUi(this);
      const SigEvent ev(m->score()->sigmap->timesig(m->tick()));

      actualZ->setValue(ev.nominator);
      actualN->setValue(ev.denominator);
      nominalZ->setValue(ev.nominator2);
      nominalN->setValue(ev.denominator2);
      irregular->setChecked(m->irregular());
      int n  = m->repeatCount();
      count->setValue(n);
      count->setEnabled(m->repeatFlags() & RepeatEnd);
      }

//---------------------------------------------------------
//   sig
//---------------------------------------------------------

SigEvent MeasureProperties::sig() const
      {
      SigEvent e(actualZ->value(), actualN->value(),
         nominalZ->value(), nominalN->value());
      return e;
      }

//---------------------------------------------------------
//   isIrregular
//---------------------------------------------------------

bool MeasureProperties::isIrregular() const
      {
      return irregular->isChecked();
      }

//---------------------------------------------------------
//   repeatCount
//---------------------------------------------------------

int MeasureProperties::repeatCount() const
      {
      return count->value();
      }

