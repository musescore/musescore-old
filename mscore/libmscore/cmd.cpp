//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

/**
 \file
 Handling of several GUI commands.
*/

#include <assert.h>

#include "score.h"
#include "utils.h"
#include "key.h"
#include "clef.h"
#include "navigate.h"
#include "slur.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "text.h"
#include "sig.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "page.h"
#include "barline.h"
#include "tuplet.h"
#include "xml.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "hairpin.h"
#include "textline.h"
#include "keysig.h"
#include "volta.h"
#include "dynamic.h"
#include "box.h"
#include "harmony.h"
#include "system.h"
#include "stafftext.h"
#include "articulation.h"
#include "layoutbreak.h"
#include "drumset.h"
#include "beam.h"
#include "lyrics.h"
#include "pitchspelling.h"
#include "measure.h"
#include "tempo.h"
#include "sig.h"
#include "undo.h"
#include "timesig.h"
#include "repeat.h"
#include "tempotext.h"
#include "clef.h"
#include "noteevent.h"
#include "breath.h"
#include "tablature.h"
#include "stafftype.h"
#include "segment.h"
#include "chordlist.h"
#include "mscore.h"
#include "accidental.h"
#include "sequencer.h"

//---------------------------------------------------------
//   startCmd
///   Start a GUI command by clearing the redraw area
///   and starting a user-visble undo.
//---------------------------------------------------------

void Score::startCmd()
      {
      if (MScore::debugMode)
            qDebug("===startCmd()");
      _layoutAll = true;      ///< do a complete relayout
      _playNote = false;

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (undo()->active()) {
            // if (MScore::debugMode)
            qDebug("Score::startCmd(): cmd already active");
            // abort();
            return;
            }
      undo()->beginMacro();
      undo(new SaveState(this));
      }

//---------------------------------------------------------
//   endCmd
///   End a GUI command by (if \a undo) ending a user-visble undo
///   and (always) updating the redraw area.
//---------------------------------------------------------

void Score::endCmd()
      {
      if (!undo()->active()) {
            // if (MScore::debugMode)
                  qDebug("Score::endCmd(): no cmd active");
            end();
            return;
            }

      foreach(Score* s, scoreList())
            s->end2();

      bool noUndo = undo()->current()->childCount() <= 1;
      if (!noUndo)
            setDirty(!noUndo);
      undo()->endMacro(noUndo);
      end();      // DEBUG
      }

//---------------------------------------------------------
//   end
///   Update the redraw area.
//---------------------------------------------------------

void Score::end()
      {
      foreach(Score* s, scoreList())
            s->end1();
      }

//---------------------------------------------------------
//   update
//    layout & update
//---------------------------------------------------------

void Score::update()
      {
      foreach(Score* s, scoreList()) {
            s->end2();
            s->end1();
            }
      }

//---------------------------------------------------------
//   end2
//---------------------------------------------------------

void Score::end2()
      {
      bool _needLayout = false;
      if (_layoutAll) {
            _updateAll  = true;
            _needLayout = true;
            startLayout = 0;
            }
      else if (startLayout) {
            _updateAll = true;
            _needLayout = true;
            }
      if (_needLayout)
            doLayout();
      _layoutAll   = false;
      startLayout = 0;
      }

//---------------------------------------------------------
//   end1
//---------------------------------------------------------

void Score::end1()
      {
      if (_updateAll) {
            foreach(MuseScoreView* v, viewer)
                  v->updateAll();
            }
      else {
            // update a little more:
            qreal d = spatium() * .5;
            refresh.adjust(-d, -d, 2 * d, 2 * d);
            foreach(MuseScoreView* v, viewer)
                  v->dataChanged(refresh);
            }
      refresh     = QRectF();
      _updateAll  = false;
      }

//---------------------------------------------------------
//   endUndoRedo
///   Common handling for ending undo or redo
//---------------------------------------------------------

void Score::endUndoRedo()
      {
      updateSelection();
      foreach(Score* score, scoreList()) {
            if (score->layoutAll()) {
                  score->setUndoRedo(true);
                  score->doLayout();           // TODO: does not really work
                                               // creation/deletion of elements are not allowed
                  score->setUndoRedo(false);
                  score->setUpdateAll(true);
                  }
            score->setPlaylistDirty(true);
            }
      end();
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void Score::moveCursor()
      {
      foreach(MuseScoreView* v, viewer)
            v->moveCursor();
      }

//---------------------------------------------------------
//   cmdAddSpanner
//   drop VOLTA, OTTAVA, TRILL, PEDAL, DYNAMIC
//        HAIRPIN, and TEXTLINE
//---------------------------------------------------------

void Score::cmdAddSpanner(Spanner* spanner, const QPointF& pos, const QPointF& /*dragOffset*/)
      {
      int staffIdx;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &staffIdx, 0, &segment, 0);
      if (mb == 0 || mb->type() != MEASURE) {
            qDebug("cmdAddSpanner: cannot put object here");
            delete spanner;
            return;
            }

      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;
      spanner->setTrack(track);

      if (spanner->anchor() == ANCHOR_SEGMENT) {
            spanner->setStartElement(segment);
            spanner->setParent(segment);

            static const SegmentType st = SegChordRest;
            Segment* ns = 0;
            for (Segment* s = segment; s; s = s->next1(st)) {
                  ns = s;
                  if (s->measure() != segment->measure())
                        break;
                  }
            if (ns == segment) {
                  qDebug("cmdAddSpanner: cannot put object on last segment");
                  delete spanner;
                  return;
                  }
            spanner->setEndElement(ns);
            }
      else {      // ANCHOR_MEASURE
            Measure* m = static_cast<Measure*>(mb);
            QRectF b(m->canvasBoundingRect());

            if (pos.x() >= (b.x() + b.width() * .5))
                  m = m->nextMeasure();
            spanner->setStartElement(m);
            spanner->setEndElement(m);
            spanner->setParent(m);
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
                  int key    = chord->staff()->key(segment->tick()).accidentalType();
                  int pitch2 = diatonicUpDown(key, pitch, 1);
                  int dpitch = pitch2 - pitch;
                  for (int i = 0; i < n; i += 2) {
                        events.append(new NoteEvent(0,      i * 1000 / n,    1000/n));
                        events.append(new NoteEvent(dpitch, (i+1) *1000 / n, 1000/n));
                        }
                  undo(new ChangeNoteEvents(chord, events));
                  }
            }
      }

