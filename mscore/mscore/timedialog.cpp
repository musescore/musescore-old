//=============================================================================
//  MusE Score
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

#include "timedialog.h"
#include "timesig.h"
#include "palette.h"
#include "mscore.h"
#include "score.h"

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

TimeDialog::TimeDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Time Signatures"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      sp = new Palette();
      PaletteScrollArea* timePalette = new PaletteScrollArea(sp);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      timePalette->setSizePolicy(policy);
      timePalette->setRestrictHeight(false);

      l->addWidget(timePalette);
      sp->setGrid(60, 60);

      Score* cs = gscore;
	sp->append(new TimeSig(cs, 2, 2), "2/2");
	sp->append(new TimeSig(cs, 4, 2), "2/4");
	sp->append(new TimeSig(cs, 4, 3), "3/4");
	sp->append(new TimeSig(cs, 4, 4), "4/4");
	sp->append(new TimeSig(cs, 4, 5), "5/4");
	sp->append(new TimeSig(cs, 4, 6), "6/4");
	sp->append(new TimeSig(cs, 8, 3), "3/8");
	sp->append(new TimeSig(cs, 8, 6), "6/8");
	sp->append(new TimeSig(cs, 8, 9), "9/8");
	sp->append(new TimeSig(cs, 8, 12), "12/8");
	sp->append(new TimeSig(cs, TSIG_FOUR_FOUR), "4/4 common time");
	sp->append(new TimeSig(cs, TSIG_ALLA_BREVE), "2/2 alla breve");

      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
      {
      TimeSig* ts = new TimeSig(gscore, n->value(), z1->value(), z2->value(), z3->value(), z4->value());
      // extend palette:
      sp->append(ts, "");
      }

