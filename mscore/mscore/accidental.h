//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2004-2010 Werner Schweer and others
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
#include "globals.h"

class Note;

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

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
      Accidental(Score* s);
      virtual Accidental* clone() const     { return new Accidental(*this); }
      virtual ElementType type() const      { return ACCIDENTAL; }
      virtual const QString subtypeName() const;
      const char* subtypeUserName() const;
      virtual void setSubtype(const QString& s);
      void setSubtype(AccidentalType t)     { Element::setSubtype(int(t)); }
      AccidentalType accidentalType() const { return AccidentalType(subtype()); }
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual void layout();
      virtual void draw(QPainter&, ScoreView*) const;
      virtual bool isEditable() const                    { return true; }
      virtual void startEdit(ScoreView*, const QPointF&) { setGenerated(false); }

      int symbol();
      Note* note() const                  { return (Note*)parent(); }
      bool hasBracket() const             { return _hasBracket;     }
      void setHasBracket(bool val)        { _hasBracket = val;      }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      static int subtype2value(AccidentalType);             // return effective pitch offset
      static const char* subtype2name(AccidentalType);      // return effective pitch offset
      static AccidentalType value2subtype(int);
      static AccidentalType name2subtype(const QString&);
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

