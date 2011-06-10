//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2002-2011 Werner Schweer
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

#ifndef __TEXT_H__
#define __TEXT_H__

// #include <QtGui/QTextDocument>
// #include <QtGui/QTextDocumentFragment>

// #include <QtGui/QTextCursor>

#include "globals.h"
#include "element.h"
#include "style.h"
#include "elementlayout.h"
#include "font.h"

class TextPalette;
class TextProp;
class Painter;

struct SymCode;

extern TextPalette* palette;

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
      TEXT_LYRICS_VERSE_NUMBER
      };

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public Element {
//      QTextDocument* _doc;
      QString _text;
      QRectF frame;           // set by layout()
      bool _styled;
      qreal _lineSpacing;
      qreal _lineHeight;
      qreal _baseLine;

   protected:
      TextStyle  _localStyle;       // use this properties if _styled is false
      TextStyleType _textStyle;     // text style to use if _styled is true
      bool _layoutToParentWidth;

   public:
      Text(Score*);
      Text(const Text&);
      ~Text();

      Text &operator=(const Text&);
      virtual Text* clone() const         { return new Text(*this); }
      virtual ElementType type() const    { return TEXT; }

      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      virtual void setSubtype(int val)      { Element::setSubtype(val);    }

      void setText(const QString& s)        { _text = s; }
//      void setText(const QTextDocumentFragment&);
      void setHtml(const QString& s);

      QString getText() const;
      QString getHtml() const;
//      QTextDocumentFragment getFragment() const { return QTextDocumentFragment(_doc); }

//      QTextDocument* doc() const            { return _doc; }

      virtual void resetMode();

      qreal frameWidth() const;
      qreal paddingWidth() const;
      Color frameColor() const;
      int frameRound() const;
      bool circle() const;
      bool sizeIsSpatiumDependent() const;
      void setSizeIsSpatiumDependent(int v);
      void setFrameWidth(qreal val);
      void setPaddingWidth(qreal val);
      void setFrameColor(const Color& val);
      void setFrameRound(int val);
      void setCircle(bool val);
      bool hasFrame() const;
      void setHasFrame(bool);
      qreal xoff() const;
      qreal yoff() const;
      Align align() const;
      OffsetType offsetType() const;
      QPointF reloff() const;
      void setAlign(Align val);
      void setXoff(qreal val);
      void setYoff(qreal val);
      void setOffsetType(OffsetType val);
      void setRxoff(qreal v);
      void setRyoff(qreal v);
      qreal rxoff() const;
      qreal ryoff() const;
      void setReloff(const QPointF&);
      Font font() const;
      void setFont(const Font&);
      void setItalic(bool);
      void setBold(bool);
      void setSize(qreal);

      virtual void draw(Painter*) const;

//      void addSymbol(const SymCode&, QTextCursor* c = 0);
//      void addChar(int code, QTextCursor* cur = 0);
//      void setCharFormat(const QTextCharFormat&);
//      void setBlockFormat(const QTextBlockFormat&);
      virtual void read(XmlReader*);
      bool readProperties(XmlReader* node);
      virtual void layout();
//      virtual QPainterPath shape() const;

      qreal lineSpacing() const      { return _lineSpacing; }
      qreal lineHeight() const       { return _lineHeight; }
      virtual qreal baseLine() const { return _baseLine; }

      void moveCursorToEnd();
      void moveCursor(int val);

      void setAbove(bool val);

      bool replaceSpecialChars();

      virtual void setTextStyle(TextStyleType);
      TextStyleType textStyle() const        { return _textStyle; }
      const TextStyle& style() const;
      const TextStyle& localStyle() const    { return _localStyle; }
      TextStyle& localStyle()                { return _localStyle; }
      void setLocalStyle(const TextStyle& s) { _localStyle = s; }

      void dragTo(const QPointF&p);

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }
      bool styled() const                 { return _styled; }
      void setStyled(bool v);

      bool isEmpty() const                { return true; } // return _doc->isEmpty(); }
      void setModified(bool /*v*/)            { } // _doc->setModified(v);   }
      void clear()                        { } // _doc->clear();          }
      QRectF pageRectangle() const;
      virtual void setScore(Score* s);
      };

#endif
