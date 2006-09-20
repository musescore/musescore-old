//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: undo.cpp,v 1.40 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

extern Measure* tick2measure(int tick);

extern QAction* undoAction;
extern QAction* redoAction;

static const char* undoName[] = {
      "RemoveObject", "AddObject",
      "InsertPart", "RemovePart",
      "InsertStaff", "RemoveStaff",
      "InsertSegStaff", "RemoveSegStaff",
      "InsertMStaff", "RemoveMStaff",
      "InsertMeasure", "RemoveMeasure",
      "SortStaves",
      "ToggleInvisible",
      "ChangeColor",
      "ChangePitch",
      "AddAccidental",
      };

//---------------------------------------------------------
//   name
//---------------------------------------------------------

const char* UndoOp::name() const
      {
      return undoName[type];
      }

//---------------------------------------------------------
//   Undo
//---------------------------------------------------------

Undo::Undo(const InputState& is, const Selection& s)
      {
      inputState = is;
      selection  = s;
      }

Undo::~Undo()
      {
      }

//---------------------------------------------------------
//    startUndo
//---------------------------------------------------------

void Score::startUndo()
      {
// printf("start undo\n");
      if (undoActive) {
            fprintf(stderr, "startUndo: already active\n");
            abort();
            }
      undoList.push_back(new Undo(*cis, *sel));

/*      ElementList*el = sel->elements();
printf("StartUndo: ==store selection %d\n", el->size());
      for (iElement i = el->begin(); i != el->end(); ++i)
            printf("  sel %p\n", *i);
  */
      undoActive = true;
      }

//---------------------------------------------------------
//   endUndo
//---------------------------------------------------------

void Score::endUndo()
      {
// printf("   end undo\n");
      if (!undoActive) {
//            fprintf(stderr, "endUndo: not active\n");
            // abort();
            return;
            }

      if (undoList.back()->empty()) {
            // nothing to undo
            delete undoList.back();
            undoList.pop_back();
            }
      else {
#if 0
            Undo* u = undoList.back();

            ElementList*el = u->selection.elements();
printf("EndUndo: %d ==store selection %d\n", undoList.back()->size(), el->size());
            for (iElement i = el->begin(); i != el->end(); ++i)
                  printf("  sel %p\n", *i);
#endif
            setDirty(true);
            for (iUndo i = redoList.begin(); i != redoList.end(); ++i)
                  delete *i;
            redoList.clear();
            undoAction->setEnabled(true);
            redoAction->setEnabled(false);
            }
      undoActive = false;
      }

//---------------------------------------------------------
//   doUndo
//---------------------------------------------------------

void Score::doUndo()
      {
      Selection oSel(*sel);
      InputState oIs(*cis);
      sel->deselectAll(this);
      Undo* u = undoList.back();
//printf("doUndo %d\n", u->size());
      for (riUndoOp i = u->rbegin(); i != u->rend(); ++i) {
//printf("   doUndo %s\n", i->name());
            switch(i->type) {
                  case UndoOp::RemoveObject:
//printf("      %d %d\n", i->obj->type(), i->obj->subtype());
                        addObject(i->obj);
                        break;
                  case UndoOp::AddObject:
//printf("      %d %d\n", i->obj->type(), i->obj->subtype());
                        removeObject(i->obj);
                        break;
                  case UndoOp::InsertPart:
                        removePart(i->part);
                        break;
                  case UndoOp::RemovePart:
                        insertPart(i->part, i->idx);
                        break;
                  case UndoOp::InsertStaff:
                        removeStaff(i->staff);
                        break;
                  case UndoOp::RemoveStaff:
                        insertStaff(i->staff, i->idx);
                        break;
                  case UndoOp::InsertSegStaff:
                        i->segment->removeStaff(i->idx);
                        break;
                  case UndoOp::RemoveSegStaff:
                        i->segment->insertStaff(i->idx);
                        break;
                  case UndoOp::InsertMStaff:
                        i->measure->removeMStaff(i->mstaff, i->idx);
                        break;
                  case UndoOp::RemoveMStaff:
                        i->measure->insertMStaff(i->mstaff, i->idx);
                        break;
                  case UndoOp::InsertMeasure:
                        removeMeasure(i->measure->tick());
                        break;
                  case UndoOp::RemoveMeasure:
                        addMeasure(i->measure);
                        break;
                  case UndoOp::SortStaves:
                        sortStaves(i->di, i->si);
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
                        note->changePitch(i->idx);
                        i->idx = pitch;
                        }
                        break;
                  case UndoOp::ChangeAccidental:
                        {
                        Note* note = (Note*)(i->obj);
                        int accidental  = note->userAccidental();
                        note->changeAccidental(i->idx);
                        i->idx = accidental;
                        }
                        break;
                  }
            }
      redoList.push_back(u); // put item on redo list
      undoList.pop_back();
      endUndoRedo(u);
      u->inputState = oIs;
      u->selection  = oSel;
      }

//---------------------------------------------------------
//   doRedo
//---------------------------------------------------------

