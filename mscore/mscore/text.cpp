//=============================================================================
//  MuseScore
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
#include "segment.h"

TextPalette* textPalette;

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : Element(s)
      {
      _doc = new QTextDocument(0);
      _doc->setDocumentMargin(1.0);
      _doc->setUseDesignMetrics(true);
      _doc->setUndoRedoEnabled(true);
      _doc->documentLayout()->setProperty("cursorWidth", QVariant(2));

      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      _doc->setDefaultTextOption(to);

      _editMode  = false;
      cursorPos  = 0;
      cursor     = 0;
      setFlag(ELEMENT_MOVABLE, true);
      _textStyle = TEXT_STYLE_INVALID;
      _styled    = false;
      _layoutToParentWidth = false;
      setSubtype(0);
      }

Text::Text(const Text& e)
   : Element(e)
      {
      _doc                  = e._doc->clone();
      frame                 = e.frame;
      _styled               = e._styled;
      _localStyle           = e._localStyle;
      _editMode             = e._editMode;
      cursorPos             = e.cursorPos;
      _textStyle            = e._textStyle;
      _layoutToParentWidth  = e._layoutToParentWidth;
      cursor                = 0;

      if (_editMode) {
            cursor = new QTextCursor(_doc);
            cursor->setVisualNavigation(true);
            cursor->setPosition(cursorPos);
            cursor->setBlockFormat(e.cursor->blockFormat());
            cursor->setCharFormat(e.cursor->charFormat());
            }
      }

