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
//   codeIcon
//---------------------------------------------------------

static QIcon codeIcon(const QString& s, QFont f)
      {
      f.setPixelSize(35);
      int w = 40;
      int h = 40;

      QWidget wi;

      QPixmap image(w, h);
      QColor bg(wi.palette().brush(QPalette::Normal, QPalette::Window).color());

      image.fill(QColor(255, 255, 255, 0));
      QPainter painter(&image);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setFont(f);

      QPen pen(wi.palette().brush(QPalette::Normal, QPalette::Text).color());

      painter.setPen(pen);
      painter.drawText(0, 0, w, h, Qt::AlignCenter, s);
      painter.end();
      return QIcon(image);
      }

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

      typefaceBold = tb->addAction(QIcon(":/data/text_bold.svg"), "");
      typefaceBold->setToolTip(tr("bold"));
      typefaceBold->setCheckable(true);

      typefaceItalic = tb->addAction(QIcon(":/data/text_italic.svg"), "");
      typefaceItalic->setToolTip(tr("italic"));
      typefaceItalic->setCheckable(true);

      typefaceUnderline = tb->addAction(QIcon(":/data/text_under.svg"), "");
      typefaceUnderline->setToolTip(tr("underline"));
      typefaceUnderline->setCheckable(true);

      tb->addSeparator();

      leftAlign   = tb->addAction(QIcon(":/data/text_left.svg"),   "");
      leftAlign->setToolTip(tr("align left"));
      leftAlign->setCheckable(true);

      centerAlign = tb->addAction(QIcon(":/data/text_center.svg"), "");
      centerAlign->setToolTip(tr("align center"));
      centerAlign->setCheckable(true);

      rightAlign  = tb->addAction(QIcon(":/data/text_right.svg"),  "");
      rightAlign->setToolTip(tr("align right"));
      rightAlign->setCheckable(true);

      typefaceSubscript   = tb->addAction(QIcon(":/data/subscript.svg"), "");
      typefaceSubscript->setToolTip(tr("subscript"));

      typefaceSuperscript = tb->addAction(QIcon(":/data/superscript.svg"), "");
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
//   setupPalette
//---------------------------------------------------------

static void setupPalette(QFrame* f, QButtonGroup* sg, int offset)
      {
      QGridLayout* gl = new QGridLayout;
      gl->setMargin(5);
      gl->setSpacing(1);
      f->setLayout(gl);
      for (unsigned i = 0; i < 256; ++i) {
            QToolButton* tb = new QToolButton;
            tb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            tb->setFixedSize(40, 40);
            tb->setText(QChar(i + offset));
            gl->addWidget(tb, i / 16, i % 16);
            sg->addButton(tb, i + offset);
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
      sg = new QButtonGroup(this);

      musicalSymbols->setChecked(true);

      for (unsigned i = 0; i < 256; ++i) {
            QPushButton* tb = new QPushButton;
            buttons[i] = tb;
            tb->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
            tb->setFixedSize(40, 40);
            gl->addWidget(tb, i / 16, i % 16);
            sg->addButton(tb, i);
            }
      curPage = 0;
      populate();
      connect(sg, SIGNAL(buttonClicked(int)), SLOT(symbolClicked(int)));
      connect(musicalSymbols, SIGNAL(toggled(bool)), SLOT(populate()));
      connect(codePage, SIGNAL(valueChanged(int)), SLOT(populate()));
      setFocusPolicy(Qt::NoFocus);
      }

//---------------------------------------------------------
//   populate
//---------------------------------------------------------

void TextPalette::populate()
      {
      bool musical = musicalSymbols->isChecked();

      QFont f(musical ? "MScore1-test" : "DejaVuSerif");
      codePage->setEnabled(!musical);

// f.setStyleStrategy(QFont::NoFontMerging);
      QFontMetrics fm(f);
      int page = musical ? 0x1d1 : codePage->value();

      int rowOffset = 0;
      bool pageEmpty = true;

      for (int row = 0; row < 16; ++row) {
            bool rowEmpty = true;
            for (int col = 0; col < 16; ++col) {
                  int idx = (row - rowOffset) * 16 + col;
                  int code = row * 16 + col + page * 256;
                  QPushButton* tb = buttons[idx];
                  //
                  // Font->inFont(QChar) does only work
                  // for unicode plane 0, as QChar is only
                  // 16 bit
                  //
                  if ((code & 0xffff0000) || fm.inFont(QChar(code))) {
                        tb->setFont(f);
                        rowEmpty = false;
                        QString ss;
                        if (code & 0xffff0000) {
                              ss = QChar(QChar::highSurrogate(code));
                              ss += QChar(QChar::lowSurrogate(code));
                              tb->setToolTip(QString("0x%1").arg(code, 5, 16, QLatin1Char('0')));
                              QIcon icon(codeIcon(ss, f));
                              tb->setText("");
                              tb->setIcon(icon);
                              }
                        else {
                              ss = QChar(code);
                              tb->setToolTip(QString("0x%1").arg(code, 4, 16, QLatin1Char('0')));
                              tb->setText(ss);
                              tb->setIcon(QIcon());
                              }
                        }
                  else {
                        tb->setText("");
                        tb->setIcon(QIcon());
                        }
                  sg->setId(tb, code);
                  }
            if (rowEmpty)
                  ++rowOffset;
            else
                  pageEmpty = false;
            }
      for (int row = 16-rowOffset; row < 16; ++row) {
            for (int col = 0; col < 16; ++col) {
                  int idx = row * 16 + col;
                  QPushButton* tb = buttons[idx];
                  tb->setText("");
                  tb->setIcon(QIcon());
                  sg->setId(tb, -1);
                  tb->setToolTip(QString(""));
                  }
            }
      if (pageEmpty) {
            int diff = 1;
            if (curPage > page)
                  diff = -1;
            curPage = page;
            page += diff;
            codePage->setValue(page);
            }
      else
            curPage = page;
      }

//---------------------------------------------------------
//   symbolClicked
//---------------------------------------------------------

void TextPalette::symbolClicked(int n)
      {
      if (n == -1)
            return;
      _textElement->addChar(n);
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
