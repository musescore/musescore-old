//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __SCCHORD_H__
#define __SCCHORD_H__

#include "scchordrest.h"

class Chord;
class Note;
class Score;
typedef Note* NotePtr;
class Note;

//---------------------------------------------------------
//   ScChord
//---------------------------------------------------------

class ScChord : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const ChordRestPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, ChordRestPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScChord(QScriptEngine* se);
      ~ScChord() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const ChordRestPtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Chord"); }
      QScriptValue prototype() const { return proto; }
      };

#if 0
//---------------------------------------------------------
//   ScChordPrototype
//---------------------------------------------------------

class ScChordPrototype : public ScChordRestPrototype
      {
      Q_OBJECT
      Chord* thisChord() const;

   public slots:
      NotePtr topNote() const;
      void addNote(NotePtr note);
      void removeNote(int);
      int notes() const;
      NotePtr note(int) const;

   public:
      ScChordPrototype(QObject *parent = 0) : ScChordRestPrototype(parent) {}
      ~ScChordPrototype() {}
      };
#endif

Q_DECLARE_METATYPE(ScChord*)

#endif


