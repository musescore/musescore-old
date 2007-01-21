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
      _align                  = ALIGN_LEFT;
      _xoff                   = 0;
      _yoff                   = 0;
      _anchor                 = ANCHOR_TICK;
      _offsetType             = OFFSET_SPATIUM;
      _sizeIsSpatiumDependent = true;
      editMode                = false;
      cursorPos               = 0;
      doc                     = new QTextDocument(0);
      cursor                  = new QTextCursor(doc);
      cursor->movePosition(QTextCursor::Start);
      }

Text::Text(const Text& e)
   : Element(e)
      {
      _align                  = e._align;
      _xoff                   = e._xoff;
      _yoff                   = e._yoff;
      _anchor                 = e._anchor;
      _offsetType             = e._offsetType;
      _sizeIsSpatiumDependent = e._sizeIsSpatiumDependent;
      editMode                = e.editMode;
      cursorPos               = e.cursorPos;
      doc                     = e.doc->clone(0);
      cursor                  = new QTextCursor(doc);
      cursor->setPosition(e.cursor->position());
      cursor->setCharFormat(e.cursor->charFormat());
      cursor->setBlockFormat(e.cursor->blockFormat());
      }

Text::~Text()
      {
      delete doc;
      delete cursor;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Text::setSubtype(int val)
      {
      Element::setSubtype(val);
      switch (val) {
            case TEXT_TITLE:            setStyle(TEXT_STYLE_TITLE); break;
            case TEXT_SUBTITLE:         setStyle(TEXT_STYLE_SUBTITLE); break;
            case TEXT_COMPOSER:         setStyle(TEXT_STYLE_COMPOSER); break;
            case TEXT_POET:             setStyle(TEXT_STYLE_POET); break;
            case TEXT_TRANSLATOR:       setStyle(TEXT_STYLE_TRANSLATOR); break;
            case TEXT_MEASURE_NUMBER:   setStyle(TEXT_STYLE_MEASURE_NUMBER); break;
            case TEXT_PAGE_NUMBER_ODD:  setStyle(TEXT_STYLE_PAGE_NUMBER_ODD); break;
            case TEXT_PAGE_NUMBER_EVEN: setStyle(TEXT_STYLE_PAGE_NUMBER_EVEN); break;
            case TEXT_COPYRIGHT:        setStyle(TEXT_STYLE_COPYRIGHT); break;
            case TEXT_FINGERING:        setStyle(TEXT_STYLE_FINGERING); break;
            case TEXT_INSTRUMENT_LONG:  setStyle(TEXT_STYLE_INSTRUMENT_LONG); break;
            case TEXT_INSTRUMENT_SHORT: setStyle(TEXT_STYLE_INSTRUMENT_SHORT); break;
            case TEXT_TEMPO:            setStyle(TEXT_STYLE_TEMPO); break;
            case TEXT_LYRIC:            setStyle(TEXT_STYLE_LYRIC); break;
            }
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString Text::subtypeName() const
      {
      switch (subtype()) {
            case TEXT_TITLE:            return "Title";
            case TEXT_SUBTITLE:         return "Subtitle";
            case TEXT_COMPOSER:         return "Composer";
            case TEXT_POET:             return "Poet";
            case TEXT_TRANSLATOR:       return "Translator";
            case TEXT_MEASURE_NUMBER:   return "MeasureNumber";
            case TEXT_PAGE_NUMBER_ODD:  return "PageNoOdd";
            case TEXT_PAGE_NUMBER_EVEN: return "PageNoEven";
            case TEXT_COPYRIGHT:        return "Copyright";
            case TEXT_FINGERING:        return "Fingering";
            case TEXT_INSTRUMENT_LONG:  return "InstrumentLong";
            case TEXT_INSTRUMENT_SHORT: return "InstrumentShort";
            case TEXT_TEMPO:            return "Tempo";
            case TEXT_LYRIC:            return "Lyric";
            }
      return "?";
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Text::setSubtype(const QString& s)
      {
      int st = 0;
      if (s == "Title")
            st = TEXT_TITLE;
      else if (s == "Subtitle")
            st = TEXT_SUBTITLE;
      else if (s == "Composer")
            st = TEXT_COMPOSER;
      else if (s == "Poet")
            st = TEXT_POET;
      else if (s == "Translator")
            st = TEXT_TRANSLATOR;
      else if (s == "MeasureNumber")
            st = TEXT_MEASURE_NUMBER;
      else if (s == "PageNoOdd")
            st = TEXT_PAGE_NUMBER_ODD;
      else if (s == "PageNoEven")
            st = TEXT_PAGE_NUMBER_EVEN;
      else if (s == "Copyright")
            st = TEXT_COPYRIGHT;
      else if (s == "Fingering")
            st = TEXT_FINGERING;
      else if (s == "InstrumentLong")
            st = TEXT_INSTRUMENT_LONG;
      else if (s == "InstrumentShort")
            st = TEXT_INSTRUMENT_SHORT;
      else if (s == "Tempo")
            st = TEXT_TEMPO;
      else if (s == "Lyric")
            st = TEXT_LYRIC;
      setSubtype(st);
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
      // doc->adjustSize();

      double tw = bbox().width();
      double th = bbox().height();

      QPointF _off(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            _off *= _score->spatium();
      else
            _off *= DPI;

      double x = 0.0, y = 0.0;
      if (_anchor == ANCHOR_PAGE) {
            Page* page = (Page*)parent();
            if (parent()->type() != PAGE) {
                  printf("fatal: text parent is not PAGE <%s>\n", getText().toLocal8Bit().data());
                  return;
                  }
            double w = page->loWidth() - page->lm() - page->rm();
            doc->setTextWidth(w);
            double h = page->loHeight() - page->tm() - page->bm();

            if (_offsetType == OFFSET_REL) {
                  _off = QPointF(_xoff * w * 0.01, _yoff * h * 0.01);
                  }
            if (_align & ALIGN_LEFT)
                  x = page->lm();
            else if (_align & ALIGN_RIGHT)
                  x  = page->lm() + w - tw;
            else if (_align & ALIGN_HCENTER)
                  x  = page->lm() + w * .5 - tw * .5;
            if (_align & ALIGN_TOP)
                  y = page->tm();
            else if (_align & ALIGN_BOTTOM)
                  y = page->tm() + h;
            else if (_align & ALIGN_VCENTER)
                  y = page->tm() + h * .5 - th * .5;
            }
      else {
            if (_align & ALIGN_LEFT)
                  ;
            else if (_align & ALIGN_RIGHT)
                  x  = -tw;
            else if (_align & ALIGN_HCENTER)
                  x  = -(tw *.5);
            if (_align & ALIGN_TOP)
                  ;
            else if (_align & ALIGN_BOTTOM)
                  y = -th;
            else if (_align & ALIGN_VCENTER) {
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
      cursor->movePosition(QTextCursor::Start);
      if (_align & ALIGN_HCENTER) {
            QTextBlockFormat bf = cursor->blockFormat();
            bf.setAlignment(Qt::AlignHCenter);
            cursor->setBlockFormat(bf);
            }
      QTextCharFormat tf = cursor->charFormat();
      tf.setFont(doc->defaultFont());
      cursor->setCharFormat(tf);
      cursor->insertText(s);
      layout();
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Text::setStyle(int n)
      {
      TextStyle* s = &textStyles[n];
      doc->setDefaultFont(s->font());
      _align       = s->align;
      _xoff        = s->xoff;
      _yoff        = s->yoff;
      _anchor      = s->anchor;
      _offsetType  = s->offsetType;
      _sizeIsSpatiumDependent = s->sizeIsSpatiumDependent;
      cursor->movePosition(QTextCursor::Start);

      if (_align & ALIGN_HCENTER) {
            QTextBlockFormat bf = cursor->blockFormat();
            bf.setAlignment(Qt::AlignHCenter);
            cursor->setBlockFormat(bf);
            }
      QTextCharFormat tf = cursor->charFormat();
      tf.setFont(s->font());
      cursor->setCharFormat(tf);
      layout();
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
      cursor->movePosition(QTextCursor::Start);
      layout();
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml) const
      {
      xml.tag("align", _align);
      xml.tag("xoffset", _xoff);
      xml.tag("yoffset", _yoff);

      const char* p;
      switch(_anchor) {
            case ANCHOR_PAGE:     p = "page"; break;
            case ANCHOR_TICK:     p = "tick"; break;
            case ANCHOR_NOTE:     p = "note"; break;
            case ANCHOR_SYSTEM:   p = "system"; break;
            }
      xml.tag("anchor", p);
      switch(_offsetType) {
            case OFFSET_ABS:        p = "absolute"; break;
            case OFFSET_REL:        p = "relative"; break;
            case OFFSET_SPATIUM:    p = "spatium"; break;
            }
      xml.tag("offsetType", p);
      xml.tag("spatiumSizeDependent", _sizeIsSpatiumDependent);
      QString s = doc->toHtml("utf8");
      xml.tag("data", s);
      Element::writeProperties(xml);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(QDomNode node)
      {
      QDomElement e = node.toElement();
      if (e.isNull())
            return true;
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "data")
            doc->setHtml(val);
      else if (tag == "align")
            _align = val.toInt();
      else if (tag == "xoffset")
            _xoff = val.toDouble();
      else if (tag == "yoffset")
            _yoff = val.toDouble();
      else if (tag == "anchor") {
            if (val == "page")
                  _anchor = ANCHOR_PAGE;
            else if (val == "tick")
                  _anchor = ANCHOR_TICK;
            else if (val == "note")
                  _anchor = ANCHOR_NOTE;
            else if (val == "system")
                  _anchor = ANCHOR_SYSTEM;
            else
                  printf("Text::readProperties: unknown anchor: <%s>\n", val.toLocal8Bit().data());
            }
      else if (tag == "offsetType") {
            if (val == "absolute")
                  _offsetType = OFFSET_ABS;
            else if (val == "relative")
                  _offsetType = OFFSET_REL;
            else if (val == "spatium")
                  _offsetType = OFFSET_SPATIUM;
            else
                  printf("Text::readProperties: unknown offset type: <%s>\n", val.toLocal8Bit().data());
            }
      else if (tag == "spatiumSizeDependent")
            _sizeIsSpatiumDependent = val.toInt();
      else if (!Element::readProperties(node))
            return false;
      return true;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Text::startEdit(QMatrix&)
      {
      cursor->movePosition(QTextCursor::Start);
//      cursor->setPosition(cursorPos);
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
                  palette->setBlockFormat(cursor->blockFormat());
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
                  cursor->insertText(QString("\r"));
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
      if (palette) {
            palette->setCharFormat(cursor->charFormat());
            palette->setBlockFormat(cursor->blockFormat());
            }
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
//   Text::draw
//---------------------------------------------------------

void Text::draw1(Painter& p)
      {
      p.save();
      p.setRenderHint(QPainter::Antialiasing, true);

      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = editMode ? cursor->position() : -1;
      QColor color = p.pen().color();
      c.palette.setColor(QPalette::Text, color);
      doc->documentLayout()->draw(&p, c);

      p.restore();
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

void Text::setCharFormat(const QTextCharFormat& f)
      {
      cursor->setCharFormat(f);
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void Text::setBlockFormat(const QTextBlockFormat& bf)
      {
      cursor->setBlockFormat(bf);
      layout();
      score()->doLayout();
      score()->update(QRectF(0.0, 0.0, 100000.0, 100000.0));
      }

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

Lyrics::Lyrics(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_LYRIC);
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
   : Text(s)
      {
      setSubtype(TEXT_FINGERING);
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
   : Text(s)
      {
      setSubtype(TEXT_INSTRUMENT_LONG);
      }

//---------------------------------------------------------
//   InstrumentName2
//---------------------------------------------------------

InstrumentName2::InstrumentName2(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_INSTRUMENT_SHORT);
      }

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_TEMPO);
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
      cursor->setPosition(0);
//      QTextCharFormat f = cursor->charFormat();
//      f.setFont(defaultFont());
//      cursor->setCharFormat(f);
//      doc->setDefaultFont(defaultFont());
      layout();
      }

