//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "element.h"
#include "style.h"

class TextPalette;
struct SymCode;

extern TextPalette* palette;

enum {
      TEXT_TITLE=1, TEXT_SUBTITLE, TEXT_COMPOSER, TEXT_POET,
      TEXT_TRANSLATOR, TEXT_MEASURE_NUMBER,
      TEXT_PAGE_NUMBER_ODD, TEXT_PAGE_NUMBER_EVEN,
      TEXT_COPYRIGHT, TEXT_FINGERING,
      TEXT_INSTRUMENT_LONG,
      TEXT_INSTRUMENT_SHORT,
      TEXT_INSTRUMENT_EXCERPT,
      TEXT_TEMPO, TEXT_LYRIC, TEXT_TUPLET, TEXT_SYSTEM,
      TEXT_STAFF, TEXT_CHORD, TEXT_REHEARSAL_MARK,
      TEXT_REPEAT, TEXT_VOLTA, TEXT_FRAME, TEXT_TEXTLINE
      };

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase {
      QTextDocument* _doc;
      double _frameWidth;                 // unit: mm
      double _paddingWidth;               // unit: mm
      QColor _frameColor;
      int _frameRound;
      bool _circle;

      QRectF frame;
      QRectF _bbox;

   public:
      TextBase();
      TextBase(const TextBase&);

      void setDoc(const QTextDocument& d);
      QTextDocument* doc() const            { return _doc;          }
      double frameWidth() const             { return _frameWidth;   }
      double paddingWidth() const           { return _paddingWidth; }
      QColor frameColor() const             { return _frameColor;   }
      int frameRound() const                { return _frameRound;   }
      bool circle() const                   { return _circle;       }
      void setFrameWidth(double v)          { _frameWidth = v;      }
      void setPaddingWidth(double v)        { _paddingWidth = v;    }
      void setFrameColor(const QColor& v)   { _frameColor = v;      }
      void setFrameRound(int v)             { _frameRound = v;      }
      void setCircle(bool v)                { _circle = v;          }

      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement e);
      QFont defaultFont() const             { return _doc->defaultFont(); }
      void setDefaultFont(QFont f)          { _doc->setDefaultFont(f);    }
      void layout(ScoreLayout*);
      QRectF bbox() const { return _bbox; }
      void draw(QPainter&, QTextCursor*) const;
      void setText(const QString&, Align);
      QString getText() const;
      QString getHtml() const;
      void setHtml(const QString& s);
      };

//---------------------------------------------------------
//   TextB
//---------------------------------------------------------

class TextB : public Element {
      bool _movable;

   protected:
      bool _sizeIsSpatiumDependent;       // font size depends on _spatium unit
      bool editMode;
      QTextCursor* cursor;
      bool setCursor(const QPointF& p);
      int cursorPos;
      TextStyle* style() const;

   public:
      TextB(Score*);
      TextB(const TextB&);

      TextB &operator=(const TextB&);

      virtual ElementType type() const        { return TEXT; }

      virtual const QString subtypeName() const;
      virtual void setSubtype(int val);
      virtual void setSubtype(const QString& s);

      virtual bool isMovable() const        { return _movable;             }
      void setMovable(bool val)             { _movable = val;              }

      virtual TextBase* textBase() const = 0;
      void setText(const QString& s)        { textBase()->setText(s, _align); }
      QString getText() const               { return textBase()->getText(); }
      QString getHtml() const               { return textBase()->getHtml(); }
      void setHtml(const QString& s)        { textBase()->setHtml(s);       }
      void setDoc(const QTextDocument&);
      QTextDocument* doc() const            { return textBase()->doc(); }

      virtual void resetMode();
      bool isEmpty() const;
      virtual void setStyle(const TextStyle*);

      double frameWidth() const             { return textBase()->frameWidth();   }
      double paddingWidth() const           { return textBase()->paddingWidth(); }
      QColor frameColor() const             { return textBase()->frameColor();   }
      int frameRound() const                { return textBase()->frameRound();   }
      bool circle() const                   { return textBase()->circle();       }

      bool sizeIsSpatiumDependent() const   { return _sizeIsSpatiumDependent;    }
      void setSizeIsSpatiumDependent(int v) { _sizeIsSpatiumDependent = v;       }

      void setFrameWidth(double val)        { textBase()->setFrameWidth(val);    }
      void setPaddingWidth(double val)      { textBase()->setPaddingWidth(val);  }
      void setFrameColor(const QColor& val) { textBase()->setFrameColor(val);    }
      void setFrameRound(int val)           { textBase()->setFrameRound(val);    }
      void setCircle(bool val)              { textBase()->setCircle(val);        }

      virtual void draw(QPainter&) const;

      virtual bool startEdit(const QPointF&);
      virtual bool edit(int, QKeyEvent*);
      void addSymbol(const SymCode&);
      void setCharFormat(const QTextCharFormat&);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void endEdit();
      virtual void write(Xml& xml) const;
      virtual void write(Xml& xml, const char*) const;
      virtual void read(QDomElement);
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement node);
      virtual void layout(ScoreLayout*);
      virtual QPainterPath shape() const;
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      double lineSpacing() const;
      void moveCursorToEnd();

      virtual QLineF dragAnchor() const;

      QFont defaultFont() const    { return textBase()->defaultFont(); }
      void setDefaultFont(QFont f) { textBase()->setDefaultFont(f);    }

      void setAbove(bool val);
      qreal basePosition() const;
      };

//---------------------------------------------------------
//   class Text
//---------------------------------------------------------

class Text : public TextB {
      TextBase* _tb;

   public:
      Text(Score*);
      Text(const Text&);
      ~Text();
      virtual Text* clone() const           { return new Text(*this); }
      virtual TextBase* textBase() const    { return _tb; }
      };

//---------------------------------------------------------
//   class Text
//---------------------------------------------------------

class TextC : public TextB {
      TextBase** _tbb;
      TextBase*  _otb;

   public:
      TextC(TextBase**, Score*);
      TextC(const TextC&);
      ~TextC();
      virtual TextC* clone() const          { return new TextC(*this); }
      virtual TextBase* textBase() const    { return *_tbb; }
      TextBase* otb()                       { return _otb; }
      void setOtb(TextBase* b)              { _otb = b; }
      virtual void setStyle(const TextStyle*);
      void baseChanged();
      };

#endif
