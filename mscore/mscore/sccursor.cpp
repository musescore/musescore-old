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

#include "sccursor.h"
#include "scscore.h"
#include "scnote.h"
#include "sctext.h"
#include "scmeasure.h"

#include "chordrest.h"
#include "chord.h"
#include "rest.h"
#include "note.h"
#include "stafftext.h"
#include "text.h"
#include "measure.h"
#include "repeatlist.h"

//---------------------------------------------------------
//   SCursor
//---------------------------------------------------------

SCursor::SCursor(Score* s)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = false;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      }

SCursor::SCursor(Score* s, bool expandRepeat)
      {
      _score    = s;
      _staffIdx = 0;
      _voice    = 0;
      _segment  = 0;
      _expandRepeat = expandRepeat;
      _curRepeatSegment = 0;
      _curRepeatSegmentIndex = 0;
      _score->updateRepeatList(expandRepeat);
      }

//---------------------------------------------------------
//   cr
//---------------------------------------------------------

ChordRest* SCursor::cr() const
      {
      if (_segment) {
            int track = _staffIdx * VOICES + _voice;
            Element* e = _segment->element(track);
            if (e && e->isChordRest())
                  return static_cast<ChordRest*>(e);
            }
      return 0;
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void SCursor::rewind()
      {
      _segment   = 0;
      Measure* m = _score->tick2measure(0);
      if(_expandRepeat && !_score->repeatList()->isEmpty()){
        _curRepeatSegment = _score->repeatList()->first();
        _curRepeatSegmentIndex = 0;
      }
      if (m) {
            _segment = m->first();
            if (_staffIdx >= 0) {
                  int track = _staffIdx * VOICES + _voice;
                  while (_segment && ((_segment->subtype() != Segment::SegChordRest) || (_segment->element(track) == 0)))
                        _segment = _segment->next1();
                  }
            }
      }

//---------------------------------------------------------
//   ScSCursorPropertyIterator
//---------------------------------------------------------

class ScSCursorPropertyIterator : public QScriptClassPropertyIterator
      {
      int m_index, m_last;

   public:
      ScSCursorPropertyIterator(const QScriptValue &object);
      ~ScSCursorPropertyIterator() {}
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
//   ScSCursor
//---------------------------------------------------------

ScSCursor::ScSCursor(QScriptEngine* engine)
   : QObject(engine), QScriptClass(engine)
      {
      qScriptRegisterMetaType<SCursor>(engine, toScriptValue, fromScriptValue);

      cursorStaff = engine->toStringHandle(QLatin1String("staff"));
      cursorVoice = engine->toStringHandle(QLatin1String("voice"));

      proto = engine->newQObject(new ScSCursorPrototype(this),
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

QScriptClass::QueryFlags ScSCursor::queryProperty(const QScriptValue &object,
   const QScriptString& name, QueryFlags flags, uint* /*id*/)
      {
      SCursor* sp = qscriptvalue_cast<SCursor*>(object.data());
      if (name == cursorStaff || name == cursorVoice)
            return flags;
      if (!sp)
            return 0;
      return 0;   // qscript handles property
      }

//---------------------------------------------------------
//   property
//---------------------------------------------------------

QScriptValue ScSCursor::property(const QScriptValue& object,
   const QScriptString& name, uint /*id*/)
      {
      SCursor* cursor = qscriptvalue_cast<SCursor*>(object.data());
      if (!cursor)
            return QScriptValue();
      if (name == cursorStaff)
            return QScriptValue(engine(), cursor->staffIdx());
      else if (name == cursorVoice)
            return QScriptValue(engine(), cursor->voice());
      return QScriptValue();
      }

//---------------------------------------------------------
//   setProperty
//---------------------------------------------------------

void ScSCursor::setProperty(QScriptValue &object,
   const QScriptString& name, uint /*id*/, const QScriptValue& value)
      {
      SCursor* cursor = qscriptvalue_cast<SCursor*>(object.data());
      if (!cursor)
            return;
      int val = value.toInt32();
      if (name == cursorStaff) {
            if (val < 0)
                  val = 0;
            else if (val >= cursor->score()->nstaves())
                  val = cursor->score()->nstaves() - 1;
            cursor->setStaffIdx(val);
            }
      else if (name == cursorVoice) {
            if (val < 0)
                  val = 0;
            else if (val >= VOICES)
                  val = VOICES - 1;
            cursor->setVoice(val);
            }
      }

//---------------------------------------------------------
//   propertyFlags
//---------------------------------------------------------

QScriptValue::PropertyFlags ScSCursor::propertyFlags(
   const QScriptValue &/*object*/, const QScriptString& /*name*/, uint /*id*/)
      {
      return QScriptValue::Undeletable;
      }

QScriptClassPropertyIterator* ScSCursor::newIterator(const QScriptValue &object)
      {
      return new ScSCursorPropertyIterator(object);
      }

//---------------------------------------------------------
//   newInstance
//---------------------------------------------------------

QScriptValue ScSCursor::newInstance(Score* score)
      {
      SCursor* cursor = new SCursor(score);
      return newInstance(*cursor);
      }


QScriptValue ScSCursor::newInstance(Score* score, bool expandRepeat)
      {
      SCursor* cursor = new SCursor(score, expandRepeat);
      return newInstance(*cursor);
      }

QScriptValue ScSCursor::newInstance(const SCursor& cursor)
      {
      QScriptValue data = engine()->newVariant(qVariantFromValue(cursor));
      return engine()->newObject(this, data);
      }

//---------------------------------------------------------
//   construct
//---------------------------------------------------------

QScriptValue ScSCursor::construct(QScriptContext *ctx, QScriptEngine *)
      {
      ScSCursor *cls = qscriptvalue_cast<ScSCursor*>(ctx->callee().data());
      if (!cls)
            return QScriptValue();
      QScriptValue v = ctx->argument(0);
      ScorePtr* sp = qscriptvalue_cast<ScorePtr*>(v.data());
      if (sp){
            bool expandRepeat = false;
            if(ctx->argumentCount () > 1){
              QScriptValue v1 = ctx->argument(1);
              if(v1.isBoolean()){
                expandRepeat = v1.toBoolean();
              }
            }
            return cls->newInstance(*sp, expandRepeat);
      }
      else
            return QScriptValue();
      }

QScriptValue ScSCursor::toScriptValue(QScriptEngine* eng, const SCursor& ba)
      {
      QScriptValue ctor = eng->globalObject().property("Cursor");
      ScSCursor* cls = qscriptvalue_cast<ScSCursor*>(ctor.data());
      if (!cls)
            return eng->newVariant(qVariantFromValue(ba));
      return cls->newInstance(ba);
      }

void ScSCursor::fromScriptValue(const QScriptValue& obj, SCursor& ba)
      {
      ba = qscriptvalue_cast<SCursor>(obj.data());
      }

//---------------------------------------------------------
//   ScSCursorPropertyIterator
//---------------------------------------------------------

ScSCursorPropertyIterator::ScSCursorPropertyIterator(const QScriptValue &object)
   : QScriptClassPropertyIterator(object)
      {
      toFront();
      }

bool ScSCursorPropertyIterator::hasNext() const
      {
//      SCursor* ba = qscriptvalue_cast<SCursor*>(object().data());
      return m_index < 1;     // TODO ba->size();
      }

void ScSCursorPropertyIterator::next()
      {
      m_last = m_index;
      ++m_index;
      }

bool ScSCursorPropertyIterator::hasPrevious() const
      {
      return (m_index > 0);
      }

void ScSCursorPropertyIterator::previous()
      {
      --m_index;
      m_last = m_index;
      }

void ScSCursorPropertyIterator::toFront()
      {
      m_index = 0;
      m_last = -1;
      }

void ScSCursorPropertyIterator::toBack()
      {
//      SCursor* ba = qscriptvalue_cast<SCursor*>(object().data());
      m_index = 0; // ba->size();
      m_last = -1;
      }

//---------------------------------------------------------
//   thisSCursor
//---------------------------------------------------------

SCursor* ScSCursorPrototype::thisSCursor() const
      {
      return qscriptvalue_cast<SCursor*>(thisObject().data());
      }

//---------------------------------------------------------
//   rewind
//    rewind cursor to start of score
//---------------------------------------------------------

void ScSCursorPrototype::rewind()
      {
      SCursor* cursor = thisSCursor();
      cursor->rewind();
      }

//---------------------------------------------------------
//   eos
//    return true if cursor is at end of score
//---------------------------------------------------------

bool ScSCursorPrototype::eos() const
      {
      SCursor* cursor = thisSCursor();
      return cursor->segment() == 0;
      }

//---------------------------------------------------------
//   chord
//    get chord at current position
//---------------------------------------------------------

ChordPtr ScSCursorPrototype::chord()
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr = cursor->cr();
      if (cr == 0 || cr->type() != CHORD)
            return 0;
      return static_cast<ChordPtr>(cr);
      }

//---------------------------------------------------------
//   rest
//    get rest at current position
//---------------------------------------------------------

RestPtr ScSCursorPrototype::rest()
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr = cursor->cr();
      if (cr == 0 || cr->type() != REST)
            return 0;
      return static_cast<RestPtr>(cr);
      }

//---------------------------------------------------------
//   measure
//    get measure at current position
//---------------------------------------------------------

MeasurePtr ScSCursorPrototype::measure()
      {
      SCursor* cursor = thisSCursor();
      Measure* m = cursor->cr()->measure();
      if (m == 0)
            return 0;
      return m;
      }

//---------------------------------------------------------
//   isChord
//---------------------------------------------------------

bool ScSCursorPrototype::isChord() const
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr = cursor->cr();
      return cr && (cr->type() == CHORD);
      }

//---------------------------------------------------------
//   isChord
//---------------------------------------------------------

bool ScSCursorPrototype::isRest() const
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr = cursor->cr();
      return cr && (cr->type() == REST);
      }

