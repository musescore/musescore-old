//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: scpart.cpp 2301 2009-11-04 11:08:30Z wschweer $
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
#include "scpart.h"
#include "part.h"
#include "utils.h"
#include "scscore.h"
#include "undo.h"

//---------------------------------------------------------
//   ScPart
//---------------------------------------------------------

ScPart::ScPart(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<PartPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScPartPrototype(this),
         QScriptEngine::QtOwnership,
         QScriptEngine::SkipMethodsInEnumeration
          | QScriptEngine::ExcludeSuperClassMethods
          | QScriptEngine::ExcludeSuperClassProperties
         );
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScPart::newInstance(Score* score)
      {
      Part* part = new Part(score);
      return newInstance(part);
      }

QScriptValue ScPart::newInstance(const PartPtr& part)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(part));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScPart::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScPart *cls = qscriptvalue_cast<ScPart*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScPart::toScriptValue(QScriptEngine* eng, const PartPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Part");
      ScPart* cls = qscriptvalue_cast<ScPart*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScPart::fromScriptValue(const QScriptValue& obj, PartPtr& ba)
      {
      PartPtr* np = qscriptvalue_cast<PartPtr*>(obj.data());
      ba = np ? *np : 0;
      }

//---------------------------------------------------------
//   thisPart
//---------------------------------------------------------

Part* ScPartPrototype::thisPart() const
      {
      PartPtr* np = qscriptvalue_cast<PartPtr*>(thisObject().data());
      if (np)
            return *np;
      return 0;
      }

//---------------------------------------------------------
//   longName
//---------------------------------------------------------

QString ScPartPrototype::getLongName() const
      {
      return thisPart()->longName()->getText().toUtf8();
      }

//---------------------------------------------------------
//   shortName
//---------------------------------------------------------

QString ScPartPrototype::getShortName() const
      {
      return thisPart()->shortName()->getText().toUtf8();
      }
      
//---------------------------------------------------------
//   midiProgram
//---------------------------------------------------------

int ScPartPrototype::getMidiProgram() const
      {
      return thisPart()->midiProgram();
      }

//---------------------------------------------------------
//   midiChannel
//---------------------------------------------------------

int ScPartPrototype::getMidiChannel() const
      {
      return thisPart()->midiChannel();
      }





