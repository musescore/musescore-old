//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "simpletext.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"

//---------------------------------------------------------
//   SimpleText
//---------------------------------------------------------

SimpleText::SimpleText(Score* s)
   : Element(s)
      {
      _textStyle           = TEXT_STYLE_STAFF;
      _layoutToParentWidth = false;
      }

SimpleText::SimpleText(const SimpleText& st)
   : Element(st)
      {
      _text                = st._text;
      _textStyle           = st._textStyle;
      _layoutToParentWidth = st._layoutToParentWidth;
      }

SimpleText::~SimpleText()
      {
      }

const TextStyle& SimpleText::style() const
      {
      return score()->textStyle(_textStyle);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString SimpleText::subtypeName() const
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
            case TEXT_FIGURED_BASS:     return "FiguredBass";
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
                  qDebug("SimpleText:subtypeName: unknown text(%s) subtype %d", name(), subtype());
                  break;
            }
      return "?";
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void SimpleText::setSubtype(const QString& s)
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
      else if (s == "FiguredBass")
            st = TEXT_FIGURED_BASS;
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
            qDebug("SimpleText(%s): setSubtype: unknown type <%s>", name(), qPrintable(s));
      setSubtype(st);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void SimpleText::draw(QPainter* p) const
      {
      p->setPen(curColor());
      p->setFont(style().fontPx(spatium()));
      QRectF r;
      int flags = alignFlags();
      p->drawText(r, flags, _text);
      }

//---------------------------------------------------------
//   alignFlags
//---------------------------------------------------------

int SimpleText::alignFlags() const
      {
      int flags = Qt::TextDontClip;
      Align align = style().align();
      if (align & ALIGN_HCENTER)
            flags |= Qt::AlignHCenter;
      else if (align & ALIGN_RIGHT)
            flags |= Qt::AlignRight;
      else
            flags |= Qt::AlignLeft;
      if (align & ALIGN_VCENTER)
            flags |= Qt::AlignVCenter;
      else if (align & ALIGN_BOTTOM)
            flags |= Qt::AlignBottom;
      else if (flags & ALIGN_BASELINE)
            ;
      else
            flags |= Qt::AlignTop;
      return flags;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void SimpleText::layout()
      {
      if (parent() == 0) {
            printf("PARENT TEXT ZERO: <%s>\n", qPrintable(getText()));
            return;
            }

      const TextStyle& s(style());

      //
      // compute text bounding box
      //
      QRectF r;
      int flags = alignFlags();
      QFontMetricsF fm(s.font(spatium()));
      setbbox(fm.boundingRect(r, flags, _text));

      //
      // apply relative offset to parent
      //
      qreal pw = .0, ph = .0;
      QPointF o(s.offset(spatium()));
      if (parent()) {
            if ((type() == MARKER || type() == JUMP) && parent()->parent()) {
                  pw = parent()->parent()->width();      // measure width
                  ph = parent()->parent()->height();
                  }
            else {
                  pw = parent()->width();
                  ph = parent()->height();
                  }
            o += QPointF(s.reloff().x() * pw * 0.01, s.reloff().y() * ph * 0.01);
            }

      if (!_layoutToParentWidth) {
            pw = 0.0; // width();
            ph = 0.0; // height();
            }
      QPointF p;
      int _align = s.align();
      if (_align & ALIGN_BOTTOM)
            p.setY(ph);
      else if (_align & ALIGN_VCENTER)
            p.setY(ph * .5);
      else if (_align & ALIGN_BASELINE)
            p.setY(-baseLine());
      if (_align & ALIGN_RIGHT)
            p.setX(pw);
      else if (_align & ALIGN_HCENTER)
            p.setX(pw * .5);
      setPos(o+p);
      }

//---------------------------------------------------------
//   lineSpacing
//---------------------------------------------------------

qreal SimpleText::lineSpacing() const
      {
      return QFontMetricsF(style().font(spatium())).lineSpacing();
      }

//---------------------------------------------------------
//   lineHeight
//---------------------------------------------------------

qreal SimpleText::lineHeight() const
      {
      return QFontMetricsF(style().font(spatium())).height();
      }

//---------------------------------------------------------
//   baseLine
//---------------------------------------------------------

qreal SimpleText::baseLine() const
      {
      return QFontMetricsF(style().font(spatium())).ascent();
      }
