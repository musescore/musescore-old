//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: textelement.cpp,v 1.6 2006/04/05 08:15:12 wschweer Exp $
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

#include "globals.h"
#include "textelement.h"
#include "xml.h"
#include "style.h"
#include "mscore.h"
#include "canvas.h"
#include "score.h"
#include "painter.h"
#include "utils.h"
#include "page.h"
#include "textpalette.h"
#include "sym.h"
#include "symbol.h"

TextPalette* palette;

//---------------------------------------------------------
//   TextElement
//---------------------------------------------------------

TextElement::TextElement(Score* s)
   : Element(s)
      {
      textStyle = TEXT_STYLE_LYRIC;
      doc.setDefaultFont(font());
      editMode = false;
      cursor = new QTextCursor(&doc);
      cursor->setPosition(0);
      }

TextElement::TextElement(Score* s, int style)
   : Element(s)
      {
      textStyle = style;
      doc.setDefaultFont(font());
      editMode = false;
      cursor = new QTextCursor(&doc);
      cursor->setPosition(0);
      }

TextElement::~TextElement()
      {
      delete cursor;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF& TextElement::bbox() const
      {
      _bbox = QRectF(0.0, 0.0, doc.size().width(), doc.size().height());
// printf("TextElement<%s>: bbox %f %f\n", doc.toPlainText().toLocal8Bit().data(),
//   _bbox.width(), _bbox.height());
      return _bbox;
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void TextElement::resetMode()
      {
      editMode = 0;
      }

//---------------------------------------------------------
//   setSelected
//---------------------------------------------------------

void TextElement::setSelected(bool val)
      {
      Element::setSelected(val);
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool TextElement::isEmpty() const
      {
      return doc.isEmpty();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString TextElement::getText() const
      {
      return doc.toPlainText();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextElement::layout()
      {
      if (parent() == 0)
            return;
      TextStyle* s = &textStyles[textStyle];

      double tw = bbox().width();
      double th = bbox().height();

      QPointF _off(QPointF(s->xoff, s->yoff));
      if (s->offsetType == OFFSET_SPATIUM)
            _off *= _score->spatium();

      double x = 0.0, y = 0.0;
      if (s->anchor == ANCHOR_PAGE) {
            Page* page = (Page*)parent();
            if (parent()->type() != PAGE) {
                  printf("fatal: text parent is not PAGE\n");
                  return;
                  }
            double w = page->loWidth() - page->lm() - page->rm();
            double h = page->loHeight() - page->tm() - page->bm();

            if (s->offsetType == OFFSET_REL)
                  _off = QPointF(s->xoff * w * 0.01, s->yoff * h * 0.01);

            if (s->align & ALIGN_LEFT)
                  x = page->lm();
            else if (s->align & ALIGN_RIGHT)
                  x  = page->lm() + w - tw;
            else if (s->align & ALIGN_HCENTER)
                  x  = page->lm() + w * .5 - tw * .5;
            if (s->align & ALIGN_TOP)
                  y = page->tm();
            else if (s->align & ALIGN_BOTTOM)
                  y = page->tm() + h;
            else if (s->align & ALIGN_VCENTER)
                  y = page->tm() + h * .5 - th * .5;
            }
      else {
            if (s->align & ALIGN_LEFT)
                  ;
            else if (s->align & ALIGN_RIGHT)
                  x  = -tw;
            else if (s->align & ALIGN_HCENTER)
                  x  = -(tw *.5);
            if (s->align & ALIGN_TOP)
                  ;
            else if (s->align & ALIGN_BOTTOM)
                  y = -th;
            else if (s->align & ALIGN_VCENTER) {
                  y = -(th * .5) - bbox().y();
                  }
            }
      setPos(x + _off.x(), y + _off.y());
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextElement::setText(const QString& s)
      {
      doc.setPlainText(s);
      layout();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void TextElement::setStyle(int n)
      {
      if (textStyle != n) {
            textStyle = n;
            doc.setDefaultFont(font());
            layout();
            }
      }

//---------------------------------------------------------
//   TextElement::write
//---------------------------------------------------------

void TextElement::write(Xml& xml) const
      {
      write(xml, "Text");
      }

//---------------------------------------------------------
//   TextElement::write
//---------------------------------------------------------

void TextElement::write(Xml& xml, const char* name) const
      {
      if (doc.isEmpty())
            return;
      xml.stag(name);
      xml.tag("style", textStyle);

      QString s = doc.toHtml("utf8");
      xml.tag("data", s);
// printf("TextElement: write<%s>\n", s.toLocal8Bit().data());
      Element::writeProperties(xml);
      xml.etag(name);
      }

//---------------------------------------------------------
//   TextElement::read
//---------------------------------------------------------

void TextElement::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement())
                  continue;
            QDomElement e = node.toElement();
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "data") {
// printf("setHtml <%s>\n", val.toLocal8Bit().data());
                  doc.setHtml(val);
                  }
            else if (tag == "style") {
                  textStyle = val.toInt();
                  }
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      layout();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool TextElement::startEdit(QMatrix&)
      {
      editMode = true;
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool TextElement::edit(QKeyEvent* ev)
      {
      if (ev->key() == Qt::Key_F2) {
            if (palette == 0)
                  palette = new TextPalette(0);
            if (palette->isVisible())
                  palette->hide();
            else {
                  palette->setTextElement(this);
                  palette->show();
                  mscore->activateWindow();
                  }
            return false;
            }
      switch (ev->key()) {
            case Qt::Key_Return:
                  cursor->insertText(QString("\n"));
                  break;

            case Qt::Key_Backspace:
                  cursor->deletePreviousChar();
                  break;

            case Qt::Key_Delete:
                  //TODO
                  break;

            case Qt::Key_Left:
                  cursor->movePosition(QTextCursor::Left);
                  break;

            case Qt::Key_Right:
                  cursor->movePosition(QTextCursor::Right);
                  break;

            case Qt::Key_Up:
                  cursor->movePosition(QTextCursor::Up);
                  break;

            case Qt::Key_Down:
                  cursor->movePosition(QTextCursor::Down);
                  break;

            case Qt::Key_Home:
                  cursor->movePosition(QTextCursor::Start);
                  break;

            case Qt::Key_End:
                  cursor->movePosition(QTextCursor::End);
                  break;

            default:
                  cursor->insertText(ev->text());
                  break;
            }
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextElement::endEdit()
      {
      if (palette)
            palette->hide();
      editMode = false;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont TextElement::font() const
      {
      TextStyle* s = &textStyles[textStyle];
      QFont f;
      f.setFamily(s->family);
      f.setItalic(s->italic);
      f.setUnderline(s->underline);
      f.setBold(s->bold);
      f.setPixelSize(lrint(s->size * _spatium * .2));
      return f;
      }

//---------------------------------------------------------
//   TextElement::draw
//---------------------------------------------------------

void TextElement::draw1(Painter& p)
      {
      p.save();
      p.setRenderHint(QPainter::Antialiasing, false);
#if 1
      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = editMode ? cursor->position() : -1;

      QAbstractTextDocumentLayout* layout = doc.documentLayout();

      doc.documentLayout()->draw(&p, c);
#endif
      p.restore();
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double TextElement::lineSpacing() const
      {
      QFontMetrics fm(doc.defaultFont());
      return fm.lineSpacing();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void TextElement::addSymbol(int n)
      {
printf("TextElement: addSymbol(%x)\n", n);
      QTextCharFormat format;
      format.setFont(symbols[n].font());
      cursor->insertText(QString(symbols[n].code()), format);
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : TextElement(s, TEXT_STYLE_LYRIC)
      {
      _no = 0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Lyrics::write(Xml& xml) const
      {
      xml.stag("Lyrics");
      xml.tag("data", getText());
      if (_no)
            xml.tag("no", _no);
      Element::writeProperties(xml);
      xml.etag("Lyrics");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Lyrics::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());
            int i = val.toInt();
            if (tag == "data")
                  setText(val);
            else if (tag == "no")
                  _no = i;
            else if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

Fingering::Fingering(Score* s)
   : TextElement(s, TEXT_STYLE_FINGERING)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      TextElement::write(xml, "Fingering");
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Fingering::setSubtype(int n)
      {
      Element::setSubtype(n);
      switch(n) {
            case 1: setText("1"); break;
            case 2: setText("2"); break;
            case 3: setText("3"); break;
            case 4: setText("4"); break;
            case 5: setText("5"); break;
            }
      }

//---------------------------------------------------------
//   InstrumentName1
//---------------------------------------------------------

InstrumentName1::InstrumentName1(Score* s)
   : TextElement(s, TEXT_STYLE_INSTRUMENT_LONG)
      {
      }

//---------------------------------------------------------
//   InstrumentName2
//---------------------------------------------------------

InstrumentName2::InstrumentName2(Score* s)
   : TextElement(s, TEXT_STYLE_INSTRUMENT_SHORT)
      {
      }

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : TextElement(s, TEXT_STYLE_TEMPO)
      {
      }

