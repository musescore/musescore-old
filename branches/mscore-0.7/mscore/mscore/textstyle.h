//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textstyle.h,v 1.3 2006/03/02 17:08:43 wschweer Exp $
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

#ifndef __TEXTSTYLE_H__
#define __TEXTSTYLE_H__

#include "style.h"
#include "ui_textstyle.h"

//---------------------------------------------------------
//   TextStyleDialog
//---------------------------------------------------------

class TextStyleDialog : public QDialog, private Ui::TextStyleBase {
      Q_OBJECT
      TextStyleList styles;

      int current;
      int fonts;        // number of font families in list
      void saveStyle(int);

   private slots:
      void nameSelected(int);
      void ok();
      void apply();
      void fontChanged();
      void fontSizeChanged(int n);
      void fontNameChanged(int);

      void alignLeftH();
      void alignRightH();
      void alignCenterH();
      void alignTopV();
      void alignBottomV();
      void alignCenterV();
      void setUnitMM();
      void setUnitPercent();
      void setUnitSpace();

   signals:

   public:
      TextStyleDialog(QWidget* parent);
      };

#endif

