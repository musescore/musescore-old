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
#include "scnote.h"
#include "note.h"
#include "utils.h"
#include "scscore.h"
#include "undo.h"

//---------------------------------------------------------
//   ScNotePropertyIterator
//---------------------------------------------------------

class ScNotePropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScNotePropertyIterator(const QScriptValue &object);
      ~ScNotePropertyIterator() {}
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
//   ScNote
//---------------------------------------------------------

ScNote::ScNote(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<NotePtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScNotePrototype(this),
         QScriptEngine::QtOwnership,
         QScriptEngine::SkipMethodsInEnumeration
          | QScriptEngine::ExcludeSuperClassMethods
          | QScriptEngine::ExcludeSuperClassProperties);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

//---------------------------------------------------------
//   queryProperty
//---------------------------------------------------------

QScriptClass::QueryFlags ScNote::queryProperty(const QScriptValue &object,
   const QScriptString& /*name*/, QueryFlags /*flags*/, uint* /*id*/)
      {
      NotePtr* sp = qscriptvalue_cast<NotePtr*>(object.data());
      if (!sp)
            return 0;

      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScNote::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("property <%s>\n", qPrintable(name.toString()));
      NotePtr* score = qscriptvalue_cast<NotePtr*>(object.data());
      if (!score)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScNote::setProperty(QScriptValue& /*object*/ , const QScriptString& /*s*/, uint /*id*/, const QScriptValue& /*value*/)
      {
/*      NotePtr* score = qscriptvalue_cast<NotePtr*>(object.data());
      if (!score)
            return;
      */
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScNote::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
#if 0
      if (name == scoreName)
            return QScriptValue::Undeletable;
      else if (name == scoreStaves)
            return QScriptValue::Undeletable | QScriptValue::ReadOnly;
#endif
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ScNote::newIterator(const QScriptValue &object)
      {
      return new ScNotePropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScNote::newInstance(Score* score)
      {
      Note* note = new Note(score);
      return newInstance(note);
      }

QScriptValue ScNote::newInstance(const NotePtr& note)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(note));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScNote::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScNote *cls = qscriptvalue_cast<ScNote*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScNote::toScriptValue(QScriptEngine* eng, const NotePtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Note");
      ScNote* cls = qscriptvalue_cast<ScNote*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScNote::fromScriptValue(const QScriptValue& obj, NotePtr& ba)
      {
      NotePtr* np = qscriptvalue_cast<NotePtr*>(obj.data());
      ba = np ? *np : 0;
      }

//---------------------------------------------------------
//   ScNotePropertyIterator
//---------------------------------------------------------

ScNotePropertyIterator::ScNotePropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScNotePropertyIterator::hasNext() const
      {
//      Note* ba = qscriptvalue_cast<Note*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScNotePropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScNotePropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScNotePropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScNotePropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScNotePropertyIterator::toBack()
      {
//      NotePtr* ba = qscriptvalue_cast<NotePtr*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisNote
//---------------------------------------------------------

Note* ScNotePrototype::thisNote() const
      {
      NotePtr* np = qscriptvalue_cast<NotePtr*>(thisObject().data());
      if (np)
            return *np;
      return 0;
      }

//---------------------------------------------------------
//   noteName
//---------------------------------------------------------

QString ScNotePrototype::getName() const
      {
      return pitch2string(thisNote()->pitch());
      }

//---------------------------------------------------------
//   getPitch
//---------------------------------------------------------

int ScNotePrototype::getPitch() const
      {
      return thisNote()->pitch();
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void ScNotePrototype::setPitch(int v)
      {
      thisNote()->setPitch(v);
      }
      
//---------------------------------------------------------
//   getTuning
//---------------------------------------------------------

double ScNotePrototype::getTuning() const
      {
      return thisNote()->tuning();
      }

//---------------------------------------------------------
//   setTuning
//---------------------------------------------------------

void ScNotePrototype::setTuning(double v)
      {
      thisNote()->setTuning(v);
      }

//---------------------------------------------------------
//   getColor
//---------------------------------------------------------

QColor ScNotePrototype::getColor() const
      {
      return thisNote()->color();
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void ScNotePrototype::setColor(QColor c)
      {
      Note* note = thisNote();
      Score* score = note->score();
      if (score)
            score->undo()->push(new ChangeColor(note, c));
      else
            note->setColor(c);
      }




