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

enum {
      ACC_NONE, ACC_SHARP, ACC_FLAT, ACC_SHARP2, ACC_FLAT2, ACC_NATURAL
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

   public:
      Accidental(Score* s) : Element(s) {}
      virtual Accidental* clone() const { return new Accidental(*this); }
      virtual ElementType type() const  { return ACCIDENTAL; }
      virtual void setSubtype(int v);
      virtual const QString subtypeName() const;
      virtual void setSubtype(const QString& s);
      virtual bool acceptDrop(ScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(ScoreView*, const QPointF&, const QPointF&, Element*);
      virtual void layout();
      virtual void draw(QPainter&) const;
      int symbol();
      Note* note() const { return (Note*)parent(); }

      const char* subTypeName() const;
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

