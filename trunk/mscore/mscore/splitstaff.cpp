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

      //
      // move notes
      //    for now we only move notes from voice 0
      //
      select(0, SELECT_SINGLE, 0);
      for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
            if (s->subtype() != Segment::SegChordRest)
                  continue;
            ChordRest* cr = static_cast<ChordRest*>(s->element(staffIdx * VOICES));
            if (cr == 0 || cr->type() == REST)
                  continue;
            Chord* c = static_cast<Chord*>(cr);
            NoteList* nl = c->noteList();
            for (iNote i = nl->begin(); i != nl->end(); ++i) {
                  Note* n = i->second;
                  if (n->pitch() < splitPoint) {
                        deleteItem(n);
                        }
                  }
            }
      }

