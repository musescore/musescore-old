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

extern Measure* tick2measure(int tick);

//
// for debugging:
//    keep in sync with enum UndoOp::UndoType
//
static const char* undoName[] = {
      "RemoveElement",      "AddElement",
      "InsertPart",        "RemovePart",
      "InsertStaff",       "RemoveStaff",
      "InsertSegStaff",    "RemoveSegStaff",
      "InsertMStaff",      "RemoveMStaff",
      "InsertMeasure",     "RemoveMeasure",
      "InsertStaves",      "RemoveStaves",
      "SortStaves",        "ToggleInvisible",
      "ChangeColor",       "ChangePitch",
      "ChangeSubtype",     "AddAccidental",
      "SetStemDirection",  "FlipSlurDirection",
      "ChangeKeySig",
      "ChangeClef",
      "ChangeSig",
      "ChangeTempo",
      "ChangeMeasureLen",
      "ChangeElement",
      "ChangeKey",
      "InsertTime",
      "ChangeRepeatFlags",
      "ChangeEndBarLine"
      };

static bool UNDO = false;

//---------------------------------------------------------
//   name
//---------------------------------------------------------

/**
 Return the UndoOp's name. For debugging only.
*/

const char* UndoOp::name() const
      {
      if (type >= int(sizeof(undoName)/sizeof(*undoName))) {
            printf("\n illegal undo type %d\n", type);
            return "???";
            }
      return undoName[type];
      }

//---------------------------------------------------------
//   Undo
//---------------------------------------------------------

Undo::Undo(const InputState& is, const Selection* s)
   : selection(*s)
      {
      inputState = is;
      }

//---------------------------------------------------------
//   doUndo
//---------------------------------------------------------

/**
 Undo a user-visible undo action, containing one or more UndoOp's.
*/

void Score::doUndo()
      {
      if (debugMode)
            printf("doUndo\n");
      Selection oSel(*sel);
      InputState oIs(_is);
      sel->deselectAll(this);
      if (!undoList.isEmpty()) {
            Undo* u = undoList.back();
            int n = u->size();
            for (int i = n-1; i >= 0; --i) {
                  UndoOp* op = &(*u)[i];
                  processUndoOp(op, true);
                  if (op->type == UndoOp::ChangeAccidental) {
                        // HACK:
                        // selection is not valid anymore because changeAccidental()
                        // changes the selected Accidental element
                        // u->selection.clear();
                        oSel.clear();
                        }
                  }
            redoList.push_back(u); // put item on redo list
            undoList.pop_back();
            endUndoRedo(u);
            u->inputState = oIs;
            u->selection  = oSel;
            if (debugMode)
                  printf("  end doUndo\n");
            }
      }

//---------------------------------------------------------
//   doRedo
//---------------------------------------------------------

/**
 Redo a user-visible undo action, containing one or more UndoOp's.
*/

void Score::doRedo()
      {
      if (debugMode)
            printf("doRedo\n");
      Selection oSel(*sel);
      InputState oIs(_is);
      sel->deselectAll(this);
      Undo* u = redoList.back();
      int n = u->size();
      for (int i = n-1; i >= 0; --i) {
            UndoOp* op = &(*u)[i];
            processUndoOp(op, false);
            if (op->type == UndoOp::ChangeAccidental) {
                  // HACK:
                  // selection is not valid anymore because changeAccidental()
                  // changes the selected Accidental element
                  u->selection.clear();
                  }
            }
      undoList.push_back(u); // put item on undo list
      redoList.pop_back();
      endUndoRedo(u);
      u->inputState = oIs;
      u->selection  = oSel;
      if (debugMode)
            printf("  end doRedo\n");
      }

//---------------------------------------------------------
//   processUndoOp
//---------------------------------------------------------

/**
 Process a single low-level undo/redo operation.
*/

