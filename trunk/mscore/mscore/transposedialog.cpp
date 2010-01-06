//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

//---------------------------------------------------------
//   Interval
//---------------------------------------------------------

struct Interval {
      int steps;
      int semitones;
      };

static Interval intervalList[26] = {
      { 0, 0 },         // Perfect Unison
      { 0, 1 },         // Augmented Unison

      { 1, 0 },         // Diminished Second
      { 1, 1 },         // Minor Second
      { 1, 2 },         // Major Second
      { 1, 3 },         // Augmented Second

      { 2, 2 },         // Diminished Third
      { 2, 3 },         // Minor Third
      { 2, 4 },         // Major Third
      { 2, 5 },         // Augmented Third

      { 3, 4 },         // Diminished Fourth
      { 3, 5 },         // Perfect Fourth
      { 3, 6 },         // Augmented Fourth

      { 4, 6 },         // Diminished Fifth
      { 4, 7 },         // Perfect Fifth
      { 4, 8 },         // Augmented Fifth

      { 5, 7 },         // Diminished Sixth
      { 5, 8 },         // Minor Sixth
      { 5, 9 },         // Major Sixth
      { 5, 10 },        // Augmented Sixth

      { 6, 9 },         // Diminished Seventh
      { 6, 10 },        // Minor Seventh
      { 6, 11 },        // Major Seventh
      { 6, 12 },        // Augmented Seventh

      { 7, 11 },        // Diminshed Octave
      { 7, 12 }         // Perfect Octave
      };

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
//   getSemitones
//---------------------------------------------------------

int TransposeDialog::getSemitones() const
      {
      return 0;
      }

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose()
      {
      if (last() == 0)     // empty score?
            return;
      if (selection()->state() == SEL_NONE) {
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
            _selection->setState(SEL_SYSTEM);
            _selection->setStartSegment(tick2segment(0));
            _selection->setEndSegment(
               tick2segment(last()->tick() + last()->tickLen())
               );
            _selection->staffStart = 0;
            _selection->staffEnd   = nstaves();
            }
      TransposeDialog td;
      td.enableTransposeKeys(_selection->state() == SEL_SYSTEM);
      if (!td.exec())
            return;
      int diff                 = td.getSemitones();
      bool transposeKeys       = td.getTransposeKeys();
      bool transposeChordNames = td.getTransposeChordNames();

      if (_selection->state() != SEL_SYSTEM)
            transposeKeys = false;
      int d = diff < 0 ? -diff : diff;
      bool fullOctave = (d % 12) == 0;
      if (fullOctave) {
            transposeKeys = false;
            transposeChordNames = false;
            }

      if (_selection->state() == SEL_SINGLE || _selection->state() == SEL_MULT) {
            QList<Element*>* el = _selection->elements();
            foreach(Element* e, *el) {
                  if (e->type() != NOTE)
                        continue;
                  transpose(static_cast<Note*>(e), diff);
                  }
            return;
            }

      int startTrack = _selection->staffStart * VOICES;
      int endTrack   = _selection->staffEnd * VOICES;
      if (_selection->state() == SEL_SYSTEM) {
            startTrack = 0;
            endTrack   = nstaves() * VOICES;
            }

      if (transposeKeys) {
            for (int staffIdx = _selection->staffStart; staffIdx < _selection->staffEnd; ++staffIdx) {
                  KeyList* km = staff(staffIdx)->keymap();
                  for (iKeyList ke = km->lower_bound(_selection->tickStart());
                     ke != km->lower_bound(_selection->tickEnd()); ++ke) {
                        KeySigEvent oKey  = ke->second;
                        int tick  = ke->first;
                        int nKey = 0;
                        if (td.mode() == TRANSPOSE_BY_KEY)
                              nKey  = td.transposeKey();
                              // nKey  = transposeKey(oKey.accidentalType, diff);
                        undoChangeKey(staff(staffIdx), tick, oKey, KeySigEvent(nKey));
                        }
                  for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
                        if (s->subtype() != Segment::SegKeySig)
                              continue;
                        if (s->tick() < _selection->tickStart())
                              continue;
                        if (s->tick() >= _selection->tickEnd())
                              break;
                        KeySig* ks = static_cast<KeySig*>(s->element(staffIdx));
                        if (ks) {
                              KeySigEvent key  = km->key(s->tick());
                              KeySigEvent okey = km->key(s->tick()-1);
                              key.naturalType  = okey.accidentalType;
                              _undo->push(new ChangeKeySig(ks, key));
                              }
                        }
                  }
            }

      for (int st = startTrack; st < endTrack; ++st) {
            for (Segment* segment = _selection->startSegment(); segment && segment != _selection->endSegment(); segment = segment->next1()) {
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
                  foreach(Note* n, nl) {
                        if (td.mode() == TRANSPOSE_BY_KEY)
                              transposeByKey(n, td.transposeKey(), td.direction());
                        else
                              transposeByInterval(n, td.transposeInterval(), td.direction());
                        }
                  }
            }

      if (transposeChordNames) {
            Measure* sm = _selection->startSegment()->measure();
            Measure* em = _selection->endSegment()->measure();
            int stick   = _selection->startSegment()->tick();
            int etick   = _selection->endSegment()->tick();

            for (Measure* m = sm;;) {
                  foreach (Element* e, *m->el()) {
                        if (e->type() != HARMONY || e->tick() < stick)
                              continue;
                        Harmony* harmony = static_cast<Harmony*>(e);
                        if (harmony->tick() >= etick)
                              break;
                        undoTransposeHarmony(harmony, diff);
                        }
                  if (m == em)
                        break;
                  MeasureBase* mb = m->next();
                  while (mb && (mb->type() != MEASURE))
                        mb = mb->next();
                  if (mb == 0)
                        break;
                  m = static_cast<Measure*>(mb);
                  }
            }
      setLayoutAll(true);
      }

