//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: text.cpp -1   $
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "globals.h"
#include "text.h"
#include "xml.h"
#include "style.h"
#include "mscore.h"
#include "scoreview.h"
#include "score.h"
#include "utils.h"
#include "page.h"
#include "textpalette.h"
#include "sym.h"
#include "symbol.h"
#include "textline.h"
#include "preferences.h"
#include "system.h"
#include "measure.h"
#include "textproperties.h"
#include "textprop.h"
#include "box.h"
#include "segment.h"
#include "texttools.h"

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

TextProperties::TextProperties(Text* t, QWidget* parent)
   : QDialog(parent)
      {
      setWindowTitle(tr("MuseScore: Text Properties"));
      QGridLayout* layout = new QGridLayout;

      tp = new TextProp;
      tp->setScore(false, t->score());

      layout->addWidget(tp, 0, 1);
      QLabel* l = new QLabel;
      l->setPixmap(QPixmap(":/data/bg1.jpg"));
      l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

      layout->addWidget(l, 0, 0, 2, 1);
      QHBoxLayout* hb = new QHBoxLayout;
      QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      hb->addWidget(bb);
      layout->addLayout(hb, 1, 1);
      setLayout(layout);

      text = t;
      if (t->styled()) {
            text->setLocalStyle(text->score()->textStyle(text->textStyle()));
            }

      tp->setTextStyle(text->localStyle());
      tp->setStyled(t->styled());
      tp->setTextStyleType(t->textStyle());

      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TextProperties::accept()
      {
      text->setLocalStyle(tp->textStyle());

      QDialog::accept();
      if (tp->isStyled() != text->styled() || tp->isStyled()) {
            // text->setTextStyle(tp->textStyleType());  // this sets styled = true

            text->_textStyle = tp->textStyleType();
            text->setStyled(tp->isStyled());
            text->styleChanged();
            }
      }

