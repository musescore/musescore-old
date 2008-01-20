//=============================================================================
//  MusE Score
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
      TEXT_TEMPO, TEXT_LYRIC, TEXT_TUPLET, TEXT_SYSTEM,
      TEXT_STAFF, TEXT_CHORD, TEXT_REHEARSAL_MARK,
      TEXT_REPEAT, TEXT_VOLTA, TEXT_FRAME, TEXT_TEXTLINE
      };

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public Element {
      bool _sizeIsSpatiumDependent;       // font size depends on _spatium unit
      double _frameWidth;                 // unit: mm
      double _marginWidth;                // unit: mm
      double _paddingWidth;               // unit: mm
      QColor _frameColor;
      int _frameRound;

      bool editMode;
      QTextCursor* cursor;
      bool setCursor(const QPointF& p);

   protected:
      QTextDocument* doc;
      int cursorPos;
      TextStyle* style() const;
      qreal basePosition() const;

   public:
      Text(Score*);
      Text(const Text&);

      virtual ~Text();
      Text &operator=(const Text&);

      virtual Text* clone() const      { return new Text(*this); }
      virtual ElementType type() const { return TEXT; }

      virtual const QString subtypeName() const;
      virtual void setSubtype(int val);
      virtual void setSubtype(const QString& s);

      void setText(const QString& s);
      QString getText() const;
      QString getHtml() const;
      void setHtml(const QString&);
      void setDoc(const QTextDocument&);
      QTextDocument* getDoc() const { return doc; }

      virtual void resetMode();
      bool isEmpty() const;
      void setStyle(const TextStyle*);

      double frameWidth() const             { return _frameWidth;   }
      double marginWidth() const            { return _marginWidth;  }
      double paddingWidth() const           { return _paddingWidth; }
      QColor frameColor() const             { return _frameColor;   }
      int frameRound() const                { return _frameRound;   }

      void setFrameWidth(double val)        { _frameWidth   = val;  }
      void setMarginWidth(double val)       { _marginWidth  = val;  }
      void setPaddingWidth(double val)      { _paddingWidth = val;  }
      void setFrameColor(const QColor& val) { _frameColor   = val;  }
      void setFrameRound(int val)           { _frameRound   = val;  }

      virtual void draw(QPainter&) const;

      virtual bool startEdit(const QPointF&);
      virtual bool edit(int, QKeyEvent*);
      void addSymbol(const SymCode&);
      void setCharFormat(const QTextCharFormat&);
      void setBlockFormat(const QTextBlockFormat&);
      virtual void endEdit();
      virtual bool isMovable() const { return true; }
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

      QFont defaultFont() const { return doc->defaultFont(); }
      void setDefaultFont(QFont f) { doc->setDefaultFont(f); }

      void setAbove(bool val);
      };

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public Text  {
      double _tempo;     // bpm

   public:
      TempoText(Score*);
      virtual TempoText* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      double tempo() const    { return _tempo; }
      void setTempo(double v) { _tempo = v; }
      virtual void write(Xml& xml) const;
      virtual void read(QDomElement);
      };

#endif
