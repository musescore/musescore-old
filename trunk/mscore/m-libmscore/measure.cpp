//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: measure.cpp 3696 2010-11-10 09:31:20Z wschweer $
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
 Implementation of most part of class Measure.
*/

#include <math.h>

#include "m-al/xml.h"
#include "measure.h"
#include "segment.h"
#include "score.h"
#include "spanner.h"
#include "durationtype.h"
#include "lyrics.h"
#include "chord.h"
#include "note.h"
#include "rest.h"
#include "staff.h"
#include "spacer.h"
#include "system.h"
#include "barline.h"
#include "slur.h"
#include "clef.h"
#include "key.h"
#include "dynamics.h"
#include "m-al/sig.h"
#include "beam.h"
#include "tuplet.h"
#include "hairpin.h"
#include "text.h"
#include "select.h"
#include "part.h"
#include "style.h"
#include "bracket.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "timesig.h"
#include "layoutbreak.h"
#include "page.h"
#include "volta.h"
#include "image.h"
#include "hook.h"
#include "beam.h"
#include "pitchspelling.h"
#include "keysig.h"
#include "breath.h"
#include "tremolo.h"
#include "drumset.h"
#include "repeat.h"
#include "box.h"
#include "harmony.h"
#include "tempotext.h"
#include "sym.h"
#include "stafftext.h"
#include "utils.h"
#include "glissando.h"
#include "articulation.h"
#include "spacer.h"
#include "duration.h"
#include "fret.h"
#include "stafftype.h"
#include "tablature.h"

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::MStaff()
      {
      distanceUp   = .0;
      distanceDown = .0;
      lines        = 0;
      hasVoices    = false;
      _vspacerUp   = 0;
      _vspacerDown = 0;
      _visible     = true;
      _slashStyle  = false;
      }

