//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
#include "chord.h"
#include "al/sig.h"
#include "key.h"
#include "mscore.h"
#include "scoreview.h"
#include "barline.h"
#include "volta.h"
#include "tuplet.h"
#include "harmony.h"
#include "pitchspelling.h"
#include "part.h"
#include "beam.h"
#include "dynamics.h"
#include "seq.h"
#include "page.h"
#include "keysig.h"
#include "timesig.h"
#include "image.h"
#include "hairpin.h"

extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   UndoCommand
//---------------------------------------------------------

UndoCommand::~UndoCommand()
      {
      foreach(UndoCommand* c, childList)
            delete c;
      }

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
//   unwind
//---------------------------------------------------------

void UndoCommand::unwind()
      {
      while (!childList.isEmpty()) {
            UndoCommand* c = childList.takeLast();
            c->undo();
            delete c;
            }
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
            cmd->redo();
            delete cmd;
abort();
            return;
            }
      curCmd->appendChild(cmd);
      cmd->redo();
      }

//---------------------------------------------------------
//   pop
//---------------------------------------------------------

void UndoStack::pop()
      {
      if (!curCmd) {
            printf("UndoStack:pop(): no active command\n");
            return;
            }
      UndoCommand* cmd = curCmd->removeChild();
      cmd->undo();
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
//   SaveState
//---------------------------------------------------------

SaveState::SaveState(Score* s)
      {
      score          = s;
      redoInputState = score->inputState();
      redoSelection  = score->selection();
      }

void SaveState::undo()
      {
      redoInputState = score->inputState();
      redoSelection  = score->selection();
      score->setInputState(undoInputState);
      score->setSelection(undoSelection);
      }

void SaveState::redo()
      {
      undoInputState = score->inputState();
      undoSelection  = score->selection();
      score->setInputState(redoInputState);
      score->setSelection(redoSelection);
      }

//---------------------------------------------------------
//   undoInsertTime
//---------------------------------------------------------

void Score::undoInsertTime(int tick, int len)
      {
      _undo->push(new InsertTime(this, tick, len));
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

void Score::undoChangePitch(Note* note, int pitch, int tpc, int userAccidental, int line, int fret)
      {
      _undo->push(new ChangePitch(note, pitch, tpc, userAccidental, line, fret));
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

void Score::undoChangeChordRestLen(ChordRest* cr, const Duration& d)
      {
      _undo->push(new ChangeChordRestLen(cr, d));
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
//   undoChangeDynamic
//---------------------------------------------------------

void Score::undoChangeDynamic(Dynamic* e, int velocity, DynamicType type)
      {
      _undo->push(new ChangeDynamic(e, velocity, type));
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

void Score::undoTransposeHarmony(Harmony* h, int rootTpc, int baseTpc)
      {
      _undo->push(new TransposeHarmony(h, rootTpc, baseTpc));
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
//   undoChangeTuning
//---------------------------------------------------------

void Score::undoChangeTuning(Note* n, double v)
      {
      _undo->push(new ChangeTuning(n, v));
      }

void Score::undoChangeUserMirror(Note* n, DirectionH d)
      {
      _undo->push(new ChangeUserMirror(n, d));
      }

//---------------------------------------------------------
//   undoChangePageFormat
//---------------------------------------------------------

void Score::undoChangePageFormat(PageFormat* p, double v)
      {
      _undo->push(new ChangePageFormat(this, p, v));
      }

//---------------------------------------------------------
//   AddElement
//---------------------------------------------------------

AddElement::AddElement(Element* e)
      {
      element = e;
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

static void undoRemoveElement(Element* element)
      {
      Score* score = element->score();
      score->removeElement(element);
      if (element->type() == CLEF)
            element->staff()->setUpdateClefList(true);
      else if (element->type() == KEYSIG)
            element->staff()->setUpdateKeymap(true);
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

static void undoAddElement(Element* element)
      {
      Score* score = element->score();
      score->addElement(element);
      if (element->type() == CLEF)
            element->staff()->setUpdateClefList(true);
      else if (element->type() == KEYSIG)
            element->staff()->setUpdateKeymap(true);
      score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void AddElement::undo()
      {
      undoRemoveElement(element);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void AddElement::redo()
      {
      undoAddElement(element);
      }

//---------------------------------------------------------
//   RemoveElement
//---------------------------------------------------------

RemoveElement::RemoveElement(Element* e)
      {
      element = e;

      Score* score = element->score();
      if (element->isChordRest()) {
            // remove any slurs pointing to this chor/rest
            ChordRest* cr = static_cast<ChordRest*>(element);
            foreach(Slur* slur, cr->slurFor())
                  score->undoRemoveElement(slur);
            foreach(Slur* slur, cr->slurBack())
                  score->undoRemoveElement(slur);
            if (cr->tuplet() && cr->tuplet()->elements().empty())
                  score->undoRemoveElement(cr->tuplet());
            }
      else if (element->type() == TUPLET) {
            Tuplet* tuplet = static_cast<Tuplet*>(element);
            if (tuplet->tuplet() && tuplet->tuplet()->elements().empty())
                  score->undoRemoveElement(tuplet->tuplet());
            }
      }

//---------------------------------------------------------
//   undo
//---------------------------------------------------------

void RemoveElement::undo()
      {
      undoAddElement(element);
      }

//---------------------------------------------------------
//   redo
//---------------------------------------------------------

void RemoveElement::redo()
      {
      undoRemoveElement(element);
      }

//---------------------------------------------------------
//   ChangeNoteHead
//---------------------------------------------------------

ChangeNoteHead::ChangeNoteHead(Note* n, int g, NoteHeadType t)
   : UndoCommand(), note(n), group(g), type(t)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeNoteHead::flip()
      {
      int headGroup = note->headGroup();
      NoteHeadType t = note->headType();
      QRectF r = note->abbox();
      note->setHeadGroup(group);
      note->setHeadType(type);
      r |= note->abbox();
      group = headGroup;
      type  = t;
      note->score()->addRefresh(r);
//      note->score()->end();
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
      score->style().set(ST_concertPitch, val);
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
      measure->score()->remove(measure);
      measure->score()->addLayoutFlag(LAYOUT_FIX_TICKS);
      }

void InsertMeasure::redo()
      {
      measure->score()->addMeasure(measure);
      measure->score()->addLayoutFlag(LAYOUT_FIX_TICKS);
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

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc,
   int _userAccidental, int l, int f)
      {
      note  = _note;
      pitch = _pitch;
      tpc   = _tpc;
      line  = l;
      fret  = f;
      userAccidental = _userAccidental;
      }

void ChangePitch::flip()
      {
      int f_pitch   = note->pitch();
      int f_tpc     = note->tpc();
      int f_userAcc = note->userAccidental();
      int f_line    = note->line();
      int f_fret    = note->fret();

      note->setPitch(pitch, tpc);
      note->setUserAccidental(userAccidental);
      note->setLine(line);
      note->setFret(fret);

      pitch          = f_pitch;
      tpc            = f_tpc;
      userAccidental = f_userAcc;
      line           = f_line;
      fret           = f_fret;
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
//   FlipTupletDirection
//---------------------------------------------------------

FlipTupletDirection::FlipTupletDirection(Tuplet* b)
      {
      tuplet = b;
      }

void FlipTupletDirection::flip()
      {
      tuplet->setDirection(tuplet->isUp() ? DOWN : UP);
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
      Score* score = oldElement->score();
      if (oldElement->selected())
            score->deselect(oldElement);
      if (newElement->selected())
            score->select(newElement);
      if (oldElement->parent() == 0) {
            score->removeElement(oldElement);
            score->addElement(newElement);
            }
      else {
            oldElement->parent()->change(oldElement, newElement);
            }
      Element* e = oldElement;
      oldElement = newElement;
      newElement = e;
      if (e->type() == CLEF)
            e->staff()->setUpdateClefList(true);
      else if (e->type() == KEYSIG)
            e->staff()->setUpdateKeymap(true);
      else if (e->type() == DYNAMIC)
            e->score()->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
      score->setLayoutAll(true);
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

ChangeKeySig::ChangeKeySig(KeySig* _keysig, KeySigEvent _ks, bool sc, bool sn)
      {
      keysig = _keysig;
      ks     = _ks;
      showCourtesy = sc;
      showNaturals = sn;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeKeySig::flip()
      {
      KeySigEvent oe = keysig->keySigEvent();
      bool sc        = keysig->showCourtesySig();
      bool sn        = keysig->showNaturals();

      keysig->setSubtype(ks);
      keysig->setShowCourtesySig(showCourtesy);
      keysig->setShowNaturals(showNaturals);

      showCourtesy = sc;
      showNaturals = sn;
      ks           = oe;
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
      int endTick = measure->tick() + nl;
      for (Segment* segment = measure->first(); segment; segment = segment->next()) {
            if (segment->subtype() != SegEndBarLine
               && segment->subtype() != SegTimeSigAnnounce)
                  continue;
            segment->setTick(endTick);
            }
      measure->score()->addLayoutFlag(LAYOUT_FIX_TICKS);
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
//   ChangeDynamic
//---------------------------------------------------------

ChangeDynamic::ChangeDynamic(Dynamic* d, int v, DynamicType dt)
      {
      dynamic  = d;
      velocity = v;
      dynType  = dt;
      }

void ChangeDynamic::flip()
      {
      int v = dynamic->velocity();
      DynamicType t = dynamic->dynType();
      dynamic->setVelocity(velocity);
      dynamic->setDynType(dynType);
      dynType  = t;
      velocity = v;
      dynamic->score()->addLayoutFlag(LAYOUT_FIX_PITCH_VELO);
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

TransposeHarmony::TransposeHarmony(Harmony* h, int rtpc, int btpc)
      {
      harmony = h;
      rootTpc = rtpc;
      baseTpc = btpc;
      }

void TransposeHarmony::flip()
      {
      int baseTpc1 = harmony->baseTpc();
      int rootTpc1 = harmony->rootTpc();
      harmony->setBaseTpc(baseTpc);
      harmony->setRootTpc(rootTpc);
      harmony->render();
      rootTpc = rootTpc1;
      baseTpc = baseTpc1;
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

ChangeChordRestLen::ChangeChordRestLen(ChordRest* c, const Duration& _d)
   : cr(c), d(_d)
      {
      }

void ChangeChordRestLen::flip()
      {
      Duration od = cr->durationType();
      cr->setDurationType(d);
      cr->setDuration(d.fraction());
      d   = od;
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

//---------------------------------------------------------
//   ChangePatch
//---------------------------------------------------------

void ChangePatch::flip()
      {
      MidiPatch op;
      op.prog     = channel->program;
      op.bank     = channel->bank;
      op.synti    = channel->synti;

      channel->program = patch.prog;
      channel->bank    = patch.bank;
      channel->synti   = patch.synti;

      patch = op;

      Event event(ME_CONTROLLER);
      event.setChannel(channel->channel);

      int hbank = (patch.bank >> 7) & 0x7f;
      int lbank = patch.bank & 0x7f;

      event.setController(CTRL_HBANK);
      event.setValue(hbank);
      seq->sendEvent(event);

      event.setController(CTRL_LBANK);
      event.setValue(lbank);
      seq->sendEvent(event);

      event.setController(CTRL_PROGRAM);
      event.setValue(patch.prog);
      seq->sendEvent(event);
      }

//---------------------------------------------------------
//   ChangeTuning
//---------------------------------------------------------

void ChangeTuning::flip()
      {
      double ot = note->tuning();
      note->setTuning(tuning);
      tuning = ot;
      }

//---------------------------------------------------------
//   ChangeUserMirror
//---------------------------------------------------------

void ChangeUserMirror::flip()
      {
      DirectionH d = note->userMirror();
      note->setUserMirror(dir);
      dir = d;
      }

//---------------------------------------------------------
//   ChangePageFormat
//---------------------------------------------------------

ChangePageFormat::ChangePageFormat(Score* cs, PageFormat* p, double s)
      {
      score   = cs;
      pf      = new PageFormat(*p);
      spatium = s;
      }

ChangePageFormat::~ChangePageFormat()
      {
      delete pf;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePageFormat::flip()
      {
      PageFormat f = *(score->pageFormat());
      double os    = score->spatium();

      *(score->pageFormat()) = *pf;
      score->setSpatium(spatium);
      score->spatiumChanged(os, spatium);

      *pf     = f;
      spatium = os;
      }

//---------------------------------------------------------
//   ChangeStaff
//---------------------------------------------------------

ChangeStaff::ChangeStaff(Staff* _staff, bool _small, bool _invisible, StaffType* st)
      {
      staff     = _staff;
      small     = _small;
      invisible = _invisible;
      staffType = st;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStaff::flip()
      {
      bool invisibleChanged = staff->invisible() != invisible;
      bool typeChanged      = staff->staffType() != staffType;

      int oldSmall      = staff->small();
      bool oldInvisible = staff->invisible();
      StaffType* st     = staff->staffType();

      staff->setSmall(small);
      staff->setInvisible(invisible);
      staff->setStaffType(staffType);

      small     = oldSmall;
      invisible = oldInvisible;
      staffType = st;

      if (invisibleChanged || typeChanged) {
            Score* score = staff->score();
            int staffIdx = score->staffIdx(staff);
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  MStaff* mstaff = m->mstaff(staffIdx);
                  mstaff->lines->setVisible(!staff->invisible());
                  }
            }
      staff->score()->rebuildMidiMapping();
      staff->score()->setPlaylistDirty(true);
      }

//---------------------------------------------------------
//   ChangePart
//---------------------------------------------------------

ChangePart::ChangePart(Part* _part, const QTextDocument* _longName, const QTextDocument* _shortName,
   const Instrument& i)
      {
      longName   = _longName->clone(0);
      shortName  = _shortName->clone(0);
      instrument = i;
      part       = _part;
      }

ChangePart::~ChangePart()
      {
      delete longName;
      delete shortName;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangePart::flip()
      {
      Instrument oi         = *part->instr();
      part->setInstrument(instrument);
      longName              = part->longName()->swapDoc(longName);
      shortName             = part->shortName()->swapDoc(shortName);
      instrument            = oi;
      part->score()->setInstrumentNames();
      }

//---------------------------------------------------------
//   ChangeTextStyles
//---------------------------------------------------------

ChangeTextStyles::ChangeTextStyles(Score* s, const QVector<TextStyle*>& st)
      {
      score = s;
      foreach(TextStyle* s, st)
            styles.append(new TextStyle(*s));
      }

ChangeTextStyles::~ChangeTextStyles()
      {
      foreach(TextStyle* s, styles)
            delete s;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTextStyles::flip()
      {
      styles = score->swapTextStyles(styles);
      }

//---------------------------------------------------------
//   ChangeStretch
//---------------------------------------------------------

ChangeStretch::ChangeStretch(Measure* m, double s)
   : measure(m), stretch(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStretch::flip()
      {
      double oStretch = measure->userStretch();
      measure->setUserStretch(stretch);
      stretch = oStretch;
      }

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const Style& st)
   : score(s), style(st)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip()
      {
      Style tmp = score->style();

      if (score->styleB(ST_concertPitch) != style[ST_concertPitch].toBool())
            score->cmdConcertPitchChanged(style[ST_concertPitch].toBool(), true);

      score->setStyle(style);
      style = tmp;
      }

//---------------------------------------------------------
//   ChangeSlurProperties
//---------------------------------------------------------

ChangeSlurProperties::ChangeSlurProperties(SlurTie* _st, int lt)
   : st(_st), lineType(lt)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeSlurProperties::flip()
      {
      int ols = st->lineType();
      st->setLineType(lineType);
      lineType = ols;
      }

//---------------------------------------------------------
//   ChangeChordStaffMove
//---------------------------------------------------------

ChangeChordStaffMove::ChangeChordStaffMove(Chord* c, int v)
   : chord(c), staffMove(v)
      {
      }

void ChangeChordStaffMove::flip()
      {
      int v = chord->staffMove();
      chord->setStaffMove(staffMove);
      staffMove = v;
      }

//---------------------------------------------------------
//   ChangeTupletProperties
//---------------------------------------------------------

ChangeTupletProperties::ChangeTupletProperties(Tuplet* t, int nt, int bt)
   : tuplet(t), numberType(nt), bracketType(bt)
      {
      }

void ChangeTupletProperties::flip()
      {
      int nt = tuplet->numberType();
      int bt = tuplet->bracketType();
      tuplet->setNumberType(numberType);
      tuplet->setBracketType(bracketType);
      numberType = nt;
      bracketType = bt;
      }

//---------------------------------------------------------
//   ChangeVelocity
//---------------------------------------------------------

ChangeVelocity::ChangeVelocity(Note* n, ValueType t, int v, int o)
   : note(n), veloType(t), velocity(v), veloOffset(o)
      {
      }

void ChangeVelocity::flip()
      {
      ValueType t = note->veloType();
      int v       = note->velocity();
      int o       = note->veloOffset();
      note->setVeloType(veloType);
      note->setVelocity(velocity);
      note->setVeloOffset(veloOffset);
      veloType   = t;
      velocity   = v;
      veloOffset = o;
      }

//---------------------------------------------------------
//   ChangeMStaffProperties
//---------------------------------------------------------

ChangeMStaffProperties::ChangeMStaffProperties(MStaff* ms, bool v, bool s)
   : mstaff(ms), visible(v), slashStyle(s)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMStaffProperties::flip()
      {
      bool v = mstaff->visible();
      bool s = mstaff->slashStyle();
      mstaff->setVisible(visible);
      mstaff->setSlashStyle(slashStyle);
      visible    = v;
      slashStyle = s;
      }

//---------------------------------------------------------
//   ChangeMeasureProperties
//---------------------------------------------------------

ChangeMeasureProperties::ChangeMeasureProperties(
   Measure* m,
   const Fraction& _sig,
   const Fraction& _len,
   bool _bmm,
   int rc,
   double s,
   int o,
   bool ir
   ) :
   measure(m),
   sig(_sig),
   len(_len),
   breakMM(_bmm),
   repeatCount(rc),
   stretch(s),
   noOffset(o),
   irregular(ir)
      {
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMeasureProperties::flip()
      {
      bool a   = measure->breakMultiMeasureRest();
      int r    = measure->repeatCount();
      double s = measure->userStretch();
      int o    = measure->noOffset();
      bool ir  = measure->irregular();
      Fraction _sig = measure->timesig();
      Fraction _len = measure->len();

      measure->setBreakMultiMeasureRest(breakMM);
      measure->setRepeatCount(repeatCount);
      measure->setUserStretch(stretch);
      measure->setTimesig(sig);
      measure->setLen(len);
      Score* score = measure->score();
      if (o != noOffset || ir != irregular) {
            measure->setNoOffset(noOffset);
            measure->setIrregular(irregular);
            score->renumberMeasures();
            }

      breakMM     = a;
      repeatCount = r;
      stretch     = s;
      noOffset    = o;
      irregular   = ir;
      sig         = _sig;
      len         = _len;

      score->addLayoutFlag(LAYOUT_FIX_TICKS);
      score->setLayoutAll(true);
      score->setDirty();
      }

//---------------------------------------------------------
//   ChangeNoteProperties
//---------------------------------------------------------

ChangeNoteProperties::ChangeNoteProperties(Note* n, ValueType v1, int v2, int v3,
   int v6, int v9)
      {
      note               = n;
      _veloType          = v1;
      _velocity          = v2;      ///< midi playback velocity (0 - 127);
      _veloOffset        = v3;      ///< velocity user offset in promille
      _onTimeUserOffset  = v6;      ///< start note user offset
      _offTimeUserOffset = v9;      ///< stop note user offset
      };

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeNoteProperties::flip()
      {
      ValueType v1 = note->veloType();
      int       v2 = note->velocity();
      int       v3 = note->veloOffset();
      int       v6 = note->onTimeUserOffset();
      int       v9 = note->offTimeUserOffset();

      note->setVeloType(_veloType);
      note->setVelocity(_velocity);
      note->setVeloOffset(_veloOffset);
      note->setOnTimeUserOffset(_onTimeUserOffset);
      note->setOffTimeUserOffset(_offTimeUserOffset);

      _veloType          = v1;
      _velocity          = v2;
      _veloOffset        = v3;
      _onTimeUserOffset  = v6;
      _offTimeUserOffset = v9;
      }

//---------------------------------------------------------
//   ChangeMeasureTimesig
//---------------------------------------------------------

ChangeMeasureTimesig::ChangeMeasureTimesig(Measure* _m, const Fraction& f)
   : m(_m), ts(f)
      {
      };

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeMeasureTimesig::flip()
      {
      Fraction nts = m->timesig();
      m->setTimesig(ts);
      ts = nts;
      }

//---------------------------------------------------------
//   ChangeTimesig
//---------------------------------------------------------

ChangeTimesig::ChangeTimesig(TimeSig* _timesig, bool sc)
      {
      timesig = _timesig;
      showCourtesy = sc;
      };

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTimesig::flip()
      {
      bool sc        = timesig->showCourtesySig();
      timesig->setShowCourtesySig(showCourtesy);
      showCourtesy = sc;
      }

//---------------------------------------------------------
//   RemoveMeasures
//---------------------------------------------------------

RemoveMeasures::RemoveMeasures(Measure* m1, Measure* m2)
   : fm(m1), lm(m2)
      {
      }

//---------------------------------------------------------
//   undo
//    insert back measures
//---------------------------------------------------------

void RemoveMeasures::undo()
      {
      fm->score()->measures()->insert(fm, lm);
      }

//---------------------------------------------------------
//   redo
//    remove measures
//---------------------------------------------------------

void RemoveMeasures::redo()
      {
      fm->score()->measures()->remove(fm, lm);
      }

//---------------------------------------------------------
//   undo
//    insert back measures
//---------------------------------------------------------

void InsertMeasures::undo()
      {
      fm->score()->measures()->remove(fm, lm);
      }

//---------------------------------------------------------
//   redo
//    remove measures
//---------------------------------------------------------

void InsertMeasures::redo()
      {
      fm->score()->measures()->insert(fm, lm);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeClef::flip()
      {
      bool sc = clef->showCourtesyClef();
      clef->setShowCourtesyClef(showCourtesy);
      showCourtesy = sc;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeImage::flip()
      {
      bool _lockAspectRatio = image->lockAspectRatio();
      bool _autoScale       = image->autoScale();
      image->setLockAspectRatio(lockAspectRatio);
      image->setAutoScale(autoScale);
      lockAspectRatio = _lockAspectRatio;
      autoScale       = _autoScale;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeHairpin::flip()
      {
      int vc        = hairpin->veloChange();
      DynamicType t = hairpin->dynType();
      hairpin->setVeloChange(veloChange);
      hairpin->setDynType(dynType);
      veloChange = vc;
      dynType    = t;
      hairpin->score()->updateHairpin(hairpin);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeDuration::flip()
      {
      Fraction od = cr->duration();
      cr->setDuration(d);
      d = od;
      }


