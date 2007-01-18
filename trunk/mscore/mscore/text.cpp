//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
#include "text.h"
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
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : Element(s)
      {
      textStyle = -1;
      doc = new QTextDocument(0);
      setStyle(TEXT_STYLE_LYRIC);
      editMode = false;
      cursor = new QTextCursor(doc);
      cursorPos = 0;
      cursor->setPosition(cursorPos);
      }

Text::Text(Score* s, int style)
   : Element(s)
      {
      textStyle = -1;
      doc = new QTextDocument(0);
      setStyle(style);
      editMode = false;
      cursor = new QTextCursor(doc);
      cursorPos = 0;
      cursor->setPosition(cursorPos);
      }

Text::Text(const Text& e)
   : Element(e)
      {
      doc       = e.doc->clone(0);
      editMode  = e.editMode;
      textStyle = e.textStyle;
      cursorPos = e.cursorPos;
      cursor    = new QTextCursor(doc);
      cursor->setCharFormat(e.cursor->charFormat());
      cursor->setPosition(e.cursor->position());
      }

Text::~Text()
      {
      delete doc;
      delete cursor;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

const QRectF& Text::bbox() const
      {
      _bbox = QRectF(0.0, 0.0, doc->size().width(), doc->size().height());
      return _bbox;
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void Text::resetMode()
      {
      editMode = 0;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Text::isEmpty() const
      {
      return doc->isEmpty();
      }

//---------------------------------------------------------
//   text
//---------------------------------------------------------

QString Text::getText() const
      {
      return doc->toPlainText();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
      if (parent() == 0)
            return;
      doc->documentLayout()->setPaintDevice(score()->scoreLayout()->paintDevice());
      doc->setUseDesignMetrics(true);

      TextStyle* s = &textStyles[textStyle];

      double tw = bbox().width();
      double th = bbox().height();

      QPointF _off(QPointF(s->xoff, s->yoff));
      if (s->offsetType == OFFSET_SPATIUM)
            _off *= _score->spatium();
      else
            _off *= DPI;

      double x = 0.0, y = 0.0;
      if (s->anchor == ANCHOR_PAGE) {
            Page* page = (Page*)parent();
            if (parent()->type() != PAGE) {
                  printf("fatal: text parent is not PAGE\n");
                  return;
                  }
            double w = page->loWidth() - page->lm() - page->rm();
            double h = page->loHeight() - page->tm() - page->bm();

            if (s->offsetType == OFFSET_REL) {
                  _off = QPointF(s->xoff * w * 0.01, s->yoff * h * 0.01);
                  }
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

void Text::setText(const QString& s)
      {
      doc->clear();
      QTextCharFormat format;
      format.setFont(font());
      cursor->setPosition(0);
      cursor->insertText(s, format);
      layout();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Text::setStyle(int n)
      {
      if (textStyle != n) {
            textStyle = n;
            doc->setDefaultFont(font());
            layout();
            }
      }

//---------------------------------------------------------
//   Text::write
//---------------------------------------------------------

void Text::write(Xml& xml) const
      {
      write(xml, "Text");
      }

//---------------------------------------------------------
//   Text::write
//---------------------------------------------------------

void Text::write(Xml& xml, const char* name) const
      {
      if (doc->isEmpty())
            return;
      xml.stag(name);
      writeProperties(xml);
      xml.etag(name);
      }

//---------------------------------------------------------
//   Text::read
//---------------------------------------------------------

void Text::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement())
                  continue;
            if (!readProperties(node))
                  domError(node);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml) const
      {
      xml.tag("style", textStyle);
      QString s = doc->toHtml("utf8");
      xml.tag("data", s);
      Element::writeProperties(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(QDomNode node)
      {
      cursor->setPosition(0);
      QTextCharFormat f = cursor->charFormat();
      f.setFont(doc->defaultFont());
      cursor->setCharFormat(f);
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "style")
            textStyle = val.toInt();
      else if (tag == "data")
            doc->setHtml(val);
      else
            return false;
      cursor->setPosition(0);
      layout();
      return true;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Text::startEdit(QMatrix&)
      {
      cursor->setPosition(cursorPos);
#if 0
      QTextCharFormat f = cursor->charFormat();
      QFont font(doc->defaultFont());
      f.setFont(font);
      cursor->setCharFormat(f);
#endif
      editMode = true;
      if (palette)
            palette->setCharFormat(cursor->charFormat());
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Text::edit(QKeyEvent* ev)
      {
      int key = ev->key();
      if (key == Qt::Key_F2) {
            if (palette == 0)
                  palette = new TextPalette(0);
            if (palette->isVisible())
                  palette->hide();
            else {
                  palette->setText(this);
                  palette->show();
                  palette->setCharFormat(cursor->charFormat());
                  }
            return false;
            }
      if (ev->modifiers() & Qt::CTRL) {
            switch (key) {
                  case Qt::Key_B:   // toggle bold face
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontWeight(f.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                        if (palette)
                              palette->setCharFormat(f);
                        cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_I:   // toggle italic
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontItalic(!f.fontItalic());
                        if (palette)
                              palette->setCharFormat(f);
                        cursor->setCharFormat(f);
                        }
                        break;
                  }
            return false;
            }
      switch (key) {
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
      if (palette)
            palette->setCharFormat(cursor->charFormat());
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
      {
      cursorPos = cursor->position();
      if (palette)
            palette->hide();
      editMode = false;
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Text::font() const
      {
      return textStyles[textStyle].font();
      }

//---------------------------------------------------------
//   Text::draw
//---------------------------------------------------------

void Text::draw1(Painter& p)
      {
      p.save();
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setFont(font());

      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = editMode ? cursor->position() : -1;
      QColor color = p.pen().color();
      c.palette.setColor(QPalette::Text, color);
      doc->documentLayout()->draw(&p, c);

      p.restore();
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double Text::lineSpacing() const
      {
      QFontMetricsF fm(doc->defaultFont());
      return fm.lineSpacing();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Text::addSymbol(const SymCode& s)
      {
// printf("Text: addSymbol(%x)\n", s.code);
      QTextCharFormat oFormat = cursor->charFormat();
      QTextCharFormat nFormat(oFormat);
      if (s.style >= 0)
            nFormat.setFontFamily(textStyles[s.style].font().family());
      cursor->setCharFormat(nFormat);
//      QString str(s.code);
      cursor->insertText(s.code);
      cursor->setCharFormat(oFormat);
      score()->layout();
      score()->endCmd(false);
      }

//---------------------------------------------------------
//   setCharFormat
//---------------------------------------------------------

void Text::setCharFormat(QTextCharFormat f)
      {
      cursor->setCharFormat(f);
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s, TEXT_STYLE_LYRIC)
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
   : Text(s, TEXT_STYLE_FINGERING)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Fingering::write(Xml& xml) const
      {
      Text::write(xml, "Fingering");
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
   : Text(s, TEXT_STYLE_INSTRUMENT_LONG)
      {
      }

//---------------------------------------------------------
//   InstrumentName2
//---------------------------------------------------------

InstrumentName2::InstrumentName2(Score* s)
   : Text(s, TEXT_STYLE_INSTRUMENT_SHORT)
      {
      }

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s, TEXT_STYLE_TEMPO)
      {
      _tempo = 120.0;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TempoText::write(Xml& xml) const
      {
      xml.stag("Tempo");
      xml.tag("tempo", _tempo);
      Text::writeProperties(xml);
      xml.etag("Tempo");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(QDomNode node)
      {
      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (!node.isElement())
                  continue;
            QDomElement e = node.toElement();
            QString tag(e.tagName());
            if (tag == "tempo")
                  setTempo(e.text().toDouble());
            else if (Text::readProperties(node))
                  ;
            else
                  domError(node);
            }
      layout();
      }

