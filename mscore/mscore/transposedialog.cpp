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
               tick2segment(last()->tick() + last()->tickLen())
               );
            _selection.setStaffStart(0);
            _selection.setStaffEnd(nstaves());
            }
      TransposeDialog td;
      td.enableTransposeKeys(_selection.state() == SEL_RANGE);
      int startStaffIdx = 0;
      int startTick = 0;
      if (selection().state() == SEL_RANGE) {
            startStaffIdx = selection().staffStart();
            startTick     = selection().tickStart();
            }
      KeyList* km = staff(startStaffIdx)->keymap();
      int key     = km->key(startTick).accidentalType;
      td.setKey(key);
      if (!td.exec())
            return;

      int semitones;
      if (td.mode() == TRANSPOSE_BY_KEY)
            semitones = -1;    // TODO
      else {
            semitones = intervalList[td.transposeInterval()].semitones;
            if (td.direction() == TRANSPOSE_DOWN)
                  semitones = -semitones;
            }
      bool trKeys               = td.getTransposeKeys();
      bool transposeChordNames  = td.getTransposeChordNames();
      bool useDoubleSharpsFlats = td.useDoubleSharpsFlats();

      if (_selection.state() != SEL_RANGE)
            trKeys = false;
      int d = semitones < 0 ? -semitones : semitones;
      bool fullOctave = (d % 12) == 0;
      if (fullOctave && td.mode() != TRANSPOSE_BY_KEY) {
            trKeys = false;
            transposeChordNames = false;
            }

      int interval  = td.transposeInterval();
      int diatonic  = intervalList[interval].steps;
      int chromatic = intervalList[interval].semitones;
      if (td.direction() == TRANSPOSE_DOWN) {
            diatonic  = -diatonic;
            chromatic = -chromatic;
            }
      if (_selection.state() == SEL_LIST) {
            if (td.mode() == TRANSPOSE_BY_KEY) {
                  foreach(Element* e, _selection.elements()) {
                        if (e->type() == NOTE) {
                              Note* n = static_cast<Note*>(e);
                              KeyList* km = n->staff()->keymap();
                              int oKey    = km->key(n->chord()->tick()).accidentalType;
                              transposeByKey(oKey, td.transposeKey(), td.direction(),
                                 useDoubleSharpsFlats, &diatonic, &chromatic);
                              break;
                              }
                        }
                  }

            foreach(Element* e, _selection.elements()) {
                  if (e->type() != NOTE)
                        continue;
                  Note* n = static_cast<Note*>(e);
                  transposeByInterval(n, diatonic, chromatic, useDoubleSharpsFlats);
                  }
            return;
            }

      int startTrack = _selection.staffStart() * VOICES;
      int endTrack   = _selection.staffEnd() * VOICES;

      km       = staff(_selection.staffStart())->keymap();
      int oKey = km->key(_selection.startSegment()->tick()).accidentalType;
      transposeByKey(oKey, td.transposeKey(), td.direction(),
         useDoubleSharpsFlats, &diatonic, &chromatic);

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* segment = _selection.startSegment(); segment && segment != _selection.endSegment(); segment = segment->next1()) {
                  Element* e = segment->element(st);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);
                  QList<Note*> nl = chord->notes();
                  foreach (Note* n, nl)
                        transposeByInterval(n, diatonic, chromatic, useDoubleSharpsFlats);
                  }
            }

      if (trKeys) {
            transposeKeys(_selection.staffStart(), _selection.staffEnd(),
               _selection.tickStart(), _selection.tickEnd(),
               td.transposeKey(), chromatic);
            }

      if (transposeChordNames) {
            Measure* sm = _selection.startSegment()->measure();
            Measure* em = _selection.endSegment()->measure();
            int stick   = _selection.startSegment()->tick();
            int etick   = _selection.endSegment()->tick();

            for (Measure* m = sm; m; m = m->nextMeasure()) {
                  foreach (Element* e, *m->el()) {
                        if (e->type() != HARMONY || e->tick() < stick)
                              continue;
                        Harmony* harmony = static_cast<Harmony*>(e);
                        if (harmony->tick() >= etick)
                              break;
                        if (td.mode() == TRANSPOSE_BY_KEY)
                              ; // TODO
                        else {
                              int rootTpc = transposeTpc(harmony->rootTpc(),
                                 td.transposeInterval(), td.direction());
                              int baseTpc = transposeTpc(harmony->baseTpc(),
                                 td.transposeInterval(), td.direction());
                              undoTransposeHarmony(harmony, rootTpc, baseTpc);
                              }
                        }
                  if (m == em)
                        break;
                  }
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, int diatonic, int chromatic, bool useDoubleSharpsFlats)
      {
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      transposeKeys(staffIdx, staffIdx+1, 0, lastSegment()->tick(), 0, chromatic);

      for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
           if (segment->subtype() != Segment::SegChordRest)
                 continue;
           for (int st = startTrack; st < endTrack; ++st) {
                 Element* e = segment->element(st);
                 if (!e || e->type() != CHORD)
                       continue;
                 Chord* chord = static_cast<Chord*>(e);
                 QList<Note*> nl = chord->notes();
                 foreach(Note* n, nl)
                        transposeByInterval(n, diatonic, chromatic, useDoubleSharpsFlats);
                 }
           }
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag, bool useDoubleSharpsFlats)
      {
      _undo->push(new ChangeConcertPitch(this, flag));

      foreach(Staff* staff, _staves) {
            Part* instr = staff->part();
            int transposeDiatonic = instr->transposeDiatonic();
            int transposeChromatic = instr->transposeChromatic();
            if (transposeChromatic == 0 && transposeDiatonic == 0)
                  continue;
            if (!flag) {
                  transposeChromatic = -transposeChromatic;
                  transposeDiatonic  = -transposeDiatonic;
                  }
            cmdTransposeStaff(staff->idx(), transposeDiatonic, transposeChromatic,
               useDoubleSharpsFlats);
            }
      }

