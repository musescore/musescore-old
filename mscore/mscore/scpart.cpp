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
#include "part.h"
#include "utils.h"
#include "undo.h"
#include "script.h"

Q_DECLARE_METATYPE(Part*);
Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(TextC*);

static const char* const function_names_part[] = {
      "longName", "shortName", "midiProgram", "midiChannel"
      };
static const int function_lengths_part[] = {
      1, 1, 1, 1
      };
static const QScriptValue::PropertyFlags flags_part[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter
      };

ScriptInterface partInterface = {
      4,
      function_names_part,
      function_lengths_part,
      flags_part
      };

//---------------------------------------------------------
//   prototype_Part_call
//---------------------------------------------------------

static QScriptValue prototype_Part_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Part* part = qscriptvalue_cast<Part*>(context->thisObject());
      if (!part) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Part.%0(): this object is not a Part")
               .arg(function_names_part[_id]));
            }
      switch(_id) {
            case 0:     // "longName",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), part->longName()->getText());
                  else if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        part->setLongName(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 1:     // "shortName",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), part->shortName()->getText());
                  else if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        part->setShortName(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 2:     // "midiProgram",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), part->midiProgram());
                  else if (context->argumentCount() == 1) {
                        int v = context->argument(0).toInt32();
                        if (v >= 0 && v < 128) {
                              part->setMidiProgram(v);
                              return context->engine()->undefinedValue();
                              }
                        }
                  break;
            case 3:     // "midiChannel"
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), part->midiChannel());
                  else if (context->argumentCount() == 1) {
                        int v = context->argument(0).toInt32();
                        if (v >= 0 && v < 16) {
                              part->setMidiChannel(v);
                              return context->engine()->undefinedValue();
                              }
                        }
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Part.%0(): bad argument count or value")
         .arg(function_names_part[_id]));
      }

//---------------------------------------------------------
//   static_Part_call
//---------------------------------------------------------

static QScriptValue static_Part_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Part(): Did you forget to construct with 'new'?"));
      Part* part = 0;
      if (context->argumentCount() == 0)
            part = new Part(0);
      else if (context->argumentCount() == 1) {
            Score* score = qscriptvalue_cast<Score*>(context->argument(0));
            part   = new Part(score);
            }
      if (part)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(part));
      return context->throwError(QString::fromLatin1("Part(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_Part_class
//---------------------------------------------------------

QScriptValue create_Part_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &partInterface;

      engine->setDefaultPrototype(qMetaTypeId<Part*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Part*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Part_call, function_lengths_part[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Part*>(), proto);
      return engine->newFunction(static_Part_call, proto, 1);
      }
