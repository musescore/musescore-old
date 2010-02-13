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
      "saveMscz", "saveMscx", "saveXml", "saveMxl", "saveMidi", "savePng", "saveSvg", "saveLilypond",
      "setExpandRepeat", "appendPart", "appendMeasures",
      "pages", "measures", "parts", "part", "startUndo", "endUndo", "setStyle", "hasLyrics", "hasHarmonies",
      "staves"
#ifdef HAS_AUDIOFILE
      , "saveWav", "saveFlac", "saveOgg",
#endif
      };
static const int function_lengths_score[] = {
      1, 1, 1, 1,
      1, 1, 1, 1, 1, 5, 1, 1,
      1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 2, 0, 0, 0
#ifdef HAS_AUDIOFILE
      , 2, 2, 2,
#endif
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
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration | QScriptValue::PropertyGetter
#ifdef HAS_AUDIOFILE
      , QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration,
      QScriptValue::SkipInEnumeration
#endif
      };

ScriptInterface scoreInterface = {
#ifdef HAS_AUDIOFILE
      28,
#else
      25,
#endif
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
      switch(_id) {
            case 0:     // "title",
                  {
                  Text* t = score->getText(TEXT_TITLE);
                  if (context->argumentCount() == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (context->argumentCount() == 1) {
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
                  if (context->argumentCount() == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (context->argumentCount() == 1) {
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
                  if (context->argumentCount() == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (context->argumentCount() == 1) {
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
                  if (context->argumentCount() == 0) {
                        QString s = t ? t->getText() : "";
                        return qScriptValueFromValue(context->engine(), s);
                        }
                  else if (context->argumentCount() == 1) {
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
            case 4:     // "saveMscz",
                  if (context->argumentCount() == 0) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        QFileInfo fi(s);
                        try {
                              score->saveCompressedFile(fi, false);
                              } catch (QString s) {
                              }
                        }
                  break;
            case 5:     // "saveMscx",
                  if (context->argumentCount() == 0) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        QFileInfo fi(s);
                        score->saveFile(fi, false);
                        }
                  break;
            case 6:     // "saveXml",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveXml(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 7:     // "saveMxl",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveMxl(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 8:     // "saveMidi",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveMidi(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 9:     // "savePng",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->savePng(s);
                        return context->engine()->undefinedValue();
                        }
                  if (context->argumentCount() == 5) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        bool screenshot  = context->argument(1).toBool();
                        bool transparent = context->argument(2).toBool();
                        double convDpi = context->argument(3).toNumber();
                        bool grayscale = context->argument(4).toBool();
                        QImage::Format f = grayscale ? QImage::Format_Indexed8 : QImage::Format_ARGB32_Premultiplied;
                        score->savePng(s, screenshot, transparent, convDpi, f);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 10:     // "saveSvg",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveSvg(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 11:    // "saveLilypond",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveLilypond(s);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 12:    // "setExpandRepeat",
                  if (context->argumentCount() == 1) {
                        bool f = context->argument(1).toBool();
                        getAction("repeat")->setChecked(f);
                        preferences.midiExpandRepeats = f;
                        score->updateRepeatList(f);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 13:    // "appendPart",
                  {
                  InstrumentTemplate* t = 0;
                  static InstrumentTemplate defaultInstrument;

                  if (context->argumentCount() == 1) {
                        QString name = qscriptvalue_cast<QString>(context->argument(0));
                        InstrumentTemplate* t = 0;
                        foreach(InstrumentTemplate* it, instrumentTemplates) {
                              if (it->trackName == name) {
                                    t = it;
                                    break;
                                    }
                              }
                        }
                  else if (context->argumentCount() != 0)
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
            case 14:    // "appendMeasures",
                  if (context->argumentCount() == 1) {
                        int n = context->argument(0).toInt32();
                        score->appendMeasures(n, MEASURE);
                        return context->engine()->undefinedValue();
                        }
                  break;

            case 15:    // "pages",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), score->pages().size());
                  break;
            case 16:    // "measures",
                  if (context->argumentCount() == 0) {
                        int n = 0;
                        for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure())
                              n++;
                        return qScriptValueFromValue(context->engine(), n);
                        }
                  break;
            case 17:    // "parts",
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), score->parts()->size());
                  break;
            case 18:    // "part",
                  if (context->argumentCount() == 1) {
                        int n = context->argument(0).toInt32();
                        Part* part = score->parts()->at(n);
                        return qScriptValueFromValue(context->engine(), part);
                        }
                  break;
            case 19:    // "startUndo",
                  if (context->argumentCount() == 1) {
                        score->startCmd();
                        return context->engine()->undefinedValue();
                        }

            case 20:    // "endUndo",
                  if (context->argumentCount() == 1) {
                        score->endCmd();
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 21:    // "setStyle",
                  if (context->argumentCount() == 2) {
                        QString name = qscriptvalue_cast<QString>(context->argument(0));
                        QString val  = qscriptvalue_cast<QString>(context->argument(1));
                        StyleVal sv(name, val);
                        score->setStyle(sv.getIdx(), sv);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 22:    // "hasLyrics",
                  if (context->argumentCount() == 0) {
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
            case 23:    // "hasHarmonies"
                  if (context->argumentCount() == 0) {
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
            case 24:    // staves
                  if (context->argumentCount() == 0)
                        return qScriptValueFromValue(context->engine(), score->nstaves());
                  break;
#ifdef HAS_AUDIOFILE
            case 25:    // "saveWav",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveWav(s);
                        return context->engine()->undefinedValue();
                        }
                  if (context->argumentCount() == 2) {
                        QString s  = qscriptvalue_cast<QString>(context->argument(0));
                        QString sf = qscriptvalue_cast<QString>(context->argument(1));
                        score->saveWav(s, sf);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 26:    // "saveFlac",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveFlac(s);
                        return context->engine()->undefinedValue();
                        }
                  if (context->argumentCount() == 2) {
                        QString s  = qscriptvalue_cast<QString>(context->argument(0));
                        QString sf = qscriptvalue_cast<QString>(context->argument(1));
                        score->saveFlac(s, sf);
                        return context->engine()->undefinedValue();
                        }
                  break;
            case 27:    // "saveOgg",
                  if (context->argumentCount() == 1) {
                        QString s = qscriptvalue_cast<QString>(context->argument(0));
                        score->saveOgg(s);
                        return context->engine()->undefinedValue();
                        }
                  if (context->argumentCount() == 2) {
                        QString s  = qscriptvalue_cast<QString>(context->argument(0));
                        QString sf = qscriptvalue_cast<QString>(context->argument(1));
                        score->saveOgg(s, sf);
                        return context->engine()->undefinedValue();
                        }
                  break;
#endif
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