MStaff::~MStaff()
      {
      delete lines;
      delete _vspacerUp;
      delete _vspacerDown;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s), _first(0), _last(0), _size(0),
     _timesig(4,4), _len(4,4)
      {
      _repeatCount           = 2;
      _repeatFlags           = 0;

      int n = _score->nstaves();
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            s->lines     = new StaffLines(score());
            s->lines->setTrack(staffIdx * VOICES);
            s->lines->setParent(this);
            s->lines->setVisible(!staff->invisible());
            staves.push_back(s);
            }

      _no                    = 0;
      _noOffset              = 0;
      _noText                = 0;
      _userStretch           = 1.0;     // ::style->measureSpacing;
      _irregular             = false;
      _breakMultiMeasureRest = false;
      _breakMMRest           = false;
      _endBarLineGenerated   = true;
      _endBarLineVisible     = true;
      _endBarLineType        = NORMAL_BAR;
      _mmEndBarLineType      = NORMAL_BAR;
      _multiMeasure          = 0;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::~Measure()
      {
      for (Segment* s = first(); s;) {
            Segment* ns = s->next();
            delete s;
            s = ns;
            }
      foreach(MStaff* m, staves)
            delete m;
      foreach(Tuplet* t, _tuplets)
            delete t;
      delete _noText;
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

/**
 Insert Segment \a e before Segment \a el.
*/

void Measure::insert(Segment* e, Segment* el)
      {
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == _first) {
            push_front(e);
            return;
            }
      e->setParent(this);
      ++_size;
      e->setNext(el);
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Measure::remove(Segment* el)
      {
      int tracks = staves.size() * VOICES;
      for (int track = 0; track < tracks; track += VOICES) {
            if (!el->element(track))
                  continue;
            }

      // debug:
      bool found = false;
      for (Segment* s = _first; s; s = s->next()) {
            if (el == s) {
                  found = true;
                  break;
                  }
            }
      --_size;
      if (el == _first) {
            _first = _first->next();
            if (_first)
                  _first->setPrev(0);
            if (el == _last)
                  _last = 0;
            return;
            }
      if (el == _last) {
            _last = _last->prev();
            if (_last)
                  _last->setNext(0);
            return;
            }
      el->prev()->setNext(el->next());
      el->next()->setPrev(el->prev());
      }

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void Measure::push_back(Segment* e)
      {
      ++_size;
      e->setParent(this);
      if (_last) {
            _last->setNext(e);
            e->setPrev(_last);
            e->setNext(0);
            }
      else {
            _first = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _last = e;
      }

//---------------------------------------------------------
//   push_front
//---------------------------------------------------------

void Measure::push_front(Segment* e)
      {
      ++_size;
      e->setParent(this);
      if (_first) {
            _first->setPrev(e);
            e->setNext(_first);
            e->setPrev(0);
            }
      else {
            _last = e;
            e->setPrev(0);
            e->setNext(0);
            }
      _first = e;
      }

//---------------------------------------------------------
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

void initLineList(char* ll, int key)
      {
      memset(ll, 0, 74);
      for (int octave = 0; octave < 11; ++octave) {
            if (key > 0) {
                  for (int i = 0; i < key; ++i) {
                        int idx = tpc2step(20 + i) + octave * 7;
                        if (idx < 74)
                              ll[idx] = 1;
                        }
                  }
            else {
                  for (int i = 0; i > key; --i) {
                        int idx = tpc2step(12 + i) + octave * 7;
                        if (idx < 74)
                              ll[idx] = -1;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      qreal x;
      };

//---------------------------------------------------------
//   layoutChords0
//    only called from layout0
//    computes note lines and accidentals
//---------------------------------------------------------

void Measure::layoutChords0(Segment* segment, int startTrack)
      {
      int staffIdx     = startTrack/VOICES;
      Staff* staff     = score()->staff(staffIdx);
      qreal staffMag  = staff->mag();
      Drumset* drumset = 0;

      if (staff->part()->instr()->useDrumset())
            drumset = staff->part()->instr()->drumset();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e)
                 continue;
            ChordRest* cr = static_cast<ChordRest*>(e);
            qreal m = staffMag;
            if (cr->small())
                  m *= score()->styleD(ST_smallNoteMag);

            if (cr->type() == CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  if (chord->noteType() != NOTE_NORMAL)
                        m *= score()->styleD(ST_graceNoteMag);
                  foreach(Note* note, chord->notes()) {
                        if (drumset) {
                              int pitch = note->pitch();
                              if (drumset->isValid(pitch)) {
                                    note->setHeadGroup(drumset->noteHead(pitch));
                                    note->setLine(drumset->line(pitch));
                                    continue;
                                    }
                              }
                        }
                  chord->computeUp();
                  }
            cr->setMag(m);
            }
      }

//---------------------------------------------------------
//   layoutChords10
//    computes note lines and accidentals
//---------------------------------------------------------

void Measure::layoutChords10(Segment* segment, int startTrack, char* tversatz)
      {
      int staffIdx     = startTrack/VOICES;
      Staff* staff     = score()->staff(staffIdx);
      Drumset* drumset = 0;

      if (staff->part()->instr()->useDrumset())
            drumset = staff->part()->instr()->drumset();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e || e->type() != CHORD)
                 continue;
            Chord* chord = static_cast<Chord*>(e);
            foreach(Note* note, chord->notes()) {
                  if (note->tieBack()) {
                        int line = note->tieBack()->startNote()->line();
                        note->setLine(line);

                        int tpc = note->tpc();
                        line = tpc2step(tpc) + (note->pitch()/12) * 7;
                        int tpcPitch   = tpc2pitch(tpc);
                        if (tpcPitch < 0)
                              line += 7;
                        else
                              line -= (tpcPitch/12)*7;
                        // tversatz[line] = tpc2alter(tpc);
                        continue;
                        }

                  if (drumset) {
                        int pitch = note->pitch();
                        if (drumset->isValid(pitch)) {
                              note->setHeadGroup(drumset->noteHead(pitch));
                              note->setLine(drumset->line(pitch));
                              continue;
                              }
                        }
                  note->layout10(tversatz);
                  }
            }
      }

//---------------------------------------------------------
//   findAccidental
//---------------------------------------------------------

int Measure::findAccidental(Note* note) const
      {
      char tversatz[75];      // list of already set accidentals for this measure
      KeySigEvent key = note->chord()->staff()->keymap()->key(tick());
      initLineList(tversatz, key.accidentalType());

      for (Segment* segment = first(); segment; segment = segment->next()) {
            if ((segment->subtype() != SegChordRest) && (segment->subtype() != SegGrace))
                  continue;
            int startTrack = note->staffIdx() * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);

                  Drumset* drumset = 0;
                  if (chord->staff()->part()->instr()->useDrumset())
                        drumset = chord->staff()->part()->instr()->drumset();

                  foreach(Note* note1, chord->notes()) {
                        if (note1->tieBack())
                              continue;
                        int pitch   = note1->pitch();
                        //
                        // compute accidental
                        //
                        int tpc        = note1->tpc();
                        int line       = tpc2step(tpc) + (pitch/12) * 7;
                        int tpcPitch   = tpc2pitch(tpc);
                        if (tpcPitch < 0)
                              line += 7;
                        else
                              line -= (tpcPitch/12)*7;

                        int accVal = ((tpc + 1) / 7) - 2;
                        if (accVal != tversatz[line]) {
                              if (note == note1) {
                                    switch(accVal) {
                                          case -2: return ACC_FLAT2;
                                          case -1: return ACC_FLAT;
                                          case  1: return ACC_SHARP;
                                          case  2: return ACC_SHARP2;
                                          case  0: return ACC_NATURAL;
                                          default:
                                                return 0;
                                          }
                                    }
                              tversatz[line] = accVal;
                              }
                        else {
                              if (note == note1)
                                    return 0;
                              }
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findAccidental2
//    return current accidental value at note position
//---------------------------------------------------------

int Measure::findAccidental2(Note* note) const
      {
      char tversatz[75];      // list of already set accidentals for this measure
      KeySigEvent key = note->chord()->staff()->keymap()->key(tick());
      initLineList(tversatz, key.accidentalType());

      for (Segment* segment = first(); segment; segment = segment->next()) {
            if ((segment->subtype() != SegChordRest) && (segment->subtype() != SegGrace))
                  continue;
            int startTrack = note->staffIdx() * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (!e || e->type() != CHORD)
                        continue;
                  Chord* chord = static_cast<Chord*>(e);

                  Drumset* drumset = 0;
                  if (chord->staff()->part()->instr()->useDrumset())
                        drumset = chord->staff()->part()->instr()->drumset();

                  foreach(Note* note1, chord->notes()) {
                        if (note1->tieBack())
                              continue;
                        int pitch   = note1->pitch();

                        //
                        // compute accidental
                        //
                        int tpc        = note1->tpc();
                        int line       = tpc2step(tpc) + (pitch/12) * 7;
                        int tpcPitch   = tpc2pitch(tpc);
                        if (tpcPitch < 0)
                              line += 7;
                        else
                              line -= (tpcPitch/12)*7;

                        if (note == note1)
                              return tversatz[line];
                        int accVal = ((tpc + 1) / 7) - 2;
                        if (accVal != tversatz[line])
                              tversatz[line] = accVal;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   Measure::layout
//---------------------------------------------------------

/**
 Layout measure; must fit into  \a width.

 Note: minWidth = width - stretch
*/

void Measure::layout(qreal width)
      {
      int nstaves = _score->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            staves[staffIdx]->distanceUp = 0.0;
            staves[staffIdx]->distanceDown = 0.0;
            StaffLines* sl = staves[staffIdx]->lines;
            if (sl)
                  sl->setMag(score()->staff(staffIdx)->mag());
            }

      // height of boundingRect will be set in system->layout2()
      // keep old value for relayout

      setbbox(QRectF(0.0, 0.0, width, height()));
      layoutX(width);
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

qreal Measure::tick2pos(int tck) const
      {
      Segment* s;
      qreal x1 = 0;
      qreal x2 = 0;
      int tick1 = tick();
      int tick2 = tick1;
      for (s = _first; s; s = s->next()) {
            if (s->subtype() != SegChordRest)
                  continue;
            x2 = s->x();
            tick2 = s->tick();
            if (tck <= tick2) {
                  if (tck == tick2)
                        x1 = x2;
                  break;
                  }
            x1    = x2;
            tick1 = tick2;
            }
      if (s == 0) {
            x2    = width();
            tick2 = tick() + ticks();
            }
      qreal x = 0;
      if (tick2 > tick1) {
            qreal dx = x2 - x1;
            int dt    = tick2 - tick1;
            if (dt == 0)
                  x = 0.0;
            else
                  x = dx * (tck - tick1) / dt;
            }
      return x1 + x;
      }

//---------------------------------------------------------
//   layout2
//    called after layout of all pages
//---------------------------------------------------------

void Measure::layout2()
      {
      if (parent() == 0)
            return;

      qreal _spatium = spatium();
      int tracks = staves.size() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            SegmentTypes st = SegGrace | SegChordRest;
            for (Segment* s = first(st); s; s = s->next(st)) {
                  ChordRest* cr = static_cast<ChordRest*>(s->element(track));
                  if (!cr)
                        continue;
                  foreach(Lyrics* lyrics, cr->lyricsList()) {
                        if (lyrics)
                              system()->layoutLyrics(lyrics, s, track/VOICES);
                        }
                  }
            if (track % VOICES == 0) {
                  int staffIdx = track / VOICES;
                  qreal y = system()->staff(staffIdx)->y();
                  Spacer* sp = staves[staffIdx]->_vspacerDown;
                  if (sp) {
                        sp->layout();
                        int n = score()->staff(staffIdx)->lines() - 1;
                        sp->setPos(_spatium * .5, y + n * _spatium);
                        }
                  sp = staves[staffIdx]->_vspacerUp;
                  if (sp) {
                        sp->layout();
                        sp->setPos(_spatium * .5, y - sp->getSpace().val() * _spatium);
                        }
                  }
            }

      foreach(const MStaff* ms, staves)
            ms->lines->setWidth(width());

      foreach (Element* element, _el) {
            element->layout();
            if (element->type() == LAYOUT_BREAK) {
                  if (_sectionBreak && (_lineBreak | _pageBreak)
                     && (element->subtype() == LAYOUT_BREAK_PAGE || element->subtype() == LAYOUT_BREAK_LINE))
                        {
                        element->rxpos() -= (element->width() + _spatium * .8);
                        }
                  }
            }

      //
      //   set measure number
      //
      int pn    = _no;
      QString s = QString("%1").arg(pn + 1);

      QString ns;
      if (score()->styleB(ST_showMeasureNumber)
         && !_irregular
         && (pn || score()->styleB(ST_showMeasureNumberOne))) {
            if (score()->styleB(ST_measureNumberSystem)) {
                  if (system() && !system()->measures().empty() && system()->measures().front() == this)
                        ns = s;
                  }
            else if ((pn % score()->style(ST_measureNumberInterval).toInt()) == 0)
                  ns = s;
            }
      if (!ns.isEmpty()) {
            if (_noText == 0) {
                  _noText = new Text(score());
                  _noText->setGenerated(true);
                  _noText->setSubtype(TEXT_MEASURE_NUMBER);
                  _noText->setTextStyle(TEXT_STYLE_MEASURE_NUMBER);
                  _noText->setParent(this);
                  _noText->setSelectable(false);
                  }
            if (_noText->getText() != s)
                  _noText->setText(s);
            }
      else {
            delete _noText;
            _noText = 0;
            }
      if (_noText)
            _noText->layout();

      //
      // slur layout needs articulation layout first
      //
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = 0; track < tracks; ++track) {
                  Element* el = s->element(track);
                  if (el && el->isChordRest()) {
                        foreach(Slur* slur, static_cast<ChordRest*>(el)->slurFor())
                              slur->layout();
                        }
                  }
            }

      foreach(Tuplet* tuplet, _tuplets)
            tuplet->layout();
      }

//---------------------------------------------------------
//   findChord
//---------------------------------------------------------

/**
 Search for chord at position \a tick in \a track at grace level \a gl.
 Grace level is 0 for a normal chord, 1 for the grace note closest
 to the normal chord, etc.
*/

Chord* Measure::findChord(int tick, int track, int gl)
      {
      int graces = 0;
      for (Segment* seg = last(); seg; seg = seg->prev()) {
            if (seg->tick() < tick)
                  return 0;
            if (seg->tick() == tick) {
                  if (seg->subtype() == SegGrace)
                        graces++;
                  Element* el = seg->element(track);
                  if (el && el->type() == CHORD && graces == gl) {
                        return (Chord*)el;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findChordRest
//---------------------------------------------------------

/**
 Search for chord or rest at position \a tick at \a staff in \a voice.
*/

ChordRest* Measure::findChordRest(int tick, int track)
      {
      for (Segment* seg = _first; seg; seg = seg->next()) {
            if (seg->tick() > tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(track);
                  if (el && (el->type() == CHORD || el->type() == REST)) {
                        return (ChordRest*)el;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   tick2segment
//---------------------------------------------------------

Segment* Measure::tick2segment(int tick, bool grace) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->tick() == tick) {
                  SegmentType t = SegmentType(s->subtype());
                  if (grace && (t == SegChordRest || t == SegGrace))
                        return s;
                  if (t == SegChordRest)
                        return s;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
//---------------------------------------------------------

/**
 Search for a segment of type \a st at position \a t.
*/

Segment* Measure::findSegment(SegmentType st, int t)
      {
      Segment* s;
      for (s = first(); s && s->tick() < t; s = s->next())
            ;

      for (Segment* ss = s; ss && ss->tick() == t; ss = ss->next()) {
            if (ss->subtype() == st)
                  return ss;
            }
      return 0;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e, int tick)
      {
      SegmentType st;
      if ((e->type() == CHORD) && (((Chord*)e)->noteType() != NOTE_NORMAL)) {
            Segment* s = findSegment(SegGrace, tick);
            if (s) {
                  if (s->element(e->track())) {
                        s = s->next();
                        if (s && s->subtype() == SegGrace && !s->element(e->track()))
                              return s;
                        }
                  else
                        return s;
                  }
            s = new Segment(this, SegGrace, tick);
            add(s);
            return s;
            }
      st = Segment::segmentType(e->type());
      return getSegment(st, tick);
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

/**
 Get a segment of type \a st at tick position \a t.
 If the segment does not exist, it is created.
*/

Segment* Measure::getSegment(SegmentType st, int t)
      {
      Segment* s = findSegment(st, t);
      if (!s) {
            s = new Segment(this, st, t);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

/**
 Get a segment of type \a st at tick position \a t and grace level \a gl.
 Grace level is 0 for a normal chord, 1 for the grace note closest
 to the normal chord, etc.
 If the segment does not exist, it is created.
*/

// when looking for a SegChordRest, return the first one found at t
// when looking for a SegGrace, first search for a SegChordRest at t,
// then search backwards for gl SegGraces

Segment* Measure::getSegment(SegmentType st, int t, int gl)
      {
      if (st != SegChordRest && st != SegGrace) {
            return 0;
            }
      Segment* s;

      // find the first segment at tick >= t
      for (s = first(); s && s->tick() < t; s = s->next())
            ;

      // find the first SegChordRest segment at tick = t
      // while counting the SegGrace segments
      int nGraces = 0;
      Segment* sCr = 0;
      for (Segment* ss = s; ss && ss->tick() == t; ss = ss->next()) {
            if (ss->subtype() == SegGrace)
                  nGraces++;
            if (ss->subtype() == SegChordRest) {
                  sCr = ss;
                  break;
                  }
            }

      if (gl == 0) {
            if (sCr)
                  return sCr;
            // no SegChordRest at tick = t, must create it
            s = new Segment(this, SegChordRest, t);
            add(s);
            return s;
            }

      if (gl > 0) {
            if (gl <= nGraces) {
                  int graces = 0;
                  // for (Segment* ss = last(); ss && ss->tick() <= t; ss = ss->prev()) {
                  for (Segment* ss = last(); ss && ss->tick() >= t; ss = ss->prev()) {
                        if (ss->tick() > t)
                              continue;
                        if ((ss->subtype() == SegGrace) && (ss->tick() == t))
                              graces++;
                        if (gl == graces)
                              return ss;
                        }
                  return 0; // should not be reached
                  }
            else {
//                  printf("creating SegGrace at tick=%d and level=%d\n", t, gl);
                  Segment* prevs = 0; // last segment inserted
                  // insert the first grace segment
                  if (nGraces == 0) {
                        ++nGraces;
                        s = new Segment(this, SegGrace, t);
//                        printf("... creating SegGrace %p at tick=%d and level=%d\n", s, t, nGraces);
                        add(s);
                        prevs = s;
                        // return s;
                        }
                  // find the first grace segment at t
                  for (Segment* ss = last(); ss && ss->tick() <= t; ss = ss->prev()) {
                        if (ss->subtype() == SegGrace && ss->tick() == t)
                              prevs = ss;
                        }

                  // add the missing grace segments before the one already present
                  while (nGraces < gl) {
                        ++nGraces;
                        s = new Segment(this, SegGrace, t);
//                        printf("... creating SegGrace %p at tick=%d and level=%d\n", s, t, nGraces);
                        insert(s, prevs);
                        prevs = s;
                        }
                  return s;
                  }
            }

      return 0; // should not be reached
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

/**
 Add new Element \a el to Measure.
*/

void Measure::add(Element* el)
      {
      _dirty = true;

      el->setParent(this);
      ElementType type = el->type();

      switch (type) {
            case SPACER:
                  {
                  Spacer* sp = static_cast<Spacer*>(el);
                  if (sp->subtype() == SPACER_UP)
                        staves[el->staffIdx()]->_vspacerUp = sp;
                  else if (sp->subtype() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = sp;
                  }
                  break;
            case SEGMENT:
                  {
                  Segment* seg = static_cast<Segment*>(el);
                  int tracks = staves.size() * VOICES;
                  for (int track = 0; track < tracks; track += VOICES) {
                        if (!seg->element(track))
                              continue;
                                             }
                  int t = seg->tick();
                  int st = el->subtype();
                  Segment* s;
                  if (seg->prev() || seg->next()) {
                        //
                        // undo operation
                        //
                        if (seg->prev())
                              seg->prev()->setNext(seg);
                        else
                              _first = seg;
                        if (seg->next())
                              seg->next()->setPrev(seg);
                        else
                              _last = seg;
                        ++_size;
                        }
                  else {
                        if (st == SegGrace) {
                              for (s = first(); s && s->tick() < t; s = s->next())
                                    ;
                              if (s && (s->tick() > t)) {
                                    insert(seg, s);
                                    break;
                                    }
                              if (s && s->subtype() != SegChordRest) {
                                    for (; s && s->subtype() != SegEndBarLine
                                       && s->subtype() != SegChordRest; s = s->next())
                                          ;
                                    }
                              }
                        else {
                              for (s = first(); s && s->tick() < t; s = s->next())
                                    ;
                              if (s) {
                                    if (st == SegChordRest) {
                                          while (s && s->subtype() != st && s->tick() == t) {
                                                if (s->subtype() == SegEndBarLine)
                                                      break;
                                                s = s->next();
                                                }
                                          }
                                    else {
                                          while (s && s->subtype() <= st) {
                                                if (s->next() && s->next()->tick() != t)
                                                      break;
                                                s = s->next();
                                                }
                                          //
                                          // place breath _after_ chord
                                          //
                                          if (s && st == SegBreath)
                                                s = s->next();
                                          }
                                    }
                              }
                        insert(seg, s);
                        }
                  }
                  break;
            case TUPLET:
                  {
                  Tuplet* tuplet = static_cast<Tuplet*>(el);
                  _tuplets.append(tuplet);
                  foreach(DurationElement* cr, tuplet->elements())
                        cr->setTuplet(tuplet);
                  if (tuplet->tuplet())
                        tuplet->tuplet()->add(tuplet);
                  }
                  break;
            case LAYOUT_BREAK:
                  for (iElement i = _el.begin(); i != _el.end(); ++i) {
                        if ((*i)->type() == LAYOUT_BREAK && (*i)->subtype() == el->subtype()) {
                              return;
                              }
                        }
                  switch(el->subtype()) {
                        case LAYOUT_BREAK_PAGE:
                              _pageBreak = true;
                              break;
                        case LAYOUT_BREAK_LINE:
                              _lineBreak = true;
                              break;
                        case LAYOUT_BREAK_SECTION:
                              _sectionBreak = true;
                              _pause = static_cast<LayoutBreak*>(el)->pause();
                              break;
                        }
                  _el.push_back(el);
                  break;

            case JUMP:
                  _repeatFlags |= RepeatJump;
                  _el.append(el);
                  break;

            case HBOX:
                  if (type == TEXT && el->subtype() == TEXT_MEASURE_NUMBER) {
                        _noText = static_cast<Text*>(el);
                        }
                  else {
                      if (el->staff() != 0 ){
                        el->setMag(el->staff()->mag());
                      }
                      _el.append(el);
                      }
                  break;

            default:
                  break;
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

/**
 Remove Element \a el from Measure.
*/

void Measure::remove(Element* el)
      {
      _dirty = true;

      switch(el->type()) {
            case SPACER:
                  if (el->subtype() == SPACER_DOWN)
                        staves[el->staffIdx()]->_vspacerDown = 0;
                  else if (el->subtype() == SPACER_UP)
                        staves[el->staffIdx()]->_vspacerUp = 0;
                  break;
            case SEGMENT:
                  remove(static_cast<Segment*>(el));
                  break;
            case TUPLET:
                  {
                  Tuplet* tuplet = static_cast<Tuplet*>(el);
                  foreach(DurationElement* de, tuplet->elements())
                        de->setTuplet(0);
                  if (!_tuplets.removeOne(tuplet)) {
                        return;
                        }
                  if (tuplet->tuplet())
                        tuplet->tuplet()->remove(tuplet);
                  }
                  break;
            case LAYOUT_BREAK:
                  switch(el->subtype()) {
                        case LAYOUT_BREAK_PAGE:
                              _pageBreak = false;
                              break;
                        case LAYOUT_BREAK_LINE:
                              _lineBreak = false;
                              break;
                        case LAYOUT_BREAK_SECTION:
                              _sectionBreak = false;
                              _pause = 0.0;
                              break;
                        }
                  _el.remove(el);
                  break;

            case JUMP:
                  _repeatFlags &= ~RepeatJump;

            case HBOX:
                  if (el->type() == TEXT && el->subtype() == TEXT_MEASURE_NUMBER)
                        break;
                  _el.remove(el);
                  break;

            case CLEF:
            case CHORD:
            case REST:
            case TIMESIG:
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        int staves = _score->nstaves();
                        int tracks = staves * VOICES;
                        for (int track = 0; track < tracks; ++track) {
                              Element* e = segment->element(track);
                              if (el == e) {
                                    segment->setElement(track, 0);
                                    return;
                                    }
                              }
                        }
                  break;

            default:
                  break;
            }
      }

//-------------------------------------------------------------------
//   moveTicks
//    Also adjust endBarLine if measure len has changed. For this
//    diff == 0 cannot be optimized away
//-------------------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      setTick(tick() + diff);
      for (Segment* segment = first(); segment; segment = segment->next()) {
            if (segment->subtype() & (SegEndBarLine | SegTimeSigAnnounce))
                  segment->setTick(tick() + ticks());
            }
      }

//---------------------------------------------------------
//   removeStaves
//---------------------------------------------------------

void Measure::removeStaves(int sStaff, int eStaff)
      {
      for (Segment* s = _first; s; s = s->next()) {
            for (int staff = eStaff-1; staff >= sStaff; --staff) {
                  s->removeStaff(staff);
                  }
            }
      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int voice = e->voice();
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      foreach(Tuplet* e, _tuplets) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  int voice    = e->voice();
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      for (int i = 0; i < staves.size(); ++i)
            staves[i]->lines->setTrack(i * VOICES);
      }

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(int sStaff, int eStaff)
      {
      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff) {
                  int voice = e->voice();
                  staffIdx += eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
#if 0
      foreach(Beam* e, _beams) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff) {
                  int voice    = e->voice();
                  staffIdx += eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
#endif
      foreach(Tuplet* e, _tuplets) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff) {
                  int voice    = e->voice();
                  staffIdx += eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
      for (Segment* s = _first; s; s = s->next()) {
            for (int staff = sStaff; staff < eStaff; ++staff) {
                  s->insertStaff(staff);
                  }
            }
      for (int i = 0; i < staves.size(); ++i)
            staves[i]->lines->setTrack(i * VOICES);
      }

//---------------------------------------------------------
//   insertMStaff
//---------------------------------------------------------

void Measure::insertMStaff(MStaff* staff, int idx)
      {
      staves.insert(idx, staff);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setTrack(staffIdx * VOICES);
            }
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff* /*staff*/, int idx)
      {
      staves.removeAt(idx);
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setTrack(staffIdx * VOICES);
            }
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, int staffIdx)
      {
      for (Segment* s = _first; s; s = s->next())
            s->insertStaff(staffIdx);

      MStaff* ms = new MStaff;
      ms->lines  = new StaffLines(score());
      ms->lines->setParent(this);
      ms->lines->setTrack(staffIdx * VOICES);
      ms->distanceUp  = 0.0;
      ms->distanceDown = point(staffIdx == 0 ? score()->styleS(ST_systemDistance) : score()->styleS(ST_staffDistance));
      ms->lines->setVisible(!staff->invisible());
      insertMStaff(ms, staffIdx);
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

void Measure::read(XmlReader* r, int staffIdx)
      {
      if (staffIdx == 0)
            _len = Fraction(0, 1);
      for (int n = staves.size(); n <= staffIdx; ++n) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(n);
            s->lines     = new StaffLines(score());
            s->lines->setParent(this);
            s->lines->setTrack(n * VOICES);
            s->distanceUp = 0.0;
            s->distanceDown = point(n == 0 ? score()->styleS(ST_systemDistance) : score()->styleS(ST_staffDistance));
            s->lines->setVisible(!staff->invisible());
            staves.append(s);
            }
      int tck = -1;
      while (r->readAttribute()) {
            if (r->tag() == "tick")
                  tck = r->intValue();
            }
      if (tck >= 0) {
            tck = score()->fileDivision(tck);
            setTick(tck);
            }
      else
            setTick(score()->curTick);
      score()->curTick = tick();

      while (r->readElement()) {
            MString8 tag = r->tag();
            QString val;
            int i;
            qreal d;

            if (r->readInt("tick", &score()->curTick))
                  ;
            else if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(score()->curTrack);
                  barLine->setParent(this);     //??
                  barLine->read(r);
                  if ((score()->curTick != tick()) && (score()->curTick != (tick() + ticks()))) {
                        // this is a mid measure bar line
                        Segment* s = getSegment(SegBarLine, score()->curTick);
                        s->add(barLine);
                        }
                  else if (barLine->subtype() == START_REPEAT) {
                        Segment* s = getSegment(SegStartRepeatBarLine, score()->curTick);
                        s->add(barLine);
                        }
                  else {
                        setEndBarLineType(barLine->barLineType(), false, barLine->visible(), barLine->color());
                        delete barLine;
                        }
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->read(r, _tuplets, score()->slurs);

                  int track = chord->track();
                  Segment* ss = 0;
                  for (Segment* ps = first(SegChordRest); ps; ps = ps->next(SegChordRest)) {
                        if (ps->tick() >= score()->curTick)
                              break;
                        if (ps->element(track))
                              ss = ps;
                        }
                  Chord* pch = 0;       // previous chord
                  if (ss) {
                        ChordRest* cr = static_cast<ChordRest*>(ss->element(track));
                        if (cr && cr->type() == CHORD)
                              pch = static_cast<Chord*>(cr);
                        }

                  Segment* s = getSegment(chord, score()->curTick);

                  if (chord->tremolo() && chord->tremolo()->subtype() < 6) {
                        //
                        // old style tremolo found
                        //
                        Tremolo* tremolo = chord->tremolo();
                        TremoloType st;
                        switch(tremolo->subtype()) {
                              case 0: st = TREMOLO_R8;  break;
                              case 1: st = TREMOLO_R16; break;
                              case 2: st = TREMOLO_R32; break;
                              case 3: st = TREMOLO_C8;  break;
                              case 4: st = TREMOLO_C16; break;
                              case 5: st = TREMOLO_C32; break;
                              }
                        tremolo->setSubtype(st);
                        if (tremolo->twoNotes()) {
                              if (pch) {
                                    tremolo->setParent(pch);
                                    pch->setTremolo(tremolo);
                                    chord->setTremolo(0);
                                    }
                              score()->curTick += chord->ticks() / 2;
                              }
                        else {
                              tremolo->setParent(chord);
                              score()->curTick += chord->ticks();
                              }
                        }
                  else {
                        score()->curTick += chord->ticks();
                        }
                  s->add(chord);

                  Fraction nl(Fraction::fromTicks(score()->curTick - tick()));
                  if (nl > _len)
                        _len = nl;
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTrack(score()->curTrack);
                  breath->read(r);
                  Segment* s = getSegment(SegBreath, score()->curTick);
                  s->add(breath);
                  }
            else if (tag == "Note") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->readNote(r, _tuplets, score()->slurs);
                  Segment* s = getSegment(chord, score()->curTick);
                  s->add(chord);
                  score()->curTick += chord->ticks();
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setDurationType(TimeDuration::V_MEASURE);
                  rest->setDuration(timesig());
                  rest->setTrack(score()->curTrack);
                  rest->read(r, _tuplets, score()->slurs);
                  Segment* s = getSegment(rest, score()->curTick);
                  s->add(rest);
                  score()->curTick += rest->ticks();
                  Fraction nl(Fraction::fromTicks(score()->curTick - tick()));
                  if (nl > _len)
                        _len = nl;
                  }
            else if (tag == "endSpanner") {
                  int id = 0;
                  while (r->readAttribute()) {
                        if (r->tag() == "id")
                              id = r->intValue();
                        }
                  Spanner* e = score()->findSpanner(id);
                  if (e) {
                        Segment* s = getSegment(SegChordRest, score()->curTick);
                        e->setEndElement(s);
                        s->addSpannerBack(e);
                        if (e->type() == OTTAVA) {
                              Ottava* o = static_cast<Ottava*>(e);
                              int shift = o->pitchShift();
                              Staff* st = o->staff();
                              int tick1 = static_cast<Segment*>(o->startElement())->tick();
                              st->pitchOffsets().setPitchOffset(tick1, shift);
                              st->pitchOffsets().setPitchOffset(s->tick(), 0);
                              }
                        else if (e->type() == HAIRPIN) {
                              Hairpin* hp = static_cast<Hairpin*>(e);
                              score()->updateHairpin(hp);
                              }
                        }
                  }
            else if (tag == "HairPin"
               || tag == "Pedal"
               || tag == "Ottava"
               || tag == "Trill"
               || tag == "TextLine"
               || tag == "Volta") {
                  QString str = QString::fromUtf8((const char*)(tag.s()));
                  Spanner* sp = static_cast<Spanner*>(Element::name2Element(str, score()));
                  sp->setTrack(staffIdx * VOICES);
                  sp->read(r);
                  Segment* s = getSegment(SegChordRest, score()->curTick);
                  sp->setStartElement(s);
                  s->add(sp);
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(score()->curTrack);
                  rm->read(r, _tuplets, score()->slurs);
                  Segment* s = getSegment(SegChordRest, score()->curTick);
                  s->add(rm);
                  score()->curTick += ticks();
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(score()->curTrack);
                  clef->read(r);
                  Segment* s = getSegment(SegClef, score()->curTick);
                  s->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(score()->curTrack);
                  ts->read(r);
                  Segment* s = getSegment(SegTimeSig, score()->curTick);
                  s->add(ts);
                  _timesig = ts->getSig();
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(score()->curTrack);
                  ks->read(r);
                  Segment* s = getSegment(SegKeySig, score()->curTick);
                  s->add(ks);
                  }
            else if (tag == "Lyrics") {                           // obsolete
                  Lyrics* lyrics = new Lyrics(score());
                  lyrics->setTrack(score()->curTrack);
                  lyrics->read(r);
                  Segment* s    = getSegment(SegChordRest, score()->curTick);
                  ChordRest* cr = static_cast<ChordRest*>(s->element(lyrics->track()));
                  if (cr)
                        cr->add(lyrics);
                  }
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->setTrack(score()->curTrack);
                  t->read(r);

                  // TODO: measure numbers are generated and should no be
                  //       in msc file (discard?)

                  if (t->subtype() == TEXT_MEASURE_NUMBER) {
                        t->setTextStyle(TEXT_STYLE_MEASURE_NUMBER);
                        t->setTrack(-1);
                        _noText = t;
                        }
                  }

            //----------------------------------------------------
            // Annotation

            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(score()->curTrack);
                  dyn->read(r);
                  dyn->resetType(); // for backward compatibility
                  Segment* s = getSegment(SegChordRest, score()->curTick);
                  s->add(dyn);
                  }
            else if (tag == "Harmony"
               || tag == "FretDiagram"
               || tag == "Symbol"
               || tag == "Tempo"
               || tag == "StaffText"
               || tag == "InstrumentChange"
               || tag == "Marker"
               || tag == "Jump"
               || tag == "StaffState"
               ) {
                  QString str = QString::fromUtf8((const char*)tag.s());
                  Element* el = Element::name2Element(str, score());
                  el->setTrack(score()->curTrack);
                  el->read(r);
                  Segment* s = getSegment(SegChordRest, score()->curTick);
                  s->add(el);
                  }
            else if (tag == "Image") {
#if 0
                  // look ahead for image type
                  QString path;
                  XmlReader* ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  QString s(path.toLower());
                  if (s.endsWith(".svg"))
                        image = new SvgImage(score());
                  else if (s.endsWith(".jpg")
                     || s.endsWith(".png")
                     || s.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  if (image) {
                        image->setTrack(score()->curTrack);
                        image->read(r);
                        Segment* s = getSegment(SegChordRest, score()->curTick);
                        s->add(image);
                        }
#endif
                  }

            //----------------------------------------------------
            else if (r->readReal("stretch", &_userStretch))
                  ;
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(r);
                  add(lb);
                  }
            else if (r->readInt("noOffset", &_noOffset))
                  ;
            else if (r->readBool("irregular", &_irregular))
                  ;
            else if (r->readBool("breakMultiMeasureRest", &_breakMultiMeasureRest))
                  ;
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->setTrack(score()->curTrack);
                  tuplet->setTick(score()->curTick);
                  tuplet->setTrack(score()->curTrack);
                  tuplet->setParent(this);
                  tuplet->read(r, _tuplets, score()->slurs);
                  add(tuplet);
                  }
            else if (tag == "startRepeat") {
                  _repeatFlags |= RepeatStart;
                  r->read();
                  }
            else if (r->readInt("endRepeat", &i)) {
                  _repeatCount = i;
                  _repeatFlags |= RepeatEnd;
                  r->read();
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(score());
                  slur->setTrack(score()->curTrack);
                  slur->read(r);
                  score()->slurs.append(slur);
                  }
            else if (r->readReal("vspacerUp", &d)) {
                  if (staves[staffIdx]->_vspacerUp == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSubtype(SPACER_UP);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerUp->setSpace(Spatium(d));
                  }
            else if (r->readReal("vspacerDown", &d)) {
                  if (staves[staffIdx]->_vspacerDown == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setSubtype(SPACER_DOWN);
                        spacer->setTrack(staffIdx * VOICES);
                        add(spacer);
                        }
                  staves[staffIdx]->_vspacerDown->setSpace(Spatium(d));
                  }
            else if (tag == "visible")
                  staves[staffIdx]->_visible = val.toInt();
            else if (tag == "slashStyle")
                  staves[staffIdx]->_slashStyle = val.toInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(score()->curTrack);
                  beam->read(r);
                  beam->setParent(0);
                  score()->beams().append(beam);
                  }
            else
                  r->unknown();
            }
      if (staffIdx == 0) {
            Segment* s = last();
            if (s && s->subtype() == SegBarLine) {
                  BarLine* b = static_cast<BarLine*>(s->element(0));
                  setEndBarLineType(b->barLineType(), false, b->visible(), b->color());
                  s->remove(b);
                  delete b;
                  }
            }
      }

//---------------------------------------------------------
//   visible
//---------------------------------------------------------

bool Measure::visible(int staffIdx) const
      {
      if (system() && !system()->staff(staffIdx)->show())
            return false;
      return score()->staff(staffIdx)->show() && staves[staffIdx]->_visible;
      }

//---------------------------------------------------------
//   slashStyle
//---------------------------------------------------------

bool Measure::slashStyle(int staffIdx) const
      {
      return score()->staff(staffIdx)->slashStyle() || staves[staffIdx]->_slashStyle;
      }

//---------------------------------------------------------
//   scanElements
//---------------------------------------------------------

void Measure::scanElements(void* data, void (*func)(void*, Element*))
      {
      MeasureBase::scanElements(data, func);
      func(data, this);                         // to make measure clickable

      int nstaves = score()->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            if (!visible(staffIdx))
                  continue;
            MStaff* ms = staves[staffIdx];
            if (ms->lines)
                  func(data, ms->lines);
            if (ms->_vspacerUp)
                  func(data, ms->_vspacerUp);
            if (ms->_vspacerDown)
                  func(data, ms->_vspacerDown);
            }

      int tracks = nstaves * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!visible(staffIdx))
                        continue;
                  }
            for (int track = 0; track < tracks; ++track) {
                  if (!visible(track/VOICES)) {
                        track += VOICES - 1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  e->scanElements(data, func);
                  }
            foreach(Spanner* e, s->spannerFor())
                  e->scanElements(data,  func);
            foreach(Element* e, s->annotations()) {
#if 0
                  if (e->type() == TEMPO_TEXT) {
                        QString s = static_cast<TempoText*>(e)->getText();
                        QRectF r(e->abbox());
                        printf("scan %f %f %f %f<%s>\n", r.x(), r.y(), r.width(), r.height(), qPrintable(s));
                        }
#endif
                  e->scanElements(data,  func);
                  }
            }
      foreach(Tuplet* tuplet, _tuplets) {
            if (visible(tuplet->staffIdx()))
                  func(data, tuplet);
            }

      if (noText())
            func(data, noText());
      }

//---------------------------------------------------------
//   setStartRepeatBarLine
//    return true if bar line type changed
//---------------------------------------------------------

bool Measure::setStartRepeatBarLine(bool val)
      {
      bool changed = false;
      const QList<Part*>* pl = score()->parts();

      foreach(const Part* part, *pl) {
            BarLine* bl  = 0;
            Staff* staff = part->staff(0);
            int track    = staff->idx() * VOICES;
            bool found   = false;
            for (Segment* s = first(); s; s = s->next()) {
                  if (s->subtype() != SegStartRepeatBarLine)
                        continue;
                  if (s->element(track)) {
                        found = true;
                        if (!val) {
                              delete s->element(track);
                              s->setElement(track, 0);
                              changed = true;
                              break;
                              }
                        else
                              bl = (BarLine*)s->element(track);
                        }
                  }
            if (!found && val) {
                  bl = new BarLine(score());
                  bl->setTrack(track);
                  bl->setBarLineType(START_REPEAT);
                  // bl->setGenerated(true);
                  Segment* seg = getSegment(SegStartRepeatBarLine, tick());
                  seg->add(bl);
                  changed = true;
                  }
            }
      return changed;
      }

//---------------------------------------------------------
//   createEndBarLines
//    actually create or modify barlines
//---------------------------------------------------------

bool Measure::createEndBarLines()
      {
      bool changed = false;

      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            Segment* s;
            BarLine* bl  = 0;
            Staff* staff = score()->staff(staffIdx);
            for (s = first(); s; s = s->next()) {
                  if (s->subtype() == SegEndBarLine) {
                        bl = (BarLine*)(s->element(staffIdx * VOICES));
                        break;
                        }
                  }
            int span = staff->barLineSpan();
            if (span == 0) {
                  if (bl) {
                        delete bl;
                        bl = 0;
                        s->setElement(staffIdx * VOICES, 0);
                        }
                  }
            else {
                  if (bl == 0) {
                        bl = new BarLine(score());
                        bl->setTrack(staffIdx * VOICES);
                        Segment* seg = getSegment(SegEndBarLine, tick() + ticks());
                        seg->add(bl);
                        changed = true;
                        bl->layout();
                        }
                  }
            if (bl) {
                  bl->setMag(staff->mag());
                  BarLineType et = _multiMeasure > 0 ? _mmEndBarLineType : _endBarLineType;
                  if (bl->subtype() != et) {
                        bl->setBarLineType(et);
                        changed = true;
                        }
                  bl->setVisible(_endBarLineVisible);
                  bl->setColor(_endBarLineColor);
                  bl->setGenerated(_endBarLineGenerated);
                  span = staff->barLineSpan();
                  if (system() && !system()->staff(staffIdx + span - 1)->show()) {
                        //
                        // if the barline ends on an invisible staff
                        // find last visible staff in barline
                        //
                        for (int j = staffIdx + bl->span() - 2; j >= staffIdx; --j) {
                              if (system()->staff(j)->show()) {
                                    bl->setSpan(j - staffIdx + 1);
                                    break;
                                    }
                              }
                        }
                  staffIdx += (span - 1);
                  }
            }

      return changed;
      }

//---------------------------------------------------------
//   setEndBarLineType
//---------------------------------------------------------

void Measure::setEndBarLineType(BarLineType val, bool g, bool visible, Color color)
      {
      _endBarLineType      = val;
      _endBarLineGenerated = g;
      _endBarLineVisible   = visible;
      _endBarLineColor     = color;
      }

//---------------------------------------------------------
//   setRepeatFlags
//---------------------------------------------------------

void Measure::setRepeatFlags(int val)
      {
      _repeatFlags = val;
      }

//---------------------------------------------------------
//   sortStaves
//---------------------------------------------------------

void Measure::sortStaves(QList<int>& dst)
      {
      QList<MStaff*> ms;
      foreach(int idx, dst)
            ms.push_back(staves[idx]);
      staves = ms;

      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setTrack(staffIdx * VOICES);
            }
      for (Segment* s = first(); s; s = s->next())
            s->sortStaves(dst);

      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int voice    = e->voice();
            int staffIdx = e->staffIdx();
            int idx = dst.indexOf(staffIdx);
            e->setTrack(idx * VOICES + voice);
            }
      }

//---------------------------------------------------------
//   exchangeVoice
//---------------------------------------------------------

void Measure::exchangeVoice(int v1, int v2, int staffIdx1, int staffIdx2)
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != SegChordRest)
                  continue;
            for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++ staffIdx) {
                  int strack = staffIdx * VOICES + v1;
                  int dtrack = staffIdx * VOICES + v2;
                  if (s->element(strack) && s->element(dtrack)) {
                        Element* e = s->element(strack);
                        e->setTrack(dtrack);
                        s->element(dtrack)->setTrack(strack);
                        s->setElement(strack, s->element(dtrack));
                        s->setElement(dtrack, e);
                        }
                  else if (s->element(strack) && !s->element(dtrack)) {
                        s->setElement(dtrack, s->element(strack));
                        s->element(dtrack)->setTrack(dtrack);
                        if(v1 != 0)
                              s->setElement(strack, 0);
                        else {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(strack));
                              Rest* r = new Rest(score(), cr->duration());
                              r->setTrack(strack);
                              s->setElement(strack, r);
                              }
                        }
                  else if (!s->element(strack) && s->element(dtrack)) {
                        s->setElement(strack, s->element(dtrack));
                        s->element(strack)->setTrack(strack);
                        if(v2 != 0)
                              s->setElement(dtrack, 0);
                        else {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(dtrack));
                              Rest* r = new Rest(score(), cr->duration());
                              r->setTrack(dtrack);
                              s->setElement(dtrack, r);
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   checkMultiVoices
//---------------------------------------------------------

/**
 Check for more than on voice in this measure and staff and
 set MStaff->hasVoices
*/

void Measure::checkMultiVoices(int staffIdx)
      {
      int strack = staffIdx * VOICES + 1;
      int etrack = staffIdx * VOICES + VOICES;
      staves[staffIdx]->hasVoices = false;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != SegChordRest)
                  continue;
            for (int track = strack; track < etrack; ++track) {
                  if (s->element(track)) {
                        staves[staffIdx]->hasVoices = true;
                        return;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   hasVoice
//---------------------------------------------------------

bool Measure::hasVoice(int track) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != SegChordRest)
                  continue;
            if (s->element(track))
                  return true;
            }
      return false;
      }

//---------------------------------------------------------
//   isMeasureRest
//---------------------------------------------------------

/**
 Check if the measure is filled by a full-measure rest or full of rests on
 this staff. If staff is -1, then check for all staves
*/

bool Measure::isMeasureRest(int staffIdx)
      {
      int strack;
      int etrack;
      if (staffIdx < 0) {
            strack = 0;
            etrack = score()->nstaves() * VOICES;
            }
      else {
            strack = staffIdx * VOICES;
            etrack = staffIdx * VOICES + VOICES;
            }
      for (Segment* s = first(SegChordRest); s; s = s->next(SegChordRest)) {
            for (int track = strack; track < etrack; ++track) {
                  Element* e = s->element(track);
                  if (e && e->type() != REST)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   isFullMeasureRest
//    Check for an empty measure, filled with full measure
//    rests.
//---------------------------------------------------------

bool Measure::isFullMeasureRest()
      {
      int strack = 0;
      int etrack = score()->nstaves() * VOICES;

      Segment* s = first(SegChordRest);
      for (int track = strack; track < etrack; ++track) {
            Element* e = s->element(track);
            if (e) {
                  if (e->type() != REST)
                        return false;
                  Rest* rest = static_cast<Rest*>(e);
                  if (rest->durationType().type() != TimeDuration::V_MEASURE)
                        return false;
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   userDistanceDown
//---------------------------------------------------------

Spatium Measure::userDistanceDown(int i) const
      {
      return staves[i]->_vspacerDown ? staves[i]->_vspacerDown->getSpace() : Spatium(0);
      }

//---------------------------------------------------------
//   userDistanceUp
//---------------------------------------------------------

Spatium Measure::userDistanceUp(int i) const
      {
      return staves[i]->_vspacerUp ? staves[i]->_vspacerUp->getSpace() : Spatium(0);
      }

//---------------------------------------------------------
//   isEmpty
//---------------------------------------------------------

bool Measure::isEmpty() const
      {
      if (_irregular)
            return false;
      int n = 0;
      const Segment* s = _first;
      for (int i = 0; i < _size; ++i) {
            if (s->subtype() == SegChordRest) {
                  for (int track = 0; track < staves.size()*VOICES; ++track) {
                        if (s->element(track) && s->element(track)->type() != REST)
                              return false;
                        }
                  if (n > 0)
                        return false;
                  ++n;
                  }
            s = s->next();
            }
      return true;
      }

//---------------------------------------------------------
//   firstCRSegment
//---------------------------------------------------------

Segment* Measure::firstCRSegment() const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() == SegChordRest)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

Segment* Measure::first(SegmentTypes types) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() & types)
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   Space::max
//---------------------------------------------------------

void Space::max(const Space& s)
      {
      if (s._lw > _lw)
            _lw = s._lw;
      if (s._rw > _rw)
            _rw = s._rw;
      }

//---------------------------------------------------------
//   Spring
//---------------------------------------------------------

struct Spring {
      int seg;
      qreal stretch;
      qreal fix;
      Spring(int i, qreal s, qreal f) : seg(i), stretch(s), fix(f) {}
      };

typedef std::multimap<qreal, Spring, std::less<qreal> > SpringMap;
typedef SpringMap::iterator iSpring;

//---------------------------------------------------------
//   sff
//    compute 1/Force for a given Extend
//---------------------------------------------------------

static qreal sff(qreal x, qreal xMin, SpringMap& springs)
      {
      if (x <= xMin)
            return 0.0;
      iSpring i = springs.begin();
      qreal c  = i->second.stretch;
      if (c == 0.0)           //DEBUG
            c = 1.1;
      qreal f = 0.0;
      for (; i != springs.end();) {
            xMin -= i->second.fix;
            f = (x - xMin) / c;
            ++i;
            if (i == springs.end() || f <= i->first)
                  break;
            c += i->second.stretch;
            }
      return f;
      }

//-----------------------------------------------------------------------------
///   \brief main layout routine for note spacing
///   Return width of measure (in MeasureWidth), taking into account \a stretch.
///   In the layout process this method is called twice, first with stretch==1
///   to find out the minimal width of the measure.
//-----------------------------------------------------------------------------

void Measure::layoutX(qreal stretch)
      {
      if (!_dirty && (stretch == 1.0))
            return;
      int nstaves     = _score->nstaves();
      int segs        = _size;

      if (nstaves == 0 || segs == 0) {
            _mw = MeasureWidth(1.0, 0.0);
            _dirty = false;
            return;
            }

      qreal _spatium           = spatium();
      int tracks                = nstaves * VOICES;
      qreal clefKeyRightMargin = score()->styleS(ST_clefKeyRightMargin).val() * _spatium;

      qreal rest[nstaves];    // fixed space needed from previous segment
      memset(rest, 0, nstaves * sizeof(qreal));
      //--------tick table for segments
      int ticksList[segs];
      memset(ticksList, 0, segs * sizeof(int));

      qreal xpos[segs+1];
      int types[segs];
      qreal width[segs];

      int segmentIdx  = 0;
      qreal x        = 0.0;
      int minTick     = 100000;
      int ntick       = tick() + ticks();   // position of next measure

      for (const Segment* s = first(); s; s = s->next(), ++segmentIdx) {
            types[segmentIdx] = s->subtype();
            bool rest2[nstaves+1];
            qreal segmentWidth    = 0.0;
            qreal minDistance     = 0.0;
            qreal stretchDistance = 0.0;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  Space space;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  if (s->subtype() & (SegChordRest | SegGrace)) {
                        qreal llw = 0.0;
                        qreal rrw = 0.0;
                        Lyrics* lyrics = 0;
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (s == first() || s->prev()->subtype() == SegStartRepeatBarLine) {
                                    qreal sp       = score()->styleS(ST_barNoteDistance).val() * _spatium;
                                    minDistance     = sp * .3;
                                    stretchDistance = sp * .7;
                                    }
                              else {
                                    int pt = s->prev()->subtype();
                                    if (! (pt & (SegChordRest | SegGrace))) {
                                          // distance to first chord/rest in measure
                                          minDistance = clefKeyRightMargin;
                                          }
                                    else
                                          minDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;
                                    if (s->subtype() == SegGrace)
                                          minDistance *= score()->styleD(ST_graceNoteMag);
                                    }
                              cr->layout();
                              space.max(cr->space());
                              foreach(Lyrics* l, cr->lyricsList()) {
                                    if (!l)
                                          continue;
                                    l->layout();
                                    lyrics = l;
                                    QRectF b(l->bbox().translated(l->pos()));
                                    // qreal lw = l->bbox().width() * .5;
                                    qreal lw = -b.left();
                                    if (lw > llw)
                                          llw = lw;
                                    if (lw > rrw)
                                          rrw = lw;
                                    qreal rw = b.right();
                                    if (rw > rrw)
                                          rrw = rw;
                                    }
                              }
                        if (lyrics) {
                              found = true;
//                              qreal y = lyrics->ipos().y() + lyrics->lineHeight()
//                                 + point(score()->styleS(ST_lyricsMinBottomDistance));
                              qreal y = lyrics->ipos().y() + point(score()->styleS(ST_lyricsMinBottomDistance));
                              if (y > staves[staffIdx]->distanceDown)
                                 staves[staffIdx]->distanceDown = y;
                              }
                        space.max(Space(llw, rrw));
                        }
                  else {
                        Element* e = s->element(track);
                        if (s->subtype() == SegStartRepeatBarLine) {
                              minDistance = .5 * _spatium;
                              }
                        else if ((s->subtype() == SegEndBarLine) && segmentIdx) {
                              if (s->prev()->subtype() == SegClef)
                                    minDistance = score()->styleS(ST_clefBarlineDistance).val() * _spatium;
                              else
                                    stretchDistance = score()->styleS(ST_noteBarDistance).val() * _spatium;
                              if (e == 0) {
                                    // look for barline
                                    for (int i = track - VOICES; i >= 0; i -= VOICES) {
                                          e = s->element(i);
                                          if (e)
                                                break;
                                          }
                                    }
                              }
                        if (e) {
                              found = true;
                              e->layout();
                              Space sp = e->space();
                              if ((e->type() == CLEF) && (s != first())) {
                                    sp.rLw() += sp.rw();
                                    sp.rRw() = 0.0;
                                    }
                              space.max(sp);
                              }
                        }
                  if (found) {
                        qreal sp = minDistance + rest[staffIdx] + stretchDistance;
                        if (space.lw() > stretchDistance)
                              sp += (space.lw() - stretchDistance);
                        rest[staffIdx]  = space.rw();
                        rest2[staffIdx] = false;
                        segmentWidth    = qMax(segmentWidth, sp);
                        }
                  else
                        rest2[staffIdx] = true;
                  }
            x += segmentWidth;
            xpos[segmentIdx]  = x;
            if (segmentIdx)
                  width[segmentIdx-1] = segmentWidth;
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (rest2[staffIdx])
                        rest[staffIdx] -= segmentWidth;
                  }
            if ((s->subtype() == SegChordRest)) {
                  const Segment* nseg = s;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->subtype() == SegChordRest)
                              break;
                        }
                  int nticks = (nseg ? nseg->tick() : ntick) - s->tick();
                  if (nticks == 0) {
                        // warn
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        }
                  ticksList[segmentIdx] = nticks;
                  }
            else
                  ticksList[segmentIdx] = 0;
            }
      qreal segmentWidth = 0.0;
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx)
            segmentWidth = qMax(segmentWidth, rest[staffIdx]);
      xpos[segmentIdx]    = x + segmentWidth;
      width[segmentIdx-1] = segmentWidth;

      if (stretch == 1.0) {
            // printf("this is pass 1\n");
            _mw = MeasureWidth(xpos[segs], 0.0);
            _dirty = false;
            return;
            }

      //---------------------------------------------------
      // compute stretches:
      //---------------------------------------------------

      SpringMap springs;
      qreal stretchList[segs];
      qreal stretchSum = 0.0;
      stretchList[0]    = 0.0;

      qreal minimum = xpos[0];
      for (int i = 0; i < segs; ++i) {
            qreal str = 1.0;
            qreal d;
            qreal w = width[i];

            int t = ticksList[i];
            if (t) {
                  if (minTick > 0)
                        str += .6 * log2(qreal(t) / qreal(minTick));
                  stretchList[i] = str;
                  d = w / str;
                  }
            else {
                  if (ticksList[i]) {
                        w = 0.0;
                        }
                  stretchList[i] = 0.0;   // dont stretch timeSig and key
                  d = 100000000.0;        // CHECK
                  }
            stretchSum += stretchList[i];
            minimum    += w;
            springs.insert(std::pair<qreal, Spring>(d, Spring(i, stretchList[i], w)));
            }

      //---------------------------------------------------
      //    distribute stretch to segments
      //---------------------------------------------------

      qreal force = sff(stretch, minimum, springs);

      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            qreal stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }
      x = xpos[0];
      for (int i = 1; i <= segs; ++i) {
            x += width[i-1];
            xpos[i] = x;
            }

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      int seg = 0;
      for (Segment* s = first(); s; s = s->next(), ++seg) {
            s->setPos(xpos[seg], 0.0);
            for (int track = 0; track < tracks; ++track) {
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  ElementType t = e->type();
                  if (t == REST) {
                        Rest* rest = static_cast<Rest*>(e);
                        if (_multiMeasure > 0) {
                              if ((track % VOICES) == 0) {
                                    Segment* ls = last();
                                    qreal eblw = 0.0;
                                    int t = (track / VOICES) * VOICES;
                                    if (ls->subtype() == SegEndBarLine) {
                                          Element* e = ls->element(t);
                                          if (!e)
                                                e = ls->element(0);
                                          eblw = e ? e->width() : 0.0;
                                          }
                                    if (seg == 1)
                                          rest->setMMWidth(xpos[segs] - 2 * s->x() - eblw);
                                    else
                                          rest->setMMWidth(xpos[segs] - s->x() - point(score()->styleS(ST_barNoteDistance)) - eblw);
                                    e->rxpos() = 0.0;
                                    }
                              }
                        else if (rest->durationType() == TimeDuration::V_MEASURE) {
                              qreal x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                              qreal w;
                              if ((segs > 2) && types[segs-2] == SegClef)
                                    w  = xpos[segs-2] - x1;
                              else
                                    w  = xpos[segs-1] - x1;
                              e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                              }
                        }
                  else if (t == REPEAT_MEASURE) {
                        qreal x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                        qreal w  = xpos[segs-1] - x1;
                        e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                        }
                  else if (t == CHORD) {
                        Chord* chord = static_cast<Chord*>(e);
                        chord->layout2();
                        }
                  else if ((t == CLEF) && (s != first())) {
                        qreal w   = 0.0;
                        if (types[seg+1] != SegChordRest)
                              w = xpos[seg+1] - xpos[seg];
                        qreal m  = score()->styleS(ST_clefBarlineDistance).val() * _spatium;
                        e->rxpos() = w - e->width() - m;
                        }
                  else {
                        e->setPos(-e->bbox().x(), 0.0);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutStage1
//---------------------------------------------------------

void Measure::layoutStage1()
      {
      setDirty();
      for (int staffIdx = 0; staffIdx < score()->nstaves(); ++staffIdx) {
            KeySigEvent key = score()->staff(staffIdx)->keymap()->key(tick());

            setBreakMMRest(false);
            if (score()->styleB(ST_createMultiMeasureRests)) {
                  if ((repeatFlags() & RepeatStart) || (prevMeasure() && (prevMeasure()->repeatFlags() & RepeatEnd)))
                        setBreakMMRest(true);
                  else if (!breakMMRest()) {
                        for (Segment* s = first(); s; s = s->next()) {
                              foreach(Element* e, s->annotations()) {
                                    if (
                                       ((e->type() == TEXT) && (e->subtype() == TEXT_REHEARSAL_MARK))
                                       || (e->type() == TEMPO_TEXT)
                                       ) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              foreach(Spanner* sp, s->spannerFor()) {
                                    if (sp->type() == VOLTA) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              foreach(Spanner* sp, s->spannerBack()) {
                                    if (sp->type() == VOLTA) {
                                          setBreakMMRest(true);
                                          break;
                                          }
                                    }
                              if (breakMMRest())      // optimize
                                    break;
                              }
                        }
                  }

            int track = staffIdx * VOICES;

            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);

                  if (segment->subtype() == SegKeySig
                     || segment->subtype() == SegStartRepeatBarLine
                     || segment->subtype() == SegTimeSig) {
                        if (e && !e->generated())
                              setBreakMMRest(true);
                        }

                  if (segment->subtype() & (SegChordRest | SegGrace))
                        layoutChords0(segment, staffIdx * VOICES);
                  }
            }
      }

