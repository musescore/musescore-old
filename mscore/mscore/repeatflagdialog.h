
//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
//
//  $Id: repeatflagdialog.h,v 1.00 2007/09/26 23:00:00 dikrau Exp $
//
//  Copyright (C) 2007- Dieter Krause (dikrau@users.sourceforge.net)
//
//  repeatflagdialog.h: handels the gui for information input of specials for repeats 
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

#ifndef __REPEATFLAGDIALOG_H__
#define __REPEATFLAGDIALOG_H__

#include "ui_repeatflagdialog.h"


class RepeatFlagDialog : public QDialog, public Ui::RepeatFlagDialogBase {
      Q_OBJECT

      
private slots:
      virtual void accept();

public:
      RepeatFlagDialog(QWidget* parent = 0);
      ~RepeatFlagDialog();

      };

#endif
