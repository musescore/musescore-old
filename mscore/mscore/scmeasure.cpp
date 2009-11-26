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
#include "scmeasure.h"
#include "measure.h"
#include "scscore.h"
#include "layoutbreak.h"
#include "page.h"

//---------------------------------------------------------
//   ScMeasurePropertyIterator
//---------------------------------------------------------

class ScMeasurePropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScMeasurePropertyIterator(const QScriptValue &object);
      ~ScMeasurePropertyIterator() {}
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
//   ScMeasure
//---------------------------------------------------------

ScMeasure::ScMeasure(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<MeasurePtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScMeasurePrototype(this),
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

QScriptClass::QueryFlags ScMeasure::queryProperty(const QScriptValue &object,
   const QScriptString& /*name*/, QueryFlags /*flags*/, uint* /*id*/)
      {
      MeasurePtr* sp = qscriptvalue_cast<MeasurePtr*>(object.data());
      if (!sp)
            return 0;

      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScMeasure::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("property <%s>\n", qPrintable(name.toString()));
      MeasurePtr* score = qscriptvalue_cast<MeasurePtr*>(object.data());
      if (!score)
            return QScriptValue();
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScMeasure::setProperty(QScriptValue& /*object*/ , const QScriptString& /*s*/, uint /*id*/, const QScriptValue& /*value*/)
      {
/*      MeasurePtr* score = qscriptvalue_cast<MeasurePtr*>(object.data());
      if (!score)
            return;
      */
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScMeasure::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ScMeasure::newIterator(const QScriptValue &object)
      {
      return new ScMeasurePropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScMeasure::newInstance(Score* score)
      {
      Measure* measure = new Measure(score);
      return newInstance(measure);
      }

QScriptValue ScMeasure::newInstance(const MeasurePtr& measure)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(measure));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScMeasure::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScMeasure *cls = qscriptvalue_cast<ScMeasure*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScMeasure::toScriptValue(QScriptEngine* eng, const MeasurePtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Measure");
      ScMeasure* cls = qscriptvalue_cast<ScMeasure*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScMeasure::fromScriptValue(const QScriptValue& obj, MeasurePtr& ba)
      {
      MeasurePtr* np = qscriptvalue_cast<MeasurePtr*>(obj.data());
      ba = np ? *np : 0;
      }

//---------------------------------------------------------
//   ScMeasurePropertyIterator
//---------------------------------------------------------

ScMeasurePropertyIterator::ScMeasurePropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScMeasurePropertyIterator::hasNext() const
      {
//      Measure* ba = qscriptvalue_cast<Measure*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScMeasurePropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScMeasurePropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScMeasurePropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScMeasurePropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScMeasurePropertyIterator::toBack()
      {
//      MeasurePtr* ba = qscriptvalue_cast<MeasurePtr*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisMeasure
//---------------------------------------------------------

Measure* ScMeasurePrototype::thisMeasure() const
      {
      MeasurePtr* mp = qscriptvalue_cast<MeasurePtr*>(thisObject().data());
      if (mp)
            return *mp;
      return 0;
      }

//---------------------------------------------------------
//   getLineBreak
//---------------------------------------------------------

bool ScMeasurePrototype::getLineBreak() const
      {
      return thisMeasure()->lineBreak();
      }

//---------------------------------------------------------
//   setLineBreak
//---------------------------------------------------------

void ScMeasurePrototype::setLineBreak(bool v)
      {
      
      Measure* measure = thisMeasure();
      bool lineb = measure->lineBreak();  
      if (!lineb && v){
        LayoutBreak* lb = new LayoutBreak(measure->score());
        lb->setSubtype(LAYOUT_BREAK_LINE);
        lb->setTrack(-1);       // this are system elements
        lb->setParent(measure);
        measure->score()->cmdAdd(lb);
      }
      if (lineb && !v){
        // remove line break
        foreach(Element* e, *measure->el()) {
          if (e->type() == LAYOUT_BREAK && e->subtype() == LAYOUT_BREAK_LINE) {
            measure->score()->cmdRemove(e);
            break;
          }
        }
      }
}

//---------------------------------------------------------
//   getPageNumber
//---------------------------------------------------------

int ScMeasurePrototype::getPageNumber() const
  {
    Measure* m = thisMeasure();
    Page* page = (Page*)m->parent()->parent();
    return page->no();
  }

//---------------------------------------------------------
//   getX
//---------------------------------------------------------

double ScMeasurePrototype::getX() const
  {
    Measure* m = thisMeasure();
    Page* page = (Page*)m->parent()->parent();
    return m->canvasPos().x() - page->canvasPos().x();              
  }

//---------------------------------------------------------
//   getY
//---------------------------------------------------------

double ScMeasurePrototype::getY() const
  {
    Measure* m = thisMeasure();
    return  m->canvasPos().y();
  }

//---------------------------------------------------------
//   getWidth
//---------------------------------------------------------

double ScMeasurePrototype::getWidth() const
  {
    Measure* m = thisMeasure();
    return m->bbox().width();
  }

//---------------------------------------------------------
//   getHeight
//---------------------------------------------------------

double ScMeasurePrototype::getHeight() const{
  Measure* m = thisMeasure();
  return m->bbox().height();
}

      





