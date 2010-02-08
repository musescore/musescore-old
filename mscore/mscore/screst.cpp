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

#include "mscore.h"
#include "screst.h"
#include "rest.h"
#include "chord.h"
#include "chordrest.h"
#include "harmony.h"
#include "scscore.h"
#include "scchord.h"
#include "measure.h"
#include "note.h"

//---------------------------------------------------------
//   ScRest
//---------------------------------------------------------

ScRest::ScRest(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<RestPtr>(engine, toScriptValue, fromScriptValue);

      proto = engine->newQObject(new ScRestPrototype(this),
         QScriptEngine::QtOwnership, QScriptEngine::SkipMethodsInEnumeration);
      QScriptValue global = engine->globalObject();
      proto.setPrototype(global.property("Object").property("prototype"));

      ctor = engine->newFunction(construct);
      ctor.setData(qScriptValueFromValue(engine, this));
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScRest::newInstance(Score* score)
      {
      Rest* rest = new Rest(score);
      return newInstance(rest);
      }

QScriptValue ScRest::newInstance(const RestPtr& cr)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(cr));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScRest::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScRest *cls = qscriptvalue_cast<ScRest*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(v.data());
      return cls->newInstance(sp ? *sp : 0);
      }

QScriptValue ScRest::toScriptValue(QScriptEngine* eng, const RestPtr& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Rest");
      ScRest* cls = qscriptvalue_cast<ScRest*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue((RestPtr&)ba));
      return cls->newInstance(ba);
      }

void ScRest::fromScriptValue(const QScriptValue& obj, RestPtr& ba)
      {
      Rest** cp = qscriptvalue_cast<RestPtr*>(obj.data());
      ba = cp ? *cp : 0;
      }

//---------------------------------------------------------
//   thisRest
//---------------------------------------------------------

Rest* ScRestPrototype::thisRest() const
      {
      // Rest** cp = (Rest**)qscriptvalue_cast<RestPtr*>(thisObject().data());
      Rest** cp = (Rest**)qscriptvalue_cast<RestPtr*>(thisObject());
      if (cp)
            return *cp;
      return 0;
      }

//---------------------------------------------------------
//   thisChordRest
//---------------------------------------------------------

ChordRest* ScChordRestPrototype::thisChordRest() const
      {
      Chord** c = qscriptvalue_cast<ChordPtr*>(thisObject());
      if (c)
            return static_cast<ChordRest*>(*c);
      Rest** r = qscriptvalue_cast<RestPtr*>(thisObject());
      if (r)
            return static_cast<ChordRest*>(*r);
      printf("thisChordRest(): null\n");
      return 0;
      }

//---------------------------------------------------------
//   getTickLen
//---------------------------------------------------------

int ScChordRestPrototype::getTickLen() const
      {
      ChordRest* cr = thisChordRest();
      return cr->tickLen();
      }

//---------------------------------------------------------
//   setTickLen
//---------------------------------------------------------

void ScChordRestPrototype::setTickLen(int v)
      {
      ChordRest* cr = thisChordRest();
      cr->setDurationVal(v);
      }

//---------------------------------------------------------
//   addHarmony
//---------------------------------------------------------

void ScChordRestPrototype::addHarmony(HarmonyPtr h)
      {
      ChordRest* cr = thisChordRest();

      h->setParent(cr->measure());
      h->setTick(cr->tick());
      Score* score = cr->score();
      if (score) {
            h->setScore(score);
            cr->score()->undoAddElement(h);
            }
      else
            cr->measure()->add(h);
      h->render();
      }

//---------------------------------------------------------
//   topNote
//---------------------------------------------------------

NotePtr ScChordRestPrototype::topNote() const
      {
      ChordRest* cr = thisChordRest();
      return cr->type() == CHORD ? static_cast<Chord*>(cr)->upNote() : 0;
      }

//---------------------------------------------------------
//   addNote
//---------------------------------------------------------

void ScChordRestPrototype::addNote(NotePtr note)
      {
      ChordRest* cr = thisChordRest();
      if (cr->type() != CHORD)
            return;
      Chord* chord = static_cast<Chord*>(cr);
      note->setParent(chord);
      Score* score = chord->score();
      if (score) {
            note->setScore(score);
            chord->score()->undoAddElement(note);
            }
      else
            chord->add(note);
      }

//---------------------------------------------------------
//   removeNote
//---------------------------------------------------------

void ScChordRestPrototype::removeNote(int idx)
      {
      ChordRest* cr = thisChordRest();
      if (cr->type() != CHORD)
            return;
      Chord* chord = static_cast<Chord*>(cr);

      NoteList* nl = chord->noteList();
      if (idx < 0 || idx >= int(nl->size()))
            return;
      Score* score = chord->score();
      if (score) {
            NotePtr n = note(idx);
            score->undoRemoveElement(n);
            }
      else {
            int k = 0;
            for (iNote i = nl->begin(); i != nl->end(); ++i) {
                  if (k == idx) {
                        nl->erase(i);
                        break;
                        }
                  ++k;
                  }
            }
      }

//---------------------------------------------------------
//   notes
//---------------------------------------------------------

int ScChordRestPrototype::notes() const
      {
      ChordRest* cr = thisChordRest();
      if (cr->type() != CHORD)
            return 0;
      Chord* chord = static_cast<Chord*>(cr);
      const NoteList* nl = chord->noteList();
      return nl->size();
      }

//---------------------------------------------------------
//   note
//---------------------------------------------------------

NotePtr ScChordRestPrototype::note(int idx) const
      {
      ChordRest* cr = thisChordRest();
      if (cr->type() != CHORD)
            return 0;
      Chord* chord = static_cast<Chord*>(cr);
      const NoteList* nl = chord->noteList();
      if (idx < 0 || idx >= int(nl->size())) {
            printf("ScChordRest::note(%d): index out of range\n", idx);
            return 0;
            }
      int k = 0;
      for (ciNote i = nl->begin(); i != nl->end(); ++i) {
            if (k == idx)
                  return i->second;
            ++k;
            }
      return 0;
      }


