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
#include "rest.h"
#include "bend.h"
#include "tremolobar.h"
#include "articulation.h"
#include "noteevent.h"
#include "slur.h"
#include "excerpt.h"

extern Measure* tick2measure(int tick);

//---------------------------------------------------------
//   updateNoteLines
//    compute line position of note heads after
//    clef change
//---------------------------------------------------------

void updateNoteLines(Segment* segment, int track)
      {
      for (Segment* s = segment->next1(); s; s = s->next1()) {
            if (s->subtype() == SegClef && s->element(track))
                  break;
            if (s->subtype() != SegChordRest)
                  continue;
            for (int t = track; t < track+VOICES; ++t) {
                  Chord* chord = static_cast<Chord*>(s->element(t));
                  if (chord && chord->type() == CHORD) {
                        foreach(Note* note, chord->notes()) {
                              note->updateLine();
                              }
                        }
                  }
            }
      }

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
      if (debugMode)
            printf("UndoStack::beginMacro %p, UndoStack %p\n", curCmd, this);
      }

//---------------------------------------------------------
//   endMacro
//---------------------------------------------------------

void UndoStack::endMacro(bool rollback)
      {
      if (debugMode)
            printf("UndoStack::endMacro %d\n", rollback);
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
            printf("UndoStack:push(): no active command, UndoStack %p\n", this);
abort();
            cmd->redo();
            delete cmd;
            return;
            }
#ifdef DEBUG_UNDO
      printf("UndoStack::push <%s>\n", cmd->name());
#endif
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
      undo()->push(new InsertTime(this, tick, len));
      }

//---------------------------------------------------------
//   undoChangeMeasureLen
//---------------------------------------------------------

void Score::undoChangeMeasureLen(Measure* m, int oldTicks, int newTicks)
      {
      undo()->push(new ChangeMeasureLen(m, oldTicks, newTicks));
      }

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(Element* oldElement, Element* newElement)
      {
      undo()->push(new ChangeElement(oldElement, newElement));
      }

//---------------------------------------------------------
//   undoChangeSubtype
//---------------------------------------------------------

void Score::undoChangeSubtype(Element* element, int st)
      {
      undo()->push(new ChangeSubtype(element, st));
      }

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch, int tpc, int line, int fret, int string)
      {
      QList<Staff*> staffList;
      Staff* ostaff = note->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      Chord* chord = note->chord();
      int noteIndex = chord->notes().indexOf(note);
      Segment* segment = chord->segment();
      Measure* measure = segment->measure();
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            Measure* m;
            Segment* s;
            if (score == this) {
                  m = measure;
                  s = segment;
                  }
            else {
                  m = score->tick2measure(measure->tick());
                  s = m->findSegment(segment->segmentType(), segment->tick());
                  }
            int staffIdx = score->staffIdx(staff);
            Chord* c     = static_cast<Chord*>(s->element(staffIdx * VOICES + chord->voice()));
            Note* n      = c->notes().at(noteIndex);
            undo()->push(new ChangePitch(n, pitch, tpc, line, fret, string));
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   undoChangeKeySig
//---------------------------------------------------------

void Score::undoChangeKeySig(Staff* ostaff, int tick, KeySigEvent st)
      {
      QList<Staff*> staffList;
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      LinkedElements* links = 0;
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();

            Measure* measure = score->tick2measure(tick);
            if (!measure) {
                  printf("measure for tick %d not found!\n", tick);
                  continue;
                  }
            Segment* s = measure->findSegment(SegKeySig, tick);
            if (!s) {
                  s = new Segment(measure, SegKeySig, tick);
                  score->undoAddElement(s);
                  }
            int track  = score->staffIdx(staff) * VOICES;
            KeySig* ks = static_cast<KeySig*>(s->element(track));

            KeySig* nks = new KeySig(score);
            nks->setTrack(track);
            nks->changeKeySigEvent(st);
            nks->setParent(s);
            if (links == 0)
                  links = new LinkedElements;
            links->add(nks);
            nks->setLinks(links);

            if (ks)
                  undo()->push(new ChangeElement(ks, nks));
            else
                  undo()->push(new AddElement(nks));
            }
      }

//---------------------------------------------------------
//   undoChangeTpc
//---------------------------------------------------------

