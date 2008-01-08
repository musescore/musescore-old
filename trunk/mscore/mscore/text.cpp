//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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
      _sizeIsSpatiumDependent = true;
      editMode                = false;
      cursorPos               = 0;
      doc                     = new QTextDocument(0);

      QTextOption to = doc->defaultTextOption();
      to.setUseDesignMetrics(true);
      to.setWrapMode(QTextOption::NoWrap);
      doc->setDefaultTextOption(to);

      cursor                  = 0;
      _frameWidth             = 0.0;
      _marginWidth            = 0.0;
      _paddingWidth           = 1.0;
      _frameColor             = QColor(Qt::black);
      _frameRound             = 5;
      setSubtype(TEXT_STAFF);
      }

Text::Text(const Text& e)
   : Element(e)
      {
      _align                  = e._align;
      _xoff                   = e._xoff;
      _yoff                   = e._yoff;
      _rxoff                  = e._rxoff;
      _ryoff                  = e._ryoff;
      _anchor                 = e._anchor;
      _offsetType             = e._offsetType;
      _sizeIsSpatiumDependent = e._sizeIsSpatiumDependent;
      _frameWidth             = e._frameWidth;
      _marginWidth            = e._marginWidth;
      _paddingWidth           = e._paddingWidth;
      _frameColor             = e._frameColor;
      _frameRound             = e._frameRound;
      editMode                = e.editMode;
      cursorPos               = e.cursorPos;
      doc                     = e.doc->clone(0);

      if (editMode) {
            cursor = new QTextCursor(doc);
            cursor->setPosition(cursorPos);
            cursor->setBlockFormat(e.cursor->blockFormat());
            cursor->setCharFormat(e.cursor->charFormat());
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
      setStyle(style());
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Text::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   style
//---------------------------------------------------------

TextStyle* Text::style() const
      {
      int st = -1;
      switch (subtype()) {
            case TEXT_TITLE:            st = TEXT_STYLE_TITLE; break;
            case TEXT_SUBTITLE:         st = TEXT_STYLE_SUBTITLE; break;
            case TEXT_COMPOSER:         st = TEXT_STYLE_COMPOSER; break;
            case TEXT_POET:             st = TEXT_STYLE_POET; break;
            case TEXT_TRANSLATOR:       st = TEXT_STYLE_TRANSLATOR; break;
            case TEXT_MEASURE_NUMBER:   st = TEXT_STYLE_MEASURE_NUMBER; break;
            case TEXT_PAGE_NUMBER_ODD:  st = TEXT_STYLE_PAGE_NUMBER_ODD; break;
            case TEXT_PAGE_NUMBER_EVEN: st = TEXT_STYLE_PAGE_NUMBER_EVEN; break;
            case TEXT_COPYRIGHT:        st = TEXT_STYLE_COPYRIGHT; break;
            case TEXT_FINGERING:        st = TEXT_STYLE_FINGERING; break;
            case TEXT_INSTRUMENT_LONG:  st = TEXT_STYLE_INSTRUMENT_LONG; break;
            case TEXT_INSTRUMENT_SHORT: st = TEXT_STYLE_INSTRUMENT_SHORT; break;
            case TEXT_TEMPO:            st = TEXT_STYLE_TEMPO; break;
            case TEXT_LYRIC:            st = TEXT_STYLE_LYRIC; break;
            case TEXT_TUPLET:           st = TEXT_STYLE_TUPLET; break;
            case TEXT_SYSTEM:           st = TEXT_STYLE_SYSTEM; break;
            case TEXT_STAFF:            st = TEXT_STYLE_STAFF; break;
            case TEXT_CHORD:            st = TEXT_STYLE_CHORD; break;
            case TEXT_REHEARSAL_MARK:   st = TEXT_STYLE_REHEARSAL_MARK; break;
            case TEXT_REPEAT:           st = TEXT_STYLE_REPEAT; break;
            case TEXT_VOLTA:            st = TEXT_STYLE_VOLTA; break;
            case TEXT_FRAME:            st = TEXT_STYLE_FRAME; break;
            default:
                  printf("unknown text subtype %d <%s>\n",
                     subtype(), qPrintable(doc->toPlainText()));
                  break;
            }
      if (st != -1)
            return score()->textStyle(st);
      return 0;
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
            case TEXT_STAFF:            return "Staff";
            case TEXT_CHORD:            return "Chordname";
            case TEXT_REHEARSAL_MARK:   return "RehearsalMark";
            case TEXT_REPEAT:           return "Repeat";
            case TEXT_VOLTA:            return "Volta";
            case TEXT_FRAME:            return "Frame";
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
      else
            printf("setSubtype: unknown type <%s>\n", qPrintable(s));
      setSubtype(st);
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
//   getText
//---------------------------------------------------------

QString Text::getText() const
      {
      return doc->toPlainText();
      }

//---------------------------------------------------------
//   getHtml
//---------------------------------------------------------

QString Text::getHtml() const
      {
      return doc->toHtml();
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Text::layout(ScoreLayout* layout)
      {
      doc->documentLayout()->setPaintDevice(layout->paintDevice());
      setbbox(QRectF(QPointF(), doc->size()));

      if (parent() == 0)
            return;

      QPointF o(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            o *= _spatium;
      else
            o *= DPI;
      o += QPointF(_rxoff * parent()->width() * 0.01, _ryoff * parent()->height() * 0.01);

      doc->setTextWidth(1000000.0);       //!? qt bug?
      double tw = doc->idealWidth();
      doc->setTextWidth(tw);
      setbbox(QRectF(QPointF(), doc->size()));

      double th = height();
      QPointF p;
      if (_align & ALIGN_BOTTOM)
            p.setY(-th);
      else if (_align & ALIGN_VCENTER)
            p.setY(-(th * .5));
      else if (_align & ALIGN_BASELINE)
            p.setY(-basePosition());
      if (_align & ALIGN_RIGHT)
            p.setX(-tw);
      else if (_align & ALIGN_HCENTER)
            p.setX(-(tw * .5));
      setPos(p + o);
      }

//---------------------------------------------------------
//   setText
//---------------------------------------------------------

void Text::setText(const QString& s)
      {
      doc->clear();
      QTextCursor cursor(doc);
      cursor.movePosition(QTextCursor::Start);
      if (_align & ALIGN_HCENTER) {
            QTextBlockFormat bf = cursor.blockFormat();
            bf.setAlignment(Qt::AlignHCenter);
            cursor.setBlockFormat(bf);
            }
      QTextCharFormat tf = cursor.charFormat();
      tf.setFont(doc->defaultFont());
      cursor.setBlockCharFormat(tf);
      cursor.insertText(s);
      }

//---------------------------------------------------------
//   setHtml
//---------------------------------------------------------

void Text::setHtml(const QString& s)
      {
      doc->setHtml(s);
      }

//---------------------------------------------------------
//   setStyle
//---------------------------------------------------------

void Text::setStyle(const TextStyle* s)
      {
      if (s == 0)
            return;
      doc->setDefaultFont(s->font());
      _align         = s->align;
      _xoff          = s->xoff;
      _yoff          = s->yoff;
      _rxoff         = s->rxoff;
      _ryoff         = s->ryoff;
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
      _align  = 0;
      _anchor = ANCHOR_PARENT;
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
      Element::writeProperties(xml);

      // write all properties which are different from style

      TextStyle* st = style();

      if (_align != st->align) {
            if (_align & (ALIGN_RIGHT | ALIGN_HCENTER)) {          // default is ALIGN_LEFT
                  xml.tag("halign", (_align & ALIGN_RIGHT) ? "right" : "center");
                  }
            if (_align & (ALIGN_BOTTOM | ALIGN_VCENTER | ALIGN_BASELINE)) {        // default is ALIGN_TOP
                  if (_align & ALIGN_BOTTOM)
                        xml.tag("valign", "bottom");
                  else if (_align & ALIGN_VCENTER)
                        xml.tag("valign", "center");
                  else if (_align & ALIGN_BASELINE)
                        xml.tag("valign", "baseline");
                  }
            }
      if (_xoff != st->xoff)
            xml.tag("xoffset", _xoff);
      if (_yoff != st->yoff)
            xml.tag("yoffset", _yoff);
      if (_rxoff != st->rxoff)
            xml.tag("rxoffset", _rxoff);
      if (_ryoff != st->ryoff)
            xml.tag("ryoffset", _ryoff);

      if (_anchor != st->anchor) {
            const char* p = 0;
            switch(_anchor) {
                  case ANCHOR_PARENT:                    break;   // default
                  case ANCHOR_MEASURE:    p = "measure"; break;
                  case ANCHOR_STAFF:      p = "staff";   break;
                  case ANCHOR_SEGMENT:    p = "segment"; break;
                  }
            if (p)
                  xml.tag("anchor", p);
            }

      if (_offsetType != st->offsetType) {
            const char* p = 0;
            switch(_offsetType) {
                  case OFFSET_SPATIUM:                    break;
                  case OFFSET_ABS:        p = "absolute"; break;
                  }
            if (p)
                  xml.tag("offsetType", p);
            }

      if (!_sizeIsSpatiumDependent && _sizeIsSpatiumDependent != st->sizeIsSpatiumDependent)
            xml.tag("spatiumSizeDependent", _sizeIsSpatiumDependent);

      xml << doc->toHtml("UTF-8") << '\n';
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool Text::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());

      if (tag == "data")                  // obsolete
            doc->setHtml(val);
      else if (tag == "html") {
            QString s = Xml::htmlToString(e);
            doc->setHtml(s);
            }
      else if (tag == "align")            // obsolete
            _align = Align(val.toInt());
      else if (tag == "halign") {
            if (val == "center")
                  _align |= ALIGN_HCENTER;
            else if (val == "right")
                  _align |= ALIGN_RIGHT;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "valign") {
            if (val == "center")
                  _align |= ALIGN_VCENTER;
            else if (val == "bottom")
                  _align |= ALIGN_BOTTOM;
            else if (val == "baseline")
                  _align |= ALIGN_BASELINE;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "xoffset")
            _xoff = val.toDouble();
      else if (tag == "yoffset")
            _yoff = val.toDouble();
      else if (tag == "rxoffset")
            _rxoff = val.toDouble();
      else if (tag == "ryoffset")
            _ryoff = val.toDouble();
      else if (tag == "anchor") {
            if (val == "parent")
                  _anchor = ANCHOR_PARENT;
            else if (val == "measure")
                  _anchor = ANCHOR_MEASURE;
            else if (val == "staff")
                  _anchor = ANCHOR_STAFF;
            else if (val == "segment")
                  _anchor = ANCHOR_SEGMENT;
            else
                  printf("Text::readProperties: unknown anchor: <%s>\n", qPrintable(val));
            }
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
      //never true?
      if (palette) {
            palette->setCharFormat(cursor->charFormat());
            palette->setBlockFormat(cursor->blockFormat());
            }

      return true;
      }

//---------------------------------------------------------
//   edit
//    return true if event is accepted
//---------------------------------------------------------

bool Text::edit(int, QKeyEvent* ev)
      {
//      score()->setLayoutAll(false);
score()->setLayoutAll(true);
      score()->addRefresh(abbox().adjusted(-6, -6, 12, 12));
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
            return true;
            }
      if (ev->modifiers() & Qt::ControlModifier) {
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
            if (key != Qt::Key_Space && key != Qt::Key_Minus)
                  return true;
            }
      QTextCursor::MoveMode mm = (ev->modifiers() & Qt::ShiftModifier)
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
                  break;

            case Qt::Key_Right:
                  cursor->movePosition(QTextCursor::Right, mm);
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

            default:
                  cursor->insertText(ev->text());
                  break;
            }
      if (palette) {
            palette->setCharFormat(cursor->charFormat());
            palette->setBlockFormat(cursor->blockFormat());
            }
//      layout(score()->mainLayout());
      score()->addRefresh(abbox().adjusted(-6, -6, 12, 12));      // HACK
      return true;
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
      if (subtype() == TEXT_COPYRIGHT) {
            score()->setCopyright(doc);
            }
      }

//---------------------------------------------------------
//   Text::draw
//---------------------------------------------------------

void Text::draw(QPainter& p) const
      {
      double dpmm = double(p.device()->logicalDpiX()) / INCH;
      p.save();
      p.setRenderHint(QPainter::Antialiasing, true);

      QAbstractTextDocumentLayout::PaintContext c;
      c.cursorPosition = editMode ? cursor->position() : -1;
      if (cursor) {
            QAbstractTextDocumentLayout::Selection sel;
            sel.cursor = *cursor;
            sel.format.setFontUnderline(true);
            c.selections.append(sel);
            }
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

      if (editMode) {
            QRectF f;
            for (QTextBlock tb = doc->begin(); tb.isValid(); tb = tb.next()) {
                  QTextLayout* tl = tb.layout();
                  int n = tl->lineCount();
                  for (int i = 0; i < n; ++i)
                        f |= tl->lineAt(0).naturalTextRect().translated(tl->position());
                  }
            qreal w = 6.0 / p.matrix().m11();   // 6 pixel border
            f.adjust(-w, -w, w, w);
            p.setPen(QPen(QBrush(Qt::blue), w / 3.0));
            p.setBrush(QBrush(Qt::NoBrush));
            p.drawRect(f);
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
//   basePosition
//    returns ascent of first text line in first block
//---------------------------------------------------------

qreal Text::basePosition() const
      {
      for (QTextBlock tb = doc->begin(); tb.isValid(); tb = tb.next()) {
            const QTextLayout* tl = tb.layout();
            if (tl->lineCount())
                  return tl->lineAt(0).ascent() + tl->position().y();
            }
      return 0.0;
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

double Text::lineSpacing() const
      {
      extern double printerMag;

      QTextBlock tb   = doc->begin();
      QTextLayout* tl = tb.layout();
      QFontMetricsF fm(tl->font());
      return fm.lineSpacing() * printerMag;     // HACK
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
            nFormat.setFontFamily(score()->textStyle(s.style)->font().family());
            cursor->setCharFormat(nFormat);
            cursor->insertText(s.code);
            cursor->setCharFormat(oFormat);
            }
      else
            cursor->insertText(s.code);
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

bool Text::setCursor(const QPointF& p)
      {
      QPointF pt  = p - canvasPos();
      if (!bbox().contains(pt))
            return false;
      int idx = doc->documentLayout()->hitTest(pt, Qt::FuzzyHit);
      if (idx == -1)
            return true;
      cursor->setPosition(idx);
      return true;
      }

//---------------------------------------------------------
//   mousePress
//    set text cursor
//---------------------------------------------------------

bool Text::mousePress(const QPointF& p, QMouseEvent* ev)
      {
      if (!setCursor(p))
            return false;

      if (ev->button() == Qt::MidButton) {
            QString txt = QApplication::clipboard()->text(QClipboard::Selection);
            cursor->insertText(txt);
            }
      return true;
      }

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

TempoText::TempoText(Score* s)
   : Text(s)
      {
      setSubtype(TEXT_TEMPO);
      _tempo = 2.0;
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

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Text::dragAnchor() const
      {
      QPointF p1(parent()->abbox().topLeft());
      double tw = width();
      double th = height();
      double x  = 0.0;
      double y  = 0.0;
      if (_align & ALIGN_BOTTOM)
            y = th;
      else if (_align & ALIGN_VCENTER)
            y = (th * .5);
      else if (_align & ALIGN_BASELINE)
            y = basePosition();
      if (_align & ALIGN_RIGHT)
            x = tw;
      else if (_align & ALIGN_HCENTER)
            x = (tw * .5);
      return QLineF(p1, QPointF(x, y) + canvasPos());
      }

