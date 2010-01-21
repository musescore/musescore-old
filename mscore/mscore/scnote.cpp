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
#include "scnote.h"
#include "note.h"
#include "utils.h"
#include "scscore.h"
#include "undo.h"

//---------------------------------------------------------
//   ScNote
//---------------------------------------------------------

ScNote::ScNote(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<NotePtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScNotePrototype(this),
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

QScriptValue ScNote::newInstance(Score* score)
      {
      Note* note = new Note(score);
      return newInstance(note);
      }

QScriptValue ScNote::newInstance(const NotePtr& note)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(note));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScNote::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScNote *cls = qscriptvalue_cast<ScNote*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp   = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScNote::toScriptValue(QScriptEngine* eng, const NotePtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Note");
      ScNote* cls = qscriptvalue_cast<ScNote*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScNote::fromScriptValue(const QScriptValue& obj, NotePtr& ba)
      {
      NotePtr* np = qscriptvalue_cast<NotePtr*>(obj.data());
      ba = np ? *np : 0;
      }

//---------------------------------------------------------
//   thisNote
//---------------------------------------------------------

Note* ScNotePrototype::thisNote() const
      {
      NotePtr* np = qscriptvalue_cast<NotePtr*>(thisObject().data());
      if (np)
            return *np;
      return 0;
      }

//---------------------------------------------------------
//   noteName
//---------------------------------------------------------

QString ScNotePrototype::getName() const
      {
      return pitch2string(thisNote()->pitch());
      }

//---------------------------------------------------------
//   getPitch
//---------------------------------------------------------

int ScNotePrototype::getPitch() const
      {
      return thisNote()->pitch();
      }

//---------------------------------------------------------
//   setPitch
//---------------------------------------------------------

void ScNotePrototype::setPitch(int v)
      {
      thisNote()->setPitch(v);
      thisNote()->setTpcFromPitch();
      }

//---------------------------------------------------------
//   getTuning
//---------------------------------------------------------

double ScNotePrototype::getTuning() const
      {
      return thisNote()->tuning();
      }

//---------------------------------------------------------
//   setTuning
//---------------------------------------------------------

void ScNotePrototype::setTuning(double v)
      {
      thisNote()->setTuning(v);
      }

//---------------------------------------------------------
//   getColor
//---------------------------------------------------------

QColor ScNotePrototype::getColor() const
      {
      return thisNote()->color();
      }

//---------------------------------------------------------
//   setColor
//---------------------------------------------------------

void ScNotePrototype::setColor(const QColor& c)
      {
      Note* note = thisNote();
      Score* score = note->score();
printf("Note: setColor score %p %d %d %d\n", score, c.red(), c.green(), c.blue());
      if (score)
            score->undo()->push(new ChangeColor(note, c));
      else
            note->setColor(c);
      }




