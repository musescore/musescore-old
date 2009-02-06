//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.h,v 1.2 2006/03/22 12:04:14 wschweer Exp $
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

#ifndef __TEXTPALETTE_H__
#define __TEXTPALETTE_H__

#include "ui_textpalette.h"

class TextB;

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

class TextPalette : public QWidget, public Ui::TextPaletteBase {
      Q_OBJECT

      TextB* _textElement;
      QTextCharFormat format;
      QTextBlockFormat bformat;

   private slots:
      void symbolClicked(int);
      void sizeChanged(double value);
      void boldClicked(bool);
      void italicClicked(bool);
      void underlineClicked(bool);
      void setLeftAlign();
      void setRightAlign();
      void setHCenterAlign();
      void fontChanged(const QFont&);
      void subscriptClicked(bool);
      void superscriptClicked(bool);
      void borderChanged(double);
      void paddingChanged(double);
      void frameRoundChanged(int val);
      void frameColorChanged(QColor);
      void circleToggled(bool val);

   public:
      TextPalette(QWidget* parent);
      void setText(TextB* te);
      void setCharFormat(const QTextCharFormat&);
      void setBlockFormat(const QTextBlockFormat&);
      };

#endif

