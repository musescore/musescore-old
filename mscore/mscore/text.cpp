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
#include "utils.h"
#include "page.h"
#include "textpalette.h"
#include "sym.h"
#include "symbol.h"
#include "layout.h"

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
      _anchor                 = ANCHOR_STAFF;
      _offsetType             = OFFSET_SPATIUM;
      _sizeIsSpatiumDependent = true;
      editMode                = false;
      cursorPos               = 0;
      doc                     = new QTextDocument(0);
      cursor                  = 0;
      _frameWidth             = 0.0;
      _marginWidth            = 0.0;
      _paddingWidth           = 1.0;
      _frameColor             = QColor(Qt::black);
      _frameRound             = 5;
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
      _frameWidth             = e._frameWidth;
      _marginWidth            = e._marginWidth;
      _paddingWidth           = e._paddingWidth;
      _frameColor             = e._frameColor;
      _frameRound             = e._frameRound;

      if (editMode) {
            cursor = new QTextCursor(doc);
            cursor->setPosition(cursorPos);
            }
      else
            cursor = 0;
      }

Text::~Text()
      {
      delete doc;
      }

//---------------------------------------------------------
//   setDoc
//---------------------------------------------------------

void Text::setDoc(const QTextDocument& d)
      {
      if (doc)
            delete doc;
      doc = d.clone(0);
      cursorPos = 0;
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void Text::setSubtype(int val)
      {
      Element::setSubtype(val);
      //
      // set default values:
      //
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
            case TEXT_TUPLET:           setStyle(TEXT_STYLE_TUPLET); break;
            case TEXT_SYSTEM:           setStyle(TEXT_STYLE_SYSTEM); break;
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
            case TEXT_TUPLET:           return "Tuplet";
            case TEXT_SYSTEM:           return "System";
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
      else if (s == "Tuplet")
            st = TEXT_TUPLET;
      else if (s == "System")
            st = TEXT_SYSTEM;
      setSubtype(st);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Text::bbox() const
      {
//      doc->documentLayout()->setPaintDevice(score()->scoreLayout()->paintDevice());
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

void Text::layout(ScoreLayout* layout)
      {
      if (parent() == 0)
            return;

      doc->documentLayout()->setPaintDevice(layout->paintDevice());
      doc->setUseDesignMetrics(true);

      QPointF _off(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            _off *= _spatium;
      else
            _off *= DPI;

      double tw = bbox().width();
      double th = bbox().height();
      double x = 0.0, y = 0.0;
      if (_anchor == ANCHOR_PAGE) {
            Page* page = (Page*)parent();
            if (parent()->type() != PAGE) {
                  printf("fatal: text parent is not PAGE <%s>\n", getText().toLocal8Bit().data());
                  return;
                  }
            double w = page->loWidth() - page->lm() - page->rm();
            doc->setTextWidth(w);
            tw = w;
            double h = page->loHeight() - page->tm() - page->bm();

            if (_offsetType == OFFSET_REL)
                  _off = QPointF(_xoff * w * 0.01, _yoff * h * 0.01);
            x = page->lm();
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
      QTextCursor cursor(doc);
      cursor.movePosition(QTextCursor::Start);
      if (_align) {
            QTextBlockFormat bf = cursor.blockFormat();
            if (_align & ALIGN_HCENTER)
                  bf.setAlignment(Qt::AlignHCenter);
            else if (_align & ALIGN_LEFT)
                  bf.setAlignment(Qt::AlignLeft);
            else if (_align & ALIGN_RIGHT)
                  bf.setAlignment(Qt::AlignRight);

            cursor.setBlockFormat(bf);
            }
      QTextCharFormat tf = cursor.charFormat();
      tf.setFont(doc->defaultFont());
      cursor.setBlockCharFormat(tf);
      cursor.insertText(s);
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Text::setStyle(int n)
      {
      TextStyle* s = &textStyles[n];
      doc->setDefaultFont(s->font());
      _align         = s->align;
      _xoff          = s->xoff;
      _yoff          = s->yoff;
      _anchor        = s->anchor;
      _offsetType    = s->offsetType;
      _sizeIsSpatiumDependent = s->sizeIsSpatiumDependent;
      _frameWidth    = s->frameWidth;
      _marginWidth   = s->marginWidth;
      _paddingWidth  = s->paddingWidth;
      _frameColor    = s->frameColor;
      _frameRound    = s->frameRound;
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
      xml.etag();
      }

//---------------------------------------------------------
//   Text::read
//---------------------------------------------------------

void Text::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml) const
      {
      xml.tag("align", _align);
      xml.tag("xoffset", _xoff);
      xml.tag("yoffset", _yoff);

      const char* p = "?";
      switch(_anchor) {
            case ANCHOR_PAGE:     p = "page"; break;
            case ANCHOR_STAFF:    p = "staff"; break;
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

bool Text::readProperties(QDomElement e)
      {
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
            else if (val == "staff")
                  _anchor = ANCHOR_STAFF;
            else if (val == "tick")             // obsolete
                  _anchor = ANCHOR_STAFF;
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
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Text::startEdit(const QPointF& p)
      {
      cursor = new QTextCursor(doc);
      cursor->setPosition(cursorPos);
      editMode = true;
      if (palette) {
            palette->setCharFormat(cursor->charFormat());
            palette->setBlockFormat(cursor->blockFormat());
            }
      mousePress(p);    // set cursor
      cursorPos = cursor->position();
      return true;
      }

//---------------------------------------------------------
//   edit
//---------------------------------------------------------

bool Text::edit(QMatrix&, QKeyEvent* ev)
      {
      int key = ev->key();
      if (key == Qt::Key_F2) {
            if (palette == 0)
                  palette = new TextPalette(score()->canvas());
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
                  case Qt::Key_U:   // toggle underline
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontUnderline(!f.fontUnderline());
                        if (palette)
                              palette->setCharFormat(f);
                        cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_Up:
                        {
                        QTextCharFormat f = cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSubScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
                        cursor->setCharFormat(f);
                        }
                        break;

                  case Qt::Key_Down:
                        {
                        QTextCharFormat f = cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSuperScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
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
                  cursor->deleteChar();
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
//   moveCursorToEnd
//---------------------------------------------------------

void Text::moveCursorToEnd()
      {
      if (cursor)
            cursor->movePosition(QTextCursor::End);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
      {
      cursorPos = cursor->position();
      if (palette)
            palette->hide();
      delete cursor;
      cursor = 0;
      editMode = false;
      }

//---------------------------------------------------------
//   Text::draw
//---------------------------------------------------------

void Text::draw(QPainter& p)
      {
      double dpmm = double(p.device()->logicalDpiX()) / INCH;
      p.save();
      p.setRenderHint(QPainter::Antialiasing, true);

      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = editMode ? cursor->position() : -1;
      QColor color = p.pen().color();
      c.palette.setColor(QPalette::Text, color);
      doc->documentLayout()->draw(&p, c);

      // draw border
      if (_frameWidth > 0.0) {
            QRectF f;
            for (QTextBlock tb = doc->begin(); tb.isValid(); tb = tb.next()) {
                  QTextLayout* tl = tb.layout();
                  int n = tl->lineCount();
                  for (int i = 0; i < n; ++i)
                        f |= tl->lineAt(0).naturalTextRect().translated(tl->position());
                  }
            double w = _paddingWidth * dpmm;
            f.adjust(-w, -w, w, w);
            p.setPen(QPen(QBrush(_frameColor), _frameWidth * dpmm));
            p.setBrush(QBrush(Qt::NoBrush));
            int r2 = _frameRound * lrint((f.width() / f.height()));
            if (r2 > 99)
                  r2 = 99;
            p.drawRoundRect(f, _frameRound, r2);
            }
      p.restore();
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Text::shape() const
      {
      QPainterPath pp;

      for (QTextBlock tb = doc->begin(); tb.isValid(); tb = tb.next()) {
            QTextLayout* tl = tb.layout();
            int n = tl->lineCount();
            for (int i = 0; i < n; ++i)
                  pp.addRect(tl->lineAt(0).naturalTextRect().translated(tl->position()));
            }
      return pp;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double Text::lineSpacing() const
      {
      QTextBlock tb = doc->begin();
      QTextLayout* tl = tb.layout();
      QFontMetricsF fm(tl->font());
      return fm.lineSpacing();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Text::addSymbol(const SymCode& s)
      {
// printf("Text: addSymbol(%x)\n", s.code);
      QTextCharFormat oFormat = cursor->charFormat();
      if (s.style >= 0) {
            QTextCharFormat oFormat = cursor->charFormat();
            QTextCharFormat nFormat(oFormat);
            nFormat.setFontFamily(textStyles[s.style].font().family());
            cursor->setCharFormat(nFormat);
            cursor->insertText(s.code);
            cursor->setCharFormat(oFormat);
            }
      else
            cursor->insertText(s.code);
      score()->endCmd(false);
      }

//---------------------------------------------------------
//   setCharFormat
//---------------------------------------------------------

void Text::setCharFormat(const QTextCharFormat& f)
      {
      if (!cursor)
            return;
      cursor->setCharFormat(f);
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void Text::setBlockFormat(const QTextBlockFormat& bf)
      {
      if (!cursor)
            return;
      cursor->setBlockFormat(bf);
      score()->layout();
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool Text::mousePress(const QPointF& p)
      {
      QPointF pt = p - canvasPos();
      bool inText = bbox().contains(pt);
      if (!inText)
            return false;
      int idx = doc->documentLayout()->hitTest(pt, Qt::FuzzyHit);
      if (idx == -1)
            return true;
      cursor->setPosition(idx);
      return true;
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
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TempoText::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "tempo")
                  setTempo(e.text().toDouble());
            else if (!Text::readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

