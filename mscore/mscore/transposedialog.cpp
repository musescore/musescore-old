//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "transposedialog.h"
#include "score.h"
#include "mscore.h"
#include "layout.h"
#include "chord.h"
#include "note.h"
#include "key.h"
#include "staff.h"
#include "harmony.h"
#include "part.h"

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose()
      {
      if (_layout->last() == 0)     // empty score?
            return;
      if (sel->state() == SEL_NONE) {
            QMessageBox::StandardButton sb = QMessageBox::question(mscore,
               tr("MuseScore: transpose"),
               tr("There is nothing selected. Transpose whole score?"),
               QMessageBox::Yes | QMessageBox::Cancel,
               QMessageBox::Yes
            );
            if (sb == QMessageBox::Cancel)
                  return;
            //
            // select all
            //
            sel->setState(SEL_SYSTEM);
            sel->setStartSegment(tick2segment(0));
            sel->setEndSegment(
               tick2segment(_layout->last()->tick() + _layout->last()->tickLen())
               );
            sel->staffStart = 0;
            sel->staffEnd   = nstaves();
            }
      TransposeDialog td;
      td.enableTransposeKeys(sel->state() == SEL_SYSTEM);
      if (!td.exec())
            return;
      int diff           = td.getSemitones();
      bool transposeKeys = td.getTransposeKeys();
      if (sel->state() != SEL_SYSTEM)
            transposeKeys = false;

      if (sel->state() == SEL_SINGLE || sel->state() == SEL_MULT) {
            QList<Element*>* el = sel->elements();
            foreach(Element* e, *el) {
                  if (e->type() != NOTE)
                        continue;
                  Note* n = static_cast<Note*>(e);
                  undoChangePitch(n, n->pitch() + diff);
                  }
            return;
            }

      int startTrack = sel->staffStart * VOICES;
      int endTrack   = sel->staffEnd * VOICES;
      if (sel->state() == SEL_SYSTEM) {
            startTrack = 0;
            endTrack   = nstaves() * VOICES;
            }
      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* segment = sel->startSegment(); segment && segment != sel->endSegment(); segment = segment->next()) {
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  NoteList* notes = chord->noteList();

                  // we have to operate on a list copy because
                  // change pitch changes chord->noteList():
                  QList<Note*> nl;
                  for (iNote i = notes->begin(); i != notes->end(); ++i)
                        nl.append(i->second);
                  foreach(Note* note, nl)
                        undoChangePitch(note, note->pitch() + diff);
                  }
            }
/* TODO
      if (td.getTransposeChordNames()) {
            foreach (Element* e, *mb->el()) {
                  if (e->type() != HARMONY)
                        continue;
                  Harmony* harmony = static_cast<Harmony*>(e);
                  undoTransposeHarmony(harmony, diff);
                  }
            }
*/
      if (transposeKeys) {
            for (int staffIdx = sel->staffStart; staffIdx < sel->staffEnd; ++staffIdx) {
                  KeyList* km = staff(staffIdx)->keymap();
                  for (iKeyEvent ke = km->lower_bound(sel->tickStart());
                     ke != km->lower_bound(sel->tickEnd()); ++ke) {
                        int oKey  = ke->second;
                        int tick  = ke->first;
                        int nKey  = transposeKey(oKey, diff);
                        undoChangeKey(staff(staffIdx), tick, oKey, nKey);
                        }
                  }
            }
      undoSetPitchSpellNeeded();
      spell();
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, int diff)
      {
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      for (MeasureBase* mb = _layout->first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            for (int st = startTrack; st < endTrack; ++st) {
                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        Element* e = segment->element(st);
                        if (!e || e->type() != CHORD)
                              continue;
                        Chord* chord = static_cast<Chord*>(e);
                        NoteList* notes = chord->noteList();

                        // we have to operate on a list copy because
                        // change pitch changes chord->noteList():
                        QList<Note*> nl;
                        for (iNote i = notes->begin(); i != notes->end(); ++i)
                              nl.append(i->second);
                        foreach(Note* note, nl)
                              undoChangePitch(note, note->pitch() + diff);
                        }
                  }
#if 0
            foreach (Element* e, *mb->el()) {
                  if (e->type() != HARMONY)
                        continue;
                  Harmony* harmony = static_cast<Harmony*>(e);
                  undoTransposeHarmony(harmony, diff);
                  }
#endif
            }
      KeyList* km = staff(staffIdx)->keymap();
      for (iKeyEvent ke = km->begin(); ke != km->end(); ++ke) {
            int oKey  = ke->second;
            int tick  = ke->first;
            int nKey  = transposeKey(oKey, diff);
            undoChangeKey(staff(staffIdx), tick, oKey, nKey);
            }
      undoSetPitchSpellNeeded();
      spell();
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag)
      {
      checkUndoOp();
      UndoOp i;
      i.type   = UndoOp::ChangeConcertPitch;
      i.val1   = flag;
      undoList.back()->push_back(i);
      processUndoOp(&undoList.back()->back(), false);

      foreach(Staff* staff, _staves) {
            Instrument* instr = staff->part()->instrument();
            int offset = instr->pitchOffset;
            if (offset == 0)
                  continue;
            if (!flag)
                 offset = -offset;
            cmdTransposeStaff(staff->idx(), offset);
            }
      }