void Score::processUndoOp(UndoOp* i, bool undo)
      {
      UNDO = true;

      if (debugMode) {
            printf("Score::processUndoOp(i->type=%s, undo=%d)\n", i->name(), undo);
            if (i->type == UndoOp::RemoveElement)
                  printf("   %s\n", i->obj->name());
            }

      switch (i->type) {
            case UndoOp::RemoveElement:
                  if (undo)
                        addElement(i->obj);
                  else
                        removeElement(i->obj);
                  break;
            case UndoOp::AddElement:
                  if (undo)
                        removeElement(i->obj);
                  else
                        addElement(i->obj);
                  break;
            case UndoOp::InsertPart:
                  if (undo)
                        removePart(i->part);
                  else
                        insertPart(i->part, i->val1);
                  break;
            case UndoOp::RemovePart:
                  if (undo)
                        insertPart(i->part, i->val1);
                  else
                        removePart(i->part);
                  break;
            case UndoOp::InsertStaff:
                  if (undo)
                        removeStaff(i->staff);
                  else
                        insertStaff(i->staff, i->val1);
                  break;
            case UndoOp::RemoveStaff:
                  if (undo)
                        insertStaff(i->staff, i->val1);
                  else
                        removeStaff(i->staff);
                  break;
            case UndoOp::InsertSegStaff:
                  if (undo)
                        i->segment->removeStaff(i->val1);
                  else
                        i->segment->insertStaff(i->val1);
                  break;
            case UndoOp::RemoveSegStaff:
                  if (undo)
                        i->segment->insertStaff(i->val1);
                  else
                        i->segment->removeStaff(i->val1);
                  break;
            case UndoOp::InsertMStaff:
                  if (undo)
                        i->measure->removeMStaff(i->mstaff, i->val1);
                  else
                        i->measure->insertMStaff(i->mstaff, i->val1);
                  break;
            case UndoOp::RemoveMStaff:
                  if (undo)
                        i->measure->insertMStaff(i->mstaff, i->val1);
                  else
                        i->measure->removeMStaff(i->mstaff, i->val1);
                  break;
            case UndoOp::InsertMeasure:
                  if (undo)
                        removeMeasure(i->measure);
                  else
                        addMeasure(i->measure);
                  break;
            case UndoOp::RemoveMeasure:
                  if (undo)
                        addMeasure(i->measure);
                  else
                        removeMeasure(i->measure);
                  break;
            case UndoOp::SortStaves:
                  if (undo)
                        sortStaves(i->di, i->si);
                  else
                        sortStaves(i->si, i->di);
                  break;
            case UndoOp::ToggleInvisible:
                  i->obj->setVisible(!i->obj->visible());
                  break;
            case UndoOp::ChangeColor:
                  {
                  QColor color = i->obj->color();
                  i->obj->setColor(i->color);
                  i->color = color;
                  }
                  break;
            case UndoOp::ChangePitch:
                  {
                  Note* note = (Note*)(i->obj);
                  int pitch  = note->pitch();
                  note->changePitch(i->val1);
                  i->val1 = pitch;
                  }
                  break;
            case UndoOp::ChangeAccidental:
                  {
                  Note* note = (Note*)(i->obj);
                  int pitch  = note->pitch();
                  int tpc    = note->tpc();
                  int acc    = note->accidentalSubtype();
                  note->setPitch(i->val1);
                  note->setTpc(i->val2);
                  note->setAccidentalSubtype(i->val3);
                  i->val1 = pitch;
                  i->val2 = tpc;
                  i->val3 = acc;
                  }
                  break;
            case UndoOp::SetStemDirection:
                  {
                  Chord* chord = (Chord*)(i->obj);
                  Direction dir = chord->stemDirection();
                  chord->setStemDirection((Direction)i->val1);
                  i->val1 = (int)dir;
                  }
                  break;
            case UndoOp::FlipSlurDirection:
                  {
                  SlurTie* slur = (SlurTie*)(i->obj);
                  slur->setSlurDirection(slur->isUp() ? DOWN : UP);
                  }
                  break;
            case UndoOp::ChangeSubtype:
                  {
                  int st = i->obj->subtype();
                  // int t = i->obj->type();
                  i->obj->setSubtype(i->val1);
                  i->val1 = st;
                  }
                  break;

            case UndoOp::ChangeElement:
                  i->obj->parent()->change(i->obj, i->obj2);
                  {
                  // swap
                  Element* e = i->obj;
                  i->obj = i->obj2;
                  i->obj2 = e;
                  }
                  break;

            case UndoOp::InsertStaves:
                  if (undo)
                        i->measure->removeStaves(i->val1, i->val2);
                  else
                        i->measure->insertStaves(i->val1, i->val2);
                  break;
            case UndoOp::RemoveStaves:
                  if (undo)
                        i->measure->insertStaves(i->val1, i->val2);
                  else
                        i->measure->removeStaves(i->val1, i->val2);
                  break;

            case UndoOp::ChangeKeySig:
                  {
                  KeyList* kl = i->staff->keymap();
                  if (undo) {
                        // remove new value if there is any
                        if (i->val3 != -1000) {
                              iKeyEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val2 != -1000)
                              (*kl)[i->val1] = i->val2;
                        }
                  else {
                        if (i->val2 != -1000) {
                              iKeyEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val3 != -1000)
                              (*kl)[i->val1] = i->val3;
                        }
                  }
                  break;

            case UndoOp::ChangeClef:
                  {
                  ClefList* kl = i->staff->clef();
                  if (undo) {
                        // remove new value if there is any
                        if (i->val3 != NO_CLEF) {
                              iClefEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val2 != NO_CLEF)
                              (*kl)[i->val1] = i->val2;
                        }
                  else {
                        if (i->val2 != NO_CLEF) {
                              iClefEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val3 != NO_CLEF)
                              (*kl)[i->val1] = i->val3;
                        }
                  }
                  break;
            case UndoOp::ChangeKey:
                  {
                  KeyList* kl = i->staff->keymap();
                  if (undo) {
                        // remove new value if there is any
                        if (i->val3 != NO_KEY) {
                              iClefEvent ik = kl->find(i->val1);
                              if (ik == kl->end())
                                    printf("UndoOp::ChangeKey1: not found\n");
                              else
                                    kl->erase(ik);
                              }
                        if (i->val2 != NO_KEY)
                              (*kl)[i->val1] = i->val2;
                        }
                  else {
                        if (i->val2 != NO_KEY) {
                              iKeyEvent ik = kl->find(i->val1);
                              if (ik == kl->end())
                                    printf("UndoOp::ChangeKey2: not found\n");
                              else
                                    kl->erase(ik);
                              }
                        if (i->val3 != NO_KEY)
                              (*kl)[i->val1] = i->val3;
                        }
                  }
                  break;
            case UndoOp::ChangeSig:
                  {
                  if (undo) {
                        if (i->sig2.valid())
                              sigmap->del(i->val1);
                        if (i->sig1.valid())
                              sigmap->add(i->val1, i->sig1);
                        }
                  else {
                        if (i->sig1.valid())
                              sigmap->del(i->val1);
                        if (i->sig2.valid())
                              sigmap->add(i->val1, i->sig2);
                        }
                  fixTicks();
                  }
                  break;
            case UndoOp::ChangeTempo:
                  {
                  if (undo) {
                        if (i->t2.valid())
                              tempomap->delTempo(i->val1);
                        if (i->t1.valid())
                              tempomap->addTempo(i->val1, i->t1);
                        }
                  else {
                        if (i->t1.valid())
                              tempomap->delTempo(i->val1);
                        if (i->t2.valid())
                              tempomap->addTempo(i->val1, i->t2);
                        }
                  }
                  break;
            case UndoOp::ChangeMeasureLen:
                  {
                  Measure* m = i->measure;
                  int ol     = m->tickLen();
                  int nl     = i->val1;
                  m->setTickLen(nl);

                  //
                  // move EndBarLin and TimeSigAnnounce
                  // to end of measure:
                  //
                  int staves = nstaves();
                  int endTick = m->tick() + nl;
                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        if (segment->subtype() != Segment::SegEndBarLine
                           && segment->subtype() != Segment::SegTimeSigAnnounce)
                              continue;
                        segment->setTick(endTick);
                        for (int track = 0; track < staves*VOICES; ++track) {
                              if (segment->element(track))
                                    segment->element(track)->setTick(endTick);
                              }
                        }
                  i->val1 = ol;
                  }
                  break;

            case UndoOp::InsertTime:
                  if (undo)
                        insertTime(i->val1, -i->val2);
                  else
                        insertTime(i->val1, i->val2);
                  fixTicks();
                  break;

            case UndoOp::ChangeRepeatFlags:
                  {
                  int tmp = i->measure->repeatFlags();
                  i->measure->setRepeatFlags(i->val1);
                  i->val1 = tmp;
                  }
            case UndoOp::ChangeEndBarLine:
                  {
                  int tmp = i->measure->endBarLineType();
                  i->measure->setEndBarLineType(i->val1, i->val1 != NORMAL_BAR);
                  i->val1 = tmp;
                  }
                  break;
            }
      UNDO = FALSE;
      }

