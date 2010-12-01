//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textpalette.cpp 3592 2010-10-18 17:24:18Z wschweer $
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

#include "texttools.h"
#include "icons.h"
#include "text.h"
#include "mscore.h"
#include "score.h"
#include "textpalette.h"

extern TextPalette* textPalette;

//---------------------------------------------------------
//   textTools
//---------------------------------------------------------

TextTools* MuseScore::textTools()
      {
      if (!_textTools) {
            _textTools = new TextTools(this);
            // addDockWidget(Qt::TopDockWidgetArea, _textTools);
            addDockWidget(Qt::BottomDockWidgetArea, _textTools);
            }
      setFocusPolicy(Qt::NoFocus);
      return _textTools;
      }

//---------------------------------------------------------
//   TextTools
//---------------------------------------------------------

TextTools::TextTools(QWidget* parent)
   : QDockWidget(parent)
      {
      _textElement = 0;
      setObjectName("text-tools");
      setWindowTitle(tr("Text Tools"));
      setAllowedAreas(Qt::TopDockWidgetArea | Qt::BottomDockWidgetArea);

      QToolBar* tb = new QToolBar(tr("Text Edit"));
      // tb->setObjectName("text-tools");

      toggleStyled = getAction("toggle-styled");
      tb->addAction(toggleStyled);
      toggleStyled->setCheckable(true);

      showKeyboard = getAction("show-keys");
      tb->addAction(showKeyboard);
      showKeyboard->setCheckable(true);

      typefaceBold = tb->addAction(*icons[textBold_ICON], "");
      typefaceBold->setToolTip(tr("bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = tb->addAction(*icons[textItalic_ICON], "");
      typefaceItalic->setToolTip(tr("italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = tb->addAction(*icons[textUnderline_ICON], "");
      typefaceUnderline->setToolTip(tr("underline"));
      typefaceUnderline->setCheckable(true);

      tb->addSeparator();

      leftAlign   = tb->addAction(*icons[textLeft_ICON],   "");
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);

      centerAlign = tb->addAction(*icons[textCenter_ICON], "");
      centerAlign->setToolTip(tr("align horizontal center"));
      centerAlign->setCheckable(true);

      rightAlign  = tb->addAction(*icons[textRight_ICON],  "");
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);

      topAlign  = tb->addAction(*icons[textTop_ICON],  "");
      topAlign->setToolTip(tr("align top"));
      topAlign->setCheckable(true);

      bottomAlign  = tb->addAction(*icons[textBottom_ICON],  "");
      bottomAlign->setToolTip(tr("align bottom"));
      bottomAlign->setCheckable(true);

      vcenterAlign  = tb->addAction(*icons[textVCenter_ICON],  "");
      vcenterAlign->setToolTip(tr("align vertical center"));
      vcenterAlign->setCheckable(true);

      typefaceSubscript   = tb->addAction(*icons[textSub_ICON], "");
      typefaceSubscript->setToolTip(tr("subscript"));

      typefaceSuperscript = tb->addAction(*icons[textSuper_ICON], "");
      typefaceSuperscript->setToolTip(tr("superscript"));

      typefaceSubscript->setCheckable(true);
      typefaceSuperscript->setCheckable(true);

      tb->addSeparator();

      typefaceFamily = new QFontComboBox(this);
      tb->addWidget(typefaceFamily);
      typefaceSize = new QDoubleSpinBox(this);
      tb->addWidget(typefaceSize);

      setWidget(tb);
      QWidget* w = new QWidget(this);
      setTitleBarWidget(w);
      titleBarWidget()->hide();

      connect(typefaceSize,        SIGNAL(valueChanged(double)), SLOT(sizeChanged(double)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(typefaceBold,        SIGNAL(triggered(bool)), SLOT(boldClicked(bool)));
      connect(typefaceItalic,      SIGNAL(triggered(bool)), SLOT(italicClicked(bool)));
      connect(typefaceUnderline,   SIGNAL(triggered(bool)), SLOT(underlineClicked(bool)));
      connect(typefaceSubscript,   SIGNAL(triggered(bool)), SLOT(subscriptClicked(bool)));
      connect(typefaceSuperscript, SIGNAL(triggered(bool)), SLOT(superscriptClicked(bool)));
      connect(typefaceFamily,      SIGNAL(currentFontChanged(const QFont&)), SLOT(fontChanged(const QFont&)));
      connect(leftAlign,           SIGNAL(triggered()), SLOT(setLeftAlign()));
      connect(rightAlign,          SIGNAL(triggered()), SLOT(setRightAlign()));
      connect(centerAlign,         SIGNAL(triggered()), SLOT(setHCenterAlign()));
      connect(topAlign,            SIGNAL(triggered()), SLOT(setTopAlign()));
      connect(bottomAlign,         SIGNAL(triggered()), SLOT(setBottomAlign()));
      connect(vcenterAlign,        SIGNAL(triggered()), SLOT(setVCenterAlign()));
      connect(showKeyboard,        SIGNAL(triggered(bool)), SLOT(showKeyboardClicked(bool)));
      connect(toggleStyled,        SIGNAL(triggered(bool)), SLOT(styledChanged(bool)));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextTools::setText(Text* te)
      {
      _textElement = te;
      styledChanged(te->styled());
      }

//---------------------------------------------------------
//   blockAllSignals
//---------------------------------------------------------

void TextTools::blockAllSignals(bool val)
      {
      typefaceSize->blockSignals(val);
      typefaceFamily->blockSignals(val);
      typefaceBold->blockSignals(val);
      typefaceItalic->blockSignals(val);
      typefaceUnderline->blockSignals(val);
      typefaceSubscript->blockSignals(val);
      typefaceSuperscript->blockSignals(val);
      typefaceFamily->blockSignals(val);
      leftAlign->blockSignals(val);
      rightAlign->blockSignals(val);
      centerAlign->blockSignals(val);
      topAlign->blockSignals(val);
      bottomAlign->blockSignals(val);
      vcenterAlign->blockSignals(val);
      showKeyboard->blockSignals(val);
      toggleStyled->blockSignals(val);
      }

//---------------------------------------------------------
//   setCharFormat
//---------------------------------------------------------

void TextTools::setCharFormat(const QTextCharFormat& cf)
      {
      blockAllSignals(true);

      format = cf;
      QFont f(cf.font());
      typefaceFamily->setCurrentFont(f);
      double ps = f.pointSizeF();
      if (ps == -1.0)
            ps = f.pixelSize() * PPI / DPI;
      typefaceSize->setValue(ps);
      typefaceItalic->setChecked(cf.fontItalic());
      typefaceBold->setChecked(cf.fontWeight() == QFont::Bold);
      typefaceUnderline->setChecked(cf.fontUnderline());
      typefaceSubscript->setChecked(cf.verticalAlignment() == QTextCharFormat::AlignSubScript);
      typefaceSuperscript->setChecked(cf.verticalAlignment() == QTextCharFormat::AlignSuperScript);

      blockAllSignals(false);
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void TextTools::setBlockFormat(const QTextBlockFormat& bf)
      {
      blockAllSignals(true);
      bformat = bf;
      centerAlign->setChecked(bf.alignment() & Qt::AlignHCenter);
      leftAlign->setChecked  (bf.alignment() & Qt::AlignLeft);
      rightAlign->setChecked (bf.alignment() & Qt::AlignRight);
      Align align = _textElement->align();
      if (align & ALIGN_BOTTOM) {
            topAlign->setChecked(false);
            bottomAlign->setChecked(true);
            vcenterAlign->setChecked(false);
            }
      else if (align & ALIGN_VCENTER) {
            topAlign->setChecked(false);
            bottomAlign->setChecked(false);
            vcenterAlign->setChecked(true);
            }
      else {
            topAlign->setChecked(true);
            bottomAlign->setChecked(false);
            vcenterAlign->setChecked(false);
            }
      blockAllSignals(false);
      }

//---------------------------------------------------------
//   sizeChanged
//---------------------------------------------------------

void TextTools::sizeChanged(double value)
      {
      format.setFontPointSize(value);
      _textElement->setCharFormat(format);
      _textElement->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   fontChanged
//---------------------------------------------------------

void TextTools::fontChanged(const QFont& f)
      {
      format.setFontFamily(f.family());
      _textElement->setCharFormat(format);
      _textElement->score()->setLayoutAll(true);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   boldClicked
//---------------------------------------------------------

void TextTools::boldClicked(bool val)
      {
      format.setFontWeight(val ? QFont::Bold : QFont::Normal);
      _textElement->setCharFormat(format);
      _textElement->score()->setLayoutAll(true);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   underlineClicked
//---------------------------------------------------------

void TextTools::underlineClicked(bool val)
      {
      format.setFontUnderline(val);
      _textElement->setCharFormat(format);
      _textElement->score()->setLayoutAll(true);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   italicClicked
//---------------------------------------------------------

void TextTools::italicClicked(bool val)
      {
      format.setFontItalic(val);
      _textElement->setCharFormat(format);
      _textElement->score()->setLayoutAll(true);
      _textElement->score()->end();
      }

//---------------------------------------------------------
//   setHCenterAlign
//---------------------------------------------------------

void TextTools::setHCenterAlign()
      {
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignHCenter);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   setLeftAlign
//---------------------------------------------------------

void TextTools::setLeftAlign()
      {
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignLeft);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   setRightAlign
//---------------------------------------------------------

void TextTools::setRightAlign()
      {
      bformat.setAlignment((bformat.alignment() & ~Qt::AlignHorizontal_Mask) | Qt::AlignRight);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   setTopAlign
//---------------------------------------------------------

void TextTools::setTopAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_HMASK) | ALIGN_TOP;
      _textElement->setAlign(align);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   setBottomAlign
//---------------------------------------------------------

void TextTools::setBottomAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_HMASK) | ALIGN_BOTTOM;
      _textElement->setAlign(align);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   setVCenterAlign
//---------------------------------------------------------

void TextTools::setVCenterAlign()
      {
      Align align = (_textElement->align() & ~ALIGN_HMASK) | ALIGN_VCENTER;
      _textElement->setAlign(align);
      setBlockFormat(bformat);
      }

//---------------------------------------------------------
//   subscriptClicked
//---------------------------------------------------------

void TextTools::subscriptClicked(bool val)
      {
      typefaceSuperscript->blockSignals(true);
      typefaceSuperscript->setChecked(false);
      typefaceSuperscript->blockSignals(false);
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSubScript : QTextCharFormat::AlignNormal);
      _textElement->setCharFormat(format);
      }

//---------------------------------------------------------
//   superscriptClicked
//---------------------------------------------------------

void TextTools::superscriptClicked(bool val)
      {
      typefaceSubscript->blockSignals(true);
      typefaceSubscript->setChecked(false);
      typefaceSubscript->blockSignals(false);
      format.setVerticalAlignment(val ? QTextCharFormat::AlignSuperScript : QTextCharFormat::AlignNormal);
      _textElement->setCharFormat(format);
      }

//---------------------------------------------------------
//   styledChanged
//---------------------------------------------------------

void TextTools::styledChanged(bool styled)
      {
      blockAllSignals(true);
      _textElement->setStyled(styled);
      bool unstyled = !styled;
      typefaceSize->setEnabled(unstyled);
      typefaceFamily->setEnabled(unstyled);
      typefaceBold->setEnabled(unstyled);
      typefaceItalic->setEnabled(unstyled);
      typefaceUnderline->setEnabled(unstyled);
      typefaceSubscript->setEnabled(unstyled);
      typefaceSuperscript->setEnabled(unstyled);
      typefaceFamily->setEnabled(unstyled);
      leftAlign->setEnabled(unstyled);
      rightAlign->setEnabled(unstyled);
      centerAlign->setEnabled(unstyled);
      topAlign->setEnabled(unstyled);
      bottomAlign->setEnabled(unstyled);
      vcenterAlign->setEnabled(unstyled);
      toggleStyled->setChecked(styled);
      blockAllSignals(false);
      }

//---------------------------------------------------------
//   showKeyboardClicked
//---------------------------------------------------------

void TextTools::showKeyboardClicked(bool val)
      {
      if (val) {
            if (textPalette == 0)
                  textPalette = new TextPalette(mscore);
            textPalette->setText(_textElement);
            textPalette->show();
            }
      else {
            if (textPalette)
                  textPalette->hide();
            }
      }

