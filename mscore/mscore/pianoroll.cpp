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
#include "voiceselector.h"
#include "note.h"

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(Staff* s, QWidget* parent)
   : QDialog(parent)
      {
      staff = s;
      QGridLayout* layout = new QGridLayout;
      layout->setSpacing(0);

      QToolBar* tb = new QToolBar;
      layout->addWidget(tb, 0, 0, 1, 2);
      VoiceSelector* vs = new VoiceSelector;
      tb->addWidget(vs);
      tb->addSeparator();
      tb->addWidget(new QLabel(tr("velocity:")));
      velocity = new QSpinBox;
      velocity->setRange(-1, 127);
      velocity->setSpecialValueText("--");
      tb->addWidget(velocity);
      tb->addWidget(new QLabel(tr("pitch:")));
      pitch = new QSpinBox;
      pitch->setRange(-1, 127);
      pitch->setSpecialValueText("--");

      tb->addWidget(pitch);

      double xmag    = .1;
      gv  = new PianoView(s);
      gv->scale(xmag, 1.0);
      layout->addWidget(gv, 2, 1);

      Ruler* ruler = new Ruler(s->score());
      ruler->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
      ruler->setFixedHeight(rulerHeight);
      ruler->setMag(xmag, 1.0);

      layout->addWidget(ruler, 1, 1);

      Piano* piano = new Piano;
      piano->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
      piano->setFixedWidth(pianoWidth);
      layout->addWidget(piano, 2, 0);

      setLayout(layout);

      connect(gv->verticalScrollBar(), SIGNAL(valueChanged(int)), piano, SLOT(setYpos(int)));
      connect(gv->horizontalScrollBar(), SIGNAL(valueChanged(int)), ruler, SLOT(setXpos(int)));
      connect(gv, SIGNAL(xposChanged(int)), ruler, SLOT(setXpos(int)));
      connect(gv, SIGNAL(magChanged(double,double)), ruler, SLOT(setMag(double,double)));

      connect(gv->scene(), SIGNAL(selectionChanged()), SLOT(selectionChanged()));
      resize(800, 400);
      }

//---------------------------------------------------------
//   selectionChanged
//---------------------------------------------------------

void PianorollEditor::selectionChanged()
      {
      QList<QGraphicsItem*> items = gv->scene()->selectedItems();
      if (items.size() == 1) {
            QGraphicsItem* item = items[0];
            Note* note = (Note*)item->data(0).value<void*>();
            velocity->setValue(note->velocity());
            pitch->setValue(note->pitch());
            }
      else if (items.size() == 0) {
            velocity->setValue(-1);
            pitch->setValue(-1);
            }
      else {
            velocity->setValue(0);
            pitch->setValue(0);
            }
      }

