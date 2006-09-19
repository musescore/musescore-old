//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: pad.h,v 1.8 2006/03/02 17:08:40 wschweer Exp $
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

#ifndef __PAD_H__
#define __PAD_H__

#include "ui_pad.h"

const int PAD_LAYOUTS = 5;

enum PadKeys {
      PAD_0, PAD_1, PAD_2, PAD_3, PAD_4, PAD_5, PAD_6,
      PAD_7, PAD_8, PAD_9, PAD_NUM, PAD_DIV, PAD_MULT,
      PAD_MINUS, PAD_PLUS, PAD_ENTER, PAD_COMMA,
      PAD_N0, PAD_N1, PAD_N2, PAD_N3, PAD_N4,
      PAD_KEYS
      };

class QPixmap;

struct PadEntry {
      QIcon* icon;
      int cmd;
      const char* help;
      };

extern PadEntry padTrans[PAD_LAYOUTS][PAD_KEYS];

//---------------------------------------------------------
//   Pad
//---------------------------------------------------------

class Pad : public QWidget, private Ui::PadBase {
      Q_OBJECT
      int dx, dy;
      int _padNo;
      QButtonGroup* bg;

      virtual void mouseMoveEvent(QMouseEvent* ev);
      virtual void mousePressEvent(QMouseEvent* ev);
      virtual bool event(QEvent*);
      virtual void closeEvent(QCloseEvent*);

   public slots:
      void setPadNo(int);
      void setOn(bool, int);

   signals:
      void keyEvent(QKeyEvent*);
      void close();

   public:
      int padNo() const { return _padNo; }
      Pad(QWidget* parent = 0);
      };

#endif
