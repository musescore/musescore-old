//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "text.h"
#include "xml.h"
#include "style.h"
#include "score.h"
#include "utils.h"
#include "page.h"
#include "sym.h"
#include "symbol.h"
#include "textline.h"
#include "system.h"
#include "measure.h"
#include "box.h"
#include "segment.h"
#include "mscore.h"

QReadWriteLock docRenderLock;

//---------------------------------------------------------
//   propertyList
//---------------------------------------------------------

Property<Text> Text::propertyList[] = {
      { P_TEXT_STYLE,  &Text::pStyleIndex, 0 },
      { P_END, 0, 0 }
      };

//---------------------------------------------------------
//   createDoc
//---------------------------------------------------------

void Text::createDoc()
      {
      _doc = new QTextDocument(0);
      _doc->setDocumentMargin(0.0);
      _doc->setUseDesignMetrics(true);
      _doc->setUndoRedoEnabled(true);
      _doc->documentLayout()->setProperty("cursorWidth", QVariant(2));
      QTextOption to = _doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      _doc->setDefaultTextOption(to);
      _doc->setDefaultFont(textStyle().font(spatium()));
      }

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

Text::Text(Score* s)
   : SimpleText(s)
      {
      setFlag(ELEMENT_MOVABLE, true);
      _doc       = 0;
      _editMode  = false;
      _cursor    = 0;
      _styleIndex = TEXT_STYLE_DEFAULT;
      }

Text::Text(const Text& e)
   : SimpleText(e)
      {
      if (e._doc)
            _doc = e._doc->clone();
      else
            _doc = 0;
      _styleIndex = e._styleIndex;
      _editMode   = false;
      _cursor     = 0;
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
      if (styled())
            SimpleText::setText(s);
      else
            setUnstyledText(s);
      textChanged();
      }

//---------------------------------------------------------
//   setUnstyledText
//---------------------------------------------------------

