//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2008-2010 Werner Schweer and others
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
#include "chord.h"
#include "note.h"
#include "key.h"
#include "staff.h"
#include "harmony.h"
#include "part.h"
#include "pitchspelling.h"
#include "measure.h"
#include "undo.h"
#include "keysig.h"
#include "utils.h"
#include "segment.h"
#include "stafftype.h"

//---------------------------------------------------------
//   keydiff2Interval
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

static Interval keydiff2Interval(int oKey, int nKey, TransposeDirection dir)
      {
      static int stepTable[15] = {
            // C  G  D  A  E  B Fis
               0, 4, 1, 5, 2, 6, 3,
            };

      int cofSteps;     // circle of fifth steps
      int diatonic;
      if (nKey > oKey)
            cofSteps = nKey - oKey;
      else
            cofSteps = 12 - (oKey - nKey);
      diatonic = stepTable[(nKey + 7) % 7] - stepTable[(oKey + 7) % 7];
      if (diatonic < 0)
            diatonic += 7;
      diatonic %= 7;
      int chromatic = (cofSteps * 7) % 12;


      if ((dir == TRANSPOSE_CLOSEST) && (chromatic > 6))
            dir = TRANSPOSE_DOWN;

      if (dir == TRANSPOSE_DOWN) {
            chromatic = chromatic - 12;
            diatonic  = diatonic - 7;
            if (diatonic == -7)
                  diatonic = 0;
            if (chromatic == -12)
                  chromatic = 0;
            }
printf("TransposeByKey %d -> %d   chromatic %d diatonic %d\n", oKey, nKey, chromatic, diatonic);
      return Interval(diatonic, chromatic);
      }

//---------------------------------------------------------
//   TransposeDialog
//---------------------------------------------------------

TransposeDialog::TransposeDialog(QWidget* parent)
   : QDialog(parent)
      {
      setupUi(this);
      connect(transposeByKey, SIGNAL(clicked(bool)), SLOT(transposeByKeyToggled(bool)));
      connect(transposeByInterval, SIGNAL(clicked(bool)), SLOT(transposeByIntervalToggled(bool)));
      }

//---------------------------------------------------------
//   transposeByKeyToggled
//---------------------------------------------------------

void TransposeDialog::transposeByKeyToggled(bool val)
      {
      transposeByInterval->setChecked(!val);
      }

//---------------------------------------------------------
//   transposeByIntervalToggled
//---------------------------------------------------------

void TransposeDialog::transposeByIntervalToggled(bool val)
      {
      transposeByKey->setChecked(!val);
      }

//---------------------------------------------------------
//   mode
//---------------------------------------------------------

TransposeMode TransposeDialog::mode() const
      {
      return transposeByKey->isChecked() ? TRANSPOSE_BY_KEY : TRANSPOSE_BY_INTERVAL;
      }

//---------------------------------------------------------
//   enableTransposeByKey
//---------------------------------------------------------

void TransposeDialog::enableTransposeByKey(bool val)
      {
      transposeByKey->setEnabled(val);
      transposeByInterval->setChecked(!val);
      transposeByKey->setChecked(val);
      }

//---------------------------------------------------------
//   direction
//---------------------------------------------------------

