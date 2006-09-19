//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: textelement.h,v 1.4 2006/03/22 12:04:14 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#ifndef __TEXTELEMENT_H__
#define __TEXTELEMENT_H__

#include "element.h"
#include "text.h"

class TextPalette;
extern TextPalette* palette;

//---------------------------------------------------------
//   TextElement
//---------------------------------------------------------

class TextElement : public Element {
      Text text;         // two dimensional array of HBox text elements
      bool editMode;

      int cursorLine;
      int cursorColumn;

      QRectF cursor;

      void init();
      void setCursor();
      int columns() const;
      void insertChar(const QString&);
      void removeChar();
      void splitLine();
      void concatLine();

   protected:
      int textStyle;
      QFont font() const;

   public:
      TextElement(Score*);
      TextElement(Score*, int style);
      virtual ~TextElement();
      virtual ElementType type() const { return TEXT; }


      void setText(const QString& s);
      void setText(const Text& v);

      double lineSpacing() const;
      virtual void resetMode()    { editMode = 0; bboxUpdate(); }

      QString getText() const;
      bool isEmpty() const;
      void setStyle(int n);
      int style() const           { return textStyle; }

      virtual void draw1(Painter&) const;

      virtual bool startEdit(QMatrix&);
      virtual bool edit(QKeyEvent*);
      void addSymbol(int);
      virtual void endEdit();
      virtual bool startDrag(const QPointF&) { return true; }
      virtual void write(Xml& xml) const;
      void write(Xml& xml, const char*) const;
      virtual void read(QDomNode);
      virtual void layout();
      virtual void bboxUpdate();
      virtual void setSelected(bool f);
      };

//---------------------------------------------------------
//   Lyrics
//---------------------------------------------------------

class Lyrics : public TextElement {
   public:
      enum Syllabic { SINGLE, BEGIN, END, MIDDLE };
   private:
      int _no;
      Syllabic _syllabic;

   public:
      Lyrics(Score*);
      Lyrics(const Lyrics& l);
      virtual ElementType type() const { return LYRICS; }

      virtual void write(Xml& xml) const;
      virtual void read(QDomNode);
      void setNo(int n)            { _no = n; }
      int no() const               { return _no; }
      void setSyllabic(Syllabic s) { _syllabic = s; }
      Syllabic syllabic() const    { return _syllabic; }
      };

//---------------------------------------------------------
//   Fingering
//---------------------------------------------------------

class Fingering : public TextElement {
   public:
      Fingering(Score*);
      Fingering(const Fingering& l) : TextElement(l) {}
      virtual ElementType type() const { return FINGERING; }

      virtual void write(Xml& xml) const;
      virtual void setSubtype(int);
      };

//---------------------------------------------------------
//   InstrumentName1
//    long name
//---------------------------------------------------------

class InstrumentName1 : public TextElement {
   public:
      InstrumentName1(Score*);
      virtual ElementType type() const { return INSTRUMENT_NAME1; }
      };

//---------------------------------------------------------
//   InstrumentName2
//    short name, repeated on ervery staff except first
//---------------------------------------------------------

class InstrumentName2 : public TextElement {
   public:
      InstrumentName2(Score*);
      virtual ElementType type() const { return INSTRUMENT_NAME2; }
      };

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public TextElement {
   public:
      TempoText(Score*);
      virtual ElementType type() const { return TEMPO_TEXT; }
      };

#endif
