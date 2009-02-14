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

      textText = engine->toStringHandle(QLatin1String("text"));
      textSize = engine->toStringHandle(QLatin1String("size"));
      textDefaultFont = engine->toStringHandle(QLatin1String("defaultFont"));

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
      if (name == textText || name == textSize || name == textDefaultFont)
            return flags;
      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScText::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
// printf("ScText::property <%s>\n", qPrintable(name.toString()));
      TextPtr* text = qscriptvalue_cast<TextPtr*>(object.data());
      if (!text)
            return QScriptValue();
      if (name == textText)
            return QScriptValue(engine(), (*text)->getText());
      else if (name == textSize)
            return QScriptValue(engine(), (*text)->defaultFont().pixelSize());
//      else if (name == textDefaultFont)
//            return QScriptValue(engine(), (*text)->defaultFont());
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScText::setProperty(QScriptValue &object,
   const QScriptString& name, uint /*id*/, const QScriptValue& value)
      {
printf("ScText::setProperty <%s>\n", qPrintable(name.toString()));
      TextPtr* text = qscriptvalue_cast<TextPtr*>(object.data());
      if (!text)
            return;
      if (name == textText)
            (*text)->setText(value.toString());
      else if (name == textSize) {
            QFont f = (*text)->defaultFont();
            double ps = value.toInteger();
            ps = (ps * DPI)/PPI;
            f.setPixelSize(lrint(ps));
            (*text)->setDefaultFont(f);
            }
      else if (name == textDefaultFont) {
            QFont qf = qscriptvalue_cast<QFont>(value.data());
printf("setFont %d %d\n", qf.pixelSize(), qf.pointSize());
            (*text)->setDefaultFont(qf);
            }
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScText::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& name, uint /*id*/)
      {
      if (name == textText || name == textSize || name == textDefaultFont)
            return QScriptValue::Undeletable;
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
      ba = *(qscriptvalue_cast<TextPtr*>(obj.data()));
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
