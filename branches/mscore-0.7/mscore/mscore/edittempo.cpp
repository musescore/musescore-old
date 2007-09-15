//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: edittempo.cpp,v 1.5 2006/03/02 17:08:33 wschweer Exp $
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

#include "edittempo.h"

struct TempoVal {
      const char* name;
      double bpm;
      };

static TempoVal tempos[] = {
      { "Adagio",         60 },
      { "Allegro",        80 },
      { "Allegretto",    100 },
      { "Allegro",       120 },
      { "Andante",        80 },
      { "Con brio",      120 },
      { "Con moto",      120 },
      { "Grave",          50 },
      { "Largo",          50 },
      { "Lento",          60 },
      { "Maestoso",       70 },
      { "Moderato",      100 },
      { "Prestissimo",   180 },
      { "Presto",        160 },
      { "Vivace",        120 },
      { "Vivo",          120 },
      { "Ballad",         60 },
      { "Fast",          120 },
      { "Lively",        120 },
      { "Moderate",      100 },
      { "Slow",           60 },
      { "Very slow",      40 },
      { "With movement", 120 },
      { "Entrainant",    120 },
      { "Lent",           60 },
      { "Rapide",        120 },
      { "Regulier",       80 },
      { "Vif",           120 },
      { "Vite",          120 },
      { "Vivement",      160 },
      { "Bewegt",        100 },
      { "Langsam",        60 },
      { "Lebhaft",       120 },
      { "Mäßig",         100 },
      { "Schnell",       120 },
      };

//---------------------------------------------------------
//   EditTempo
//---------------------------------------------------------

EditTempo::EditTempo(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      for (unsigned i = 0; i < sizeof(tempos)/sizeof(*tempos); ++i) {
            QListWidgetItem* item = new QListWidgetItem(tempos[i].name, tempoList);
            item->setData(Qt::UserRole, i);
            }
      selectTempo(3);
      connect(tempoList, SIGNAL(currentRowChanged(int)), SLOT(selectTempo(int)));
      connect(tempoList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), SLOT(itemDoubleClicked(QListWidgetItem*)));
      }

//---------------------------------------------------------
//   itemDoubleClicked
//---------------------------------------------------------

void EditTempo::itemDoubleClicked(QListWidgetItem* item)
      {
      int idx = item->data(Qt::UserRole).toInt();
      selectTempo(idx);
      accept();
      }

//---------------------------------------------------------
//   selectTempo
//---------------------------------------------------------

void EditTempo::selectTempo(int n)
      {
      if (n < 0 || n >= int(sizeof(tempos)/sizeof(*tempos)))
            return;
      _text = tempos[n].name;
      _bpm  = tempos[n].bpm;
      tempoText->setText(_text);
      tempoBPM->setValue(_bpm);
      }

