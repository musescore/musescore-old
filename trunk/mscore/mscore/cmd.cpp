//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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
 Handling of several GUI commands.
*/

#include "globals.h"
#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "scoreview.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "text.h"
#include "al/sig.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "preferences.h"
#include "page.h"
#include "barline.h"
#include "seq.h"
#include "mscore.h"
#include "tuplet.h"
#include "padids.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "textline.h"
#include "keysig.h"
#include "volta.h"
#include "dynamics.h"
#include "box.h"
#include "harmony.h"
#include "system.h"
#include "stafftext.h"
#include "articulation.h"
#include "metaedit.h"
#include "chordedit.h"
#include "layoutbreak.h"
#include "drumset.h"
#include "beam.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "measure.h"
#include "al/al.h"
#include "al/tempo.h"
#include "al/sig.h"
#include "undo.h"
#include "editstyle.h"
#include "textstyle.h"
#include "timesig.h"
#include "repeat.h"
#include "tempotext.h"
#include "excerpt.h"
#include "clef.h"
#include "noteevent.h"
#include "breath.h"
#include "tablature.h"
#include "stafftype.h"
#include "segment.h"

//---------------------------------------------------------
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visble undo.
//---------------------------------------------------------

void Score::startCmd()
      {
      if (debugMode)
            printf("===startCmd()\n");
      layoutAll = true;      ///< do a complete relayout

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (undo()->active()) {
            // if (debugMode)
            fprintf(stderr, "Score::startCmd(): cmd already active\n");
            return;
            }
      undo()->beginMacro();
      undo()->push(new SaveState(this));
      }

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visble undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd()
      {
      if (debugMode)
            printf("===endCmd()\n");
      if (!undo()->active()) {
            // if (debugMode)
                  fprintf(stderr, "Score::endCmd(): no cmd active\n");
            end();
            return;
            }
      bool noUndo = undo()->current()->childCount() <= 1;
      if (!noUndo)
            setClean(noUndo);
      end();
      undo()->endMacro(noUndo);
      }

//---------------------------------------------------------
//   end
///   Update the redraw area.
//---------------------------------------------------------

void Score::end()
      {
      Score* score = parentScore() ? parentScore() : this;
      score->end1();
      foreach(Excerpt* e, score->_excerpts)
            e->score()->end1();
      }

//---------------------------------------------------------
//   end1
//---------------------------------------------------------

void Score::end1()
      {
      bool _needLayout = false;
      if (layoutAll) {
            _updateAll  = true;
            _needLayout = true;
            startLayout = 0;
            }
      else if (startLayout) {
            _updateAll = true;
            _needLayout = true;
            }
      if (_updateAll)
            emit updateAll();
      else {
            // update a little more:
            double d = _spatium * .5;
            refresh.adjust(-d, -d, 2 * d, 2 * d);
            emit dataChanged(refresh);
            }
      if (_needLayout)
            doLayout();
      refresh     = QRectF();
      layoutAll   = false;
      _updateAll  = false;
      startLayout = 0;
      if (!noteEntryMode())
            setPadState();
      }

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const QPointF& pos, const QPointF& /*dragOffset*/)
      {
      int pitch, staffIdx;
      QPointF offset;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &staffIdx, &pitch, &segment, &offset);
      if (mb == 0 || mb->type() != MEASURE) {
            printf("cmdAddSpanner: cannot put object here\n");
            delete spanner;
            return;
            }

      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;
      spanner->setTrack(track);
      spanner->setStartElement(segment);
      spanner->setParent(segment);

      if (spanner->anchor() == ANCHOR_SEGMENT) {
            static const SegmentType st = SegChordRest;
            Segment* s = segment;
            for (;;) {
                  Segment* sn = s->next1(st);
                  if (sn == 0)
                        break;
                  s = sn;
                  if (s->measure() != segment->measure())
                        break;
                  }
            if (s == segment) {
                  printf("cmdAddSpanner: cannot put object on last segment\n");
                  delete spanner;
                  return;
                  }
            spanner->setEndElement(s);
            }
      else {      // ANCHOR_MEASURE
            spanner->setEndElement(segment->measure()->last());
            }

      undoAddElement(spanner);
      select(spanner, SELECT_SINGLE, 0);

      if (spanner->type() == TRILL) {
            Element* e = segment->element(staffIdx * VOICES);
            if (e && e->type() == CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  Fraction l = chord->duration();
                  if (chord->notes().size() > 1) {
                        // trill do not work for chords
                        }
                  Note* note = chord->upNote();
                  while (note->tieFor()) {
                        note = note->tieFor()->endNote();
                        l += note->chord()->duration();
                        }
                  Segment* s = note->chord()->segment();
                  s = s->next1(SegChordRest);
                  while (s) {
                        Element* e = s->element(staffIdx * VOICES);
                        if (e)
                              break;
                        s = s->next1(SegChordRest);
                        }
                  if (s)
                        spanner->setEndElement(s);
                  Fraction d(1,32);
                  Fraction e = l / d;
                  int n = e.numerator() / e.denominator();
                  QList<NoteEvent*> events;
                  int pitch  = chord->upNote()->ppitch();
                  int clef   = chord->staff()->clef(segment->tick());
                  int pitch2 = diatonicUpDown(clef, pitch, 1);
                  int dpitch = pitch2 - pitch;
                  for (int i = 0; i < n; i += 2) {
                        events.append(new NoteEvent(0,      i * 1000 / n,    1000/n));
                        events.append(new NoteEvent(dpitch, (i+1) *1000 / n, 1000/n));
                        }
                  undo()->push(new ChangeNoteEvents(chord, events));
                  }
            }
      }

//---------------------------------------------------------
//   cmdInsertNote
//---------------------------------------------------------

void ScoreView::cmdInsertNote(int note)
      {
      printf("cmdInsertNote %d\n", note);

      }

//---------------------------------------------------------
//   cmdAddPitch
//    c d e f g a b entered:
//       insert note or add note to chord
//---------------------------------------------------------

void ScoreView::cmdAddPitch(int note, bool addFlag)
      {
      InputState& is = _score->inputState();
      Drumset* ds    = is.drumset();
      int pitch;
      if (ds) {
            char note1 = "CDEFGAB"[note];
            pitch = -1;
            for (int i = 0; i < 127; ++i) {
                  if (!ds->isValid(i))
                        continue;
                  if (ds->shortcut(i) && (ds->shortcut(i) == note1)) {
                        pitch = i;
                        break;
                        }
                  }
            if (pitch == -1) {
                  printf("  shortcut %c not defined in drumset\n", note1);
                  return;
                  }
            is.setDrumNote(pitch);
            }
      else {
            KeySigEvent key = _score->staff(is.track() / VOICES)->keymap()->key(is.tick());
            int octave = is.pitch / 12;
            pitch      = pitchKeyAdjust(note, key.accidentalType());
            int delta  = is.pitch - (octave*12 + pitch);
            if (delta > 6)
                  is.pitch = (octave+1)*12 + pitch;
            else if (delta < -6)
                  is.pitch = (octave-1)*12 + pitch;
            else
                  is.pitch = octave*12 + pitch;
            if (is.pitch < 0)
                  is.pitch = 0;
            if (is.pitch > 127)
                  is.pitch = 127;
            pitch = is.pitch;
            }
      cmdAddPitch1(pitch, addFlag);
      }

//---------------------------------------------------------
//   cmdAddPitch1
//---------------------------------------------------------

