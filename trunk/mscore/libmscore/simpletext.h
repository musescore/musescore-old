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

#ifndef __SIMPLETEXT_H__
#define __SIMPLETEXT_H__

#include "element.h"
#include "style.h"
#include "elementlayout.h"

class MuseScoreView;
struct SymCode;

enum {
      TEXT_UNKNOWN = 0,
      TEXT_TITLE,
      TEXT_SUBTITLE,
      TEXT_COMPOSER,
      TEXT_POET,
      TEXT_TRANSLATOR,
      TEXT_MEASURE_NUMBER,
      TEXT_FINGERING,
      TEXT_INSTRUMENT_LONG,
      TEXT_INSTRUMENT_SHORT,
      TEXT_INSTRUMENT_EXCERPT,
      TEXT_TEMPO,
      TEXT_LYRIC,
      TEXT_TUPLET,
      TEXT_SYSTEM,
      TEXT_STAFF,
      TEXT_CHORD,
      TEXT_REHEARSAL_MARK,
      TEXT_REPEAT,
      TEXT_VOLTA,
      TEXT_FRAME,
      TEXT_TEXTLINE,
      TEXT_STRING_NUMBER,
      TEXT_HEADER,
      TEXT_FOOTER,
      TEXT_INSTRUMENT_CHANGE,
      TEXT_LYRICS_VERSE_NUMBER,
      TEXT_FIGURED_BASS
      };

//---------------------------------------------------------
//   SimpleText
//---------------------------------------------------------

class SimpleText : public Element {
      QString _text;
      TextStyleType _textStyle;
      bool _layoutToParentWidth;

      int alignFlags() const;

   public:
      SimpleText(Score*);
      SimpleText(const SimpleText&);
      ~SimpleText();

      SimpleText &operator=(const SimpleText&);

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      virtual void setSubtype(int val)      { Element::setSubtype(val);    }

      virtual void setTextStyle(TextStyleType st) { _textStyle = st; }
      TextStyleType textStyle() const             { return _textStyle; }
      const TextStyle& style() const;

      void setText(const QString& s)        { _text = s;    }
      QString getText() const               { return _text; }

      virtual void draw(QPainter*) const;

      virtual void layout();
      qreal lineSpacing() const;
      qreal lineHeight() const;
      virtual qreal baseLine() const;

      bool isEmpty() const                { return _text.isEmpty(); }
      void clear()                        { _text.clear();          }

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }
      };

#endif
