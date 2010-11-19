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

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

TextStyleDialog::TextStyleDialog(QWidget* parent, Score* score)
   : QDialog(parent)
      {
      setWindowTitle(tr("MuseScore: Edit Text Styles"));
      QGridLayout* layout = new QGridLayout;
      tp = new TextProp(true, score);
      layout->addWidget(tp, 0, 1);
      textNames = new QListWidget;
      layout->addWidget(textNames, 0, 0);
      bb = new QDialogButtonBox(
         QDialogButtonBox::Ok | QDialogButtonBox::Apply | QDialogButtonBox::Cancel
         );
      layout->addWidget(bb, 1, 0, 1, 2);
      setLayout(layout);

      cs     = score;
      styles = cs->style().textStyles();

      textNames->setSelectionMode(QListWidget::SingleSelection);
      textNames->clear();
      for (int i = 0; i < styles.size(); ++i) {
            const TextStyle& s = styles.at(i);
            textNames->addItem(qApp->translate("MuseScore", s.name().toAscii().data()));
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
      }

//---------------------------------------------------------
//   nameSelected
//---------------------------------------------------------

void TextStyleDialog::nameSelected(int n)
      {
      if (current != -1)
            saveStyle(current);
      current = n;
      tp->setTextStyle(styles[current]);
      }

//---------------------------------------------------------
//   saveStyle
//---------------------------------------------------------

void TextStyleDialog::saveStyle(int n)
      {
      TextStyle st = tp->textStyle();
      // set data members not set by TextProp::textStyle()
      st.setName(styles[n].name());
      st.setSizeIsSpatiumDependent(styles[n].sizeIsSpatiumDependent());
      styles[n] = st;
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

      int n = cs->style().textStyles().size();
      for (int i = 0; i < n; ++i) {
            const TextStyle& os = cs->textStyle(TextStyleType(i));
            const TextStyle& ns = styles[i];
            if (os != ns)
                  cs->undo()->push(new ChangeTextStyle(cs, ns));
            }
      cs->end();
      }

