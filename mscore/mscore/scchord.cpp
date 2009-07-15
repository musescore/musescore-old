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

#include "mscore.h"
#include "scchord.h"
#include "chord.h"
#include "scscore.h"
#include "scnote.h"
#include "note.h"

//---------------------------------------------------------
//   ScChordPropertyIterator
//---------------------------------------------------------

class ScChordPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScChordPropertyIterator(const QScriptValue &object);
      ~ScChordPropertyIterator() {}
      bool hasNext() const;
      void next();
      bool hasPrevious() const;
      void previous();
      void toFront();
      void toBack();
      QScriptString name() const { return QScriptString(); }
      uint id() const            { return m_last; }
      };

//---------------------------------------------------------
//   ScChord
//---------------------------------------------------------

ScChord::ScChord(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<ChordRestPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScChordPrototype(this),
         QScriptEngine::QtOwnership,
         QScriptEngine::SkipMethodsInEnumeration);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

//---------------------------------------------------------
//   queryProperty
//---------------------------------------------------------

QScriptClass::QueryFlags ScChord::queryProperty(const QScriptValue &object,
   const QScriptString& /*name*/, QueryFlags /*flags*/, uint* /*id*/)
      {
      Chord** sp = (Chord**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!sp)
            return 0;

      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScChord::property(const QScriptValue& object,
   const QScriptString& /*name*/, uint /*id*/)
      {
      Chord** score = (Chord**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!score)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScChord::setProperty(QScriptValue &object,
   const QScriptString& /*s*/, uint /*id*/, const QScriptValue& /*value*/)
      {
      Chord** score = (Chord**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!score)
            return;
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScChord::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ScChord::newIterator(const QScriptValue &object)
      {
      return new ScChordPropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScChord::newInstance(Score* score)
      {
      Chord* chord = new Chord(score);
      return newInstance(chord);
      }

QScriptValue ScChord::newInstance(const ChordRestPtr& score)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(score));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScChord::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScChord *cls = qscriptvalue_cast<ScChord*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScChord::toScriptValue(QScriptEngine* eng, const ChordRestPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Chord");
      ScChord* cls = qscriptvalue_cast<ScChord*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScChord::fromScriptValue(const QScriptValue& obj, ChordRestPtr& ba)
      {
      Chord** cp = (Chord**)qscriptvalue_cast<ChordRestPtr*>(obj.data());
      ba = cp ? *cp : 0;
      }

//---------------------------------------------------------
//   ScChordPropertyIterator
//---------------------------------------------------------

ScChordPropertyIterator::ScChordPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScChordPropertyIterator::hasNext() const
      {
//      Chord* ba = qscriptvalue_cast<Chord*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScChordPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScChordPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScChordPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScChordPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScChordPropertyIterator::toBack()
      {
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisChord
//---------------------------------------------------------

Chord* ScChordPrototype::thisChord() const
      {
      Chord** cp = (Chord**)qscriptvalue_cast<ChordRestPtr*>(thisObject().data());
      if (cp)
            return *cp;
      return 0;
      }

//---------------------------------------------------------
//   topNote
//---------------------------------------------------------

NotePtr ScChordPrototype::topNote() const
      {
      Chord* chord = thisChord();
      return chord->upNote();
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void ScChordPrototype::addNote(NotePtr note)
      {
      note->setParent(thisChord());
      Score* score = thisChord()->score();
      if (score) {
            note->setScore(score);
            thisChord()->score()->undoAddElement(note);
            }
      else
            thisChord()->add(note);
      }

//---------------------------------------------------------
//   removeNote
//---------------------------------------------------------

void ScChordPrototype::removeNote(int idx)
      {
      Chord* chord = thisChord();
      NoteList* nl = chord->noteList();
      if (idx < 0 || idx >= int(nl->size()))
            return;
      Score* score = chord->score();
      if (score) {
            NotePtr n = note(idx);
            score->undoRemoveElement(n);
            }
      else {
            int k = 0;
            for (iNote i = nl->begin(); i != nl->end(); ++i) {
                  if (k == idx) {
                        nl->erase(i);
                        break;
                        }
                  ++k;
                  }
            }
      }

//---------------------------------------------------------
//   notes
//---------------------------------------------------------

int ScChordPrototype::notes() const
      {
      Chord* chord = thisChord();
      const NoteList* nl = chord->noteList();
      return nl->size();
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

NotePtr ScChordPrototype::note(int idx) const
      {
      Chord* chord = thisChord();
      const NoteList* nl = chord->noteList();
      if (idx < 0 || idx >= int(nl->size()))
            return 0;
      int k = 0;
      for (ciNote i = nl->begin(); i != nl->end(); ++i) {
            if (k == idx)
                  return i->second;
            ++k;
            }
      return 0;
      }


