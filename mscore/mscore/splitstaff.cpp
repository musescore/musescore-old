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
      Staff* ns = new Staff(this, p, 1);
      ns->setRstaff(1);
      ns->clefList()->setClef(0, CLEF_F);

      undoInsertStaff(ns, staffIdx+1);

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure())
            m->cmdAddStaves(staffIdx+1, staffIdx+2);
      undoChangeBarLineSpan(s, p->nstaves());
      adjustBracketsIns(staffIdx+1, staffIdx+2);
      ns->changeKeySig(0, s->key(0));

      Bracket* b = new Bracket(this);
      b->setSubtype(BRACKET_AKKOLADE);
      b->setTrack(staffIdx * VOICES);
      b->setParent(firstMeasure()->system());
      b->setLevel(-1);  // add bracket
      b->setSpan(2);
      cmdAdd(b);

      rebuildMidiMapping();
      seq->initInstruments();
      startLayout = 0;
      doLayout();

      QList<SNote>notes;

      //
      // move notes
      //    for now we only move notes from voice 0
      //
      select(0, SELECT_SINGLE, 0);
      int strack = staffIdx * VOICES;
      int dtrack = (staffIdx + 1) * VOICES;
      QList<Note*> notesToRemove;
      for (Segment* s = firstSegment(); s; s = s->next1()) {
            if (s->subtype() != Segment::SegChordRest)
                  continue;
            ChordRest* cr = static_cast<ChordRest*>(s->element(strack));
            if (cr == 0 || cr->type() == REST)
                  continue;
            Chord* c = static_cast<Chord*>(cr);
            NoteList* nl = c->noteList();
            for (iNote i = nl->begin(); i != nl->end(); ++i) {
                  Note* note = i->second;
                  if (note->pitch() < splitPoint) {
                        if (note->tieBack()) {
                              notesToRemove.append(note);
                              continue;
                              }
                        SNote n;
                        n.tick     = c->tick();
                        n.pitch    = note->pitch();
                        n.fraction = c->duration().fraction();
                        Note* nn = note;
                        while (nn && nn->tieFor()) {
                              n.fraction += nn->chord()->duration().fraction();
                              nn = nn->tieFor()->endNote();
                              }
                        n.note     = note;
                        notes.append(n);
                        }
                  }
            }
      int ctick  = 0;
      foreach(SNote n, notes) {
            Measure* m = n.note->chord()->measure();
            if (ctick < m->tick())
                  ctick = m->tick();
            if (n.tick > ctick) {
                  //
                  // fill with rest
                  //
                  Segment* s = m->tick2segment(ctick);
                  if (s == 0) {
                        printf("no segment at %d - measure %d\n", ctick, m->tick());
                        continue;
                        }
                  ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
                  if (cr == 0) {
                        printf("no cr at %d - measure %d\n", ctick, m->tick());
                        continue;
                        }
                  Fraction f = Fraction::fromTicks(n.tick - ctick);
                  f = makeGap(cr, f, 0);

                  setRest(ctick, dtrack, f, false, 0);
                  }
            ctick = n.tick;
            Segment* s = m->tick2segment(n.tick);
            if (s == 0) {
                  printf("no segment at %d - measure %d\n", n.tick, m->tick());
                  continue;
                  }
            ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
            if (cr == 0) {
                  printf("no cr at %d - measure %d\n", ctick, m->tick());
                  continue;
                  }
            setNoteRest(cr, dtrack, n.pitch, n.fraction, 0, AUTO);
            ctick += n.fraction.ticks();
            }
      foreach(SNote n, notes)
            deleteItem(n.note);
      foreach(Note* n, notesToRemove)
            deleteItem(n);
      }

