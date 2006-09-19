//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: edittempo.h,v 1.3 2006/03/02 17:08:33 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __EDITTEMPO_H__
#define __EDITTEMPO_H__

#include "ui_edittempo.h"

//---------------------------------------------------------
//   EditTempo
//---------------------------------------------------------

class EditTempo : public QDialog, private Ui::EditTempoBase {
      Q_OBJECT
      QString _text;
      int _bpm;

   private slots:
      void selectTempo(int);

   public:
      EditTempo(QWidget* parent);
      int bpm() const { return _bpm; }
      QString text() const { return _text; }
      };

#endif

