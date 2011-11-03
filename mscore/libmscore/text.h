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

#ifndef __TEXT_H__
#define __TEXT_H__

#include "element.h"
#include "style.h"
#include "elementlayout.h"

class TextPalette;
class MuseScoreView;
class TextProp;

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
      TEXT_LYRICS_VERSE_NUMBER,
      TEXT_FIGURED_BASS
      };

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public Element {
      QTextDocument* _doc;
      QRectF frame;           // set by layout()
      bool _styled;
      QString s;

      Q_DECLARE_TR_FUNCTIONS(Text)

   protected:
      bool _editMode;
      QTextCursor* cursor;
      bool setCursor(const QPointF& p, QTextCursor::MoveMode mm = QTextCursor::MoveAnchor);
      int cursorPos;

      TextStyle  _localStyle;       // use this properties if _styled is false
      TextStyleType _textStyle;     // text style to use if _styled is true
      QString _styleName;           // style name of _localStyle (or "")
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

      void setText(const QString& s);
      void setText(const QTextDocumentFragment&);
      void setHtml(const QString& s);

      QString getText() const;
      QString getHtml() const;
      QTextDocumentFragment getFragment() const { return QTextDocumentFragment(_doc); }

      QTextDocument* doc() const            { return _doc; }

      qreal frameWidth() const;
      qreal paddingWidth() const;
      QColor frameColor() const;
      int frameRound() const;
      bool circle() const;
      bool sizeIsSpatiumDependent() const;
      void setSizeIsSpatiumDependent(int v);
      void setFrameWidth(qreal val);
      void setPaddingWidth(qreal val);
      void setFrameColor(const QColor& val);
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
      QFont font() const;
      void setFont(const QFont&);
      void setItalic(bool);
      void setBold(bool);
      void setSize(qreal);

      virtual void draw(Painter*) const;

      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString&);
      virtual void endEdit();
      void addSymbol(const SymCode&, QTextCursor* c = 0);
      void addChar(int code, QTextCursor* cur = 0);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void write(Xml& xml) const;
      virtual void write(Xml& xml, const char*) const;
      virtual void read(QDomElement);
      void writeProperties(Xml& xml, bool writeText = true) const;
      bool readProperties(QDomElement node);
      virtual void layout();
      virtual void layout(qreal width, qreal x, qreal y);
      virtual QPainterPath shape() const;
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      qreal lineSpacing() const;
      qreal lineHeight() const;
      void moveCursorToEnd();
      void moveCursor(int val);

      virtual QLineF dragAnchor() const;

      void setAbove(bool val);
      virtual qreal baseLine() const;
      virtual void paste();

      bool replaceSpecialChars();
      QTextCursor* getCursor() const { return cursor; }

      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      virtual void setTextStyle(TextStyleType);
      TextStyleType textStyle() const        { return _textStyle; }
      const TextStyle& style() const;
      const TextStyle& localStyle() const    { return _localStyle; }
      TextStyle& localStyle()                { return _localStyle; }
      void setLocalStyle(const TextStyle& s) { _localStyle = s; }

      void dragTo(const QPointF&p);
      bool editMode() const { return _editMode; }

      bool layoutToParentWidth() const    { return _layoutToParentWidth; }
      void setLayoutToParentWidth(bool v) { _layoutToParentWidth = v;   }
      bool styled() const                 { return _styled; }
      void setStyled(bool v);

      bool isEmpty() const                { return _doc->isEmpty(); }
      void setModified(bool v)            { _doc->setModified(v);   }
      void clear()                        { _doc->clear();          }
      QRectF pageRectangle() const;
      virtual void styleChanged();
      virtual void setScore(Score* s);
      friend class TextProperties;

      virtual void textChanged()          {}
      QString styleName() const           { return _styleName; }
      void setStyleName(const QString& v) { _styleName = v;    }
      };

#endif