void ScoreView::cmdAddPitch1(int pitch, bool addFlag)
      {
      InputState& is = _score->inputState();

      if (is.segment() == 0) {
            printf("cannot enter notes here (no chord rest at current position)\n");
            return;
            }
      _score->startCmd();
      _score->expandVoice();
      if (is.cr() == 0) {
            printf("cannot enter notes here (no chord rest at current position)\n");
            return;
            }
      if (!noteEntryMode()) {
            Element* e = _score->selection().element();
            if (e == 0 || e->type() != NOTE) {
                  _score->endCmd();
                  return;
                  }
            Note* note = static_cast<Note*>(e);
            Chord* chord = note->chord();
            int key = _score->staff(chord->staffIdx())->key(chord->segment()->tick()).accidentalType();
            int newTpc = pitch2tpc(pitch, key);
            _score->undoChangePitch(note, pitch, newTpc, note->line(), note->fret(), note->string());
            }
      else {
            const InputState& is = _score->inputState();
            Note* note = _score->addPitch(pitch, addFlag);
            if (note)
                  adjustCanvasPosition(note, false);
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   expandVoice
//---------------------------------------------------------

void Score::expandVoice()
      {
      Segment* s = _is.segment();
      int track  = _is.track();

      if (s->element(track))
            return;

      Segment* ps;
      for (ps = s; ps; ps = ps->prev(SegChordRest)) {
            if (ps->element(track))
                  break;
            }
      if (ps) {
            ChordRest* cr = static_cast<ChordRest*>(ps->element(track));
            if (cr->tick() + cr->ticks() > s->tick()) {
                  printf("expandVoice: cannot insert element here\n");
                  return;
                  }
            }
      Segment* ns;
      for (ns = s->next(SegChordRest); ns; ns = ns->next(SegChordRest)) {
            if (ns->element(track))
                  break;
            }
      Measure* m  = s->measure();
      Fraction f;
      if (ns)
            f = Fraction::fromTicks(ns->tick() - s->tick());
      else
            f = Fraction::fromTicks(m->ticks() - s->rtick());
      addRest(_is.segment(), _is.track(), Duration(f), 0);
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(int pitch, bool addFlag)
      {
      if (addFlag) {
            // add note to chord
            Note* on = getSelectedNote();
            if (on == 0)
                  return 0;
            Note* n = addNote(on->chord(), pitch);
            setLayoutAll(false);
            setLayout(on->chord()->measure());
            moveToNextInputPos();
            return n;
            }
      expandVoice();

      // insert note
      Direction stemDirection = AUTO;
      int headGroup           = 0;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            pitch     = _is.drumNote();
            Drumset* ds   = _is.drumset();
            headGroup     = ds->noteHead(pitch);
            stemDirection = ds->stemDirection(pitch);
            track         = ds->voice(pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if(!_is.cr())
            return 0;
      NoteVal nval;
      nval.pitch = pitch;
      nval.headGroup = headGroup;
      Segment* seg = setNoteRest(_is.cr(), track, nval, _is.duration().fraction(), stemDirection);
      Note* note = static_cast<Chord*>(seg->element(track))->upNote();
      setLayout(note->chord()->measure());

      if (_is.slur) {
            //
            // extend slur
            //
            ChordRest* e = searchNote(_is.tick(), _is.track());
            if (e) {
                  int stick = 0;
                  Element* ee = _is.slur->startElement();
                  if (ee->isChordRest())
                        stick = static_cast<ChordRest*>(ee)->tick();
                  else if (ee->type() == NOTE)
                        stick = static_cast<Note*>(ee)->chord()->tick();
                  if (stick == e->tick()) {
                        if (_is.slur->startElement())
                              static_cast<ChordRest*>(_is.slur->startElement())->removeSlurFor(_is.slur);
                        _is.slur->setStartElement(e);
                        static_cast<ChordRest*>(e)->addSlurFor(_is.slur);
                        }
                  else
                        _is.slur->setEndElement(e);
                  }
            else
                  printf("addPitch: cannot find slur note\n");
            setLayoutAll(true);
            }
      moveToNextInputPos();
      return note;
      }

//---------------------------------------------------------
//   cmdAddInterval
//---------------------------------------------------------

void Score::cmdAddInterval(int val, const QList<Note*>& nl)
      {
      startCmd();
      foreach(Note* on, nl) {
            Note* note = new Note(*on);
            Chord* chord = on->chord();
            note->setParent(chord);
            int valTmp = val < 0 ? val+1 : val-1;
            int line = on->line() - valTmp;

            int tick   = chord->tick();
            Staff* estaff = staff(on->staffIdx() + chord->staffMove());
            int clef   = estaff->clef(tick);
            int key    = estaff->key(tick).accidentalType();
            int npitch = line2pitch(line, clef, key);
            int ntpc   = pitch2tpc(npitch, key);
            note->setPitch(npitch, ntpc);

            undoAddElement(note);
            mscore->play(note);
            setLayout(on->chord()->measure());

            select(note, SELECT_SINGLE, 0);
            _is.pitch = note->pitch();
            }
      moveToNextInputPos();
      endCmd();
      }

//---------------------------------------------------------
//   setGraceNote
///   Create a grace note in front of a normal note.
///   \arg chord is the normal note
///   \arg pitch is the pitch of the grace note
///   \arg is the grace note type
///   \len is the visual duration of the grace note (1/16 or 1/32)
//---------------------------------------------------------

void Score::setGraceNote(Chord* ch, int pitch, NoteType type, int len)
      {
      Staff* ostaff = ch->staff();
      QList<Staff*> staffList;
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      int tick = ch->segment()->tick();

      foreach(Staff* staff, staffList) {
            Score* score     = staff->score();
            Measure* measure = score->tick2measure(tick);
            Segment* seg     = measure->findSegment(SegChordRest, tick);
            int track        = score->staffIdx(staff) * VOICES + ch->voice();

            SegmentType st = SegGrace;
            Segment* s     = seg->prev();

            while (s && s->subtype() == st && s->element(track))
                  s = s->prev();
            if (s && (s->subtype() == st) && (!s->element(track)))
                  seg = s;
            else {
                  seg = new Segment(measure, st, tick);
                  undo()->push(new AddElement(seg));
                  }
            double mag = staff->mag() * styleD(ST_graceNoteMag);

            Note* note = new Note(score);
            note->setTrack(track);
            note->setMag(mag);
            note->setPitch(pitch);

            Chord* chord = new Chord(score);
            chord->setTrack(track);
            chord->add(note);

            Duration d;
            d.setVal(len);
            chord->setDurationType(d);
            chord->setDuration(d.fraction());
            chord->setStemDirection(UP);
            chord->setNoteType(type);
            chord->setParent(seg);
            chord->setMag(mag);

            undoAddElement(chord);
            note->setTpcFromPitch();      // tick must be known
            if (staff == ostaff)
                  select(note, SELECT_SINGLE, 0);
            }
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(ChordRest* cr, int track, NoteVal nval, Fraction sd,
Direction stemDirection)
      {
      int tick      = cr->tick();
      Element* nr   = 0;
      Tie* tie      = 0;

      Segment* seg = 0;
      Measure* measure = 0;
      for (;;) {
            // the returned gap ends at the measure boundary or at tuplet end
            Fraction dd = makeGap(cr, sd, cr->tuplet());

            if (dd.isZero()) {
                  printf("cannot get gap at %d type: %d/%d\n", tick, sd.numerator(),
                     sd.denominator());
                  break;
                  }
            QList<Duration> dl = toDurationList(dd, true);

            measure = cr->measure();
            int n = dl.size();
            for (int i = 0; i < n; ++i) {
                  Duration d = dl[i];

                  //SegmentType st = SegChordRest;
                  //seg = measure->findSegment(st, tick);
                  //if (seg == 0) {
                  //      seg = new Segment(measure, st, tick);
                  //      undoAddElement(seg);
                  //      }
                  ChordRest* ncr;
                  Note* note = 0;
                  if (nval.pitch == -1) {
                        nr = new Rest(this);
                        nr->setTrack(track);
                        ncr = (Rest*)nr;
                        ncr->setDurationType(d);
                        ncr->setDuration(d.fraction());
                        // ncr->setParent(seg);
                        }
                  else {
                        note = new Note(this);
                        nr = note;
                        note->setTrack(track);

                        if (tie) {
                              tie->setEndNote(note);
                              note->setTieBack(tie);
                              }
                        Chord* chord = new Chord(this);
                        chord->setTrack(track);
                        chord->setDurationType(d);
                        chord->setDuration(d.fraction());
                        chord->setStemDirection(stemDirection);
                        chord->add(note);
                        note->setNval(nval);
                        ncr = chord;
                        if (i+1 < n) {
                              tie = new Tie(this);
                              tie->setStartNote((Note*)nr);
                              tie->setTrack(nr->track());
                              note->setTieFor(tie);
                              }
                        }
                  ncr->setTuplet(cr->tuplet());
                  undoAddCR(ncr, measure, tick);
                  if (ncr->type() == CHORD) {
                        mscore->play(ncr);
                        }
                  seg = ncr->segment();
                  tick += ncr->ticks();
                  }

            sd -= dd;
            if (sd.isZero())
                  break;

            Segment* nseg = tick2segment(tick, false, SegChordRest);
            if (nseg == 0) {
                  printf("reached end of score\n");
                  break;
                  }
            seg = nseg;

            cr = static_cast<ChordRest*>(seg->element(track));

            if (cr == 0) {
                  if (track % VOICES)
                        cr = addRest(seg, track, Duration(Duration::V_MEASURE), 0);
                  else {
                        printf("no rest in voice 0\n");
                        break;
                        }
                  }
            //
            //  Note does not fit on current measure, create Tie to
            //  next part of note
            if (nval.pitch != -1) {
                  tie = new Tie(this);
                  tie->setStartNote((Note*)nr);
                  tie->setTrack(nr->track());
                  ((Note*)nr)->setTieFor(tie);
                  }
            }
      if (tie)
            connectTies();
      if (nr)
            select(nr, SELECT_SINGLE, 0);
      return seg;
      }

//---------------------------------------------------------
//   makeGap
//    make time gap at tick by removing/shortening
//    chord/rest
//
//    gap does not exceed measure or scope of tuplet
//
//    return size of actual gap
//---------------------------------------------------------

Fraction Score::makeGap(ChordRest* cr, const Fraction& _sd, Tuplet* tuplet)
      {
// printf("makeGap %d/%d at %d track %d\n", _sd.numerator(), _sd.denominator(), cr->tick(), cr->track());
      int track = cr->track();
      Measure* measure = cr->measure();
      setLayout(measure);
      Fraction akkumulated;
      Fraction sd = _sd;

      for (Segment* seg = cr->segment(); seg; seg = seg->next()) {
            if (seg->subtype() == SegGrace) {
                  if (seg->element(track)) {
                        undoRemoveElement(seg->element(track));
                        if (seg->isEmpty())
                              undoRemoveElement(seg);
                        }
                  continue;
                  }
            if (!seg->isChordRest())
                  continue;
            if (!seg->element(track))
                  continue;
            cr = static_cast<ChordRest*>(seg->element(track));
            //
            // limit to tuplet level
            //
            if (tuplet) {
                  bool tupletEnd = true;
                  Tuplet* t = cr->tuplet();
                  while (t) {
                        if (cr->tuplet() == tuplet) {
                              tupletEnd = false;
                              break;
                              }
                        t = t->tuplet();
                        }
                  if (tupletEnd) {
//                        printf("makeGap: end of tuplet reached\n");
                        break;
                        }
                  }
            Fraction td(cr->duration());
printf("remove %s\n", qPrintable(cr->duration().print()));

            Tuplet* ltuplet = cr->tuplet();
            if (cr->tuplet() != tuplet) {
//                  printf("   remove tuplet %d\n", sd >= ltuplet->fraction());
                  //
                  // Current location points to the start of a (nested)tuplet.
                  // We have to remove the complete tuplet.

                  Tuplet* t = ltuplet;
                  while (t->elements().last()->type() == TUPLET)
                        t = static_cast<Tuplet*>(t->elements().last());
                  seg = static_cast<ChordRest*>(t->elements().last())->segment();

                  td = ltuplet->duration();
                  cmdDeleteTuplet(ltuplet, false);
                  tuplet = 0;
                  }
            else {
//                  printf("  makeGap: remove %d/%d at %d\n", td.numerator(), td.denominator(), cr->tick());
                  undoRemoveElement(cr);
                  if (seg->isEmpty())
                        undoRemoveElement(seg);
                  else {
                        foreach(Element* e, seg->annotations()) {
                              if (e->track() == cr->track())
                                    undoRemoveElement(e);
                              }
                        }
                  }

            if (sd < td) {
                  //
                  // we removed too much
                  //
                  akkumulated = _sd;
                  Fraction rd = td - sd;

printf("  makeGap: %d/%d removed %d/%d too much\n", sd.numerator(), sd.denominator(), rd.numerator(), rd.denominator());

                  QList<Duration> dList = toDurationList(rd, false);
                  if (dList.isEmpty())
                        return akkumulated;
                  int ticks = sd.ticks();
// printf("   gap ticks %d+%d\n", cr->tick(), ticks);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        ticks = ticks * t->ratio().denominator() / t->ratio().numerator();
                  int tick = cr->tick() + ticks;

                  if ((tuplet == 0) && (((measure->tick() - tick) % dList[0].ticks()) == 0)) {
                        foreach(Duration d, dList) {
//                              printf("   addClone %d\n", tick);
                              tick += addClone(cr, tick, d)->ticks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i)
                              tick += addClone(cr, tick, dList[i])->ticks();
                        }
// printf("  return %d/%d\n", akkumulated.numerator(), akkumulated.denominator());
                  return akkumulated;
                  }
            akkumulated += td;
// printf("  akkumulated %d/%d\n", akkumulated.numerator(), akkumulated.denominator());
            sd          -= td;
            if (sd.numerator() == 0)
                  break;
            }
      return akkumulated;
      }

//---------------------------------------------------------
//   makeGap1
//    make time gap at tick by removing/shortening
//    chord/rest
//    - cr is top level (not part of a tuplet)
//    - do not stop at measure end
//
//    return size of actual gap
//---------------------------------------------------------

bool Score::makeGap1(int tick, int staffIdx, Fraction len)
      {
      ChordRest* cr = 0;
      Segment* seg = tick2segment(tick, true);
      if (!seg) {
            printf("1:makeGap1: no segment at %d\n", tick);
            return false;
            }
      while (seg && !(seg->subtype() & (SegChordRest | SegGrace)))
            seg = seg->next1();
      if (!seg) {
            printf("2:makeGap1: no segment at %d\n", tick);
            return false;
            }
      cr = static_cast<ChordRest*>(seg->element(staffIdx * VOICES));
      if (!cr) {
            printf("makeGap1: no chord/rest at %d staff %d\n", tick, staffIdx);
            return false;
            }

      Fraction gap;
      for (;;) {
            Fraction l = makeGap(cr, len, 0);
            if (l.isZero())
                  break;
            len -= l;
            gap += l;
            if (len.isZero())
                  break;
            // go to next cr
            Measure* m = cr->measure()->nextMeasure();
            if (m == 0) {
                  printf("EOS reached\n");
                  appendMeasures(1, MEASURE);
                  m = cr->measure()->nextMeasure();
                  if (m == 0) {
                        printf("===EOS reached\n");
                        return true;
                        }
                  }
            Segment* s = m->firstCRSegment();
            int track  = cr->track();
            cr = static_cast<ChordRest*>(s->element(track));
            if (cr == 0) {
                  addRest(s->tick(), track, Duration(Duration::V_MEASURE), 0);
                  cr = static_cast<ChordRest*>(s->element(track));
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const Duration& d)
      {
      Fraction srcF(cr->duration());
      Fraction dstF = d.type() == Duration::V_MEASURE ? cr->measure()->timesig() : d.fraction();

      if (srcF == dstF)
            return;
      Tuplet* tuplet = cr->tuplet();
      if (srcF > dstF) {
            //
            // make shorter and fill with rest
            //
            if (cr->type() == CHORD) {
                  //
                  // remove ties
                  //
                  Chord* c = static_cast<Chord*>(cr);
                  foreach(Note* n, c->notes()) {
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  }
            undoChangeChordRestLen(cr, Duration(dstF));
            setRest(cr->tick() + cr->ticks(), cr->track(), srcF - dstF, false, tuplet);
            select(cr, SELECT_SINGLE, 0);
            return;
            }

      //
      // make longer
      //

      // split required len into Measures
      QList<Fraction> flist;

      Fraction f = dstF;
      Fraction f1;
      Segment* s = cr->segment();
      int track  = cr->track();

      if (tuplet && tuplet->duration() < dstF) {
            printf("does not fit in tuplet\n");
            return;
            }

      while (f > Fraction(0)) {
            while (s && ((s->element(track) == 0) || (s->subtype() != SegChordRest)))
                  s = s->next1();
            if (s == 0)
                  break;
            if ((f1 > Fraction(0)) && (s->tick() == s->measure()->tick())) {
                  flist.append(f1);
                  f1 = Fraction(0);
                  }
            ChordRest* cr = static_cast<ChordRest*>(s->element(track));
            Duration d(cr->durationType());
            Fraction f2 = (d.type() == Duration::V_MEASURE) ? cr->measure()->timesig() : d.fraction();
            if (f2 > f)
                  f2 = f;
            f1 += f2;
            f  -= f2;
            s = s->next1();
            }
      if (f1 > Fraction(0))
            flist.append(f1);

// printf("List:\n");
//      foreach (Fraction f, flist)
//            printf("  %d/%d\n", f.numerator(), f.denominator());

      int tick       = cr->tick();
      f              = dstF;
      ChordRest* cr1 = cr;
      Chord* oc      = 0;

      bool first = true;
      foreach (Fraction f2, flist) {
            f  -= f2;
            makeGap(cr1, f2, tuplet);

            if (cr->type() == REST) {

                  Rest* r = setRest(tick, track, f2, (d.dots() > 0), tuplet);
                  if (first) {
                        select(r, SELECT_SINGLE, 0);
                        first = false;
                        }
                  tick += f2.ticks();
                  }
            else {
                  QList<Duration> dList = toDurationList(f2, true);
//printf("   sublist:\n");
//      foreach (Duration d, dList)
//            printf("      %d/%d\n", d.fraction().numerator(), d.fraction().denominator());

                  Measure* measure = tick2measure(tick);
                  if (((tick - measure->tick()) % dList[0].ticks()) == 0) {
                        foreach(Duration d, dList) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    }
                              oc = addChord(tick, d, cc, genTie, tuplet);
                              if (first) {
                                    select(oc, SELECT_SINGLE, 0);
                                    first = false;
                                    }
                              tick += oc->ticks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i) {
                              bool genTie;
                              Chord* cc;
                              if (oc) {
                                    genTie = true;
                                    cc = oc;
                                    }
                              else {
                                    genTie = false;
                                    cc = static_cast<Chord*>(cr);
                                    }
                              oc = addChord(tick, dList[i], cc, genTie, tuplet);
                              if (first) {
                                    select(oc, SELECT_SINGLE, 0);
                                    first = false;
                                    }
                              tick += oc->ticks();
                              }
                        }
                  }
            Measure* m  = cr1->measure();
            Measure* m1 = m->nextMeasure();
            if (m1 == 0)
                  break;
            cr1 = static_cast<ChordRest*>(m1->firstCRSegment()->element(track));
            }
      connectTies();
      }

//---------------------------------------------------------
//   cmdAddChordName
//---------------------------------------------------------

void ScoreView::cmdAddChordName()
      {
      if (!_score->checkHasMeasures())
            return;
      ChordRest* cr = _score->getSelectedChordRest();
      if (!cr)
            return;
      _score->startCmd();
      Harmony* s = new Harmony(_score);
      s->setTrack(cr->track());
      s->setParent(cr->segment());
      _score->undoAddElement(s);

      _score->setLayoutAll(true);

      _score->select(s, SELECT_SINGLE, 0);
      adjustCanvasPosition(s, false);
      startEdit(s);
      }

//---------------------------------------------------------
//   cmdAddChordName2
//---------------------------------------------------------

void Score::cmdAddChordName2()
      {
      if (!checkHasMeasures())
            return;
      ChordRest* cr = getSelectedChordRest();
      if (!cr)
            return;
      int rootTpc = 14;
      if (cr->type() == CHORD) {
            Chord* chord = static_cast<Chord*>(cr);
            rootTpc = chord->downNote()->tpc();
            }
      Harmony* s = 0;
      Segment* segment = cr->segment();

      foreach(Element* e, segment->annotations()) {
            if (e->type() == HARMONY && (e->track() == cr->track())) {
                  s = static_cast<Harmony*>(e);
                  break;
                  }
            }

      bool created = false;
      if (s == 0) {
            s = new Harmony(this);
            s->setTrack(cr->track());
            s->setParent(segment);
            s->setRootTpc(rootTpc);
            created = true;
            }

      ChordEdit ce(this);
      ce.setHarmony(s);
      int rv = ce.exec();
      if (rv) {
            const Harmony* h = ce.harmony();
            s->setRootTpc(h->rootTpc());
            s->setBaseTpc(h->baseTpc());
            s->setId(h->id());
            s->clearDegrees();
            for (int i = 0; i < h->numberOfDegrees(); i++)
                  s->addDegree(h->degree(i));
            s->render();
            select(s, SELECT_SINGLE, 0);
            undoAddElement(s);
            layoutAll = true;
            }
      else {
            if (created)
                  delete s;
            }
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void ScoreView::cmdAddText(int subtype)
      {
      if (!_score->checkHasMeasures())
            return;
      Page* page = _score->pages().front();
      const QList<System*>* sl = page->systems();
      const QList<MeasureBase*>& ml = sl->front()->measures();
      Text* s = 0;
      _score->startCmd();
      switch(subtype) {
            case TEXT_TITLE:
            case TEXT_SUBTITLE:
            case TEXT_COMPOSER:
            case TEXT_POET:
                  {
                  MeasureBase* measure = ml.front();
                  if (measure->type() != VBOX) {
                        measure = new VBox(_score);
                        measure->setNext(ml.front());
                        measure->setTick(0);
                        _score->undoInsertMeasure(measure);
                        }
                  s = new Text(_score);
                  switch(subtype) {
                        case TEXT_TITLE:    s->setTextStyle(TEXT_STYLE_TITLE);    break;
                        case TEXT_SUBTITLE: s->setTextStyle(TEXT_STYLE_SUBTITLE); break;
                        case TEXT_COMPOSER: s->setTextStyle(TEXT_STYLE_COMPOSER); break;
                        case TEXT_POET:     s->setTextStyle(TEXT_STYLE_POET);     break;
                        }
                  s->setSubtype(subtype);
                  s->setParent(measure);
                  }
                  break;

            case TEXT_REHEARSAL_MARK:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new Text(_score);
                  s->setTrack(0);
                  s->setSubtype(subtype);
                  s->setTextStyle(TEXT_STYLE_REHEARSAL_MARK);
                  s->setParent(cr->segment());
                  }
                  break;
            case TEXT_STAFF:
            case TEXT_SYSTEM:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new StaffText(_score);
                  if (subtype == TEXT_SYSTEM) {
                        s->setTrack(0);
                        s->setSystemFlag(true);
                        s->setTextStyle(TEXT_STYLE_SYSTEM);
                        s->setSubtype(TEXT_SYSTEM);
                        }
                  else {
                        s->setTrack(cr->track());
                        s->setSystemFlag(false);
                        s->setTextStyle(TEXT_STYLE_STAFF);
                        s->setSubtype(TEXT_STAFF);
                        }
                  s->setSubtype(subtype);
                  s->setParent(cr->segment());
                  }
                  break;
            }

      if (s) {
            _score->undoAddElement(s);
            _score->setLayoutAll(true);
            _score->select(s, SELECT_SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            }
      else
            _score->endCmd();
      }

//---------------------------------------------------------
//   upDown
///   Increment/decrement pitch of note by one or by an octave.
//---------------------------------------------------------

void Score::upDown(bool up, UpDownMode mode)
      {
      QList<Note*> el;
      int tick = -1;
      bool playNotes = true;
      foreach (Note* note, selection().noteList()) {
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  if (el.indexOf(note) == -1) {
                        el.append(note);
                        if (tick == -1)
                              tick = note->chord()->tick();
                        else {
                              if (tick != note->chord()->tick())
                                    playNotes = false;      // don't scare the cat
                              }
                        }
                  }
            }
      if (el.empty())
            return;

      foreach(Note* oNote, el) {
            Part* part  = oNote->staff()->part();
            int pitch   = oNote->pitch();
            int newTpc;
            int newPitch;
            int string = oNote->string();
            int fret   = oNote->fret();

            switch(oNote->staff()->staffType()->group()) {
                  case PERCUSSION_STAFF:
                        {
                        Drumset* ds = part->instr()->drumset();
                        newPitch    = up ? ds->prevPitch(pitch) : ds->nextPitch(pitch);
                        newTpc      = oNote->tpc();
                        }
                        break;
                  case TAB_STAFF:
                        {
                        Tablature* tab = part->instr()->tablature();
                        switch(mode) {
                              case UP_DOWN_OCTAVE:
                                    {
                                    string += (up ? -1 : 1);
                                    if (string < 0)
                                          string = 0;
                                    else if (string >= tab->strings())
                                          string = tab->strings() - 1;
                                    fret = 0;
                                    newPitch      = tab->getPitch(string, fret);
                                    Chord* chord  = oNote->chord();
                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
                                    KeySigEvent ks = estaff->key(chord->tick());
                                    newTpc         = pitch2tpc(newPitch, ks.accidentalType());
                                    }
                                    break;

                              case UP_DOWN_CHROMATIC:
                                    newPitch = up ? pitch+1 : pitch-1;
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = pitch2tpc2(newPitch, up);
                                    break;

                              case UP_DOWN_DIATONIC:
                                    {
                                    fret += (up ? 1 : -1);
                                    if (fret < 0)
                                          fret = 0;
                                    else if (fret >= tab->frets())
                                          fret = tab->frets() - 1;
                                    newPitch      = tab->getPitch(string, fret);
                                    Chord* chord  = oNote->chord();
                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
                                    KeySigEvent ks = estaff->key(chord->tick());
                                    newTpc         = pitch2tpc(newPitch, ks.accidentalType());
                                    }
                                    break;
                              }
                        }
                        break;
                  case PITCHED_STAFF:
                        switch(mode) {
                              case UP_DOWN_OCTAVE:
                                    newPitch = pitch + (up ? 12 : -12);
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = oNote->tpc();
                                    break;

                              case UP_DOWN_CHROMATIC:
                                    newPitch = up ? pitch+1 : pitch-1;
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = pitch2tpc2(newPitch, up);
                                    break;

                              case UP_DOWN_DIATONIC:
                                    {
                                    Chord* chord  = oNote->chord();
                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
                                    int clef      = estaff->clef(chord->tick());
                                    int line      = oNote->line() + (up ? -1 : 1);
                                    newPitch      = line2pitch(line, clef, 0);
                                    int step      = clefTable[clef].pitchOffset - line;
                                    while (step < 0)
                                          step += 7;
                                    step %= 7;
                                    newTpc = step2tpc(step, 0);
                                    }
                                    break;
                              }
                        break;
                  }
            _is.pitch = newPitch;

            if ((oNote->pitch() != newPitch) || (oNote->tpc() != newTpc)
               || oNote->string() != string || oNote->fret() != fret) {
                  undoChangePitch(oNote, newPitch, newTpc, oNote->line(), fret, string);
                  }

            // play new note with velocity 80 for 0.3 sec:
            if (playNotes)
                  mscore->play(oNote);
            }
      _selection.updateState();     // accidentals may have changed
      }

//---------------------------------------------------------
//   cmdAppendMeasures
///   Append \a n measures.
///
///   Keyboard callback, called from pulldown menu.
//
//    - called from pulldown menu
//---------------------------------------------------------

void ScoreView::cmdAppendMeasures(int n, ElementType type)
      {
      _score->startCmd();
      appendMeasures(n, type);
      _score->endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* Score::appendMeasure(ElementType type)
      {
      int tick = 0;
      if (last()) {
            tick = last()->tick();
            if (last()->type() == MEASURE)
                  tick += static_cast<Measure*>(last())->ticks();
            }
      MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, this));
      mb->setTick(tick);

      if (type == MEASURE) {
            Fraction ts(lastMeasure()->timesig());
            Measure* measure = static_cast<Measure*>(mb);
            measure->setTimesig(ts);
            measure->setLen(ts);
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Rest* rest = new Rest(this, Duration(Duration::V_MEASURE));
                  rest->setDuration(ts);
                  rest->setTrack(staffIdx * VOICES);
                  Segment* s = measure->getSegment(SegChordRest, tick);
                  s->add(rest);
                  }
            }
      else if (type == TBOX) {
            undoInsertMeasure(mb);
            Text* s = new Text(this);
            s->setTextStyle(TEXT_STYLE_FRAME);
            s->setSubtype(TEXT_FRAME);
            s->setParent(mb);
            s->setStyled(false);
            undoAddElement(s);
            return mb;
            }
      undoInsertMeasure(mb);
      return mb;
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::appendMeasure(ElementType type)
      {
      _score->startCmd();
      MeasureBase* mb = _score->appendMeasure(type);
      _score->endCmd();
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void ScoreView::appendMeasures(int n, ElementType type)
      {
      if (_score->noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      bool createEndBar = false;
      bool endBarGenerated = false;
      if (type == MEASURE) {
            Measure* lm = _score->lastMeasure();
            if (lm && lm->endBarLineType() == END_BAR) {
                  if (!lm->endBarLineGenerated()) {
                        _score->undoChangeEndBarLineType(lm, NORMAL_BAR);
                        createEndBar = true;
                        // move end Bar to last Measure;
                        }
                  else {
                        createEndBar    = true;
                        endBarGenerated = true;
                        lm->setEndBarLineType(NORMAL_BAR, endBarGenerated);
                        }
                  }
            else if (lm == 0)
                  createEndBar = true;
            }
      for (int i = 0; i < n; ++i)
            _score->appendMeasure(type);
      if (createEndBar) {
            Measure* lm = _score->lastMeasure();
            if (lm)
                  lm->setEndBarLineType(END_BAR, endBarGenerated);
            }
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n, ElementType type)
      {
      bool createEndBar    = false;
      bool endBarGenerated = false;
      if (type == MEASURE) {
            Measure* lm = lastMeasure();
            if (lm && lm->endBarLineType() == END_BAR) {
                  if (!lm->endBarLineGenerated()) {
                        undoChangeEndBarLineType(lm, NORMAL_BAR);
                        createEndBar = true;
                        // move end Bar to last Measure;
                        }
                  else {
                        createEndBar    = true;
                        endBarGenerated = true;
                        lm->setEndBarLineType(NORMAL_BAR, endBarGenerated);
                        }
                  }
            else if (lm == 0)
                  createEndBar = true;
            }
      for (int i = 0; i < n; ++i)
            appendMeasure(type);
      if (createEndBar) {
            Measure* lm = lastMeasure();
            if (lm)
                  lm->setEndBarLineType(END_BAR, endBarGenerated);
            }
      }

//---------------------------------------------------------
//   cmdInsertMeasures
//    - keyboard callback
//    - called from pulldown menu
//---------------------------------------------------------

void ScoreView::cmdInsertMeasures(int n, ElementType type)
      {
      _score->startCmd();
      insertMeasures(n, type);
      _score->endCmd();
      }

//---------------------------------------------------------
//   insertMeasures
//---------------------------------------------------------

void ScoreView::insertMeasures(int n, ElementType type)
      {
	if (_score->selection().state() != SEL_RANGE) {
		QMessageBox::warning(0, "MuseScore",
			tr("No Measure selected:\n"
			"please select a measure and try again"));
		return;
            }
	int tick  = _score->selection().startSegment()->tick();
	for (int i = 0; i < n; ++i)
            insertMeasure(type, tick);
      _score->select(0, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdInsertMeasure
//---------------------------------------------------------

void ScoreView::cmdInsertMeasure(ElementType type)
      {
	if (_score->selection().state() != SEL_RANGE) {
		QMessageBox::warning(0, "MuseScore",
			tr("No Measure selected:\n"
			"please select a measure and try again"));
		return;
            }
      _score->startCmd();
	int tick  = _score->selection().startSegment()->tick();
      MeasureBase* mb = insertMeasure(type, tick);
      if (mb->type() == TBOX) {
            Text* s = new Text(_score);
            s->setTextStyle(TEXT_STYLE_FRAME);
            s->setSubtype(TEXT_FRAME);
            s->setParent(mb);
            s->setStyled(false);
            _score->undoAddElement(s);
            _score->select(s, SELECT_SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            return;
            }
      _score->select(0, SELECT_SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   insertMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::insertMeasure(ElementType type, int tick)
      {
      MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, _score));
      mb->setTick(tick);

      if (type == MEASURE) {
            Measure* m = _score->tick2measure(tick);
            Fraction f(m->timesig());
	      int ticks = f.ticks();

            Measure* measure = static_cast<Measure*>(mb);
            measure->setTimesig(f);
            measure->setLen(f);
	      for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
    	            Rest* rest = new Rest(_score, Duration(Duration::V_MEASURE));
                  rest->setDuration(measure->len());
              	rest->setTrack(staffIdx * VOICES);
                    Segment* s = measure->getSegment(SegChordRest, tick);
              	s->add(rest);
	            }
            QList<TimeSig*> tsl;
            QList<KeySig*>  ksl;

            if (tick == 0) {
                  //
                  // remove time and key signatures
                  //
                  for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
                        for (Segment* s = _score->firstSegment(); s && s->tick() == 0; s = s->next()) {
                              if (s->subtype() == SegKeySig) {
                                    KeySig* e = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                                    if (e) {
                                          if (!e->generated())
                                                ksl.append(static_cast<KeySig*>(e));
                                          _score->undoRemoveElement(e);
                                          if (e->segment()->isEmpty())
                                                _score->undoRemoveElement(e->segment());
                                          }
                                    }
                              else if (s->subtype() == SegTimeSig) {
                                    TimeSig* e = static_cast<TimeSig*>(s->element(staffIdx * VOICES));
                                    if (e) {
                                          if (!e->generated())
                                                tsl.append(static_cast<TimeSig*>(e));
                                          _score->undoRemoveElement(e);
                                          if (e->segment()->isEmpty())
                                                _score->undoRemoveElement(e->segment());
                                          }
                                    }
                              else if (s->subtype() == SegClef) {
                                    Element* e = s->element(staffIdx * VOICES);
                                    if (e) {
                                          _score->undoRemoveElement(e);
                                          if (static_cast<Segment*>(e->parent())->isEmpty())
                                                _score->undoRemoveElement(e->parent());
                                          }
                                    }
                              }
                        }
                  }
            _score->undoInsertMeasure(measure);
            _score->undoInsertTime(tick, ticks);

            //
            // if measure is inserted at tick zero,
            // create key and time signature
            //
            foreach(TimeSig* ts, tsl) {
                  TimeSig* nts = new TimeSig(*ts);
                  SegmentType st = SegTimeSig;
                  Segment* s = measure->findSegment(st, 0);
                  if (s == 0) {
                        s = new Segment(measure, st, 0);
                        _score->undoAddElement(s);
                        }
                  nts->setParent(s);
                  _score->undoAddElement(nts);
                  }
            foreach(KeySig* ks, ksl) {
                  KeySig* nks = new KeySig(*ks);
                  SegmentType st = SegKeySig;
                  Segment* s = measure->findSegment(st, 0);
                  if (s == 0) {
                        s = new Segment(measure, st, 0);
                        _score->undoAddElement(s);
                        }
                  nks->setParent(s);
                  _score->undoAddElement(nks);
                  }
            }
      else
            _score->undoInsertMeasure(mb);
      return mb;
      }

//---------------------------------------------------------
//   addArticulation
///   Add attribute \a attr to all selected notes/rests.
///
///   Called from padToggle() to add note prefix/accent.
//---------------------------------------------------------

void Score::addArticulation(int attr)
      {
      foreach(Element* el, selection().elements()) {
            if (el->type() == NOTE || el->type() == CHORD) {
                  Articulation* na = new Articulation(this);
                  na->setSubtype(attr);
                  addArticulation(el, na);
                  }
            }
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \a idx for all selected
///   notes.
//---------------------------------------------------------

void Score::changeAccidental(AccidentalType idx)
      {
      foreach(Note* note, selection().noteList())
            changeAccidental(note, idx);
      }

//---------------------------------------------------------
//   changeAccidental
///   Change accidental to subtype \accidental for
///   note \a note.
//---------------------------------------------------------

void Score::changeAccidental(Note* note, AccidentalType accidental)
      {
      QList<Staff*> staffList;
      Staff* ostaff = note->chord()->staff();
      LinkedStaves* linkedStaves = ostaff->linkedStaves();
      if (linkedStaves)
            staffList = linkedStaves->staves();
      else
            staffList.append(ostaff);

      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      int voice        = chord->voice();
      Measure* measure = segment->measure();
      int tick         = segment->tick();
      int noteIndex    = chord->notes().indexOf(note);
      Staff* estaff    = staff(chord->staffIdx() + chord->staffMove());
      int clef         = estaff->clef(tick);
      int step         = clefTable[clef].pitchOffset - note->line();
      while (step < 0)
            step += 7;
      step %= 7;
      //
      // accidental change may result in pitch change
      //
      int acc    = Accidental::subtype2value(accidental);
      int acc2   = measure->findAccidental2(note);
      AccidentalType accType;

      int pitch, tpc;
      if (accidental == ACC_NONE) {
            //
            //  delete accidental
            //
            accType = ACC_NONE;
            pitch   = line2pitch(note->line(), clef, 0) + acc2;
            tpc     = step2tpc(step, acc2);
            }
      else {
            if (acc2 == acc) {
                  //
                  // this is a precautionary accidental
                  //
                  pitch = note->pitch();
                  tpc   = note->tpc();
                  Accidental* a = new Accidental(this);
                  a->setParent(note);
                  a->setSubtype(accidental);
                  a->setRole(ACC_USER);
                  note->setAccidental(a);
                  }
            else {
                  accType = accidental;
                  pitch = line2pitch(note->line(), clef, 0) + Accidental::subtype2value(accType);
                  tpc   = step2tpc(step, acc);
                  }
            }

      foreach(Staff* st, staffList) {
            Score* score = st->score();
            Measure* m;
            Segment* s;
            if (score == this) {
                  m = measure;
                  s = segment;
                  }
            else {
                  m   = score->tick2measure(measure->tick());
                  s   = m->findSegment(segment->segmentType(), segment->tick());
                  }
            int staffIdx  = score->staffIdx(st);
            Chord* chord  = static_cast<Chord*>(s->element(staffIdx * VOICES + voice));
            Note* n       = chord->notes().at(noteIndex);

            int fret   = n->fret();
            int string = n->string();
            if (st->useTablature()) {
                  if (pitch != n->pitch()) {
                        //
                        // as pitch has changed, calculate new
                        // string & fret
                        //
                        Tablature* tab = n->staff()->part()->instr()->tablature();
                        if (tab)
                              tab->convertPitch(pitch, &string, &fret);
                        }
                  }
            undo()->push(new ChangePitch(n, pitch, tpc, n->line(), fret, string));
            if (!st->useTablature()) {
                  //
                  // handle ties
                  //
                  if (n->tieBack()) {
                        undoRemoveElement(n->tieBack());
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  else {
                        Note* nn = n;
                        while (nn->tieFor()) {
                              nn = nn->tieFor()->endNote();
                              undo()->push(new ChangePitch(nn, pitch, tpc, nn->line(), fret, string));
                              }
                        }
                  }
            //
            // recalculate needed accidentals for
            // whole measure
            //
            score->updateAccidentals(m, staffIdx);
            }
      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

void Score::addArticulation(Element* el, Articulation* atr)
      {
      ChordRest* cr;
      if (el->type() == NOTE)
            cr = static_cast<ChordRest*>(((Note*)el)->chord());
      else if (el->type() == REST)
            cr = static_cast<ChordRest*>(el);
      else if (el->type() == CHORD)
            cr = static_cast<ChordRest*>(el);
      else {
            delete atr;
            return;
            }
      atr->setParent(cr);
      Articulation* oa = cr->hasArticulation(atr);
      if (oa) {
            delete atr;
            atr = 0;
            undoRemoveElement(oa);
            }
      else
            undoAddElement(atr);
      }

//---------------------------------------------------------
//   resetUserOffsets
///   Reset user offset for all selected notes.
///
///   Called from pulldown menu.
//---------------------------------------------------------

void Score::toDefault()
      {
      foreach(Element* e, selection().elements())
            e->toDefault();
      layoutAll = true;
      setClean(false);
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      Measure* m1 = _selection.startSegment()->measure();
      Measure* m2 = _selection.endSegment()->measure();

      for (Measure* m = m1; m; m = m->nextMeasure()) {
            _undo->push(new ChangeStretch(m, 1.0));
            if (m == m2)
                  break;
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   moveUp
//---------------------------------------------------------

void Score::moveUp(Chord* chord)
      {
      int rstaff    = chord->staff()->rstaff();
      int staffMove = chord->staffMove();

      if ((staffMove == -1) || (rstaff + staffMove <= 0))
            return;
      undo()->push(new ChangeChordStaffMove(chord, staffMove - 1));
      layoutAll = true;
      }

//---------------------------------------------------------
//   moveDown
//---------------------------------------------------------

void Score::moveDown(Chord* chord)
      {
      Staff* staff  = chord->staff();
      Part* part    = staff->part();
      int rstaff    = staff->rstaff();
      int rstaves   = part->nstaves();
      int staffMove = chord->staffMove();

      if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1))
            return;
      undo()->push(new ChangeChordStaffMove(chord, staffMove + 1));
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(double val)
      {
      if (selection().state() != SEL_RANGE)
            return;
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            double stretch = m->userStretch();
            stretch += val;
            undo()->push(new ChangeStretch(m, stretch));
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdInsertClef
//---------------------------------------------------------

void Score::cmdInsertClef(ClefType type)
      {
      if (!noteEntryMode())
            return;
      int tick = inputPos();
      staff(inputTrack()/VOICES)->changeClef(tick, type);
      }

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      if (selection().state() != SEL_RANGE) {
            printf("no system or staff selected\n");
            return;
            }
      int startTick = selection().tickStart();
      int endTick   = selection().tickEnd();

      SegmentTypes st = SegChordRest | SegGrace;
      for (Segment* seg = firstMeasure()->first(st); seg; seg = seg->next1(st)) {
            if (seg->tick() < startTick)
                  continue;
            if (seg->tick() >= endTick)
                  break;
            for (int track = 0; track < nstaves() * VOICES; ++track) {
                  ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
                  if (cr == 0)
                        continue;
                  if (cr->type() == CHORD) {
                        if (cr->beamMode() != BEAM_AUTO)
                              undoChangeBeamMode(cr, BEAM_AUTO);
                        }
                  else if (cr->type() == REST) {
                        if (cr->beamMode() != BEAM_NO)
                              undoChangeBeamMode(cr, BEAM_NO);
                        }
                  }
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdMove
//---------------------------------------------------------

void Score::cmdMove(Element* e, QPointF delta)
      {
      undoMove(e, e->userOff() + delta);
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
      if (debugMode)
            printf("Score::cmd <%s>\n", qPrintable(cmd));

      if (cmd == "print")
            printFile();
      else if (cmd == "repeat") {
            preferences.playRepeats = !preferences.playRepeats;
            updateRepeatList(preferences.playRepeats);
            _playlistDirty = true;
            }
      else if (cmd == "rewind")
            seq->rewindStart();
      else if (cmd == "play-next-measure")
            seq->nextMeasure();
      else if (cmd == "play-next-chord")
            seq->nextChord();
      else if (cmd == "play-prev-measure")
            seq->prevMeasure();
      else if (cmd == "play-prev-chord")
            seq->prevChord();
      else if (cmd == "seek-begin")
            seq->rewindStart();
      else if (cmd == "seek-end")
            seq->seekEnd();
      else if (cmd == "save-style")
            saveStyle();
      else if (cmd == "edit-meta") {
            MetaEditDialog med(this, 0);
            med.exec();
            }
      else if (cmd == "show-invisible") {
            setShowInvisible(getAction(cmd.toLatin1().data())->isChecked());
            _updateAll = true;
            end();
            }
      else if (cmd == "show-frames") {
            setShowFrames(getAction(cmd.toLatin1().data())->isChecked());
            _updateAll = true;
            end();
            }
      else {
            if (undo()->active()) {
                  printf("Score::cmd(): cmd already active\n");
                  return;
                  }
            startCmd();

            //
            // Hack for moving articulations while selected
            //
            Element* el = selection().element();
            if (cmd == "pitch-up") {
                  if (el && el->type() == ARTICULATION)
                        cmdMove(el, QPointF(0.0, -.25));
                  else if (el && el->type() == REST)
                        cmdMoveRest(static_cast<Rest*>(el), UP);
                  else if (el && el->type() == LYRICS)
                        cmdMoveLyrics(static_cast<Lyrics*>(el), UP);
                  else
                        upDown(true, UP_DOWN_CHROMATIC);
                  }
            else if (cmd == "pitch-down") {
                  if (el && el->type() == ARTICULATION)
                        cmdMove(el, QPointF(0.0, .25));
                  else if (el && el->type() == REST)
                        cmdMoveRest(static_cast<Rest*>(el), DOWN);
                  else if (el && el->type() == LYRICS)
                        cmdMoveLyrics(static_cast<Lyrics*>(el), DOWN);
                  else
                        upDown(false, UP_DOWN_CHROMATIC);
                  }
	      else if (cmd == "add-staccato")
                  addArticulation(StaccatoSym);
	      else if (cmd == "add-tenuto")
                  addArticulation(TenutoSym);
	      else if (cmd == "add-trill")
                  addArticulation(TrillSym);
            else if (cmd == "add-hairpin")
                  cmdAddHairpin(false);
            else if (cmd == "add-hairpin-reverse")
                  cmdAddHairpin(true);
            else if (cmd == "delete")
                  cmdDeleteSelection();
            else if (cmd == "delete-measures")
                  cmdDeleteSelectedMeasures();
            else if (cmd == "time-delete") {
                  // TODO:
                  // remove measures if stave-range is 0-nstaves()
                  cmdDeleteSelectedMeasures();
                  }
            else if (cmd == "pitch-up-octave")
                  upDown(true, UP_DOWN_OCTAVE);
            else if (cmd == "pitch-down-octave")
                  upDown(false, UP_DOWN_OCTAVE);
            else if (cmd == "pitch-up-diatonic")
                  upDown(true, UP_DOWN_DIATONIC);
            else if (cmd == "pitch-down-diatonic")
                  upDown(false, UP_DOWN_DIATONIC);
            else if (cmd == "move-up") {
                  setLayoutAll(false);
                  Element* el = selection().element(); // single selection
                  if (el && el->type() == NOTE) {
                        Note* note = static_cast<Note*>(el);
                        moveUp(note->chord());
                        }
                  }
            else if (cmd == "move-down") {
                  setLayoutAll(false);
                  Element* el = selection().element(); // single selection
                  if (el && el->type() == NOTE) {
                        Note* note = static_cast<Note*>(el);
                        moveDown(note->chord());
                        }
                  }
            else if (cmd == "up-chord") {
                  Element* el = selection().element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = upAlt(el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _is.pitch = static_cast<Note*>(e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, SELECT_SINGLE, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "down-chord") {
                  Element* el = selection().element(); // single selection
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        Element* e = downAlt(el);
                        if (e) {
                              if (e->type() == NOTE) {
                                    _is.pitch = static_cast<Note*>(e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, SELECT_SINGLE, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "top-chord" ) {
                  Element* el = selection().element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = upAltCtrl(static_cast<Note*>(el));
                        if (e) {
                              if (e->type() == NOTE) {
                                    _is.pitch = static_cast<Note*>(e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, SELECT_SINGLE, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "bottom-chord") {
                  Element* el = selection().element(); // single selection
                  if (el && el->type() == NOTE) {
                        Element* e = downAltCtrl(static_cast<Note*>(el));
                        if (e) {
                              if (e->type() == NOTE) {
                                    _is.pitch = static_cast<Note*>(e)->pitch();
                                    mscore->play(e);
                                    }
                              select(e, SELECT_SINGLE, 0);
                              }
                        }
                  setLayoutAll(false);
                  }
            else if (cmd == "note-longa")
                  padToggle(PAD_NOTE00);
            else if (cmd == "note-breve")
                  padToggle(PAD_NOTE0);
            else if (cmd == "pad-note-1")
                  padToggle(PAD_NOTE1);
            else if (cmd == "pad-note-2")
                  padToggle(PAD_NOTE2);
            else if (cmd == "pad-note-4")
                  padToggle(PAD_NOTE4);
            else if (cmd == "pad-note-8")
                  padToggle(PAD_NOTE8);
            else if (cmd == "pad-note-16")
                  padToggle(PAD_NOTE16);
            else if (cmd == "pad-note-32")
                  padToggle(PAD_NOTE32);
            else if (cmd == "pad-note-64")
                  padToggle(PAD_NOTE64);
            else if (cmd == "pad-rest")
                  padToggle(PAD_REST);
            else if (cmd == "pad-dot")
                  padToggle(PAD_DOT);
            else if (cmd == "pad-dotdot")
                  padToggle(PAD_DOTDOT);
            else if (cmd == "beam-start")
                  padToggle(PAD_BEAM_START);
            else if (cmd == "beam-mid")
                  padToggle(PAD_BEAM_MID);
            else if (cmd == "no-beam")
                  padToggle(PAD_BEAM_NO);
            else if (cmd == "beam-32")
                  padToggle(PAD_BEAM32);
            else if (cmd == "sharp2")
                  changeAccidental(ACC_SHARP2);
            else if (cmd == "sharp")
                  changeAccidental(ACC_SHARP);
            else if (cmd == "nat")
                  changeAccidental(ACC_NATURAL);
            else if (cmd == "flat")
                  changeAccidental(ACC_FLAT);
            else if (cmd == "flat2")
                  changeAccidental(ACC_FLAT2);
            else if (cmd == "flip")
                  cmdFlip();
            else if (cmd == "stretch+")
                  cmdAddStretch(0.1);
            else if (cmd == "stretch-")
                  cmdAddStretch(-0.1);
            else if (cmd == "tempo")
                  addTempo();
            else if (cmd == "metronome")
                  addMetronome();
            else if (cmd == "pitch-spell")
                  spell();
            else if (cmd == "harmony-properties")
                  cmdAddChordName2();
            else if (cmd == "select-all") {
                  MeasureBase* mb = _measures.last();
                  if (mb) {   // check for empty score
                        _selection.setState(SEL_RANGE);
                        int tick = mb->tick();
                        if (mb->type() == MEASURE)
                              tick += static_cast<Measure*>(mb)->ticks();
                        _selection.setRange(tick2segment(0), tick2segment(tick), 0, nstaves());
                        }
                  }
            else if (cmd == "transpose")
                  transpose();
            else if (cmd == "concert-pitch") {
                  if (styleB(ST_concertPitch) != a->isChecked())
                        cmdConcertPitchChanged(a->isChecked(), true);
                  }
            else if (cmd == "reset-beammode")
                  cmdResetBeamMode();
            else if (cmd == "clef-violin")
                  cmdInsertClef(CLEF_G);
            else if (cmd == "clef-bass")
                  cmdInsertClef(CLEF_F);
            else if (cmd == "load-style") {
                  loadStyle();
                  }
            else if (cmd == "tuplet-dialog")
                  tupletDialog();
            else if (cmd == "voice-x12")
                  cmdExchangeVoice(0, 1);
            else if (cmd == "voice-x13")
                  cmdExchangeVoice(0, 2);
            else if (cmd == "voice-x14")
                  cmdExchangeVoice(0, 3);
            else if (cmd == "voice-x23")
                  cmdExchangeVoice(1, 2);
            else if (cmd == "voice-x24")
                  cmdExchangeVoice(1, 3);
            else if (cmd == "voice-x34")
                  cmdExchangeVoice(2, 3);
            else if (cmd == "system-break" || cmd == "page-break" || cmd == "section-break") {
                  int type;
                  if (cmd == "system-break")
                        type = LAYOUT_BREAK_LINE;
                  else if (cmd == "page-break")
                        type = LAYOUT_BREAK_PAGE;
                  else
                        type = LAYOUT_BREAK_SECTION;

                  Element* e = selection().element();
                  if (e && e->type() == BAR_LINE) {
                        BarLine* barline = static_cast<BarLine*>(e);
                        Measure* measure = barline->measure();
                        if (!measure->lineBreak()) {
                              LayoutBreak* lb = new LayoutBreak(this);
                              lb->setSubtype(type);
                              lb->setTrack(-1);       // this are system elements
                              lb->setParent(measure);
                              undoAddElement(lb);
                              }
                        else {
                              // remove line break
                              foreach(Element* e, *measure->el()) {
                                    if (e->type() == LAYOUT_BREAK && e->subtype() ==type) {
                                          undoRemoveElement(e);
                                          break;
                                          }
                                    }
                              }
                        }
                  }
            else if (cmd == "reset-positions")
                  toDefault();
            else if (cmd == "reset-stretch")
                  resetUserStretch();
            else if (cmd == "mirror-note")
                  cmdMirrorNoteHead();
            else if (cmd == "edit-style") {
                  EditStyle es(this, 0);
                  es.exec();
                  }
            else if (cmd == "edit-text-style") {
                  TextStyleDialog es(0, this);
                  es.exec();
                  }
            else if (cmd == "double-duration")
                  cmdDoubleDuration();
            else if (cmd == "half-duration")
                  cmdHalfDuration();
            else if (cmd == ""){ //Midi note received only?
                  if(!noteEntryMode())
                      setLayoutAll(false);
                  }
            else
                  printf("1unknown cmd <%s>\n", qPrintable(cmd));

            endCmd();
            }
      }

//---------------------------------------------------------
//   processMidiInput
//---------------------------------------------------------

void Score::processMidiInput()
      {
      if (debugMode)
          printf("processMidiInput\n");
      if (midiInputQueue.isEmpty())
            return;

      bool cmdActive = false;
      Note* n = 0;
      while (!midiInputQueue.isEmpty()) {
            MidiInputEvent ev = midiInputQueue.dequeue();
            if (debugMode)
                  printf("<-- !noteentry dequeue %i\n", ev.pitch);
            if (!noteEntryMode()) {
                  int staffIdx = selection().staffStart();
                  Part* p;
                  if (staffIdx < 0 || staffIdx >= nstaves())
                        p = part(0);
                  else
                        p = staff(staffIdx)->part();
                  if (p)
                        seq->startNote(p->instr()->channel(0), ev.pitch, 80, preferences.defaultPlayDuration, 0.0);
                  }
            else  {
                  startCmd();
                  cmdActive = true;
                  n = addPitch(ev.pitch, ev.chord);
                  }
            }
      if (cmdActive) {
            layoutAll = true;
            endCmd();
            //after relayout
            if (n)
                  mscore->currentScoreView()->adjustCanvasPosition(n, false);
            }
      }

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste(ScoreView* view)
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0) {
            printf("no application mime data\n");
            return;
            }
      if (selection().isSingle() && ms->hasFormat(mimeSymbolFormat)) {
            QByteArray data(ms->data(mimeSymbolFormat));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading paste data at line %d column %d: %s\n",
                     line, column, qPrintable(err));
                  printf("%s\n", data.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();
            QPointF dragOffset;
            ElementType type    = Element::readType(e, &dragOffset);
            if (type != INVALID) {
                  Element* el = Element::create(type, this);
                  if (el) {
                        el->read(e);
                        addRefresh(selection().element()->abbox());   // layout() ?!
#if 0
                        if (el->type() == NOTE) {
                              Note* n = static_cast<Note*>(el);
                              if (!styleB(ST_concertPitch) && part->transpose().chromatic) {
                                    int npitch;
                                    int ntpc;
                                    Interval interval = part->transpose();
                                    interval.flip();
                                    transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc,
                                      interval, true);
                                    n->setPitch(npitch, ntpc);
                                    }
                              }
#endif
                        selection().element()->drop(view, QPointF(), QPointF(), el);
                        if (selection().element())
                              addRefresh(selection().element()->abbox());
                        }
                  }
            else
                  printf("cannot read type\n");
            }
      else if ((selection().state() == SEL_RANGE || selection().state() == SEL_LIST)
         && ms->hasFormat(mimeStaffListFormat)) {
            ChordRest* cr = 0;
            if (selection().state() == SEL_RANGE) {
                  cr = selection().firstChordRest();
                  }
            else if (selection().isSingle()) {
                  Element* e = selection().element();
                  if (e->type() != NOTE && e->type() != REST) {
                        printf("cannot paste to %s\n", e->name());
                        return;
                        }
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0) {
                  printf("no destination for paste\n");
                  return;
                  }

            QByteArray data(ms->data(mimeStaffListFormat));
printf("paste <%s>\n", data.data());
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading paste data at line %d column %d: %s\n",
                     line, column, qPrintable(err));
                  printf("%s\n", data.data());
                  return;
                  }
            docName = "--";
            pasteStaff(doc.documentElement(), cr);
            }
      else if (ms->hasFormat(mimeSymbolListFormat) && selection().isSingle()) {
            printf("cannot paste symbol list to element\n");
            }
      else {
            printf("cannot paste selState %d staffList %d\n",
               selection().state(), ms->hasFormat(mimeStaffListFormat));
            foreach(const QString& s, ms->formats())
                  printf("  format %s\n", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(QDomElement e, ChordRest* dst)
      {
      foreach(Beam* beam, _beams)
            beam->setId(-1);
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(-1);
            }

      for (Segment* s = firstMeasure()->first(SegChordRest); s; s = s->next1(SegChordRest)) {
            foreach(Spanner* e, s->spannerFor())
                  e->setId(-1);
            }
      int dstStaffStart = dst->staffIdx();
      int dstTick = dst->tick();
      for (; !e.isNull(); e = e.nextSiblingElement()) {
            if (e.tagName() != "StaffList") {
                  domError(e);
                  continue;
                  }
            int tickStart     = e.attribute("tick","0").toInt();
            int tickLen       = e.attribute("len", "0").toInt();
            int srcStaffStart = e.attribute("staff", "0").toInt();
            int staves        = e.attribute("staves", "0").toInt();
            curTick           = tickStart;

            QSet<int> blackList;
            for (int i = 0; i < staves; ++i) {
                  int staffIdx = i + dstStaffStart;
                  if (staffIdx >= nstaves())
                        break;
                  if (!makeGap1(dst->tick(), staffIdx, Fraction::fromTicks(tickLen)))
                        blackList.insert(staffIdx);
                  }

            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "Staff") {
                        domError(ee);
                        continue;
                        }
                  int srcStaffIdx = ee.attribute("id", "0").toInt();
                  if(blackList.contains(srcStaffIdx))
                        continue;
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
                  if (dstStaffIdx >= nstaves())
                        break;
                  QList<Tuplet*> tuplets;
                  QList<Slur*> slurs;
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        const QString& tag(eee.tagName());
                        if (tag == "tick")
                              curTick = eee.text().toInt();
                        else if (tag == "Tuplet") {
                              Tuplet* tuplet = new Tuplet(this);
                              tuplet->setTrack(curTrack);
                              tuplet->read(eee, tuplets, slurs);
                              int tick = curTick - tickStart + dstTick;
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              tuplets.append(tuplet);
                              undoAddElement(tuplet);
                              }
                        else if (tag == "Slur") {
                              Slur* slur = new Slur(this);
                              slur->read(eee);
                              slur->setTrack(dstStaffIdx * VOICES);
                              slurs.append(slur);
                              }
                        else if (tag == "Chord" || tag == "Rest" || tag == "RepeatMeasure") {
                              ChordRest* cr = static_cast<ChordRest*>(Element::name2Element(tag, this));
                              cr->setTrack(curTrack);
                              cr->read(eee, tuplets, slurs);
                              cr->setSelected(false);
                              int voice = cr->voice();
                              int track = dstStaffIdx * VOICES + voice;
                              cr->setTrack(track);
                              int tick = curTick - tickStart + dstTick;

                              if (cr->type() == CHORD) {
                                    // set note track
                                    // check if staffMove moves a note to a
                                    // nonexistant staff
                                    //
                                    Chord* c      = static_cast<Chord*>(cr);
                                    Part* part    = cr->staff()->part();
                                    int nn = (track / VOICES) + c->staffMove();
                                    if (nn < 0 || nn >= nstaves())
                                          c->setStaffMove(0);
                                    foreach(Note* n, c->notes()) {
                                          if (!styleB(ST_concertPitch) && part->instr()->transpose().chromatic) {
                                                int npitch;
                                                int ntpc;
                                                Interval interval = part->instr()->transpose();
                                                interval.flip();
                                                transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc,
                                                   interval, true);
                                                n->setPitch(npitch, ntpc);
                                                }
                                          n->setTrack(track);
                                          }
                                    }

                              Measure* measure = tick2measure(tick);
                              bool isGrace = (cr->type() == CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL);
                              int measureEnd = measure->tick() + measure->ticks();
                              if (!isGrace && (cr->tick() + cr->ticks() > measureEnd)) {
                                    if (cr->type() == CHORD) {
                                          // split Chord
                                          Chord* c = static_cast<Chord*>(cr);
                                          int rest = c->ticks();
                                          int len  = measureEnd - c->tick();
                                          rest    -= len;
                                          Duration d;
                                          d.setVal(len);
                                          c->setDurationType(d);
                                          undoAddCR(c, measure, tick);
                                          while (rest) {
                                                int tick = c->tick() + c->ticks();
                                                measure = tick2measure(tick);
                                                if (measure->tick() != tick)  // last measure
                                                      break;
                                                Chord* c2 = static_cast<Chord*>(c->clone());
                                                len = measure->ticks() > rest ? rest : measure->ticks();
                                                Duration d;
                                                d.setVal(len);
                                                c2->setDurationType(d);
                                                rest -= len;
                                                undoAddCR(c2, measure, tick);

                                                QList<Note*> nl1 = c->notes();
                                                QList<Note*> nl2 = c2->notes();

                                                for (int i = 0; i < nl1.size(); ++i) {
                                                      Tie* tie = new Tie(this);
                                                      tie->setStartNote(nl1[i]);
                                                      tie->setEndNote(nl2[i]);
                                                      tie->setTrack(c->track());
                                                      Tie* tie2 = nl1[i]->tieFor();
                                                      if (tie2) {
                                                            nl2[i]->setTieFor(nl1[i]->tieFor());
                                                            tie2->setStartNote(nl2[i]);
                                                            }
                                                      nl1[i]->setTieFor(tie);
                                                      nl2[i]->setTieBack(tie);
                                                      }
                                                c = c2;
                                                }
                                          }
                                    else {
                                          // split Rest
                                          Rest* r  = static_cast<Rest*>(cr);
                                          int rest = r->ticks();
                                          int len  = measureEnd - r->tick();
                                          rest    -= len;
                                          Duration d;
                                          d.setVal(len);
                                          r->setDurationType(d);
                                          undoAddCR(r, measure, tick);
                                          while (rest) {
                                                Rest* r2 = static_cast<Rest*>(r->clone());
                                                int tick = r->tick() + r->ticks();
                                                measure = tick2measure(tick);
                                                len = measure->ticks() > rest ? rest : measure->ticks();
                                                Duration d;
                                                d.setVal(len);
                                                r2->setDurationType(d);
								rest -= len;
                                                undoAddCR(r2, measure, tick);
                                                r = r2;
                                                }
                                          }
                                    }
                              else {
                                    undoAddCR(cr, measure, tick);
                                    }
                              curTick += cr->ticks();
                              }
                        else if (tag == "HairPin"
                           || tag == "Pedal"
                           || tag == "Ottava"
                           || tag == "Trill"
                           || tag == "TextLine"
                           || tag == "Volta") {
                              Spanner* sp = static_cast<Spanner*>(Element::name2Element(tag, this));
                              sp->setTrack(dstStaffIdx * VOICES);
                              sp->read(eee);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->findSegment(SegChordRest, tick);
                              if (segment == 0) {
                                    segment = new Segment(m, SegChordRest, tick);
                                    undoAddElement(segment);
                                    }
                              sp->setStartElement(segment);
                              sp->setParent(segment);
                              undoAddElement(sp);
                              }
                        else if (tag == "endSpanner") {
                              int id = eee.attribute("id").toInt();
                              Spanner* e = findSpanner(id);
                              if (e) {
                                    int tick = curTick - tickStart + dstTick;
                                    Measure* m = tick2measure(tick);
                                    Segment* seg = m->findSegment(SegChordRest, tick);
                                    if (seg == 0) {
                                          seg = new Segment(m, SegChordRest, tick);
                                          undoAddElement(seg);
                                          }
                                    e->setEndElement(seg);
                                    seg->addSpannerBack(e);
                                    if (e->type() == OTTAVA) {
                                          Ottava* o = static_cast<Ottava*>(e);
                                          int shift = o->pitchShift();
                                          Staff* st = o->staff();
                                          int tick1 = static_cast<Segment*>(o->startElement())->tick();
                                          st->pitchOffsets().setPitchOffset(tick1, shift);
                                          st->pitchOffsets().setPitchOffset(tick, 0);
                                          }
                                    else if (e->type() == HAIRPIN) {
                                          Hairpin* hp = static_cast<Hairpin*>(e);
                                          updateHairpin(hp);
                                          }
                                    }
                              }

                        else if (tag == "Lyrics") {
                              Lyrics* lyrics = new Lyrics(this);
                              lyrics->setTrack(curTrack);
                              lyrics->read(eee);
                              lyrics->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Segment* segment = tick2segment(tick);
                              if (segment) {
                                    lyrics->setParent(segment);
                                    undoAddElement(lyrics);
                                    }
                              else {
                                    delete lyrics;
                                    printf("no segment found for lyrics\n");
                                    }
                              }
                        else if (tag == "Harmony") {
                              Harmony* harmony = new Harmony(this);
                              harmony->setTrack(curTrack);
                              harmony->read(eee);
                              harmony->setTrack(dstStaffIdx * VOICES);
                              //transpose
                              Part* partDest = staff(dstStaffIdx)->part();
                              Part* partSrc = staff(srcStaffIdx)->part();
                              Interval intervalDest = partDest->instr()->transpose();
                              Interval intervalSrc = partSrc->instr()->transpose();
                              Interval interval = Interval(intervalSrc.diatonic - intervalDest.diatonic, intervalSrc.chromatic - intervalDest.chromatic);
                              if (!styleB(ST_concertPitch)) {
                                    int rootTpc = transposeTpc(harmony->rootTpc(), interval, false);
                                    int baseTpc = transposeTpc(harmony->baseTpc(), interval, false);
                                    undoTransposeHarmony(harmony, rootTpc, baseTpc);
                                    }

                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->findSegment(SegChordRest, tick);
                              if (seg == 0) {
                                    seg = new Segment(m, SegChordRest, tick);
                                    undoAddElement(seg);
                                    }
                              harmony->setParent(seg);
                              undoAddElement(harmony);
                              }
                        else if (tag == "Dynamic"
                           || tag == "Symbol"
                           || tag == "FretDiagram"
                           || tag == "Marker"
                           || tag == "Jump"
                           || tag == "Image"
                           || tag == "Text"
                           || tag == "StaffText"
                           || tag == "TempoText"
                           ) {
                              Element* e = Element::name2Element(tag, this);
                              e->read(eee);
                              e->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->findSegment(SegChordRest, tick);
                              if (seg == 0) {
                                    seg = new Segment(m, SegChordRest, tick);
                                    undoAddElement(seg);
                                    }
                              e->setParent(seg);
                              undoAddElement(e);
                              }
                        else if (tag == "Clef") {
                              Clef* clef = new Clef(this);
                              clef->read(eee);
                              clef->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              if (m->tick() && m->tick() == tick)
                                    m = m->prevMeasure();
                              Segment* segment = m->findSegment(SegClef, tick);
                              if (!segment) {
                                    segment = new Segment(m, SegClef, tick);
                                    undoAddElement(segment);
                                    }
                              segment->add(clef);
                              clef->staff()->setClef(segment->tick(), clef->clefType());
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(eee);
                              breath->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->findSegment(SegBreath, tick);
                              if (!segment) {
                                    segment = new Segment(m, SegBreath, tick);
                                    undoAddElement(segment);
                                    }
                              segment->add(breath);
                              }
                        else if (tag == "BarLine") {
                              // ignore bar line
                              }
                        else {
                              domError(eee);
                              continue;
                              }
                        }
                  foreach(Slur* slur, slurs)
                        undoAddElement(slur);
                  }
            Segment* s1 = tick2segment(dstTick);
            Segment* s2 = tick2segment(dstTick + tickLen);
            _selection.setRange(s1, s2, dstStaffStart, dstStaffStart+staves);
            _selection.updateSelectedElements();
            if (selection().state() != SEL_RANGE)
                  _selection.setState(SEL_RANGE);
            }
      connectTies();
      updateNotes();    // TODO: undoable version needed

      layoutFlags |= LAYOUT_FIX_PITCH_VELO;
      }

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void Score::moveInputPos(Segment* s)
      {
      if (s == 0)
            return;
      _is.setSegment(s);
      emit posChanged(s->tick());
#if 0 // TODO-S
      Element* el;
      if (s->element(_is.track()))
            el = s->element(_is.track());
      else
            el = s->element(_is.track() / VOICES * VOICES);
      if (el->type() == CHORD)
            el = static_cast<Chord*>(el)->upNote();
      emit adjustCanvasPosition(el, false);
#endif
      }

//---------------------------------------------------------
//   moveToNextInputPos
//   TODO: special case: note is first note of tie: goto to last note of tie
//---------------------------------------------------------

void Score::moveToNextInputPos()
      {
      Segment* s = _is.segment();
      Measure* m = s->measure();
      int track  = _is.track();
      for (s = s->next1(SegChordRest); s; s = s->next1(SegChordRest)) {
            if (s->element(track) || s->measure() != m)
                  break;
            }
      moveInputPos(s);
      }

//---------------------------------------------------------
//   move
//    move current selection
//---------------------------------------------------------

Element* Score::move(const QString& cmd)
      {
      ChordRest* cr = selection().lastChordRest();
      if (selection().activeCR())
            cr = selection().activeCR();
      if (!cr) {
            if (selection().elements().isEmpty())
                  return 0;
            Element* e = selection().elements().front();
            if (e->type() == NOTE)
                  cr = static_cast<Note*>(e)->chord();
            else if (e->isChordRest())
                  cr = static_cast<ChordRest*>(e);
            else if (e->parent() && e->parent()->type() == SEGMENT) {
                  Segment* segment = static_cast<Segment*>(e->parent());
                  if (segment->subtype() != SegChordRest) {
                        segment = segment->next1(SegChordRest);
                        Element* el = segment->element(e->track());
                        if (el == 0)
                              return 0;
                        if (el->type() == CHORD) {
                              // el = static_cast<Chord*>(el)->upNote();
                              // mscore->play(static_cast<Note*>(el));
                              mscore->play(static_cast<Chord*>(el));
                              }
                        select(el, SELECT_SINGLE, 0);
                        return el;
                        }
                  cr = static_cast<ChordRest*>(segment->element(e->track()));
                  if (cr == 0)
                        return 0;
                  }
            else if (e->type() == LYRICS) {
                  Lyrics* lyrics = static_cast<Lyrics*>(e);
                  ChordRest* cr  = lyrics->chordRest();
                  int no         = lyrics->no();
                  Element* el    = 0;
                  if (cmd == "next-chord") {
                        for (ChordRest* ncr = cr;;) {
                              ncr = nextChordRest(ncr);
                              if (ncr == 0)
                                    break;
                              if (ncr->isChordRest() && ncr->lyrics(no)) {
                                    el = ncr->lyrics(no);
                                    break;
                                    }
                              }
                        }
                  else if (cmd == "prev-chord") {
                        for (ChordRest* pcr = cr;;) {
                              pcr = prevChordRest(pcr);
                              if (pcr == 0)
                                    break;
                              if (pcr->isChordRest() && pcr->lyrics(no)) {
                                    el = pcr->lyrics(no);
                                    break;
                                    }
                              }
                        }
                  if (el)
                        select(el, SELECT_SINGLE, 0);
                  return el;
                  }
            else
                  return 0;
            }

      Element* el = 0;
      if (cmd == "next-chord") {
            if (noteEntryMode())
                  moveToNextInputPos();
            el = nextChordRest(cr);
            }
      else if (cmd == "prev-chord") {
            if (noteEntryMode()) {
                  Segment* s = _is.segment()->prev1();
                  //
                  // if _is._segment is first chord/rest segment in measure
                  // make sure "m" points to previous measure
                  //
                  while (s && s->subtype() != SegChordRest)
                        s = s->prev1();
                  if (s == 0)
                        return 0;
                  Measure* m = s->measure();

                  int track  = _is.track();
                  for (; s; s = s->prev1()) {
                        if (s->subtype() != SegChordRest)
                              continue;
                        if (s->element(track) || s->measure() != m)
                              break;
                        }
                  if (s && !s->element(track))
                        s = m->firstCRSegment();
                  moveInputPos(s);
                  }
            el = prevChordRest(cr);
            }
      else if (cmd == "next-measure"){
            el = nextMeasure(cr);
            if (noteEntryMode() && el && (el->type() == CHORD || el->type() == REST)){
                ChordRest* crc = static_cast<ChordRest*>(el);
                moveInputPos(crc->segment());
                }
            }
      else if (cmd == "prev-measure"){
            el = prevMeasure(cr);
            if (noteEntryMode() && el && (el->type() == CHORD || el->type() == REST)){
                ChordRest* crc = static_cast<ChordRest*>(el);
                moveInputPos(crc->segment());
                }
            }
      if (el) {
            if (el->type() == CHORD) {
                  el = static_cast<Chord*>(el)->upNote();
                  mscore->play(static_cast<Note*>(el));
                  }
            select(el, SELECT_SINGLE, 0);
            }
      return el;
      }

//---------------------------------------------------------
//   selectMove
//---------------------------------------------------------

Element* Score::selectMove(const QString& cmd)
      {
      ChordRest* cr;
      if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();
      ChordRest* el = 0;
      if (cr) {
            if (cmd == "select-next-chord")
                  el = nextChordRest(cr);
            else if (cmd == "select-prev-chord")
                  el = prevChordRest(cr);
            else if (cmd == "select-next-measure")
                  el = nextMeasure(cr, true);
            else if (cmd == "select-prev-measure")
                  el = prevMeasure(cr);
            else if (cmd == "select-begin-line") {
                  Measure* measure = cr->segment()->measure()->system()->firstMeasure();
                  if (!measure)
                        return 0;
                  el = measure->first()->nextChordRest(cr->track());
                  }
            else if (cmd == "select-end-line") {
                  Measure* measure = cr->segment()->measure()->system()->lastMeasure();
                  if (!measure)
                        return 0;
                  el = measure->last()->nextChordRest(cr->track(), true);
                  }
            else if (cmd == "select-begin-score") {
                  Measure* measure = first()->system()->firstMeasure();
                  if (!measure)
                        return 0;
                  el = measure->first()->nextChordRest(cr->track());
                  }
            else if (cmd == "select-end-score") {
                  Measure* measure = last()->system()->lastMeasure();
                  if (!measure)
                        return 0;
                  el = measure->last()->nextChordRest(cr->track(), true);
                  }
            else if (cmd == "select-staff-above")
                  el = upStaff(cr);
            else if (cmd == "select-staff-below")
                  el = downStaff(cr);
            if (el)
                  select(el, SELECT_RANGE, el->staffIdx());
            }
      return el;
      }

//---------------------------------------------------------
//   cmdMirrorNoteHead
//---------------------------------------------------------

void Score::cmdMirrorNoteHead()
      {
      Element* e = getSelectedElement();
      if (e->type() != NOTE)
            return;
      Note* note = static_cast<Note*>(e);
      DirectionH d = note->userMirror();
      if (d == DH_AUTO)
            d = note->chord()->up() ? DH_RIGHT : DH_LEFT;
      else
            d = d == DH_LEFT ? DH_RIGHT : DH_LEFT;
      undoChangeUserMirror(note, d);
      }

//---------------------------------------------------------
//   cmdHalfDuration
//---------------------------------------------------------

void Score::cmdHalfDuration()
      {
      Element* el = selection().element();
      if (el == 0)
            return;
      if (el->type() == NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      Duration d = _is.duration().shift(1);
      if (!d.isValid() || (d.type() > Duration::V_64TH))
            return;
      if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setDurationType(d);
            }
      else
            changeCRlen(cr, d);
      _is.setDuration(d);
      setPadState();
      nextInputPos(cr, false);
      }

//---------------------------------------------------------
//   cmdDoubleDuration
//---------------------------------------------------------

void Score::cmdDoubleDuration()
      {
      Element* el = selection().element();
      if (el == 0)
            return;
      if (el->type() == NOTE)
            el = el->parent();
      if (!el->isChordRest())
            return;

      ChordRest* cr = static_cast<ChordRest*>(el);
      Duration d = _is.duration().shift(-1);
      if (!d.isValid() || (d.type() < Duration::V_WHOLE))
            return;
      if (cr->type() == CHORD && (static_cast<Chord*>(cr)->noteType() != NOTE_NORMAL)) {
            //
            // handle appoggiatura and acciaccatura
            //
            cr->setDurationType(d);
            }
      else
            changeCRlen(cr, d);
      _is.setDuration(d);
      setPadState();
      nextInputPos(cr, false);
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void ScoreView::cmdRepeatSelection()
      {
      const Selection& selection = _score->selection();
      if (selection.isSingle() && noteEntryMode()) {
            const InputState& is = _score->inputState();
            cmdAddPitch1(is.pitch, false);
            return;
            }

      if (selection.state() != SEL_RANGE) {
            printf("wrong selection type\n");
            return;
            }

      QString mimeType = selection.mimeType();
      if (mimeType.isEmpty()) {
            printf("mime type is empty\n");
            return;
            }
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, selection.mimeData());
      if (debugMode)
            printf("cmdRepeatSelection: <%s>\n", mimeData->data(mimeType).data());
      QApplication::clipboard()->setMimeData(mimeData);

      QByteArray data(mimeData->data(mimeType));

// printf("repeat <%s>\n", data.data());

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading paste data at line %d column %d: %s\n",
               line, column, qPrintable(err));
            printf("%s\n", data.data());
            return;
            }
      docName = "--";

      int dStaff = selection.staffStart();
      Segment* endSegment = selection.endSegment();
      if (endSegment && endSegment->subtype() != SegChordRest)
            endSegment = endSegment->next(SegChordRest);
      if (endSegment && endSegment->element(dStaff * VOICES)) {
            Element* e = endSegment->element(dStaff * VOICES);
            if (e) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  _score->startCmd();
                  _score->pasteStaff(doc.documentElement(), cr);
                  _score->setLayoutAll(true);
                  _score->endCmd();
                  }
            else
                  printf("??? %p <%s>\n", e, e ? e->name() : "");
            }
      else {
            printf("cmdRepeatSelection: endSegment: %p dStaff %d\n", endSegment, dStaff);
            }
      }

//---------------------------------------------------------
//   search
//---------------------------------------------------------

void ScoreView::search(const QString& s)
      {
      bool ok;

      int n = s.toInt(&ok);
      if (!ok || n <= 0)
            return;

      int i = 0;
      for (Measure* measure = _score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (++i < n)
                  continue;
            adjustCanvasPosition(measure, true);
            int tracks = _score->nstaves() * VOICES;
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != SegChordRest)
                        continue;
                  int track;
                  for (track = 0; track < tracks; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                        if (cr) {
                              Element* e;
                              if(cr->type() == CHORD)
                                    e =  static_cast<Chord*>(cr)->upNote();
                              else //REST
                                    e = cr;

                              _score->select(e, SELECT_SINGLE, 0);
                              break;
                              }
                        }
                  if (track != tracks)
                        break;
                  }
            _score->setUpdateAll(true);
            _score->end();
            break;
            }
      }

//---------------------------------------------------------
//   cmdMoveRest
//---------------------------------------------------------

void Score::cmdMoveRest(Rest* rest, Direction dir)
      {
      QPointF pos(rest->userOff());
      if (dir == UP)
            pos.ry() -= spatium();
      else if (dir == DOWN)
            pos.ry() += spatium();
      undoChangeUserOffset(rest, pos);
      }

//---------------------------------------------------------
//   cmdMoveLyrics
//---------------------------------------------------------

void Score::cmdMoveLyrics(Lyrics* lyrics, Direction dir)
      {
      ChordRest* cr      = lyrics->chordRest();
      QList<Lyrics*>& ll = cr->lyricsList();
      int no             = lyrics->no();
      if (dir == UP) {
            if (no) {
                  if (ll[no-1] == 0) {
                        ll[no-1] = ll[no];
                        ll[no] = 0;
                        lyrics->setNo(no-1);
                        }
                  }
            }
      else {
            if (no == ll.size()-1) {
                  ll.append(ll[no]);
                  ll[no] = 0;
                  lyrics->setNo(no+1);
                  }
            else if (ll[no + 1] == 0) {
                  ll[no+1] = ll[no];
                  ll[no] = 0;
                  lyrics->setNo(no+1);
                  }
            }
      }

