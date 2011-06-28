//=============================================================================
//  MusE Score
//  Linux Music Score Editor

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
#include "timesig.h"
#include "script.h"


Q_DECLARE_METATYPE(TimeSig*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_timeSig[] = {
      "setAllaBreve", "setCommonTime", "type", "change"
      };
static const int function_lengths_timeSig[] = {
      0, 0, 1, 2
      };
static const QScriptValue::PropertyFlags flags_timeSig[] = {
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration
      };

ScriptInterface timeSigInterface = {
      sizeof(function_names_timeSig) / sizeof(*function_names_timeSig),
      function_names_timeSig,
      function_lengths_timeSig,
      flags_timeSig
      };

//---------------------------------------------------------
//   prototype_TimeSig_call
//---------------------------------------------------------

static QScriptValue prototype_TimeSig_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      TimeSig* timeSig = qscriptvalue_cast<TimeSig*>(context->thisObject());
      if (!timeSig) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("TimeSig.%0(): this object is not a TimeSig")
               .arg(function_names_timeSig[_id]));
            }
      switch(_id) {
            case 0:     // "setAllaBreve"
                  if (context->argumentCount() == 0) {
                        // check if timesig added to score and change all clones of it, if any
                        if (timeSig->score() != 0)
                              timeSig->score()->changeTimeSig(timeSig->tick(), TSIG_ALLA_BREVE);
                        else
                            timeSig->setSubtype(TSIG_ALLA_BREVE);

                        return context->engine()->undefinedValue();
                  }
                  break;
            case 1:     // "setCommonTime"
                  if (context->argumentCount() == 0) {
                        // check if timesig added to score and change all clones of it, if any
                        if (timeSig->score() != 0)
                              timeSig->score()->changeTimeSig(timeSig->tick(), TSIG_FOUR_FOUR);
                        else
                              timeSig->setSubtype(TSIG_FOUR_FOUR);

                        return context->engine()->undefinedValue();
                  }
                  break;
            case 2:     // "type"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), timeSig->subtype());
                  else if (context->argumentCount() == 1) {
                        int subtype = context->argument(0).toInt32();

                        // check if timesig added to score and change all clones of it, if any
                        if (timeSig->score() != 0)
                              timeSig->score()->changeTimeSig(timeSig->tick(), subtype);
                        else
                              timeSig->setSubtype(subtype);

                        return context->engine()->undefinedValue();
                  }
                  break;
            case 3:     // "change",
                  if (context->argumentCount() == 2) {
                        int numerator = context->argument(0).toInt32();
                        int denominator = context->argument(1).toInt32();

                        // check if timesig added to score and change all clones of it, if any
                        if (timeSig->score() != 0)
                              timeSig->score()->changeTimeSig(timeSig->tick(), TimeSig::sigtype(denominator, numerator));
                        else
                              timeSig->setSig(denominator, numerator);

                  return context->engine()->undefinedValue();
                  }

                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("TimeSig.%0(): bad argument count or value")
         .arg(function_names_timeSig[_id]));
      }

//---------------------------------------------------------
//   static_TimeSig_call
//---------------------------------------------------------

static QScriptValue static_TimeSig_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("TimeSig(): Did you forget to construct with 'new'?"));
      TimeSig* timeSig = 0;

      //Score* score = qscriptvalue_cast<Score*>(context->argument(0));
      if (context->argumentCount() == 0) {
          timeSig = new TimeSig(0);
      }
      else if (context->argumentCount() == 2)
          timeSig = new TimeSig(0, context->argument(1).toInt32(), context->argument(0).toInt32());

      // TODO should support more than one nominator

      if (timeSig)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(timeSig));
      return context->throwError(QString::fromLatin1("TimeSig(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_TimeSig_class
//---------------------------------------------------------

QScriptValue create_TimeSig_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &timeSigInterface;

      engine->setDefaultPrototype(qMetaTypeId<TimeSig*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((TimeSig*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_TimeSig_call, function_lengths_timeSig[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<TimeSig*>(), proto);
      return engine->newFunction(static_TimeSig_call, proto, 1);
      }

