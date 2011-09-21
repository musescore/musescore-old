//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2004-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#ifndef __ACCIDENTAL_H__
#define __ACCIDENTAL_H__

/**
 \file
 Definition of class Accidental
*/

#include "element.h"
#include "mscore.h"

class Note;
class Painter;

//---------------------------------------------------------
//   SymElement
//---------------------------------------------------------

struct SymElement {
      int sym;
      qreal x;
      SymElement(int _sym, qreal _x) : sym(_sym), x(_x) {}
      };

//---------------------------------------------------------
//   AccidentalRole
//---------------------------------------------------------

enum AccidentalRole {
      ACC_AUTO,               // layout created accidental
      ACC_USER                // user created accidental
      };

//---------------------------------------------------------
//   Accidental
//---------------------------------------------------------

class Accidental : public Element {
      QList<SymElement> el;
      bool _hasBracket;
      AccidentalRole _role;
      bool _small;

   public:
      Accidental(Score* s);
      virtual Accidental* clone() const     { return new Accidental(*this); }
      virtual ElementType type() const      { return ACCIDENTAL; }
      virtual const QString subtypeName() const;
      const char* subtypeUserName() const;
      virtual void setSubtype(const QString& s);
      void setSubtype(AccidentalType t)     { Element::setSubtype(int(t)); }
      AccidentalType accidentalType() const { return AccidentalType(subtype()); }
      virtual bool acceptDrop(MuseScoreView*, const QPointF&, int, int) const;
      virtual Element* drop(const DropData&);
      virtual void layout();
      virtual void draw(Painter*) const;
      virtual bool isEditable() const                    { return true; }
      virtual void startEdit(MuseScoreView*, const QPointF&) { setGenerated(false); }

      int symbol();
      Note* note() const                  { return (Note*)parent(); }
      bool hasBracket() const             { return _hasBracket;     }
      void setHasBracket(bool val)        { _hasBracket = val;      }
      AccidentalRole role() const         { return _role;           }
      void setRole(AccidentalRole r)      { _role = r;              }
      bool small() const                  { return _small;          }
      void setSmall(bool val)             { _small = val;           }

      virtual void read(QDomElement);
      virtual void write(Xml& xml) const;

      virtual QVariant getProperty(int propertyId) const;
      virtual void setProperty(int propertyId, const QVariant&);

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

