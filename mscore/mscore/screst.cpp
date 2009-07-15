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

#include "mscore.h"
#include "screst.h"
#include "rest.h"
#include "harmony.h"
#include "scscore.h"
#include "measure.h"

//---------------------------------------------------------
//   ScRestPropertyIterator
//---------------------------------------------------------

class ScRestPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScRestPropertyIterator(const QScriptValue &object);
      ~ScRestPropertyIterator() {}
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
//   ScRest
//---------------------------------------------------------

ScRest::ScRest(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<ChordRestPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScRestPrototype(this),
         QScriptEngine::QtOwnership, QScriptEngine::SkipMethodsInEnumeration);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

//---------------------------------------------------------
//   queryProperty
//---------------------------------------------------------

QScriptClass::QueryFlags ScRest::queryProperty(const QScriptValue &object,
   const QScriptString& /*name*/, QueryFlags /*flags*/, uint* /*id*/)
      {
      Rest** sp = (Rest**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!sp)
            return 0;
      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScRest::property(const QScriptValue& object,
   const QScriptString& /*name*/, uint /*id*/)
      {
      Rest** rest = (Rest**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!rest)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScRest::setProperty(QScriptValue &object,
   const QScriptString& /*s*/, uint /*id*/, const QScriptValue& /*value*/)
      {
      Rest** rest = (Rest**)qscriptvalue_cast<ChordRestPtr*>(object.data());
      if (!rest)
            return;
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScRest::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator* ScRest::newIterator(const QScriptValue &object)
      {
      return new ScRestPropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScRest::newInstance(Score* score)
      {
// printf("ScRest::newInstance\n");
      Rest* rest = new Rest(score);
      return newInstance(rest);
      }

QScriptValue ScRest::newInstance(const ChordRestPtr& cr)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(cr));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScRest::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScRest *cls = qscriptvalue_cast<ScRest*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScRest::toScriptValue(QScriptEngine* eng, const ChordRestPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Rest");
      ScRest* cls = qscriptvalue_cast<ScRest*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue((ChordRestPtr&)ba));
      return cls->newInstance(ba);
      }

void ScRest::fromScriptValue(const QScriptValue& obj, ChordRestPtr& ba)
      {
      Rest** cp = (Rest**)qscriptvalue_cast<ChordRestPtr*>(obj.data());
      ba = cp ? *cp : 0;
      }

//---------------------------------------------------------
//   ScRestPropertyIterator
//---------------------------------------------------------

ScRestPropertyIterator::ScRestPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScRestPropertyIterator::hasNext() const
      {
      return m_index < 1;     // TODO ba->size();
      }

void ScRestPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScRestPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScRestPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScRestPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScRestPropertyIterator::toBack()
      {
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisRest
//---------------------------------------------------------

Rest* ScRestPrototype::thisRest() const
      {
      Rest** cp = (Rest**)qscriptvalue_cast<ChordRestPtr*>(thisObject().data());
      if (cp)
            return *cp;
      return 0;
      }

ChordRest* ScChordRestPrototype::thisChordRest() const
      {
      QScriptValue sv(thisObject().data());
      ChordRestPtr* cp = qscriptvalue_cast<ChordRestPtr*>(sv);

      if (cp)
            return *cp;
      return 0;
      }

//---------------------------------------------------------
//   getTickLen
//---------------------------------------------------------

int ScChordRestPrototype::getTickLen() const
      {
      ChordRest* cr = thisChordRest();
      return cr->tickLen();
      }

//---------------------------------------------------------
//   setTickLen
//---------------------------------------------------------

void ScChordRestPrototype::setTickLen(int v)
      {
      ChordRest* cr = thisChordRest();
      cr->setLen(v);
      }

//---------------------------------------------------------
//   addHarmony
//---------------------------------------------------------

void ScChordRestPrototype::addHarmony(HarmonyPtr h)
      {
      ChordRest* cr = thisChordRest();

      h->setParent(cr->measure());
      h->setTick(cr->tick());
      Score* score = cr->score();
      if (score) {
            h->setScore(score);
            cr->score()->undoAddElement(h);
            }
      else
            cr->measure()->add(h);
      h->render();
      }

