//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
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
#include "scoreview.h"

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

      showKeyboard = getAction("show-keys");
      tb->addAction(showKeyboard);
      showKeyboard->setCheckable(true);

      typefaceBold      = tb->addAction(QIcon(":/data/text_bold.svg"), "");
      typefaceItalic    = tb->addAction(QIcon(":/data/text_italic.svg"), "");
      typefaceUnderline = tb->addAction(QIcon(":/data/text_under.svg"), "");
      typefaceBold->setCheckable(true);
      typefaceItalic->setCheckable(true);
      typefaceUnderline->setCheckable(true);
      tb->addSeparator();
      leftAlign   = tb->addAction(QIcon(":/data/text_left.svg"),   "");
      centerAlign = tb->addAction(QIcon(":/data/text_center.svg"), "");
      rightAlign  = tb->addAction(QIcon(":/data/text_right.svg"),  "");
      leftAlign->setCheckable(true);
      centerAlign->setCheckable(true);
      rightAlign->setCheckable(true);
      typefaceSubscript   = tb->addAction(QIcon(":/data/subscript.svg"), "");
      typefaceSuperscript = tb->addAction(QIcon(":/data/superscript.svg"), "");
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
      connect(showKeyboard,        SIGNAL(triggered(bool)), SLOT(showKeyboardClicked(bool)));
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextTools::setText(TextB* te)
      {
      _textElement = te;
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
      showKeyboard->blockSignals(val);
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
//      _textElement->score()->end();
      moveFocus();
      }

//---------------------------------------------------------
//   moveFocus
//---------------------------------------------------------

void TextTools::moveFocus()
      {
      mscore->currentScoreView()->update();
//TODO-S      _textElement->score()->canvas()->setFocus();
//cannot work:      mscore->currentScoreView()->setFocus();
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
      moveFocus();
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
      moveFocus();
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
      moveFocus();
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
      moveFocus();
      }

//---------------------------------------------------------
//   setHCenterAlign
//---------------------------------------------------------

void TextTools::setHCenterAlign()
      {
      bformat.setAlignment(Qt::AlignHCenter);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      moveFocus();
      }

//---------------------------------------------------------
//   setLeftAlign
//---------------------------------------------------------

void TextTools::setLeftAlign()
      {
      bformat.setAlignment(Qt::AlignLeft);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      moveFocus();
      }

//---------------------------------------------------------
//   setRightAlign
//---------------------------------------------------------

void TextTools::setRightAlign()
      {
      bformat.setAlignment(Qt::AlignRight);
      _textElement->setBlockFormat(bformat);
      setBlockFormat(bformat);
      moveFocus();
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
      moveFocus();
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
      moveFocus();
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

//---------------------------------------------------------
//   TextPalette
//---------------------------------------------------------

TextPalette::TextPalette(QWidget* parent)
   : QWidget(parent)
      {
      setWindowFlags(Qt::Tool);
      setupUi(this);
      QGridLayout* gl = new QGridLayout;
      gl->setMargin(5);
      gl->setSpacing(1);
      symbolBox->setLayout(gl);
      QButtonGroup* sg = new QButtonGroup(this);

      unsigned int buttonIndex = 0;
      for (unsigned i = 0; pSymbols[i].code != -1; ++i) {
            if (!pSymbols[i].show)
                  continue;
            if (pSymbols[i].code == 0) {    // empty slot?
                  ++buttonIndex;
                  continue;
                  }
            QToolButton* tb = new QToolButton;
            tb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            tb->setFixedSize(40, 40);

            SymCode sc(pSymbols[i]);
            if (sc.fontId == -1)
                  tb->setText(sc.code);
            else {
                  Sym sym("", sc.code.unicode(), sc.fontId);
                  QIcon* icon = symIcon(sym, 25, 35, 35);
                  tb->setIconSize(QSize(35, 35));
                  tb->setIcon(*icon);
                  }
            gl->addWidget(tb, buttonIndex / 16, buttonIndex % 16);
            sg->addButton(tb, i);
            ++buttonIndex;
            }
      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));
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
//   setText
//---------------------------------------------------------

void TextPalette::setText(TextB* te)
      {
      _textElement = te;
      }

//---------------------------------------------------------
//   closeEvent
//---------------------------------------------------------

void TextPalette::closeEvent(QCloseEvent* ev)
      {
      QWidget::closeEvent(ev);
      getAction("show-keys")->setChecked(false);
      }
