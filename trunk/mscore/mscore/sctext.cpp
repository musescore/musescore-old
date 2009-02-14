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

#include "sctext.h"
#include "scscore.h"
#include "text.h"

//---------------------------------------------------------
//   ScTextPropertyIterator
//---------------------------------------------------------

class ScTextPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScTextPropertyIterator(const QScriptValue &object);
      ~ScTextPropertyIterator() {}
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
//   ScText
//---------------------------------------------------------

ScText::ScText(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<TextPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScTextPrototype(this),
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

QScriptClass::QueryFlags ScText::queryProperty(const QScriptValue &object,
   const QScriptString& name, QueryFlags flags, uint* /*id*/)
      {
      TextPtr* sp = qscriptvalue_cast<TextPtr*>(object.data());
      if (!sp)
            return 0;

      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScText::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("property <%s>\n", qPrintable(name.toString()));
      TextPtr* score = qscriptvalue_cast<TextPtr*>(object.data());
      if (!score)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScText::setProperty(QScriptValue &object,
   const QScriptString& s, uint /*id*/, const QScriptValue& value)
      {
      TextPtr* score = qscriptvalue_cast<TextPtr*>(object.data());
      if (!score)
            return;
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScText::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& name, uint /*id*/)
      {
#if 0
      if (name == scoreName)
            return QScriptValue::Undeletable;
      else if (name == scoreStaves)
            return QScriptValue::Undeletable | QScriptValue::ReadOnly;
#endif
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ScText::newIterator(const QScriptValue &object)
      {
      return new ScTextPropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScText::newInstance(Score* score)
      {
      Text* text = new Text(score);
      return newInstance(text);
      }

QScriptValue ScText::newInstance(const TextPtr& note)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(note));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScText::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScText *cls = qscriptvalue_cast<ScText*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      if (sp)
            return cls->newInstance(*sp);
      else
            return QScriptValue();
      }

QScriptValue ScText::toScriptValue(QScriptEngine* eng, const TextPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Text");
      ScText* cls = qscriptvalue_cast<ScText*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScText::fromScriptValue(const QScriptValue& obj, TextPtr& ba)
      {
      ba = qscriptvalue_cast<TextPtr>(obj.data());
      }

//---------------------------------------------------------
//   ScTextPropertyIterator
//---------------------------------------------------------

ScTextPropertyIterator::ScTextPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScTextPropertyIterator::hasNext() const
      {
//      Text* ba = qscriptvalue_cast<Text*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScTextPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScTextPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScTextPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScTextPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScTextPropertyIterator::toBack()
      {
//      TextPtr* ba = qscriptvalue_cast<TextPtr*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisText
//---------------------------------------------------------

Text* ScTextPrototype::thisText() const
      {
      TextPtr* np = qscriptvalue_cast<TextPtr*>(thisObject().data());
      if (np)
            return *np;
      return 0;
      }
