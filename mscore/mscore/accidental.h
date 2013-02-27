//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2004-2009 Werner Schweer and others
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

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

/**
 \file
 Definition of class Accidental
*/

#include "element.h"

class Note;

// Accidental Subtype Values
enum AccidentalType {
      ACC_NONE,
      ACC_SHARP,
      ACC_FLAT,
      ACC_SHARP2,
      ACC_FLAT2,
      ACC_NATURAL,

      ACC_FLAT_SLASH,
      ACC_FLAT_SLASH2,
      ACC_MIRRORED_FLAT2,
      ACC_MIRRORED_FLAT,
      ACC_MIRRIRED_FLAT_SLASH,
      ACC_FLAT_FLAT_SLASH,

      ACC_SHARP_SLASH,
      ACC_SHARP_SLASH2,
      ACC_SHARP_SLASH3,
      ACC_SHARP_SLASH4,

      ACC_SHARP_ARROW_UP,
      ACC_SHARP_ARROW_DOWN,
      ACC_SHARP_ARROW_BOTH,
      ACC_FLAT_ARROW_UP,
      ACC_FLAT_ARROW_DOWN,
      ACC_FLAT_ARROW_BOTH,
      ACC_NATURAL_ARROW_UP,
      ACC_NATURAL_ARROW_DOWN,
      ACC_NATURAL_ARROW_BOTH,
      ACC_SORI,
      ACC_KORON,
      ACC_END
      };

struct SymElement {
      int sym;
      double x;
      SymElement(int _sym, double _x) : sym(_sym), x(_x) {}
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

class Accidental : public Element {
      QList<SymElement> el;
      bool _hasBracket;

   public:
      Accidental(Score* s) : Element(s) { _hasBracket = false; }
      virtual Accidental* clone() const { return new Accidental(*this); }
      virtual ElementType type() const  { return ACCIDENTAL; }
      virtual const QString subtypeName() const;
      const char* subtypeUserName() const;
      virtual void setSubtype(const QString& s);
      void setSubtype(int i)            { Element::setSubtype(i); }
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual void layout();
      virtual void draw(QPainter&) const;
      int symbol();
      Note* note() const { return (Note*)parent(); }
      bool hasBracket() const      { return _hasBracket; }
      void setHasBracket(bool val) { _hasBracket = val;  }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      static int subtype2value(int);      // return effective pitch offset
      static int value2subtype(int);
      };

//---------------------------------------------------------
//   AccidentalBracket
//    used as icon in palette
//---------------------------------------------------------

class AccidentalBracket : public Compound {
   public:
      AccidentalBracket(Score*);
      virtual AccidentalBracket* clone() const { return new AccidentalBracket(*this); }
      virtual ElementType type() const         { return ACCIDENTAL_BRACKET; }
      virtual void setSubtype(int v);
      };

#endif

