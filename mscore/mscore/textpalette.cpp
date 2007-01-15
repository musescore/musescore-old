//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.cpp,v 1.2 2006/03/22 12:04:14 wschweer Exp $
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

#include "textpalette.h"
#include "icons.h"
#include "text.h"
#include "sym.h"

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      QButtonGroup* sg = new QButtonGroup(this);
      symSharp->setIcon(sharpIcon);
      sg->addButton(symSharp, sharpSym);
      symFlat->setIcon(flatIcon);
      sg->addButton(symFlat, flatSym);
      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));

      connect(typefaceSize, SIGNAL(valueChanged(int)), SLOT(sizeChanged(int)));
      connect(typefaceBold, SIGNAL(clicked(bool)), SLOT(boldClicked(bool)));
      connect(typefaceItalic, SIGNAL(clicked(bool)), SLOT(italicClicked(bool)));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   symbolClicked
//---------------------------------------------------------

void TextPalette::symbolClicked(int n)
      {
      _textElement->addSymbol(n);
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextPalette::sizeChanged(int value)
      {
      QFont f(format.font());
      f.setPointSize(value);
//      format.setFontPointSize(value);
      format.setFont(f);
      _textElement->setCharFormat(format);
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextPalette::boldClicked(bool val)
      {
      format.setFontWeight(val ? QFont::Bold : QFont::Normal);
      _textElement->setCharFormat(format);
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextPalette::italicClicked(bool val)
      {
      format.setFontItalic(val);
      _textElement->setCharFormat(format);
      }

//---------------------------------------------------------
//   setCharFormat
//---------------------------------------------------------

void TextPalette::setCharFormat(QTextCharFormat cf)
      {
      format = cf;
      typefaceFamily->setCurrentFont(cf.font());
      int fs = cf.font().pointSize();
      qreal rfs = cf.fontPointSize();
printf("points %d %f\n", fs, rfs);
      typefaceSize->setValue(fs);
      typefaceItalic->setChecked(cf.fontItalic());
      typefaceBold->setChecked(cf.fontWeight() == QFont::Bold);
      }

