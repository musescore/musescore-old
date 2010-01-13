//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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
#include "segment.h"
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

//---------------------------------------------------------
//   startCmd
//---------------------------------------------------------

/**
 Start a GUI command by clearing the redraw area
 and starting a user-visble undo.
*/

void Score::startCmd()
      {
      layoutAll = true;      ///< do a complete relayout

      // Start collecting low-level undo operations for a
      // user-visible undo action.

      if (_undo->active()) {
            // if (debugMode)
            fprintf(stderr, "Score::startCmd(): cmd already active\n");
            return;
            }
      _undo->beginMacro();
      _undo->push(new SaveState(this));
      }

//---------------------------------------------------------
//   endCmd
//---------------------------------------------------------

/**
 End a GUI command by (if \a undo) ending a user-visble undo
 and (always) updating the redraw area.
*/

void Score::endCmd()
      {
      if (!_undo->active()) {
            // if (debugMode)
                  fprintf(stderr, "Score::endCmd(): no cmd active\n");
            end();
            return;
            }
      _undo->endMacro(_undo->current()->childCount() <= 1);
      foreach(Element* e, selection()->elements())
            e->setSelected(true);
      end();
      }

//---------------------------------------------------------
//   end
//---------------------------------------------------------

/**
 Update the redraw area.
*/
void Score::end()
      {
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
      refresh     = QRectF();
      layoutAll   = false;
      _updateAll  = false;
      startLayout = 0;
      if (!noteEntryMode())
            setPadState();
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd(Element* e)
      {
      e->setSelected(false);
      undoAddElement(e);
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAdd
//---------------------------------------------------------

void Score::cmdAdd1(Element* e, const QPointF& pos, const QPointF& dragOffset)
      {
      e->setSelected(false);
      int staffIdx = -1;
      int pitch, tick;
      QPointF offset;
      Segment* segment;
      MeasureBase* mb = pos2measure(pos, &tick, &staffIdx, &pitch, &segment, &offset);
      if (mb == 0 || mb->type() != MEASURE) {
            printf("cmdAdd: cannot put object here\n");
            delete e;
            return;
            }

      int track = staffIdx == -1 ? -1 : staffIdx * VOICES;
      Measure* measure = (Measure*)mb;
      e->setTrack(track);
      e->setParent(0);

      // calculate suitable endposition
      int tick2 = measure->last()->tick();
      MeasureBase* m2 = measure;
      while (tick2 <= tick) {
            m2 = m2->next();
            if (m2 == 0)
                  break;
            if (m2->type() != MEASURE)
                  continue;
            tick2 = m2->tick();
            }

      switch(e->type()) {
            case VOLTA:
                  {
                  Volta* volta = static_cast<Volta*>(e);
                  int tick;
                  Measure* m = pos2measure3(pos, &tick);
                  volta->setTick(m->tick());
                  volta->setTick2(m->tick() + m->tickLen());
                  volta->layout();
                  const QList<LineSegment*> lsl = volta->lineSegments();
                  if (lsl.isEmpty()) {
                        delete e;
                        return;
                        }
                  else {
                        LineSegment* ls = lsl.front();
                        ls->setScore(this);
                        QPointF uo(pos - ls->canvasPos() - dragOffset);
                        ls->setUserOff(uo);
                        }
                  }
                  break;

            case PEDAL:
            case OTTAVA:
            case TRILL:
            case HAIRPIN:
            case TEXTLINE:
                  {
                  SLine* line = static_cast<SLine*>(e);
                  if (e->type() == TEXTLINE) {
                        TextLine* tl = static_cast<TextLine*>(e);
                        if (!tl->beginText()) {
                              Segment* seg = tick2segment(tick);
                              if (seg && seg->nextCR())
                                    tick2 = seg->nextCR()->tick();
                              }
                        }
                  line->setTick(tick);
                  line->setTick2(tick2);
                  line->layout();
                  LineSegment* ls = line->lineSegments().front();
                  ls->setScore(this);
                  QPointF uo(pos - ls->canvasPos() - dragOffset);
                  ls->setUserOff(uo);
                  }
                  break;

            case DYNAMIC:
                  {
                  Dynamic* dyn = static_cast<Dynamic*>(e);
                  dyn->setTick(tick);
                  dyn->setParent(measure);
                  dyn->layout();

                  double xx = measure->tick2pos(tick);
                  QPointF uo(pos - measure->canvasPos() - QPointF(xx, 0.0) - dragOffset);
                  uo -= QPointF(0.0, dyn->ipos().y());
                  dyn->setUserOff(uo);
                  }
                  break;
            default:
                  return;
            }
      cmdAdd(e);
      select(e, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   cmdRemoveClef
//---------------------------------------------------------

void Score::cmdRemoveClef(Clef* clef)
      {
      Staff* staff  = clef->staff();
      ClefList* cl  = staff->clefList();
      int tick      = clef->tick();
      iClefEvent ki = cl->find(tick);
      if (ki == cl->end()) {
            printf("cmdRemove(Clef): cannot find clef at %d\n", tick);
            return;
            }
      int oval = (*cl)[tick];
      iClefEvent nki = ki;
      ++nki;

      undoChangeClef(staff, tick, oval, NO_CLEF);
      undoRemoveElement(clef);
      Segment* segment = clef->segment();
      segment->measure()->cmdRemoveEmptySegment(segment);

      oval = cl->clef(tick);
      if (nki->second != oval)
            return;

      undoChangeClef(staff, nki->first, oval, NO_CLEF);

      int track = clef->track();
      for (segment = segment->next1(); segment; segment = segment->next1()) {
            if (segment->subtype() != Segment::SegClef)
                  continue;
            //
            // we assume clefs are only in first track (voice 0)
            //
            Clef* e = static_cast<Clef*>(segment->element(track));
            if (e) {
                  int cst = e->subtype();
                  if (cst == oval) {
                        undoRemoveElement(e);
                        e->measure()->cmdRemoveEmptySegment(segment);
                        }
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemoveKeySig
//---------------------------------------------------------

void Score::cmdRemoveKeySig(KeySig* ks)
      {
      Staff* staff = ks->staff();
      KeyList* kl  = staff->keymap();
      int tick     = ks->tick();
      iKeyList ki = kl->find(tick);
      if (ki == kl->end()) {
            printf("cmdRemove(KeySig): cannot find keysig at %d\n", tick);
            return;
            }
      KeySigEvent oval = ki->second;
      iKeyList nki = ki;
      ++nki;

      undoChangeKey(staff, tick, oval, KeySigEvent());

      undoRemoveElement(ks);
      Segment* segment = ks->segment()->next1();
      ks->measure()->cmdRemoveEmptySegment(ks->segment());

      oval = kl->key(tick);
      if ((nki != kl->end()) && (nki->second == oval))
            undoChangeKey(staff, nki->first, oval, KeySigEvent());

      int track = ks->track();
      for (; segment; segment = segment->next1()) {
            if (segment->subtype() != Segment::SegKeySig)
                  continue;
            KeySig* e = static_cast<KeySig*>(segment->element(track));
            if (e) {
                  KeySigEvent cst = e->keySigEvent();
                  if (cst == oval) {
                        // remove redundant key signature
                        undoRemoveElement(e);
                        segment->measure()->cmdRemoveEmptySegment(segment);
                        }
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   cmdRemove
//---------------------------------------------------------

void Score::cmdRemove(Element* e)
      {
      switch(e->type()) {
            case CLEF:
                  cmdRemoveClef(static_cast<Clef*>(e));
                  break;
            case KEYSIG:
                  cmdRemoveKeySig(static_cast<KeySig*>(e));
                  break;
            case TIMESIG:
                  cmdRemoveTimeSig(static_cast<TimeSig*>(e));
                  break;
            case TEMPO_TEXT:
                  {
                  int tick = e->tick();
                  AL::iTEvent i = _tempomap->find(tick);
                  if (i != _tempomap->end())
                        undoChangeTempo(tick, i->second, AL::TEvent());
                  else
                        printf("remove tempotext: tempo event at %d not found\n", tick);
                  undoRemoveElement(e);
                  }
                  break;

            default:
                  {
                  Segment* seg = 0;
                  if (e->parent()->type() == SEGMENT)
                        seg = (Segment*) e->parent();
                  undoRemoveElement(e);
                  if (seg && seg->isEmpty())
                        undoRemoveElement(seg);
                  if (e->type() == DYNAMIC)
                        fixPpitch();      // recalculate all velocities
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   cmdAddPitch
//    c d e f g a b entered:
//       insert note or add note to chord
//---------------------------------------------------------

void ScoreView::cmdAddPitch(int note, bool addFlag)
      {
      InputState& is = _score->inputState();

      _score->startCmd();
      _score->expandVoice();
      if (is.cr() == 0) {
            printf("cannot enter notes here (no chord rest at current position)\n");
            return;
            }
      if (!noteEntryMode())
            sm->postEvent(new CommandEvent("note-input"));

      KeySigEvent key;

      if (!preferences.alternateNoteEntryMethod)
            key = _score->staff(is.track / VOICES)->keymap()->key(is.tick());
      int pitch;
      Drumset* ds = is.drumset;
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
            }
      else {
            int octave = is.pitch / 12;
            pitch      = pitchKeyAdjust(note, key.accidentalType);
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
      _score->cmdAddPitch1(pitch, addFlag);
      moveCursor();
      ChordRest* cr = _score->inputState().cr();
      if (cr) {
            Element* e = cr;
            if (cr->type() == CHORD)
                  e = static_cast<Chord*>(cr)->upNote();
            adjustCanvasPosition(e, false);
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   expandVoice
//---------------------------------------------------------

void Score::expandVoice()
      {
      if (_is.voice() && (_is.cr() == 0)) {
            //
            // if there is no chord/rest at current position for voice > 0
            // then there is no chord/rest for this voice at all in this measure
            //
            addRest(_is._segment, _is.track, Duration(Duration::V_MEASURE), 0);
            }
      }

//---------------------------------------------------------
//   cmdAddPitch1
//---------------------------------------------------------

Note* Score::cmdAddPitch1(int pitch, bool addFlag)
      {
      if (addFlag) {
            // add note to chord
            Note* on = getSelectedNote();
            if (on == 0)
                  return 0;
            Note* n = addNote(on->chord(), pitch);
            select(n, SELECT_SINGLE, 0);
            setLayoutAll(false);
            setLayout(on->chord()->measure());
            moveToNextInputPos();
            return n;
            }
      expandVoice();

      // insert note
      Direction stemDirection = AUTO;
      int headGroup           = 0;
      int track               = _is.track;
      if (_is.drumNote != -1) {
            int pitch     = _is.drumNote;
            Drumset* ds   = _is.drumset;
            headGroup     = ds->noteHead(pitch);
            stemDirection = ds->stemDirection(pitch);
            track         = ds->voice(pitch) + (_is.track / VOICES) * VOICES;
            }

      Segment* seg = setNoteRest(_is.cr(), track, pitch, _is.duration().fraction(), headGroup, stemDirection);
      Note* note = static_cast<Chord*>(seg->element(track))->upNote();
      setLayout(note->chord()->measure());

      if (_is.slur) {
            //
            // extend slur
            //
            Element* e = searchNote(_is.tick(), _is.track);
            if (e) {
                  if (e->type() == NOTE)
                        e = e->parent();
                  if (_is.slur->startElement()->tick() == e->tick()) {
                        if (_is.slur->startElement())
                              static_cast<ChordRest*>(_is.slur->startElement())->removeSlurFor(_is.slur);
                        _is.slur->setStartElement(e);
                        static_cast<ChordRest*>(e)->addSlurFor(_is.slur);
                        }
                  else
                        _is.slur->setEndElement(e);
                  }
            else
                  printf("cmdAddPitch1: cannot find slur note\n");
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
#if 0
            Staff* staff = on->staff();

            KeySigEvent key = staff->keymap()->key(on->chord()->tick());
            int kt[15] = {
                  //  cb gb db ab  eb bb  f  c  g  d  a  e   b  f# c#
                  // -7  -6 -5 -4 -3  -2 -1  0  1  2  3  4   5  6  7
                     11,  6, 1, 8, 3, 10, 5, 0, 7, 2, 9, 4, 11, 6, 1
                  };

            int po = 12 - kt[key.accidentalType + 7];

            static int pt[12][16] = {
                  //   2   3   4  5   6    7  OK   9   -2  -3  -4, -5, -6, -7,   OK   -9

            /*C */   { 2,  4,  5, 7,  9,  11, 12, 14,  -1, -3, -5, -7, -8, -10, -12, -13 },
            /*C#*/   { 1,  3,  4, 6,  8,  10, 12, 13,  -2, -4, -6, -8, -9, -11, -12, -14 },
            /*D */   { 2,  3,  5, 7,  9,  10, 12, 14,  -2, -3, -5, -7, -9, -10, -12, -14 },
            /*D#*/   { 1,  2,  4, 6,  8,   9, 12, 13,  -1, -3, -4, -6, -8, -10, -12, -13 },
            /*E */   { 1,  3,  5, 7,  8,  10, 12, 13,  -2, -4, -5, -7, -9, -11, -12, -14 },
            /*F */   { 2,  4,  6, 7,  9,  11, 12, 14,  -1, -3, -5, -6, -8, -10, -12, -13 },
            /*F#*/   { 1,  3,  5, 6,  8,  10, 12, 13,  -1, -2, -4, -6, -7,  -9, -12, -13 },
            /*G */   { 2,  4,  5, 7,  9,  10, 12, 14,  -2, -3, -5, -7, -8, -10, -12, -14 },
            /*G#*/   { 1,  3,  4, 6,  8,   9, 12, 13,  -1, -3, -4, -6, -8,  -9, -12, -13 },
            /*A */   { 2,  3,  5, 7,  8,  10, 12, 14,  -2, -4, -5, -7, -9, -10, -12, -14 },
            /*A#*/   { 1,  2,  4, 6,  7,   9, 11, 13,  -1, -3, -5, -6, -8, -10, -12, -13 },
            /*B */   { 1,  3,  5, 6,  8,  10, 12, 13,  -2, -4, -6, -7, -9, -11, -12, -14 },
                  };

            static int it[19] = {
               // -9, -8, -7, -6, -5, -4, -3, -2, -1,  0,  1, 2, 3, 4, 5, 6, 7, 8, 9
                  15, 14, 13, 12, 11, 10,  9,  8, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7
                  };

            int pitch = on->pitch();
            int idx = it[val + 9];
            int interval = 0;
            if (idx != -1)
                  interval = pt[(pitch+po) % 12][idx];

            pitch += interval;
            if (pitch > 127)
                  pitch = 127;
            if (pitch < 0)
                  pitch = 0;
#endif
            static int itable[] = {
             //   0,  1, 2, 3,  4,  5,  6,  7,  8,  9
                  0,  0, 4, 8, 11, 14, 18, 22, 25,  4
                  };

            int interval = itable[qAbs(val)];
printf("val %d -> interval %d\n", val, interval);

printf("note %d %d\n", on->pitch(), on->tpc());
            Note* note = new Note(*on);
            note->setParent(on->chord());
printf("note %d %d\n", note->pitch(), note->tpc());
            int npitch, ntpc;
            transposeInterval(note->pitch(), note->tpc(), &npitch, &ntpc, interval, val > 0 ? TRANSPOSE_UP : TRANSPOSE_DOWN);
            note->setPitch(npitch, ntpc);

printf("  note %d %d\n", note->pitch(), note->tpc());
            if (val > 8)
                  note->setPitch(note->pitch() + 12, note->tpc());
            else if (val < -8)
                  note->setPitch(note->pitch() - 12, note->tpc());

            cmdAdd(note);
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

void Score::setGraceNote(Chord* chord, int pitch, NoteType type, int len)
      {
      Segment* seg     = chord->segment();
      Measure* measure = seg->measure();
      int tick         = chord->tick();
      int track        = chord->track();

      Segment::SegmentType st = Segment::SegGrace;
      Segment* s = seg->prev();
      while (s && s->subtype() == st && s->element(track))
            s = s->prev();
      if (s && (s->subtype() == st) && (!s->element(track)))
            seg = s;
      else {
            seg = measure->createSegment(st, tick);
            undoAddElement(seg);
            }
      double mag = staff(track/VOICES)->mag() * styleD(ST_graceNoteMag);

      Note* note = new Note(this);
      note->setTrack(track);
      note->setMag(mag);

      chord = new Chord(this);
      chord->setTick(tick);
      chord->setTrack(track);
      chord->add(note);
      note->setPitch(pitch);

      Duration d;
      d.setVal(len);
      chord->setDuration(d);

      chord->setStemDirection(UP);
      chord->setNoteType(type);
      chord->setParent(seg);
      chord->setMag(mag);

      undoAddElement(chord);
      select(note, SELECT_SINGLE, 0);
      }

//---------------------------------------------------------
//   setNoteRest
//    pitch == -1  -> set rest
//    return segment of last created note/rest
//---------------------------------------------------------

Segment* Score::setNoteRest(ChordRest* cr, int track, int pitch, Fraction sd,
   int headGroup, Direction stemDirection)
      {
      int tick = cr->tick();
      Element* nr   = 0;
      Tie* tie      = 0;

      Segment* seg = 0;
      Measure* measure = 0;
      while (true) {
            // the returned gap ends at the measure boundary or at tuplet end
            Fraction dd = makeGap(cr, sd, cr->tuplet());

            if (dd.isZero()) {
                  printf("cannot get gap at %d type: %d/%d\n", tick, sd.numerator(),
                     sd.denominator());
                  break;
                  }
            QList<Duration> dl = toDurationList(dd, true);

            int n = dl.size();
            for (int i = 0; i < n; ++i) {
                  Duration d = dl[i];

                  ChordRest* ncr;
                  if (pitch == -1) {
                        nr = new Rest(this);
                        nr->setTrack(track);
                        ncr = (Rest*)nr;
                        ncr->setDuration(d);
                        }
                  else {
                        Note* note = new Note(this);
                        nr = note;
                        note->setTrack(track);
                        note->setHeadGroup(headGroup);

                        if (tie) {
                              tie->setEndNote(note);
                              note->setTieBack(tie);
                              }
                        Chord* chord = new Chord(this);
                        chord->setTick(tick);
                        chord->setTrack(track);
                        chord->setDuration(d);
                        chord->setTuplet(cr->tuplet());
                        chord->setStemDirection(stemDirection);
                        chord->add(note);
                        note->setPitch(pitch);
                        mscore->play(note);
                        ncr = chord;
                        if (i+1 < n) {
                              tie = new Tie(this);
                              tie->setStartNote((Note*)nr);
                              tie->setTrack(nr->track());
                              note->setTieFor(tie);
                              }
                        }
                  measure = tick2measure(tick);
                  Segment::SegmentType st = Segment::SegChordRest;
                  seg = measure->findSegment(st, tick);
                  if (seg == 0) {
                        seg = measure->createSegment(st, tick);
                        undoAddElement(seg);
                        }
                  ncr->setParent(seg);
                  undoAddElement(ncr);
                  tick += ncr->ticks();
                  }

            sd -= dd;
            if (sd.isZero())
                  break;

            measure = measure->nextMeasure();
            if (measure == 0) {
                  printf("reached end of score\n");
                  break;
                  }
            tick = measure->tick();
            seg  = measure->firstCRSegment();
            cr   = static_cast<ChordRest*>(seg->element(track));
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
            if (pitch != -1) {
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
printf("makeGap %d/%d at %d\n", _sd.numerator(), _sd.denominator(), cr->tick());
      int track = cr->track();
      Measure* measure = cr->measure();
      setLayout(measure);
      Fraction akkumulated;
      Fraction sd = _sd;

      for (Segment* seg = cr->segment(); seg; seg = seg->next()) {
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
                        printf("makeGap: end of tuplet reached\n");
                        break;
                        }
                  }
            Fraction td(cr->fraction());

            Tuplet* ltuplet = cr->tuplet();
            if (cr->tuplet() != tuplet) {
                  printf("   remove tuplet %d\n", sd >= ltuplet->fraction());
                  //
                  // Current location points to the start of a (nested)tuplet.
                  // We have to remove the complete tuplet.

                  Tuplet* t = ltuplet;
                  while (t->elements().last()->type() == TUPLET)
                        t = static_cast<Tuplet*>(t->elements().last());
                  seg = static_cast<ChordRest*>(t->elements().last())->segment();

                  td = ltuplet->fraction();
                  cmdDeleteTuplet(ltuplet, false);
                  tuplet = 0;
                  }
            else {
                  printf("  makeGap: remove %d/%d at %d\n", td.numerator(), td.denominator(), cr->tick());
                  undoRemoveElement(cr);
                  if (seg->isEmpty())
                        undoRemoveElement(seg);
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
printf("   gap ticks %d+%d\n", cr->tick(), ticks);
                  for (Tuplet* t = tuplet; t; t = t->tuplet())
                        ticks = ticks * t->ratio().denominator() / t->ratio().numerator();
                  int tick = cr->tick() + ticks;

                  if ((tuplet == 0) && (((measure->tick() - tick) % dList[0].ticks()) == 0)) {
                        foreach(Duration d, dList) {
                              printf("   addClone %d\n", tick);
                              tick += addClone(cr, tick, d)->ticks();
                              }
                        }
                  else {
                        for (int i = dList.size() - 1; i >= 0; --i)
                              tick += addClone(cr, tick, dList[i])->ticks();
                        }
printf("  return %d/%d\n", akkumulated.numerator(), akkumulated.denominator());
                  return akkumulated;
                  }
            akkumulated += td;
printf("  akkumulated %d/%d\n", akkumulated.numerator(), akkumulated.denominator());
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

Fraction Score::makeGap1(ChordRest* cr, Fraction len)
      {
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
                        return gap;
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
      return gap;
      }

//---------------------------------------------------------
//   changeCRlen
//---------------------------------------------------------

void Score::changeCRlen(ChordRest* cr, const Duration& d)
      {
      Fraction srcF = cr->fraction();
      Fraction dstF;
      if (d.type() == Duration::V_MEASURE)
            dstF = cr->measure()->fraction();
      else
            dstF = d.fraction();

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
                  NoteList* nl = c->noteList();
                  for (iNote i = nl->begin(); i != nl->end(); ++i) {
                        Note* n = i->second;
                        if (n->tieFor())
                              undoRemoveElement(n->tieFor());
                        }
                  }
            undoChangeChordRestLen(cr, d);
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

      if (tuplet && tuplet->fraction() < dstF) {
            printf("does not fit in tuplet\n");
            return;
            }

      while (f > Fraction(0)) {
            while (s && ((s->element(track) == 0) || (s->subtype() != Segment::SegChordRest)))
                  s = s->next1();
            if (s == 0)
                  break;
            if ((f1 > Fraction(0)) && (s->tick() == s->measure()->tick())) {
                  flist.append(f1);
                  f1 = Fraction(0);
                  }
            ChordRest* cr = static_cast<ChordRest*>(s->element(track));
            Duration d(cr->duration());
            Fraction f2 = (d.type() == Duration::V_MEASURE) ? cr->measure()->fraction() : d.fraction();
            if (f2 > f)
                  f2 = f;
            f1 += f2;
            f  -= f2;
            s = s->next1();
            }
      if (f1 > Fraction(0))
            flist.append(f1);

printf("List:\n");
      foreach (Fraction f, flist)
            printf("  %d/%d\n", f.numerator(), f.denominator());

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
printf("   sublist:\n");
      foreach (Duration d, dList)
            printf("      %d/%d\n", d.fraction().numerator(), d.fraction().denominator());

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
      Measure* measure = cr->measure();
      Harmony* s = new Harmony(_score);
      s->setTrack(cr->track());
      s->setParent(measure);
      s->setTick(cr->tick());
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
      Measure* measure = cr->measure();
      Harmony* s = 0;

      foreach(Element* element, *measure->el()) {
            if ((element->type() == HARMONY) && (element->tick() == cr->tick())) {
                  s = static_cast<Harmony*>(element);
                  break;
                  }
            }
      bool created = false;
      if (s == 0) {
            s = new Harmony(this);
            s->setTrack(cr->track());
            s->setParent(measure);
            s->setTick(cr->tick());
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
      TextB* s = 0;
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
            case TEXT_COPYRIGHT:
                  s = new TextC(_score);
                  s->setParent(page);
                  s->setTextStyle(TEXT_STYLE_COPYRIGHT);
                  s->setSubtype(subtype);
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
                  s->setParent(cr->measure());
                  s->setTick(cr->tick());
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
                        }
                  else {
                        s->setTrack(cr->track());
                        s->setSystemFlag(false);
                        s->setTextStyle(TEXT_STYLE_STAFF);
                        }
                  s->setSubtype(subtype);
                  s->setParent(cr->measure());
                  s->setTick(cr->tick());
                  }
                  break;
            }

      if (s) {
printf("insert TEXT\n");
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
//---------------------------------------------------------

/**
 Increment/decrement pitch of note by one or by an octave.
*/

void Score::upDown(bool up, bool octave)
      {
      layoutAll   = false;
      startLayout = 0;        // DEBUG
      ElementList el;

      QList<Note*> nl = selection()->noteList();

      int tick = -1;
      bool playNotes = true;
      foreach(Note* note, nl) {
            if (startLayout == 0)
                  startLayout = note->chord()->segment()->measure();
            else if (startLayout != note->chord()->segment()->measure())
                  layoutAll = true;
            for (; note; note = note->tieFor() ? note->tieFor()->endNote() : 0) {
                  iElement ii;
                  for (ii = el.begin(); ii != el.end(); ++ii) {
                        if (*ii == note)
                              break;
                        }
                  if (ii == el.end()) {
                        el.push_back(note);
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

      for (iElement i = el.begin(); i != el.end(); ++i) {
            Note* oNote = (Note*)(*i);
            Part* part  = oNote->staff()->part();
            int pitch   = oNote->pitch();
            int newTpc;
            int newPitch;
            if (part->useDrumset()) {
                  Drumset* ds = part->drumset();
                  newPitch    = up ? ds->prevPitch(pitch) : ds->nextPitch(pitch);
                  newTpc      = oNote->tpc();
                  }
            else {
                  if (octave)  {
                        newPitch = pitch + (up ? 12 : -12);
                        newTpc   = oNote->tpc();
                        }
                  else {
                        newPitch = up ? pitch+1 : pitch-1;
                        newTpc   = pitch2tpc2(newPitch, up);
                        }
                  }
            if (newPitch < 0) {
                  newPitch = 0;
                  newTpc   = pitch2tpc(newPitch);
                  }
            else if (newPitch > 127) {
                  newPitch = 127;
                  newTpc   = pitch2tpc(newPitch);
                  }
            _is.pitch = newPitch;

            undoChangePitch(oNote, newPitch, newTpc, 0);

            // play new note with velocity 80 for 0.3 sec:
            if (playNotes)
                  mscore->play(oNote);
            }
      selection()->updateState();     // accidentals may have changed
      }

//---------------------------------------------------------
//   cmdAppendMeasures
//    - keyboard callback
//    - called from pulldown menu
//---------------------------------------------------------

/**
 Append \a n measures.

 Keyboard callback, called from pulldown menu.
*/

void Score::cmdAppendMeasures(int n)
      {
      startCmd();
      appendMeasures(n, MEASURE);
      endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* Score::appendMeasure(int type)
      {
      MeasureBase* last = _measures.last();
      int tick = 0;
      if (last) {
            tick = last->tick();
            if (last->type() == MEASURE)
                  tick += static_cast<Measure*>(last)->tickLen();
            }
      MeasureBase* mb = 0;
      if (type == MEASURE)
            mb = new Measure(this);
      else if (type == HBOX)
            mb = new HBox(this);
      else if (type == VBOX)
            mb = new VBox(this);
      mb->setTick(tick);
      if (!last)
        _sigmap->add(tick, Fraction(4, 4));

      if (type == MEASURE) {
            Measure* measure = (Measure*)mb;
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  Rest* rest = new Rest(this, tick, Duration(Duration::V_MEASURE));
                  rest->setTrack(staffIdx * VOICES);
                  Segment* s = measure->getSegment(rest);
                  s->add(rest);
                  }
            }
      undoInsertMeasure(mb);

      if (type == MEASURE)
            undoFixTicks();
      layoutAll = true;
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void Score::appendMeasures(int n, int type)
      {
      if (noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      bool createEndBar = false;
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
// Added from cmdAppendMeasures by DK 06.08.07
//---------------------------------------------------------

void Score::cmdInsertMeasures(int n)
      {
      startCmd();
      insertMeasures(n, MEASURE);
      endCmd();
      }

//---------------------------------------------------------
//   insertMeasures
//  Changed by DK 05.08.07, loop for inserting "n"
//    Measures to be added
//    (loopcounter ino -- insert numbers)
//---------------------------------------------------------

void Score::insertMeasures(int n, int type)
      {
	if (selection()->state() != SEL_STAFF && selection()->state() != SEL_SYSTEM) {
		QMessageBox::warning(0, "MuseScore",
			tr("No Measure selected:\n"
			"please select a measure and try again"));
		return;
            }

	int tick  = selection()->startSegment()->tick();
	int ticks = _sigmap->ticksMeasure(tick == 0 ? 0 : tick-1);

	for (int i = 0; i < n; ++i) {
            MeasureBase* m;
            if (type == MEASURE)
                  m = new Measure(this);
            else if (type == HBOX)
                  m = new HBox(this);
            else        // if (type == VBOX)
                  m = new VBox(this);
		m->setTick(tick);
            if (type == MEASURE) {
      	      Measure* measure = static_cast<Measure*>(m);
	      	for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
    		            Rest* rest = new Rest(this, tick, Duration(Duration::V_MEASURE));
        	      	rest->setTrack(staffIdx * VOICES);
        		      Segment* s = measure->getSegment(rest);
        			s->add(rest);
		            }
              	undoFixTicks();

                  QList<TimeSig*> tsl;
                  QList<KeySig*>  ksl;

                  if (tick == 0) {
                        //
                        // remove time and key signatures
                        //
                        for (Segment* s = firstMeasure()->first(); s; s = s->next()) {
                              if (s->subtype() == Segment::SegKeySig) {
                                    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                                          KeySig* e = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                                          if (e && !e->generated()) {
                                                ksl.append(static_cast<KeySig*>(e));
                                                undoRemoveElement(e);
                                                if (e->segment()->isEmpty()) {
                                                      undoRemoveElement(e->segment());
                                                      }
                                                }
                                          }
                                    }
                              else if (s->subtype() == Segment::SegTimeSig) {
                                    for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                                          TimeSig* e = static_cast<TimeSig*>(s->element(staffIdx * VOICES));
                                          if (e && !e->generated()) {
                                                tsl.append(static_cast<TimeSig*>(e));
                                                undoRemoveElement(e);
                                                if (e->segment()->isEmpty()) {
                                                      undoRemoveElement(e->segment());
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  undoInsertMeasure(measure);
                  undoInsertTime(tick, ticks);
                  undoSigInsertTime(tick, ticks);
                  undoFixTicks();

                  //
                  // if measure is inserted at tick zero,
                  // create key and time signature
                  //
                  foreach(TimeSig* ts, tsl) {
                        TimeSig* nts = new TimeSig(*ts);
                        Segment::SegmentType st = Segment::SegTimeSig;
                        Segment* s = measure->findSegment(st, 0);
                        if (s == 0) {
                              s = measure->createSegment(st, 0);
                              undoAddElement(s);
                              }
                        nts->setParent(s);
                        undoAddElement(nts);
                        }
                  foreach(KeySig* ks, ksl) {
                        KeySig* nks = new KeySig(*ks);
                        Segment::SegmentType st = Segment::SegKeySig;
                        Segment* s = measure->findSegment(st, 0);
                        if (s == 0) {
                              s = measure->createSegment(st, 0);
                              undoAddElement(s);
                              }
                        nks->setParent(s);
                        undoAddElement(nks);
                        }
                  }
            else
                  undoInsertMeasure(m);
            }
      select(0, SELECT_SINGLE, 0);
      layoutAll = true;
      }

//---------------------------------------------------------
//   addArticulation
//---------------------------------------------------------

/**
 Add attribute \a attr to all selected notes/rests.

 Called from padToggle() to add note prefix/accent.
*/

void Score::addArticulation(int attr)
      {
      foreach(Element* el, selection()->elements()) {
            if (el->type() == NOTE || el->type() == CHORD) {
                  Articulation* na = new Articulation(this);
                  na->setSubtype(attr);
                  addArticulation(el, na);
                  }
            }
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to all selected notes.
*/

void Score::addAccidental(int idx)
      {
      foreach(Note* note, selection()->noteList())
            addAccidental(note, idx);
      }

//---------------------------------------------------------
//   addAccidental
//---------------------------------------------------------

/**
 Add accidental of subtype \a idx to note \a oNote.
*/

void Score::addAccidental(Note* note, int accidental)
      {
      _undo->push(new ChangeAccidental(note, accidental));

      //
      // look for note heads on the same staff line
      //
      Chord* chord     = note->chord();
      Segment* segment = chord->segment();
      int line         = note->line();
      int t1 = chord->staffIdx() * VOICES;
      int t2 = t1 + VOICES;
      for (int track = t1; track < t2; ++track) {
            Element* e = segment->element(track);
            if (!e || e->type() != CHORD)
                  continue;
            Chord* c = static_cast<Chord*>(e);
            NoteList* nl = c->noteList();
            for (iNote in = nl->begin(); in != nl->end(); ++in) {
                  Note* n = in->second;
                  if ((n != note) && (n->line() == line) && (n->accidentalType() != accidental)) {
                        _undo->push(new ChangeAccidental(n, accidental));
                        }
                  }
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
            cmdRemove(oa);
            }
      else
            cmdAdd(atr);
      }

//---------------------------------------------------------
//   resetUserOffsets
//---------------------------------------------------------

/**
 Reset user offset for all selected notes.

 Called from pulldown menu.
*/

void Score::toDefault()
      {
      foreach(Element* e, selection()->elements())
            e->toDefault();
      layoutAll = true;
      setClean(false);
      }

//---------------------------------------------------------
//   resetUserStretch
//---------------------------------------------------------

void Score::resetUserStretch()
      {
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            if (m->type() == MEASURE)
                  static_cast<Measure*>(m)->setUserStretch(1.0);
            }
      setClean(false);
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
      _undo->push(new ChangeChordStaffMove(chord, staffMove - 1));
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
      _undo->push(new ChangeChordStaffMove(chord, staffMove + 1));
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdAddStretch
//---------------------------------------------------------

void Score::cmdAddStretch(double val)
      {
      if (selection()->state() != SEL_SYSTEM && selection()->state() != SEL_STAFF)
            return;
      int startTick = selection()->startSegment()->tick();
      int endTick   = selection()->endSegment()->tick();
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            double stretch = m->userStretch();
            stretch += val;
            _undo->push(new ChangeStretch(m, stretch));
            }
      layoutAll = true;
      }

//---------------------------------------------------------
//   cmdInsertClef
//---------------------------------------------------------

void Score::cmdInsertClef(int type)
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
      if (selection()->state() != SEL_SYSTEM && selection()->state() != SEL_STAFF) {
            printf("no system or staff selected\n");
            return;
            }
      int startTick = selection()->startSegment()->tick();
      int endTick   = selection()->endSegment()->tick();
      for (MeasureBase* m = _measures.first(); m; m = m->next()) {
            if (m->type() != MEASURE)
                  continue;
            if (m->tick() < startTick)
                  continue;
            if (m->tick() >= endTick)
                  break;
            Measure* measure = (Measure*)m;
            for (Segment* seg = measure->first(); seg; seg = seg->next()) {
                  if (seg->subtype() != Segment::SegChordRest)
                        continue;
                  for (int track = 0; track < nstaves() * VOICES; ++track) {
                        ChordRest* cr = (ChordRest*)seg->element(track);
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
      else if (cmd == "repeat")
            preferences.playRepeats = !preferences.playRepeats;
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
            if (_undo->active()) {
                  printf("Score::cmd(): cmd already active\n");
                  return;
                  }
            startCmd();

            //
            // Hack for moving articulations while selected
            //
            Element* el = selection()->element();
            if (el && el->type() == ARTICULATION && cmd == "pitch-up")
                  cmdMove(el, QPointF(0.0, -.25));
            else if (el && el->type() == ARTICULATION && cmd == "pitch-down")
                  cmdMove(el, QPointF(0.0, .25));

            else if (cmd == "append-measure")
                  appendMeasures(1, MEASURE);
            else if (cmd == "insert-measure")
		      insertMeasures(1, MEASURE);
            else if (cmd == "insert-hbox")
		      insertMeasures(1, HBOX);
            else if (cmd == "insert-vbox")
		      insertMeasures(1, VBOX);
            else if (cmd == "append-hbox") {
		      MeasureBase* mb = appendMeasure(HBOX);
                  select(mb, SELECT_SINGLE, 0);
                  }
            else if (cmd == "append-vbox") {
		      MeasureBase* mb = appendMeasure(VBOX);
                  select(mb, SELECT_SINGLE, 0);
                  }
	      else if (cmd == "add-staccato")
                  addArticulation(StaccatoSym);
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
            else if (cmd == "pitch-up")
                  upDown(true, false);
            else if (cmd == "pitch-down")
                  upDown(false, false);
            else if (cmd == "pitch-up-octave")
                  upDown(true, true);
            else if (cmd == "pitch-down-octave")
                  upDown(false, true);
            else if (cmd == "move-up") {
                  setLayoutAll(false);
                  Element* el = selection()->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Note* note = static_cast<Note*>(el);
                        moveUp(note->chord());
                        }
                  }
            else if (cmd == "move-down") {
                  setLayoutAll(false);
                  Element* el = selection()->element(); // single selection
                  if (el && el->type() == NOTE) {
                        Note* note = static_cast<Note*>(el);
                        moveDown(note->chord());
                        }
                  }
            else if (cmd == "up-chord") {
                  Element* el = selection()->element(); // single selection
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
                  Element* el = selection()->element(); // single selection
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
                  Element* el = selection()->element(); // single selection
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
                  Element* el = selection()->element(); // single selection
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
                  addAccidental(3);
            else if (cmd == "sharp")
                  addAccidental(1);
            else if (cmd == "nat")
                  addAccidental(5);
            else if (cmd == "flat")
                  addAccidental(2);
            else if (cmd == "flat2")
                  addAccidental(4);
            else if (cmd == "flip")
                  cmdFlip();
            else if (cmd == "voice-1")
                  changeVoice(0);
            else if (cmd == "voice-2")
                  changeVoice(1);
            else if (cmd == "voice-3")
                  changeVoice(2);
            else if (cmd == "voice-4")
                  changeVoice(3);
            else if (cmd == "duplet")
                  cmdTuplet(2);
            else if (cmd == "triplet")
                  cmdTuplet(3);
            else if (cmd == "quadruplet")
                  cmdTuplet(4);
            else if (cmd == "quintuplet")
                  cmdTuplet(5);
            else if (cmd == "sextuplet")
                  cmdTuplet(6);
            else if (cmd == "septuplet")
                  cmdTuplet(7);
            else if (cmd == "octuplet")
                  cmdTuplet(8);
            else if (cmd == "nonuplet")
                  cmdTuplet(9);
            else if (cmd == "stretch+")
                  cmdAddStretch(0.1);
            else if (cmd == "stretch-")
                  cmdAddStretch(-0.1);
            else if (cmd == "cut") {
                  if (selection()->state() == SEL_SINGLE) {
                        QMimeData* mimeData = new QMimeData;
                        Element* el = selection()->element();
                        mimeData->setData(mimeSymbolFormat, el->mimeData(QPointF()));
                        QApplication::clipboard()->setMimeData(mimeData);
                        deleteItem(el);
                        selection()->clear();
                        }
                  }
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
                        selection()->setState(SEL_SYSTEM);
                        int tick = mb->tick();
                        if (mb->type() == MEASURE)
                              tick += static_cast<Measure*>(mb)->tickLen();
                        selection()->setRange(tick2segment(0), tick2segment(tick), 0, nstaves());
                        }
                  }
            else if (cmd == "transpose")
                  transpose();
            else if (cmd == "concert-pitch") {
                  if (styleB(ST_concertPitch) != a->isChecked())
                        cmdConcertPitchChanged(a->isChecked());
                  }
            else if (cmd == "reset-beammode")
                  cmdResetBeamMode();
            else if (cmd == "clef-violin")
                  cmdInsertClef(CLEF_G);
            else if (cmd == "clef-bass")
                  cmdInsertClef(CLEF_F);
            else if (cmd == "load-style") {
                  loadStyle();
                  setLayoutAll(true);
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
            else if (cmd == "system-break") {
                  Element* e = selection()->element();
                  if (e && e->type() == BAR_LINE) {
                        BarLine* barline = static_cast<BarLine*>(e);
                        Measure* measure = barline->measure();
                        if (!measure->lineBreak()) {
                              LayoutBreak* lb = new LayoutBreak(this);
                              lb->setSubtype(LAYOUT_BREAK_LINE);
                              lb->setTrack(-1);       // this are system elements
                              lb->setParent(measure);
                              cmdAdd(lb);
                              }
                        else {
                              // remove line break
                              foreach(Element* e, *measure->el()) {
                                    if (e->type() == LAYOUT_BREAK && e->subtype() == LAYOUT_BREAK_LINE) {
                                          cmdRemove(e);
                                          break;
                                          }
                                    }
                              }
                        }
                  }
            else if (cmd == "page-break") {
                  Element* e = selection()->element();
                  if (e && e->type() == BAR_LINE) {
                        BarLine* barline = static_cast<BarLine*>(e);
                        Measure* measure = barline->measure();
                        if (!measure->pageBreak()) {
                              LayoutBreak* lb = new LayoutBreak(this);
                              lb->setSubtype(LAYOUT_BREAK_PAGE);
                              lb->setTrack(-1);       // this are system elements
                              lb->setParent(measure);
                              cmdAdd(lb);
                              }
                        else {
                              // remove line break
                              foreach(Element* e, *measure->el()) {
                                    if (e->type() == LAYOUT_BREAK && e->subtype() == LAYOUT_BREAK_PAGE) {
                                          cmdRemove(e);
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
            else if (cmd == "repeat-sel") {
printf("repeat selection\n");
                  cmdRepeatSelection();
                  }
            else if (cmd == "")
                  ;
            else
                  printf("unknown cmd <%s>\n", qPrintable(cmd));
            endCmd();
            }
      }

//---------------------------------------------------------
//   processMidiInput
//---------------------------------------------------------

void Score::processMidiInput()
      {
      if (midiInputQueue.isEmpty())
            return;

//TODO-S      if (!noteEntryMode())
//            setNoteEntry(true);
      if (!noteEntryMode())
            return;

      while (!midiInputQueue.isEmpty()) {
            MidiInputEvent ev = midiInputQueue.dequeue();
            if (midiActionMap[ev.pitch] && midiActionMap[ev.pitch]->action)
                  midiActionMap[ev.pitch]->action->activate(QAction::Trigger);
            else {
                  startCmd();
                  cmdAddPitch1(ev.pitch, ev.chord);
                  layoutAll = true;
                  endCmd();
                  }
            }
      }

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Score::cmdPaste()
      {
      const QMimeData* ms = QApplication::clipboard()->mimeData();
      if (ms == 0) {
            printf("no application mime data\n");
            return;
            }
      if (selection()->state() == SEL_SINGLE && ms->hasFormat(mimeSymbolFormat)) {
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
                        addRefresh(selection()->element()->abbox());   // layout() ?!
                        selection()->element()->drop(QPointF(), QPointF(), el);
                        if (selection()->element())
                              addRefresh(selection()->element()->abbox());
                        }
                  }
            else
                  printf("cannot read type\n");
            }
      else if ((selection()->state() == SEL_STAFF || selection()->state() == SEL_SINGLE)
         && ms->hasFormat(mimeStaffListFormat)) {
            ChordRest* cr = 0;
            if (selection()->state() == SEL_STAFF) {
                  cr = selection()->firstChordRest();
                  }
            else if (selection()->state() == SEL_SINGLE) {
                  Element* e = selection()->element();
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
// printf("paste <%s>\n", data.data());
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
      else if (ms->hasFormat(mimeSymbolListFormat) && selection()->state() == SEL_SINGLE) {
            printf("cannot paste symbol list to element\n");
            }
      else {
            printf("cannot paste selState %d staffList %d\n",
               selection()->state(), ms->hasFormat(mimeStaffListFormat));
            foreach(const QString& s, ms->formats())
                  printf("  format %s\n", qPrintable(s));
            }
      }

//---------------------------------------------------------
//   pasteStaff
//---------------------------------------------------------

void Score::pasteStaff(QDomElement e, ChordRest* dst)
      {
      foreach(Element* el, _gel) {
            if (el->type() == SLUR)
                  static_cast<Slur*>(el)->setId(0);
            }
      foreach(Beam* beam, _beams)
            beam->setId(-1);
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            foreach(Tuplet* tuplet, *m->tuplets())
                  tuplet->setId(-1);
            }

      int dstStaffStart = dst->staffIdx();
      curTick = 0;
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

            for (int i = 0; i < staves; ++i) {
                  int staffIdx = i + dstStaffStart;
                  if (staffIdx >= nstaves())
                        break;
                  Fraction len = Fraction(tickLen, 1) / Fraction(AL::division * 4, 1);
                  Fraction gap = makeGap1(dst, len);
                  if (gap != len)
                        printf("cannot make gap %d/%d at %d (got %d/%d) staff %d\n",
                           len.numerator(), len.denominator(), tickLen, gap.numerator(), gap.denominator(), staffIdx);
                  }

            for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                  if (ee.tagName() != "Staff") {
                        domError(ee);
                        continue;
                        }
                  int srcStaffIdx = ee.attribute("id", "0").toInt();
                  int dstStaffIdx = srcStaffIdx - srcStaffStart + dstStaffStart;
// printf("srcStaffIDx %d  dstStaffIdx %d  staves %d\n", srcStaffIdx, dstStaffIdx, nstaves());
                  if (dstStaffIdx >= nstaves())
                        break;
                  QList<Tuplet*> tuplets;
                  for (QDomElement eee = ee.firstChildElement(); !eee.isNull(); eee = eee.nextSiblingElement()) {
                        const QString& tag(eee.tagName());
                        if (tag == "Tuplet") {
                              Tuplet* tuplet = new Tuplet(this);
                              tuplet->setTrack(curTrack);
                              tuplet->setTick(curTick);
                              tuplet->read(eee);
                              curTick  = tuplet->tick();
                              int tick = curTick - tickStart + dstTick;
                              Measure* measure = tick2measure(tick);
                              tuplet->setParent(measure);
                              tuplets.append(tuplet);
                              undoAddElement(tuplet);
                              }
                        else if (tag == "Slur") {
                              Slur* slur = new Slur(this);
                              slur->read(eee);
                              slur->setTrack(-1);
                              slur->setTick(-1);
                              undoAddElement(slur);
                              }
                        else if (tag == "Chord" || tag == "Rest") {
                              ChordRest* cr;
                              if (tag == "Chord")
                                   cr = new Chord(this);
                              else
                                   cr = new Rest(this);
                              cr->setTrack(curTrack);
                              cr->setTick(curTick);         // set default tick position
                              cr->read(eee, tuplets);
                              int voice = cr->voice();
                              int track = dstStaffIdx * VOICES + voice;
                              cr->setTrack(track);

                              curTick  = cr->tick();
                              int tick = cr->tick() - tickStart + dstTick;
                              cr->setTick(tick);

                              if (cr->type() == CHORD) {
                                    // set note track
                                    // check if staffMove moves a note to a
                                    // nonexistant staff
                                    //
                                    Chord* c      = static_cast<Chord*>(cr);
                                    NoteList* nl  = c->noteList();
                                    Part* part    = cr->staff()->part();
                                    int nn = (track / VOICES) + c->staffMove();
                                    if (nn < 0 || nn >= nstaves())
                                          c->setStaffMove(0);
                                    for (iNote i = nl->begin(); i != nl->end(); ++i) {
                                          Note* n = i->second;
                                          n->setPitch(n->pitch() - part->pitchOffset());
                                          n->setTrack(track);
                                          }
                                    }

                              Measure* measure = tick2measure(tick);

                              Segment* s;
                              bool isGrace = false;
                              if ((cr->type() == CHORD) && (((Chord*)cr)->noteType() != NOTE_NORMAL)) {
                                    s = measure->createSegment(Segment::SegGrace, tick);
                                    undoAddElement(s);
                                    isGrace = true;
                                    }
                              else {
                                    Segment::SegmentType st;
                                    st = Segment::segmentType(cr->type());
                                    s  = measure->findSegment(st, tick);
                                    if (!s) {
                                          s = measure->createSegment(st, tick);
                                          undoAddElement(s);
                                          }
                                    }
                              cr->setParent(s);
                              curTick  += cr->ticks();

                              int measureEnd = measure->tick() + measure->tickLen();
                              if (!isGrace && (cr->tick() + cr->tickLen() > measureEnd)) {
                                    if (cr->type() == CHORD) {
                                          // split Chord
                                          Chord* c = static_cast<Chord*>(cr);
                                          int rest = c->tickLen();
                                          int len  = measureEnd - c->tick();
                                          rest    -= len;
                                          Duration d;
                                          d.setVal(len);
                                          c->setDuration(d);
                                          undoAddElement(c);
                                          while (rest) {
                                                int tick = c->tick() + c->tickLen();
                                                measure = tick2measure(tick);
                                                if (measure->tick() != tick)  // last measure
                                                      break;
                                                Chord* c2 = static_cast<Chord*>(c->clone());
                                                c2->setTick(tick);
                                                len = measure->tickLen() > rest ? rest : measure->tickLen();
                                                Duration d;
                                                d.setVal(len);
                                                c2->setDuration(d);
                                                rest -= len;
                                                s     = measure->findSegment(Segment::SegChordRest, tick);
                                                if (s == 0) {
                                                      s = measure->createSegment(Segment::SegChordRest, tick);
                                                      undoAddElement(s);
                                                      }
                                                c2->setParent(s);
                                                undoAddElement(c2);

                                                NoteList* nl1 = c->noteList();
                                                NoteList* nl2 = c2->noteList();
                                                iNote i1      = nl1->begin();
                                                iNote i2      = nl2->begin();

                                                for (; i1 != nl1->end(); i1++, i2++) {
                                                      Tie* tie = new Tie(this);
                                                      tie->setStartNote(i1->second);
                                                      tie->setEndNote(i2->second);
                                                      tie->setTrack(c->track());
                                                      Tie* tie2 = i1->second->tieFor();
                                                      if (tie2) {
                                                            i2->second->setTieFor(i1->second->tieFor());
                                                            tie2->setStartNote(i2->second);
                                                            }
                                                      i1->second->setTieFor(tie);
                                                      i2->second->setTieBack(tie);
                                                      }
                                                c = c2;
                                                }
                                          }
                                    else {
                                          // split Rest
                                          Rest* r  = static_cast<Rest*>(cr);
                                          int rest = r->tickLen();
                                          int len  = measureEnd - r->tick();
                                          rest    -= len;
                                          Duration d;
                                          d.setVal(len);
                                          r->setDuration(d);
                                          undoAddElement(r);
                                          while (rest) {
                                                Rest* r2 = static_cast<Rest*>(r->clone());
                                                int tick = r->tick() + r->tickLen();
                                                r2->setTick(tick);
                                                measure = tick2measure(tick);
                                                len = measure->tickLen() > rest ? rest : measure->tickLen();
                                                Duration d;
                                                d.setVal(len);
                                                r2->setDuration(d);
								rest -= len;
                                                s     = measure->findSegment(Segment::SegChordRest, tick);
                                                if (s == 0) {
                                                      s = measure->createSegment(Segment::SegChordRest, tick);
                                                      undoAddElement(s);
                                                      }
                                                r2->setParent(s);
                                                undoAddElement(r2);
                                                r = r2;
                                                }
                                          }
                                    }
                              else {
                                    undoAddElement(cr);
                                    }
                              }
                        else if (tag == "Lyrics") {
                              Lyrics* lyrics = new Lyrics(this);
                              lyrics->setTick(curTick);         // set default tick position
                              lyrics->setTrack(curTrack);
                              lyrics->read(eee);
                              lyrics->setTrack(dstStaffIdx * VOICES);
                              int tick = lyrics->tick() - tickStart + dstTick;
                              lyrics->setTick(tick);
                              Segment* segment = tick2segment(tick);
                              if (segment) {
                                    lyrics->setParent(segment);
                                    undoAddElement(lyrics);
                                    }
                              else
                                    printf("no segment found for lyrics\n");
                              }
                        else if (tag == "Harmony") {
                              Harmony* harmony = new Harmony(this);
                              harmony->setTick(curTick);         // set default tick position
                              harmony->setTrack(curTrack);
                              harmony->read(eee);
                              harmony->setTrack(dstStaffIdx * VOICES);
                              int tick = harmony->tick() - tickStart + dstTick;
                              harmony->setTick(tick);
                              Measure* m = tick2measure(tick);
                              harmony->setParent(m);
                              undoAddElement(harmony);
                              }
                        else {
                              domError(eee);
                              continue;
                              }
                        }
                  }
            Segment* s1 = tick2segment(dstTick);
            Segment* s2 = tick2segment(dstTick + tickLen);
            selection()->setRange(s1, s2, dstStaffStart, dstStaffStart+staves);
            updateSelectedElements(SEL_STAFF);
            if (selection()->state() != SEL_STAFF) {
                  selection()->setState(SEL_STAFF);
                  emit selectionChanged(int(selection()->state()));
                  }
            }
      connectTies();
      fixPpitch();
      }

//---------------------------------------------------------
//   cmdReplaceElements
//---------------------------------------------------------

void Score::cmdReplaceElements(Measure* sm, Measure* dm, int srcStaffIdx, int dstStaffIdx)
      {
      //
      // TODO: handle special cases: sm->tickLen() != ds->tickLen()
      //

      select(0, SELECT_SINGLE, 0);
      //
      // clear staff in destination Measure
      //
      for (Segment* s = dm->first(); s;) {
            if (s->subtype() == Segment::SegEndBarLine) {     // do not remove
                  s = s->next();
                  continue;
                  }
            int startTrack = dstStaffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (e) {
                        if (e->generated()) {
                              s->remove(e);
                              }
                        else {
                              undoRemoveElement(e);
                              }
                        }
                  }
            Segment* ns = s->next();
            dm->cmdRemoveEmptySegment(s);
            s = ns;
            }

      foreach(Tuplet* tuplet, *dm->tuplets()) {
            if (tuplet->staffIdx() == dstStaffIdx)
                  undoRemoveElement(tuplet);
            }

      int trackOffset   = (dstStaffIdx - srcStaffIdx) * VOICES;
      foreach(Tuplet* tuplet, *sm->tuplets()) {
            tuplet->setParent(dm);
            tuplet->setTrack(tuplet->track() + trackOffset);
            tuplet->clear();
            undoAddElement(tuplet);
            }

      //
      // add src elements to destination
      //

      int srcTickOffset = sm->tick();
      int dstTickOffset = dm->tick();

      for (Segment* s = sm->first(); s; s = s->next()) {
            //
            // paste only notes and rests
            //
            if (s->subtype() != Segment::SegGrace && s->subtype() != Segment::SegChordRest)
                  continue;
            int startTrack = srcStaffIdx * VOICES;
            int endTrack   = startTrack + VOICES;
            int tick       = s->tick() - srcTickOffset + dstTickOffset;
            Segment* ns    = dm->findSegment((Segment::SegmentType)s->subtype(), tick);
            if (ns == 0) {
                  ns = dm->createSegment((Segment::SegmentType)s->subtype(), tick);
                  undoAddElement(ns);
                  printf("add segment %s\n", ns->subTypeName());
                  }
            for (int t = startTrack; t < endTrack; ++t) {
                  Element* e = s->element(t);
                  if (!e || !e->isChordRest())
                        continue;
                  e->setParent(ns);
                  e->setTick(tick);
                  e->setTrack(e->track() + trackOffset);
                  undoAddElement(e);
// printf("add elem %s\n", e->name());
                  e->setSelected(false);
                  if (e->type() == REST)
                        select(e, SELECT_RANGE, 0);
                  else {
                        Chord* c = (Chord*)e;
                        NoteList* nl = c->noteList();
                        for (iNote in = nl->begin(); in != nl->end(); ++in) {
                              select(in->second, SELECT_RANGE, 0);
                              }
                        }
                  }
            if (ns->isEmpty()) {
                  dm->cmdRemoveEmptySegment(ns);
// printf("remove empty segment %s in copy!\n", ns->subTypeName());
                  }
            }
      }

//---------------------------------------------------------
//   moveInputPos
//---------------------------------------------------------

void Score::moveInputPos(Segment* s)
      {
      if (s == 0)
            return;
      _is._segment = s;
      emit posChanged(s->tick());
      Element* el;
      if (s->element(_is.track))
            el = s->element(_is.track);
      else
            el = s->element(_is.track / VOICES * VOICES);
      if (el->type() == CHORD)
            el = static_cast<Chord*>(el)->upNote();
//TODO-S      emit adjustCanvasPosition(el, false);
      }

//---------------------------------------------------------
//   moveToNextInputPos
//   TODO: special case: note is first note of tie: goto to last note of tie
//---------------------------------------------------------

void Score::moveToNextInputPos()
      {
      Segment* s = _is._segment;
      Measure* m = s->measure();
      int track  = _is.track;
      for (s = s->next1(); s; s = s->next1()) {
            if (s->subtype() != Segment::SegChordRest)
                  continue;
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
      ChordRest* cr = selection()->lastChordRest();
      if (selection()->activeCR())
            cr = selection()->activeCR();
      if (!cr)
            return 0;

      Element* el = 0;
      if (cmd == "next-chord") {
            if (noteEntryMode())
                  moveToNextInputPos();
            el = nextChordRest(cr);
            }
      else if (cmd == "prev-chord") {
            if (noteEntryMode()) {
                  Segment* s = _is._segment->prev1();
                  //
                  // if _is._segment is first chord/rest segment in measure
                  // make sure "m" points to previous measure
                  //
                  while (s && s->subtype() != Segment::SegChordRest)
                        s = s->prev1();
                  if (s == 0)
                        return 0;
                  Measure* m = s->measure();

                  int track  = _is.track;
                  for (; s; s = s->prev1()) {
                        if (s->subtype() != Segment::SegChordRest)
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
      else if (cmd == "next-measure")
            el = nextMeasure(cr);
      else if (cmd == "prev-measure")
            el = prevMeasure(cr);
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
      ChordRest* cr = selection()->lastChordRest();
      if (selection()->activeCR())
            cr = selection()->activeCR();
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
      Element* el = selection()->element();
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
            cr->setDuration(d);
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
      Element* el = selection()->element();
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
            cr->setDuration(d);
            }
      else
            changeCRlen(cr, d);
      _is.setDuration(d);
      nextInputPos(cr, false);
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void Score::cmdRepeatSelection()
      {
      if ((selection()->state() != SEL_STAFF) && (selection()->state() != SEL_SYSTEM)) {
            printf("wrong selection type\n");
            return;
            }

      QString mimeType = selection()->mimeType();
      if (mimeType.isEmpty()) {
            printf("mime type is empty\n");
            return;
            }
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, selection()->mimeData());
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

      int dStaff = selection()->staffStart();
      Segment* endSegment = selection()->endSegment();
      if (endSegment && endSegment->element(dStaff)) {
            Element* e = endSegment->element(dStaff * VOICES);
            if (e && e->isChordRest()) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  pasteStaff(doc.documentElement(), cr);
                  }
            else
                  printf("??? %p <%s>\n", e, e ? e->name() : "");
            }
      else
            printf("?? %p\n", endSegment);
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
                  if (segment->subtype() != Segment::SegChordRest)
                        continue;
                  int track;
                  for (track = 0; track < tracks; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                        if (cr) {
                              Element* e = cr->type() == CHORD ? static_cast<Chord*>(cr)->upNote() : 0;
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


