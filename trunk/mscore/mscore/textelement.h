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

class TextPalette;
extern TextPalette* palette;

//---------------------------------------------------------
//   TextElement
//---------------------------------------------------------

class TextElement : public Element {
      QTextCursor* cursor;
      bool editMode;

   protected:
      QTextDocument* doc;
      int textStyle;
      QFont font() const;

   public:
      TextElement(Score*);
      TextElement(Score*, int style);
      TextElement(const TextElement&);

      virtual ~TextElement();
      TextElement &operator=(const TextElement&);

      virtual Element* clone() const { return new TextElement(*this); }
      virtual ElementType type() const { return TEXT; }


      void setText(const QString& s);
      QString getText() const;

      double lineSpacing() const;
      virtual void resetMode();

      bool isEmpty() const;
      void setStyle(int n);
      int style() const           { return textStyle; }

      virtual void draw1(Painter&);

      virtual bool startEdit(QMatrix&);
      virtual bool edit(QKeyEvent*);
      void addSymbol(int);
      virtual void endEdit();
      virtual bool isMovable() const { return true; }
      virtual void write(Xml& xml) const;
      void write(Xml& xml, const char*) const;
      virtual void read(QDomNode);
      virtual void layout();
      virtual const QRectF& bbox() const;
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
      virtual Element* clone() const { return new Lyrics(*this); }
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
      virtual Element* clone() const { return new Fingering(*this); }
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
      virtual Element* clone() const { return new InstrumentName1(*this); }
      virtual ElementType type() const { return INSTRUMENT_NAME1; }
      };

//---------------------------------------------------------
//   InstrumentName2
//    short name, repeated on ervery staff except first
//---------------------------------------------------------

class InstrumentName2 : public TextElement {
   public:
      InstrumentName2(Score*);
      virtual Element* clone() const { return new InstrumentName2(*this); }
      virtual ElementType type() const { return INSTRUMENT_NAME2; }
      };

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public TextElement {
   public:
      TempoText(Score*);
      virtual Element* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      };

#endif
