//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: mscore.h,v 1.54 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

//---------------------------------------------------------
//   TimeDialog
//---------------------------------------------------------

TimeDialog::TimeDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: TimeSignature"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      QScrollArea* timePalette = new QScrollArea;
      l->addWidget(timePalette);
      sp = new SymbolPalette(4, 4);
      timePalette->setWidget(sp);
      sp->setGrid(60, 60);
      sp->showStaff(true);

      Score* cs = 0;
	sp->addObject(0,   new TimeSig(cs, 2, 2), "2/2");
	sp->addObject(1,   new TimeSig(cs, 4, 2), "2/4");
	sp->addObject(2,   new TimeSig(cs, 4, 3), "3/4");
	sp->addObject(3,   new TimeSig(cs, 4, 4), "4/4");
	sp->addObject(4,   new TimeSig(cs, 4, 5), "5/4");
	sp->addObject(5,   new TimeSig(cs, 4, 6), "6/4");
	sp->addObject(6,   new TimeSig(cs, 8, 3), "3/8");
	sp->addObject(7,   new TimeSig(cs, 8, 6), "6/8");
	sp->addObject(8,   new TimeSig(cs, 8, 9), "9/8");
	sp->addObject(9,   new TimeSig(cs, 8, 12), "12/8");
	sp->addObject(10,  new TimeSig(cs, TSIG_FOUR_FOUR), "4/4");
	sp->addObject(11,  new TimeSig(cs, TSIG_ALLA_BREVE), "3/4 alla breve");

      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
      {
      TimeSig* ts = new TimeSig(0, n->value(), z1->value(), z2->value(), z3->value(), z4->value());

      // look for free slot:

      int nr = sp->getRows();
      int nc = sp->getColumns();
      for (int r = 0; r < nr; ++r) {
            for (int c = 0; c < nc; ++c) {
                  int idx = r * nc + c;
                  Element* e = sp->element(idx);
                  if (e == 0) {
                        // free slot found
                        sp->addObject(idx, ts, "");
                        return;
                        }
                  }
            }
      // extend palette:
      sp->setRowsColumns(nr+1, nc);
      sp->addObject(nr * nc, ts, "");
      }

