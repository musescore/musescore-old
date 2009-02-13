//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
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

#ifndef __SCNOTE_H__
#define __SCNOTE_H__

class Note;
class Score;
typedef Note* NotePtr;

//---------------------------------------------------------
//   ScNote
//---------------------------------------------------------

class ScNote : public QObject, public QScriptClass {
      static QScriptValue construct(QScriptContext* ctx, QScriptEngine* eng);
      static QScriptValue toScriptValue(QScriptEngine *eng, const NotePtr& ba);
      static void fromScriptValue(const QScriptValue &obj, NotePtr& ba);

      QScriptValue proto;
      QScriptValue ctor;

   public:
      ScNote(QScriptEngine* se);
      ~ScNote() {}

      QScriptValue constructor() { return ctor; }
      QScriptValue newInstance(Score*);
      QScriptValue newInstance(const NotePtr&);
      QueryFlags queryProperty(const QScriptValue& object,
         const QScriptString& name, QueryFlags flags, uint* id);
      QScriptValue property(const QScriptValue& obhect,
         const QScriptString& name, uint id);
      virtual void setProperty(QScriptValue& object, const QScriptString& name,
         uint id, const QScriptValue& value);
      QScriptValue::PropertyFlags propertyFlags(
         const QScriptValue& object, const QScriptString& name, uint id);
      QScriptClassPropertyIterator* newIterator(const QScriptValue& object);
      QString name() const           { return QLatin1String("Note"); }
      QScriptValue prototype() const { return proto; }
      };

class ScNotePrototype : public QObject, public QScriptable
      {
      Q_OBJECT
      Note* thisNote() const;

   public:
      ScNotePrototype(QObject *parent = 0) : QObject(parent) {}
      ~ScNotePrototype() {}

   public slots:
      QString name() const;
      };

Q_DECLARE_METATYPE(NotePtr)
Q_DECLARE_METATYPE(NotePtr*)
Q_DECLARE_METATYPE(ScNote*)

#endif


