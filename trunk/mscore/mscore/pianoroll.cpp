//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#include "pianoroll.h"
#include "piano.h"
#include "ruler.h"
#include "pianoscene.h"
#include "staff.h"
#include "score.h"
#include "measure.h"

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      QGridLayout* layout = new QGridLayout;

      Measure* lm = s->score()->lastMeasure();
      int ticks = lm->tick() + lm->tickLen();

      double xmag = .1;
      QGraphicsScene* scene = new PianoScene(s);
      scene->setSceneRect(0.0, 0.0, double(ticks), keyHeight * 75);
      QGraphicsView* gv     = new QGraphicsView(scene);
      gv->scale(xmag, 1.0);
      layout->addWidget(gv, 1, 1);

      Ruler* ruler = new Ruler(s->score());
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);
      ruler->setXmag(xmag);

      layout->addWidget(ruler, 0, 1);

      Piano* piano = new Piano;
      piano->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      piano->setFixedWidth(pianoWidth);
      layout->addWidget(piano, 1, 0);

      setLayout(layout);

      connect(gv->verticalScrollBar(), SIGNAL(valueChanged(int)), piano, SLOT(setYpos(int)));
      connect(gv->horizontalScrollBar(), SIGNAL(valueChanged(int)), ruler, SLOT(setXpos(int)));
      resize(800, 400);
      }