//---------------------------------------------------------
//   endUndoRedo
//---------------------------------------------------------

/**
 Common handling for ending undo or redo
*/

void Score::endUndoRedo(Undo* undo)
      {
      getAction("undo")->setEnabled(!undoList.empty());
      getAction("redo")->setEnabled(!redoList.empty());
      setDirty(true);

      _is = undo->inputState;

      if (!noteEntryMode()) {
            // no input state
            canvas()->setState(Canvas::NORMAL);
            mscore->setState(STATE_NORMAL);
            }
      else {
            // input state
            canvas()->setState(Canvas::NOTE_ENTRY);
            mscore->setState(STATE_NOTE_ENTRY);
            }
      *sel = undo->selection;
      sel->update();
      }

//---------------------------------------------------------
//   checkUndoOp
//---------------------------------------------------------

/**
 Abort with error message if not in undo handling
*/

void Score::checkUndoOp()
      {
      if (!cmdActive) {
            fprintf(stderr, "undoOp: cmd not started\n");
            if (debugMode)
                  abort();
            }
      if (UNDO) {
            fprintf(stderr, "create undo op in undo/redo operation\n");
            if (debugMode)
                  abort();
            }
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Element* object, int val)
      {
      checkUndoOp();
      UndoOp i;
      i.type = type;
      i.obj  = object;
      i.val1 = val;
      undoList.back()->push_back(i);
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Element* object)
      {
      checkUndoOp();
      UndoOp i;
      i.type = type;
      i.obj  = object;
      undoList.back()->push_back(i);
      }

