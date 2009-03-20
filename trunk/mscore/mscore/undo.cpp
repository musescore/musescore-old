//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: undo.cpp,v 1.40 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

/**
 \file
 Implementation of undo functions.

 The undo system requires calling startUndo() when starting a GUI command
 and calling endUndo() when ending the command. All changes to a score
 in response to a GUI command must be undoable/redoable by executing
 a sequence of low-level undo operations. This sequence is built by the code
 handling the command, by calling one or more undoOp()'s
 between startUndo() and endUndo().
*/

#include "undo.h"
#include "element.h"
#include "note.h"
#include "score.h"
#include "segment.h"
#include "measure.h"
#include "system.h"
#include "select.h"
#include "input.h"
#include "slur.h"
#include "clef.h"
#include "staff.h"
#include "layout.h"
#include "chord.h"
#include "sig.h"
#include "key.h"
#include "mscore.h"
#include "canvas.h"
#include "barline.h"
#include "volta.h"
#include "tuplet.h"
#include "harmony.h"
#include "pitchspelling.h"
#include "part.h"
#include "beam.h"
#include "dynamics.h"

extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoCommand::undo()
      {
      int n = childList.size();
      for (int i = n-1; i >= 0; --i)
            childList[i]->undo();
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoCommand::redo()
      {
      int n = childList.size();
      for (int i = 0; i < n; ++i)
            childList[i]->redo();
      }

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::UndoStack()
      {
      group    = 0;
      curCmd   = 0;
      curIdx   = 0;
      cleanIdx = 0;
      }

//---------------------------------------------------------
//   UndoStack
//---------------------------------------------------------

UndoStack::~UndoStack()
      {
      if (group)
            group->removeStack(this);
      }

//---------------------------------------------------------
//   beginMacro
//---------------------------------------------------------

void UndoStack::beginMacro()
      {
      if (curCmd) {
            printf("UndoStack:beginMacro(): alread active\n");
            return;
            }
      curCmd = new UndoCommand();
      }

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
      {
      if (curCmd == 0) {
            printf("UndoStack:endMacro(): not active\n");
            return;
            }
      if (rollback) {
            delete curCmd;
            curCmd = 0;
            return;
            }
      bool a = isClean();
      if (list.size() > curIdx)
            emit canRedoChanged(false);
      if (curIdx == 0)
            emit canUndoChanged(true);
      while (list.size() > curIdx) {
            UndoCommand* cmd = list.takeLast();
            delete cmd;
            }
      list.append(curCmd);
      curCmd = 0;
      ++curIdx;
      if (a != isClean())
            emit cleanChanged(!a);
      }

//---------------------------------------------------------
//   push
//---------------------------------------------------------

void UndoStack::push(UndoCommand* cmd)
      {
      if (!curCmd) {
            printf("UndoStack:push(): no active command\n");
            return;
            }
      curCmd->appendChild(cmd);
      cmd->redo();
      }

//---------------------------------------------------------
//   setClean
//---------------------------------------------------------

void UndoStack::setClean()
      {
      if (cleanIdx != curIdx) {
            cleanIdx = curIdx;
            emit cleanChanged(true);
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoStack::undo()
      {
      if (curIdx) {
            bool a = true;
            bool b = canRedo();
            bool c = isClean();
            --curIdx;
            list[curIdx]->undo();
            if (a != canUndo())
                  emit canUndoChanged(!a);
            if (b != canRedo())
                  emit canRedoChanged(!b);
            if (c != isClean())
                  emit cleanChanged(!c);
            }
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoStack::redo()
      {
      if (canRedo()) {
            bool a = canUndo();
            bool b = true;
            bool c = isClean();
            list[curIdx++]->redo();
            if (a != canUndo())
                  emit canUndoChanged(!a);
            if (b != canRedo())
                  emit canRedoChanged(!b);
            if (c != isClean())
                  emit cleanChanged(!c);
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void UndoGroup::undo()
      {
      if (_activeStack)
            _activeStack->undo();
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void UndoGroup::redo()
      {
      if (_activeStack)
            _activeStack->redo();
      }

//---------------------------------------------------------
//   UndoGroup
//---------------------------------------------------------

UndoGroup::UndoGroup()
      {
      _activeStack = 0;
      }

//---------------------------------------------------------
//   addStack
//---------------------------------------------------------

void UndoGroup::addStack(UndoStack* stack)
      {
      stack->setGroup(this);
      group.append(stack);
      connect(stack, SIGNAL(canUndoChanged(bool)), this, SIGNAL(canUndoChanged(bool)));
      connect(stack, SIGNAL(canRedoChanged(bool)), this, SIGNAL(canRedoChanged(bool)));
      connect(stack, SIGNAL(cleanChanged(bool)), this, SIGNAL(cleanChanged(bool)));
      }

//---------------------------------------------------------
//   removeStack
//---------------------------------------------------------

void UndoGroup::removeStack(UndoStack* stack)
      {
      bool a = canUndo();
      bool b = canRedo();
      group.removeOne(stack);
      if (stack == _activeStack)
            _activeStack = 0;
      if (a)
            emit canUndoChanged(!a);
      if (b)
            emit canRedoChanged(!b);
      }

//---------------------------------------------------------
//   setActiveStack
//---------------------------------------------------------

void UndoGroup::setActiveStack(UndoStack* stack)
      {
      bool a = canUndo();
      bool b = canRedo();
      bool c = isClean();
      _activeStack = stack;
      if (a != canUndo())
            emit canUndoChanged(!a);
      if (b != canRedo())
            emit canRedoChanged(!b);
      if (c != isClean())
            emit cleanChanged(!c);
      }

//---------------------------------------------------------
//   canUndo
//---------------------------------------------------------

bool UndoGroup::canUndo() const
      {
      return _activeStack ? _activeStack->canUndo() : false;
      }

//---------------------------------------------------------
//   canRedo
//---------------------------------------------------------

bool UndoGroup::canRedo() const
      {
      return _activeStack ? _activeStack->canRedo() : false;
      }

//---------------------------------------------------------
//   isClean
//---------------------------------------------------------

bool UndoGroup::isClean() const
      {
      return _activeStack ? _activeStack->isClean() : false;
      }

//---------------------------------------------------------
//   endUndoRedo
//---------------------------------------------------------

/**
 Common handling for ending undo or redo
*/

void Score::endUndoRedo()
      {
      emit posChanged(_is.pos());

      if (!noteEntryMode()) {
            // no input state
            canvas()->setState(Canvas::NORMAL);
            setState(STATE_NORMAL);
            }
      else {
            // input state
            canvas()->setState(Canvas::NOTE_ENTRY);
            setState(STATE_NOTE_ENTRY);
            }
      sel->update();
      layoutAll = true;
      end();
      }

//---------------------------------------------------------
//   SaveState
//---------------------------------------------------------

SaveState::SaveState(Score* s)
      {
      score     = s;
      selection = 0;
      }

SaveState::~SaveState()
      {
      if (selection)
            delete selection;
      }

void SaveState::undo()
      {
      score->setInputState(inputState);
      score->setSelection(new Selection(*selection));
      }

void SaveState::redo()
      {
      inputState = score->inputState();
      if (!selection)
            selection = new Selection(*score->selection());
      else
            *selection = *(score->selection());
      }

//---------------------------------------------------------
//   undoInsertTime
//---------------------------------------------------------

void Score::undoInsertTime(int tick, int len)
      {
      _undo->push(new InsertTime(this, tick, len));
      }

//---------------------------------------------------------
//   undoFixTicks
//---------------------------------------------------------

void Score::undoFixTicks()
      {
      _undo->push(new FixTicks(this));
      }

//---------------------------------------------------------
//   undoChangeMeasureLen
//---------------------------------------------------------

void Score::undoChangeMeasureLen(Measure* m, int oldTicks, int newTicks)
      {
      _undo->push(new ChangeMeasureLen(m, oldTicks, newTicks));
      }

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(Element* oldElement, Element* newElement)
      {
      _undo->push(new ChangeElement(oldElement, newElement));
      }

//---------------------------------------------------------
//   undoChangeSubtype
//---------------------------------------------------------

void Score::undoChangeSubtype(Element* element, int st)
      {
      _undo->push(new ChangeSubtype(element, st));
      }

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch, int tpc, int userAccidental)
      {
      _undo->push(new ChangePitch(note, pitch, tpc, userAccidental));
      }

//---------------------------------------------------------
//   undoChangeTpc
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int tpc)
      {
      _undo->push(new ChangeTpc(note, tpc));
      }

//---------------------------------------------------------
//   undoChangeBeamMode
//---------------------------------------------------------

void Score::undoChangeBeamMode(ChordRest* cr, BeamMode mode)
      {
      _undo->push(new ChangeBeamMode(cr, mode));
      }

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, int len)
      {
      _undo->push(new ChangeChordRestLen(cr, len));
      }

//---------------------------------------------------------
//   undoChangeEndBarLineType
//---------------------------------------------------------

void Score::undoChangeEndBarLineType(Measure* m, int subtype)
      {
      _undo->push(new ChangeEndBarLineType(m, subtype));
      }

//---------------------------------------------------------
//   undoChangeBarLineSpan
//---------------------------------------------------------

void Score::undoChangeBarLineSpan(Staff* staff, int span)
      {
      _undo->push(new ChangeBarLineSpan(staff, span));
      }

//---------------------------------------------------------
//   undoChangeUserOffset
//---------------------------------------------------------

void Score::undoChangeUserOffset(Element* e, const QPointF& offset)
      {
      _undo->push(new ChangeUserOffset(e, offset));
      }

//---------------------------------------------------------
//   undoChangeVelocity
//---------------------------------------------------------

void Score::undoChangeVelocity(Dynamic* e, int velocity)
      {
      _undo->push(new ChangeVelocity(e, velocity));
      }

//---------------------------------------------------------
//   undoChangeCopyright
//---------------------------------------------------------

void Score::undoChangeCopyright(const QString& s)
      {
      _undo->push(new ChangeCopyright(this, s));
      }

//---------------------------------------------------------
//   undoTransposeHarmony
//---------------------------------------------------------

void Score::undoTransposeHarmony(Harmony* h, int semitones)
      {
      _undo->push(new TransposeHarmony(h, semitones));
      }

//---------------------------------------------------------
//   undoExchangeVoice
//---------------------------------------------------------

void Score::undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2)
      {
      _undo->push(new ExchangeVoice(measure, val1, val2, staff1, staff2));
      }

//---------------------------------------------------------
//   undoRemovePart
//---------------------------------------------------------

void Score::undoRemovePart(Part* part, int idx)
      {
      _undo->push(new RemovePart(part, idx));
      }

//---------------------------------------------------------
//   undoInsertPart
//---------------------------------------------------------

void Score::undoInsertPart(Part* part, int idx)
      {
      _undo->push(new InsertPart(part, idx));
      }

//---------------------------------------------------------
//   undoInsertMeasure
//---------------------------------------------------------

void Score::undoInsertMeasure(MeasureBase* m)
      {
      _undo->push(new InsertMeasure(m));
      }

//---------------------------------------------------------
//   undoRemoveStaff
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff, int idx)
      {
      _undo->push(new RemoveStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoInsertStaff
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, int idx)
      {
      _undo->push(new InsertStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoMove
//---------------------------------------------------------

void Score::undoMove(Element* e, const QPointF& pt)
      {
      _undo->push(new MoveElement(e, pt));
      }

//---------------------------------------------------------
//   undoChangeSig
//---------------------------------------------------------

void Score::undoChangeSig(int tick, const SigEvent& o, const SigEvent& n)
      {
      _undo->push(new ChangeSig(this, tick, o, n));
      }

//---------------------------------------------------------
//   undoSigInsertTime
//---------------------------------------------------------

void Score::undoSigInsertTime(int tick, int len)
      {
      _undo->push(new SigInsertTime(this, tick, len));
      }

//---------------------------------------------------------
//   undoChangeTempo
//---------------------------------------------------------

void Score::undoChangeTempo(int tick, const TEvent& o, const TEvent& n)
      {
      _undo->push(new ChangeTempo(this, tick, o, n));
      }

//---------------------------------------------------------
//   undoChangeKey
//---------------------------------------------------------

void Score::undoChangeKey(Staff* staff, int tick, int o, int n)
      {
      _undo->push(new ChangeKey(staff, tick, o, n));
      }

//---------------------------------------------------------
//   undoChangeClef
//---------------------------------------------------------

void Score::undoChangeClef(Staff* staff, int tick, int o, int n)
      {
      _undo->push(new ChangeClef(staff, tick, o, n));
      }

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* staff, int tick, int o, int n)
      {
      _undo->push(new ChangeKeySig(staff, tick, o, n));
      }

//---------------------------------------------------------
//   undoChangeRepeatFlags
//---------------------------------------------------------

void Score::undoChangeRepeatFlags(Measure* m, int flags)
      {
      _undo->push(new ChangeRepeatFlags(m, flags));
      }

//---------------------------------------------------------
//   undoChangeVoltaEnding
//---------------------------------------------------------

void Score::undoChangeVoltaEnding(Volta* volta, const QList<int>& l)
      {
      _undo->push(new ChangeVoltaEnding(volta, l));
      }

//---------------------------------------------------------
//   undoChangeVoltaText
//---------------------------------------------------------

void Score::undoChangeVoltaText(Volta* volta, const QString& s)
      {
      _undo->push(new ChangeVoltaText(volta, s));
      }

//---------------------------------------------------------
//   undoChangeChordRestSize
//---------------------------------------------------------

void Score::undoChangeChordRestSize(ChordRest* cr, bool small)
      {
      _undo->push(new ChangeChordRestSize(cr, small));
      }

//---------------------------------------------------------
//   undoChangeChordNoStem
//---------------------------------------------------------

void Score::undoChangeChordNoStem(Chord* cr, bool noStem)
      {
      _undo->push(new ChangeChordNoStem(cr, noStem));
      }

//---------------------------------------------------------
//   undoChangeChordRestSpace
//---------------------------------------------------------

void Score::undoChangeChordRestSpace(ChordRest* cr, Spatium l, Spatium t)
      {
      _undo->push(new ChangeChordRestSpace(cr, l, t));
      }

//---------------------------------------------------------
//   undoChangeBracketSpan
//---------------------------------------------------------

void Score::undoChangeBracketSpan(Staff* staff, int column, int span)
      {
      _undo->push(new ChangeBracketSpan(staff, column, span));
      }

//---------------------------------------------------------
//   undoChangeAccidental
//---------------------------------------------------------

void Score::undoChangeAccidental(Note* note, int accidental)
      {
      _undo->push(new ChangeAccidental(note, accidental));
      }

//---------------------------------------------------------
//   undoToggleInvisible
//---------------------------------------------------------

void Score::undoToggleInvisible(Element* e)
      {
      _undo->push(new ToggleInvisible(e));
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(Element* element)
      {
      element->setScore(this);
      _undo->push(new AddElement(element));
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      _undo->push(new RemoveElement(element));
      }

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(Element* e)
      {
      element = e;
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo()
      {
      element->score()->removeElement(element);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo()
      {
      element->score()->addElement(element);
      }

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(Element* e)
      {
      element = e;

      if (element->isChordRest()) {
            Score* score = element->score();
            // remove any slurs pointing to this chor/rest
            ChordRest* cr = (ChordRest*)element;
            foreach(Slur* slur, cr->slurFor()) {
                  score->undoRemoveElement(slur);
                  }
            foreach(Slur* slur, cr->slurBack()) {
                  score->undoRemoveElement(slur);
                  }
            if (cr->tuplet() && cr->tuplet()->elements().empty())
                  score->undoRemoveElement(cr->tuplet());
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo()
      {
      element->score()->addElement(element);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo()
      {
      element->score()->removeElement(element);
      }

//---------------------------------------------------------
//   ChangeNoteHead
//---------------------------------------------------------

ChangeNoteHead::ChangeNoteHead(Note* n, int g)
   : UndoCommand(), note(n), group(g)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeNoteHead::flip()
      {
      int headGroup = note->headGroup();
      QRectF r = note->abbox();
      note->setHeadGroup(group);
      r |= note->abbox();
      group = headGroup;
      note->score()->addRefresh(r);
      note->score()->end();
      }

//---------------------------------------------------------
//   ChangeConcertPitch
//---------------------------------------------------------

ChangeConcertPitch::ChangeConcertPitch(Score* s, bool v)
      {
      score = s;
      val   = v;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeConcertPitch::flip()
      {
      int oval = int(score->styleB(ST_concertPitch));
      score->setStyle(ST_concertPitch, StyleVal(val));
      QAction* action = getAction("concert-pitch");
      action->setChecked(score->styleB(ST_concertPitch));
      val = oval;
      }

//---------------------------------------------------------
//   InsertPart
//---------------------------------------------------------

InsertPart::InsertPart(Part* p, int i)
      {
      part = p;
      idx  = i;
      }

void InsertPart::undo()
      {
      part->score()->removePart(part);
      }

void InsertPart::redo()
      {
      part->score()->insertPart(part, idx);
      }

//---------------------------------------------------------
//   RemovePart
//---------------------------------------------------------

RemovePart::RemovePart(Part* p, int i)
      {
      part = p;
      idx  = i;
      }

void RemovePart::undo()
      {
      part->score()->insertPart(part, idx);
      }

void RemovePart::redo()
      {
      part->score()->removePart(part);
      }

//---------------------------------------------------------
//   InsertStaff
//---------------------------------------------------------

InsertStaff::InsertStaff(Staff* p, int i)
      {
      staff = p;
      idx  = i;
      }

void InsertStaff::undo()
      {
      staff->score()->removeStaff(staff);
      }

void InsertStaff::redo()
      {
      staff->score()->insertStaff(staff, idx);
      }

//---------------------------------------------------------
//   RemoveStaff
//---------------------------------------------------------

RemoveStaff::RemoveStaff(Staff* p, int i)
      {
      staff = p;
      idx  = i;
      }

void RemoveStaff::undo()
      {
      staff->score()->insertStaff(staff, idx);
      }

void RemoveStaff::redo()
      {
      staff->score()->removeStaff(staff);
      }

//---------------------------------------------------------
//   InsertMStaff
//---------------------------------------------------------

InsertMStaff::InsertMStaff(Measure* m, MStaff* ms, int i)
      {
      measure = m;
      mstaff  = ms;
      idx     = i;
      }

void InsertMStaff::undo()
      {
      measure->removeMStaff(mstaff, idx);
      }

void InsertMStaff::redo()
      {
      measure->insertMStaff(mstaff, idx);
      }

//---------------------------------------------------------
//   RemoveMStaff
//---------------------------------------------------------

RemoveMStaff::RemoveMStaff(Measure* m, MStaff* ms, int i)
      {
      measure = m;
      mstaff  = ms;
      idx     = i;
      }

void RemoveMStaff::undo()
      {
      measure->insertMStaff(mstaff, idx);
      }

void RemoveMStaff::redo()
      {
      measure->removeMStaff(mstaff, idx);
      }

//---------------------------------------------------------
//   InsertMeasure
//---------------------------------------------------------

InsertMeasure::InsertMeasure(MeasureBase* m)
      {
      measure = m;
      }

void InsertMeasure::undo()
      {
      measure->score()->removeMeasure(measure);
      }

void InsertMeasure::redo()
      {
      measure->score()->addMeasure(measure);
      }

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

SortStaves::SortStaves(Score* s, QList<int> l)
      {
      score = s;
      list  = l;
      }

void SortStaves::flip()
      {
      score->sortStaves(list);
      }

//---------------------------------------------------------
//   ToggleInvisible
//---------------------------------------------------------

ToggleInvisible::ToggleInvisible(Element* e)
      {
      element = e;
      }

void ToggleInvisible::flip()
      {
      element->setVisible(!element->visible());
      }

//---------------------------------------------------------
//   ChangeColor
//---------------------------------------------------------

ChangeColor::ChangeColor(Element* e, QColor c)
      {
      element = e;
      color   = c;
      }

void ChangeColor::flip()
      {
      QColor c = element->color();
      element->setColor(color);
      color = c;
      }

//---------------------------------------------------------
//   ChangePitch
//---------------------------------------------------------

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc, int _userAccidental)
      {
      note  = _note;
      pitch = _pitch;
      tpc   = _tpc;
      userAccidental = _userAccidental;
      }

void ChangePitch::flip()
      {
      int f_pitch  = note->pitch();
      int f_tpc    = note->tpc();
      int f_userAcc = note->userAccidental();

      note->changePitch(pitch);
      note->setTpc(tpc);
      note->setUserAccidental(userAccidental);

      pitch          = f_pitch;
      tpc            = f_tpc;
      userAccidental = f_userAcc;
      }

//---------------------------------------------------------
//   ChangeTpc
//---------------------------------------------------------

ChangeTpc::ChangeTpc(Note* _note, int _tpc)
      {
      note = _note;
      tpc  = _tpc;
      }

void ChangeTpc::flip()
      {
      int ntpc = note->tpc();
      note->setTpc(tpc);
      tpc = ntpc;
      }

//---------------------------------------------------------
//   ChangeAccidental
//---------------------------------------------------------

ChangeAccidental::ChangeAccidental(Note* _note, int _acc)
      {
      note  = _note;
      acc   = _acc;
      }

void ChangeAccidental::redo()
      {
      pitch = note->pitch();
      tpc   = note->tpc();
      int a = note->accidentalSubtype();
      note->changeAccidental(acc);
      acc   = a;
      }

void ChangeAccidental::undo()
      {
      int a  = note->pitch();
      int b  = note->tpc();
      int c  = note->accidentalSubtype();

      note->setPitch(pitch);
      note->setTpc(tpc);
      note->setAccidentalSubtype(acc);

      pitch = a;
      tpc   = b;
      acc   = c;
      }

//---------------------------------------------------------
//   SetStemDirection
//---------------------------------------------------------

SetStemDirection::SetStemDirection(Chord* c, Direction d)
      {
      chord     = c;
      direction = d;
      }

void SetStemDirection::flip()
      {
      Direction dir = chord->stemDirection();
      chord->setStemDirection(direction);
      direction = dir;
      }

//---------------------------------------------------------
//   FlipSlurDirection
//---------------------------------------------------------

FlipSlurDirection::FlipSlurDirection(SlurTie* s)
      {
      slur = s;
      }

void FlipSlurDirection::flip()
      {
      slur->setSlurDirection(slur->isUp() ? DOWN : UP);
      }

//---------------------------------------------------------
//   FlipBeamDirection
//---------------------------------------------------------

FlipBeamDirection::FlipBeamDirection(Beam* b)
      {
      beam = b;
      }

void FlipBeamDirection::flip()
      {
      beam->setBeamDirection(beam->isUp() ? DOWN : UP);
      }

//---------------------------------------------------------
//   ChangeSubtype
//---------------------------------------------------------

ChangeSubtype::ChangeSubtype(Element* e, int st)
      {
      element = e;
      subtype = st;
      }

void ChangeSubtype::flip()
      {
      int st = element->subtype();
      element->setSubtype(subtype);
      subtype = st;
      }

//---------------------------------------------------------
//   ChangeElement
//---------------------------------------------------------

ChangeElement::ChangeElement(Element* oe, Element* ne)
      {
      oldElement = oe;
      newElement = ne;
      }

void ChangeElement::flip()
      {
      oldElement->parent()->change(oldElement, newElement);
      // swap
      Element* e = oldElement;
      oldElement = newElement;
      newElement = e;
      }

//---------------------------------------------------------
//   InsertStaves
//---------------------------------------------------------

InsertStaves::InsertStaves(Measure* m, int _a, int _b)
      {
      measure = m;
      a       = _a;
      b       = _b;
      }

void InsertStaves::undo()
      {
      measure->removeStaves(a, b);
      }

void InsertStaves::redo()
      {
      measure->insertStaves(a, b);
      }

//---------------------------------------------------------
//   RemoveStaves
//---------------------------------------------------------

RemoveStaves::RemoveStaves(Measure* m, int _a, int _b)
      {
      measure = m;
      a       = _a;
      b       = _b;
      }

void RemoveStaves::undo()
      {
      measure->insertStaves(a, b);
      }

void RemoveStaves::redo()
      {
      measure->removeStaves(a, b);
      }

//---------------------------------------------------------
//   ChangeKeySig
//---------------------------------------------------------

ChangeKeySig::ChangeKeySig(Staff* s, int t, int oks, int nks)
      {
      staff     = s;
      tick      = t;
      oldKeySig = oks;
      newKeySig = nks;
      }

void ChangeKeySig::undo()
      {
      KeyList* kl = staff->keymap();
      // remove new value if there is any
      if (newKeySig != NO_KEY) {
            iKeyEvent ik = kl->find(tick);
            kl->erase(ik);
            }
      if (oldKeySig != NO_KEY)
            (*kl)[tick] = oldKeySig;
      }

void ChangeKeySig::redo()
      {
      KeyList* kl = staff->keymap();
      if (oldKeySig != NO_KEY) {
            iKeyEvent ik = kl->find(tick);
            kl->erase(ik);
            }
      if (newKeySig != NO_KEY)
            (*kl)[tick] = newKeySig;
      }

//---------------------------------------------------------
//   ChangeClef
//---------------------------------------------------------

ChangeClef::ChangeClef(Staff* s, int _tick, int _o, int _n)
      {
      staff = s;
      tick  = _tick;
      o     = _o;
      n     = _n;
      }

void ChangeClef::undo()
      {
      ClefList* kl = staff->clefList();
      // remove new value if there is any
      if (n != NO_CLEF) {
            iClefEvent ik = kl->find(tick);
            kl->erase(ik);
            }
      if (o != NO_CLEF)
            (*kl)[tick] = o;
      }

void ChangeClef::redo()
      {
      ClefList* kl = staff->clefList();
      if (o != NO_CLEF) {
            iClefEvent ik = kl->find(tick);
            kl->erase(ik);
            }
      if (n != NO_CLEF)
            (*kl)[tick] = n;
      }

//---------------------------------------------------------
//   ChangeKey
//---------------------------------------------------------

ChangeKey::ChangeKey(Staff* s, int _tick, int _o, int _n)
      {
      staff = s;
      tick  = _tick;
      o     = _o;
      n     = _n;
      }

void ChangeKey::undo()
      {
      KeyList* kl = staff->keymap();
      // remove new value if there is any
      if (n != NO_KEY) {
            iClefEvent ik = kl->find(tick);
            if (ik == kl->end())
                  printf("UndoOp::ChangeKey1 %d: not found\n", tick);
            else
                  kl->erase(ik);
            }
      if (o != NO_KEY)
            (*kl)[tick] = o;
      }

void ChangeKey::redo()
      {
      KeyList* kl = staff->keymap();
      if (o != NO_KEY) {
            iKeyEvent ik = kl->find(tick);
            if (ik == kl->end())
                  printf("UndoOp::ChangeKey2 %d: not found\n", tick);
            else
                  kl->erase(ik);
            }
      if (n != NO_KEY)
            (*kl)[tick] = n;
      }

//---------------------------------------------------------
//   ChangeSig
//---------------------------------------------------------

ChangeSig::ChangeSig(Score* s, int _tick, const SigEvent& _o, const SigEvent& _n)
      {
      score = s;
      tick  = _tick;
      o     = _o;
      n     = _n;
      }

void ChangeSig::undo()
      {
      SigList* sigmap = score->getSigmap();
      if (n.valid())
            sigmap->del(tick);
      if (o.valid())
            sigmap->add(tick, o);
      }

void ChangeSig::redo()
      {
      SigList* sigmap = score->getSigmap();
      if (o.valid())
            sigmap->del(tick);
      if (n.valid())
            sigmap->add(tick, n);
      }

//---------------------------------------------------------
//   FixTicks
//---------------------------------------------------------

void FixTicks::flip()
      {
      score->fixTicks();
      }

//---------------------------------------------------------
//   ChangeTempo
//---------------------------------------------------------

ChangeTempo::ChangeTempo(Score* s, int t, const TEvent& _o, const TEvent& _n)
      {
      score = s;
      tick  = t;
      o     = _o;
      n     = _n;
      }

void ChangeTempo::undo()
      {
      TempoList* tempomap = score->getTempomap();
      if (n.valid())
            tempomap->delTempo(tick);
      if (o.valid())
            tempomap->addTempo(tick, o);
      }

void ChangeTempo::redo()
      {
      TempoList* tempomap = score->getTempomap();
      if (o.valid())
            tempomap->delTempo(tick);
      if (n.valid())
            tempomap->addTempo(tick, n);
      }

//---------------------------------------------------------
//   ChangeMeasureLen
//---------------------------------------------------------

ChangeMeasureLen::ChangeMeasureLen(Measure* m, int ot, int nt)
      {
      measure  = m;
      oldTicks = ot;
      newTicks = nt;
      }

void ChangeMeasureLen::flip()
      {
      int ol = newTicks;
      int nl = oldTicks;

      //
      // move EndBarLine and TimeSigAnnounce
      // to end of measure:
      //
      int staves = measure->score()->nstaves();
      int endTick = measure->tick() + nl;
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->subtype() != Segment::SegEndBarLine
               && segment->subtype() != Segment::SegTimeSigAnnounce)
                  continue;
            segment->setTick(endTick);
            for (int track = 0; track < staves*VOICES; ++track) {
                  if (segment->element(track))
                        segment->element(track)->setTick(endTick);
                  }
            }
      oldTicks = ol;
      newTicks = nl;
      }

//---------------------------------------------------------
//   InsertTime
//---------------------------------------------------------

InsertTime::InsertTime(Score* s, int t, int l)
      {
      score = s;
      tick  = t;
      len   = l;
      }

void InsertTime::flip()
      {
      score->insertTime(tick, len);
      len = -len;
      }

//---------------------------------------------------------
//   SigInsertTime
//---------------------------------------------------------

SigInsertTime::SigInsertTime(Score* s, int t, int l)
      {
      score = s;
      tick  = t;
      len   = l;
      }

void SigInsertTime::flip()
      {
      SigList* sigmap = score->getSigmap();
      if (len < 0)
            sigmap->removeTime(tick, -len);
      else
            sigmap->insertTime(tick, len);
      len = -len;
      }

//---------------------------------------------------------
//   ChangeRepeatFlags
//---------------------------------------------------------

ChangeRepeatFlags::ChangeRepeatFlags(Measure* m, int f)
      {
      measure = m;
      flags   = f;
      }

void ChangeRepeatFlags::flip()
      {
      int tmp = measure->repeatFlags();
      measure->setRepeatFlags(flags);
      flags = tmp;
      }

//---------------------------------------------------------
//   ChangeVoltaEnding
//---------------------------------------------------------

ChangeVoltaEnding::ChangeVoltaEnding(Volta* v, const QList<int>& l)
      {
      volta = v;
      list  = l;
      }

void ChangeVoltaEnding::flip()
      {
      QList<int> l = volta->endings();
      volta->setEndings(list);
      list = l;
      }

//---------------------------------------------------------
//   ChangeVoltaText
//---------------------------------------------------------

ChangeVoltaText::ChangeVoltaText(Volta* v, const QString& t)
      {
      volta = v;
      text  = t;
      }

void ChangeVoltaText::flip()
      {
      QString s = volta->text();
      volta->setText(text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeChordRestSize
//---------------------------------------------------------

ChangeChordRestSize::ChangeChordRestSize(ChordRest* _cr, bool _small)
      {
      cr = _cr;
      small = _small;
      }

void ChangeChordRestSize::flip()
      {
      bool s = cr->small();
      cr->setSmall(small);
      small = s;
      }

//---------------------------------------------------------
//   ChangeChordNoStem
//---------------------------------------------------------

ChangeChordNoStem::ChangeChordNoStem(Chord* c, bool f)
      {
      chord = c;
      noStem = f;
      }

void ChangeChordNoStem::flip()
      {
      bool ns = chord->noStem();
      chord->setNoStem(noStem);
      noStem = ns;
      }

//---------------------------------------------------------
//   ChangeChordRestSpace
//---------------------------------------------------------

ChangeChordRestSpace::ChangeChordRestSpace(ChordRest* _cr, Spatium _l, Spatium _t)
      {
      cr = _cr;
      l  = _l;
      t  = _t;
      }

void ChangeChordRestSpace::flip()
      {
      Spatium ol = cr->extraLeadingSpace();
      Spatium ot = cr->extraTrailingSpace();
      cr->setExtraLeadingSpace(l);
      cr->setExtraTrailingSpace(t);
      l = ol;
      t = ot;
      }

//---------------------------------------------------------
//   ChangeBeamMode
//---------------------------------------------------------

ChangeBeamMode::ChangeBeamMode(ChordRest* _cr, BeamMode _mode)
      {
      cr   = _cr;
      mode = _mode;
      }

void ChangeBeamMode::flip()
      {
      BeamMode omode = cr->beamMode();
      cr->setBeamMode(mode);
      mode = omode;
      }

//---------------------------------------------------------
//   ChangeEndBarLineType
//---------------------------------------------------------

ChangeEndBarLineType::ChangeEndBarLineType(Measure* m, int st)
      {
      measure = m;
      subtype = st;
      }

void ChangeEndBarLineType::flip()
      {
      int typ = measure->endBarLineType();
      measure->setEndBarLineType(subtype, false);
      subtype = typ;
      }

//---------------------------------------------------------
//   ChangeBarLineSpan
//---------------------------------------------------------

ChangeBarLineSpan::ChangeBarLineSpan(Staff* _staff, int _span)
      {
      staff = _staff;
      span  = _span;
      }

void ChangeBarLineSpan::flip()
      {
      int nspan = staff->barLineSpan();
      staff->setBarLineSpan(span);
      span = nspan;
      }

//---------------------------------------------------------
//   ChangeUserOffset
//---------------------------------------------------------

ChangeUserOffset::ChangeUserOffset(Element* e, const QPointF& o)
      {
      element = e;
      offset  = o;
      }

void ChangeUserOffset::flip()
      {
      QPointF p = element->userOff();
      element->setUserOff(offset);
      offset = p;
      }

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Dynamic* d, int v)
      {
      dynamic = d;
      velocity = v;
      }

void ChangeVelocity::flip()
      {
      int v = dynamic->velocity();
      dynamic->setVelocity(velocity);
      velocity = v;
      }

//---------------------------------------------------------
//   ChangeCopyright
//---------------------------------------------------------

ChangeCopyright::ChangeCopyright(Score* s, const QString& t)
      {
      score = s;
      text  = t;
      }

void ChangeCopyright::flip()
      {
      QString s;
      if (score->copyright())
            s = score->copyright()->getHtml();
      score->setCopyrightHtml(text);
      text = s;
      }

//---------------------------------------------------------
//   TransposeHarmony
//---------------------------------------------------------

TransposeHarmony::TransposeHarmony(Harmony* h, int st)
      {
      harmony = h;
      semitones = st;
      }

void TransposeHarmony::flip()
      {
      int baseTpc = harmony->baseTpc();
      int rootTpc = harmony->rootTpc();
      harmony->setBaseTpc(transposeTpc(baseTpc, semitones));
      harmony->setRootTpc(transposeTpc(rootTpc, semitones));
      harmony->buildText();
      semitones = -semitones;
      }

//---------------------------------------------------------
//   ExchangeVoice
//---------------------------------------------------------

ExchangeVoice::ExchangeVoice(Measure* m, int _val1, int _val2, int _staff1, int _staff2)
      {
      measure = m;
      val1    = _val1;
      val2    = _val2;
      staff1  = _staff1;
      staff2  = _staff2;
      }

void ExchangeVoice::undo()
      {
      measure->exchangeVoice(val2, val1, staff1, staff2);
      }

void ExchangeVoice::redo()
      {
      measure->exchangeVoice(val1, val2, staff1, staff2);
      }

//---------------------------------------------------------
//   ChangeInstrumentShort
//---------------------------------------------------------

ChangeInstrumentShort::ChangeInstrumentShort(Part* p, const QString& t)
      {
      part = p;
      text = t;
      }

void ChangeInstrumentShort::flip()
      {
      QString s = part->shortNameHtml();
      part->setShortNameHtml(text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeInstrumentLong
//---------------------------------------------------------

ChangeInstrumentLong::ChangeInstrumentLong(Part* p, const QString& t)
      {
      part = p;
      text = t;
      }

void ChangeInstrumentLong::flip()
      {
      QString s = part->longNameHtml();
      part->setLongNameHtml(text);
      text = s;
      }

//---------------------------------------------------------
//   ChangeChordRestLen
//---------------------------------------------------------

ChangeChordRestLen::ChangeChordRestLen(ChordRest* c, int l)
      {
      cr = c;
      len = l;
      }

void ChangeChordRestLen::flip()
      {
      int oldLen = cr->tickLen();
      cr->setLen(len);
      len = oldLen;
      }

//---------------------------------------------------------
//   MoveElement
//---------------------------------------------------------

MoveElement::MoveElement(Element* e, const QPointF& o)
      {
      element = e;
      offset = o;
      }

void MoveElement::flip()
      {
      QPointF po = element->userOff();
      element->setUserOff(offset);
      offset = po;
      }

//---------------------------------------------------------
//   ChangeBracketSpan
//---------------------------------------------------------

ChangeBracketSpan::ChangeBracketSpan(Staff* s, int c, int sp)
      {
      staff  = s;
      column = c;
      span   = sp;
      }

void ChangeBracketSpan::flip()
      {
      int oSpan  = staff->bracketSpan(column);
      staff->setBracketSpan(column, span);
      span = oSpan;
      }

//---------------------------------------------------------
//   EditText
//---------------------------------------------------------

void EditText::undo()
      {
      for (int i = 0; i < undoLevel; ++i)
            text->doc()->undo();
      }

void EditText::redo()
      {
      for (int i = 0; i < undoLevel; ++i)
            text->doc()->redo();
      }

