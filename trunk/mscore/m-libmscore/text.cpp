//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: text.cpp 3693 2010-11-09 17:23:35Z wschweer $
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

#include <QtGui/QTextLayout>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>


#include "globals.h"
#include "text.h"
#include "m-al/xml.h"
#include "style.h"
#include "score.h"
#include "utils.h"
#include "page.h"
#include "sym.h"
#include "symbol.h"
#include "textline.h"
#include "preferences.h"
#include "system.h"
#include "measure.h"
#include "box.h"
#include "segment.h"
#include "font.h"
#include "scoreproxy.h"

TextPalette* textPalette;

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : Element(s)
      {
#if 0
      _doc = new QTextDocument(0);
      _doc->setDocumentMargin(1.0);
      _doc->setUseDesignMetrics(true);
      _doc->setUndoRedoEnabled(true);

      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      _doc->setDefaultTextOption(to);
#endif

      setFlag(ELEMENT_MOVABLE, true);
      _textStyle = TEXT_STYLE_INVALID;
      _styled    = false;
      _layoutToParentWidth = false;
      setSubtype(0);
      _lineSpacing = 0.0;
      _lineHeight  = 0.0;
      _baseLine    = 0.0;
      }

Text::Text(const Text& e)
   : Element(e)
      {
//      _doc                  = e._doc->clone();
      frame                 = e.frame;
      _styled               = e._styled;
      _localStyle           = e._localStyle;
      _textStyle            = e._textStyle;
      _layoutToParentWidth  = e._layoutToParentWidth;
      _lineSpacing          = e._lineSpacing;
      _lineHeight           = e._lineHeight;
      _baseLine             = e._baseLine;
      }

