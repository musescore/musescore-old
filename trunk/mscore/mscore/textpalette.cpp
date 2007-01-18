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
#include "style.h"

static SymCode pSymbols[] = {
      SymCode(0xe10e, TEXT_STYLE_DYNAMICS1),    // sharp
      SymCode(0xe112, TEXT_STYLE_DYNAMICS1),    // flat
      SymCode(0xe102, TEXT_STYLE_DYNAMICS1),    // note2_Sym
      SymCode(0xe0fc, TEXT_STYLE_DYNAMICS1),    // note4_Sym
      SymCode(0xe0f8, TEXT_STYLE_DYNAMICS1),    // note8_Sym
      SymCode(0xe0f9, TEXT_STYLE_DYNAMICS1),    // note16_Sym
      SymCode(0xe0fa, TEXT_STYLE_DYNAMICS1),    // note32_Sym
      SymCode(0xe0fb, TEXT_STYLE_DYNAMICS1),    // note64_Sym
      };

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setupUi(this);
      QGridLayout* gl = new QGridLayout;
      symbolBox->setLayout(gl);
      QButtonGroup* sg = new QButtonGroup(this);

      for (unsigned i = 0; i < sizeof(pSymbols)/sizeof(*pSymbols); ++i) {
            QIcon icon = symIcon(pSymbols[i], 20);
            QToolButton* tb = new QToolButton;
            tb->setIcon(icon);
            gl->addWidget(tb, i / 8, i % 8);
            sg->addButton(tb, i);
            }
      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));
      connect(typefaceSize, SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceBold, SIGNAL(clicked(bool)), SLOT(boldClicked(bool)));
      connect(typefaceItalic, SIGNAL(clicked(bool)), SLOT(italicClicked(bool)));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   symbolClicked
//---------------------------------------------------------

void TextPalette::symbolClicked(int n)
      {
      _textElement->addSymbol(pSymbols[n]);
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextPalette::sizeChanged(double value)
      {
      format.setFontPointSize(value);
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

void TextPalette::setCharFormat(const QTextCharFormat& cf)
      {
      format = cf;
      QFont f(cf.font());
      typefaceFamily->setCurrentFont(f);
      typefaceSize->setValue(f.pointSizeF());
      typefaceItalic->setChecked(cf.fontItalic());
      typefaceBold->setChecked(cf.fontWeight() == QFont::Bold);
      }

