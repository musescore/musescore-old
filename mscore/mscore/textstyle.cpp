//=============================================================================
//  MuseScore
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

#include "style.h"
#include "textstyle.h"
#include "globals.h"
#include "score.h"
#include "scoreview.h"
#include "textproperties.h"
#include "undo.h"

static const int INTERNAL_STYLES = 2;     // do not present first two styles to user

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent, Score* score)
   : QDialog(parent)
      {
      setWindowTitle(tr("MuseScore: Edit Text Styles"));
      QGridLayout* layout = new QGridLayout;
      tp = new TextProp;
      layout->addWidget(tp, 0, 1);
      textNames = new QListWidget;
      layout->addWidget(textNames, 0, 0);
      bb = new QDialogButtonBox(
         QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel
         );
      layout->addWidget(bb, 1, 0, 1, 2);
      setLayout(layout);

      cs = score;
      foreach(TextStyle* s, score->textStyles())
            styles.append(new TextStyle(*s));

      textNames->setSelectionMode(QListWidget::SingleSelection);
      textNames->clear();
      for (int i = INTERNAL_STYLES; i < styles.size(); ++i) {
            TextStyle* s = styles.at(i);
            textNames->addItem(qApp->translate("MuseScore", s->name.toAscii().data()));
            }

      connect(bb, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
      connect(textNames, SIGNAL(currentRowChanged(int)), SLOT(nameSelected(int)));

      current   = -1;
      textNames->setCurrentItem(textNames->item(0));
      }

//---------------------------------------------------------
//   ~TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::~TextStyleDialog()
      {
      foreach(TextStyle* ts, styles)
            delete ts;
      styles.clear();
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      current = n + INTERNAL_STYLES;
      tp->set(styles[current]);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      TextStyle* s = styles[n];
      tp->get(s);
      }

//---------------------------------------------------------
//   buttonClicked
//---------------------------------------------------------

void TextStyleDialog::buttonClicked(QAbstractButton* b)
      {
      switch (bb->standardButton(b)) {
            case QDialogButtonBox::Apply:
                  apply();
                  break;
            case QDialogButtonBox::Ok:
                  apply();
                  done(1);
                  break;
            default:
                  cs->undo()->current()->unwind();
                  cs->setLayoutAll(true);
                  done(0);
                  break;
            }
      }

//---------------------------------------------------------
//   apply
//---------------------------------------------------------

void TextStyleDialog::apply()
      {
      saveStyle(current);                 // update local copy of style list

      bool styleChanged = false;
      int n = cs->textStyles().size();
      for (int i = 0; i < n; ++i) {
            TextStyle* os = cs->textStyle(i);
            TextStyle* ns = styles[i];
            if (*os != *ns) {
                  styleChanged = true;
                  break;
                  }
            }
      if (!styleChanged)
            return;
      cs->undo()->push(new ChangeTextStyles(cs, styles));
      cs->setLayoutAll(true);
      cs->end();
      }