//---------------------------------------------------------
//   expandVoice
//---------------------------------------------------------

void Score::expandVoice(Segment* s, int track)
      {
      if (s->element(track)) {
            ChordRest* cr = (ChordRest*)(s->element(track));
            qDebug("expand voice: found %s %s", cr->name(), qPrintable(cr->duration().print()));
            return;
            }

      Segment* ps;
      for (ps = s; ps; ps = ps->prev(SegChordRest)) {
            if (ps->element(track))
                  break;
            }
      if (ps) {
            ChordRest* cr = static_cast<ChordRest*>(ps->element(track));
            int tick = cr->tick() + cr->actualTicks();
            if (tick == s->tick())
                  return;
            if (tick > s->tick()) {
                  qDebug("expandVoice: cannot insert element here");
                  return;
                  }
            }
      //
      // fill upto s->tick() with rests
      //
      Measure* m = s->measure();
      int stick  = ps ?  ps->tick() : m->tick();
      int ticks  = s->tick() - stick;
      if (ticks)
            setRest(stick, track, Fraction::fromTicks(ticks), false, 0);

      //
      // fill from s->tick() until next chord/rest
      //
      Segment* ns;
      for (ns = s->next(SegChordRest); ns; ns = ns->next(SegChordRest)) {
            if (ns->element(track))
                  break;
            }
      ticks  = ns ? (ns->tick() - s->tick()) : (m->ticks() - s->rtick());
      if (ticks == m->ticks())
            addRest(s, track, TDuration(TDuration::V_MEASURE), 0);
      else
            setRest(s->tick(), track, Fraction::fromTicks(ticks), false, 0);
      }

void Score::expandVoice()
      {
      Segment* s = _is.segment();
      int track  = _is.track();
      expandVoice(s, track);
      }

//---------------------------------------------------------
//   addPitch
//---------------------------------------------------------

