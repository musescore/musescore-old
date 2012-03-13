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

extern bool useFactorySettings;

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
      sp->setReadOnly(false);
      connect(sp, SIGNAL(changed()), SLOT(setDirty()));
      
      PaletteScrollArea* timePalette = new PaletteScrollArea(sp);
      QSizePolicy policy(QSizePolicy::Expanding, QSizePolicy::Expanding);
      timePalette->setSizePolicy(policy);
      timePalette->setRestrictHeight(false);
            
      l->addWidget(timePalette);
      sp->setGrid(60, 60);

      _dirty = false;
      connect(addButton, SIGNAL(clicked()), SLOT(addClicked()));

      if (!useFactorySettings) {
            QFile f(dataPath + "/" + "timesigs.xml");
            if (f.exists() && sp->read(&f))
                  return;
            }
      //
      // create default palette
      //
	sp->append(new TimeSig(gscore, 2, 2), "2/2");
	sp->append(new TimeSig(gscore, 4, 2), "2/4");
	sp->append(new TimeSig(gscore, 4, 3), "3/4");
	sp->append(new TimeSig(gscore, 4, 4), "4/4");
	sp->append(new TimeSig(gscore, 4, 5), "5/4");
	sp->append(new TimeSig(gscore, 4, 6), "6/4");
	sp->append(new TimeSig(gscore, 8, 3), "3/8");
	sp->append(new TimeSig(gscore, 8, 6), "6/8");
	sp->append(new TimeSig(gscore, 8, 9), "9/8");
	sp->append(new TimeSig(gscore, 8, 12), "12/8");
	sp->append(new TimeSig(gscore, TSIG_FOUR_FOUR), tr("4/4 common time"));
	sp->append(new TimeSig(gscore, TSIG_ALLA_BREVE), tr("2/2 alla breve"));
      }

//---------------------------------------------------------
//   addClicked
//---------------------------------------------------------

void TimeDialog::addClicked()
      {
      TimeSig* ts = new TimeSig(gscore, n->value(), z1->value(), z2->value(), z3->value(), z4->value());
      // extend palette:
      sp->append(ts, "");
      _dirty = true;
      }

//---------------------------------------------------------
//   save
//---------------------------------------------------------

void TimeDialog::save()
      {
      QDir dir;
      dir.mkpath(dataPath);
      sp->write(dataPath + "/" + "timesigs.xml");
      }

