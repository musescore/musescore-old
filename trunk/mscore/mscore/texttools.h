//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.h 3592 2010-10-18 17:24:18Z wschweer $
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

#ifndef __TEXTTOOLS_H__
#define __TEXTTOOLS_H__


class Text;

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

class TextTools : public QDockWidget {
      Q_OBJECT

      Text* _textElement;

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
      QAction* toggleStyled;
      QAction* unorderedList;
      QAction* orderedList;
      QAction* indentMore;
      QAction* indentLess;

      void blockAllSignals(bool val);
      void updateText();
      QTextCursor* cursor();

   private slots:
      void sizeChanged(double value);
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
      void styledChanged(bool);
      void unorderedListClicked();
      void orderedListClicked();
      void indentMoreClicked();
      void indentLessClicked();

   public:
      TextTools(QWidget* parent = 0);
      void setText(Text* te);
      void updateTools();
      QAction* kbAction() const { return showKeyboard; }
      };

#endif

