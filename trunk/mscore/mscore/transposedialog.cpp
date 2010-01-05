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
      int diff           = td.getSemitones();
      bool transposeKeys = td.getTransposeKeys();
      if (_selection->state() != SEL_SYSTEM)
            transposeKeys = false;
      int d = diff < 0 ? -diff : diff;
      bool fullOctave = (d % 12) == 0;
      if (fullOctave)
            transposeKeys = false;

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
                        int nKey  = transposeKey(oKey.accidentalType, diff);
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
                              KeySigEvent key = km->key(s->tick());
                              KeySigEvent okey = km->key(s->tick()-1);
                              ks->setSig(okey.accidentalType, key.accidentalType);
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
                  foreach(Note* n, nl)
                        transpose(n, diff);
                  }
            }

      if (!fullOctave && td.getTransposeChordNames()) {
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

      // do not respell if transposing a full octave
      //
      if (!fullOctave) {
//            spell(_selection->staffStart, _selection->staffEnd, _selection->startSegment(), _selection->endSegment());
            }
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
                  KeySigEvent key  = km->key(s->tick());
                  KeySigEvent okey = km->key(s->tick()-1);
                  ks->setSig(okey.accidentalType, key.accidentalType);
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

