//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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
#include "instrtemplate.h"
#include "clef.h"
#include "staff.h"
#include "part.h"
#include "system.h"
#include "page.h"
#include "text.h"
#include "box.h"
#include "preferences.h"
#include "style.h"
#include "measure.h"
#include "segment.h"
#include "harmony.h"
#include "script.h"
#include "score.h"


Q_DECLARE_METATYPE(Score*);
Q_DECLARE_METATYPE(Part*);
Q_DECLARE_METATYPE(Text*);

static const char* const function_names_score[] = {
      "title", "subtitle", "composer", "poet",
      "load", "save",
      "setExpandRepeat", "appendPart", "appendMeasures",
      "pages", "measures", "parts", "part", "startUndo", "endUndo", "setStyle", "hasLyrics", "hasHarmonies",
      "staves"
      };
static const int function_lengths_score[] = {
      1, 1, 1, 1,
      1, 2,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0,
      };

static const QScriptValue::PropertyFlags flags_score[] = {
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter | QScriptValue::PropertySetter,

      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,

      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,

      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter,
      };

ScriptInterface scoreInterface = {
      19,
      function_names_score,
      function_lengths_score,
      flags_score
      };

//---------------------------------------------------------
//   prototype_Score_call
//---------------------------------------------------------

static QScriptValue prototype_Score_call(QScriptContext* context, QScriptEngine*)
      {
      Q_ASSERT(context->callee().isFunction());
      uint _id = context->callee().data().toUInt32();
      Q_ASSERT((_id & 0xFFFF0000) == 0xBABF0000);
      _id &= 0xffff;

      Score* score = qscriptvalue_cast<Score*>(context->thisObject());
      if (!score) {
            return context->throwError(QScriptContext::TypeError,
               QString::fromLatin1("Score.%0(): this object is not a Score")
               .arg(function_names_score[_id]));
            }
      int argc = context->argumentCount();
      switch(_id) {
            case 0:     // "title",
                  {
                  Text* t = score->getText(TEXT_TITLE);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else {
                              ; // TODO-SCRIPT
                              }
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 1:     // "subtitle",
                  {
                  Text* t = score->getText(TEXT_SUBTITLE);
                  if (argc) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else {
                              ; // TODO-SCRIPT
                              }
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 2:     // "composer",
                  {
                  Text* t = score->getText(TEXT_COMPOSER);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else {
                              ; // TODO-SCRIPT
                              }
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 3:     // "poet",
                  {
                  Text* t = score->getText(TEXT_POET);
                  if (argc == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        if (t)
                              t->setText(s);
                        else {
                              ; // TODO-SCRIPT
                              }
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 4:    // "load",
                  if (argc == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        return qScriptValueFromValue(context->engine(), score->read(s));
                        }
                  break;
            case 5:     // "save",
                  {
                  QString s, ext, sf;
                  if (argc >= 2) {
                        s = qscriptvalue_cast<QString>(context->argument(0));
                        ext = qscriptvalue_cast<QString>(context->argument(1));
                        }
                  if (argc == 3) {
                        sf = qscriptvalue_cast<QString>(context->argument(1));
                        return qScriptValueFromValue(context->engine(), score->saveAudio(s, ext, sf));
                        }
                  else if (argc == 2)
                        return qScriptValueFromValue(context->engine(), score->saveAs(false, s, ext));
                  else if (argc == 6 && ext == "png") {
                        bool screenshot  = context->argument(2).toBool();
                        bool transparent = context->argument(3).toBool();
                        double convDpi = context->argument(4).toNumber();
                        bool grayscale = context->argument(5).toBool();
                        QImage::Format f = grayscale ? QImage::Format_Indexed8 : QImage::Format_ARGB32_Premultiplied;
                        score->savePng(s, screenshot, transparent, convDpi, f);
                        return context->engine()->undefinedValue();
                        }
                  }
                  break;
            case 6:    // "setExpandRepeat",
                  if (argc == 1) {
                        bool f = context->argument(1).toBool();
                        getAction("repeat")->setChecked(f);
                        preferences.midiExpandRepeats = f;
                        score->updateRepeatList(f);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 7:    // "appendPart",
                  {
                  InstrumentTemplate* t = 0;
                  static InstrumentTemplate defaultInstrument;

                  if (argc == 1)
                        t = searchTemplate(qscriptvalue_cast<QString>(context->argument(0)));
                  else if (argc != 0)
                        break;
                  if (t == 0) {
                        t = &defaultInstrument;
                        if (t->channel.isEmpty()) {
                              Channel* a      = new Channel();
                              a->chorus       = 0;
                              a->reverb       = 0;
                              a->name         = "normal";
                              a->program      = 0;
                              a->bank         = 0;
                              a->volume       = 100;
                              a ->pan         = 60;
                              t->channel.append(a);
                              }
                        }
                  Part* part = new Part(score);
                  part->initFromInstrTemplate(t);
                  for (int i = 0; i < t->staves; ++i) {
                        Staff* staff = new Staff(score, part, i);
                        staff->clefList()->setClef(0, t->clefIdx[i]);
                        staff->setLines(t->staffLines[i]);
                        staff->setSmall(t->smallStaff[i]);
                        staff->setRstaff(i);
                        if (i == 0) {
                              staff->setBracket(0, t->bracket);
                              staff->setBracketSpan(0, t->staves);
                              }
                        score->staves().insert(i, staff);
                        part->staves()->push_back(staff);
                        }
                  score->insertPart(part, 0);
                  score->fixTicks();
                  score->rebuildMidiMapping();
                  return context->engine()->undefinedValue();
                  }
                  break;
            case 8:    // "appendMeasures",
                  if (argc == 1) {
                        int n = context->argument(0).toInt32();
                        score->appendMeasures(n, MEASURE);
                        return context->engine()->undefinedValue();
                        }
                  break;

            case 9:    // "pages",
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->pages().size());
                  break;
            case 10:    // "measures",
                  if (argc == 0) {
                        int n = 0;
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure())
                              n++;
                        return qScriptValueFromValue(context->engine(), n);
                        }
                  break;
            case 11:    // "parts",
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->parts()->size());
                  break;
            case 12:    // "part",
                  if (argc == 1) {
                        int n = context->argument(0).toInt32();
                        Part* part = score->parts()->at(n);
                        return qScriptValueFromValue(context->engine(), part);
                        }
                  break;
            case 13:    // "startUndo",
                  if (argc == 0) {
                        score->startCmd();
                        return context->engine()->undefinedValue();
                        }

            case 14:    // "endUndo",
                  if (argc == 0) {
                        score->endCmd();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 15:    // "setStyle",
                  if (argc == 2) {
                        QString name = qscriptvalue_cast<QString>(context->argument(0));
                        QString val  = qscriptvalue_cast<QString>(context->argument(1));
                        StyleVal sv(name, val);
                        score->setStyle(sv.getIdx(), sv);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 16:    // "hasLyrics",
                  if (argc == 0) {
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                              for (Segment* seg = m->first(); seg; seg = seg->next()) {
                                    for (int i = 0; i < score->nstaves(); ++i) {
                                          if (seg->lyricsList(i)->size() > 0)
                                                return qScriptValueFromValue(context->engine(), true);
                                          }
                                    }
                              }
                        return qScriptValueFromValue(context->engine(), false);
                        }
                  break;
            case 17:    // "hasHarmonies"
                  if (argc == 0) {
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                              foreach(Element* element, *m->el()) {
                                    if (element->type() == HARMONY) {
                                          Harmony* h = static_cast<Harmony*>(element);
                                          if (h->id() != -1)
                                                return qScriptValueFromValue(context->engine(), true);
                                          }
                                    }
                              }
                        return qScriptValueFromValue(context->engine(), false);
                        }
                  break;
            case 18:    // staves
                  if (argc == 0)
                        return qScriptValueFromValue(context->engine(), score->nstaves());
                  break;
            }
      return context->throwError(QScriptContext::TypeError,
         QString::fromLatin1("Score.%0(): bad argument count or value")
         .arg(function_names_score[_id]));
      }

//---------------------------------------------------------
//   static_Score_call
//---------------------------------------------------------

static QScriptValue static_Score_call(QScriptContext* context, QScriptEngine*)
      {
      if (context->thisObject().strictlyEquals(context->engine()->globalObject()))
            return context->throwError(QString::fromLatin1("Score(): Did you forget to construct with 'new'?"));
      Score* score = new Score(defaultStyle);
      score->setName(mscore->createDefaultName());
      mscore->setCurrentScoreView(mscore->appendScore(score));
      score->startCmd();
      return context->engine()->newVariant(context->thisObject(), qVariantFromValue(score));
      }

//---------------------------------------------------------
//   create_Score_class
//---------------------------------------------------------

QScriptValue create_Score_class(QScriptEngine* engine)
      {
      ScriptInterface* si = &scoreInterface;

      engine->setDefaultPrototype(qMetaTypeId<Score*>(), QScriptValue());
      QScriptValue proto = engine->newVariant(qVariantFromValue((Score*)0));

      for (int i = 0; i < si->n; ++i) {
            QScriptValue fun = engine->newFunction(prototype_Score_call, function_lengths_score[i]);
            fun.setData(QScriptValue(engine, uint(0xBABF0000 + i)));
            proto.setProperty(si->name(i), fun, si->flag(i));
            }

      engine->setDefaultPrototype(qMetaTypeId<Score*>(), proto);
      return engine->newFunction(static_Score_call, proto, 1);
      }


#if 0
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
#endif