TransposeDirection TransposeDialog::direction() const
      {
      if (mode() == TRANSPOSE_BY_KEY) {
            if (closestKey->isChecked())
                  return TRANSPOSE_CLOSEST;
            if (upKey->isChecked())
                  return TRANSPOSE_UP;
            return TRANSPOSE_DOWN;
            }
      if (upInterval->isChecked())
            return TRANSPOSE_UP;
      return TRANSPOSE_DOWN;
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose()
      {
      if (last() == 0)     // empty score?
            return;
      if (selection().state() == SEL_NONE) {
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
            _selection.setState(SEL_RANGE);
            _selection.setStartSegment(tick2segment(0));
            _selection.setEndSegment(
               tick2segment(last()->tick() + last()->ticks())
               );
            _selection.setStaffStart(0);
            _selection.setStaffEnd(nstaves());
            }
      bool rangeSelection = _selection.state() == SEL_RANGE;
      TransposeDialog td;

      // TRANSPOSE_BY_KEY and "transpose keys" is only possible if selection state is SEL_RANGE
      td.enableTransposeKeys(rangeSelection);
      td.enableTransposeByKey(rangeSelection);

      int startStaffIdx = 0;
      int startTick     = 0;
      if (rangeSelection) {
            startStaffIdx = selection().staffStart();
            startTick     = selection().tickStart();
            }
      KeyList* km = staff(startStaffIdx)->keymap();
      int key     = km->key(startTick).accidentalType();
      td.setKey(key);
      if (!td.exec())
            return;

      Interval interval;
      if (td.mode() == TRANSPOSE_BY_KEY) {
            // calculate interval from "transpose by key"
            km       = staff(startStaffIdx)->keymap();
            int oKey = km->key(startTick).accidentalType();
            interval = keydiff2Interval(oKey, td.transposeKey(), td.direction());
            }
      else {
            interval = intervalList[td.transposeInterval()];
            if (td.direction() == TRANSPOSE_DOWN)
                  interval.flip();
            }

      bool trKeys               = td.getTransposeKeys();
      bool transposeChordNames  = td.getTransposeChordNames();

      if (!rangeSelection)
            trKeys = false;
      bool fullOctave = (interval.chromatic % 12) == 0;
      if (fullOctave && (td.mode() != TRANSPOSE_BY_KEY)) {
            trKeys = false;
            transposeChordNames = false;
            }

      bool useDoubleSharpsFlats = td.useDoubleSharpsFlats();
      if (_selection.state() == SEL_LIST) {
            foreach(Element* e, _selection.elements()) {
                  if (e->staff()->staffType()->group() == PERCUSSION_STAFF)
                        continue;
                  if (e->type() == NOTE)
                        transpose(static_cast<Note*>(e), interval, useDoubleSharpsFlats);
                  else if ((e->type() == HARMONY) && transposeChordNames) {
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  else if ((e->type() == KEYSIG) && trKeys) {
                        KeySig* ks = static_cast<KeySig*>(e);
                        KeySigEvent key  = km->key(ks->tick());
                        KeySigEvent okey = km->key(ks->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo()->push(new ChangeKeySig(ks, key, ks->showCourtesySig(),
                           ks->showNaturals()));
                        }
                  }
            return;
            }

      int startTrack = _selection.staffStart() * VOICES;
      int endTrack   = _selection.staffEnd() * VOICES;

      for (Segment* segment = _selection.startSegment(); segment && segment != _selection.endSegment(); segment = segment->next1()) {
            for (int st = startTrack; st < endTrack; ++st) {
                  if (staff(st/VOICES)->staffType()->group() == PERCUSSION_STAFF)
                        continue;
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach (Note* n, nl)
                        transpose(n, interval, useDoubleSharpsFlats);
                  }
            if (transposeChordNames) {
                  foreach (Element* e, segment->annotations()) {
                        if ((e->type() != HARMONY) || (e->track() < startTrack) || (e->track() >= endTrack))
                              continue;
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }

      if (trKeys) {
            transposeKeys(_selection.staffStart(), _selection.staffEnd(),
               _selection.tickStart(), _selection.tickEnd(), interval.chromatic);
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, Interval interval, bool useDoubleSharpsFlats)
      {
      if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF)
            return;
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      transposeKeys(staffIdx, staffIdx+1, 0, lastSegment()->tick(), interval.chromatic);

      for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
           for (int st = startTrack; st < endTrack; ++st) {
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                      continue;

                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach(Note* n, nl)
                      transpose(n, interval, useDoubleSharpsFlats);
                  }
            }

      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach (Element* e, *m->el()) {
                  if (e->type() != HARMONY)
                        continue;
                  if (e->track() >= startTrack && e->track() < endTrack) {
                        Harmony* h  = static_cast<Harmony*>(e);
                        int rootTpc = transposeTpc(h->rootTpc(), interval, false);
                        int baseTpc = transposeTpc(h->baseTpc(), interval, false);
                        undoTransposeHarmony(h, rootTpc, baseTpc);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag, bool useDoubleSharpsFlats)
      {
      undo()->push(new ChangeConcertPitch(this, flag));

      foreach(Staff* staff, _staves) {
            if (staff->staffType()->group() == PERCUSSION_STAFF)
                  continue;
            Instrument* instr = staff->part()->instr();
            Interval interval = instr->transpose();
            if (interval.isZero())
                  continue;
            if (!flag)
                  interval.flip();
            cmdTransposeStaff(staff->idx(), interval, useDoubleSharpsFlats);
            }
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(Note* n, Interval interval, bool useDoubleSharpsFlats)
      {
      int npitch;
      int ntpc;
      transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, interval,
        useDoubleSharpsFlats);
      undoChangePitch(n, npitch, ntpc, n->line(), n->fret(), n->string());
      }

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd, int semitones)
      {
      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            if (staff(staffIdx)->staffType()->group() == PERCUSSION_STAFF)
                  continue;
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  if (s->subtype() != SegKeySig)
                        continue;
                  if (s->tick() < tickStart)
                        continue;
                  if (s->tick() >= tickEnd)
                        break;
                  KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                  if (ks) {
                        KeyList* km      = staff(staffIdx)->keymap();
                        KeySigEvent key  = km->key(s->tick());
                        KeySigEvent okey = km->key(s->tick() - 1);
                        key.setNaturalType(okey.accidentalType());
                        undo()->push(new ChangeKeySig(ks, key, ks->showCourtesySig(),
                           ks->showNaturals()));
                        }
                  }
            }
      }