//---------------------------------------------------------
//   undoAddElement
//---------------------------------------------------------

void Score::undoAddElement(Element* element)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::AddElement;
      i.obj  = element;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoInsertTime
//---------------------------------------------------------

void Score::undoInsertTime(int tick, int len)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::InsertTime;
      i.val1  = tick;
      i.val2  = len;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::RemoveElement;
      i.obj  = element;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeMeasureLen
//---------------------------------------------------------

void Score::undoChangeMeasureLen(Measure* m, int tick)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeMeasureLen;
      i.measure  = m;
      i.val1     = tick;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeElement
//---------------------------------------------------------

void Score::undoChangeElement(Element* oldElement, Element* newElement)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeElement;
      i.obj      = oldElement;
      i.obj2     = newElement;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Element* object, const QColor& color)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = type;
      i.obj   = object;
      i.color = color;
      undoList.back()->push_back(i);
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Segment* seg, int staff)
      {
      checkUndoOp();
      UndoOp i;
      i.type    = type;
      i.segment = seg;
      i.val1     = staff;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Part* part, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type = type;
      i.part = part;
      i.val1  = idx;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Staff* staff, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = type;
      i.staff = staff;
      i.val1   = idx;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Measure* m, MStaff s, int staff)
      {
      checkUndoOp();
      UndoOp i;
      i.type    = type;
      i.measure = m;
      i.mstaff  = s;
      i.val1     = staff;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Measure* m)
      {
      checkUndoOp();
      UndoOp i;
      i.type    = type;
      i.measure = m;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Measure* m, int a, int b)
      {
      checkUndoOp();
      UndoOp i;
      i.type    = type;
      i.measure = m;
      i.val1    = a;
      i.val2    = b;
      undoList.back()->push_back(i);
      }