//---------------------------------------------------------
//   transpose
//    transpose by semitones
//---------------------------------------------------------

void Score::transposeBySemitones(Note* n, int diff)
      {
      int npitch = n->pitch() + diff;
      int d      = diff < 0 ? -diff : diff;
      KeySigEvent key = n->staff()->key(n->chord()->tick());
      int tpc    = (d % 12) == 0 ? n->tpc() : pitch2tpc(npitch, key.accidentalType);
      int acc    = (d % 12) == 0 ? n->userAccidental() : 0;
      undoChangePitch(n, npitch, tpc, acc);
      }

//---------------------------------------------------------
//   transposeByKey
//    keysig -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeByKey(int oKey, int nKey, TransposeDirection dir, bool useDoubleSharpsFlats,
   int* dia, int* chr)
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
      int chromatic = (cofSteps * 7) % 12;


      if ((dir == TRANSPOSE_CLOSEST) && (chromatic > 6))
            dir = TRANSPOSE_DOWN;

      if (dir == TRANSPOSE_DOWN) {
            chromatic = chromatic - 12;
            diatonic  = diatonic - 7;
            }
      *dia = diatonic;
      *chr = chromatic;
printf("TransposeByKey %d -> %d   chromatic %d diatonic %d\n", oKey, nKey, chromatic, diatonic);
      }

//---------------------------------------------------------
//   transposeByInterval
//---------------------------------------------------------

void Score::transposeByInterval(Note* n, int diatonic, int chromatic, bool useDoubleSharpsFlats)
      {
      int npitch;
      int ntpc;
      transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, diatonic, chromatic,
        useDoubleSharpsFlats);
      undoChangePitch(n, npitch, ntpc, 0);
      }

//---------------------------------------------------------
//   transposeKeys
//    key -   -7(Cb) - +7(C#)
//---------------------------------------------------------

void Score::transposeKeys(int staffStart, int staffEnd, int tickStart, int tickEnd,
   int key, int semitones)
      {
      for (int staffIdx = staffStart; staffIdx < staffEnd; ++staffIdx) {
            KeyList* km = staff(staffIdx)->keymap();
            for (iKeyList ke = km->lower_bound(tickStart);
               ke != km->lower_bound(tickEnd); ++ke) {
                  KeySigEvent oKey  = ke->second;
                  int tick  = ke->first;
                  int nKey = 0;
                  nKey  = transposeKey(oKey.accidentalType, semitones);
                  undoChangeKey(staff(staffIdx), tick, oKey, KeySigEvent(nKey));
                  }
            for (Segment* s = firstSegment(); s; s = s->next1()) {
                  if (s->subtype() != Segment::SegKeySig)
                        continue;
                  if (s->tick() < tickStart)
                        continue;
                  if (s->tick() >= tickEnd)
                        break;
                  KeySig* ks = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                  if (ks) {
                        KeySigEvent key  = km->key(s->tick());
                        KeySigEvent okey = km->key(s->tick() - 1);
                        key.naturalType  = okey.accidentalType;
                        _undo->push(new ChangeKeySig(ks, key));
                        }
                  }
            }
      }

