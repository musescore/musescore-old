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

class Chord;
class Note;
class Score;
typedef Chord* ChordPtr;
typedef Note* NotePtr;
class Note;

//---------------------------------------------------------
//   ScChord
//---------------------------------------------------------

class ScChord : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const ChordPtr& ba);
      static void fromScriptValue(const QScriptValue &obj, ChordPtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScChord(QScriptEngine* se);
      ~ScChord() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const ChordPtr&);
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

//---------------------------------------------------------
//   ScChordPrototype
//---------------------------------------------------------

class ScChordPrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Chord* thisChord() const;
      Q_PROPERTY(int tickLen READ getTickLen WRITE setTickLen SCRIPTABLE true)

   public slots:
      NotePtr topNote() const;
      void addNote(NotePtr note);
      void removeNote(int);
      int notes() const;
      NotePtr note(int) const;

   public:
      ScChordPrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScChordPrototype() {}

      int getTickLen() const;
      void setTickLen(int v);
      };

Q_DECLARE_METATYPE(ChordPtr)
Q_DECLARE_METATYPE(ChordPtr*)
Q_DECLARE_METATYPE(ScChord*)

#endif