void Score::undoOp(QList<int> si, QList<int> di)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::SortStaves;
      i.si   = si;
      i.di   = di;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, int a, int b)
      {
      checkUndoOp();
      UndoOp i;
      i.type = type;
      i.val1 = a;
      i.val2 = b;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Staff* staff, int tick, int oval, int nval)
      {
      checkUndoOp();
      UndoOp i;
      i.type = type;
      i.staff = staff;
      i.val1 = tick;
      i.val2 = oval;
      i.val3 = nval;
      undoList.back()->push_back(i);
      }

void Score::undoChangeSig(int tick, const SigEvent& o, const SigEvent& n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeSig;
      i.val1 = tick;
      i.sig1 = o;
      i.sig2 = n;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeTempo(int tick, const TEvent& o, const TEvent& n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeTempo;
      i.val1 = tick;
      i.t1 = o;
      i.t2 = n;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeKey(Staff* staff, int tick, int o, int n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeKey;
      i.val1 = tick;
      i.val1 = o;
      i.val3 = n;
      i.staff = staff;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeClef(Staff* staff, int tick, int o, int n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeClef;
      i.val1 = tick;
      i.val1 = o;
      i.val3 = n;
      i.staff = staff;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeRepeatFlags(Measure* m, int flags)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeRepeatFlags;
      i.measure = m;
      i.val1 = flags;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeEndBarLine(Measure* m, int flags)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeEndBarLine;
      i.measure = m;
      i.val1 = flags;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   addElement
//---------------------------------------------------------

/**
 Add \a element to its parent.

 Several elements (clef, keysig, timesig) need special handling, as they may cause
 changes throughout the score.
*/

void Score::addElement(Element* element)
      {
      if (debugMode)
            printf("   Score::addElement %p %s parent %s\n",
               element, element->name(), element->parent()->name());

      element->parent()->add(element);

      if (element->type() == CLEF) {
            int staffIdx = element->staffIdx();
            Clef* clef   = (Clef*) element;
            int tick     = clef->tick();

            //-----------------------------------------------
            //   move notes
            //-----------------------------------------------

            bool endFound = false;
            for (Measure* measure = _layout->first(); measure; measure = measure->next()) {
                  for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                        int startTrack = staffIdx * VOICES;
                        int endTrack   = startTrack + VOICES;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* ie = segment->element(track);
                              if (ie && ie->type() == CLEF && ie->tick() > tick) {
                                    endFound = true;
                                    break;
                                    }
                              }
                        if (endFound)
                              break;
                        }
                  if (endFound)
                        break;
                  }
            }
      else if (element->type() == KEYSIG) {
            // FIXME: update keymap here (and remove that from Score::changeKeySig)
            // but only after fixing redo for elements contained in segments

            // fixup all accidentals
            layoutAll = true;
            }
      }

//---------------------------------------------------------
//   removeElement
//---------------------------------------------------------

/**
 Remove \a element from its parent.

 Several elements (clef, keysig, timesig) need special handling, as they may cause
 changes throughout the score.
*/

void Score::removeElement(Element* element)
      {
      Element* parent = element->parent();

      if (debugMode)
            printf("   Score::removeElement %p %s parent %p %s\n",
               element, element->name(), parent, parent->name());

      parent->remove(element);
      if (element->type() == CLEF) {
            Clef* clef   = (Clef*)element;
            int tick     = clef->tick();
            int staffIdx = clef->staffIdx();

            //-----------------------------------------------
            //   move notes
            //-----------------------------------------------

            bool endFound = false;
            for (Measure* measure = _layout->first(); measure; measure = measure->next()) {
                  for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                        int startTrack = staffIdx * VOICES;
                        int endTrack   = startTrack + VOICES;
                        for (int track = startTrack; track < endTrack; ++track) {
                              Element* ie = segment->element(track);
                              if (ie && ie->type() == CLEF && ie->tick() > tick) {
                                    endFound = true;
                                    break;
                                    }
                              }
                        if (endFound)
                              break;
                        }
                  if (endFound)
                        break;
                  }
            }
      else if (element->type() == KEYSIG) {
            layoutAll = true;
            }
      }

