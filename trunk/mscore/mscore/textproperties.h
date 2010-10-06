//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __TEXTPROPERTIES_H__
#define __TEXTPROPERTIES_H__

#include "ui_textproperties.h"
#include "style.h"

class TextB;

//---------------------------------------------------------
//   TextProp
//---------------------------------------------------------

class TextProp : public QWidget, public Ui::TextProperties {
      Q_OBJECT

      int curUnit;
      bool onlyStyle;

   private slots:
      void mmToggled(bool);
      void styledToggled(bool);
      void unstyledToggled(bool);

   public:
      TextProp(bool _onlyStyle, QWidget* parent = 0);
      void set(TextB*);
      void get(TextB*);
      void set(const TextStyle&);
      TextStyle getTextStyle() const;
      };

#endif

