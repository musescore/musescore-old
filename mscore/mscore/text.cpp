//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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
#include "scoreview.h"
#include "score.h"
#include "utils.h"
#include "page.h"
#include "textpalette.h"
#include "sym.h"
#include "symbol.h"
#include "textline.h"
#include "preferences.h"
#include "system.h"
#include "measure.h"
#include "textproperties.h"
#include "box.h"

TextPalette* textPalette;

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

TextBase::TextBase()
      {
      _refCount     = 1;
      _doc          = new QTextDocument(0);
      _doc->setDocumentMargin(1.0);

      _doc->setUseDesignMetrics(true);
      _doc->setUndoRedoEnabled(true);
      _doc->documentLayout()->setProperty("cursorWidth", QVariant(2));

      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      _doc->setDefaultTextOption(to);

      _hasFrame     = false;
      _frameWidth   = 0.35;         // default line width
      _paddingWidth = 0.0;
      _frameColor   = preferences.defaultColor;
      _frameRound   = 25;
      _circle       = false;
      _sp           = 0.0;
      }

TextBase::~TextBase()
      {
      delete _doc;
      }

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

TextBase::TextBase(const TextBase& t)
      {
      _refCount     = 1;
      _doc          = t._doc->clone(0);
      _frameWidth   = t._frameWidth;
      _paddingWidth = t._paddingWidth;
      _frameColor   = t._frameColor;
      _frameRound   = t._frameRound;
      _circle       = t._circle;
      _hasFrame     = t._hasFrame;
      frame         = t.frame;
      _bbox         = t._bbox;
      _sp = 0.0;
      }

//---------------------------------------------------------
//   scale
//---------------------------------------------------------

void TextBase::scale(double oldVal, double newVal)
      {
      if (newVal == _sp)
            return;
      _sp = newVal;
      double v = newVal / oldVal;
      QTextCursor cursor(_doc);
      cursor.movePosition(QTextCursor::Start);
      for (;;) {
            cursor.select(QTextCursor::BlockUnderCursor);
            QTextCharFormat cf = cursor.charFormat();
            QFont font = cf.font();
            font.setPointSizeF(font.pointSizeF() * v);
            cf.setFont(font);
            cursor.setCharFormat(cf);
            if (!cursor.movePosition(QTextCursor::NextBlock))
                  break;
            }
      }

//---------------------------------------------------------
//   setDoc
//---------------------------------------------------------

void TextBase::setDoc(const QTextDocument& d)
      {
      delete _doc;
      _doc = d.clone(0);
      }

//---------------------------------------------------------
//   swapDoc
//---------------------------------------------------------

QTextDocument* TextBase::swapDoc(QTextDocument* d)
      {
      QTextDocument* od = _doc;
      _doc = d;
      return od;
      }

//---------------------------------------------------------
//   defaultFont
//---------------------------------------------------------

QFont TextBase::defaultFont() const
      {
      QTextCursor cursor(_doc);
      cursor.movePosition(QTextCursor::Start);
      return cursor.charFormat().font();
      }

//---------------------------------------------------------
//   setDefaultFont
//---------------------------------------------------------

