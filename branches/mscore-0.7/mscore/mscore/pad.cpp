//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: pad.cpp,v 1.14 2006/03/02 17:08:40 wschweer Exp $
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

#include "pad.h"
#include "padids.h"
#include "sym.h"
#include "mscore.h"
#include "icons.h"

//---------------------------------------------------------
//   Pad
//    Toolbox
//---------------------------------------------------------

Pad::Pad(QWidget* parent)
   : QDockWidget(tr("Pad"), parent)
      {
      QToolBox* tb = new QToolBox;

      QWidget* w = new QWidget;
      tb->addItem(w, tr("Notes"));
      QGridLayout* gl = new QGridLayout;
      gl->setSpacing(2);
      w->setLayout(gl);
      QToolButton* b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-1"));
      gl->addWidget(b, 0, 0);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-2"));
      gl->addWidget(b, 0, 1);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-4"));
      gl->addWidget(b, 0, 2);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-8"));
      gl->addWidget(b, 0, 3);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-16"));
      gl->addWidget(b, 0, 4);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-32"));
      gl->addWidget(b, 0, 5);
      b = new QToolButton;
      b->setDefaultAction(getAction("pad-note-64"));
      gl->addWidget(b, 0, 6);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-rest"));
      gl->addWidget(b, 1, 0);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-dot"));
      gl->addWidget(b, 1, 1);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-sharp2"));
      gl->addWidget(b, 2, 0);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-sharp"));
      gl->addWidget(b, 2, 1);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-nat"));
      gl->addWidget(b, 2, 2);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-flat"));
      gl->addWidget(b, 2, 3);

      b = new QToolButton;
      b->setDefaultAction(getAction("pad-flat2"));
      gl->addWidget(b, 2, 4);
      gl->setRowStretch(3, 100);

      //-----------

      w = new QWidget;
      tb->addItem(w, tr("Beams"));
      gl = new QGridLayout;
      gl->setSpacing(2);
      w->setLayout(gl);

      b = new QToolButton;
      b->setDefaultAction(getAction("beam-start"));
      gl->addWidget(b, 0, 0);

      b = new QToolButton;
      b->setDefaultAction(getAction("beam-mid"));
      gl->addWidget(b, 0, 1);

      b = new QToolButton;
      b->setDefaultAction(getAction("no-beam"));
      gl->addWidget(b, 0, 2);

      b = new QToolButton;
      b->setDefaultAction(getAction("beam32"));
      gl->addWidget(b, 0, 3);

      gl->setRowStretch(1, 100);
      setWidget(tb);
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void Pad::closeEvent(QCloseEvent* ev)
      {
      emit padVisible(false);
      QWidget::closeEvent(ev);
      }

