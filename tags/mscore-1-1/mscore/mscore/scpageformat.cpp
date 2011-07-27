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
#include "page.h"
#include "utils.h"
#include "script.h"

Q_DECLARE_METATYPE(PageFormat*);
Q_DECLARE_METATYPE(Score*);

static const char* const function_names_pageFormat[] = {
      "landscape", "twosided", "width", "height", "size"
      };
static const int function_lengths_pageFormat[] = {
      0, 0, 1, 1, 1
      };
static const QScriptValue::PropertyFlags flags_pageFormat[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      };

ScriptInterface pageFormatInterface = {
      sizeof(function_names_pageFormat) / sizeof(*function_names_pageFormat),
      function_names_pageFormat,
      function_lengths_pageFormat,
      flags_pageFormat
      };

//---------------------------------------------------------
//   prototype_PageFormat_call
//---------------------------------------------------------

static QScriptValue prototype_PageFormat_call(QScriptContext* context, QScriptEngine* se)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      PageFormat* pageFormat = qscriptvalue_cast<PageFormat*>(context->thisObject());
      if (!pageFormat) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("PageFormat.%0(): this object is not a PageFormat")
               .arg(function_names_pageFormat[_id]));
            }
      switch(_id) {
            case 0:     // "landscape",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), pageFormat->landscape);
                  break;
            case 1:     // "twosided",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), pageFormat->twosided);
                  break;
            case 2:     // "width",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), pageFormat->width() * INCH );
                  else if (context->argumentCount() == 1) {
                        int v = context->argument(0).toInt32();
                        const PageFormat& pfo = *pageFormat;
                        PageFormat pf(pfo);
                        pf._width = v / INCH;
                        pf.size = paperSizeNameToIndex("Custom");

                        Score* cs = qscriptvalue_cast<Score*>(se->globalObject().property("curScore"));
                        
                        cs->undoChangePageFormat(&pf, cs->spatium());
                        cs->setLayoutAll(true);
                        
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 3:     // "height",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), pageFormat->height() * INCH );
                  else if (context->argumentCount() == 1) {
                        int v = context->argument(0).toInt32();
                        const PageFormat& pfo = *pageFormat;
                        PageFormat pf(pfo);
                        pf._height = v / INCH;
                        pf.size = paperSizeNameToIndex("Custom");

                        Score* cs = qscriptvalue_cast<Score*>(se->globalObject().property("curScore"));
                        
                        cs->undoChangePageFormat(&pf, cs->spatium());
                        cs->setLayoutAll(true);
                        
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 4:     // "size",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), pageFormat->size);
                  else if (context->argumentCount() == 1) {
                        int v = context->argument(0).toInt32();
                        const PageFormat& pfo = *pageFormat;
                        PageFormat pf(pfo);
                        pf.size = v;
                        Score* cs = qscriptvalue_cast<Score*>(se->globalObject().property("curScore"));
                        
                        cs->undoChangePageFormat(&pf, cs->spatium());
                        cs->setLayoutAll(true);
                        
                        return context->engine()->undefinedValue();
                        }
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("PageFormat.%0(): bad argument count or value")
         .arg(function_names_pageFormat[_id]));
      }

//---------------------------------------------------------
//   static_PageFormat_call
//---------------------------------------------------------

static QScriptValue static_PageFormat_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("PageFormat(): Did you forget to construct with 'new'?"));
      PageFormat* pageFormat = 0;
      if (context->argumentCount() == 0)
            pageFormat = new PageFormat();
      if (pageFormat)
            return context->engine()->newVariant(context->thisObject(), qVariantFromValue(pageFormat));
      return context->throwError(QString::fromLatin1("PageFormat(): wrong argument count"));
      }

//---------------------------------------------------------
//   create_PageFormat_class
//---------------------------------------------------------

QScriptValue create_PageFormat_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &pageFormatInterface;

      engine->setDefaultPrototype(qMetaTypeId<PageFormat*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((PageFormat*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_PageFormat_call, function_lengths_pageFormat[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<PageFormat*>(), proto);
      return engine->newFunction(static_PageFormat_call, proto, 1);
      }