Text::~Text()
      {
      delete _doc;
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QString& s)
      {
      Align align = style().align();
      _doc->clear();
      QFont font(style().font(spatium()));
      _doc->setDefaultFont(font);

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
      tf.setFont(font);
      cursor.setBlockCharFormat(tf);
      cursor.insertText(s);
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void Text::setHtml(const QString& s)
      {
      _doc->clear();
      _doc->setHtml(s);
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString Text::getText() const
      {
      return _doc->toPlainText();
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString Text::getHtml() const
      {
      return _doc->toHtml("utf-8");
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
      else
            printf("setSubtype: unknown type <%s>\n", qPrintable(s));
      setSubtype(st);
      }

//---------------------------------------------------------
//   resetMode
//---------------------------------------------------------

void Text::resetMode()
      {
      _editMode = 0;
      delete cursor;
      cursor = 0;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
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
            double w = (paddingWidth() + frameWidth() * .5) * DPMM;
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
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX)) {
            QRectF r = parent()->abbox();
            Box* box = static_cast<Box*>(parent());
            double x = r.x() + box->leftMargin() * DPMM;
            double y = r.y() + box->topMargin() * DPMM;
            double h = r.height() - (box->topMargin() + box->bottomMargin()) * DPMM;
            double w = r.width()  - (box->leftMargin() + box->rightMargin()) * DPMM;

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

void Text::draw(QPainter& p, ScoreView*) const
      {
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
      QColor color = p.pen().color();
      c.palette.setColor(QPalette::Text, color);

      _doc->documentLayout()->draw(&p, c);

      // draw frame
      if (hasFrame()) {
            p.setPen(QPen(QBrush(frameColor()), frameWidth() * DPMM));
            p.setBrush(QBrush(Qt::NoBrush));
            if (circle())
                  p.drawArc(frame, 0, 5760);
            else {
                  int r2 = frameRound() * lrint((frame.width() / frame.height()));
                  if (r2 > 99)
                        r2 = 99;
                  p.drawRoundRect(frame, frameRound(), r2);
                  }
            }
      }

//---------------------------------------------------------
//   setTextStyle
//---------------------------------------------------------

void Text::setTextStyle(TextStyleType idx)
      {
      _textStyle = idx;
      if (_textStyle != TEXT_STYLE_INVALID) {
            _styled = true;
            _localStyle = score()->textStyle(_textStyle);
            setText(getText());      // init style
            }
      else
            _styled = false;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml) const
      {
      write(xml, "Text");
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml, const char* name) const
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

void Text::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      if (score()->mscVersion() < 119) {
            //
            // Reset text in old version to
            // style.
            //
            if (_textStyle != TEXT_STYLE_INVALID) {
                  _styled = true;
                  styleChanged();
                  }
            }
      cursorPos = 0;
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml, bool writeText) const
      {
      if (_styled)
            xml.tag("style", score()->textStyle(_textStyle).name());
      Element::writeProperties(xml);
      if (!_styled)
            _localStyle.writeProperties(xml);
      if (writeText) {
            if (_styled)
                  xml.tag("text", getText());
            else {
                  xml.stag("html-data");
                  xml.writeHtml(_doc->toHtml("utf-8"));
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Text::spatiumChanged(double oldVal, double newVal)
      {
      Element::spatiumChanged(oldVal, newVal);
      if (!sizeIsSpatiumDependent())
            return;
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
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(QDomElement e)
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
                        case 16: i = TEXT_STYLE_INVALID; break;
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
            _styled = true;
            }
      else if (tag == "align") {            // obsolete
            _localStyle.setAlign(Align(val.toInt()));
            _styled = false;
            }
      else if (tag == "spatiumSizeDependent") {
            setSizeIsSpatiumDependent(val.toInt());
            _styled = false;
            }
      else if (tag == "data")                  // obsolete
            _doc->setHtml(val);
      else if (tag == "frame") {
            setHasFrame(val.toInt());
            _styled = false;
            }
      else if (tag == "html") {
            QString s = Xml::htmlToString(e);
            _doc->setHtml(s);
            }
      else if (tag == "text")
            setText(val);
      else if (tag == "html-data") {
            QString s = Xml::htmlToString(e.firstChildElement());
            setHtml(s);
            }
      else if (tag == "frameWidth") {
            setFrameWidth(val.toDouble());
            setHasFrame(true);
            _styled = false;
            }
      else if (tag == "paddingWidth") {
            setPaddingWidth(val.toDouble());
            _styled = false;
            }
      else if (tag == "frameColor") {
            setFrameColor(readColor(e));
            _styled = false;
            }
      else if (tag == "frameRound") {
            setFrameRound(val.toInt());
            _styled = false;
            }
      else if (tag == "circle") {
            setCircle(val.toInt());
            _styled = false;
            }
      else if (_localStyle.readProperties(e))
            _styled = false;
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Text::startEdit(ScoreView* view, const QPointF& p)
      {
      mscore->textTools()->show();
      cursor = new QTextCursor(doc());
      cursor->setVisualNavigation(true);
      cursor->setPosition(cursorPos);
      _editMode = true;
      setCursor(p);
      cursorPos = cursor->position();

      if (cursorPos == 0 && align()) {
            QTextBlockFormat bf = cursor->blockFormat();
            Qt::Alignment alignment = 0;
            if (align() & ALIGN_HCENTER)
                  alignment |= Qt::AlignHCenter;
            else if (align() & ALIGN_LEFT)
                  alignment |= Qt::AlignLeft;
            else if (align() & ALIGN_RIGHT)
                  alignment |= Qt::AlignRight;
            bf.setAlignment(alignment);
            setBlockFormat(bf);
            }
      qreal w = 8.0 / view->matrix().m11();
      score()->addRefresh(abbox().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Text::edit(ScoreView* view, int /*grip*/, int key, Qt::KeyboardModifiers modifiers, const QString& s)
      {
      if (debugMode)
            printf("Text::edit(%p) key 0x%x mod 0x%x\n", this, key, int(modifiers));
      if (!_editMode || !cursor) {
            printf("Text::edit(%p): not in edit mode: %d %p\n", this, _editMode, cursor);
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
      if (parent() && parent()->type() == TBOX) {
            TBox* tb = static_cast<TBox*>(parent());
            qreal h;
            if (isEmpty()) {
                  QFontMetricsF fm(font());
                  h = fm.lineSpacing();
                  }
            else
                  h = height();
            tb->setHeight(h);
            score()->addRefresh(tb->abbox().adjusted(-w, -w, w, w));
            }
      else
            score()->addRefresh(abbox().adjusted(-w, -w, w, w));
      return true;
      }

//---------------------------------------------------------
//   replaceSpecialChars
//---------------------------------------------------------

bool Text::replaceSpecialChars()
      {
      QTextCursor startCur = *cursor;
      foreach (const char* s, charReplaceMap.keys()) {
            SymCode sym = *charReplaceMap.value(s);
            switch (sym.type) {
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

void Text::moveCursorToEnd()
      {
      if (cursor)
            cursor->movePosition(QTextCursor::End);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void Text::moveCursor(int col)
      {
      if (cursor)
            cursor->setPosition(col);
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
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
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Text::shape() const
      {
      QPainterPath pp;

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
      return pp;
      }

//---------------------------------------------------------
//   baseLine
//    returns ascent of first text line in first block
//---------------------------------------------------------

qreal Text::baseLine() const
      {
      for (QTextBlock tb = doc()->begin(); tb.isValid(); tb = tb.next()) {
            const QTextLayout* tl = tb.layout();
            if (tl->lineCount())
                  return (tl->lineAt(0).ascent() + tl->position().y());
            }
      return 0.0;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double Text::lineSpacing() const
      {
      return QFontMetricsF(style().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//    HACK
//---------------------------------------------------------

double Text::lineHeight() const
      {
      return QFontMetricsF(style().font(spatium())).height();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Text::addSymbol(const SymCode& s, QTextCursor* cur)
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

void Text::addChar(int code, QTextCursor* cur)
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
      score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   setCursor
//---------------------------------------------------------

bool Text::setCursor(const QPointF& p, QTextCursor::MoveMode mode)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;

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

bool Text::mousePress(const QPointF& p, QMouseEvent* ev)
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

void Text::paste()
      {
#if defined(Q_WS_MAC) || defined(__MINGW32__)
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
#else
      QString txt = QApplication::clipboard()->text(QClipboard::Selection);
#endif
      if (debugMode)
            printf("Text::paste() <%s>\n", qPrintable(txt));
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

bool Text::genPropertyMenu(QMenu* popup) const
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

void Text::propertyAction(ScoreView* viewer, const QString& s)
      {
      if (s == "props") {
            Text* nText = (Text*)clone();
            TextProperties tp(nText);
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
                        if (nText->styled() != styled())
                              tt->setStyled(nText->styled());
                        if (nText->textStyle() != textStyle())
                              tt->setTextStyle(nText->textStyle());
                        if (!nText->styled()) {
                              if (nText->hasFrame() != hasFrame())
                                    tt->setHasFrame(nText->hasFrame());
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

                              if (nText->localStyle().family() != localStyle().family())
                                    tt->localStyle().setFamily(nText->localStyle().family());
                              if (nText->localStyle().size() != localStyle().size())
                                    tt->localStyle().setSize(nText->localStyle().size());
                              if (nText->localStyle().bold() != localStyle().bold())
                                    tt->localStyle().setBold(nText->localStyle().bold());
                              if (nText->localStyle().italic() != localStyle().italic())
                                    tt->localStyle().setItalic(nText->localStyle().italic());
                              if (nText->localStyle().underline() != localStyle().underline())
                                    tt->localStyle().setUnderline(nText->localStyle().underline());

                              if (nText->align() != align())
                                    tt->setAlign(nText->align());
                              if (nText->xoff() != xoff())
                                    tt->setXoff(nText->xoff());
                              if (nText->yoff() != yoff())
                                    tt->setYoff(nText->yoff());
                              if (nText->reloff() != reloff())
                                    tt->setReloff(nText->reloff());
                              if (nText->offsetType() != offsetType())
                                    tt->setOffsetType(nText->offsetType());
                              }
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

QLineF Text::dragAnchor() const
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
      if (align() & ALIGN_BOTTOM)
            y = th;
      else if (align() & ALIGN_VCENTER)
            y = (th * .5);
      else if (align() & ALIGN_BASELINE)
            y = baseLine();
      if (align() & ALIGN_RIGHT)
            x = tw;
      else if (align() & ALIGN_HCENTER)
            x = (tw * .5);
      return QLineF(p1, abbox().topLeft() + QPointF(x, y));
      }

//---------------------------------------------------------
//   dragTo
//---------------------------------------------------------

void Text::dragTo(const QPointF& p)
      {
      setCursor(p, QTextCursor::KeepAnchor);
      score()->setUpdateAll();
      score()->end();
      }

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

double Text::frameWidth() const
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

double Text::paddingWidth() const
      {
      return style().paddingWidth();
      }

//---------------------------------------------------------
//   frameColor
//---------------------------------------------------------

QColor Text::frameColor() const
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

double Text::xoff() const
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

void Text::setXoff(double val)
      {
      _localStyle.setXoff(val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Text::setYoff(double val)
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

void Text::setRxoff(double v)
      {
      _localStyle.setRxoff(v);
      }

//---------------------------------------------------------
//   setRyoff
//---------------------------------------------------------

void Text::setRyoff(double v)
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

double Text::rxoff() const
      {
      return style().rxoff();
      }

//---------------------------------------------------------
//   ryoff
//---------------------------------------------------------

double Text::ryoff() const
      {
      return style().ryoff();
      }

//---------------------------------------------------------
//   yoff
//---------------------------------------------------------

double Text::yoff() const
      {
      return style().yoff();
      }

//---------------------------------------------------------
//   setFrameWidth
//---------------------------------------------------------

void Text::setFrameWidth(double val)
      {
      _localStyle.setFrameWidth(val);
      }

//---------------------------------------------------------
//   setPaddingWidth
//---------------------------------------------------------

void Text::setPaddingWidth(double val)
      {
      _localStyle.setPaddingWidth(val);
      }

//---------------------------------------------------------
//   setFrameColor
//---------------------------------------------------------

void Text::setFrameColor(const QColor& val)
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

void Text::setSize(double v)
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

QFont Text::font() const
      {
      return style().font(spatium());
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Text::styleChanged()
      {
      if (_styled) {
            setText(getText());     // destroy formatting
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Text::setScore(Score* s)
      {
      Element::setScore(s);
      styleChanged();
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void Text::setFont(const QFont& f)
      {
      _localStyle.setFont(f);
      }

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

TextProperties::TextProperties(Text* t, QWidget* parent)
   : QDialog(parent)
      {
      setWindowTitle(tr("MuseScore: Text Properties"));
      QGridLayout* layout = new QGridLayout;

      tp = new TextProp(false, t->score());

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

      text = t;
      if (t->styled())
            text->setLocalStyle(text->score()->textStyle(text->textStyle()));
      tp->setTextStyle(text->localStyle());
      tp->setStyled(t->styled());
      tp->setTextStyleType(t->textStyle());

      connect(bb, SIGNAL(accepted()), SLOT(accept()));
      connect(bb, SIGNAL(rejected()), SLOT(reject()));
      }

//---------------------------------------------------------
//   accept
//---------------------------------------------------------

void TextProperties::accept()
      {
      text->setLocalStyle(tp->textStyle());
      QDialog::accept();
      if (tp->isStyled() != text->styled()) {
            text->setTextStyle(tp->textStyleType());  // this sets styled = true
            text->setStyled(tp->isStyled());
            text->styleChanged();
            }
      }


