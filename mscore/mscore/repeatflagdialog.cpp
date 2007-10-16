//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeatflagdialog.cpp,v 1.00 2007/08/23 14:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
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
//


#include "repeatflagdialog.h"
#include "repeatflag.h"


void RepeatFlagDialog::accept()
      {
      RepeatFlag* rf;
      Element* el;

      el = actElement;
      rf = el->repeatFlag();

      rf->setCycle(cycles->value());
      rf->setNo(no->value());
      rf->setDestNo(destno->value());
      rf->setCycleList(cyclesToPlay->text());

      done(1);
      }

RepeatFlagDialog::RepeatFlagDialog(QWidget* parent)
      : QDialog(parent)
      {
	setupUi(this);
      }

RepeatFlagDialog::~RepeatFlagDialog()
      {
      }
