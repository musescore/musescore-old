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
#include "awl/pitchlabel.h"
#include "awl/pitchedit.h"
#include "awl/poslabel.h"

//---------------------------------------------------------
//   PianorollEditor
//---------------------------------------------------------

PianorollEditor::PianorollEditor(Staff* st, QWidget* parent)
   : QDialog(parent)
      {
      staff = st;
      Score* s = staff->score();

      QGridLayout* layout = new QGridLayout;
      layout->setSpacing(0);

      QToolBar* tb = new QToolBar;
      layout->addWidget(tb, 0, 0, 1, 2);
      VoiceSelector* vs = new VoiceSelector;
      tb->addWidget(vs);

      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Cursor:")));
      Awl::PosLabel* pos = new Awl::PosLabel(s->getTempomap(), s->getSigmap());
      tb->addWidget(pos);
      Awl::PitchLabel* pl = new Awl::PitchLabel();
      tb->addWidget(pl);


      tb->addSeparator();
      tb->addWidget(new QLabel(tr("Velocity:")));
      velocity = new QSpinBox;
      velocity->setRange(-1, 127);
      velocity->setSpecialValueText("--");
      velocity->setReadOnly(true);
      tb->addWidget(velocity);

      tb->addWidget(new QLabel(tr("Pitch:")));
      pitch = new Awl::PitchEdit;
      pitch->setReadOnly(true);
      tb->addWidget(pitch);

      double xmag    = .1;
      gv  = new PianoView(staff);
      gv->scale(xmag, 1.0);
      layout->addWidget(gv, 2, 1);

      Ruler* ruler = new Ruler(s);
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
      connect(gv, SIGNAL(magChanged(double,double)), piano, SLOT(setMag(double,double)));

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
            velocity->setEnabled(true);
            velocity->setValue(note->velocity());
            pitch->setEnabled(true);
            pitch->setValue(note->pitch());
            }
      else if (items.size() == 0) {
            velocity->setValue(0);
            velocity->setEnabled(false);
            pitch->setValue(0);
            pitch->setEnabled(false);
            }
      else {
            velocity->setEnabled(true);
            velocity->setValue(0);
            pitch->setEnabled(true);
            pitch->setDeltaMode(true);
            pitch->setValue(0);
            }
      }

