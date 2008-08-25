//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.cpp,v 1.2 2006/03/22 12:04:14 wschweer Exp $
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

#include "textpalette.h"
#include "icons.h"
#include "text.h"
#include "sym.h"
#include "style.h"
#include "mscore.h"
#include "score.h"

static SymCode pSymbols[] = {
      SymCode(0xe10e, TEXT_STYLE_DYNAMICS1),    // sharp
      SymCode(0xe112, TEXT_STYLE_DYNAMICS1),    // flat
      SymCode(0xe102, TEXT_STYLE_DYNAMICS1),    // note2_Sym
      SymCode(0xe0fc, TEXT_STYLE_DYNAMICS1),    // note4_Sym
      SymCode(0xe0f8, TEXT_STYLE_DYNAMICS1),    // note8_Sym
      SymCode(0xe0f9, TEXT_STYLE_DYNAMICS1),    // note16_Sym
      SymCode(0xe0fa, TEXT_STYLE_DYNAMICS1),    // note32_Sym
      SymCode(0xe0fb, TEXT_STYLE_DYNAMICS1),    // note64_Sym
      SymCode(0xe168, TEXT_STYLE_DYNAMICS1),    // coda
      SymCode(0xe169, TEXT_STYLE_DYNAMICS1),    // varcoda
      SymCode(0xe167, TEXT_STYLE_DYNAMICS1),    // segno
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0, 0),
      SymCode(0xa9, -1),
      SymCode(0x00c0, -1),
      SymCode(0x00c1, -1),
      SymCode(0x00c2, -1),
      SymCode(0x00c3, -1),
      SymCode(0x00c4, -1),
      SymCode(0x00c5, -1),
      SymCode(0x00c6, -1),
      SymCode(0x00c7, -1),
      SymCode(0x00c8, -1),
      SymCode(0x00c9, -1),
      SymCode(0x00ca, -1),
      SymCode(0x00cb, -1),
      SymCode(0x00cc, -1),
      SymCode(0x00cd, -1),
      SymCode(0x00ce, -1),
      SymCode(0x00cf, -1),

      SymCode(0x00d0, -1),
      SymCode(0x00d1, -1),
      SymCode(0x00d2, -1),
      SymCode(0x00d3, -1),
      SymCode(0x00d4, -1),
      SymCode(0x00d5, -1),
      SymCode(0x00d6, -1),
      SymCode(0x00d7, -1),
      SymCode(0x00d8, -1),
      SymCode(0x00d9, -1),
      SymCode(0x00da, -1),
      SymCode(0x00db, -1),
      SymCode(0x00dc, -1),
      SymCode(0x00dd, -1),
      SymCode(0x00de, -1),
      SymCode(0x00df, -1),

      SymCode(0x00e0, -1),
      SymCode(0x00e1, -1),
      SymCode(0x00e2, -1),
      SymCode(0x00e3, -1),
      SymCode(0x00e4, -1),
      SymCode(0x00e5, -1),
      SymCode(0x00e6, -1),
      SymCode(0x00e7, -1),
      SymCode(0x00e8, -1),
      SymCode(0x00e9, -1),
      SymCode(0x00ea, -1),
      SymCode(0x00eb, -1),
      SymCode(0x00ec, -1),
      SymCode(0x00ed, -1),
      SymCode(0x00ee, -1),
      SymCode(0x00ef, -1),

      SymCode(0x00f0, -1),
      SymCode(0x00f1, -1),
      SymCode(0x00f2, -1),
      SymCode(0x00f3, -1),
      SymCode(0x00f4, -1),
      SymCode(0x00f5, -1),
      SymCode(0x00f6, -1),
      SymCode(0x00f7, -1),
      SymCode(0x00f8, -1),
      SymCode(0x00f9, -1),
      SymCode(0x00fa, -1),
      SymCode(0x00fb, -1),
      SymCode(0x00fc, -1),
      SymCode(0x00fd, -1),
      SymCode(0x00fe, -1),
      SymCode(0x00ff, -1),
      };

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setWindowFlags(Qt::Tool | Qt::FramelessWindowHint);
      setupUi(this);
      QGridLayout* gl = new QGridLayout;
      gl->setMargin(5);
      gl->setSpacing(1);
      symbolBox->setLayout(gl);
      QButtonGroup* sg = new QButtonGroup(this);

      for (unsigned i = 0; i < sizeof(pSymbols)/sizeof(*pSymbols); ++i) {
            if (pSymbols[i].code == 0)    // empty slot?
                  continue;
            QToolButton* tb = new QToolButton;
            tb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            tb->setFixedSize(40, 40);

            SymCode sc(pSymbols[i]);
            if (sc.style == -1)
                  tb->setText(sc.code);
            else {
                  int id = 2;
                  TextStyle* style = &defaultTextStyles[sc.style];
                  QString family = style->family;
                  if (family == "MScore")
                        id = 0;
                  else if (family == "MScore1")
                        id = 1;
                  Sym sym("", sc.code, id);
                  QIcon icon = symIcon(sym, 25, 35, 35);
                  tb->setIconSize(QSize(35, 35));
                  tb->setIcon(icon);
                  }
            gl->addWidget(tb, i / 16, i % 16);
            sg->addButton(tb, i);
            }

      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));
      connect(typefaceSize, SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceBold, SIGNAL(clicked(bool)), SLOT(boldClicked(bool)));
      connect(typefaceItalic, SIGNAL(clicked(bool)), SLOT(italicClicked(bool)));
      connect(typefaceUnderline, SIGNAL(clicked(bool)), SLOT(underlineClicked(bool)));
      connect(typefaceSubscript, SIGNAL(clicked(bool)), SLOT(subscriptClicked(bool)));
      connect(typefaceSuperscript, SIGNAL(clicked(bool)), SLOT(superscriptClicked(bool)));
      connect(typefaceFamily, SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(leftAlign, SIGNAL(clicked()), SLOT(setLeftAlign()));
      connect(rightAlign, SIGNAL(clicked()), SLOT(setRightAlign()));
      connect(centerAlign, SIGNAL(clicked()), SLOT(setHCenterAlign()));
      connect(frameWidth, SIGNAL(valueChanged(double)),  SLOT(borderChanged(double)));
      connect(paddingWidth, SIGNAL(valueChanged(double)), SLOT(paddingChanged(double)));
      connect(borderRounding, SIGNAL(valueChanged(int)), SLOT(frameRoundChanged(int)));
      connect(circle, SIGNAL(toggled(bool)), SLOT(circleToggled(bool)));
      connect(frameColor, SIGNAL(colorChanged(QColor)), SLOT(frameColorChanged(QColor)));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   symbolClicked
//---------------------------------------------------------

void TextPalette::symbolClicked(int n)
      {
      _textElement->addSymbol(pSymbols[n]);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextPalette::sizeChanged(double value)
      {
      format.setFontPointSize(value);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextPalette::boldClicked(bool val)
      {
      format.setFontWeight(val ? QFont::Bold : QFont::Normal);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   underlineClicked
//---------------------------------------------------------

void TextPalette::underlineClicked(bool val)
      {
      format.setFontUnderline(val);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextPalette::fontChanged(const QFont& f)
      {
      format.setFontFamily(f.family());
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextPalette::italicClicked(bool val)
      {
      format.setFontItalic(val);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
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
      typefaceUnderline->setChecked(cf.fontUnderline());
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void TextPalette::setBlockFormat(const QTextBlockFormat& bf)
      {
      bformat = bf;
      if (bf.alignment() & Qt::AlignHCenter)
            centerAlign->setChecked(true);
      else if (bf.alignment() & Qt::AlignLeft)
            leftAlign->setChecked(true);
      else if (bf.alignment() & Qt::AlignRight)
            rightAlign->setChecked(true);
      }

//---------------------------------------------------------
//   setHCenterAlign
//---------------------------------------------------------

void TextPalette::setHCenterAlign()
      {
      bformat.setAlignment(Qt::AlignHCenter);
      _textElement->setBlockFormat(bformat);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   setLeftAlign
//---------------------------------------------------------

void TextPalette::setLeftAlign()
      {
      bformat.setAlignment(Qt::AlignLeft);
      _textElement->setBlockFormat(bformat);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   setRightAlign
//---------------------------------------------------------

void TextPalette::setRightAlign()
      {
      bformat.setAlignment(Qt::AlignRight);
      _textElement->setBlockFormat(bformat);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   subscriptClicked
//---------------------------------------------------------

void TextPalette::subscriptClicked(bool val)
      {
      typefaceSuperscript->blockSignals(true);
      typefaceSuperscript->setChecked(false);
      typefaceSuperscript->blockSignals(false);
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   superscriptClicked
//---------------------------------------------------------

void TextPalette::superscriptClicked(bool val)
      {
      typefaceSubscript->blockSignals(true);
      typefaceSubscript->setChecked(false);
      typefaceSubscript->blockSignals(false);
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
      _textElement->setCharFormat(format);
      mscore->activateWindow();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextPalette::setText(TextB* te)
      {
      _textElement = te;

      frameWidth->setValue(_textElement->frameWidth());
      paddingWidth->setValue(_textElement->paddingWidth());
      borderRounding->setValue(_textElement->frameRound());
      circle->setChecked(_textElement->circle());
      borderRounding->setEnabled(!_textElement->circle());
      frameColor->setColor(_textElement->frameColor());
      }

//---------------------------------------------------------
//   borderChanged
//---------------------------------------------------------

void TextPalette::borderChanged(double val)
      {
      _textElement->setFrameWidth(val);
      _textElement->layout(0);
      _textElement->score()->addRefresh(_textElement->abbox().adjusted(-6, -6, 12, 12));
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   paddingChanged
//---------------------------------------------------------

void TextPalette::paddingChanged(double val)
      {
      _textElement->setPaddingWidth(val);
      _textElement->score()->addRefresh(_textElement->abbox().adjusted(-6, -6, 12, 12));
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   frameRoundChanged
//---------------------------------------------------------

void TextPalette::frameRoundChanged(int val)
      {
      _textElement->setFrameRound(val);
      _textElement->score()->addRefresh(_textElement->abbox().adjusted(-6, -6, 12, 12));
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   frameColorChanged
//---------------------------------------------------------

void TextPalette::frameColorChanged(QColor color)
      {
      if (color.isValid()) {
            _textElement->setFrameColor(color);
            mscore->activateWindow();
            _textElement->score()->addRefresh(_textElement->abbox().adjusted(-6, -6, 12, 12));
            _textElement->score()->end();
            }
      }

//---------------------------------------------------------
//   circleToggled
//---------------------------------------------------------

void TextPalette::circleToggled(bool val)
      {
      _textElement->setCircle(val);
      mscore->activateWindow();
      borderRounding->setEnabled(!val);
      _textElement->score()->addRefresh(_textElement->abbox().adjusted(-6, -6, 12, 12));
      _textElement->score()->end();
      }

