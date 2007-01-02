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
//   setFontFamily
//---------------------------------------------------------

void TextPalette::setFontFamily(const QString& s)
      {
      typefaceFamily->addItem(s);
      }

//---------------------------------------------------------
//   setBold
//---------------------------------------------------------

void TextPalette::setBold(bool val)
      {
      typefaceBold->setChecked(val);
      }

//---------------------------------------------------------
//   setItalic
//---------------------------------------------------------

void TextPalette::setItalic(bool val)
      {
      typefaceItalic->setChecked(val);
      }

//---------------------------------------------------------
//   setFontSize
//---------------------------------------------------------

void TextPalette::setFontSize(int val)
      {
      typefaceSize->setValue(val);
      }

