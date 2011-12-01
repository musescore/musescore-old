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

#include "style.h"
#include "elementlayout.h"
#include "simpletext.h"

class MuseScoreView;
class TextProp;

struct SymCode;

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public SimpleText {
      QTextDocument* _doc;
      bool _styled;

      Q_DECLARE_TR_FUNCTIONS(Text)

      void createDoc();
      void setUnstyledText(const QString& s);

   protected:
      bool _editMode;
      QTextCursor* _cursor;

      TextStyle  _localStyle;       // use this properties if _styled is false
      QString _styleName;           // style name of _localStyle (or "")

      bool setCursor(const QPointF& p, QTextCursor::MoveMode mm = QTextCursor::MoveAnchor);

   public:
      Text(Score*);
      Text(const Text&);
      ~Text();

      Text &operator=(const Text&);
      virtual Text* clone() const         { return new Text(*this); }
      virtual ElementType type() const    { return TEXT; }

      virtual void setSubtype(const QString& s) { SimpleText::setSubtype(s); }
      virtual void setSubtype(int val)          { Element::setSubtype(val);    }

      void setText(const QString& s);
      void setText(const QTextDocumentFragment&);
      void setHtml(const QString& s);

      QString getText() const;
      QString getHtml() const;
      QTextDocumentFragment getFragment() const;

      bool sizeIsSpatiumDependent() const;
      void setSizeIsSpatiumDependent(int v);
      void setFrameWidth(qreal val);
      void setPaddingWidth(qreal val);
      void setFrameColor(const QColor& val);
      void setFrameRound(int val);
      void setCircle(bool val);
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
      void setReloff(const QPointF&);
      QFont font() const;
      void setFont(const QFont&);
      void setItalic(bool);
      void setBold(bool);
      void setSize(qreal);

      virtual void draw(QPainter*) const;

      virtual void startEdit(MuseScoreView*, const QPointF&);
      virtual bool edit(MuseScoreView*, int grip, int key, Qt::KeyboardModifiers, const QString&);
      QTextCursor* startCursorEdit();
      void endCursorEdit();
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
      virtual void spatiumChanged(qreal oldValue, qreal newValue);

      virtual void setTextStyle(TextStyleType);
      virtual const TextStyle& style() const;
      const TextStyle& localStyle() const    { return _localStyle; }
      TextStyle& localStyle()                { return _localStyle; }
      void setLocalStyle(const TextStyle& s) { _localStyle = s; }

      void dragTo(const QPointF&p);
      bool editMode() const               { return _editMode; }

      bool styled() const                 { return _styled; }
      void setStyled(bool v);

      bool isEmpty() const;
      void setModified(bool v);
      void clear();
      QRectF pageRectangle() const;
      virtual void styleChanged();
      virtual void setScore(Score* s);
      friend class TextProperties;

      virtual void textChanged()          {}
      QString styleName() const           { return _styleName; }
      void setStyleName(const QString& v) { _styleName = v;    }

      QTextCursor* cursor()               { return _cursor; }
      };

#endif
