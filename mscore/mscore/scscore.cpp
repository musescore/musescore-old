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
#include "scscore.h"
#include "instrtemplate.h"
#include "clef.h"
#include "staff.h"
#include "part.h"
#include "system.h"
#include "page.h"
#include "text.h"
#include "box.h"
#include "preferences.h"

//---------------------------------------------------------
//   ScScorePropertyIterator
//---------------------------------------------------------

class ScScorePropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScScorePropertyIterator(const QScriptValue &object);
      ~ScScorePropertyIterator() {}
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
//   ScScore
//---------------------------------------------------------

ScScore::ScScore(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<ScorePtr>(engine, toScriptValue, fromScriptValue);

      scoreName   = engine->toStringHandle(QLatin1String("name"));
      scoreStaves = engine->toStringHandle(QLatin1String("staves"));

      proto = engine->newQObject(new ScScorePrototype(this),
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

QScriptClass::QueryFlags ScScore::queryProperty(const QScriptValue &object,
   const QScriptString& name, QueryFlags flags, uint* /*id*/)
      {
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(object.data());
      if (!sp)
            return 0;

      if ((name == scoreName) || (name == scoreStaves))
            return flags;
      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScScore::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
printf("property <%s>\n", qPrintable(name.toString()));
      ScorePtr* score = qscriptvalue_cast<ScorePtr*>(object.data());
      if (!score)
            return QScriptValue();
      if (name == scoreName)
            return QScriptValue(engine(), (*score)->name());
      else if (name == scoreStaves)
            return QScriptValue(engine(), (*score)->nstaves());
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScScore::setProperty(QScriptValue &object,
   const QScriptString& s, uint /*id*/, const QScriptValue& value)
      {
      ScorePtr* score = qscriptvalue_cast<ScorePtr*>(object.data());
      if (!score)
            return;
      if (s == scoreName) {
            (*score)->setName(value.toString());
            mscore->updateTabNames();
            }
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScScore::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& name, uint /*id*/)
      {
      if (name == scoreName)
            return QScriptValue::Undeletable;
      else if (name == scoreStaves)
            return QScriptValue::Undeletable | QScriptValue::ReadOnly;
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator *ScScore::newIterator(const QScriptValue &object)
      {
      return new ScScorePropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScScore::newInstance(const QString& name)
      {
      printf("ScScore::newInstance(name)\n");

      QString s(name);
      Score* ns = new Score(defaultStyle);
      if (s.isEmpty())
            s = mscore->createDefaultName();
      ns->setName(s);
      mscore->appendScore(ns);
      return newInstance(ns);
      }

QScriptValue ScScore::newInstance(const ScorePtr& score)
      {
      printf("ScScore::newInstance(ScorePtr)\n");

      QScriptValue data = engine()->newVariant(qVariantFromValue(score));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScScore::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScScore *cls = qscriptvalue_cast<ScScore*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QString s = ctx->argument(0).toString();
      return cls->newInstance(s);
      }

QScriptValue ScScore::toScriptValue(QScriptEngine* eng, const ScorePtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Score");
      ScScore* cls = qscriptvalue_cast<ScScore*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScScore::fromScriptValue(const QScriptValue& obj, ScorePtr& ba)
      {
      ba = qscriptvalue_cast<ScorePtr>(obj.data());
      }

//---------------------------------------------------------
//   ScScorePropertyIterator
//---------------------------------------------------------

ScScorePropertyIterator::ScScorePropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScScorePropertyIterator::hasNext() const
      {
//      Score* ba = qscriptvalue_cast<Score*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScScorePropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScScorePropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScScorePropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScScorePropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScScorePropertyIterator::toBack()
      {
//      ScorePtr* ba = qscriptvalue_cast<ScorePtr*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisScore
//---------------------------------------------------------

Score* ScScorePrototype::thisScore() const
      {
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(thisObject().data());
      if (sp)
            return *sp;
      return 0;
      }

//---------------------------------------------------------
//   saveXml
//---------------------------------------------------------

bool ScScorePrototype::saveXml(const QString& name)
      {
      return thisScore()->saveXml(name);
      }

//---------------------------------------------------------
//   saveMxl
//---------------------------------------------------------

bool ScScorePrototype::saveMxl(const QString& name)
      {
      return thisScore()->saveMxl(name);
      }

//---------------------------------------------------------
//   saveMidi
//---------------------------------------------------------

bool ScScorePrototype::saveMidi(const QString& name)
      {
      return thisScore()->saveMidi(name);
      }

//---------------------------------------------------------
//   savePng
//---------------------------------------------------------

bool ScScorePrototype::savePng(const QString& name)
      {
      return thisScore()->savePng(name);
      }

//---------------------------------------------------------
//   savePng with options
//---------------------------------------------------------

bool ScScorePrototype::savePng(const QString& name, bool screenshot, bool transparent, double convDpi)
      {
      return thisScore()->savePng(name, screenshot, transparent, convDpi);
      }

//---------------------------------------------------------
//   saveSvg
//---------------------------------------------------------

bool ScScorePrototype::saveSvg(const QString& name)
      {
      return thisScore()->saveSvg(name);
      }

//---------------------------------------------------------
//   saveLilypond
//---------------------------------------------------------

bool ScScorePrototype::saveLilypond(const QString& name)
      {
      return thisScore()->saveLilypond(name);
      }

#ifdef HAS_AUDIOFILE
//---------------------------------------------------------
//   saveWav
//---------------------------------------------------------

bool ScScorePrototype::saveWav(const QString& name)
      {
      return thisScore()->saveWav(name);
      }


//---------------------------------------------------------
//   saveWav
//---------------------------------------------------------

bool ScScorePrototype::saveWav(const QString& name, const QString& soundFont)
      {
      bool result = false;
      if(soundFont.endsWith(".sf2",Qt::CaseInsensitive)){
        QString save = preferences.soundFont;
        preferences.soundFont = soundFont;
        result = thisScore()->saveWav(name); 
        preferences.soundFont = save;
      }
      return result;
      }

//---------------------------------------------------------
//   saveOgg
//---------------------------------------------------------

bool ScScorePrototype::saveOgg(const QString& name)
      {
      return thisScore()->saveOgg(name);
      }


//---------------------------------------------------------
//   saveOgg
//---------------------------------------------------------

bool ScScorePrototype::saveOgg(const QString& name, const QString& soundFont)
      {
      bool result = false;
      if(soundFont.endsWith(".sf2",Qt::CaseInsensitive)){
        QString save = preferences.soundFont;
        preferences.soundFont = soundFont;
        result = thisScore()->saveOgg(name); 
        preferences.soundFont = save;
      }
      return result;
      }
      
//---------------------------------------------------------
//   saveFlac
//---------------------------------------------------------

bool ScScorePrototype::saveFlac(const QString& name)
      {
      return thisScore()->saveFlac(name);
      }


//---------------------------------------------------------
//   saveFlac
//---------------------------------------------------------

bool ScScorePrototype::saveFlac(const QString& name, const QString& soundFont)
      {
      bool result = false;
      if(soundFont.endsWith(".sf2",Qt::CaseInsensitive)){
        QString save = preferences.soundFont;
        preferences.soundFont = soundFont;
        result = thisScore()->saveFlac(name); 
        preferences.soundFont = save;
      }
      return result;
      }
#endif

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void ScScorePrototype::appendMeasures(int n)
      {
      thisScore()->appendMeasures(n, MEASURE);
      }

//---------------------------------------------------------
//   appendPart
//---------------------------------------------------------

void ScScorePrototype::appendPart(const QString& name)
      {
      InstrumentTemplate* t = 0;
      foreach(InstrumentTemplate* it, instrumentTemplates) {
            if (it->trackName == name) {
                  t = it;
                  break;
                  }
            }
      if (t == 0)
            return;
      Part* part = new Part(thisScore());
      part->initFromInstrTemplate(t);
      for (int i = 0; i < t->staves; ++i) {
            Staff* staff = new Staff(thisScore(), part, i);
            staff->clefList()->setClef(0, t->clefIdx[i]);
            staff->setLines(t->staffLines[i]);
            staff->setSmall(t->smallStaff[i]);
            staff->setRstaff(i);
            if (i == 0) {
                  staff->setBracket(0, t->bracket);
                  staff->setBracketSpan(0, t->staves);
                  }
            thisScore()->staves().insert(i, staff);
            part->staves()->push_back(staff);
            }
      thisScore()->insertPart(part, 0);
      thisScore()->fixTicks();
      thisScore()->rebuildMidiMapping();
      }

//---------------------------------------------------------
//   setTitle
//---------------------------------------------------------

void ScScorePrototype::setTitle(const QString& text)
      {
      MeasureBaseList* ml = thisScore()->measures();
      MeasureBase* measure;
      if (!ml->first() || ml->first()->type() != VBOX) {
            measure = new VBox(thisScore());
            measure->setTick(0);
            thisScore()->undoInsertMeasure(measure);
            }
      else
            measure = ml->first();
      Text* s = new Text(thisScore());
      s->setTextStyle(TEXT_STYLE_TITLE);
      s->setSubtype(TEXT_TITLE);
      s->setParent(measure);
      s->setText(text);
      thisScore()->undoAddElement(s);
      }

