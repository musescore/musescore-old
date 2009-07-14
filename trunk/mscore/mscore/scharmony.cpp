//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scnote.cpp 1840 2009-05-20 11:57:51Z wschweer $
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
#include "scharmony.h"
#include "harmony.h"
#include "utils.h"
#include "scscore.h"
#include "undo.h"

//---------------------------------------------------------
//   ScHarmonyPropertyIterator
//---------------------------------------------------------

class ScHarmonyPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScHarmonyPropertyIterator(const QScriptValue &object);
      ~ScHarmonyPropertyIterator() {}
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
//   ScHarmony
//---------------------------------------------------------

ScHarmony::ScHarmony(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<HarmonyPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScHarmonyPrototype(this),
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

QScriptClass::QueryFlags ScHarmony::queryProperty(const QScriptValue &object,
   const QScriptString& /*name*/, QueryFlags /*flags*/, uint* /*id*/)
      {
      HarmonyPtr* sp = qscriptvalue_cast<HarmonyPtr*>(object.data());
      if (!sp)
            return 0;

      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScHarmony::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("property <%s>\n", qPrintable(name.toString()));
      HarmonyPtr* score = qscriptvalue_cast<HarmonyPtr*>(object.data());
      if (!score)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScHarmony::setProperty(QScriptValue& /*object*/ , const QScriptString& /*s*/, uint /*id*/, const QScriptValue& /*value*/)
      {
/*      HarmonyPtr* score = qscriptvalue_cast<HarmonyPtr*>(object.data());
      if (!score)
            return;
      */
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScHarmony::propertyFlags(
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

QScriptClassPropertyIterator *ScHarmony::newIterator(const QScriptValue &object)
      {
      return new ScHarmonyPropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScHarmony::newInstance(Score* score)
      {
      Harmony* note = new Harmony(score);
      return newInstance(note);
      }

QScriptValue ScHarmony::newInstance(const HarmonyPtr& note)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(note));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScHarmony::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScHarmony *cls = qscriptvalue_cast<ScHarmony*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScHarmony::toScriptValue(QScriptEngine* eng, const HarmonyPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Harmony");
      ScHarmony* cls = qscriptvalue_cast<ScHarmony*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScHarmony::fromScriptValue(const QScriptValue& obj, HarmonyPtr& ba)
      {
      HarmonyPtr* np = qscriptvalue_cast<HarmonyPtr*>(obj.data());
      ba = np ? *np : 0;
      }

//---------------------------------------------------------
//   ScHarmonyPropertyIterator
//---------------------------------------------------------

ScHarmonyPropertyIterator::ScHarmonyPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScHarmonyPropertyIterator::hasNext() const
      {
//      Harmony* ba = qscriptvalue_cast<Harmony*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScHarmonyPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScHarmonyPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScHarmonyPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScHarmonyPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScHarmonyPropertyIterator::toBack()
      {
//      HarmonyPtr* ba = qscriptvalue_cast<HarmonyPtr*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisHarmony
//---------------------------------------------------------

Harmony* ScHarmonyPrototype::thisHarmony() const
      {
      HarmonyPtr* np = qscriptvalue_cast<HarmonyPtr*>(thisObject().data());
      if (np)
            return *np;
      return 0;
      }

//---------------------------------------------------------
//   getId
//---------------------------------------------------------

int ScHarmonyPrototype::getId() const
      {
      return thisHarmony()->id();
      }

//---------------------------------------------------------
//   getBase
//---------------------------------------------------------

int ScHarmonyPrototype::getBase() const
      {
      return thisHarmony()->baseTpc();
      }

//---------------------------------------------------------
//   getRoot
//---------------------------------------------------------

int ScHarmonyPrototype::getRoot() const
      {
      return thisHarmony()->rootTpc();
      }

//---------------------------------------------------------
//   setId
//---------------------------------------------------------

void ScHarmonyPrototype::setId(int v)
      {
      thisHarmony()->setId(v);
      }

//---------------------------------------------------------
//   setRoot
//---------------------------------------------------------

void ScHarmonyPrototype::setRoot(int v)
      {
      thisHarmony()->setRootTpc(v);
      }

//---------------------------------------------------------
//   setBase
//---------------------------------------------------------

void ScHarmonyPrototype::setBase(int v)
      {
      thisHarmony()->setRootTpc(v);
      }