//---------------------------------------------------------
//   transposeStaff
//---------------------------------------------------------

void Score::cmdTransposeStaff(int staffIdx, int diff)
      {
      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;

      KeyList* km = staff(staffIdx)->keymap();
      for (iKeyList ke = km->begin(); ke != km->end(); ++ke) {
            KeySigEvent oKey  = ke->second;
            int tick  = ke->first;
            int nKey  = transposeKey(oKey.accidentalType, diff);
            undoChangeKey(staff(staffIdx), tick, oKey, KeySigEvent(nKey));
            }
      for (Segment* s = firstMeasure()->first(); s; s = s->next1()) {
            if (s->subtype() != Segment::SegKeySig)
                  continue;
            KeySig* ks = static_cast<KeySig*>(s->element(staffIdx));
            if (ks) {
                  KeySigEvent key = km->key(s->tick());
                  KeySigEvent okey = km->key(s->tick()-1);
                  key.naturalType = okey.accidentalType;
                  _undo->push(new ChangeKeySig(ks, key));
                  }
            }
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
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
                        foreach(Note* n, nl)
                              transpose(n, diff);
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
      // spell(staffIdx, staffIdx+1, _selection->startSegment(), _selection->endSegment());
      spell();
      }

//---------------------------------------------------------
//   cmdConcertPitchChanged
//---------------------------------------------------------

void Score::cmdConcertPitchChanged(bool flag)
      {
      _undo->push(new ChangeConcertPitch(this, flag));

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

//---------------------------------------------------------
//   transpose
//---------------------------------------------------------

void Score::transpose(Note* n, int diff)
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
//---------------------------------------------------------

void Score::transposeByKey(Note* /*n*/, int /*keysig*/, TransposeDirection /*dir*/)
      {

      }

//---------------------------------------------------------
//   transposeByInterval
//---------------------------------------------------------

void Score::transposeByInterval(Note* n, int interval, TransposeDirection dir)
      {
      int pitch     = n->pitch();
      int npitch;
      int tpc       = n->tpc();
      int steps     = intervalList[interval].steps;
      int semitones = intervalList[interval].semitones;

      if (dir == TRANSPOSE_DOWN) {
            steps     = -steps;
            semitones = -semitones;
            npitch    = pitch - intervalList[interval].semitones;
            }
      else
            npitch    = pitch + intervalList[interval].semitones;

      int step, alter;

      for (;;) {
            int s1     = tpc2step(tpc);
            int octave = (pitch / 12);

            step       = tpc2step(tpc) + steps;
            while (step < 0) {
                  step += 7;
                  octave -= 1;
                  }
            while (step >= 7) {
                  step -= 7;
                  octave += 1;
                  }

            int p1     = tpc2pitch(step2tpc(step, 0)) + octave * 12;
            alter      = semitones - (p1 - pitch);
printf("Interval(%d,%d,%d) step %d octave %d p1 %d(%d-%d) alter %d\n",
    interval, steps, semitones, step, octave, p1, pitch, npitch, alter);
            if (alter > 2)
                  steps -= 1;
            else if (alter < -2)
                  steps += 1;
            else
                  break;
            }
      tpc  = step2tpc(step, alter);
      undoChangePitch(n, npitch, tpc, 0);
      }

