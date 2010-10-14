//=============================================================================
//  MuseScore
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

#include "splitstaff.h"
#include "score.h"
#include "staff.h"
#include "clef.h"
#include "measure.h"
#include "part.h"
#include "note.h"
#include "chord.h"
#include "bracket.h"
#include "system.h"
#include "seq.h"
#include "slur.h"
#include "segment.h"

//---------------------------------------------------------
//   SplitStaff
//---------------------------------------------------------

SplitStaff::SplitStaff(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      splitPoint->setValue(60);
      }

//---------------------------------------------------------
//   splitStaff
//---------------------------------------------------------

struct SNote {
      int tick;
      int pitch;
      Fraction fraction;
      Note* note;
      };

void Score::splitStaff(int staffIdx, int splitPoint)
      {
      printf("split staff %d point %d\n", staffIdx, splitPoint);

      //
      // create second staff
      //
      Staff* s  = staff(staffIdx);
      Part*  p  = s->part();
      int rstaff = s->rstaff();
      Staff* ns = new Staff(this, p, rstaff + 1);
      ns->setRstaff(rstaff + 1);
      ns->clefList()->setClef(0, CLEF_F);

      undoInsertStaff(ns, staffIdx+1);

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->cmdAddStaves(staffIdx+1, staffIdx+2, false);

//      undoChangeBarLineSpan(s, p->nstaves());
      adjustBracketsIns(staffIdx+1, staffIdx+2);
      undoChangeKeySig(ns, 0, s->key(0));

//      Bracket* b = new Bracket(this);
//      b->setSubtype(BRACKET_AKKOLADE);
//      b->setTrack(staffIdx * VOICES);
//      b->setParent(firstMeasure()->system());
//      b->setLevel(-1);  // add bracket
//      b->setSpan(2);
//      cmdAdd(b);

      rebuildMidiMapping();
      seq->initInstruments();
      startLayout = 0;
      doLayout();

      //
      // move notes
      //
      select(0, SELECT_SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;

      for (Segment* s = firstSegment(SegChordRest); s; s = s->next1(SegChordRest)) {
            for (int voice = 0; voice < VOICES; ++voice) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack + voice));
                  if (cr == 0 || cr->type() == REST)
                        continue;
                  Chord* c = static_cast<Chord*>(cr);
                  QList<Note*> removeNotes;
                  foreach(Note* note, c->notes()) {
                        if (note->pitch() >= splitPoint)
                              continue;
                        Chord* chord = static_cast<Chord*>(s->element(dtrack + voice));
                        if (chord && (chord->type() != CHORD))
                              abort();
                        if (chord == 0) {
                              chord = new Chord(*c);
                              foreach(Note* note, chord->notes())
                                    delete note;
                              chord->notes().clear();
                              chord->setTrack(dtrack + voice);
                              undoAddElement(chord);
                              }
                        Note* nnote = new Note(*note);
                        nnote->setTrack(dtrack + voice);
                        chord->add(nnote);
                        removeNotes.append(note);
                        }
                  foreach(Note* note, removeNotes) {
                        undoRemoveElement(note);
                        if (note->chord()->notes().isEmpty())
                              undoRemoveElement(note->chord());
                        }
                  }
            }
      //
      // make sure that the timeline for dtrack
      // has no gaps
      //
      int ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              printf("no segment at %d\n", ctick);
                              continue;
                              }
                        setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->ticks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, dtrack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      //
      // same for strack
      //
      ctick  = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            for (Segment* s = m->first(SegChordRest); s; s = s->next1(SegChordRest)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(strack));
                  if (cr == 0)
                        continue;
                  int rest = s->tick() - ctick;
                  if (rest) {
                        // insert Rest
                        Segment* s = tick2segment(ctick);
                        if (s == 0) {
                              printf("no segment at %d\n", ctick);
                              continue;
                              }
                        setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                        }
                  ctick = s->tick() + cr->ticks();
                  }
            int rest = m->tick() + m->ticks() - ctick;
            if (rest) {
                  setRest(ctick, strack, Fraction::fromTicks(rest), false, 0);
                  ctick += rest;
                  }
            }
      }