//---------------------------------------------------------
//   next
//    go to next segment
//    return false if end of score is reached
//---------------------------------------------------------

bool ScSCursorPrototype::next()
      {
      SCursor* cursor = thisSCursor();
      Segment* seg = cursor->segment();;
      seg = seg->next1();
      RepeatSegment* rs = cursor->repeatSegment();
      if(rs && cursor->expandRepeat()){
            Score* score = cursor->score();
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((seg  && (seg->tick() >= endTick) ) || (!seg) ){
                int rsIdx = cursor->repeatSegmentIndex();
                rsIdx ++;
                if (rsIdx < score->repeatList()->size()){ //there is a next repeat segment
                    rs = score->repeatList()->at(rsIdx);
                    cursor->setRepeatSegment(rs);
                    cursor->setRepeatSegmentIndex(rsIdx);
                    Measure* m = score->tick2measure(rs->tick);
                    if(m)
                      seg = m->first();
                    else
                      seg = 0;
                }else{
                    seg = 0;
                }
            }
      }

      int staffIdx = cursor->staffIdx();
      if (staffIdx >= 0) {
            int track = staffIdx * VOICES + cursor->voice();
            while (seg && ((seg->subtype() != Segment::SegChordRest) || (seg->element(track) == 0)))
                  seg = seg->next1();
            }
      cursor->setSegment(seg);
      return seg != 0;
      }