Note* Score::addPitch(int pitch, bool addFlag)
      {
qDebug("add pitch %d %d", pitch, addFlag);

      if (addFlag) {
            if (_is.cr() == 0 || _is.cr()->type() != CHORD)
                  return 0;
            Chord* chord = static_cast<Chord*>(_is.cr());
            Note* n = addNote(chord, pitch);
            setLayoutAll(false);
            setLayout(chord->measure());
            moveToNextInputPos();
            return n;
            }
      expandVoice();

      // insert note
      Direction stemDirection = AUTO;
      NoteHeadGroup headGroup = HEAD_NORMAL;
      int track               = _is.track();
      if (_is.drumNote() != -1) {
            pitch         = _is.drumNote();
            Drumset* ds   = _is.drumset();
            headGroup     = ds->noteHead(pitch);
            stemDirection = ds->stemDirection(pitch);
            track         = ds->voice(pitch) + (_is.track() / VOICES) * VOICES;
            _is.setTrack(track);
            expandVoice();
            }
      if (!_is.cr())
            return 0;
      NoteVal nval;
      nval.pitch     = pitch;
      nval.headGroup = headGroup;
      Fraction duration;
      if (_is.repitchMode())
            duration = _is.cr()->duration();
      else
            duration = _is.duration().fraction();
      Segment* seg   = setNoteRest(_is.segment(), track, nval, duration, stemDirection);
      Note* note     = 0;
      if (seg) {
            note = static_cast<Chord*>(seg->element(track))->upNote();
            setLayout(note->chord()->measure());
            }

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
                  qDebug("addPitch: cannot find slur note");
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

            int npitch;
            int ntpc;
            if( abs(valTmp) != 7 ) {
                  int line = on->line() - valTmp;
                  int tick   = chord->tick();
                  Staff* estaff = staff(on->staffIdx() + chord->staffMove());
                  int clef   = estaff->clef(tick);
                  int key    = estaff->key(tick).accidentalType();
                  npitch = line2pitch(line, clef, key);
                  ntpc   = pitch2tpc(npitch, key);
                  }
            else { //special case for octave
                  Interval interval(7, 12);
                  if(val < 0) {
                        interval.flip();
                        }
                  transposeInterval(on->pitch(), on->tpc(), &npitch, &ntpc, interval, false);
                  }
            note->setPitch(npitch, ntpc);

            undoAddElement(note);
            _playNote = true;
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

void Score::setGraceNote(Chord* ch, int pitch, NoteType type, bool behind, int len)
      {
      Note* note = new Note(this);
      note->setPitch(pitch);

      Chord* chord = new Chord(this);
      chord->setTrack(ch->track());
      chord->add(note);

      TDuration d;
      d.setVal(len);
      chord->setDurationType(d);
      chord->setDuration(d.fraction());
      chord->setNoteType(type);
      chord->setMag(ch->staff()->mag() * styleD(ST_graceNoteMag));

      undoAddGrace(chord, ch->segment(), behind);

      note->setTpcFromPitch();      // tick must be known
      select(note, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(Segment* segment, int track, NoteVal nval, Fraction sd,
   Direction stemDirection)
      {
      if (segment->subtype() == SegGrace) {
            Chord* chord = static_cast<Chord*>(segment->element(track));
            if (chord->notes().size() == 1) {
                  Note* note = chord->upNote();
                  int tpc = pitch2tpc2(nval.pitch, true);
                  int line = note->line();
                  undoChangePitch(note, nval.pitch, tpc, line/*, nval.fret, nval.string*/);
                  }
            return segment;
            }
      assert(segment->subtype() == SegChordRest);

      int tick      = segment->tick();
      Element* nr   = 0;
      Tie* tie      = 0;
      ChordRest* cr = static_cast<ChordRest*>(segment->element(track));

      Measure* measure = 0;
      for (;;) {
            if (track % VOICES)
                  expandVoice(segment, track);

            // the returned gap ends at the measure boundary or at tuplet end
            Fraction dd = makeGap(segment, track, sd, cr ? cr->tuplet() : 0);

            if (dd.isZero()) {
                  qDebug("cannot get gap at %d type: %d/%d", tick, sd.numerator(),
                     sd.denominator());
                  break;
                  }
            QList<TDuration> dl = toDurationList(dd, true);

            measure = segment->measure();
            int n = dl.size();
            for (int i = 0; i < n; ++i) {
                  TDuration d = dl[i];

                  ChordRest* ncr;
                  Note* note = 0;
                  if (nval.pitch == -1) {
                        nr = ncr = new Rest(this);
                        nr->setTrack(track);
                        ncr->setDurationType(d);
                        ncr->setDuration(d.fraction());
                        }
                  else {
                        nr = note = new Note(this);

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
                              tie->setStartNote(note);
                              tie->setTrack(track);
                              note->setTieFor(tie);
                              }
                        }
                  ncr->setTuplet(cr ? cr->tuplet() : 0);
                  undoAddCR(ncr, measure, tick);
                  _playNote = true;
                  segment = ncr->segment();
                  tick += ncr->actualTicks();
                  }

            sd -= dd;
            if (sd.isZero())
                  break;

            Segment* nseg = tick2segment(tick, false, SegChordRest);
            if (nseg == 0) {
                  qDebug("reached end of score");
                  break;
                  }
            segment = nseg;

            cr = static_cast<ChordRest*>(segment->element(track));

            if (cr == 0) {
                  if (track % VOICES)
                        cr = addRest(segment, track, TDuration(TDuration::V_MEASURE), 0);
                  else {
                        qDebug("no rest in voice 0");
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
      return segment;
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

Fraction Score::makeGap(Segment* segment, int track, const Fraction& _sd, Tuplet* tuplet)
      {
qDebug("makeGap %s at %d track %d", qPrintable(_sd.print()), segment->tick(), track);
      assert(_sd.numerator());

      Measure* measure = segment->measure();
      setLayout(measure);
      Fraction akkumulated;
      Fraction sd = _sd;

      //
      // remember first segment which should
      // not be deleted (it may contain other elements we want to preserve)
      //
      Segment* firstSegment = segment;
      int nextTick = segment->tick();

      for (Segment* seg = firstSegment; seg; seg = seg->next(SegChordRest | SegGrace)) {
            if (seg->subtype() == SegGrace) {
                  if (seg->element(track)) {
                        undoRemoveElement(seg->element(track));
                        if (seg->isEmpty() && seg != firstSegment)
                              undoRemoveElement(seg);
                        }
                  continue;
                  }
            //
            // voices != 0 may have gaps:
            //
            ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
            if (!cr) {
                  if (seg->tick() < nextTick)
                        continue;
                  Segment* seg1 = seg->next(SegChordRest);
                  int tick2 = seg1 ? seg1->tick() : seg->measure()->tick() + seg->measure()->ticks();
                  Fraction td(Fraction::fromTicks(tick2 - seg->tick()));
                  segment = seg;
                  if (td > sd)
                        td = sd;
                  akkumulated += td;
                  sd -= td;
                  if (sd.isZero())
                        return akkumulated;
                  nextTick = seg1->tick();
                  continue;
                  }
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
//                        qDebug("makeGap: end of tuplet reached");
                        return akkumulated;
                        }
                  }
            Fraction td(cr->duration());
qDebug("remove %s %s at tick %d track %d",
   cr->name(), qPrintable(cr->duration().print()), seg->tick(), track);

            Tuplet* ltuplet = cr->tuplet();
            if (cr->tuplet() != tuplet) {
//                  qDebug("   remove tuplet %d", sd >= ltuplet->fraction());
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
qDebug("  makeGap: remove %d/%d at %d", td.numerator(), td.denominator(), cr->tick());
                  undoRemoveElement(cr);
                  if (seg->isEmpty() && seg != firstSegment)
                        undoRemoveElement(seg);
                  else if (seg != firstSegment) {     // keep _all_ annotations on first segment?
                        foreach(Element* e, seg->annotations()) {
                              if (e->track() == cr->track())
                                    undoRemoveElement(e);
                              }
                        }
                  }
            nextTick += td.ticks();
            if (sd < td) {
                  //
                  // we removed too much
                  //
                  akkumulated = _sd;
                  Fraction rd = td - sd;

qDebug("  makeGap: %d/%d removed %d/%d too much", sd.numerator(), sd.denominator(), rd.numerator(), rd.denominator());

                  QList<TDuration> dList = toDurationList(rd, false);
                  if (dList.isEmpty())
                        return akkumulated;
qDebug("   dList: %d\n", dList.size());

                  Fraction f(cr->staff()->timeStretch(cr->tick()) * sd);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        f /= t->ratio();
                  int tick  = cr->tick() + f.ticks();
qDebug("   gap at tick %d+%d", cr->tick(), f.ticks());

                  if ((tuplet == 0) && (((measure->tick() - tick) % dList[0].ticks()) == 0)) {
                        foreach(TDuration d, dList) {
                              qDebug("    addClone at %d, %d", tick, d.ticks());
                              tick += addClone(cr, tick, d)->actualTicks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i)
                              tick += addClone(cr, tick, dList[i])->actualTicks();
                        }
// qDebug("  return %d/%d", akkumulated.numerator(), akkumulated.denominator());
                  return akkumulated;
                  }
            akkumulated += td;
            sd          -= td;
qDebug("  akkumulated %d/%d rest %d/%d (-%d/%d)",
   akkumulated.numerator(), akkumulated.denominator(), sd.numerator(), sd.denominator(),
   td.numerator(), td.denominator());
            if (sd.isZero())
                  return akkumulated;
            }
//      int ticks = measure->tick() + measure->ticks() - segment->tick();
//      Fraction td = Fraction::fromTicks(ticks);
// NEEDS REVIEW !!
// once the statement below is removed, these two lines do nothing
//      if (td > sd)
//            td = sd;
// ???  akkumulated should already contain the total value of the created gap: line 749, 811 or 838
//      this line creates a qreal-sized gap if the needed gap crosses a measure boundary
//      by adding again the duration already added in line 838
//      akkumulated += td;
      return akkumulated;
      }

//---------------------------------------------------------
//   makeGap1
//    make time gap at tick by removing/shortening
//    chord/rest
//    - cr is top level (not part of a tuplet)
//    - do not stop at measure end
//---------------------------------------------------------

bool Score::makeGap1(int tick, int staffIdx, Fraction len)
      {
      ChordRest* cr = 0;
      Segment* seg = tick2segment(tick, true, SegChordRest | SegGrace);
      if (!seg) {
            qDebug("1:makeGap1: no segment at %d", tick);
            return false;
            }
      int track = staffIdx * VOICES;
      cr = static_cast<ChordRest*>(seg->element(track));
      if (!cr) {
            if (seg->subtype() & SegGrace) {
                  seg = seg->next1(SegChordRest);
                  if (!seg || !seg->element(track)) {
                        qDebug("makeGap1: no chord/rest at %d staff %d", tick, staffIdx);
                        return false;
                        }
                  cr = static_cast<ChordRest*>(seg->element(track));
                  }
            else {
                  // check if we are in the middle of a chord/rest
                  Segment* seg1 = 0;
                  for (;;) {
                        seg1 = seg->prev(SegChordRest);
                        if (seg1 == 0) {
                              qDebug("1:makeGap1: no segment at %d", tick);
                              return false;
                              }
                        if (seg1->element(track))
                              break;
                        }
                  ChordRest* cr1 = static_cast<ChordRest*>(seg1->element(track));
                  Fraction dstF = Fraction::fromTicks(tick - cr1->tick());
                  len -= cr1->duration() - dstF;
                  undoChangeChordRestLen(cr1, TDuration(dstF));
                  for (;;) {
                        seg = seg->next1(SegChordRest | SegGrace);
                        if (seg == 0) {
                              qDebug("2:makeGap1: no segment");
                              return false;
                              }
                        if (seg->element(track)) {
                              tick = seg->tick();
                              cr = static_cast<ChordRest*>(seg->element(track));
                              break;
                              }
                        }
                  }
            }

      for (;;) {
            if (!cr) {
                  qDebug("makeGap1: cannot make gap");
                  return false;
                  }
            Fraction l = makeGap(cr->segment(), cr->track(), len, 0);
            if (l.isZero()) {
                  qDebug("makeGap1: makeGap returns zero gap");
                  return false;
                  }
            len -= l;
            if (len.isZero())
                  break;
            // go to next cr
            Measure* m = cr->measure()->nextMeasure();
            if (m == 0) {
                  qDebug("EOS reached");
                  insertMeasure(MEASURE, 0, false);
                  m = cr->measure()->nextMeasure();
                  if (m == 0) {
                        qDebug("===EOS reached");
                        return true;
                        }
                  }
            Segment* s = m->firstCRSegment();
            int track  = cr->track();
            cr = static_cast<ChordRest*>(s->element(track));
            if (cr == 0) {
                  addRest(s, track, TDuration(TDuration::V_MEASURE), 0);
                  cr = static_cast<ChordRest*>(s->element(track));
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   splitGapToMeasureBoundaries
//    cr  - start of gap
//    gap - gap len
//---------------------------------------------------------

QList<Fraction> Score::splitGapToMeasureBoundaries(ChordRest* cr, Fraction gap)
      {
      QList<Fraction> flist;

      Tuplet* tuplet = cr->tuplet();
      if (tuplet) {
            Fraction rest(tuplet->duration());

            if (rest < gap)
                  qDebug("does not fit in tuplet");
            else
                  flist.append(gap);
            return flist;
            }

      Segment* s = cr->segment();
      while (gap > Fraction(0)) {
            Measure* m    = s->measure();
            Fraction rest = Fraction::fromTicks(m->ticks() - s->rtick());
            if (rest >= gap) {
                  flist.append(gap);
                  return flist;
                  }
            flist.append(rest);
            gap -= rest;
            m = m->nextMeasure();
            if (m == 0)
                  return flist;
            s = m->first(SegChordRest);
            }
      return flist;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const TDuration& d)
      {
      deselectAll();
      Fraction srcF(cr->duration());
      Fraction dstF;
      if (d.type() == TDuration::V_MEASURE)
            dstF = cr->measure()->stretchedLen(cr->staff());
      else
            dstF = d.fraction();

qDebug("changeCRlen: %d/%d -> %d/%d", srcF.numerator(), srcF.denominator(),
      dstF.numerator(), dstF.denominator());

      if (srcF == dstF)
            return;
      int track = cr->track();
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
            undoChangeChordRestLen(cr, TDuration(dstF));
            setRest(cr->tick() + cr->actualTicks(), track, srcF - dstF, false, tuplet);
            select(cr, SELECT_SINGLE, 0);
            return;
            }

      //
      // make longer
      //
      // split required len into Measures
      QList<Fraction> flist = splitGapToMeasureBoundaries(cr, dstF);
      if (flist.isEmpty())
            return;

qDebug("ChangeCRLen::List:");
      foreach (Fraction f, flist)
            qDebug("  %d/%d", f.numerator(), f.denominator());

      int tick       = cr->tick();
      Fraction f     = dstF;
      ChordRest* cr1 = cr;
      Chord* oc      = 0;

      bool first = true;
      foreach (Fraction f2, flist) {
            f  -= f2;
            makeGap(cr1->segment(), cr1->track(), f2, tuplet);

            if (cr->type() == REST) {
qDebug("  +ChangeCRLen::setRest %d/%d", f2.numerator(), f2.denominator());
                  Fraction timeStretch = cr1->staff()->timeStretch(cr1->tick());
                  Rest* r = setRest(tick, track, f2 * timeStretch, (d.dots() > 0), tuplet);
                  if (first) {
                        select(r, SELECT_SINGLE, 0);
                        first = false;
                        }
qDebug("  ChangeCRLen:: %d += %d(actual=%d)", tick, f2.ticks(), f2.ticks() * timeStretch.numerator() / timeStretch.denominator());
                  tick += f2.ticks() * timeStretch.numerator() / timeStretch.denominator();
                  }
            else {
                  QList<TDuration> dList = toDurationList(f2, true);
                  Measure* measure = tick2measure(tick);
                  int etick = measure->tick();
//                  if (measure->tick() != tick)
//                        etick += measure->ticks();
                  if (((tick - etick) % dList[0].ticks()) == 0) {
                        foreach(TDuration d, dList) {
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
                              if (oc && first) {
                                    select(oc, SELECT_SINGLE, 0);
                                    first = false;
                                    }
                              if (oc)
                                    tick += oc->actualTicks();
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
                              tick += oc->actualTicks();
                              }
                        }
                  }
            Measure* m  = cr1->measure();
            Measure* m1 = m->nextMeasure();
            if (m1 == 0)
                  break;
            Segment* s = m1->firstCRSegment();
            expandVoice(s, track);
            cr1 = static_cast<ChordRest*>(s->element(track));
            }
      connectTies();
      }

//---------------------------------------------------------
//   upDown
///   Increment/decrement pitch of note by one or by an octave.
//---------------------------------------------------------

void Score::upDown(bool up, UpDownMode mode)
      {
      QList<Note*> el;
      int tick = -1;
      foreach (Note* note, selection().noteList()) {
            while (note->tieBack())
                  note = note->tieBack()->startNote();
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  if (el.indexOf(note) == -1) {
                        el.append(note);
                        if (tick == -1)
                              tick = note->chord()->tick();
                        }
                  }
            }
      if (el.empty())
            return;

      foreach(Note* oNote, el) {
            Part* part  = oNote->staff()->part();
            int pitch   = oNote->pitch();
            int newTpc = 0;
            int newPitch = 0;
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
                              case UP_DOWN_OCTAVE:          // move same note to next string, if possible
                                    {
                                    string += (up ? -1 : 1);
//                                    if (string < 0)
//                                          string = 0;
//                                    else if (string >= tab->strings())
//                                          string = tab->strings() - 1;
//                                    fret = 0;
//                                    newPitch      = tab->getPitch(string, fret);
//                                    Chord* chord  = oNote->chord();
//                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
//                                    KeySigEvent ks = estaff->key(chord->tick());
//                                    newTpc         = pitch2tpc(newPitch, ks.accidentalType());
                                    if(string < 0 || string >= tab->strings())
                                          return;           // no next string to move to
                                    fret = tab->fret(pitch, string);
                                    if(fret == -1)          // can't have that note on that string
                                          return;
                                    newPitch = pitch;       // these didn't change
                                    newTpc   = oNote->tpc();
                                    }
                                    break;

                              case UP_DOWN_CHROMATIC:       // increase / decrease the pitch,
                                                            // letting the algorithm to choose fret & string
                                    newPitch = up ? pitch+1 : pitch-1;
                                    if (newPitch < 0)
                                          newPitch = 0;
                                    else if (newPitch > 127)
                                          newPitch = 127;
                                    newTpc = pitch2tpc2(newPitch, up);
                                    break;

                              case UP_DOWN_DIATONIC:        // increase / decrease the fret
                                    {                       // without changing the string
                                    fret += (up ? 1 : -1);
                                    if (fret < 0)
                                          fret = 0;
                                    else if (fret >= tab->frets())
                                          fret = tab->frets() - 1;
                                    newPitch      = tab->getPitch(string, fret);
//                                    Chord* chord  = oNote->chord();
//                                    Staff* estaff = staff(chord->staffIdx() + chord->staffMove());
//                                    KeySigEvent ks = estaff->key(chord->tick());
//                                    newTpc         = pitch2tpc(newPitch, ks.accidentalType());
                                    newTpc = pitch2tpc2(newPitch, up);
                                    // store the fretting change before undoChangePitch() chooses
                                    // a fretting of its own liking!
                                    undoChangeFret(oNote, fret, string);
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

            if ( (oNote->pitch() != newPitch) || (oNote->tpc() != newTpc) )
                  undoChangePitch(oNote, newPitch, newTpc, oNote->line()/*, fret, string*/);
            // store fret change only if undoChangePitch has not been called,
            // as undoChangePitch() already manages fret changes, if necessary
            else if( oNote->staff()->staffType()->group() == TAB_STAFF
                        && (oNote->string() != string || oNote->fret() != fret) )
                  undoChangeFret(oNote, fret, string);

            // play new note with velocity 80 for 0.3 sec:
            _playNote = true;
            }
      _selection.updateState();     // accidentals may have changed
      }

//---------------------------------------------------------
//   addArticulation
///   Add attribute \a attr to all selected notes/rests.
///
///   Called from padToggle() to add note prefix/accent.
//---------------------------------------------------------

void Score::addArticulation(ArticulationType attr)
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
      int acc2   = measure->findAccidental(note);
      AccidentalType accType;

      int pitch, tpc;
      if (accidental == ACC_NONE) {
            //
            //  delete accidentals
            //
            accType = ACC_NONE;
            pitch   = line2pitch(note->line(), clef, 0) + acc2;
            tpc     = step2tpc(step, acc2);
            // check if there's accidentals left, previously set as
            // precautionary accidentals
            Accidental *a_rem = note->accidental();
            if (a_rem)
                  undoRemoveElement(note->accidental());
            }
      else {
            if (acc2 == acc) {
                  //
                  // this is a precautionary accidental
                  //
                  accType = accidental;
                  pitch = line2pitch(note->line(), clef, 0) + Accidental::subtype2value(accType);
                  tpc   = step2tpc(step, acc);

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
                  s   = m->findSegment(segment->subtype(), segment->tick());
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
            undo(new ChangePitch(n, pitch, tpc, n->line()/*, fret, string*/));
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
                              undo(new ChangePitch(nn, pitch, tpc, nn->line()/*, fret, string*/));
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
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      Measure* m1;
      Measure* m2;
      // retrieve span of selection
      Segment* s1 = _selection.startSegment();
      Segment* s2 = _selection.endSegment();
      // if either segment is not returned by the selection
      // (for instance, no selection) fall back to first/last measure
      if(!s1)
            m1 = firstMeasure();
      else
            m1 = s1->measure();
      if(!s2)
            m2 = lastMeasure();
      else
            m2 = s2->measure();
      if(!m1 || !m2)                // should not happen!
            return;

      for (Measure* m = m1; m; m = m->nextMeasure()) {
            _undo->push(new ChangeStretch(m, 1.0));
            if (m == m2)
                  break;
            }
      _layoutAll = true;
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
      undo(new ChangeChordStaffMove(chord, staffMove - 1));
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

      if ((staffMove == 1) || (rstaff + staffMove >= rstaves - 1)) {
qDebug("moveDown staffMove==%d  rstaff %d rstaves %d", staffMove, rstaff, rstaves);
            return;
            }
      undo(new ChangeChordStaffMove(chord, staffMove + 1));
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(qreal val)
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
            qreal stretch = m->userStretch();
            stretch += val;
            undo(new ChangeStretch(m, stretch));
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   cmdInsertClef
//---------------------------------------------------------

void Score::cmdInsertClef(ClefType type)
      {
      if (!noteEntryMode())
            return;
      undoChangeClef(staff(inputTrack()/VOICES), inputState().segment(), type);
      }

//---------------------------------------------------------
//   cmdResetBeamMode
//---------------------------------------------------------

void Score::cmdResetBeamMode()
      {
      if (selection().state() != SEL_RANGE) {
            qDebug("no system or staff selected");
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
                              undoChangeProperty(cr, P_BEAM_MODE, int(BEAM_AUTO));
                        }
                  else if (cr->type() == REST) {
                        if (cr->beamMode() != BEAM_NO)
                              undoChangeProperty(cr, P_BEAM_MODE, int(BEAM_NO));
                        }
                  }
            }
      _layoutAll = true;
      }

//---------------------------------------------------------
//   processMidiInput
//---------------------------------------------------------

bool Score::processMidiInput()
      {
      if (MScore::debugMode)
          qDebug("processMidiInput");
      if (midiInputQueue.isEmpty())
            return false;

      bool cmdActive = false;
      Note* n = 0;
      while (!midiInputQueue.isEmpty()) {
            MidiInputEvent ev = midiInputQueue.dequeue();
            if (MScore::debugMode)
                  qDebug("<-- !noteentry dequeue %i", ev.pitch);
            if (!noteEntryMode()) {
                  int staffIdx = selection().staffStart();
                  Part* p;
                  if (staffIdx < 0 || staffIdx >= nstaves())
                        p = part(0);
                  else
                        p = staff(staffIdx)->part();
                  if (p)
                        MScore::seq->startNote(p->instr()->channel(0), ev.pitch, 80,
                           MScore::defaultPlayDuration, 0.0);
                  }
            else  {
                  if (!cmdActive) {
                        startCmd();
                        cmdActive = true;
                        }
                  n = addPitch(ev.pitch, ev.chord);
                  }
            }
      if (cmdActive) {
            _layoutAll = true;
            endCmd();
            //after relayout
            foreach(MuseScoreView* v, viewer)
                  v->adjustCanvasPosition(n, false);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste(MuseScoreView* view)
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0) {
            qDebug("no application mime data");
            return;
            }
      if (selection().isSingle() && ms->hasFormat(mimeSymbolFormat)) {
            QByteArray data(ms->data(mimeSymbolFormat));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  qDebug("error reading paste data at line %d column %d: %s",
                     line, column, qPrintable(err));
                  qDebug("%s", data.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();
            QPointF dragOffset;
            Fraction duration(1, 4);
            ElementType type = Element::readType(e, &dragOffset, &duration);
            if (type != INVALID) {
                  Element* el = Element::create(type, this);
                  if (el) {
                        el->read(e);
                        addRefresh(selection().element()->abbox());   // layout() ?!
                        DropData ddata;
                        ddata.view       = view;
                        ddata.element    = el;
                        ddata.duration   = duration;
                        selection().element()->drop(ddata);
                        if (selection().element())
                              addRefresh(selection().element()->abbox());
                        }
                  }
            else
                  qDebug("cannot read type");
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
                        qDebug("cannot paste to %s", e->name());
                        return;
                        }
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  cr  = static_cast<ChordRest*>(e);
                  }
            if (cr == 0) {
                  qDebug("no destination for paste");
                  return;
                  }

            QByteArray data(ms->data(mimeStaffListFormat));
qDebug("paste <%s>", data.data());
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  qDebug("error reading paste data at line %d column %d: %s",
                     line, column, qPrintable(err));
                  qDebug("%s", data.data());
                  return;
                  }
            docName = "--";
            pasteStaff(doc.documentElement(), cr);
            }
      else if (ms->hasFormat(mimeSymbolListFormat) && selection().isSingle()) {
            qDebug("cannot paste symbol list to element");
            }
      else {
            qDebug("cannot paste selState %d staffList %d",
               selection().state(), ms->hasFormat(mimeStaffListFormat));
            foreach(const QString& s, ms->formats())
                  qDebug("  format %s", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(const QDomElement& de, ChordRest* dst)
      {
      beams.clear();
      spanner.clear();
//      QList<Tuplet*> invalidTuplets;

      for (Segment* s = firstMeasure()->first(SegChordRest); s; s = s->next1(SegChordRest)) {
            foreach(Spanner* e, s->spannerFor())
                  e->setId(-1);
            }
      int dstStaffStart = dst->staffIdx();
      int dstTick = dst->tick();
      for (QDomElement e = de; !e.isNull(); e = e.nextSiblingElement()) {
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
                  if (!makeGap1(dst->tick(), staffIdx, Fraction::fromTicks(tickLen))) {
qDebug("cannot make gap in staff %d at tick %d", staffIdx, dst->tick());
                        blackList.insert(staffIdx);
                        }
                  }
            bool pasted = false;
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
                  QList<Spanner*> spanner;
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        pasted = true;
                        const QString& tag(eee.tagName());
                        if (tag == "tick")
                              curTick = eee.text().toInt();
                        else if (tag == "Tuplet") {
                              Tuplet* tuplet = new Tuplet(this);
                              tuplet->setTrack(curTrack);
                              tuplet->read(eee, &tuplets, &spanner);
                              int tick = curTick - tickStart + dstTick;
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplet->setTick(tick);
                              tuplets.append(tuplet);
                              }
                        else if (tag == "Slur") {
                              Slur* slur = new Slur(this);
                              slur->read(eee);
                              slur->setTrack(dstStaffIdx * VOICES);
                              spanner.append(slur);
                              }
                        else if (tag == "Chord" || tag == "Rest" || tag == "RepeatMeasure") {
                              ChordRest* cr = static_cast<ChordRest*>(Element::name2Element(tag, this));
                              cr->setTrack(curTrack);
                              cr->read(eee, &tuplets, &spanner);
                              cr->setSelected(false);
                              int voice = cr->voice();
                              int track = dstStaffIdx * VOICES + voice;
                              cr->setTrack(track);
                              int tick = curTick - tickStart + dstTick;
#if 0
                              //
                              // check for tuplet
                              //
                              if (cr->tuplet()) {
                                    Tuplet* tuplet = cr->tuplet();
                                    if (tuplet->elements().isEmpty()) {
                                          Measure* measure = tick2measure(tick);
                                          int measureEnd = measure->tick() + measure->ticks();
                                          if (tick + tuplet->actualTicks() > measureEnd) {
                                                invalidTuplets.append(tuplet);
                                                cr->setDuration(tuplet->duration());
                                                cr->setDurationType(cr->duration());
                                                cr->setTuplet(0);
                                                tuplet->add(cr);
                                                qDebug("cannot paste tuplet across bar line");
                                                }
                                          }
                                    else {
                                          foreach(Tuplet* t, invalidTuplets) {
                                                if (tuplet == t) {
                                                      delete cr;
                                                      cr = 0;
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              if (cr == 0)
                                    continue;
#endif
                              curTick += cr->actualTicks();
                              pasteChordRest(cr, tick);
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
                              Segment* segment = m->undoGetSegment(SegChordRest, tick);
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
                                    Segment* seg = m->undoGetSegment(SegChordRest, tick);
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
                                    qDebug("no segment found for lyrics");
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
                              Segment* seg = m->undoGetSegment(SegChordRest, tick);
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
                              e->setTrack(dstStaffIdx * VOICES);

                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* seg = m->undoGetSegment(SegChordRest, tick);
                              e->setParent(seg);
                              e->read(eee);

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
                              Segment* segment = m->undoGetSegment(SegClef, tick);
                              clef->setParent(segment);
                              undoAddElement(clef);
                              }
                        else if (tag == "Breath") {
                              Breath* breath = new Breath(this);
                              breath->read(eee);
                              breath->setTrack(dstStaffIdx * VOICES);
                              int tick = curTick - tickStart + dstTick;
                              Measure* m = tick2measure(tick);
                              Segment* segment = m->undoGetSegment(SegBreath, tick);
                              breath->setParent(segment);
                              undoAddElement(breath);
                              }
                        else if (tag == "BarLine") {
                              // ignore bar line
                              }
                        else {
                              domError(eee);
                              continue;
                              }
                        }
                  foreach(Spanner* s, spanner) {
                        if (s->type() == SLUR)
                              undoAddElement(s);
                        }
                  foreach (Tuplet* tuplet, tuplets) {
                        if (tuplet->elements().isEmpty()) {
                              // this should not happen and is a sign of input file corruption
                              qDebug("Measure:pasteStaff(): empty tuplet");
                              delete tuplet;
                              }
                        else {
                              Measure* measure = tick2measure(tuplet->tick());
                              tuplet->setParent(measure);
                              tuplet->sortElements();
                              }
                        }
                  }

            if (pasted) { //select only if we pasted something
                  Segment* s1 = tick2segment(dstTick);
                  Segment* s2 = tick2segment(dstTick + tickLen);
                  int endStaff = dstStaffStart + staves;
                  if (endStaff > nstaves())
                        endStaff = nstaves();
                  _selection.setRange(s1, s2, dstStaffStart, endStaff);
                  _selection.updateSelectedElements();
                  foreach(MuseScoreView* v, viewer)
                        v->adjustCanvasPosition(s1, false);
                  if (selection().state() != SEL_RANGE)
                        _selection.setState(SEL_RANGE);
                  }
            }

/*      foreach(Tuplet* t, invalidTuplets) {
            t->measure()->remove(t);
            delete t;
            }
*/
      connectTies();
//      updateNotes();    // TODO: undoable version needed

//      layoutFlags |= LAYOUT_FIX_PITCH_VELO;
      }

//---------------------------------------------------------
//   pasteChordRest
//---------------------------------------------------------

void Score::pasteChordRest(ChordRest* cr, int tick)
      {
// qDebug("pasteChordRest %s at %d", cr->name(), tick);
      if (cr->type() == CHORD) {
            // set note track
            // check if staffMove moves a note to a
            // nonexistant staff
            //
            int track  = cr->track();
            Chord* c   = static_cast<Chord*>(cr);
            Part* part = cr->staff()->part();
            int nn     = (track / VOICES) + c->staffMove();
            if (nn < 0 || nn >= nstaves())
                  c->setStaffMove(0);
            if (!styleB(ST_concertPitch) && part->instr()->transpose().chromatic) {
                  Interval interval = part->instr()->transpose();
                  if (!interval.isZero()) {
                        interval.flip();
                        foreach(Note* n, c->notes()) {
                              int npitch;
                              int ntpc;
                              transposeInterval(n->pitch(), n->tpc(), &npitch, &ntpc, interval, true);
                              n->setPitch(npitch, ntpc);
                              }
                        }
                  }
            }

      Measure* measure = tick2measure(tick);
      bool isGrace = (cr->type() == CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL);
      int measureEnd = measure->tick() + measure->ticks();
      if (tick >= measureEnd)       // end of score
            return;

      if (!isGrace && (tick + cr->actualTicks() > measureEnd)) {
            if (cr->type() == CHORD) {
                  // split Chord
                  Chord* c = static_cast<Chord*>(cr);
                  int rest = c->actualTicks();
                  int len  = measureEnd - tick;
                  rest    -= len;
                  TDuration d;
                  d.setVal(len);
                  c->setDurationType(d);
                  c->setDuration(d.fraction());
                  undoAddCR(c, measure, tick);
                  while (rest) {
                        tick += c->actualTicks();
                        measure = tick2measure(tick);
                        if (measure->tick() != tick) {  // last measure
                              qDebug("==last measure %d != %d", measure->tick(), tick);
                              break;
                              }
                        Chord* c2 = static_cast<Chord*>(c->clone());
                        len = measure->ticks() > rest ? rest : measure->ticks();
                        TDuration d;
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
                  Rest* r       = static_cast<Rest*>(cr);
                  Fraction rest = r->duration();

                  while (!rest.isZero()) {
                        Rest* r2      = static_cast<Rest*>(r->clone());
                        measure       = tick2measure(tick);
                        Fraction mlen = Fraction::fromTicks(measure->tick() + measure->ticks() - tick);
                        Fraction len  = rest > mlen ? mlen : rest;
                        r2->setDuration(len);
                        r2->setDurationType(TDuration(len));
                        undoAddCR(r2, measure, tick);
                        rest -= len;
                        tick += r2->actualTicks();
                        }
                  delete r;
                  }
            }
      else {
            undoAddCR(cr, measure, tick);
            }
      }

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void Score::moveInputPos(Segment* s)
      {
      if (s == 0)
            return;
      _is.setSegment(s);
//      emit posChanged(s->tick());
#if 0
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
      ChordRest* cr;
      if (selection().activeCR())
            cr = selection().activeCR();
      else
            cr = selection().lastChordRest();
      if (cr == 0 && inputState().noteEntryMode)
            cr = inputState().cr();
      if (cr == 0)
            return 0;

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
                        _playNote = true;
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
            _playNote = true;
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
      if (cr == 0 && inputState().noteEntryMode)
            cr = inputState().cr();
      if (cr == 0)
            return 0;

      ChordRest* el = 0;
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
      return el;
      }

//---------------------------------------------------------
//   cmdMirrorNoteHead
//---------------------------------------------------------

void Score::cmdMirrorNoteHead()
      {
      const QList<Element*>& el = selection().elements();
      foreach(Element* e, el) {
            if (e->type() == NOTE) {
                  Note* note = static_cast<Note*>(e);
                  if (note->staff() && note->staff()->useTablature())
                        note->score()->undoChangeProperty(e, P_GHOST, true);
                  else {
                        DirectionH d = note->userMirror();
                        if (d == DH_AUTO)
                              d = note->chord()->up() ? DH_RIGHT : DH_LEFT;
                        else
                              d = d == DH_LEFT ? DH_RIGHT : DH_LEFT;
                        undoChangeUserMirror(note, d);
                        }
                  }
            }
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
      TDuration d = _is.duration().shift(1);
      if (!d.isValid() || (d.type() > TDuration::V_64TH))
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
      TDuration d = _is.duration().shift(-1);
      if (!d.isValid() || (d.type() < TDuration::V_WHOLE))
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
      nextInputPos(cr, false);
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
      setLayoutAll(false);
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

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void Score::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
      if (MScore::debugMode)
            qDebug("Score::cmd <%s>", qPrintable(cmd));

      //
      // Hack for moving articulations while selected
      //
      Element* el = selection().element();
      if (cmd == "pitch-up") {
            if (el && (el->type() == ARTICULATION || el->type() == FINGERING))
                  undoMove(el, el->userOff() + QPointF(0.0, -MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == REST)
                  cmdMoveRest(static_cast<Rest*>(el), UP);
            else if (el && el->type() == LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), UP);
            else
                  upDown(true, UP_DOWN_CHROMATIC);
            }
      else if (cmd == "pitch-down") {
            if (el && (el->type() == ARTICULATION || el->type() == FINGERING))
                  undoMove(el, el->userOff() + QPointF(0.0, MScore::nudgeStep * el->spatium()));
            else if (el && el->type() == REST)
                  cmdMoveRest(static_cast<Rest*>(el), DOWN);
            else if (el && el->type() == LYRICS)
                  cmdMoveLyrics(static_cast<Lyrics*>(el), DOWN);
            else
                  upDown(false, UP_DOWN_CHROMATIC);
            }
	else if (cmd == "add-staccato")
            addArticulation(Articulation_Staccato);
	else if (cmd == "add-tenuto")
            addArticulation(Articulation_Tenuto);
  else if (cmd == "add-marcato")
            addArticulation(Articulation_Marcato);            
	else if (cmd == "add-trill")
            addArticulation(Articulation_Trill);
      else if (cmd == "add-hairpin")
            cmdAddHairpin(false);
      else if (cmd == "add-hairpin-reverse")
            cmdAddHairpin(true);
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
                              _playNote = true;
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
                              _playNote = true;
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
                              _playNote = true;
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
                              _playNote = true;
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
      else if (cmd == "pad-note-128")
            padToggle(PAD_NOTE128);
      else if (cmd == "pad-rest")
            padToggle(PAD_REST);
      else if (cmd == "pad-dot")
            padToggle(PAD_DOT);
      else if (cmd == "pad-dotdot")
            padToggle(PAD_DOTDOT);
      else if (cmd == "beam-start")
            cmdSetBeamMode(BEAM_BEGIN);
      else if (cmd == "beam-mid")
            cmdSetBeamMode(BEAM_MID);
      else if (cmd == "no-beam")
            cmdSetBeamMode(BEAM_NO);
      else if (cmd == "beam-32")
            cmdSetBeamMode(BEAM_BEGIN32);
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
      else if (cmd == "repitch")
            _is.setRepitchMode(a->isChecked());
      else if (cmd == "flip")
            cmdFlip();
      else if (cmd == "stretch+")
            cmdAddStretch(0.1);
      else if (cmd == "stretch-")
            cmdAddStretch(-0.1);
      else if (cmd == "pitch-spell")
            spell();
      else if (cmd == "select-all")
            cmdSelectAll();
      else if (cmd == "select-section")
            cmdSelectSection();
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
            LayoutBreakType type;
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
                              if (e->type() == LAYOUT_BREAK && static_cast<LayoutBreak*>(e)->subtype() ==type) {
                                    undoRemoveElement(e);
                                    break;
                                    }
                              }
                        }
                  }
            }
      else if (cmd == "reset-stretch")
            resetUserStretch();
      else if (cmd == "mirror-note")
            cmdMirrorNoteHead();
      else if (cmd == "double-duration")
            cmdDoubleDuration();
      else if (cmd == "half-duration")
            cmdHalfDuration();
      else if (cmd == "") {               //Midi note received only?
            if (!noteEntryMode())
                  setLayoutAll(false);
            }
      else if (cmd == "add-audio")
            addAudioTrack();
      else if (cmd == "transpose-up")
            transposeSemitone(1);
      else if (cmd == "transpose-down")
            transposeSemitone(-1);
      else
            qDebug("1unknown cmd <%s>", qPrintable(cmd));
      }