Text::~Text()
      {
//      delete _doc;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

#if 0
void Text::setText(const QString& s)
      {
#if 0
      Align align = style().align();
//      _doc->clear();
      Font font(style().font(spatium()));
//      _doc->setDefaultFont(font);

      QTextCursor cursor(_doc);
      cursor.setVisualNavigation(true);
      cursor.movePosition(QTextCursor::Start);
      Qt::Alignment a;
      if (align & ALIGN_HCENTER)
            a = Qt::AlignHCenter;
      else if (align & ALIGN_RIGHT)
            a = Qt::AlignRight;
      else
            a = Qt::AlignLeft;
      QTextBlockFormat bf = cursor.blockFormat();
      bf.setAlignment(a);
      cursor.setBlockFormat(bf);

      QTextCharFormat tf = cursor.charFormat();
//      tf.setFont(font);
      cursor.setBlockCharFormat(tf);
      cursor.insertText(s);
#endif
      }
#endif
//void Text::setText(const QTextDocumentFragment& f)
//      {
 //     setHtml(f.toHtml());
  //    }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void Text::setHtml(const QString& /*s*/)
      {
//      _doc->clear();
//      _doc->setHtml(s);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString Text::getText() const
      {
      return _text;
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString Text::getHtml() const
      {
      return QString(); // _doc->toHtml("utf-8");
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Text::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
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
            case TEXT_INSTRUMENT_CHANGE: return "InstrumentChange";
            case TEXT_LYRICS_VERSE_NUMBER: return "LyricsVerseNumber";
            default:
                  printf("unknown text subtype %d\n", subtype());
                  break;
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
      else if (s == "InstrumentChange")
            st = TEXT_INSTRUMENT_CHANGE;
      else if (s == "LyricsVerseNumber")
            st = TEXT_LYRICS_VERSE_NUMBER;
      else
            printf("setSubtype: unknown type <%s>\n", qPrintable(s));
      setSubtype(st);
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void Text::resetMode()
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
#if 0
      QSizeF pageSize(-1.0, 1000000);
      setPos(0.0, 0.0);
      if (parent() && _layoutToParentWidth) {
            pageSize.setWidth(parent()->width());
            if (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX) {
                  Box* box = static_cast<Box*>(parent());
                  rxpos() += box->leftMargin() * DPMM;
                  rypos() += box->topMargin() * DPMM;
                  // pageSize.setHeight(box->height() - (box->topMargin() + box->bottomMargin()) * DPMM);
                  pageSize.setWidth(box->width()   - (box->leftMargin() + box->rightMargin()) * DPMM);
                  }
            }

      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(pageSize.width() <= 0.0 ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
      _doc->setDefaultTextOption(to);

      if (pageSize.width() <= 0.0)
            _doc->setTextWidth(_doc->idealWidth());
      else
            _doc->setPageSize(pageSize);

      if (hasFrame()) {
            frame = QRectF();
            for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
                  QTextLayout* tl = tb.layout();
                  int n = tl->lineCount();
                  for (int i = 0; i < n; ++i)
                        // frame |= tl->lineAt(0).naturalTextRect().translated(tl->position());
                        frame |= tl->lineAt(0).rect().translated(tl->position());
                  }
            if (circle()) {
                  if (frame.width() > frame.height()) {
                        frame.setY(frame.y() + (frame.width() - frame.height()) * -.5);
                        frame.setHeight(frame.width());
                        }
                  else {
                        frame.setX(frame.x() + (frame.height() - frame.width()) * -.5);
                        frame.setWidth(frame.height());
                        }
                  }
            qreal w = (paddingWidth() + frameWidth() * .5) * DPMM;
            frame.adjust(-w, -w, w, w);
            w = frameWidth() * DPMM;
            _bbox = frame.adjusted(-w, -w, w, w);
            }
      else {
            _bbox = QRectF(QPointF(0.0, 0.0), _doc->size()); //_doc->documentLayout()->frameBoundingRect(_doc->rootFrame());
            }
      _doc->setModified(false);
      style().layout(this);      // process alignment

      if ((style().align() & ALIGN_VCENTER) && (subtype() == TEXT_TEXTLINE)) {
            // special case: vertically centered text with TextLine needs to
            // take into account the line width
            TextLineSegment* tls = (TextLineSegment*)parent();
            TextLine* tl = (TextLine*)(tls->line());
            qreal textlineLineWidth = point(tl->lineWidth());
            rypos() -= textlineLineWidth * .5;
            }

      if (parent() == 0)
            return;
      if (parent()->type() == SEGMENT) {
            Segment* s = static_cast<Segment*>(parent());
            rypos() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
            }
#endif
      Font f = style().font(spatium());
      qreal asc, desc, leading;
      qreal w = textMetrics(f.family(), _text, f.size(), &asc, &desc, &leading);

// printf("text(%s) asc %f desc %f leading %f  w %f\n", qPrintable(_text), asc, desc, leading, w);
      _lineHeight  = asc + desc;
      _lineSpacing = _lineHeight + leading;
      _baseLine    = asc;

      setbbox(QRectF(0.0, -asc, w, _lineHeight));
#if 0
      if (parent() && _layoutToParentWidth) {
            qreal wi = parent()->width();
            qreal ph = parent()->height();
            qreal x;
            qreal y = pos.y();
            if (align() & ALIGN_HCENTER)
                  x = (wi - w) * .5;
            else if (align() & ALIGN_RIGHT)
                  x = wi - w;
            else
                  x = 0.0;
            if (align() & ALIGN_VCENTER)
                  y = (ph - asc) * .5;
            else if (align() & ALIGN_BOTTOM)
                  y = ph - asc;
            else
                  y = asc;
            setPos(x, y + asc);
            }
#endif
      style().layout(this);      // process alignment
      rypos() += asc;

      if (parent() && _layoutToParentWidth) {
            qreal wi = parent()->width();
            if (align() & ALIGN_HCENTER)
                  rxpos() = (wi - w) * .5;
            else if (align() & ALIGN_RIGHT)
                  rxpos() = wi - w;
            }

      if ((style().align() & ALIGN_VCENTER) && (subtype() == TEXT_TEXTLINE)) {
            // special case: vertically centered text with TextLine needs to
            // take into account the line width
            TextLineSegment* tls = (TextLineSegment*)parent();
            TextLine* tl = (TextLine*)(tls->line());
            qreal textlineLineWidth = point(tl->lineWidth());
            rypos() -= textlineLineWidth * .5;
            }

      if (parent() == 0)
            return;
      if (parent()->type() == SEGMENT) {
            Segment* s = static_cast<Segment*>(parent());
            rypos() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
            }
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX)) {
            QRectF r = parent()->abbox();
            Box* box = static_cast<Box*>(parent());
            qreal x = r.x() + box->leftMargin() * DPMM;
            qreal y = r.y() + box->topMargin() * DPMM;
            qreal h = r.height() - (box->topMargin() + box->bottomMargin()) * DPMM;
            qreal w = r.width()  - (box->leftMargin() + box->rightMargin()) * DPMM;

            // QSizeF ps = _doc->pageSize();
            // return QRectF(x, y, ps.width(), ps.height());

            return QRectF(x, y, w, h);
            }
      else
            return abbox();
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Text::draw(Painter* p) const
      {
      Font f = style().font(spatium());
      p->drawText(f, QPointF(), _text);
#if 0
      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = -1;
      if (cursor && !(score() && score()->printing())) {
            if (cursor->hasSelection()) {
                  QAbstractTextDocumentLayout::Selection selection;
                  selection.cursor = *cursor;
                  selection.format.setBackground(c.palette.brush(QPalette::Active, QPalette::Highlight));
                  selection.format.setForeground(c.palette.brush(QPalette::Active, QPalette::HighlightedText));
                  c.selections.append(selection);
                  }
            c.cursorPosition = cursor->position();
            }
      Color color = p->pen().color();
      c.palette.setColor(QPalette::Text, color);

      _doc->documentLayout()->draw(p->painter(), c);

      // draw frame
      if (hasFrame()) {
            Color color(frameColor());
            if (!visible())
                  color = Qt::gray;
            else if (selected())
                  color = preferences.selectColor[track() == -1 ? 0 : voice()];
            p->setPen(Pen(QBrush(color), frameWidth() * DPMM));
            p->setBrush(QBrush(Qt::NoBrush));
            if (circle())
                  p->drawArc(frame, 0, 5760);
            else {
                  int r2 = frameRound() * lrint((frame.width() / frame.height()));
                  if (r2 > 99)
                        r2 = 99;
                  p->drawRoundRect(frame, frameRound(), r2);
                  }
            }
#endif
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void Text::setTextStyle(TextStyleType idx)
      {
      _textStyle = idx;
      bool isStyled;
      if (_textStyle != TEXT_STYLE_INVALID) {
            isStyled = true;
            _localStyle = score()->textStyle(_textStyle);
            setText(getText());      // init style
            }
      else
            isStyled = false;
      setStyled(isStyled);
      }

//---------------------------------------------------------
//   setStyled
//---------------------------------------------------------

void Text::setStyled(bool v)
      {
      _styled = v;
      setSystemFlag(style().systemFlag());
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (!readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(XmlReader* r)
      {
      MString8 tag = r->tag();
      QString val;
      int i;
      qreal d;
      Color color;

      if (r->readString("style", &val)) {
            setTextStyle(score()->style().textStyleType(val));
            if (textStyle() != TEXT_STYLE_INVALID)
                  _styled = true;
            }
      else if (r->readInt("spatiumSizeDependent", &i)) {
            setSizeIsSpatiumDependent(i);
            _styled = false;
            }
      else if (r->readInt("frame", &i)) {
            setHasFrame(i);
            _styled = false;
            }
      else if (r->readString("text", &val))
            setText(val);
      else if (tag == "frameWidth") {
            setFrameWidth(val.toDouble());
            setHasFrame(true);
            _styled = false;
            }
      else if (r->readReal("paddingWidth", &d)) {
            setPaddingWidth(d);
            _styled = false;
            }
      else if (r->readColor("frameColor", &color)) {
//            setFrameColor(readColor(e));
            _styled = false;
            }
      else if (r->readInt("frameRound", &i)) {
            setFrameRound(i);
            _styled = false;
            }
      else if (r->readInt("circle", &i)) {
            setCircle(i);
            _styled = false;
            }
      else if (_localStyle.readProperties(r))
            _styled = false;
      else if (!Element::readProperties(r))
            return false;
      return true;
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

#if 0
QPainterPath Text::shape() const
      {
      QPainterPath pp;
#if 0
      for (QTextBlock tb = doc()->begin(); tb.isValid(); tb = tb.next()) {
            QTextLayout* tl = tb.layout();
            int n = tl->lineCount();
            for (int i = 0; i < n; ++i) {
                  QTextLine l = tl->lineAt(i);
                  QRectF r(l.naturalTextRect().translated(tl->position()));
                  r.adjust(-l.position().x(), 0.0, 0.0, 0.0);
                  pp.addRect(r);
                  }
            }
#endif
      return pp;
      }
#endif

//---------------------------------------------------------
//   style
//---------------------------------------------------------

const TextStyle& Text::style() const
      {
      return _styled ? score()->textStyle(_textStyle) : _localStyle;
      }

//---------------------------------------------------------
//   sizeIsSpatiumDependent
//---------------------------------------------------------

bool Text::sizeIsSpatiumDependent() const
      {
      return style().sizeIsSpatiumDependent();
      }

//---------------------------------------------------------
//   setSizeIsSpatiumDependent
//---------------------------------------------------------

void Text::setSizeIsSpatiumDependent(int v)
      {
      _localStyle.setSizeIsSpatiumDependent(v);
      }

//---------------------------------------------------------
//   frameWidth
//---------------------------------------------------------

qreal Text::frameWidth() const
      {
      return style().frameWidth();
      }

//---------------------------------------------------------
//   hasFrame
//---------------------------------------------------------

bool Text::hasFrame() const
      {
      return style().hasFrame();
      }

//---------------------------------------------------------
//   paddingWidth
//---------------------------------------------------------

qreal Text::paddingWidth() const
      {
      return style().paddingWidth();
      }

//---------------------------------------------------------
//   frameColor
//---------------------------------------------------------

Color Text::frameColor() const
      {
      return style().frameColor();
      }

//---------------------------------------------------------
//   frameRound
//---------------------------------------------------------

int Text::frameRound() const
      {
      return style().frameRound();
      }

//---------------------------------------------------------
//   circle
//---------------------------------------------------------

bool Text::circle() const
      {
      return style().circle();
      }

//---------------------------------------------------------
//   xoff
//---------------------------------------------------------

qreal Text::xoff() const
      {
      return style().xoff();
      }

//---------------------------------------------------------
//   align
//---------------------------------------------------------

Align Text::align() const
      {
      return style().align();
      }

//---------------------------------------------------------
//   offsetType
//---------------------------------------------------------

OffsetType Text::offsetType() const
      {
      return style().offsetType();
      }

//---------------------------------------------------------
//   reloff
//---------------------------------------------------------

QPointF Text::reloff() const
      {
      return style().reloff();
      }

//---------------------------------------------------------
//   setAlign
//---------------------------------------------------------

void Text::setAlign(Align val)
      {
      _localStyle.setAlign(val);
      }

//---------------------------------------------------------
//   setXoff
//---------------------------------------------------------

void Text::setXoff(qreal val)
      {
      _localStyle.setXoff(val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Text::setYoff(qreal val)
      {
      _localStyle.setYoff(val);
      }

//---------------------------------------------------------
//   setOffsetType
//---------------------------------------------------------

void Text::setOffsetType(OffsetType val)
      {
      _localStyle.setOffsetType(val);
      }

//---------------------------------------------------------
//   setRxoff
//---------------------------------------------------------

void Text::setRxoff(qreal v)
      {
      _localStyle.setRxoff(v);
      }

//---------------------------------------------------------
//   setRyoff
//---------------------------------------------------------

void Text::setRyoff(qreal v)
      {
      _localStyle.setRyoff(v);
      }

//---------------------------------------------------------
//   setReloff
//---------------------------------------------------------

void Text::setReloff(const QPointF& p)
      {
      _localStyle.setReloff(p);
      }

//---------------------------------------------------------
//   rxoff
//---------------------------------------------------------

qreal Text::rxoff() const
      {
      return style().rxoff();
      }

//---------------------------------------------------------
//   ryoff
//---------------------------------------------------------

qreal Text::ryoff() const
      {
      return style().ryoff();
      }

//---------------------------------------------------------
//   yoff
//---------------------------------------------------------

qreal Text::yoff() const
      {
      return style().yoff();
      }

//---------------------------------------------------------
//   setFrameWidth
//---------------------------------------------------------

void Text::setFrameWidth(qreal val)
      {
      _localStyle.setFrameWidth(val);
      }

//---------------------------------------------------------
//   setPaddingWidth
//---------------------------------------------------------

void Text::setPaddingWidth(qreal val)
      {
      _localStyle.setPaddingWidth(val);
      }

//---------------------------------------------------------
//   setFrameColor
//---------------------------------------------------------

void Text::setFrameColor(const Color& val)
      {
      _localStyle.setFrameColor(val);
      }

//---------------------------------------------------------
//   setFrameRound
//---------------------------------------------------------

void Text::setFrameRound(int val)
      {
      _localStyle.setFrameRound(val);
      }

//---------------------------------------------------------
//   setCircle
//---------------------------------------------------------

void Text::setCircle(bool val)
      {
      _localStyle.setCircle(val);
      }

//---------------------------------------------------------
//   setItalic
//---------------------------------------------------------

void Text::setItalic(bool val)
      {
      _localStyle.setItalic(val);
      }

//---------------------------------------------------------
//   setBold
//---------------------------------------------------------

void Text::setBold(bool val)
      {
      _localStyle.setBold(val);
      }

//---------------------------------------------------------
//   setSize
//---------------------------------------------------------

void Text::setSize(qreal v)
      {
      _localStyle.setSize(v);
      }

//---------------------------------------------------------
//   setHasFrame
//---------------------------------------------------------

void Text::setHasFrame(bool val)
      {
      _localStyle.setHasFrame(val);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

Font Text::font() const
      {
      return style().font(spatium());
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Text::setScore(Score* s)
      {
      Element::setScore(s);
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void Text::setFont(const Font& f)
      {
      _localStyle.setFont(f);
      }

