//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __TEXT_H__
#define __TEXT_H__

#include "element.h"

class TextPalette;
extern TextPalette* palette;

//---------------------------------------------------------
//   Text
//---------------------------------------------------------

class Text : public Element {
      QTextCursor* cursor;
      bool editMode;

   protected:
      QTextDocument* doc;
      int textStyle;
      QFont font() const;

   public:
      Text(Score*);
      Text(Score*, int style);
      Text(const Text&);

      virtual ~Text();
      Text &operator=(const Text&);

      virtual Element* clone() const { return new Text(*this); }
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

class Lyrics : public Text {
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

class Fingering : public Text {
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

class InstrumentName1 : public Text {
   public:
      InstrumentName1(Score*);
      virtual Element* clone() const { return new InstrumentName1(*this); }
      virtual ElementType type() const { return INSTRUMENT_NAME1; }
      };

//---------------------------------------------------------
//   InstrumentName2
//    short name, repeated on ervery staff except first
//---------------------------------------------------------

class InstrumentName2 : public Text {
   public:
      InstrumentName2(Score*);
      virtual Element* clone() const { return new InstrumentName2(*this); }
      virtual ElementType type() const { return INSTRUMENT_NAME2; }
      };

//---------------------------------------------------------
//   TempoText
//---------------------------------------------------------

class TempoText : public Text  {
   public:
      TempoText(Score*);
      virtual Element* clone() const { return new TempoText(*this); }
      virtual ElementType type() const { return TEMPO_TEXT; }
      };

#endif