//---------------------------------------------------------
//   nextMeasure
//    go to first segment of next measure
//    return false if end of score is reached
//---------------------------------------------------------

bool ScSCursorPrototype::nextMeasure()
{
      SCursor* cursor = thisSCursor();
      Measure* m = cursor->segment()->measure();
      m = m->nextMeasure();
      RepeatSegment* rs = cursor->repeatSegment();
      if(rs && cursor->expandRepeat()){
            Score* score = cursor->score();
            int startTick  = rs->tick;
            int endTick    = startTick + rs->len;
            if ((m  && (m->tick() + m->tickLen() > endTick) ) || (!m) ){
                int rsIdx = cursor->repeatSegmentIndex();
                rsIdx ++;
                if (rsIdx < score->repeatList()->size()){ //there is a next repeat segment
                    rs = score->repeatList()->at(rsIdx);
                    cursor->setRepeatSegment(rs);
                    cursor->setRepeatSegmentIndex(rsIdx);
                    m = score->tick2measure(rs->tick);
                }else{
                    m = 0;
                }
            }
      }

      if (m == 0) {
            cursor->setSegment(0);
            return false;
      }
      Segment* seg = m->first();
      int staffIdx = cursor->staffIdx();
      if (staffIdx >= 0) {
            int track = staffIdx * VOICES + cursor->voice();
            while (seg && ((seg->subtype() != Segment::SegChordRest) || (seg->element(track) == 0)))
                  seg = seg->next1();
            }
      cursor->setSegment(seg);
      return seg != 0;

}

//---------------------------------------------------------
//   putStaffText
//---------------------------------------------------------

void ScSCursorPrototype::putStaffText(TextPtr s)
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr = cursor->cr();
      if (!cr || !s)
            return;
      QFont f = s->defaultFont();
      s->setTrack(cr->track());
      s->setSystemFlag(false);
      s->setSubtype(TEXT_STAFF);
      s->setParent(cr->measure());
      s->setTick(cr->tick());
      s->score()->undoAddElement(s);
      s->score()->setLayoutAll(true);
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ScSCursorPrototype::add(ChordRest* c)
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr   = cursor->cr();
      int tick        = cr->tick();
      int staffIdx    = cursor->staffIdx();
      int voice       = cursor->voice();
      Score* score    = cursor->score();

      Fraction len(c->duration().fraction());
      int track       = staffIdx * VOICES + voice;
      Fraction gap    = score->makeGap(cr, len, cr->tuplet());
      if (gap < len) {
            printf("cannot make gap\n");
            return;
            }
      Measure* measure = score->tick2measure(tick);
      Segment::SegmentType st = Segment::SegChordRest;
      Segment* seg = measure->findSegment(st, tick);
      if (seg == 0) {
            seg = measure->createSegment(st, tick);
            score->undoAddElement(seg);
            }
      c->setScore(score);
      if (c->type() == CHORD) {
            Chord* chord = static_cast<Chord*>(c);
            NoteList* nl = chord->noteList();
            for (iNote in = nl->begin(); in != nl->end(); ++in)
                  in->second->setScore(score);
            }

      cursor->setSegment(seg);
      c->setParent(seg);
      c->setTrack(track);
      score->undoAddElement(c);
      }

//---------------------------------------------------------
//   tick
//---------------------------------------------------------

int ScSCursorPrototype::tick()
      {
      SCursor* cursor = thisSCursor();
      ChordRest* cr   = cursor->cr();
      int offset = 0;
      RepeatSegment* rs = cursor->repeatSegment();
      if (rs && cursor->expandRepeat())
            offset = rs->utick - rs->tick;
      return cr->tick() + offset;
      }

//---------------------------------------------------------
//   time
//---------------------------------------------------------

double ScSCursorPrototype::time()
      {
      int tick = this->tick();
      SCursor* cursor = thisSCursor();
      return cursor->score()->utick2utime(tick)*1000;
      }
