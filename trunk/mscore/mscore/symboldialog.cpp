//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2007 Werner Schweer (ws@seh.de)
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
//   SymbolDialog
//---------------------------------------------------------

SymbolDialog::SymbolDialog(QWidget* parent)
   : QWidget(parent, Qt::Dialog | Qt::Window)
      {
      setupUi(this);
      setWindowTitle(tr("MuseScore: Symbols"));
      QLayout* l = new QVBoxLayout();
      frame->setLayout(l);
      QScrollArea* symbolPalette = new QScrollArea;
      l->addWidget(symbolPalette);

      QButtonGroup* bg = new QButtonGroup();
      bg->addButton(anchorPage, ANCHOR_PAGE);
      bg->addButton(anchorTime, ANCHOR_TICK);
      bg->addButton(anchorNote, ANCHOR_NOTE);
      bg->addButton(anchorSystem, ANCHOR_SYSTEM);
      connect(bg, SIGNAL(buttonClicked(int)), SLOT(anchorClicked(int)));
      anchorPage->setChecked(true);

      sp = preferences.sp;
      sp->setAcceptDrops(true);
      sp->setDrawGrid(true);
      sp->setShowSelection(true);

      connect(sp, SIGNAL(droppedElement(Element*)), SLOT(elementDropped(Element*)));
      connect(deleteButton, SIGNAL(clicked()), SLOT(deleteElement()));
      symbolPalette->setWidget(sp);
      }

//---------------------------------------------------------
//   anchorClicked
//---------------------------------------------------------

void SymbolDialog::anchorClicked(int val)
      {
      Anchor anchor = (Anchor)val;
      int rows    = sp->getRows();
      int columns = sp->getColumns();

      for (int i = 0; i < rows * columns; ++i) {
            Element* e = sp->element(i);
            if (e && e->type() == SYMBOL)
                  ((Symbol*)e)->setAnchor(anchor);
            }
      }

//---------------------------------------------------------
//   elementDropped
//---------------------------------------------------------

void SymbolDialog::elementDropped(Element*)
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
      sp->addObject(idx, 0, QString());
      preferences.dirty = true;
      }