void Text::setUnstyledText(const QString& s)
      {
      Align align = textStyle().align();
      _doc->clear();

      QTextCursor c(_doc);
      c.setVisualNavigation(true);
      c.movePosition(QTextCursor::Start);
      Qt::Alignment a;
      if (align & ALIGN_HCENTER)
            a = Qt::AlignHCenter;
      else if (align & ALIGN_RIGHT)
            a = Qt::AlignRight;
      else
            a = Qt::AlignLeft;
      QTextBlockFormat bf = c.blockFormat();
      bf.setAlignment(a);
      c.setBlockFormat(bf);

      QTextCharFormat tf = c.charFormat();
      tf.setFont(textStyle().font(spatium()));
      c.setBlockCharFormat(tf);
      c.insertText(s);
      textChanged();
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QTextDocumentFragment& f)
      {
      setHtml(f.toHtml());
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void Text::setHtml(const QString& s)
      {
      setUnstyled();
      _doc->clear();
      _doc->setHtml(s);
      textChanged();
      }

//---------------------------------------------------------
//   getText
//---------------------------------------------------------

QString Text::getText() const
      {
      return (styled() && !_editMode) ? SimpleText::getText() : _doc->toPlainText();
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString Text::getHtml() const
      {
      return styled() ? "" : _doc->toHtml("utf-8");
      }

//---------------------------------------------------------
//   systemFlag
//---------------------------------------------------------

bool Text::systemFlag() const
      {
      return textStyle().systemFlag();
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Text::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout()
      {
      if (styled() && !_editMode) {
            SimpleText::layout();
            }
      else {
            _doc->setDefaultFont(textStyle().font(spatium()));
            qreal w = -1.0;
            qreal x = 0.0;
            qreal y = 0.0;
            if (parent() && layoutToParentWidth()) {
                  w = parent()->width();
                  if (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX) {
                        Box* box = static_cast<Box*>(parent());
                        x += box->leftMargin() * MScore::DPMM;
                        y += box->topMargin() * MScore::DPMM;
                        w = box->width()   - ((box->leftMargin() + box->rightMargin()) * MScore::DPMM);
                        }
                  }

            QTextOption to = _doc->defaultTextOption();
            to.setUseDesignMetrics(true);
            to.setWrapMode(w <= 0.0 ? QTextOption::NoWrap : QTextOption::WrapAtWordBoundaryOrAnywhere);
            _doc->setDefaultTextOption(to);

            if (w < 0.0)
                  w = _doc->idealWidth();
            _doc->setTextWidth(w);

            setbbox(QRectF(QPointF(0.0, 0.0), _doc->size()));
            if (hasFrame())
                  layoutFrame();
            _doc->setModified(false);
            textStyle().layout(this);      // process alignment

#if 0 // TODO  TEXT_STYLE_TEXTLINE
            if ((textStyle().align() & ALIGN_VCENTER) && (textStyle() == TEXT_STYLE_TEXTLINE)) {
                  // special case: vertically centered text with TextLine needs to
                  // take into account the line width
                  TextLineSegment* tls = static_cast<TextLineSegment*>(parent());
                  TextLine* tl = tls->textLine();
                  if (tl) {
                        qreal textlineLineWidth = point(tl->lineWidth());
                        rypos() -= textlineLineWidth * .5;
                        }
                  }
#endif
            rxpos() += x;
            rypos() += y;
            }
      if (parent() && parent()->type() == SEGMENT) {
            Segment* s = static_cast<Segment*>(parent());
            rypos() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
            }
      adjustReadPos();
      }

//---------------------------------------------------------
//   pageRectangle
//---------------------------------------------------------

QRectF Text::pageRectangle() const
      {
      if (parent() && (parent()->type() == HBOX || parent()->type() == VBOX || parent()->type() == TBOX)) {
            QRectF r = parent()->abbox();
            Box* box = static_cast<Box*>(parent());
            qreal x = r.x() + box->leftMargin() * MScore::DPMM;
            qreal y = r.y() + box->topMargin() * MScore::DPMM;
            qreal h = r.height() - (box->topMargin() + box->bottomMargin()) * MScore::DPMM;
            qreal w = r.width()  - (box->leftMargin() + box->rightMargin()) * MScore::DPMM;

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

void Text::draw(QPainter* painter) const
      {
      if (styled() && !_editMode) {
            SimpleText::draw(painter);
            return;
            }
      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = -1;
      if (_cursor && !(score() && score()->printing())) {
            if (_cursor->hasSelection()) {
                  QAbstractTextDocumentLayout::Selection selection;
                  selection.cursor = *_cursor;
                  selection.format.setBackground(c.palette.brush(QPalette::Active, QPalette::Highlight));
                  selection.format.setForeground(c.palette.brush(QPalette::Active, QPalette::HighlightedText));
                  c.selections.append(selection);
                  }
            c.cursorPosition = _cursor->position();
            }
      bool printing = score() && score()->printing();
      if ((printing || !score()->showInvisible()) && !visible())
            return;
      c.palette.setColor(QPalette::Text, textColor());

#if 1
      // make it thread save
      {
      QWriteLocker locker(&docRenderLock);
      QScopedPointer<QTextDocument> __doc(_doc->clone());
      __doc.data()->documentLayout()->draw(painter, c);
      // _doc->documentLayout()->draw(painter, c);
      }
#else
      _doc->documentLayout()->draw(painter, c);
#endif
      drawFrame(painter);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Text::write(Xml& xml) const
      {
      if (isEmpty())
            return;
      xml.stag(name());
      writeProperties(xml, true);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Text::read(const QDomElement& de)
      {
      for (QDomElement e = de.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   writeProperties
//---------------------------------------------------------

void Text::writeProperties(Xml& xml, bool writeText) const
      {
      Element::writeProperties(xml);
      if (xml.clipboardmode || styled())
            xml.tag("style", textStyle().name());
      if (xml.clipboardmode || !styled())
            _textStyle.writeProperties(xml);
      if (writeText) {
            if (styled())
                  xml.tag("text", getText());
            else {
                  xml.stag("html-data");
                  xml.writeHtml(_doc->toHtml("utf-8"));
                  xml.etag();
                  }
            }
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(const QDomElement& e)
      {
      const QString& tag(e.tagName());
      const QString& val(e.text());

      if (tag == "style") {
            int st;
            bool ok;
            int i = val.toInt(&ok);
            if (ok) {
                  // obsolete old text styles
                  switch (i) {
                        case 1:  i = TEXT_STYLE_UNSTYLED;  break;
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
                        case 16: i = TEXT_STYLE_FOOTER;    break;  // TEXT_STYLE_COPYRIGHT
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
                        case 0:
                        default:
                              qDebug("Text:readProperties: style %d<%s> invalid", i, qPrintable(val));
                              i = TEXT_STYLE_UNSTYLED;
                              break;
                        }
                  st = i;
                  }
            else
                  st = score()->style()->textStyleType(val);

            if (st == TEXT_STYLE_UNSTYLED)
                  setUnstyled();
            else if (st == TEXT_STYLE_UNKNOWN)
                  _styleIndex = st;
            else
                  setTextStyleType(st);
            }
      else if (tag == "styleName")          // obsolete, unstyled text
            ; // _styleName = val;
      else if (tag == "data")                  // obsolete
            _doc->setHtml(val);
      else if (tag == "html") {
            QString s = Xml::htmlToString(e);
            setHtml(s);
            }
      else if (tag == "text")
            setText(val);
      else if (tag == "html-data") {
            QString s = Xml::htmlToString(e.firstChildElement());
            setHtml(s);
            }
      else if (tag == "subtype")          // obsolete
            ;
      else if (_textStyle.readProperties(e))
            ; // setUnstyled();
      else if (!Element::readProperties(e))
            return false;
      return true;
      }

//---------------------------------------------------------
//   spatiumChanged
//---------------------------------------------------------

void Text::spatiumChanged(qreal oldVal, qreal newVal)
      {
      Element::spatiumChanged(oldVal, newVal);
#if 0
qDebug("Text::spatiumChanged %d  -- %s %s %p %f\n",
      sizeIsSpatiumDependent(), name(), parent() ? parent()->name() : "?", this, newVal);
#endif
      if (!sizeIsSpatiumDependent() || styled())
            return;
      qreal v = newVal / oldVal;
      QTextCursor c(_doc);
      c.movePosition(QTextCursor::Start);
      for (;;) {
            c.select(QTextCursor::BlockUnderCursor);
            QTextCharFormat cf = c.charFormat();
            QFont font = cf.font();
            font.setPointSizeF(font.pointSizeF() * v);
            cf.setFont(font);
            c.setCharFormat(cf);
            if (!c.movePosition(QTextCursor::NextBlock))
                  break;
            }
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Text::startEdit(MuseScoreView*, const QPointF& p)
      {
      _editMode = true;
      if (styled()) {
            createDoc();
            setUnstyledText(SimpleText::getText());
            layout();
            }
      _cursor = new QTextCursor(_doc);
      _cursor->setVisualNavigation(true);
      setCursor(p);

      if (_cursor->position() == 0 && align()) {
            QTextBlockFormat bf = _cursor->blockFormat();
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
      qreal w = 2.0; // 8.0 / view->matrix().m11();
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Text::edit(MuseScoreView*, int /*grip*/, int key, Qt::KeyboardModifiers modifiers, const QString& s)
      {
      if (MScore::debugMode)
            qDebug("Text::edit(%p) key 0x%x mod 0x%x\n", this, key, int(modifiers));
      if (!_editMode || !_cursor) {
            qDebug("Text::edit(%p): not in edit mode: %d %p\n", this, _editMode, _cursor);
            return false;
            }
      bool lo = type() == INSTRUMENT_NAME;
      score()->setLayoutAll(lo);
      static const qreal w = 2.0; // 8.0 / view->matrix().m11();
      score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));

      if (modifiers == Qt::ControlModifier) {
            switch (key) {
                  case Qt::Key_A:   // select all
                        _cursor->select(QTextCursor::Document);
                        break;
                  case Qt::Key_B:   // toggle bold face
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontWeight(f.fontWeight() == QFont::Bold ? QFont::Normal : QFont::Bold);
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_I:   // toggle italic
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontItalic(!f.fontItalic());
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_U:   // toggle underline
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        f.setFontUnderline(!f.fontUnderline());
                        _cursor->setCharFormat(f);
                        }
                        break;
                  case Qt::Key_Up:
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSuperScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSubScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
                        _cursor->setCharFormat(f);
                        }
                        break;

                  case Qt::Key_Down:
                        {
                        QTextCharFormat f = _cursor->charFormat();
                        if (f.verticalAlignment() == QTextCharFormat::AlignNormal)
                              f.setVerticalAlignment(QTextCharFormat::AlignSubScript);
                        else if (f.verticalAlignment() == QTextCharFormat::AlignSuperScript)
                              f.setVerticalAlignment(QTextCharFormat::AlignNormal);
                        _cursor->setCharFormat(f);
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
                  _cursor->insertText(QString("\r"));
                  break;

            case Qt::Key_Backspace:
                  _cursor->deletePreviousChar();
                  break;

            case Qt::Key_Delete:
                  _cursor->deleteChar();
                  break;

            case Qt::Key_Left:
                  if (!_cursor->movePosition(QTextCursor::Left, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  break;

            case Qt::Key_Right:
                  if (!_cursor->movePosition(QTextCursor::Right, mm) && (type() == LYRICS || type() == FIGURED_BASS))
                        return false;
                  break;

            case Qt::Key_Up:
                  _cursor->movePosition(QTextCursor::Up, mm);
                  break;

            case Qt::Key_Down:
                  _cursor->movePosition(QTextCursor::Down, mm);
                  break;

            case Qt::Key_Home:
                  _cursor->movePosition(QTextCursor::Start, mm);
                  break;

            case Qt::Key_End:
                  _cursor->movePosition(QTextCursor::End, mm);
                  break;

            case Qt::Key_Space:
                  _cursor->insertText(" ");
                  break;

            case Qt::Key_Minus:
                  _cursor->insertText("-");
                  break;

            default:
                  if (!s.isEmpty())
                        _cursor->insertText(s);
                  break;
            }
      if (key == Qt::Key_Return || key == Qt::Key_Space || key == Qt::Key_Tab) {
            replaceSpecialChars();
            }
      layoutEdit();
      return true;
      }

//---------------------------------------------------------
//   layoutEdit
//---------------------------------------------------------

void Text::layoutEdit()
      {
      layout();
      if (parent() && parent()->type() == TBOX) {
            TBox* tbox = static_cast<TBox*>(parent());
            tbox->layout();
            System* system = tbox->system();
            system->setHeight(tbox->height());
            score()->doLayoutPages();
            score()->setUpdateAll(true);
            }
      else {
            static const qreal w = 2.0; // 8.0 / view->matrix().m11();
            score()->addRefresh(canvasBoundingRect().adjusted(-w, -w, w, w));
            }
      }

//---------------------------------------------------------
//   replaceSpecialChars
//---------------------------------------------------------

bool Text::replaceSpecialChars()
      {
      QTextCursor startCur = *_cursor;
      foreach (const char* s, charReplaceMap.keys()) {
            SymCode sym = *charReplaceMap.value(s);
            switch (sym.type) {
                  case SYMBOL_FRACTION:
                        if (!MScore::replaceFractions)
                              continue;
                        break;
                  default:
                        ;
                  }
            QTextCursor cur = _doc->find(s, _cursor->position() - 1 - strlen(s),
                  QTextDocument::FindWholeWords);
            if (cur.isNull())
                  continue;
            // do not go beyond the cursor
            if (cur.selectionEnd() > _cursor->selectionEnd())
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
      if (_cursor)
            _cursor->movePosition(QTextCursor::End);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void Text::moveCursor(int col)
      {
      if (_cursor)
            _cursor->setPosition(col);
      }

//---------------------------------------------------------
//   shape
//---------------------------------------------------------

QPainterPath Text::shape() const
      {
      if (styled())
            return SimpleText::shape();
      QPainterPath pp;

      for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
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
      if (styled())
            return SimpleText::baseLine();
      for (QTextBlock tb = _doc->begin(); tb.isValid(); tb = tb.next()) {
            const QTextLayout* tl = tb.layout();
            if (tl->lineCount())
                  return (tl->lineAt(0).ascent() + tl->position().y());
            }
      return 0.0;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal Text::lineSpacing() const
      {
      return QFontMetricsF(textStyle().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//    HACK
//---------------------------------------------------------

qreal Text::lineHeight() const
      {
      return QFontMetricsF(textStyle().font(spatium())).height();
      }

//---------------------------------------------------------
//   addSymbol
//---------------------------------------------------------

void Text::addSymbol(const SymCode& s, QTextCursor* cur)
      {
      if (cur == 0)
            cur = _cursor;
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
            cur = _cursor;

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
//   setBlockFormat
//---------------------------------------------------------

void Text::setBlockFormat(const QTextBlockFormat& bf)
      {
      if (!_cursor)
            return;
      _cursor->setBlockFormat(bf);
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

      int idx = _doc->documentLayout()->hitTest(pt, Qt::FuzzyHit);
      if (idx == -1)
            return true;
      if (_cursor) {
            _cursor->setPosition(idx, mode);
            if (_cursor->hasSelection())
                  QApplication::clipboard()->setText(_cursor->selectedText(), QClipboard::Selection);
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
      QString txt = QApplication::clipboard()->text(QClipboard::Clipboard);
      if (MScore::debugMode)
            qDebug("Text::paste() <%s>\n", qPrintable(txt));
      _cursor->insertText(txt);
      layoutEdit();
      bool lo = type() == INSTRUMENT_NAME;
      score()->setLayoutAll(lo);
      score()->setUpdateAll();
      score()->end();
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
            qreal yp       = system->staff(staffIdx())->y() + system->y() + system->page()->pos().x();
            qreal xp       = m->canvasPos().x(); // m->tick2pos(tick()) + m->canvasPos().x();
            p1 = QPointF(xp, yp);
            }
      else {
            p1 = QPointF(parent()->canvasBoundingRect().topLeft());
            if (parent()->type() == SEGMENT) {
                  Segment* s = static_cast<Segment*>(parent());
                  p1.ry() += s ? s->measure()->system()->staff(staffIdx())->y() : 0.0;
                  }
            }
      qreal tw = width();
      qreal th = height();
      qreal x  = 0.0;
      qreal y  = 0.0;
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
      return QLineF(p1, canvasBoundingRect().topLeft() + QPointF(x, y));
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
//   sizeIsSpatiumDependent
//---------------------------------------------------------

bool Text::sizeIsSpatiumDependent() const
      {
      return textStyle().sizeIsSpatiumDependent();
      }

//---------------------------------------------------------
//   setSizeIsSpatiumDependent
//---------------------------------------------------------

void Text::setSizeIsSpatiumDependent(int v)
      {
      _textStyle.setSizeIsSpatiumDependent(v);
      }

//---------------------------------------------------------
//   xoff
//---------------------------------------------------------

qreal Text::xoff() const
      {
      return textStyle().offset().x();
      }

//---------------------------------------------------------
//   align
//---------------------------------------------------------

Align Text::align() const
      {
      return textStyle().align();
      }

//---------------------------------------------------------
//   offsetType
//---------------------------------------------------------

OffsetType Text::offsetType() const
      {
      return textStyle().offsetType();
      }

//---------------------------------------------------------
//   reloff
//---------------------------------------------------------

QPointF Text::reloff() const
      {
      return textStyle().reloff();
      }

//---------------------------------------------------------
//   setAlign
//---------------------------------------------------------

void Text::setAlign(Align val)
      {
      _textStyle.setAlign(val);
      }

//---------------------------------------------------------
//   setXoff
//---------------------------------------------------------

void Text::setXoff(qreal val)
      {
      _textStyle.setXoff(val);
      }

//---------------------------------------------------------
//   setYoff
//---------------------------------------------------------

void Text::setYoff(qreal val)
      {
      _textStyle.setYoff(val);
      }

//---------------------------------------------------------
//   setOffsetType
//---------------------------------------------------------

void Text::setOffsetType(OffsetType val)
      {
      _textStyle.setOffsetType(val);
      }

//---------------------------------------------------------
//   setRxoff
//---------------------------------------------------------

void Text::setRxoff(qreal v)
      {
      _textStyle.setRxoff(v);
      }

//---------------------------------------------------------
//   setRyoff
//---------------------------------------------------------

void Text::setRyoff(qreal v)
      {
      _textStyle.setRyoff(v);
      }

//---------------------------------------------------------
//   setReloff
//---------------------------------------------------------

void Text::setReloff(const QPointF& p)
      {
      _textStyle.setReloff(p);
      }

//---------------------------------------------------------
//   yoff
//---------------------------------------------------------

qreal Text::yoff() const
      {
      return textStyle().offset().y();
      }

//---------------------------------------------------------
//   setFrameWidth
//---------------------------------------------------------

void Text::setFrameWidth(qreal val)
      {
      _textStyle.setFrameWidth(val);
      }

//---------------------------------------------------------
//   setPaddingWidth
//---------------------------------------------------------

void Text::setPaddingWidth(qreal val)
      {
      _textStyle.setPaddingWidth(val);
      }

//---------------------------------------------------------
//   setFrameColor
//---------------------------------------------------------

void Text::setFrameColor(const QColor& val)
      {
      _textStyle.setFrameColor(val);
      }

//---------------------------------------------------------
//   setFrameRound
//---------------------------------------------------------

void Text::setFrameRound(int val)
      {
      _textStyle.setFrameRound(val);
      }

//---------------------------------------------------------
//   setCircle
//---------------------------------------------------------

void Text::setCircle(bool val)
      {
      _textStyle.setCircle(val);
      }

//---------------------------------------------------------
//   setItalic
//---------------------------------------------------------

void Text::setItalic(bool val)
      {
      _textStyle.setItalic(val);
      }

//---------------------------------------------------------
//   setBold
//---------------------------------------------------------

void Text::setBold(bool val)
      {
      _textStyle.setBold(val);
      }

//---------------------------------------------------------
//   setSize
//---------------------------------------------------------

void Text::setSize(qreal v)
      {
      _textStyle.setSize(v);
      }

//---------------------------------------------------------
//   setHasFrame
//---------------------------------------------------------

void Text::setHasFrame(bool val)
      {
      _textStyle.setHasFrame(val);
      }

//---------------------------------------------------------
//   font
//---------------------------------------------------------

QFont Text::font() const
      {
      return _textStyle.font(spatium());
      }

//---------------------------------------------------------
//   styleChanged
//---------------------------------------------------------

void Text::styleChanged()
      {
      if (styled()) {
            if (_styleIndex != TEXT_STYLE_UNKNOWN)
                  setTextStyle(score()->textStyle(_styleIndex));
            setText(getText());     // destroy formatting
            score()->setLayoutAll(true);
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Text::setScore(Score* s)
      {
      if (s == score())
            return;
      Element::setScore(s);
      // TODO: handle custom text styles
      styleChanged();
      }

//---------------------------------------------------------
//   setFont
//---------------------------------------------------------

void Text::setFont(const QFont& f)
      {
      _textStyle.setFont(f);
      }

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

void Text::clear()
      {
      if (styled())
            SimpleText::clear();
      else
            _doc->clear();
      }

//---------------------------------------------------------
//   setTextStyleType
//---------------------------------------------------------

void Text::setTextStyleType(int st)
      {
      if (st == _styleIndex)
            return;
      _styleIndex = st;
      if (st != TEXT_STYLE_UNKNOWN)
            setTextStyle(score()->textStyle(st));
      if (_doc && !_doc->isEmpty() && !_editMode) {
            SimpleText::setText(_doc->toPlainText());
            delete _doc;
            _doc = 0;
            }
      }

//---------------------------------------------------------
//   setUnstyled
//---------------------------------------------------------

void Text::setUnstyled()
      {
      if (!styled())
            return;
      _styleIndex = TEXT_STYLE_UNSTYLED;
      if (_editMode)
            return;
      createDoc();
      if (!SimpleText::isEmpty())
            setUnstyledText(SimpleText::getText());
      }

//---------------------------------------------------------
//   startCursorEdit
//---------------------------------------------------------

QTextCursor* Text::startCursorEdit()
      {
      if (styled()) {
            qDebug("Text::startCursorEdit(): edit styled text\n");
            return 0;
            }
      if (_cursor) {
            qDebug("Text::startCursorEdit(): cursor already active\n");
            return 0;
            }
      _cursor = new QTextCursor(_doc);
      return _cursor;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Text::endEdit()
      {
      if (!_cursor) {
            qDebug("endEdit<%p>: no cursor: edit mode %d %p\n", this, _editMode, _cursor);
            return;
            }
      _editMode = false;
      endCursorEdit();
      layoutEdit();
      textChanged();
      }

//---------------------------------------------------------
//   endCursorEdit
//---------------------------------------------------------

void Text::endCursorEdit()
      {
      delete _cursor;
      _cursor = 0;
      if (styled()) {
            SimpleText::setText(_doc->toPlainText());
            delete _doc;
            _doc = 0;
            }
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Text::isEmpty() const
      {
      return styled() ? SimpleText::getText().isEmpty() : _doc->isEmpty();
      }

//---------------------------------------------------------
//   setModified
//---------------------------------------------------------

void Text::setModified(bool v)
      {
      if (!styled())
            _doc->setModified(v);
      }

//---------------------------------------------------------
//   getFragment
//---------------------------------------------------------

QTextDocumentFragment Text::getFragment() const
      {
      if (styled())
            return QTextDocumentFragment::fromPlainText(getText());
      else
            return QTextDocumentFragment(_doc);
      }

Property<Text>* Text::property(P_ID id) const
      {
      for (int i = 0;; ++i) {
            if (propertyList[i].id == P_END)
                  break;
            if (propertyList[i].id == id)
                  return &propertyList[i];
            }
      return 0;
      }

QVariant Text::getProperty(P_ID propertyId) const
      {
      Property<Text>* p = property(propertyId);
      if (p)
            return getVariant(propertyId, ((*(Text*)this).*(p->data))());
      return Element::getProperty(propertyId);
      }

bool Text::setProperty(P_ID propertyId, const QVariant& v)
      {
      score()->addRefresh(canvasBoundingRect());
      Property<Text>* p = property(propertyId);
      bool rv = true;
      if (p) {
            setVariant(propertyId, ((*this).*(p->data))(), v);
            setGenerated(false);
            }
      else
            rv = Element::setProperty(propertyId, v);
      score()->setLayoutAll(true);
      return rv;
      }