void Score::undoChangeTpc(Note* note, int tpc)
      {
      undo()->push(new ChangeTpc(note, tpc));
      }

//---------------------------------------------------------
//   undoChangeBeamMode
//---------------------------------------------------------

void Score::undoChangeBeamMode(ChordRest* cr, BeamMode mode)
      {
      undo()->push(new ChangeBeamMode(cr, mode));
      }

//---------------------------------------------------------
//   findLinkedVoiceElement
//---------------------------------------------------------

static Element* findLinkedVoiceElement(Element* e, Staff* nstaff)
      {
      Score* score     = nstaff->score();
      Segment* segment = static_cast<Segment*>(e->parent());
      Measure* measure = segment->measure();
      Measure* m       = score->tick2measure(measure->tick());
      Segment* s       = m->findSegment(segment->segmentType(), segment->tick());
      int staffIdx     = score->staffIdx(nstaff);
      return s->element(staffIdx * VOICES + e->voice());
      }

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, const Duration& d)
      {
      Staff* ostaff = cr->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves) {
            foreach(Staff* staff, linkedStaves->staves()) {
                  if (staff == cr->staff())
                        continue;
                  ChordRest* ncr = static_cast<ChordRest*>(findLinkedVoiceElement(cr, staff));
                  undo()->push(new ChangeChordRestLen(ncr, d));
                  }
            }
      undo()->push(new ChangeChordRestLen(cr, d));
      }

//---------------------------------------------------------
//   undoChangeEndBarLineType
//---------------------------------------------------------

void Score::undoChangeEndBarLineType(Measure* m, BarLineType subtype)
      {
      undo()->push(new ChangeEndBarLineType(m, subtype));
      }

//---------------------------------------------------------
//   undoChangeBarLineSpan
//---------------------------------------------------------

void Score::undoChangeBarLineSpan(Staff* staff, int span)
      {
      undo()->push(new ChangeBarLineSpan(staff, span));
      }

//---------------------------------------------------------
//   undoChangeUserOffset
//---------------------------------------------------------

void Score::undoChangeUserOffset(Element* e, const QPointF& offset)
      {
      undo()->push(new ChangeUserOffset(e, offset));
      }

//---------------------------------------------------------
//   undoChangeDynamic
//---------------------------------------------------------

void Score::undoChangeDynamic(Dynamic* e, int velocity, DynamicType type)
      {
      undo()->push(new ChangeDynamic(e, velocity, type));
      }

//---------------------------------------------------------
//   undoTransposeHarmony
//---------------------------------------------------------

void Score::undoTransposeHarmony(Harmony* h, int rootTpc, int baseTpc)
      {
      undo()->push(new TransposeHarmony(h, rootTpc, baseTpc));
      }

//---------------------------------------------------------
//   undoExchangeVoice
//---------------------------------------------------------

void Score::undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2)
      {
      undo()->push(new ExchangeVoice(measure, val1, val2, staff1, staff2));
      }

//---------------------------------------------------------
//   undoRemovePart
//---------------------------------------------------------

void Score::undoRemovePart(Part* part, int idx)
      {
      undo()->push(new RemovePart(part, idx));
      }

//---------------------------------------------------------
//   undoInsertPart
//---------------------------------------------------------

void Score::undoInsertPart(Part* part, int idx)
      {
      undo()->push(new InsertPart(part, idx));
      }

//---------------------------------------------------------
//   undoInsertMeasure
//---------------------------------------------------------

void Score::undoInsertMeasure(MeasureBase* m)
      {
      undo()->push(new InsertMeasure(m));
      }

