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
#include "score.h"

//---------------------------------------------------------
//   MetaEditDialog
//---------------------------------------------------------

MetaEditDialog::MetaEditDialog(Score* s, QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      score = s;

      movementNumber->setText(score->movementNumber());
      movementTitle->setText(score->movementTitle());
      workNumber->setText(score->workNumber());
      workTitle->setText(score->workTitle());
      source->setText(score->source());
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void MetaEditDialog::accept()
      {
      score->setMovementNumber(movementNumber->text());
      score->setMovementTitle(movementTitle->text());
      score->setWorkNumber(workNumber->text());
      score->setWorkTitle(workTitle->text());
      score->setSource(source->text());
      score->setDirty(true);
      QDialog::accept();
      }

