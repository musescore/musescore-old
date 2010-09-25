//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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

#include "symboldialog.h"
#include "palette.h"
#include "mscore.h"
#include "sym.h"
#include "style.h"
#include "element.h"
#include "symbol.h"
#include "preferences.h"

//---------------------------------------------------------
//   createSymbolPalette
//---------------------------------------------------------

void SymbolDialog::createSymbolPalette()
      {
      sp = new Palette();
      for (int i = 0; i < lastSym; ++i)
            sp->append(i);
      }

//---------------------------------------------------------
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Symbols"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      createSymbolPalette();
      QScrollArea* sa = new PaletteScrollArea(sp);
      l->addWidget(sa);

      sp->setAcceptDrops(true);
      sp->setDrawGrid(true);
      sp->setSelectable(true);

      connect(sp, SIGNAL(changed()), SLOT(setDirty()));
      connect(sp, SIGNAL(startDragElement(Element*)), SLOT(startDragElement(Element*)));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteElement()));
      sa->setWidget(sp);
      }

//---------------------------------------------------------
//   setDirty
//---------------------------------------------------------

void SymbolDialog::setDirty()
      {
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   deleteElement
//---------------------------------------------------------

void SymbolDialog::deleteElement()
      {
      int idx = sp->getSelectedIdx();
      if (idx == -1)
            return;
      sp->add(idx, 0, QString());
      preferences.dirty = true;
      }

//---------------------------------------------------------
//   startDragElement
//---------------------------------------------------------

void SymbolDialog::startDragElement(Element* el)
      {
      el->setSystemFlag(systemFlag->isChecked());
      }