void TextBase::setDefaultFont(QFont f)
      {
      _doc->setDefaultFont(f);
      QTextCursor cursor(_doc);
      cursor.select(QTextCursor::Document);
      QTextCharFormat cf = cursor.charFormat();
      cf.setFont(f);
      cursor.setCharFormat(cf);
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void TextBase::setText(const QString& s, Align align)
      {
      _doc->clear();
      QTextCursor cursor(_doc);
      cursor.setVisualNavigation(true);
      cursor.movePosition(QTextCursor::Start);
      if (align & (ALIGN_HCENTER | ALIGN_RIGHT)) {
            Qt::Alignment a;
            if (align & ALIGN_HCENTER)
                  a |= Qt::AlignHCenter;
            else if (align & ALIGN_LEFT)
                  a |= Qt::AlignLeft;
            else if (align & ALIGN_RIGHT)
                  a |= Qt::AlignRight;
            QTextBlockFormat bf = cursor.blockFormat();
            bf.setAlignment(a);
            cursor.setBlockFormat(bf);
            }
      QTextCharFormat tf = cursor.charFormat();
      tf.setFont(_doc->defaultFont());
      cursor.setBlockCharFormat(tf);
      cursor.insertText(s);
      double w = _doc->idealWidth();
      _doc->setTextWidth(w);   // to make alignment work
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void TextBase::setHtml(const QString& s)
      {
      _doc->clear();
      _doc->setHtml(s);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString TextBase::getText() const
      {
      return _doc->toPlainText();
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString TextBase::getHtml() const
      {
      return _doc->toHtml("utf-8");
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextBase::writeProperties(Xml& xml, bool styled, const TextStyle* ts, bool writeText) const
      {
      // write all properties which are different from style

      xml.tag("frame", _hasFrame);
      if (!styled && _hasFrame) {
            if (ts == 0 || _frameWidth != ts->frameWidth())
                  xml.tag("frameWidth", _frameWidth);
            if (ts == 0 || _paddingWidth != ts->paddingWidth())
                  xml.tag("paddingWidth", _paddingWidth);
            if (ts == 0 || _frameColor != ts->frameColor())
                  xml.tag("frameColor", _frameColor);
            if (ts == 0 || _frameRound != ts->frameRound())
                  xml.tag("frameRound", _frameRound);
            if (ts == 0 || _circle != ts->circle())
                  xml.tag("circle", _circle);
            }
      if (writeText) {
            xml.stag("html-data");
            xml.writeHtml(_doc->toHtml("utf-8"));
            xml.etag();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextBase::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "data")                  // obsolete
            _doc->setHtml(val);
      else if (tag == "frame")
            _hasFrame = val.toInt();
      else if (tag == "html") {
            QString s = Xml::htmlToString(e);
            _doc->setHtml(s);
            }
      else if (tag == "text")
            _doc->setPlainText(val);
      else if (tag == "html-data") {
            QString s = Xml::htmlToString(e.firstChildElement());
            _doc->setHtml(s);
            }
      else if (tag == "frameWidth") {
            _frameWidth = val.toDouble();
            _hasFrame   = true;
            }
      else if (tag == "paddingWidth")
            _paddingWidth = val.toDouble();
      else if (tag == "frameColor")
            _frameColor = readColor(e);
      else if (tag == "frameRound")
            _frameRound = val.toInt();
      else if (tag == "circle")
            _circle = val.toInt();
      else
            return false;
      return true;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextBase::layout(double w)
      {
      const double mag = DPI / PDPI;

      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(w <= 0.0 ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
      _doc->setDefaultTextOption(to);

      if (w <= 0.0)
            w = _doc->idealWidth();
      else
            w /= mag;

      _doc->setTextWidth(w);   // to make alignment work

      if (_hasFrame) {
            frame = QRectF();
            for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
                  QTextLayout* tl = tb.layout();
                  int n = tl->lineCount();
                  for (int i = 0; i < n; ++i)
                        // frame |= tl->lineAt(0).naturalTextRect().translated(tl->position());
                        frame |= tl->lineAt(0).rect().translated(tl->position());
                  }
            if (_circle) {
                  if (frame.width() > frame.height()) {
                        frame.setY(frame.y() + (frame.width() - frame.height()) * -.5);
                        frame.setHeight(frame.width());
                        }
                  else {
                        frame.setX(frame.x() + (frame.height() - frame.width()) * -.5);
                        frame.setWidth(frame.height());
                        }
                  }
            frame = QRectF(frame.x() * mag, frame.y() * mag, frame.width() * mag, frame.height() * mag);
            double w = (_paddingWidth + _frameWidth * .5) * DPMM;
            frame.adjust(-w, -w, w, w);
            w = _frameWidth * DPMM;
            _bbox = frame.adjusted(-w, -w, w, w);
            }
      else {
            _bbox = _doc->documentLayout()->frameBoundingRect(_doc->rootFrame());
            _bbox = QRectF(_bbox.x() * mag, _bbox.y() * mag, _bbox.width() * mag, _bbox.height() * mag);
            }
      _doc->setModified(false);
      }

//---------------------------------------------------------
//   TextBase::draw
//---------------------------------------------------------

void TextBase::draw(QPainter& p, QTextCursor* cursor) const
      {
      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = -1;
      if (cursor) {
            if (cursor->hasSelection()) {
                  QAbstractTextDocumentLayout::Selection selection;
                  selection.cursor = *cursor;
                  selection.format.setBackground(c.palette.brush(QPalette::Active, QPalette::Highlight));
                  selection.format.setForeground(c.palette.brush(QPalette::Active, QPalette::HighlightedText));
                  c.selections.append(selection);
                  }
            c.cursorPosition = cursor->position();
            }
      QColor color = p.pen().color();
      c.palette.setColor(QPalette::Text, color);

      p.save();
      p.scale(DPI/PDPI, DPI/PDPI);
      _doc->documentLayout()->draw(&p, c);
      p.restore();

      // draw frame
      if (_hasFrame) {
            p.setPen(QPen(QBrush(_frameColor), _frameWidth * DPMM));
            p.setBrush(QBrush(Qt::NoBrush));
            if (_circle)
                  p.drawArc(frame, 0, 5760);
            else {
                  double mag = 1.0; // DPI/PDPI;

                  int r2 = _frameRound * lrint((frame.width() / frame.height()));
                  if (r2 > 99)
                        r2 = 99;
                  QRectF fr(frame.x(), frame.y(), frame.width() * mag, frame.height() * mag);
                  p.drawRoundRect(fr, _frameRound, r2);
                  }
            }
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

TextB::TextB(Score* s)
   : Element(s)
      {
      _editMode   = false;
      cursorPos  = 0;
      cursor     = 0;
      setFlag(ELEMENT_MOVABLE, true);
      _textStyle = TEXT_STYLE_INVALID;
      _styled    = false;
      _layoutToParentWidth = false;
      }

TextB::TextB(const TextB& e)
   : Element(e)
      {
      _sizeIsSpatiumDependent = e._sizeIsSpatiumDependent;
      _editMode               = e._editMode;
      cursorPos               = e.cursorPos;
      _textStyle              = e._textStyle;
      _styled                 = e._styled;
      _layoutToParentWidth    = e._layoutToParentWidth;
      cursor                  = 0;
      }

//---------------------------------------------------------
//   TextC
//---------------------------------------------------------

TextC::TextC(Score* s)
   : TextB(s)
      {
      _tb = new TextBase();
      setSubtype(TEXT_STAFF);
      }

TextC::TextC(const TextC& e)
   : TextB(e)
      {
      _tb    = e._tb;
      _tb->incRefCount();
      cursor = 0;
      baseChanged();
      }

TextC* TextC::clone() const
      {
      TextC* t = new TextC(*this);
      //? _tb->decRefCount();
      t->_tb = new TextBase(*_tb);
      return t;
      }

TextC::~TextC()
      {
      _tb->decRefCount();
      if (_tb->refCount() <= 0)
            delete _tb;
      }

//---------------------------------------------------------
//   changeBase
//---------------------------------------------------------

void TextC::changeBase(TextBase* b)
      {
      _tb->decRefCount();
      if (_tb->refCount() <= 0)
            delete _tb;
      _tb = b;
      _tb->incRefCount();
      baseChanged();
      }

//---------------------------------------------------------
//   baseChanged
//---------------------------------------------------------

void TextC::baseChanged()
      {
      if (_editMode) {
            delete cursor;
            cursor = new QTextCursor(textBase()->doc());
            cursor->setVisualNavigation(true);
            cursor->setPosition(cursorPos);
            }
      else {
            delete cursor;
            cursor = 0;
            }
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : TextB(s)
      {
      _tb = new TextBase;
      setSubtype(0);
      }

Text::Text(const Text& e)
   : TextB(e)
      {
      _tb = new TextBase(*(e._tb));
      if (_editMode) {
            cursor = new QTextCursor(textBase()->doc());
            cursor->setVisualNavigation(true);
            cursor->setPosition(cursorPos);
            cursor->setBlockFormat(e.cursor->blockFormat());
            cursor->setCharFormat(e.cursor->charFormat());
            }
      else
            cursor = 0;
      }

Text::~Text()
      {
      delete _tb;
      }

//---------------------------------------------------------
//   setDoc
//---------------------------------------------------------

void TextB::setDoc(const QTextDocument& d)
      {
      textBase()->setDoc(d);
      cursorPos = 0;
      }

//---------------------------------------------------------
//   swapDoc
//---------------------------------------------------------

QTextDocument* TextB::swapDoc(QTextDocument* d)
      {
      return textBase()->swapDoc(d);
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void TextB::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString TextB::subtypeName() const
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
            case TEXT_INSTRUMENT_EXCERPT: return "InstrumentExcerpt";
            case TEXT_TEMPO:            return "Tempo";
            case TEXT_LYRIC:            return "Lyric";
            case TEXT_TUPLET:           return "Tuplet";
            case TEXT_SYSTEM:           return "System";
            case TEXT_STAFF:            return "Staff";
            case TEXT_CHORD:            return "";     // "Chordname";
            case TEXT_REHEARSAL_MARK:   return "RehearsalMark";
            case TEXT_REPEAT:           return "Repeat";
            case TEXT_VOLTA:            return "Volta";
            case TEXT_FRAME:            return "Frame";
            case TEXT_TEXTLINE:         return "TextLine";
            case TEXT_STRING_NUMBER:    return "StringNumber";
            default:
                  printf("unknown text subtype %d\n", subtype());
                  break;
            }
      return "?";
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void TextB::setSubtype(const QString& s)
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
      else if (s == "InstrumentExcerpt")
            st = TEXT_INSTRUMENT_EXCERPT;
      else if (s == "Tempo")
            st = TEXT_TEMPO;
      else if (s == "Lyric")
            st = TEXT_LYRIC;
      else if (s == "Tuplet")
            st = TEXT_TUPLET;
      else if (s == "System")
            st = TEXT_SYSTEM;
      else if (s == "Staff")
            st = TEXT_STAFF;
      else if (s == "Chordname")
            st = TEXT_CHORD;
      else if (s == "RehearsalMark")
            st = TEXT_REHEARSAL_MARK;
      else if (s == "Repeat")
            st = TEXT_REPEAT;
      else if (s == "Volta")
            st = TEXT_VOLTA;
      else if (s == "Frame")
            st = TEXT_FRAME;
      else if (s == "TextLine")
            st = TEXT_TEXTLINE;
      else if (s == "StringNumber")
            st = TEXT_STRING_NUMBER;
      else
            printf("setSubtype: unknown type <%s>\n", qPrintable(s));
      setSubtype(st);
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void TextB::resetMode()
      {
      _editMode = 0;
      delete cursor;
      cursor = 0;
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool TextB::isEmpty() const
      {
      return doc()->isEmpty();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void TextB::layout()
      {
      double x = 0.0, y = 0.0, lw = -1;
      if (parent() && _layoutToParentWidth) {
            lw = parent()->width();
            if (parent()->type() == HBOX || parent()->type() == VBOX) {
                  Box* box = static_cast<Box*>(parent());
                  x = box->leftMargin() * DPMM;
                  lw -= (box->leftMargin() + box->rightMargin()) * DPMM;
                  y = box->topMargin() * DPMM;
                  }
            }
      textBase()->layout(lw);
      setbbox(textBase()->bbox());
      Element::layout();      // process alignment
      rxpos() += x;
      rypos() += y;

      if ((_align & ALIGN_VCENTER) && (subtype() == TEXT_TEXTLINE)) {
            // special case: vertically centered text with TextLine needs to
            // take into account the line width
            TextLineSegment* tls = (TextLineSegment*)parent();
            TextLine* tl = (TextLine*)(tls->line());
            qreal textlineLineWidth = point(tl->lineWidth());
            rypos() = ipos().y() - textlineLineWidth * .5;
            }

      if (parent() == 0)
            return;
      if (parent()->type() == SEGMENT) {
            Segment* s = static_cast<Segment*>(parent());
            rypos() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TextB::draw(QPainter& p, ScoreView*) const
      {
      textBase()->draw(p, cursor);
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void TextB::setTextStyle(TextStyleType idx)
      {
      _textStyle   = idx;
      if (_textStyle != TEXT_STYLE_INVALID)
            _styled = true;
      const TextStyle& s = score()->textStyle(idx);
      doc()->setDefaultFont(s.font(spatium()));

      _align         = s.align();
      _xoff          = s.xoff();
      _yoff          = s.yoff();
      _reloff.rx()   = s.rxoff();
      _reloff.ry()   = s.ryoff();
      _offsetType    = s.offsetType();
      setColor(s.foregroundColor());
      _sizeIsSpatiumDependent = s.sizeIsSpatiumDependent();
      setSystemFlag(s.systemFlag());

      textBase()->setFrameWidth(s.frameWidth());
      textBase()->setHasFrame(s.hasFrame());
      textBase()->setPaddingWidth(s.paddingWidth());
      textBase()->setFrameColor(s.frameColor());
      textBase()->setFrameRound(s.frameRound());
      textBase()->setCircle(s.circle());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextB::write(Xml& xml) const
      {
      write(xml, "Text");
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void TextB::write(Xml& xml, const char* name) const
      {
      if (doc()->isEmpty())
            return;
      xml.stag(name);
      writeProperties(xml, true);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void TextB::read(QDomElement e)
      {
      _align = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void TextB::writeProperties(Xml& xml, bool writeText) const
      {
      // write all properties which are different from style

      const TextStyle* st;
      if (_textStyle != TEXT_STYLE_INVALID) {
            st = &score()->textStyle(_textStyle);
            xml.tag("style", st->name());
            }
      else
            st = 0;

      Element::writeProperties(xml);

      if (!_styled) {
            if (!st || _align != st->align()) {
                  if (_align & ALIGN_HCENTER)
                        xml.tag("halign", "center");
                  else if (_align & ALIGN_RIGHT)
                        xml.tag("halign", "right");
                  else
                        xml.tag("halign", "left");

                  if (_align & ALIGN_BOTTOM)
                        xml.tag("valign", "bottom");
                  else if (_align & ALIGN_VCENTER)
                        xml.tag("valign", "center");
                  else if (_align & ALIGN_BASELINE)
                        xml.tag("valign", "baseline");
                  else
                        xml.tag("valign", "top");
                  }
            if (!st || _xoff != st->xoff())
                  xml.tag("xoffset", _xoff);
            if (!st || _yoff != st->yoff())
                  xml.tag("yoffset", _yoff);
            if (!st || _reloff.x() != st->rxoff())
                  xml.tag("rxoffset", _reloff.x());
            if (!st || _reloff.y() != st->ryoff())
                  xml.tag("ryoffset", _reloff.y());

            if (!st || _offsetType != st->offsetType()) {
                  const char* p = 0;
                  switch(_offsetType) {
                        case OFFSET_SPATIUM:    p = "spatium"; break;
                        case OFFSET_ABS:        p = "absolute"; break;
                        }
                  if (p)
                        xml.tag("offsetType", p);
                  }

            if (!st || (!_sizeIsSpatiumDependent && _sizeIsSpatiumDependent != st->sizeIsSpatiumDependent()))
                  xml.tag("spatiumSizeDependent", _sizeIsSpatiumDependent);
            if (_textStyle == TEXT_STYLE_MEASURE_NUMBER)
                  return;
            }
      textBase()->writeProperties(xml, _styled, st, writeText);
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void TextB::spatiumChanged(double oldValue, double newValue)
      {
      Element::spatiumChanged(oldValue, newValue);
      if (!_sizeIsSpatiumDependent)
            return;
      textBase()->scale(oldValue, newValue);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool TextB::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "style") {
            bool ok;
            int i = val.toInt(&ok);
            if (ok) {
                  // obsolete old text styles
                  switch (i) {
                        case 0:  i = TEXT_STYLE_INVALID;   break;
                        case 1:  i = TEXT_STYLE_INVALID;   break;
                        case 2:  i = TEXT_STYLE_TITLE;     break;
                        case 3:  i = TEXT_STYLE_SUBTITLE;  break;
                        case 4:  i = TEXT_STYLE_COMPOSER;  break;
                        case 5:  i = TEXT_STYLE_POET;      break;
                        case 6:  i = TEXT_STYLE_LYRIC1;    break;
                        case 7:  i = TEXT_STYLE_LYRIC2;    break;
                        case 8:  i = TEXT_STYLE_FINGERING; break;
                        case 9:  i = TEXT_STYLE_INSTRUMENT_LONG;    break;
                        case 10: i = TEXT_STYLE_INSTRUMENT_SHORT;   break;
                        case 11: i = TEXT_STYLE_INSTRUMENT_EXCERPT; break;

                        case 12: i = TEXT_STYLE_DYNAMICS;  break;
                        case 13: i = TEXT_STYLE_TECHNIK;   break;
                        case 14: i = TEXT_STYLE_TEMPO;     break;
                        case 15: i = TEXT_STYLE_METRONOME; break;
                        case 16: i = TEXT_STYLE_COPYRIGHT; break;
                        case 17: i = TEXT_STYLE_MEASURE_NUMBER; break;
                        case 18: i = TEXT_STYLE_FOOTER; break;    // TEXT_STYLE_PAGE_NUMBER_ODD
                        case 19: i = TEXT_STYLE_FOOTER; break;    // TEXT_STYLE_PAGE_NUMBER_EVEN
                        case 20: i = TEXT_STYLE_TRANSLATOR; break;
                        case 21: i = TEXT_STYLE_TUPLET;     break;

                        case 22: i = TEXT_STYLE_SYSTEM;         break;
                        case 23: i = TEXT_STYLE_STAFF;          break;
                        case 24: i = TEXT_STYLE_HARMONY;        break;
                        case 25: i = TEXT_STYLE_REHEARSAL_MARK; break;
                        case 26: i = TEXT_STYLE_REPEAT;         break;
                        case 27: i = TEXT_STYLE_VOLTA;          break;
                        case 28: i = TEXT_STYLE_FRAME;          break;
                        case 29: i = TEXT_STYLE_TEXTLINE;       break;
                        case 30: i = TEXT_STYLE_GLISSANDO;      break;
                        case 31: i = TEXT_STYLE_STRING_NUMBER;  break;

                        case 32: i = TEXT_STYLE_OTTAVA;  break;
                        case 33: i = TEXT_STYLE_BENCH;   break;
                        case 34: i = TEXT_STYLE_HEADER;  break;
                        case 35: i = TEXT_STYLE_FOOTER;  break;
                        default: i = TEXT_STYLE_INVALID; break;
                        }
                  setTextStyle(TextStyleType(i));
                  }
            else
                  setTextStyle(score()->style().textStyleType(val));
            }
      else if (tag == "align")            // obsolete
            _align = Align(val.toInt());
      else if (tag == "halign") {
            _align &= ~(ALIGN_HCENTER | ALIGN_RIGHT);
            if (val == "center")
                _align |= ALIGN_HCENTER;
            else if (val == "right")
                _align |= ALIGN_RIGHT;
            else if (val == "left")
                  ;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "valign") {
            _align &= ~(ALIGN_VCENTER | ALIGN_BOTTOM | ALIGN_BASELINE);
            if (val == "center")
                  _align |= ALIGN_VCENTER;
            else if (val == "bottom")
                  _align |= ALIGN_BOTTOM;
            else if (val == "baseline")
                  _align |= ALIGN_BASELINE;
            else if (val == "top")
                  ;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "xoffset")
            _xoff = val.toDouble();
      else if (tag == "yoffset")
            _yoff = val.toDouble();
      else if (tag == "rxoffset")
            _reloff.rx() = val.toDouble();
      else if (tag == "ryoffset")
            _reloff.ry() = val.toDouble();
      else if (tag == "offsetType") {
            if (val == "absolute")
                  _offsetType = OFFSET_ABS;
            else if (val == "spatium")
                  _offsetType = OFFSET_SPATIUM;
            else
                  printf("Text::readProperties: unknown offset type: <%s>\n", qPrintable(val));
            }
      else if (tag == "spatiumSizeDependent")
            _sizeIsSpatiumDependent = val.toInt();
      else if (textBase()->readProperties(e))
            ;
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void TextB::startEdit(ScoreView* view, const QPointF& p)
      {
      mscore->textTools()->show();
      cursor = new QTextCursor(doc());
      cursor->setVisualNavigation(true);
      cursor->setPosition(cursorPos);
      _editMode = true;
      setCursor(p);
      cursorPos = cursor->position();

      if (cursorPos == 0 && _align) {
            QTextBlockFormat bf = cursor->blockFormat();
            Qt::Alignment alignment = 0;
            if (_align & ALIGN_HCENTER)
                  alignment |= Qt::AlignHCenter;
            else if (_align & ALIGN_LEFT)
                  alignment |= Qt::AlignLeft;
            else if (_align & ALIGN_RIGHT)
                  alignment |= Qt::AlignRight;
            bf.setAlignment(alignment);
            setBlockFormat(bf);
            }
      qreal w = 8.0 / view->matrix().m11();
      score()->addRefresh(abbox().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   isEditable
//---------------------------------------------------------

bool TextB::isEditable() const
      {
      return !((type() == TEXT) && ((subtype() == TEXT_MEASURE_NUMBER) || (subtype() == TEXT_PAGE_NUMBER_ODD) || (subtype() == TEXT_PAGE_NUMBER_EVEN)));
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool TextB::edit(ScoreView* view, int /*grip*/, int key, Qt::KeyboardModifiers modifiers, const QString& s)
      {
      if (debugMode)
            printf("TextB::edit(%p) key 0x%x mod 0x%x\n", this, key, int(modifiers));
      if (!_editMode || !cursor) {
            printf("TextB::edit(%p): not in edit mode: %d %p\n", this, _editMode, cursor);
            return false;
            }
      bool lo = (subtype() == TEXT_INSTRUMENT_SHORT) || (subtype() == TEXT_INSTRUMENT_LONG);
      score()->setLayoutAll(lo);
      qreal w = 8.0 / view->matrix().m11();
      score()->addRefresh(abbox().adjusted(-w, -w, w, w));

      if (modifiers == Qt::ControlModifier) {
            switch (key) {
                  case Qt::Key_A:   // select all
                        cursor->select(QTextCursor::Document);
                        break;
                  case Qt::Key_B:   // toggle bold face
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontWeight(f.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                        mscore->textTools()->setCharFormat(f);
                        cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_I:   // toggle italic
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontItalic(!f.fontItalic());
                        mscore->textTools()->setCharFormat(f);
                        cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_U:   // toggle underline
                        {
                        QTextCharFormat f = cursor->charFormat();
                        f.setFontUnderline(!f.fontUnderline());
                        mscore->textTools()->setCharFormat(f);
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
#ifndef Q_WS_MAC
            if (key != Qt::Key_Space && key != Qt::Key_Minus)
                  return true;
#endif
            }
#ifdef Q_WS_MAC
      else if (modifiers == Qt::AltModifier) {
	      if (key != Qt::Key_Space && key != Qt::Key_Minus)
                  return true;
            }
#endif
      QTextCursor::MoveMode mm = (modifiers & Qt::ShiftModifier)
         ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;
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
                  cursor->movePosition(QTextCursor::Left, mm);
                  // if (!cursor->movePosition(QTextCursor::Left, mm))
                  //      return false;
                  break;

            case Qt::Key_Right:
                  cursor->movePosition(QTextCursor::Right, mm);
                  // if (!cursor->movePosition(QTextCursor::Right, mm))
                  //      return false;
                  break;

            case Qt::Key_Up:
                  cursor->movePosition(QTextCursor::Up, mm);
                  break;

            case Qt::Key_Down:
                  cursor->movePosition(QTextCursor::Down, mm);
                  break;

            case Qt::Key_Home:
                  cursor->movePosition(QTextCursor::Start, mm);
                  break;

            case Qt::Key_End:
                  cursor->movePosition(QTextCursor::End, mm);
                  break;

            case Qt::Key_Space:
                  cursor->insertText(" ");
                  break;

            case Qt::Key_Minus:
                  cursor->insertText("-");
                  break;
#if 0
            case Qt::Key_Shift:
            case Qt::Key_Control:
            case Qt::Key_Meta:
            case Qt::Key_Alt:
            case Qt::Key_AltGr:
            case Qt::Key_CapsLock:
            case Qt::Key_NumLock:
            case Qt::Key_ScrollLock:
                  break;
#endif
            default:
                  if (!s.isEmpty())
                        cursor->insertText(s);
                  break;
            }
      if (key == Qt::Key_Return || key == Qt::Key_Space || key == Qt::Key_Tab) {
            replaceSpecialChars();
            }
      mscore->textTools()->setCharFormat(cursor->charFormat());
      mscore->textTools()->setBlockFormat(cursor->blockFormat());
      layout();
      score()->addRefresh(abbox().adjusted(-w, -w, w, w));
      return true;
      }

//---------------------------------------------------------
//   replaceSpecialChars
//---------------------------------------------------------

bool TextB::replaceSpecialChars()
      {
      QTextCursor startCur = *cursor;
      foreach (const char* s, charReplaceMap.keys()) {
            SymCode sym = *charReplaceMap.value(s);
            switch (sym.type) {
                  case SYMBOL_COPYRIGHT:
                        if (!preferences.replaceCopyrightSymbol || subtype() != TEXT_COPYRIGHT)
                              continue;
                        break;
                  case SYMBOL_FRACTION:
                        if (!preferences.replaceFractions)
                              continue;
                        break;
                  default:
                        ;
                  }
            QTextCursor cur = doc()->find(s, cursor->position() - 1 - strlen(s),
                  QTextDocument::FindWholeWords);
            if (cur.isNull())
                  continue;
            // do not go beyond the cursor
            if (cur.selectionEnd() > cursor->selectionEnd())
                  continue;
            addSymbol(sym, &cur);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   moveCursorToEnd
//---------------------------------------------------------

void TextB::moveCursorToEnd()
      {
      if (cursor)
            cursor->movePosition(QTextCursor::End);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void TextB::moveCursor(int col)
      {
      if (cursor)
            cursor->setPosition(col);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void TextB::endEdit()
      {
      if (!_editMode || !cursor) {
            printf("endEdit<%p>: not in edit mode or no cursor: %d %p\n", this, _editMode, cursor);
            return;
            }
      cursorPos = cursor->position();
      if (textPalette) {
            textPalette->hide();
            mscore->textTools()->kbAction()->setChecked(false);
            }
      mscore->textTools()->hide();
      delete cursor;
      cursor = 0;
      _editMode = false;

//      if (subtype() == TEXT_COPYRIGHT)
//            score()->undoChangeCopyright(doc()->toHtml("UTF-8"));
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath TextB::shape() const
      {
      QPainterPath pp;

      const double mag = DPI / PDPI;
      for (QTextBlock tb = doc()->begin(); tb.isValid(); tb = tb.next()) {
            QTextLayout* tl = tb.layout();
            int n = tl->lineCount();
            for (int i = 0; i < n; ++i) {
                  QRectF r(tl->lineAt(0).naturalTextRect().translated(tl->position()));
                  r = QRectF(r.x() * mag, r.y() * mag, r.width() * mag, r.height() * mag);
                  pp.addRect(r);
                  }
            }
      return pp;
      }

//---------------------------------------------------------
//   baseLine
//    returns ascent of first text line in first block
//---------------------------------------------------------

qreal TextB::baseLine() const
      {
      for (QTextBlock tb = doc()->begin(); tb.isValid(); tb = tb.next()) {
            const QTextLayout* tl = tb.layout();
            if (tl->lineCount()) {
                  const double mag = DPI / PDPI;
                  return (tl->lineAt(0).ascent() + tl->position().y()) * mag;
                  }
            }
      return 0.0;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double TextB::lineSpacing() const
      {
      return QFontMetricsF(doc()->defaultFont(), pdev).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//    HACK
//---------------------------------------------------------

double TextB::lineHeight() const
      {
      QFontMetricsF fm(doc()->defaultFont(), pdev);
      return fm.height();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void TextB::addSymbol(const SymCode& s, QTextCursor* cur)
      {
      if (cur == 0)
            cur = cursor;
      if (s.fontId >= 0) {
            QTextCharFormat nFormat(cur->charFormat());
            nFormat.setFontFamily(fontId2font(s.fontId).family());
            QString ss;
            if (s.code >= 0x10000) {
                  ss = QChar(QChar::highSurrogate(s.code));
                  ss += QChar(QChar::lowSurrogate(s.code));
                  }
            else
                  ss = QChar(s.code);
            cur->insertText(ss, nFormat);
            }
      else
            cur->insertText(QChar(s.code));
      score()->setLayoutAll(true);
      score()->end();
      }

//---------------------------------------------------------
//   addChar
//---------------------------------------------------------

void TextB::addChar(int code, QTextCursor* cur)
      {
      if (cur == 0)
            cur = cursor;

      QString ss;
      if (code & 0xffff0000) {
            ss = QChar(QChar::highSurrogate(code));
            ss += QChar(QChar::lowSurrogate(code));
            }
      else
            ss = QChar(code);
      cur->insertText(ss);
      score()->setLayoutAll(true);
      score()->end();
      }

//---------------------------------------------------------
//   setCharFormat
//---------------------------------------------------------

void TextB::setCharFormat(const QTextCharFormat& f)
      {
      if (!cursor)
            return;
      cursor->setCharFormat(f);
      }

//---------------------------------------------------------
//   setBlockFormat
//---------------------------------------------------------

void TextB::setBlockFormat(const QTextBlockFormat& bf)
      {
      if (!cursor)
            return;
      cursor->setBlockFormat(bf);
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

bool TextB::setCursor(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;

      const double mag = DPI / PDPI;
      pt /= mag;

      int idx = doc()->documentLayout()->hitTest(pt, Qt::FuzzyHit);
      if (idx == -1)
            return true;
      if (cursor) {
            cursor->setPosition(idx, mode);
            if (cursor->hasSelection())
                  QApplication::clipboard()->setText(cursor->selectedText(), QClipboard::Selection);
            }
      return true;
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool TextB::mousePress(const QPointF& p, QMouseEvent* ev)
      {
      bool shift = ev->modifiers() & Qt::ShiftModifier;
      if (!setCursor(p, shift ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor))
            return false;
      if (ev->button() == Qt::MidButton)
            paste();
      return true;
      }

//---------------------------------------------------------
//   paste
//---------------------------------------------------------

void TextB::paste()
      {
#if defined(Q_WS_MAC) || defined(__MINGW32__)
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
#else
      QString txt = QApplication::clipboard()->text(QClipboard::Selection);
#endif
      if (debugMode)
            printf("TextB::paste() <%s>\n", qPrintable(txt));
      cursor->insertText(txt);
      layout();
      bool lo = (subtype() == TEXT_INSTRUMENT_SHORT) || (subtype() == TEXT_INSTRUMENT_LONG);
      score()->setLayoutAll(lo);
      score()->setUpdateAll();
      score()->end();
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool TextB::genPropertyMenu(QMenu* popup) const
      {
      QAction* a;
      if (visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("Text Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void TextB::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            Text* nText = (Text*)clone();
            TextProperties tp(nText, 0);
            int rv = tp.exec();
            if (rv) {
                  QList<Element*> sl;
                  if (tp.applyToAll())
                        score()->scanElements(&sl, collectElements);
                  else
                        sl = score()->selection().elements();
                  QList<Element*> selectedElements;
                  foreach(Element* e, sl) {
                        if ((e->type() != type()) || (e->subtype() != subtype()))
                              continue;
                        Text* t  = static_cast<Text*>(e);
                        Text* tt = t->clone();
                        if (nText->textBase()->hasFrame() != textBase()->hasFrame())
                              tt->textBase()->setHasFrame(nText->textBase()->hasFrame());
                        if (nText->frameWidth() != frameWidth())
                              tt->setFrameWidth(nText->frameWidth());
                        if (nText->paddingWidth() != paddingWidth())
                              tt->setPaddingWidth(nText->paddingWidth());
                        if (nText->frameColor() != frameColor())
                              tt->setFrameColor(nText->frameColor());
                        if (nText->frameRound() != frameRound())
                              tt->setFrameRound(nText->frameRound());
                        if (nText->circle() != circle())
                              tt->setCircle(nText->circle());
                        if (nText->color() != color())
                              tt->setColor(nText->color());
                        if (nText->defaultFont() != defaultFont()) {      // was font changed?
                              QFont a = nText->defaultFont();
                              QFont b = defaultFont();
                              QFont f = t->defaultFont();
                              if (a.family() != b.family())
                                    f.setFamily(a.family());
                              if (a.pointSizeF() != b.pointSizeF())
                                    f.setPointSizeF(a.pointSizeF());
                              if (a.bold() != b.bold())
                                    f.setBold(a.bold());
                              if (a.italic() != b.italic())
                                    f.setItalic(a.italic());
                              if (a.underline() != b.underline())
                                    f.setUnderline(a.underline());
                              tt->setDefaultFont(f);
                              }
                        if (nText->align() != align())
                              tt->setAlign(nText->align());
                        if (nText->xoff() != xoff())
                              tt->setXoff(nText->xoff());
                        if (nText->yoff() != yoff())
                              tt->setYoff(nText->yoff());
                        if (nText->reloff() != _reloff)
                              tt->setReloff(nText->reloff());
                        if (nText->offsetType() != offsetType())
                              tt->setOffsetType(nText->offsetType());

                        tt->doc()->setModified(true);
                        if (t->selected())
                              selectedElements.append(tt);
                        score()->undoChangeElement(t, tt);
                        }
                  score()->select(0, SELECT_SINGLE, 0);
                  foreach(Element* e, selectedElements)
                        score()->select(e, SELECT_ADD, 0);
                  }
            delete nText;
            }
      else
            Element::propertyAction(viewer, s);
      }

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF TextB::dragAnchor() const
      {
      QPointF p1;

      if (parent()->type() == MEASURE) {
            Measure* m     = static_cast<Measure*>(parent());
            System* system = m->system();
            double yp      = system->staff(staffIdx())->y() + system->y();
            // TODO1 double xp      = m->tick2pos(tick()) + m->canvasPos().x();
            double xp = 0.0;
            p1 = QPointF(xp, yp);
            }
      else {
            p1 = QPointF(parent()->abbox().topLeft());
            }
      double tw = width();
      double th = height();
      double x  = 0.0;
      double y  = 0.0;
      if (_align & ALIGN_BOTTOM)
            y = th;
      else if (_align & ALIGN_VCENTER)
            y = (th * .5);
      else if (_align & ALIGN_BASELINE)
            y = baseLine();
      if (_align & ALIGN_RIGHT)
            x = tw;
      else if (_align & ALIGN_HCENTER)
            x = (tw * .5);
      return QLineF(p1, abbox().topLeft() + QPointF(x, y));
      }

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

TextProperties::TextProperties(TextB* t, QWidget* parent)
   : QDialog(parent)
      {
      setWindowTitle(tr("MuseScore: Text Properties"));
      QGridLayout* layout = new QGridLayout;

      tp                  = new TextProp(false);

      layout->addWidget(tp, 0, 1);
      QLabel* l = new QLabel;
      l->setPixmap(QPixmap(":/data/bg1.jpg"));
      l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Ignored);

      layout->addWidget(l, 0, 0, 2, 1);
      QHBoxLayout* hb = new QHBoxLayout;
      cb = new QCheckBox;
      cb->setText(tr("apply to all elements of same type"));
      hb->addWidget(cb);
      QDialogButtonBox* bb = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
      hb->addWidget(bb);
      layout->addLayout(hb, 1, 1);
      setLayout(layout);

      tb = t;

      tp->set(tb);

      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TextProperties::accept()
      {
      tp->get(tb);
      QDialog::accept();
      }

//---------------------------------------------------------
//   dragTo
//---------------------------------------------------------

void TextB::dragTo(const QPointF& p)
      {
      setCursor(p, QTextCursor::KeepAnchor);
      score()->setUpdateAll();
      score()->end();
      }