//---------------------------------------------------------
//   undoRemoveStaff
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff, int idx)
      {
      undo()->push(new RemoveStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoInsertStaff
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, int idx)
      {
      undo()->push(new InsertStaff(staff, idx));
      }

//---------------------------------------------------------
//   undoMove
//---------------------------------------------------------

void Score::undoMove(Element* e, const QPointF& pt)
      {
      undo()->push(new MoveElement(e, pt));
      }

//---------------------------------------------------------
//   undoChangeRepeatFlags
//---------------------------------------------------------

void Score::undoChangeRepeatFlags(Measure* m, int flags)
      {
      undo()->push(new ChangeRepeatFlags(m, flags));
      }

//---------------------------------------------------------
//   undoChangeVoltaEnding
//---------------------------------------------------------

void Score::undoChangeVoltaEnding(Volta* volta, const QList<int>& l)
      {
      undo()->push(new ChangeVoltaEnding(volta, l));
      }

//---------------------------------------------------------
//   undoChangeVoltaText
//---------------------------------------------------------

void Score::undoChangeVoltaText(Volta* volta, const QString& s)
      {
      undo()->push(new ChangeVoltaText(volta, s));
      }

//---------------------------------------------------------
//   undoChangeChordRestSize
//---------------------------------------------------------

void Score::undoChangeChordRestSize(ChordRest* cr, bool small)
      {
      undo()->push(new ChangeChordRestSize(cr, small));
      }

//---------------------------------------------------------
//   undoChangeChordNoStem
//---------------------------------------------------------

void Score::undoChangeChordNoStem(Chord* cr, bool noStem)
      {
      undo()->push(new ChangeChordNoStem(cr, noStem));
      }

//---------------------------------------------------------
//   undoChangeChordRestSpace
//---------------------------------------------------------

void Score::undoChangeChordRestSpace(ChordRest* cr, Spatium l, Spatium t)
      {
      undo()->push(new ChangeChordRestSpace(cr, l, t));
      }

//---------------------------------------------------------
//   undoChangeBracketSpan
//---------------------------------------------------------

void Score::undoChangeBracketSpan(Staff* staff, int column, int span)
      {
      undo()->push(new ChangeBracketSpan(staff, column, span));
      }

//---------------------------------------------------------
//   undoToggleInvisible
//---------------------------------------------------------

void Score::undoToggleInvisible(Element* e)
      {
      undo()->push(new ToggleInvisible(e));
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(Element* element)
      {
      QList<Staff*> staffList;
      Staff* ostaff;
      if (element->type() == SLUR)
            ostaff = static_cast<Slur*>(element)->startElement()->staff();
      else
            ostaff = element->staff();
      if (ostaff == 0 || (element->type() != ARTICULATION
         && element->type() != SLUR
         && element->type() != TIE
         && element->type() != NOTE
         && element->type() != HAIRPIN
         && element->type() != OTTAVA
         && element->type() != TRILL
         && element->type() != TEXTLINE
         && element->type() != VOLTA
         && element->type() != TUPLET)
            ) {
            undo()->push(new AddElement(element));
            return;
            }
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            int staffIdx = score->staffIdx(staff);
            Element* ne;
            if (staff == ostaff)
                  ne = element;
            else {
                  ne = element->linkedClone();
                  ne->setScore(score);
                  ne->setSelected(false);
                  }
            if (element->type() == ARTICULATION) {
                  Articulation* a  = static_cast<Articulation*>(element);
                  ChordRest* cr    = static_cast<ChordRest*>(a->parent());
                  Segment* segment = cr->segment();
                  int tick         = segment->tick();
                  Measure* m       = score->tick2measure(tick);
                  Segment* seg     = m->findSegment(SegChordRest, tick);
                  Articulation* na = static_cast<Articulation*>(ne);
                  int ntrack       = staffIdx * VOICES + a->voice();
                  na->setTrack(ntrack);
                  ChordRest* ncr   = static_cast<ChordRest*>(seg->element(ntrack));
                  na->setParent(ncr);
                  undo()->push(new AddElement(na));
                  }
            else if (element->type() == SLUR) {
                  Slur* slur     = static_cast<Slur*>(element);
                  ChordRest* cr1 = static_cast<ChordRest*>(slur->startElement());
                  ChordRest* cr2 = static_cast<ChordRest*>(slur->endElement());
                  Segment* s1    = cr1->segment();
                  Segment* s2    = cr2->segment();
                  Measure* m1    = s1->measure();
                  Measure* m2    = s2->measure();
                  Measure* nm1   = score->tick2measure(m1->tick());
                  Measure* nm2   = score->tick2measure(m2->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  Chord* c2      = static_cast<Chord*>(ns2->element(staffIdx * VOICES + cr2->voice()));
                  Slur* nslur    = static_cast<Slur*>(ne);
                  nslur->setStartElement(c1);
                  nslur->setEndElement(c2);
                  nslur->setParent(0);
                  undo()->push(new AddElement(nslur));
                  }
            else if (element->type() == HAIRPIN
               || element->type() == OTTAVA
               || element->type() == TRILL
               || element->type() == TEXTLINE
               || element->type() == VOLTA
               ) {
                  SLine* hp      = static_cast<SLine*>(element);
                  Segment* s1    = static_cast<Segment*>(hp->startElement());
                  Segment* s2    = static_cast<Segment*>(hp->endElement());
                  Measure* m1    = s1->measure();
                  Measure* m2    = s2->measure();
                  Measure* nm1   = score->tick2measure(m1->tick());
                  Measure* nm2   = score->tick2measure(m2->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                  SLine* nhp     = static_cast<SLine*>(ne);
                  nhp->setStartElement(ns1);
                  nhp->setEndElement(ns2);
                  nhp->setParent(ns1);
                  undo()->push(new AddElement(nhp));
                  }
            else if (element->type() == TIE) {
                  Tie* tie       = static_cast<Tie*>(element);
                  Note* n1       = tie->startNote();
                  Note* n2       = tie->endNote();
                  Chord* cr1     = n1->chord();
                  Chord* cr2     = n2->chord();
                  Segment* s1    = cr1->segment();
                  Segment* s2    = cr2->segment();
                  Measure* nm1   = score->tick2measure(s1->tick());
                  Measure* nm2   = score->tick2measure(s2->tick());
                  Segment* ns1   = nm1->findSegment(s1->segmentType(), s1->tick());
                  Segment* ns2   = nm2->findSegment(s2->segmentType(), s2->tick());
                  Chord* c1      = static_cast<Chord*>(ns1->element(staffIdx * VOICES + cr1->voice()));
                  Chord* c2      = static_cast<Chord*>(ns2->element(staffIdx * VOICES + cr2->voice()));
                  Note* nn1      = c1->findNote(n1->pitch());
                  Note* nn2      = c2->findNote(n2->pitch());
                  Tie* ntie      = static_cast<Tie*>(ne);
                  QList<SpannerSegment*>& segments = ntie->spannerSegments();
                  foreach(SpannerSegment* segment, segments)
                              delete segment;
                  segments.clear();
                  ntie->setTrack(c1->track());
                  ntie->setStartNote(nn1);
                  ntie->setEndNote(nn2);
                  undo()->push(new AddElement(ntie));
                  score->updateAccidentals(nm1, staffIdx);
                  if (nm1 != nm2)
                        score->updateAccidentals(nm2, staffIdx);
                  }
            else if (element->type() == NOTE) {
                  Note* note       = static_cast<Note*>(element);
                  Chord* cr        = note->chord();
                  Segment* segment = cr->segment();
                  int tick         = segment->tick();
                  Measure* m       = score->tick2measure(tick);
                  Segment* seg     = m->findSegment(SegChordRest, tick);
                  Note* nnote      = static_cast<Note*>(ne);
                  int ntrack       = staffIdx * VOICES + nnote->voice();
                  nnote->setScore(score);
                  nnote->setTrack(ntrack);
                  Chord* ncr       = static_cast<Chord*>(seg->element(ntrack));
                  nnote->setParent(ncr);
                  undo()->push(new AddElement(nnote));
                  score->updateAccidentals(m, staffIdx);
                  score->setLayout(m);
                  }
            else if (element->type() == TUPLET) {
                  Tuplet* t      = static_cast<Tuplet*>(element);
                  Tuplet* nt     = static_cast<Tuplet*>(ne);
                  int ntrack     = staffIdx * VOICES + t->voice();
                  Measure* m     = score->tick2measure(t->tick());
                  nt->setTrack(ntrack);
                  nt->setParent(m);
                  undo()->push(new AddElement(nt));
                  }
            else
                  printf("undoAddElement: unhandled: <%s>\n", element->name());
            }
      }

//---------------------------------------------------------
//   undoAddCR
//---------------------------------------------------------

void Score::undoAddCR(ChordRest* cr, Measure* measure, int tick)
      {
      QList<Staff*> staffList;
      Staff* ostaff = cr->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);
      SegmentType segmentType;
      if ((cr->type() == CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL))
            segmentType = SegGrace;
      else
            segmentType = SegChordRest;
      foreach(Staff* staff, staffList) {
            Score* score = staff->score();
            Measure* m   = (score == this) ? measure : score->tick2measure(tick);
            // always create new segment for grace note:
            Segment* seg = 0;
            if (segmentType != SegGrace)
                  seg = m->findSegment(segmentType, tick);
            if (seg == 0) {
                  seg = new Segment(m, segmentType, tick);
                  score->undoAddElement(seg);
                  }
            ChordRest* newcr = (staff == ostaff) ? cr : static_cast<ChordRest*>(cr->clone());
            newcr->setScore(score);
            int staffIdx = score->staffIdx(staff);
            int ntrack = staffIdx * VOICES + cr->voice();
            newcr->setTrack(ntrack);
            newcr->setParent(seg);
            if (newcr->type() == CHORD) {
                  Chord* chord = static_cast<Chord*>(newcr);
                  // setTpcFromPitch needs to know the note tick position
                  foreach(Note* note, chord->notes())
                        note->setTpcFromPitch();
                  }
            if (cr->tuplet()) {
                  int tick = cr->tuplet()->tick();
                  Tuplet* nt = 0;
                  foreach(Tuplet* t, *m->tuplets()) {
                        if (t->tick() == tick && t->track() == ntrack) {
                              nt = t;
                              break;
                              }
                        }
                  if (nt)
                        newcr->setTuplet(nt);
                  else
                        printf("Tuplet not found\n");
                  }
            undo()->push(new AddElement(newcr));
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      LinkedElements* le = element->links();
      if (le) {
            foreach(Element* e, le->elements())
                  undo()->push(new RemoveElement(e));
            }
      else
            undo()->push(new RemoveElement(element));
      }

//---------------------------------------------------------
//   undoChangeTuning
//---------------------------------------------------------

void Score::undoChangeTuning(Note* n, double v)
      {
      undo()->push(new ChangeTuning(n, v));
      }

void Score::undoChangeUserMirror(Note* n, DirectionH d)
      {
      undo()->push(new ChangeUserMirror(n, d));
      }

//---------------------------------------------------------
//   undoChangePageFormat
//---------------------------------------------------------

void Score::undoChangePageFormat(PageFormat* p, double v)
      {
      undo()->push(new ChangePageFormat(this, p, v));
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
//   name
//---------------------------------------------------------

#ifdef DEBUG_UNDO
const char* AddElement::name() const
      {
      static char buffer[64];
      sprintf(buffer, "Add: %s", element->name());
      return buffer;
      }
#endif


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
//   name
//---------------------------------------------------------

#ifdef DEBUG_UNDO
const char* RemoveElement::name() const
      {
      static char buffer[64];
      sprintf(buffer, "Remove: %s", element->name());
      return buffer;
      }
#endif

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
      if(note->noteHead() != -1){
        r |= note->abbox();
        group = headGroup;
        type  = t;
        note->score()->addRefresh(r);
        }
      else{
          note->setHeadGroup(headGroup);
          note->setHeadType(t);
          }
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
      measure->score()->addLayoutFlags(LAYOUT_FIX_TICKS);
      }

void InsertMeasure::redo()
      {
      measure->score()->addMeasure(measure);
      measure->score()->addLayoutFlags(LAYOUT_FIX_TICKS);
      }

//---------------------------------------------------------
//   SortStaves
//---------------------------------------------------------

SortStaves::SortStaves(Score* s, QList<int> l)
      {
      score = s;

      for(int i=0 ; i < l.size(); i++) {
            rlist.append(l.indexOf(i));
            }
      list  = l;
      }

void SortStaves::redo()
      {
      score->sortStaves(list);
      }

void SortStaves::undo()
      {
      score->sortStaves(rlist);
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

ChangePitch::ChangePitch(Note* _note, int _pitch, int _tpc, int l, int f, int s)
      {
      note  = _note;
      if (_note == 0)
            abort();
      pitch  = _pitch;
      tpc    = _tpc;
      line   = l;
      fret   = f;
      string = s;
      }

void ChangePitch::flip()
      {
      int f_pitch                 = note->pitch();
      int f_tpc                   = note->tpc();
      int f_line                  = note->line();
      int f_fret                  = note->fret();
      int f_string                = note->string();

      note->setPitch(pitch, tpc);
      note->setLine(line);
      note->setFret(fret);
      note->setString(string);

      pitch          = f_pitch;
      tpc            = f_tpc;
      line           = f_line;
      fret           = f_fret;
      string         = f_string;

      note->score()->setLayout(note->chord()->segment()->measure());
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
      if (element->type() == CLEF) {
            Clef* clef       = static_cast<Clef*>(element);
            Segment* segment = clef->segment();
            Staff* staff     = clef->staff();
            staff->setClef(segment->tick(), ClefType(subtype));
            updateNoteLines(segment, clef->track());
            clef->score()->setLayoutAll(true);
            }
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
      LinkedElements* links = oldElement->links();
      if (links) {
            links->remove(oldElement);
            links->add(newElement);
            }

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
      qSwap(oldElement, newElement);
      if (newElement->type() == CLEF)
            newElement->staff()->setUpdateClefList(true);
      else if (newElement->type() == KEYSIG)
            newElement->staff()->setUpdateKeymap(true);
      else if (newElement->type() == DYNAMIC)
            newElement->score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
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

      keysig->setKeySigEvent(ks);
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
      measure->score()->addLayoutFlags(LAYOUT_FIX_TICKS);
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
      measure->score()->setLayout(measure);
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

ChangeEndBarLineType::ChangeEndBarLineType(Measure* m, BarLineType st)
      {
      measure = m;
      subtype = st;
      }

void ChangeEndBarLineType::flip()
      {
      BarLineType typ = measure->endBarLineType();
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
//   ChangeSlurOffsets
//---------------------------------------------------------

void ChangeSlurOffsets::flip()
      {
      for (int i = 0; i < 4; ++i) {
            QPointF f = slur->slurOffset(i);
            slur->setSlurOffset(i, off[i]);
            off[i] = f;
            }
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
      dynamic->score()->addLayoutFlags(LAYOUT_FIX_PITCH_VELO);
      }

#if 0
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
      QString s = score->rights();
      score->setRights(text);
      text = s;
      }
#endif

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
      cr->score()->setLayout(cr->measure());
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

ChangeStaff::ChangeStaff(Staff* _staff, bool _small, bool _invisible, bool _show, StaffType* st)
      {
      staff     = _staff;
      small     = _small;
      invisible = _invisible;
      show      = _show;
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
      bool oldShow      = staff->show();
      StaffType* st     = staff->staffType();

      staff->setSmall(small);
      staff->setInvisible(invisible);
      staff->setShow(show);
      staff->setStaffType(staffType);

      small     = oldSmall;
      invisible = oldInvisible;
      show      = oldShow;
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
//TODOxx      longName              = part->longName()->swapDoc(longName);
//      shortName             = part->shortName()->swapDoc(shortName);
      instrument            = oi;
      part->score()->setInstrumentNames();
      }

//---------------------------------------------------------
//   ChangeTextStyle
//---------------------------------------------------------

ChangeTextStyle::ChangeTextStyle(Score* s, const TextStyle& st)
      {
      score = s;
      style = st;
      }

static void updateTextStyle(void* a, Element* e)
      {
      TextStyleType ts = *(TextStyleType*)a;
      if (e->isText()) {
            Text* text = static_cast<Text*>(e);
            if ((text->textStyle() == ts) && text->styled())
                  text->setText(text->getText());     // destroy formatting
            }
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTextStyle::flip()
      {
      TextStyle os = score->style().textStyle(style.name());
      score->style().setTextStyle(style);
      style = os;
      TextStyleType ts = score->style().textStyleType(style.name());
      score->scanElements(&ts, updateTextStyle);
      score->setLayoutAll(true);
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
      measure->score()->setLayoutAll(true);
      stretch = oStretch;
      }

//---------------------------------------------------------
//   ChangeStyle
//---------------------------------------------------------

ChangeStyle::ChangeStyle(Score* s, const Style& st)
   : score(s), style(st)
      {
      }

static void updateTextStyle2(void*, Element* e)
      {
      if (e->isText()) {
            Text* text = static_cast<Text*>(e);
            if (text->styled())
                  text->setText(text->getText());     // destroy formatting
            }
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeStyle::flip()
      {
      Style tmp = score->style();

      if (score->styleB(ST_concertPitch) != style.valueB(ST_concertPitch))
            score->cmdConcertPitchChanged(style.valueB(ST_concertPitch), true);

      score->setStyle(style);
      score->scanElements(0, updateTextStyle2);
      score->setLayoutAll(true);

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

ChangeVelocity::ChangeVelocity(Note* n, ValueType t, int o)
   : note(n), veloType(t), veloOffset(o)
      {
      }

void ChangeVelocity::flip()
      {
      ValueType t = note->veloType();
      int o       = note->veloOffset();
      note->setVeloType(veloType);
      note->setVeloOffset(veloOffset);
      veloType   = t;
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

      score->addLayoutFlags(LAYOUT_FIX_TICKS);
      score->setLayoutAll(true);
      score->setDirty();
      }

//---------------------------------------------------------
//   ChangeNoteProperties
//---------------------------------------------------------

ChangeNoteProperties::ChangeNoteProperties(Note* n, ValueType v1, int v3,
   int v6, int v9)
      {
      note               = n;
      _veloType          = v1;
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
      int       v3 = note->veloOffset();
      int       v6 = note->onTimeUserOffset();
      int       v9 = note->offTimeUserOffset();

      note->setVeloType(_veloType);
      note->setVeloOffset(_veloOffset);
      note->setOnTimeUserOffset(_onTimeUserOffset);
      note->setOffTimeUserOffset(_offTimeUserOffset);

      _veloType          = v1;
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

//---------------------------------------------------------
//   AddExcerpt::undo
//---------------------------------------------------------

void AddExcerpt::undo()
      {
      score->parentScore()->removeExcerpt(score);
      }

//---------------------------------------------------------
//   AddExcerpt::redo
//---------------------------------------------------------

void AddExcerpt::redo()
      {
      score->parentScore()->addExcerpt(score);
      }

//---------------------------------------------------------
//   RemoveExcerpt::undo()
//---------------------------------------------------------

void RemoveExcerpt::undo()
      {
      score->parentScore()->addExcerpt(score);
      }

//---------------------------------------------------------
//   RemoveExcerpt::redo()
//---------------------------------------------------------

void RemoveExcerpt::redo()
      {
      score->parentScore()->removeExcerpt(score);
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBend::flip()
      {
      QList<PitchValue> pv = bend->points();
      bend->setPoints(points);
      points = pv;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeTremoloBar::flip()
      {
      QList<PitchValue> pv = bend->points();
      bend->setPoints(points);
      points = pv;
      }

//---------------------------------------------------------
//   ChangeNoteEvents::flip
//---------------------------------------------------------

void ChangeNoteEvents::flip()
      {
      QList<NoteEvent*> e = chord->playEvents();
      chord->setPlayEvents(events);
      events = e;
      }

//---------------------------------------------------------
//   flip
//---------------------------------------------------------

void ChangeBeamProperties::flip()
      {
      double g1 = beam->grow1();
      double g2 = beam->grow2();
      beam->setGrow1(grow1);
      beam->setGrow2(grow2);
      grow1 = g1;
      grow2 = g2;
      }

//---------------------------------------------------------
//   undoChangeBarLine
//---------------------------------------------------------

void Score::undoChangeBarLine(Measure* m, BarLineType barType)
      {
      Score* s = parentScore() ? parentScore() : this;
      QList<Score*> scores;
      scores.append(s);
      foreach (Excerpt* ex, *s->excerpts())
            scores.append(ex->score());
      foreach(Score* score, scores) {
            Measure* measure = score->tick2measure(m->tick());
            Measure* nm      = m->nextMeasure();
            switch(barType) {
                  case END_BAR:
                  case NORMAL_BAR:
                  case DOUBLE_BAR:
                  case BROKEN_BAR:
                        {
                        s->undoChangeRepeatFlags(measure, measure->repeatFlags() & ~RepeatEnd);
                        if (nm)
                              s->undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                        score->undoChangeEndBarLineType(measure, barType);
                        measure->setEndBarLineGenerated (false);
                        }
                        break;
                  case START_REPEAT:
                        s->undoChangeRepeatFlags(measure, measure->repeatFlags() | RepeatStart);
                        break;
                  case END_REPEAT:
                        s->undoChangeRepeatFlags(measure, measure->repeatFlags() | RepeatEnd);
                        if (nm)
                              s->undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                        break;
                  case END_START_REPEAT:
                        s->undoChangeRepeatFlags(measure, measure->repeatFlags() | RepeatEnd);
                        if (nm)
                              s->undoChangeRepeatFlags(nm, nm->repeatFlags() | RepeatStart);
                        break;
                  }
            }
      }



