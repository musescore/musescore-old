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

#include "edittempo.h"

struct TempoVal {
      QString name;
      double bpm;

      TempoVal(const TempoVal& v) {
            name = v.name;
            bpm  = v.bpm;
            }
      TempoVal(const char* s, int t) {
            name = QString::fromUtf8(s);
            bpm  = t;
            }
      };

static TempoVal tempos[] = {
      TempoVal("Adagio",         60),
      TempoVal("Allegro",        80),
      TempoVal("Allegretto",    100),
      TempoVal("Allegro",       120),
      TempoVal("Andante",        80),
      TempoVal("Con brio",      120),
      TempoVal("Con moto",      120),
      TempoVal("Grave",          50),
      TempoVal("Largo",          50),
      TempoVal("Lento",          60),
      TempoVal("Maestoso",       70),
      TempoVal("Moderato",      100),
      TempoVal("Prestissimo",   180),
      TempoVal("Presto",        160),
      TempoVal("Vivace",        120),
      TempoVal("Vivo",          120),
      TempoVal("Ballad",         60),
      TempoVal("Fast",          120),
      TempoVal("Lively",        120),
      TempoVal("Moderate",      100),
      TempoVal("Slow",           60),
      TempoVal("Very slow",      40),
      TempoVal("With movement", 120),
      TempoVal("Entrainant",    120),
      TempoVal("Lent",           60),
      TempoVal("Rapide",        120),
      TempoVal("Regulier",       80),
      TempoVal("Vif",           120),
      TempoVal("Vite",          120),
      TempoVal("Vivement",      160),
      TempoVal("Bewegt",        100),
      TempoVal("Langsam",        60),
      TempoVal("Lebhaft",       120),
      TempoVal("Mäßig",         100),     // utf8!
      TempoVal("Schnell",       120)
      };

static bool tempoListInit = true;
static bool tempoListChanged = true;

static QList<TempoVal*> tempoL;

//---------------------------------------------------------
//   EditTempo
//---------------------------------------------------------

EditTempo::EditTempo(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      if (tempoListInit) {
            tempoListInit = false;
            for (unsigned i = 0; i < sizeof(tempos)/sizeof(*tempos); ++i) {
                  tempoL.append(new TempoVal(tempos[i]));
                  }
            }
      int idx = 0;
      foreach(TempoVal* v, tempoL) {
            QListWidgetItem* item = new QListWidgetItem(v->name, tempoList);
            item->setData(Qt::UserRole, idx++);
            }
      selectTempo(3);
      connect(tempoText, SIGNAL(textChanged(const QString&)), SLOT(textChanged(const QString&)));
      connect(tempoBPM,  SIGNAL(valueChanged(double)), SLOT(bpmChanged(double)));
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
      if (n < 0 || n >= tempoL.size())
            return;
      _text = tempoL[n]->name;
      tempoText->setText(_text);
      _bpm  = tempoL[n]->bpm;
      tempoBPM->setValue(_bpm);
      }

//---------------------------------------------------------
//   textChanged
//---------------------------------------------------------

void EditTempo::textChanged(const QString& s)
      {
      _text = s;
      int idx = tempoList->currentItem()->data(Qt::UserRole).toInt();
      tempoL[idx]->name = s;
      tempoListChanged = true;
      tempoList->currentItem()->setText(s);
      }

//---------------------------------------------------------
//   bpmChanged
//---------------------------------------------------------

void EditTempo::bpmChanged(double v)
      {
      _bpm = v;
      int idx = tempoList->currentItem()->data(Qt::UserRole).toInt();
      tempoL[idx]->bpm = v;
      tempoListChanged = true;
      }

