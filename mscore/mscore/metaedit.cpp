//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "metaedit.h"
#include "libmscore/score.h"
#include "libmscore/undo.h"

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

MetaEditDialog::MetaEditDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      score = s;

      movementNumber->setText(score->metaTag("movementNumber"));
      movementTitle->setText(score->metaTag("movementTitle"));
      workNumber->setText(score->metaTag("workNumber"));
      workTitle->setText(score->metaTag("workTitle"));
      source->setText(score->metaTag("source"));
      copyright->setText(score->metaTag("copyright"));
      date->setDate(score->creationDate());
      level->setValue(score->mscVersion());
      version->setText(score->mscoreVersion());
      revision->setValue(score->mscoreRevision());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MetaEditDialog::accept()
      {
#define T(x, y) if (score->metaTag(x) != y->text()) \
            score->undo()->push(new ChangeMetaText(score, x, y->text()));

      T("movementNumber", movementNumber)
      T("movementTitle", movementTitle)
      T("workNumber", workNumber)
      T("workTitle", workTitle)
      T("source", source)
      T("copyright", copyright)

      QDialog::accept();
      }

