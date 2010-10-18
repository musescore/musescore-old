//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

class Text;

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

class TextPalette : public QWidget, public Ui::TextPaletteBase {
      Q_OBJECT

      Text* _textElement;
      QPushButton* buttons[256];;
      QButtonGroup* sg;
      int curPage;

      void closeEvent(QCloseEvent* ev);

   private slots:
      void symbolClicked(int);
      void populate();

   public:
      TextPalette(QWidget* parent);
      void setText(Text* te);
      Text* text() { return _textElement; }
      };

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

class TextTools : public QDockWidget {
      Q_OBJECT

      Text* _textElement;
      QTextCharFormat format;
      QTextBlockFormat bformat;
      QDoubleSpinBox* typefaceSize;
      QFontComboBox* typefaceFamily;
      QAction* typefaceBold;
      QAction* typefaceItalic;
      QAction* typefaceUnderline;
      QAction* leftAlign;
      QAction* centerAlign;
      QAction* topAlign;
      QAction* bottomAlign;
      QAction* vcenterAlign;
      QAction* rightAlign;
      QAction* typefaceSubscript;
      QAction* typefaceSuperscript;
      QAction* showKeyboard;

      void blockAllSignals(bool val);

   private slots:
      void sizeChanged(double value);
      void moveFocus();
      void fontChanged(const QFont&);
      void boldClicked(bool);
      void italicClicked(bool);
      void underlineClicked(bool);
      void subscriptClicked(bool);
      void superscriptClicked(bool);
      void setLeftAlign();
      void setRightAlign();
      void setHCenterAlign();
      void setTopAlign();
      void setBottomAlign();
      void setVCenterAlign();
      void showKeyboardClicked(bool);

   public:
      TextTools(QWidget* parent = 0);
      void setText(Text* te);
      void setCharFormat(const QTextCharFormat&);
      void setBlockFormat(const QTextBlockFormat&);
      QAction* kbAction() const { return showKeyboard; }
      };

#endif