void Score::doRedo()
      {
      Selection oSel(*sel);
      InputState oIs(*cis);

      sel->deselectAll(this);
      Undo* u = redoList.back();
//printf("doRedo %d\n", u->size());
      for (iUndoOp i = u->begin(); i != u->end(); ++i) {
//printf("   doRedo %s\n", i->name());
            switch(i->type) {
                  case UndoOp::RemoveObject:
//printf("      %d %d\n", i->obj->type(), i->obj->subtype());
                        removeObject(i->obj);
                        break;
                  case UndoOp::AddObject:
//printf("      %d %d\n", i->obj->type(), i->obj->subtype());
                        addObject(i->obj);
                        break;
                  case UndoOp::InsertPart:
                        insertPart(i->part, i->idx);
                        break;
                  case UndoOp::RemovePart:
                        removePart(i->part);
                        break;
                  case UndoOp::InsertStaff:
                        insertStaff(i->staff, i->idx);
                        break;
                  case UndoOp::RemoveStaff:
                        removeStaff(i->staff);
                        break;
                  case UndoOp::InsertSegStaff:
                        i->segment->insertStaff(i->idx);
                        break;
                  case UndoOp::RemoveSegStaff:
                        i->segment->removeStaff(i->idx);
                        break;
                  case UndoOp::InsertMStaff:
                        i->measure->insertMStaff(i->mstaff, i->idx);
                        break;
                  case UndoOp::RemoveMStaff:
                        i->measure->removeMStaff(i->mstaff, i->idx);
                        break;
                  case UndoOp::InsertMeasure:
                        addMeasure(i->measure);
                        break;
                  case UndoOp::RemoveMeasure:
                        i->measure->score()->removeMeasure(i->measure->tick());
                        break;
                  case UndoOp::SortStaves:
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
                        note->changePitch(i->idx);
                        i->idx = pitch;
                        }
                        break;
                  case UndoOp::ChangeAccidental:
                        {
                        Note* note = (Note*)(i->obj);
                        int accidental  = note->userAccidental();
                        note->changeAccidental(i->idx);
                        i->idx = accidental;
                        }
                        break;
                  }
            }
      undoList.push_back(u); // put item on undo list
      redoList.pop_back();
      endUndoRedo(u);
      u->inputState = oIs;
      u->selection  = oSel;
      }

//---------------------------------------------------------
//   endUndoRedo
//---------------------------------------------------------

void Score::endUndoRedo(Undo* undo)
      {
      undoAction->setEnabled(!undoList.empty());
      redoAction->setEnabled(!redoList.empty());
      setDirty(true);

      *cis = undo->inputState;
      moveCursor();
      *sel = undo->selection;
      sel->update();
      layout();
      endCmd(false);
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Element* object)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      assert(type == UndoOp::RemoveObject || type == UndoOp::AddObject);
      UndoOp i;
      i.type = type;
      i.obj  = object;
      undoList.back()->push_back(i);
      }

//---------------------------------------------------------
//   undoOp
//---------------------------------------------------------

void Score::undoOp(UndoOp::UndoType type, Element* object, const QColor& color)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      assert(type == UndoOp::RemoveObject || type == UndoOp::AddObject);
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
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      assert(type == UndoOp::InsertSegStaff || type == UndoOp::RemoveSegStaff);
      UndoOp i;
      i.type    = type;
      i.segment = seg;
      i.idx     = staff;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Part* part, int idx)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      UndoOp i;
      i.type = type;
      i.part = part;
      i.idx  = idx;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Staff* staff, int idx)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      UndoOp i;
      i.type  = type;
      i.staff = staff;
      i.idx   = idx;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Measure* m, MStaff s, int staff)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      UndoOp i;
      i.type    = type;
      i.measure = m;
      i.mstaff  = s;
      i.idx     = staff;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, Measure* m)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      UndoOp i;
      i.type    = type;
      i.measure = m;
      undoList.back()->push_back(i);
      }

void Score::undoOp(std::list<int> si, std::list<int> di)
      {
      if (!undoActive) {
            fprintf(stderr, "undoOp: undo not started\n");
            abort();
            }
      UndoOp i;
      i.type = UndoOp::SortStaves;
      i.si   = si;
      i.di   = di;
      undoList.back()->push_back(i);
      }

//---------------------------------------------------------
//   addObject
//---------------------------------------------------------

void Score::addObject(Element* element)
      {
// printf("Score::addObject %p %s parent %s\n", element, element->name(), element->parent()->name());
      element->parent()->add(element);

      if (element->type() == CLEF) {
            int staffIdx = element->staffIdx();
            Clef* clef   = (Clef*) element;
            ClefList* ct = staff(staffIdx)->clef();
            int tick     = clef->tick();
            ct->setClef(clef->tick(), clef->subtype());

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
                        measure->layoutNoteHeads(staffIdx);
                        if (endFound)
                              break;
                        }
                  if (endFound)
                        break;
                  }
            }
/*      else if (element->type() == SLUR_SEGMENT) {
            SlurSegment* ss = (SlurSegment*)element;
            SlurTie* slur = ss->slurTie();
            slur->add(element);
            }
      */
      }

//---------------------------------------------------------
//   removeObject
//---------------------------------------------------------

void Score::removeObject(Element* element)
      {
      Element* parent = element->parent();

// printf("Score::removeObject %p %s parent %p %s\n", 
//   element, element->name(), parent, parent->name());

      parent->remove(element);
      if (element->type() == CLEF) {
            Clef* clef = (Clef*)element;
            int tick  = clef->tick();
            int staffIdx = clef->staffIdx();

            Staff* instr = staff(staffIdx);
            ClefList* ct = instr->clef();
            ct->erase(tick);

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
                        measure->layoutNoteHeads(staffIdx);
                        if (endFound)
                              break;
                        }
                  if (endFound)
                        break;
                  }
            }
/*      else if (element->type() == SLUR_SEGMENT) {
            SlurSegment* ss = (SlurSegment*)element;
            SlurTie* slur = ss->slurTie();
            slur->remove(element);
            }
      */
      }

