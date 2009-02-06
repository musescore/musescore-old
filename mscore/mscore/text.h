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

#include "ui_textproperties.h"

class TextPalette;
class Viewer;

struct SymCode;

extern TextPalette* palette;

enum {
      TEXT_UNKNOWN = 0, TEXT_TITLE, TEXT_SUBTITLE, TEXT_COMPOSER, TEXT_POET,
      TEXT_TRANSLATOR, TEXT_MEASURE_NUMBER,
      TEXT_PAGE_NUMBER_ODD, TEXT_PAGE_NUMBER_EVEN,
      TEXT_COPYRIGHT, TEXT_FINGERING,
      TEXT_INSTRUMENT_LONG,
      TEXT_INSTRUMENT_SHORT,
      TEXT_INSTRUMENT_EXCERPT,
      TEXT_TEMPO, TEXT_LYRIC, TEXT_TUPLET, TEXT_SYSTEM,
      TEXT_STAFF, TEXT_CHORD, TEXT_REHEARSAL_MARK,
      TEXT_REPEAT, TEXT_VOLTA, TEXT_FRAME, TEXT_TEXTLINE,
      TEXT_STRING_NUMBER
      };

//---------------------------------------------------------
//   TextBase
//---------------------------------------------------------

class TextBase {
      int _refCount;
      QTextDocument* _doc;
      double _frameWidth;                 // unit: mm
      double _paddingWidth;               // unit: mm
      QColor _frameColor;
      int _frameRound;
      bool _circle;
      bool _hasFrame;
      double _layoutWidth;

      QRectF frame;
      QRectF _bbox;

   public:
      TextBase();
      TextBase(const TextBase&);

      int refCount() const    { return _refCount; }
      void incRefCount()      { ++_refCount; }
      void decRefCount()      { --_refCount; }

      void setDoc(const QTextDocument& d);
      QTextDocument* doc() const            { return _doc;          }
      bool hasFrame() const                 { return _hasFrame;     }
      void setHasFrame(bool val)            { _hasFrame = val;      }
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
      void clear()                          { _doc->clear();              }
      void layout(double w);
      QRectF bbox() const                   { return _bbox; }
      void draw(QPainter&, QTextCursor*) const;
      void setText(const QString&, Align);
      QString getText() const;
      QString getHtml() const;
      void setHtml(const QString& s);
      bool isEmpty() const                  { return _doc->isEmpty(); }
      void setModified(bool v)              { _doc->setModified(v);   }
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
      void clear()                          { textBase()->clear();          }
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

      virtual bool startEdit(Viewer*, const QPointF&);
      virtual bool edit(Viewer*, int grip, int key, Qt::KeyboardModifiers, const QString&);
      virtual void endEdit();
      void addSymbol(const SymCode&, QTextCursor* c = 0);
      void setCharFormat(const QTextCharFormat&);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void write(Xml& xml) const;
      virtual void write(Xml& xml, const char*) const;
      virtual void read(QDomElement);
      void writeProperties(Xml& xml) const;
      bool readProperties(QDomElement node);
      virtual void layout(ScoreLayout*);
      virtual QPainterPath shape() const;
      virtual bool mousePress(const QPointF&, QMouseEvent* ev);
      double lineSpacing() const;
      double lineHeight() const;
      void moveCursorToEnd();
      void moveCursor(int val);

      virtual QLineF dragAnchor() const;

      QFont defaultFont() const    { return textBase()->defaultFont(); }
      void setDefaultFont(QFont f) { textBase()->setDefaultFont(f);    }

      void setAbove(bool val);
      virtual qreal baseLine() const;
      virtual void paste();

      bool replaceSpecialChars();
      void styleChanged();
      QTextCursor* getCursor() const { return cursor; }
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
      virtual Text* clone() const        { return new Text(*this); }
      virtual TextBase* textBase() const { return _tb; }

      virtual bool genPropertyMenu(QMenu* popup) const;
      virtual void propertyAction(const QString& s);

      void setModified(bool v)           { _tb->setModified(v); }
      };

//---------------------------------------------------------
//   class Text
//    shared text
//---------------------------------------------------------

class TextC : public TextB {
      TextBase*  _tb;

   public:
      TextC(Score*);
      TextC(const TextC&);
      ~TextC();
      virtual TextC* clone() const;
      virtual TextBase* textBase() const { return _tb; }
      virtual void setStyle(const TextStyle*);
      void baseChanged();
      };

//---------------------------------------------------------
//   TextProperties
//---------------------------------------------------------

class TextProperties : public QDialog, public Ui::TextProperties {
      Q_OBJECT
      TextB* tb;

   private slots:
      virtual void accept();
      void mmToggled(bool);

   public:
      TextProperties(TextB*, QWidget* parent = 0);
      };

#endif
