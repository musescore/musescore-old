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

#include "sccursormeasure.h"
#include "scscore.h"
#include "measure.h"

//---------------------------------------------------------
//   SCursorMeasure
//---------------------------------------------------------

SCursorMeasure::SCursorMeasure(Score* s)
      {
      _score    = s;
      _measure  = 0;
      }


//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void SCursorMeasure::rewind()
      {
      _measure   =  _score->firstMeasure();
      }

//---------------------------------------------------------
//   ScSCursorMeasurePropertyIterator
//---------------------------------------------------------

class ScSCursorMeasurePropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScSCursorMeasurePropertyIterator(const QScriptValue &object);
      ~ScSCursorMeasurePropertyIterator() {}
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
//   ScSCursorMeasure
//---------------------------------------------------------

ScSCursorMeasure::ScSCursorMeasure(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<SCursorMeasure>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScSCursorMeasurePrototype(this),
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

QScriptClass::QueryFlags ScSCursorMeasure::queryProperty(const QScriptValue &object,
   const QScriptString& name, QueryFlags flags, uint* /*id*/)
      {
      SCursorMeasure* sp = qscriptvalue_cast<SCursorMeasure*>(object.data());
      
      if (!sp)
            return 0;
      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScSCursorMeasure::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("ScCursorMeasure::property <%s>\n", qPrintable(name.toString()));
      SCursorMeasure* cursor = qscriptvalue_cast<SCursorMeasure*>(object.data());
      if (!cursor)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScSCursorMeasure::setProperty(QScriptValue &object,
   const QScriptString& name, uint /*id*/, const QScriptValue& value)
      {
printf("ScSCursorMeasure::setProperty <%s>\n", qPrintable(name.toString()));
      SCursorMeasure* cursor = qscriptvalue_cast<SCursorMeasure*>(object.data());
      if (!cursor)
            return;
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScSCursorMeasure::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator* ScSCursorMeasure::newIterator(const QScriptValue &object)
      {
      return new ScSCursorMeasurePropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScSCursorMeasure::newInstance(Score* score)
      {
      SCursorMeasure* cursor = new SCursorMeasure(score);
      return newInstance(*cursor);
      }

QScriptValue ScSCursorMeasure::newInstance(const SCursorMeasure& cursor)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(cursor));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScSCursorMeasure::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScSCursorMeasure *cls = qscriptvalue_cast<ScSCursorMeasure*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(v.data());
      if (sp)
            return cls->newInstance(*sp);
      else
            return QScriptValue();
      }

QScriptValue ScSCursorMeasure::toScriptValue(QScriptEngine* eng, const SCursorMeasure& ba)
      {
      QScriptValue ctor = eng->globalObject().property("CursorMeasure");
      ScSCursorMeasure* cls = qscriptvalue_cast<ScSCursorMeasure*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScSCursorMeasure::fromScriptValue(const QScriptValue& obj, SCursorMeasure& ba)
      {
      ba = qscriptvalue_cast<SCursorMeasure>(obj.data());
      }

//---------------------------------------------------------
//   ScSCursorMeasurePropertyIterator
//---------------------------------------------------------

ScSCursorMeasurePropertyIterator::ScSCursorMeasurePropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScSCursorMeasurePropertyIterator::hasNext() const
      {
//      SCursor* ba = qscriptvalue_cast<SCursor*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScSCursorMeasurePropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScSCursorMeasurePropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScSCursorMeasurePropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScSCursorMeasurePropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScSCursorMeasurePropertyIterator::toBack()
      {
//      SCursor* ba = qscriptvalue_cast<SCursor*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisSCursorMeasure
//---------------------------------------------------------

SCursorMeasure* ScSCursorMeasurePrototype::thisSCursorMeasure() const
      {
      return qscriptvalue_cast<SCursorMeasure*>(thisObject().data());
      }

//---------------------------------------------------------
//   rewind
//    rewind cursor to start of score
//---------------------------------------------------------

void ScSCursorMeasurePrototype::rewind()
      {
      SCursorMeasure* cursor = thisSCursorMeasure();
      cursor->rewind();
      }

//---------------------------------------------------------
//   eos
//    return true if cursor is at end of score
//---------------------------------------------------------

bool ScSCursorMeasurePrototype::eos() const
      {
      SCursorMeasure* cursor = thisSCursorMeasure();
      return cursor->measure() == 0;
      }

//---------------------------------------------------------
//   measure
//    get measure at current position
//---------------------------------------------------------

MeasurePtr ScSCursorMeasurePrototype::measure()
      {
      SCursorMeasure* cursor = thisSCursorMeasure();
      Measure* m = cursor->measure();
      if (m == 0)
            return 0;
      return m;
      }

//---------------------------------------------------------
//   next
//    go to next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool ScSCursorMeasurePrototype::next()
      {
      SCursorMeasure* cursor = thisSCursorMeasure();
      Measure* cm = cursor->measure();;
      
      MeasureBase* nmb = cm->next();
      while (nmb && nmb->type() != MEASURE)
                nmb = nmb->next();
      
      cursor->setMeasure(static_cast<Measure*>(nmb));
      return nmb != 0;
      }
