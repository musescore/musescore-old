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
      "SetStemDirection",  "FlipSlurDirection", "FlipBeamDirection",
      "ChangeKeySig",
      "ChangeClef",
      "ChangeSig",
      "ChangeTempo",
      "ChangeMeasureLen",
      "ChangeElement",
      "ChangeKey",
      "InsertTime",
      "ChangeRepeatFlags",
      "ChangeVoltaEnding", "ChangeVoltaText",
      "ChangeChordRestSize",
      "ChangeNoteHead",
      "ChangeEndBarLineType",
      "ChangeBarLineSpan",
      "SigInsertTime",
      "FixTicks",
      "ChangeBeamMode",
      "ChangeCopyright",
      "TransposeHarmony",
      "ExchangeVoice",
      "ChangeConcertPitch",
      "ChangeInstrumentShort", "ChangeInstrumentLong",
      "ChangeChordRestLen"
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
      pitchSpellNeeded = false;
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
            printf("doUndo %d redo: %d\n", undoList.size(), redoList.size());
      Selection oSel(*sel);
      InputState oIs(_is);
      sel->deselectAll(this);
      if (undoList.isEmpty()) {
            printf("doUndo: undoList is empty!\n");
            getAction("undo")->setEnabled(false);
            return;
            }
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
      if (u->pitchSpellNeeded)
            spell();
      redoList.append(u); // put item on redo list
      undoList.removeLast();
      endUndoRedo(u);
      u->inputState = oIs;
      u->selection  = oSel;
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
      for (int i = 0; i < n; ++i) {
            UndoOp* op = &(*u)[i];
            processUndoOp(op, false);
            if (op->type == UndoOp::ChangeAccidental) {
                  // HACK:
                  // selection is not valid anymore because changeAccidental()
                  // changes the selected Accidental element
                  u->selection.clear();
                  }
            }
      if (u->pitchSpellNeeded)
            spell();
      undoList.append(u); // put item on undo list
      redoList.removeLast();
      endUndoRedo(u);
      u->inputState = oIs;
      u->selection  = oSel;
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
                  printf("   %s\n", i->element1->name());
            }

      switch (i->type) {
            case UndoOp::RemoveElement:
                  if (undo)
                        addElement(i->element1);
                  else
                        removeElement(i->element1);
                  break;
            case UndoOp::AddElement:
                  if (undo)
                        removeElement(i->element1);
                  else
                        addElement(i->element1);
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
                        ((Measure*)i->measure)->removeMStaff(i->mstaff, i->val1);
                  else
                        ((Measure*)i->measure)->insertMStaff(i->mstaff, i->val1);
                  break;
            case UndoOp::RemoveMStaff:
                  if (undo)
                        ((Measure*)i->measure)->insertMStaff(i->mstaff, i->val1);
                  else
                        ((Measure*)i->measure)->removeMStaff(i->mstaff, i->val1);
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
                  sortStaves(i->si);
                  break;
            case UndoOp::ToggleInvisible:
                  i->element1->setVisible(!i->element1->visible());
                  break;
            case UndoOp::ChangeColor:
                  {
                  QColor color = i->element1->color();
                  i->element1->setColor(i->color);
                  i->color = color;
                  }
                  break;
            case UndoOp::ChangePitch:
                  {
                  Note* note = (Note*)(i->element1);
                  int pitch  = note->pitch();
                  note->changePitch(i->val1);
                  i->val1 = pitch;
                  }
                  break;
            case UndoOp::ChangeAccidental:
                  {
                  Note* note = (Note*)(i->element1);
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
                  Chord* chord = (Chord*)(i->element1);
                  Direction dir = chord->stemDirection();
                  chord->setStemDirection((Direction)i->val1);
                  i->val1 = (int)dir;
                  }
                  break;
            case UndoOp::FlipSlurDirection:
                  {
                  SlurTie* slur = (SlurTie*)(i->element1);
                  slur->setSlurDirection(slur->isUp() ? DOWN : UP);
                  }
                  break;
            case UndoOp::FlipBeamDirection:
                  {
                  Beam* beam = static_cast<Beam*>(i->element1);
                  beam->setBeamDirection(beam->isUp() ? DOWN : UP);
                  }
                  break;
            case UndoOp::ChangeSubtype:
                  {
                  int st = i->element1->subtype();
                  i->element1->setSubtype(i->val1);
                  i->val1 = st;
                  }
                  break;

            case UndoOp::ChangeElement:
                  i->element1->parent()->change(i->element1, i->element2);
                  {
                  // swap
                  Element* e = i->element1;
                  i->element1 = i->element2;
                  i->element2 = e;
                  }
                  break;

            case UndoOp::InsertStaves:
                  if (undo)
                        ((Measure*)i->measure)->removeStaves(i->val1, i->val2);
                  else
                        ((Measure*)i->measure)->insertStaves(i->val1, i->val2);
                  break;
            case UndoOp::RemoveStaves:
                  if (undo)
                        ((Measure*)i->measure)->insertStaves(i->val1, i->val2);
                  else
                        ((Measure*)i->measure)->removeStaves(i->val1, i->val2);
                  break;

            case UndoOp::ChangeKeySig:
                  {
                  KeyList* kl = i->staff->keymap();
                  if (undo) {
                        // remove new value if there is any
                        if (i->val3 != NO_KEY) {
                              iKeyEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val2 != NO_KEY)
                              (*kl)[i->val1] = i->val2;
                        }
                  else {
                        if (i->val2 != NO_KEY) {
                              iKeyEvent ik = kl->find(i->val1);
                              kl->erase(ik);
                              }
                        if (i->val3 != NO_KEY)
                              (*kl)[i->val1] = i->val3;
                        }
                  }
                  break;

            case UndoOp::ChangeClef:
                  {
                  ClefList* kl = i->staff->clefList();
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
                                    printf("UndoOp::ChangeKey1 %d: not found\n", i->val1);
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
                                    printf("UndoOp::ChangeKey2 %d: not found\n", i->val1);
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
//                  fixTicks();
                  }
                  break;
            case UndoOp::FixTicks:
                  fixTicks();
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
                  Measure* m = (Measure*)i->measure;
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
                  break;

            case UndoOp::SigInsertTime:
                  {
                  int len = undo ? -i->val2 : i->val2;
                  if (len < 0)
                        sigmap->removeTime(i->val1, -len);
                  else
                        sigmap->insertTime(i->val1, len);
                  }
                  break;

            case UndoOp::ChangeRepeatFlags:
                  {
                  int tmp = ((Measure*)i->measure)->repeatFlags();
                  ((Measure*)i->measure)->setRepeatFlags(i->val1);
                  i->val1 = tmp;
                  }
                  break;
            case UndoOp::ChangeVoltaEnding:
                  {
                  Volta* volta = (Volta*)i->element1;
                  QList<int> l = volta->endings();
                  volta->setEndings(i->si);
                  i->si = l;
                  }
                  break;
            case UndoOp::ChangeVoltaText:
                  {
                  Volta* volta = (Volta*)i->element1;
                  QString s = volta->text();
                  volta->setText(i->s);
                  i->s = s;
                  }
                  break;
            case UndoOp::ChangeChordRestSize:
                  {
                  ChordRest* cr = (ChordRest*)i->element1;
                  bool small = cr->small();
                  cr->setSmall(i->val1);
                  i->val1 = small;
                  }
                  break;
            case UndoOp::ChangeNoteHead:
                  {
                  Note* note = (Note*)i->element1;
                  int headGroup = note->headGroup();
                  note->setHeadGroup(i->val1);
                  i->val1 = headGroup;
                  }
                  break;
            case UndoOp::ChangeBeamMode:
                  {
                  ChordRest* cr = (ChordRest*)i->element1;
                  int mode = int(cr->beamMode());
                  cr->setBeamMode(BeamMode(i->val1));
                  i->val1 = int(mode);
                  }
                  break;
            case UndoOp::ChangeEndBarLineType:
                  {
                  Measure* m = (Measure*)i->measure;
                  int typ    = m->endBarLineType();
                  m->setEndBarLineType(i->val1, false);
                  i->val1 = typ;
                  }
                  break;
            case UndoOp::ChangeBarLineSpan:
                  {
                  int span = i->staff->barLineSpan();
                  i->staff->setBarLineSpan(i->val1);
                  i->val1 = span;
                  }
                  break;
            case UndoOp::ChangeCopyright:
                  {
                  QString s;
                  if (rights)
                        s = rights->toHtml("UTF-8");
                  setCopyrightHtml(i->s);
                  i->s = s;
                  }
                  break;
            case UndoOp::TransposeHarmony:
                  {
                  Harmony* h = static_cast<Harmony*>(i->element1);
                  int semitones = i->val1;
                  if (undo)
                        semitones = -semitones;
                  int baseTpc = h->baseTpc();
                  int rootTpc = h->rootTpc();
                  h->setBaseTpc(transposeTpc(baseTpc, semitones));
                  h->setRootTpc(transposeTpc(rootTpc, semitones));
                  h->buildText();
                  }
                  break;
            case UndoOp::ExchangeVoice:
                  {
                  Measure* m = static_cast<Measure*>(i->element1);
                  if (undo)
                        m->exchangeVoice(i->val2, i->val1, i->val3, i->val4);
                  else
                        m->exchangeVoice(i->val1, i->val2, i->val3, i->val4);
                  }
                  break;
            case UndoOp::ChangeConcertPitch:
                  {
                  int oval = int(_style->concertPitch);
                  _style->concertPitch = bool(i->val1);
                  QAction* action = getAction("concert-pitch");
                  action->setChecked(_style->concertPitch);
                  i->val1 = oval;
                  }
                  break;
            case UndoOp::ChangeInstrumentShort:
                  {
                  QString s = i->part->shortNameHtml();
                  i->part->setShortNameHtml(i->s);
                  i->s = s;
                  }
                  break;
            case UndoOp::ChangeInstrumentLong:
                  {
                  QString s = i->part->longNameHtml();
                  i->part->setLongNameHtml(i->s);
                  i->s = s;
                  }
                  break;
            case UndoOp::ChangeChordRestLen:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(i->element1);
                  int oldLen = cr->tickLen();
                  cr->setLen(i->val1);
                  i->val1 = oldLen;
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
      setDirty(true);

      _is = undo->inputState;

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

      *sel = undo->selection;
      sel->update();
      getAction("undo")->setEnabled(!undoEmpty());
      getAction("redo")->setEnabled(!redoEmpty());
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
            undoList.push_back(new Undo(_is, sel));
            cmdActive = true;
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
      i.element1 = object;
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
      i.element1 = object;
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
      i.element1 = element;
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
//   undoFixTicks
//---------------------------------------------------------

void Score::undoFixTicks()
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::FixTicks;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoSetPitchSpellNeeded
//---------------------------------------------------------

void Score::undoSetPitchSpellNeeded()
      {
      undoList.back()->pitchSpellNeeded = true;
      }

//---------------------------------------------------------
//   undoRemoveElement
//---------------------------------------------------------

void Score::undoRemoveElement(Element* element)
      {
      if (element->isChordRest()) {
            // remove any slurs pointing to this chor/rest
            ChordRest* cr = (ChordRest*)element;
            foreach(Slur* slur, cr->slurFor()) {
                  undoRemoveElement(slur);
                  }
            foreach(Slur* slur, cr->slurBack()) {
                  undoRemoveElement(slur);
                  }
            if (cr->tuplet() && cr->tuplet()->elements().empty())
                  undoRemoveElement(cr->tuplet());
            }
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::RemoveElement;
      i.element1 = element;
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
      i.element1 = oldElement;
      i.element2 = newElement;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeSubtype
//---------------------------------------------------------

void Score::undoChangeSubtype(Element* element, int st)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeSubtype;
      i.element1 = element;
      i.val1     = st;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeNoteHead
//---------------------------------------------------------

void Score::undoChangeNoteHead(Note* note, int group)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeNoteHead;
      i.element1 = note;
      i.val1     = group;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangePitch
//---------------------------------------------------------

void Score::undoChangePitch(Note* note, int pitch)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangePitch;
      i.element1 = note;
      i.val1     = pitch;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeBeamMode
//---------------------------------------------------------

void Score::undoChangeBeamMode(ChordRest* cr, int mode)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeBeamMode;
      i.element1 = cr;
      i.val1     = int(mode);
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeChordRestLen
//---------------------------------------------------------

void Score::undoChangeChordRestLen(ChordRest* cr, int len)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeChordRestLen;
      i.element1 = cr;
      i.val1     = len;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeEndBarLineType
//---------------------------------------------------------

void Score::undoChangeEndBarLineType(Measure* m, int subtype)
      {
      checkUndoOp();
      UndoOp i;
      i.type     = UndoOp::ChangeEndBarLineType;
      i.measure  = m;
      i.val1     = subtype;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeBarLineSpan
//---------------------------------------------------------

void Score::undoChangeBarLineSpan(Staff* staff, int span)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = UndoOp::ChangeBarLineSpan;
      i.staff = staff;
      i.val1  = span;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeCopyright
//---------------------------------------------------------

void Score::undoChangeCopyright(const QString& s)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = UndoOp::ChangeCopyright;
      i.s    = s;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoTransposeHarmony
//---------------------------------------------------------

void Score::undoTransposeHarmony(Harmony* h, int semitones)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = UndoOp::TransposeHarmony;
      i.element1 = h;
      i.val1     = semitones;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoExchangeVoice
//---------------------------------------------------------

void Score::undoExchangeVoice(Measure* measure, int val1, int val2, int staff1, int staff2)
      {
      checkUndoOp();
      UndoOp i;
      i.type  = UndoOp::ExchangeVoice;
      i.element1 = measure;
      i.val1     = val1;
      i.val2     = val2;
      i.val3     = staff1;
      i.val4     = staff2;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoRemovePart
//---------------------------------------------------------

void Score::undoRemovePart(Part* part, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::RemovePart;
      i.part = part;
      i.val1  = idx;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoInsertPart
//---------------------------------------------------------

void Score::undoInsertPart(Part* part, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::InsertPart;
      i.part = part;
      i.val1  = idx;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoRemoveStaff
//---------------------------------------------------------

void Score::undoRemoveStaff(Staff* staff, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::RemoveStaff;
      i.staff = staff;
      i.val1  = idx;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoInsertStaff
//---------------------------------------------------------

void Score::undoInsertStaff(Staff* staff, int idx)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::InsertStaff;
      i.staff = staff;
      i.val1  = idx;
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
      i.element1 = object;
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

#if 0
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
#endif

void Score::undoOp(UndoOp::UndoType type, Measure* m, MStaff* s, int staff)
      {
      checkUndoOp();
      UndoOp i;
      i.type    = type;
      i.measure = m;
      i.mstaff  = s;
      i.val1     = staff;
      undoList.back()->push_back(i);
      }

void Score::undoOp(UndoOp::UndoType type, MeasureBase* m)
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

void Score::undoOp(QList<int> si)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::SortStaves;
      i.si   = si;
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

void Score::undoSigInsertTime(int tick, int len)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::SigInsertTime;
      i.val1 = tick;
      i.val2 = len;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeTempo(int tick, const TEvent& o, const TEvent& n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeTempo;
      i.val1 = tick;
      i.t1   = o;
      i.t2   = n;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

void Score::undoChangeKey(Staff* staff, int tick, int o, int n)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeKey;
      i.val1 = tick;
      i.val2 = o;
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
      i.val2 = o;
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

//---------------------------------------------------------
//   undoChangeVoltaEnding
//---------------------------------------------------------

void Score::undoChangeVoltaEnding(Volta* volta, const QList<int>& l)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeVoltaEnding;
      i.element1 = volta;
      i.si   = l;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeVoltaText
//---------------------------------------------------------

void Score::undoChangeVoltaText(Volta* volta, const QString& s)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeVoltaText;
      i.element1 = volta;
      i.s    = s;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);
      }

//---------------------------------------------------------
//   undoChangeChordRestSize
//---------------------------------------------------------

void Score::undoChangeChordRestSize(ChordRest* cr, bool small)
      {
      checkUndoOp();
      UndoOp i;
      i.type = UndoOp::ChangeChordRestSize;
      i.element1 = cr;
      i.val1 = small;
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

      if (element->type() == MEASURE
         || (element->type() == HBOX && element->parent()->type() != VBOX)
         || element->type() == VBOX
         ) {
            _layout->add(element);
            return;
            }

      element->parent()->add(element);

      if (element->type() == CLEF) {
            int staffIdx = element->staffIdx();
            Clef* clef   = (Clef*) element;
            int tick     = clef->tick();

            //-----------------------------------------------
            //   move notes
            //-----------------------------------------------

            bool endFound = false;
            for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* measure = (Measure*)mb;
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
      else if (element->type() == SLUR) {
            Slur* s = (Slur*)element;
            ((ChordRest*)s->startElement())->addSlurFor(s);
            ((ChordRest*)s->endElement())->addSlurBack(s);
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

      // special for MEASURE, HBOX, VBOX
      // their parent is not static

      if (element->type() == MEASURE || element->type() == HBOX || element->type() == VBOX) {
            _layout->remove(element);
            return;
            }
      parent->remove(element);

      switch(element->type()) {
            case CHORD:
            case REST:
                  {
                  ChordRest* cr = static_cast<ChordRest*>(element);
                  cr->setBeam(0);
                  }
                  break;
            case CLEF:
                  {
                  Clef* clef   = static_cast<Clef*>(element);
                  int tick     = clef->tick();
                  int staffIdx = clef->staffIdx();

                  //-----------------------------------------------
                  //   move notes
                  //-----------------------------------------------

                  bool endFound = false;
                  for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
                        if (mb->type() != MEASURE)
                              continue;
                        Measure* measure = static_cast<Measure*>(mb);
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
                  break;
            case KEYSIG:
                  layoutAll = true;
                  break;
            case SLUR:
                  {
                  Slur* s = static_cast<Slur*>(element);
                  static_cast<ChordRest*>(s->startElement())->removeSlurFor(s);
                  static_cast<ChordRest*>(s->endElement())->removeSlurBack(s);
                  }
                  break;
            default:
                  break;
            }
      }

