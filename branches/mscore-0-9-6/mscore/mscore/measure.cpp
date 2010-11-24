//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#include "measure.h"
#include "segment.h"
#include "note.h"
#include "rest.h"
#include "chord.h"
#include "xml.h"
#include "score.h"
#include "clef.h"
#include "key.h"
#include "dynamics.h"
#include "slur.h"
#include "al/sig.h"
#include "beam.h"
#include "tuplet.h"
#include "system.h"
#include "undo.h"
#include "hairpin.h"
#include "text.h"
#include "select.h"
#include "staff.h"
#include "part.h"
#include "style.h"
#include "bracket.h"
#include "ottava.h"
#include "trill.h"
#include "pedal.h"
#include "timesig.h"
#include "barline.h"
#include "layoutbreak.h"
#include "page.h"
#include "lyrics.h"
#include "measureproperties.h"
#include "scoreview.h"
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

//---------------------------------------------------------
//   MStaff
//---------------------------------------------------------

MStaff::MStaff()
      {
      distance     = .0;
      lines        = 0;
      hasVoices    = false;
      _vspacer     = 0;
      _visible     = true;
      _slashStyle  = false;
      }

MStaff::~MStaff()
      {
      delete lines;
      delete _vspacer;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : MeasureBase(s)
      {
      _tickLen  = -1;
      sigSerial = 0;

      _first   = 0;
      _last    = 0;
      _size    = 0;

      int n = _score->nstaves();
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(staffIdx);
            s->lines     = new StaffLines(score());
            s->lines->setTrack(staffIdx * VOICES);
            s->lines->setLines(staff->lines());
            s->lines->setParent(this);
            s->lines->setVisible(!staff->invisible());
            staves.push_back(s);
            }

      _userStretch = 1.0;     // ::style->measureSpacing;
      _lineBreak   = false;
      _pageBreak   = false;
      _no          = 0;

      _irregular             = false;
      _breakMultiMeasureRest = false;
      _breakMMRest           = false;
      _multiMeasure          = 0;

      _repeatCount = 2;
      _repeatFlags = 0;
      _noOffset    = 0;
      _noText      = 0;
      _endBarLineType = NORMAL_BAR;
      _mmEndBarLineType = NORMAL_BAR;
      _endBarLineGenerated = true;
      _endBarLineVisible   = true;
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
      /*foreach(Tuplet* t, _tuplets)
            delete t;*/
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
//   dump
//---------------------------------------------------------

/**
 Debug only.
*/

void Measure::dump() const
      {
      printf("dump measure:\n");
      }

//---------------------------------------------------------
//   tickLen
//---------------------------------------------------------

int Measure::tickLen() const
      {
      unsigned ss = _score->sigmap()->serial();
      if (_tickLen == -1 || sigSerial != ss) {
            _tickLen = _score->sigmap()->ticksMeasure(tick());
            sigSerial = ss;
            }
      return _tickLen;
      }

//---------------------------------------------------------
//   setTick
//---------------------------------------------------------

void Measure::setTick(int t)
      {
      Element::setTick(t);
      _tickLen = -1;          // invalidate cached len
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Measure::remove(Segment* el)
      {
      // debug:
      bool found = false;
      for (Segment* s = _first; s; s = s->next()) {
            if (el == s) {
                  found = true;
                  break;
                  }
            }
      if (!found) {
            printf("Measure::remove segment: not found %p\n", el);
            abort();
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
      double x;
      };

//---------------------------------------------------------
//   layoutChords0
//    only called from layout0
//    computes note lines and accidentals
//---------------------------------------------------------

void Measure::layoutChords0(Segment* segment, int startTrack, char* tversatz)
      {
      int staffIdx     = startTrack/VOICES;
      Staff* staff     = score()->staff(staffIdx);
      double staffMag  = staff->mag();
      Drumset* drumset = 0;

      if (staff->part()->useDrumset())
            drumset = staff->part()->drumset();

      int endTrack = startTrack + VOICES;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e)
                 continue;
            ChordRest* cr = static_cast<ChordRest*>(e);
            double m = staffMag;
            if (cr->small())
                  m *= score()->styleD(ST_smallNoteMag);

            if (cr->type() == CHORD) {
                  Chord* chord = static_cast<Chord*>(e);
                  if (chord->noteType() != NOTE_NORMAL)
                        m *= score()->styleD(ST_graceNoteMag);
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
                              tversatz[line] = ((tpc + 1) / 7) - 2;
                              continue;
                              }

                        if (drumset) {
                              int pitch = note->pitch();
                              if (!drumset->isValid(pitch)) {
                                    printf("unmapped drum note %d\n", pitch);
                                    }
                              else {
                                    note->setHeadGroup(drumset->noteHead(pitch));
                                    note->setLine(drumset->line(pitch));
                                    continue;
                                    }
                              }
                        note->layout1(tversatz);
                        }
                  chord->computeUp();
                  }
            cr->setMag(m);
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
                  if (chord->staff()->part()->useDrumset())
                        drumset = chord->staff()->part()->drumset();

                  foreach(Note* note1, chord->notes()) {
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

//                        if (note1->userAccidental())
//                              ;
//                        else  {
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
                                                      printf("bad accidental\n");
                                                      return 0;
                                                }
                                          }
                                    tversatz[line] = accVal;
                                    }
                              else {
                                    if (note == note1)
                                          return 0;
                                    }
//                              }
                        }
                  }
            }
      printf("note not found\n");
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
                  if (chord->staff()->part()->useDrumset())
                        drumset = chord->staff()->part()->drumset();

                  foreach(Note* note1, chord->notes()) {
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
      printf("note not found\n");
      return 0;
      }

//---------------------------------------------------------
//   Measure::layout
//---------------------------------------------------------

/**
 Layout measure; must fit into  \a width.

 Note: minWidth = width - stretch
*/

void Measure::layout(double width)
      {
      int nstaves = _score->nstaves();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            staves[staffIdx]->distance = 0.0;
            if (staves[staffIdx]->lines)
                  staves[staffIdx]->lines->setMag(score()->staff(staffIdx)->mag());
            }

      // height of boundingRect will be set in system->layout2()
      // keep old value for relayout

      setbbox(QRectF(0.0, 0.0, width, height()));
      layoutX(width);
      }

//---------------------------------------------------------
//   tick2pos
//---------------------------------------------------------

double Measure::tick2pos(int tck) const
      {
      Segment* s;
      double x1 = 0;
      double x2 = 0;
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
            tick2 = tick() + _score->sigmap()->ticksMeasure(tick());
            }
      double x = 0;
      if (tick2 > tick1) {
            double dx = x2 - x1;
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

      double _spatium = spatium();
      for (int staffIdx = 0; staffIdx < staves.size(); ++staffIdx) {
            for (Segment* s = first(); s; s = s->next()) {
                  LyricsList* ll = s->lyricsList(staffIdx);
                  if (!ll)
                        continue;
                  foreach(Lyrics* l, *ll) {
                        if (!l)
                              continue;
                        system()->layoutLyrics(l, s, staffIdx);
                        }
                  }
            if (staves[staffIdx]->_vspacer) {
                  staves[staffIdx]->_vspacer->layout();
                  double y = system()->staff(staffIdx)->y();
                  staves[staffIdx]->_vspacer->setPos(_spatium * .5, y + 4 * _spatium);
                  }
            }

      foreach(const MStaff* ms, staves)
            ms->lines->setWidth(width());

      foreach(Element* element, _el)
            element->layout();

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
      int tracks = _score->nstaves() * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = 0; track < tracks; ++track) {
                  Element* el = s->element(track);
                  if (el) {
                        if (el->type() == CHORD) {
                              Chord* a = static_cast<Chord*>(el);
                              foreach(Note* n, a->notes()) {
                                    Tie* tie = n->tieFor();
                                    if (tie)
                                          tie->layout();
                                    }
                              a->layoutArticulations();     // DEBUG
                              }
                        else if (el->type() == BAR_LINE)
                              el->layout();
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
#if 0
      printf("segment at %d type %d not found\n", t, st);
      for (Segment* s = first(); s; s = s->next())
            printf("  %d: %d\n", s->tick(), s->subtype());
#endif
      return 0;
      }

//---------------------------------------------------------
//   createSegment
//---------------------------------------------------------

Segment* Measure::createSegment(SegmentType st, int tick)
      {
      Segment* newSegment = new Segment(this, tick);
      newSegment->setSubtype(st);
      return newSegment;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e)
      {
      SegmentType st;
      if ((e->type() == CHORD) && (((Chord*)e)->noteType() != NOTE_NORMAL)) {
            Segment* s = findSegment(SegGrace, e->tick());
            if (s) {
                  if (s->element(e->track())) {
                        s = s->next();
                        if (s && s->subtype() == SegGrace && !s->element(e->track()))
                              return s;
                        }
                  else
                        return s;
                  }
            s = createSegment(SegGrace, e->tick());
            add(s);
            return s;
            }
      else
            st = Segment::segmentType(e->type());
      return getSegment(st, e->tick());
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
            s = createSegment(st, t);
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
// printf("Measure::getSegment(st=%d, t=%d, gl=%d)\n", st, t, gl);
      if (st != SegChordRest && st != SegGrace) {
            printf("Measure::getSegment(st=%d, t=%d, gl=%d): incorrect segment type\n", st, t, gl);
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

//      printf("s=%p sCr=%p nGr=%d\n", s, sCr, nGraces);
//      printf("segment list\n");
//      for (Segment* s = first(); s; s = s->next())
//            printf("  %d: %d\n", s->tick(), s->subtype());

      if (gl == 0) {
            if (sCr)
                  return sCr;
            // no SegChordRest at tick = t, must create it
//            printf("creating SegChordRest at tick=%d\n", t);
            s = createSegment(SegChordRest, t);
            add(s);
            return s;
            }

      if (gl > 0) {
            if (gl <= nGraces) {
// printf("grace segment %d already exist, returning it\n", gl);
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
printf("creating SegGrace at tick=%d and level=%d\n", t, gl);
                  Segment* prevs = 0; // last segment inserted
                  // insert the first grace segment
                  if (nGraces == 0) {
                        ++nGraces;
                        s = createSegment(SegGrace, t);
printf("... creating SegGrace %p at tick=%d and level=%d\n", s, t, nGraces);
                        add(s);
printf("   <%d - %d - %d>\n", s->prev() ? s->prev()->tick() : -1, s->tick(),
                   s->next() ? s->next()->tick() : -1);
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
                        s = createSegment(SegGrace, t);
// printf("... creating SegGrace %p at tick=%d and level=%d\n", s, t, nGraces);
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
      int t = el->tick();
      ElementType type = el->type();


//      if (debugMode)
//            printf("measure %p(%d): add %s %p\n", this, _no, el->name(), el);

      switch (type) {
            case SPACER:
                  staves[el->staffIdx()]->_vspacer = static_cast<Spacer*>(el);
                  break;
            case SEGMENT:
                  {
                  Segment* seg = static_cast<Segment*>(el);
// printf("measure add Segment(%d, %s) %p %p\n", el->tick(), seg->subTypeName(), seg->next(), seg->prev());
// printf("   measure %d %d\n", tick(), tickLen());

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
                        break;
                        }
                  int st = el->subtype();
                  if (st == SegGrace) {
                        Segment* s;
                        for (s = first(); s && s->tick() < t; s = s->next())
                              ;
                        if (s && s->tick() > t) {
                              insert(seg, s);
                              break;
                              }
                        if (s && s->subtype() != SegEndBarLine) {
                              for (; s && s->subtype() != SegChordRest; s = s->next())
                                    ;
                              }
                        insert(seg, s);
                        break;
                        }
                  Segment* s;
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
                  insert(seg, s);
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
                              if (debugMode)
                                    printf("warning: layout break already set\n");
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
                        }
                  _el.push_back(el);
                  break;

            case JUMP:
                  _repeatFlags |= RepeatJump;
                  _el.append(el);
                  break;

            case IMAGE:
                  static_cast<Image*>(el)->reference();

            case DYNAMIC:
            case SYMBOL:
            case TEXT:
            case TEMPO_TEXT:
            case HARMONY:
            case MARKER:
            case STAFF_TEXT:
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
                  printf("Measure::add(%s) not impl.\n", el->name());
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
                  staves[el->staffIdx()]->_vspacer = 0;
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
                        printf("Measure remove: Tuplet not found\n");
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
                        }
                  if (!_el.remove(el))
                        printf("Measure(%p)::remove(%s,%p) not found\n",
                           this, el->name(), el);
                  break;

            case JUMP:
                  _repeatFlags &= ~RepeatJump;
                  goto marker;

            case IMAGE:
                  static_cast<Image*>(el)->dereference();

marker:
            case MARKER:
            case DYNAMIC:
            case TEMPO_TEXT:
            case TEXT:
            case SYMBOL:
            case HARMONY:
            case STAFF_TEXT:
            case HBOX:
                  if (el->type() == TEXT && el->subtype() == TEXT_MEASURE_NUMBER)
                        break;
                  if (!_el.remove(el)) {
                        printf("Measure(%p)::remove(%s,%p) not found\n",
                           this, el->name(), el);
                        }
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
                  printf("Measure::remove: %s %p not found\n", el->name(), el);
                  break;

            default:
                  printf("Measure::remove %s: not impl.\n", el->name());
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
      foreach(Element* e, _el)
            e->setTick(e->tick() + diff);
      setTick(tick() + diff);
      int staves = _score->nstaves();
      int tracks = staves * VOICES;
      for (Segment* segment = first(); segment; segment = segment->next()) {
            int ltick;
            if ((segment->subtype() == SegEndBarLine)
               || (segment->subtype() == SegTimeSigAnnounce)) {
                  ltick = tick() + tickLen();
                  }
            else {
                  ltick = segment->tick() + diff;
                  }
            segment->setTick(ltick);

            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (e)
                        // e->setTick(e->tick() + diff);
                        e->setTick(ltick);
                  }
            for (int staff = 0; staff < staves; ++staff) {
                  const LyricsList* ll = segment->lyricsList(staff);
                  for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                        if (*i) {
                              (*i)->setTick((*i)->tick() + diff);
                              if ((*i)->endTick() != 0)
                                    (*i)->setEndTick((*i)->endTick() + diff);
                              }
                        }
                  }
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
#if 0
      foreach(Beam* e, _beams) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= eStaff) {
                  int voice    = e->voice();
                  staffIdx -= eStaff - sStaff;
                  e->setTrack(staffIdx * VOICES + voice);
                  }
            }
#endif
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
//   cmdRemoveStaves
//---------------------------------------------------------

void Measure::cmdRemoveStaves(int sStaff, int eStaff)
      {
      int sTrack = sStaff * VOICES;
      int eTrack = eStaff * VOICES;
      for (Segment* s = _first; s; s = s->next()) {
            for (int track = eTrack - 1; track >= sTrack; --track) {
                  Element* el = s->element(track);
                  if (el && !el->generated())
                        _score->undoRemoveElement(el);
                  }
            if (s->isEmpty())
                  _score->undoRemoveElement(s);
            }
      foreach(Element* e, _el) {
            if (e->track() == -1)
                  continue;
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && staffIdx < eStaff)
                  _score->undoRemoveElement(e);
            }
      foreach(Tuplet* e, _tuplets) {
            int staffIdx = e->staffIdx();
            if (staffIdx >= sStaff && staffIdx < eStaff)
                  _score->undoRemoveElement(e);
            }

      _score->undo()->push(new RemoveStaves(this, sStaff, eStaff));

      for (int i = eStaff - 1; i >= sStaff; --i)
            _score->undo()->push(new RemoveMStaff(this, *(staves.begin()+i), i));

      for (int i = 0; i < staves.size(); ++i)
            staves[i]->lines->setTrack(i * VOICES);

      // barLine
      // TODO
      }

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(int sStaff, int eStaff)
      {
      _score->undo()->push(new InsertStaves(this, sStaff, eStaff));

      Segment* ts = findSegment(SegTimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = _score->staff(i);
            MStaff* ms   = new MStaff;
            ms->lines    = new StaffLines(score());
            ms->lines->setTrack(i * VOICES);
            ms->lines->setLines(staff->lines());
            ms->lines->setParent(this);
            ms->lines->setVisible(!staff->invisible());

            _score->undo()->push(new InsertMStaff(this, ms, i));

            Rest* rest = new Rest(score(), tick(), Duration(Duration::V_MEASURE));
            rest->setTrack(i * VOICES);
            Segment* s = findSegment(SegChordRest, tick());
            if (s == 0) {
                  s = createSegment(SegChordRest, tick());
                  score()->undoAddElement(s);
                  }
            rest->setParent(s);
            score()->undoAddElement(rest);

            // replicate time signature
            if (ts) {
                  TimeSig* ots = 0;
                  for (int track = 0; track < staves.size() * VOICES; ++track) {
                        if (ts->element(track)) {
                              ots = (TimeSig*)ts->element(track);
                              break;
                              }
                        }
                  if (ots) {
                        TimeSig* timesig = new TimeSig(*ots);
                        timesig->setTrack(i * VOICES);
                        score()->undoAddElement(timesig);
                        }
                  }
            }
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
      ms->lines->setLines(staff->lines());
      ms->lines->setParent(this);
      ms->lines->setTrack(staffIdx * VOICES);
      ms->distance = point(staffIdx == 0 ? score()->styleS(ST_systemDistance) : score()->styleS(ST_staffDistance));
      ms->lines->setVisible(!staff->invisible());
      insertMStaff(ms, staffIdx);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

/**
 Return true if an Element of type \a type can be dropped on a Measure
 at canvas relative position \a p.

 Note special handling for clefs (allow drop if left of rightmost chord or rest in this staff)
 and key- and timesig (allow drop if left of first chord or rest).
*/

bool Measure::acceptDrop(ScoreView* viewer, const QPointF& p, int type, int) const
      {
      // convert p from canvas to measure relative position and take x and y coordinates
      QPointF mrp = p - canvasPos(); // pos() - system()->pos() - system()->page()->pos();
      double mrpx = mrp.x();
      double mrpy = mrp.y();

      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1) {
            return false;                       // staff not found
            }
      QRectF sb(s->staff(idx)->bbox());
      qreal t = sb.top();    // top of staff
      qreal b = sb.bottom(); // bottom of staff

      // compute rectangle of staff in measure
      QRectF rrr(sb.translated(s->canvasPos()));
      QRectF rr(abbox());
      QRectF r(rr.x(), rrr.y(), rr.width(), rrr.height());

      switch(type) {
            case STAFF_LIST:
                  viewer->setDropRectangle(r);
                  return true;

            case MEASURE_LIST:
            case JUMP:
            case MARKER:
            case LAYOUT_BREAK:
                  viewer->setDropRectangle(rr);
                  return true;

            case BRACKET:
            case REPEAT_MEASURE:
            case MEASURE:
            case SPACER:
                  viewer->setDropRectangle(r);
                  return true;

            case BAR_LINE:
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  return true;

            case CLEF:
                  {
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  // search segment list backwards for segchordrest
                  for (Segment* seg = _last; seg; seg = seg->prev()) {
                        if (seg->subtype() != SegChordRest)
                              continue;
                        // SegChordRest found, check if it contains anything in this staff
                        for (int track = idx * VOICES; track < idx * VOICES + VOICES; ++track)
                              if (seg->element(track)) {
                                    // LVIFIX: for the rest in newly created empty measures,
                                    // seg->pos().x() is incorrect
                                    return mrpx < seg->pos().x();
                                    }
                        }
                  }
                  return false;

            case KEYSIG:
            case TIMESIG:
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  viewer->setDropRectangle(r);
                  for (Segment* seg = _first; seg; seg = seg->next()) {
                        if (seg->subtype() == SegChordRest) {
                              if (mrpx < seg->pos().x())
                                    return true;
                              }
                        }
                  // fall through if no chordrest segment found

            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   drop
///   Drop element.
///   Handle a dropped element at position \a pos of given
///   element \a type and \a subtype.
//---------------------------------------------------------

Element* Measure::drop(ScoreView*, const QPointF& p, const QPointF& dragOffset, Element* e)
      {
      // determine staff
      System* s = system();
      int staffIdx = s->y2staff(p.y());
      if (staffIdx == -1 || e->systemFlag()) {
            staffIdx = 0;
            }
      QPointF mrp(p - canvasPos());
      double mrpx = mrp.x();
      Staff* staff = score()->staff(staffIdx);
      int t = tick();
      for (Segment* seg = _first; seg; seg = seg->next()) {
            if (seg->subtype() == SegChordRest) {
                  if (mrpx < seg->pos().x()) {
                        t = seg->tick();
                        break;
                        }
                  }
            }

      switch(e->type()) {
            case MEASURE_LIST:
                  printf("drop measureList or StaffList\n");
                  delete e;
                  break;

            case STAFF_LIST:
printf("drop staffList\n");
//TODO                  score()->pasteStaff(e, this, staffIdx);
                  delete e;
                  break;

            case MARKER:
            case JUMP:
                  e->setParent(this);
                  e->setTrack(-1);        // this are system elements
                  score()->cmdAdd(e);
                  break;

            case SYMBOL:
                  e->setParent(this);
                  e->setTrack(staffIdx * VOICES);
                  e->layout();
                  {
                  QPointF uo(p - e->canvasPos() - dragOffset);
                  e->setUserOff(uo);
                  }
                  score()->cmdAdd(e);
                  break;

            case BRACKET:
                  {
                  Bracket* b = static_cast<Bracket*>(e);
                  b->setTrack(staffIdx * VOICES);
                  b->setParent(system());
                  b->setLevel(-1);  // add bracket
                  score()->cmdAdd(b);
                  }
                  break;

            case CLEF:
                  staff->changeClef(t, e->subtype());
                  delete e;
                  break;

            case KEYSIG:
                  {
                  KeySig* ks    = static_cast<KeySig*>(e);
                  KeySigEvent k = ks->keySigEvent();
                  //add custom key to score if not exist
                  if (k.custom()) {
                        int customIdx = score()->customKeySigIdx(ks);
                        if (customIdx == -1) {
                              customIdx = score()->addCustomKeySig(ks);
                              k.setCustomType(customIdx);
                              }
                        else
                              delete ks;
                      }
                  else
                        delete ks;
                  staff->changeKeySig(tick(), k);
                  break;
                  }

            case TIMESIG:
                  score()->changeTimeSig(tick(), e->subtype());
                  delete e;
                  break;

            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = static_cast<LayoutBreak*>(e);
                  if (_pageBreak || _lineBreak) {
                        if ((lb->subtype() == LAYOUT_BREAK_PAGE && _pageBreak)
                           || (lb->subtype() == LAYOUT_BREAK_LINE && _lineBreak)) {
                              //
                              // if break already set
                              //
                              delete lb;
                              break;
                              }
                        foreach(Element* elem, _el) {
                              if (elem->type() == LAYOUT_BREAK) {
                                    score()->undoChangeElement(elem, e);
                                    break;
                                    }
                              }
                        break;
                        }
                  lb->setTrack(-1);       // this are system elements
                  lb->setParent(this);
                  score()->cmdAdd(lb);
                  return lb;
                  }

            case SPACER:
                  {
                  Spacer* spacer = static_cast<Spacer*>(e);
                  spacer->setTrack(staffIdx * VOICES);
                  spacer->setParent(this);
                  score()->cmdAdd(spacer);
                  return spacer;
                  }

            case BAR_LINE:
                  {
                  BarLine* bl = (BarLine*)e;
                  Measure* nm = nextMeasure();
                  switch(bl->subtype()) {
                        case END_BAR:
                        case NORMAL_BAR:
                        case DOUBLE_BAR:
                        case BROKEN_BAR:
                              {
                              score()->undoChangeRepeatFlags(this, _repeatFlags & ~RepeatEnd);
                              if (nm)
                                    score()->undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                              score()->undoChangeEndBarLineType(this, bl->subtype());
                              _endBarLineGenerated = false;
                              }
                              break;
                        case START_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatStart);
                              break;
                        case END_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatEnd);
                              if (nm)
                                    score()->undoChangeRepeatFlags(nm, nm->repeatFlags() & ~RepeatStart);
                              break;
                        case END_START_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatEnd);
                              if (nm)
                                    score()->undoChangeRepeatFlags(nm, nm->repeatFlags() | RepeatStart);
                              break;
                        }
                  delete bl;
                  }
                  break;

            case REPEAT_MEASURE:
                  {
                  delete e;
                  //
                  // see also cmdDeleteSelection()
                  //
                  _score->select(0, SELECT_SINGLE, 0);
                  for (Segment* s = first(); s; s = s->next()) {
                        if (s->subtype() == SegChordRest || s->subtype() == SegGrace) {
                              int strack = staffIdx * VOICES;
                              int etrack = strack + VOICES;
                              for (int track = strack; track < etrack; ++track) {
                                    Element* el = s->element(track);
                                    if (el)
                                          _score->undoRemoveElement(el);
                                    }
                              if (s->isEmpty())
                                    _score->undoRemoveElement(s);
                              }
                        }
                  //
                  // add repeat measure
                  //

                  Segment* seg = findSegment(SegChordRest, tick());
                  if (seg == 0) {
                        seg = createSegment(SegChordRest, tick());
                        _score->undoAddElement(seg);
                        }
                  RepeatMeasure* rm = new RepeatMeasure(_score);
                  rm->setTick(tick());
                  rm->setTrack(staffIdx * VOICES);
                  rm->setParent(seg);
                  _score->undoAddElement(rm);
                  foreach(Element* el, _el) {
                        if (el->type() == SLUR && el->staffIdx() == staffIdx)
                              _score->undoRemoveElement(el);
                        }
                  _score->select(rm, SELECT_SINGLE, 0);
                  }
                  break;

            default:
                  printf("Measure: cannot drop %s here\n", e->name());
                  delete e;
                  break;
            }
      return 0;
      }

//---------------------------------------------------------
//   cmdRemoveEmptySegment
//---------------------------------------------------------

void Measure::cmdRemoveEmptySegment(Segment* s)
      {
      if (s->isEmpty())
            _score->undoRemoveElement(s);
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Measure::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(tr("Measure Properties..."));
      a->setData("props");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Measure::propertyAction(ScoreView*, const QString& s)
      {
      if (s == "props") {
            MeasureProperties im(this);
            im.exec();
            }
      }

//---------------------------------------------------------
//   adjustToLen
//    the measure len has changed, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(int ol, int nl)
      {
      int staves = score()->nstaves();
      int diff   = nl - ol;

      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            int rests  = 0;
            int chords = 0;
            Rest* rest = 0;
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;
                  for (int track = strack; track < etrack; ++track) {
                        Element* e = segment->element(track);
                        if (e && e->type() == REST) {
                              ++rests;
                              rest = static_cast<Rest*>(e);
                              }
                        else if (e && e->type() == CHORD)
                              ++chords;
                        }
                  }
            if (rests == 1 && chords == 0 && rest->duration().type() == Duration::V_MEASURE)
                  continue;
            // printf("rests = %d\n", rests);
            const AL::SigEvent ev(score()->sigmap()->timesig(tick()));
            if (ev.nominalEqualActual() && (rests == 1) && (chords == 0) && !_irregular) {
                  rest->setDuration(Duration::V_MEASURE);    // whole measure rest
                  }
            else {
                  int strack = staffIdx * VOICES;
                  int etrack = strack + VOICES;

                  for (int trk = strack; trk < etrack; ++trk) {
                        int n = diff;
                        bool rFlag = false;
                        if (n < 0)  {
                              for (Segment* segment = last(); segment;) {
                                    Segment* pseg = segment->prev();
                                    Element* e = segment->element(trk);
                                    if (e && e->isChordRest()) {
                                          ChordRest* cr = static_cast<ChordRest*>(e);
                                          if (cr->duration() == Duration::V_MEASURE)
                                                n = nl;
                                          else
                                                n += cr->ticks();
                                          score()->undoRemoveElement(e);
                                          if (segment->isEmpty())
                                                score()->undoRemoveElement(segment);
                                          if (n >= 0)
                                                break;
                                          }
                                    segment = pseg;
                                    }
                              rFlag = true;
                              }
                        int voice = trk % VOICES;
                        if ((n > 0) && (rFlag || voice == 0)) {
                              // add rest to measure
                              int rtick = tick() + nl - n;
                              Segment* seg = findSegment(SegChordRest, rtick);
                              if (seg == 0) {
                                    seg = createSegment(SegChordRest, rtick);
                                    score()->undoAddElement(seg);
                                    }
                              Duration d;
                              d.setVal(n);
                              rest = new Rest(score(), rtick, d);
                              rest->setTrack(staffIdx * VOICES + voice);
                              rest->setParent(seg);
                              score()->undoAddElement(rest);
                              }
                        }
                  }
            }
      score()->undoChangeMeasureLen(this, ol, nl);
      if (diff < 0) {
            //
            //  CHECK: do not remove all slurs
            //
            foreach(Element* e, _el) {
                  if (e->type() == SLUR)
                        score()->undoRemoveElement(e);
                  }
            score()->cmdRemoveTime(tick() + nl, -diff);
            }
      else{
            score()->undoInsertTime(tick() + ol, diff);
            score()->undoSigInsertTime(tick() + ol, diff);
            }
      }

//---------------------------------------------------------
//   writeTuplets
//---------------------------------------------------------

void Measure::writeTuplets(Xml& xml, int staff) const
      {
      for (int level = 0; true; level++) {                // write tuplets
            bool found = false;
            foreach(Tuplet* tuplet, _tuplets) {
                  if (tuplet->staffIdx() == staff) {
                        int l = 0;
                        for (Tuplet* t = tuplet->tuplet(); t; t = t->tuplet())
                              ++l;
                        if (l == level) {
                              tuplet->write(xml);
                              found = true;
                              }
                        }
                  }
            if (!found)
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml, int staff, bool writeSystemElements) const
      {
      if (xml.curTick != tick())
            xml.stag(QString("Measure number=\"%1\" tick=\"%2\"").arg(_no + 1).arg(tick()));
      else
            xml.stag(QString("Measure number=\"%1\"").arg(_no + 1));
      xml.curTick = tick();

      if (writeSystemElements) {
            if (_repeatFlags & RepeatStart)
                  xml.tagE("startRepeat");
            if (_repeatFlags & RepeatEnd)
                  xml.tag("endRepeat", _repeatCount);
            if (_irregular)
                  xml.tagE("irregular");
            if (_breakMultiMeasureRest)
                  xml.tagE("breakMultiMeasureRest");
            if (_userStretch != 1.0)
                  xml.tag("stretch", _userStretch);
            if (_noText)
                  _noText->write(xml);
            if (_noOffset)
                  xml.tag("noOffset", _noOffset);
            }

      MStaff* mstaff = staves[staff];
      if (mstaff->_vspacer)
            xml.tag("vspacer", mstaff->_vspacer->getSpace().val());
      if (!mstaff->_visible)
            xml.tag("visible", mstaff->_visible);
      if (mstaff->_slashStyle)
            xml.tag("slashStyle", mstaff->_slashStyle);

      foreach (const Element* el, _el) {
            if ((el->staffIdx() == staff) || (el->systemFlag() && writeSystemElements)) {
                  el->write(xml);
                  }
            }

      writeTuplets(xml, staff);

      for (int track = staff * VOICES; track < staff * VOICES + VOICES; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  //
                  // special case: - barline span > 1
                  //               - part (excerpt) staff starts after
                  //                 barline element
                  if ((segment->subtype() == SegEndBarLine)
                     && (e == 0)
                     && writeSystemElements
                     && ((track % VOICES) == 0)) {
                        // search barline:
                        for (int idx = track - VOICES; idx >= 0; idx -= VOICES) {
                              if (segment->element(idx)) {
                                    int oDiff = xml.trackDiff;
                                    xml.trackDiff = idx;          // staffIdx should be zero
                                    segment->element(idx)->write(xml);
                                    xml.trackDiff = oDiff;
                                    break;
                                    }
                              }
                        }

                  if (e && !e->generated()) {
                        if (e->isChordRest()) {
                              ChordRest* cr = static_cast<ChordRest*>(e);
                              Beam* beam = cr->beam();
                              if (beam && beam->elements().front() == cr)
                                    beam->write(xml);
                              }
                        if (segment->subtype() == SegEndBarLine && _multiMeasure > 0) {
                              xml.stag("BarLine");
                              xml.tag("subtype", _endBarLineType);
                              xml.tag("visible", _endBarLineVisible);
                              xml.etag();
                              }
                        else
                              e->write(xml);
                        }
                  }
            }
      for (Segment* segment = first(); segment; segment = segment->next()) {
            const LyricsList* ll = segment->lyricsList(staff);
            for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                  if (*i)
                        (*i)->write(xml);
                  }
            }
      xml.etag();
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml) const
      {
      xml.stag(QString("Measure tick=\"%1\"").arg(tick()));
      xml.curTick = tick();

      if (_repeatFlags & RepeatStart)
            xml.tagE("startRepeat");
      if (_repeatFlags & RepeatEnd)
            xml.tag("endRepeat", _repeatCount);
      if (_irregular)
            xml.tagE("irregular");
      if (_breakMultiMeasureRest)
            xml.tagE("breakMultiMeasureRest");
      xml.tag("stretch", _userStretch);

      if (_noText)
            _noText->write(xml);

      for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
            xml.stag("Staff");
            for (ciElement i = _el.begin(); i != _el.end(); ++i) {
                  if ((*i)->staff() == _score->staff(staffIdx) && (*i)->type() != SLUR_SEGMENT)
                        (*i)->write(xml);
                  }
            for (int track = staffIdx * VOICES; track < staffIdx * VOICES + VOICES; ++track) {
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        Element* e = segment->element(track);
                        if (e && !e->generated()) {
                              if (e->isDurationElement()) {
                                    DurationElement* de = static_cast<DurationElement*>(e);
                                    Tuplet* tuplet = de->tuplet();
                                    if (tuplet && tuplet->elements().front() == de) {
                                          tuplet->setId(xml.tupletId++);
                                          tuplet->write(xml);
                                          }
                                    }
                              if (segment->subtype() == SegEndBarLine && _multiMeasure > 0) {
                                    xml.stag("BarLine");
                                    xml.tag("subtype", _endBarLineType);
                                    xml.tag("visible", _endBarLineVisible);
                                    xml.etag();
                                    }
                              else
                                    e->write(xml);
                              }
                        }
                  }
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  const LyricsList* ll = segment->lyricsList(staffIdx);
                  for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                        if (*i)
                              (*i)->write(xml);
                        }
                  }
            xml.etag();
            }
      xml.etag();
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

/**
 Read Measure.
*/

void Measure::read(QDomElement e, int idx)
      {
      for (int n = staves.size(); n <= idx; ++n) {
            MStaff* s    = new MStaff;
            Staff* staff = score()->staff(n);
            s->lines     = new StaffLines(score());
            s->lines->setLines(staff->lines());
            s->lines->setParent(this);
            s->lines->setTrack(n * VOICES);
            s->distance = point(n == 0 ? score()->styleS(ST_systemDistance) : score()->styleS(ST_staffDistance));
            s->lines->setVisible(!staff->invisible());
            staves.append(s);
            }

      int tck = e.attribute("tick", "-1").toInt();
      if (tck >= 0) {
            tck = score()->fileDivision(tck);
            setTick(tck);
            score()->curTick = tick();
            }
      else {
            setTick(score()->curTick);
            }
      score()->curTick = tick();

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setTrack(score()->curTrack);
                  barLine->setParent(this);
                  barLine->setTick(score()->curTick);
                  barLine->read(e);
                  if ((barLine->tick() != tick()) && (barLine->tick() != (tick() + tickLen()))) {
                        // this is a mid measure bar line
                        Segment* s = getSegment(SegBarLine, barLine->tick());
                        s->add(barLine);
                        }
                  else if (barLine->subtype() == START_REPEAT) {
                        Segment* s = getSegment(SegStartRepeatBarLine, barLine->tick());
                        s->add(barLine);
                        }
                  else {
                        setEndBarLineType(barLine->subtype(), false, barLine->visible(), barLine->color());
                        delete barLine;
                        }
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->setTick(score()->curTick);   // set default tick position
                  chord->read(e, _tuplets);
                  if (chord->tremolo() && chord->tremolo()->twoNotes()) {
                        //
                        // search first note of tremolo
                        //
                        Chord* c1 = 0;
                        int track = score()->curTrack;
                        for (Segment* s = first(SegChordRest); s; s = s->next(SegChordRest)) {
                              if (s->tick() >= chord->tick())
                                    break;
                              if (s->element(track) && s->element(track)->type() == CHORD)
                                    c1 = static_cast<Chord*>(s->element(track));
                              }
                        if (c1 && (chord->tick() != c1->tick() + c1->tickLen() / 2)) {
                              //
                              // fixup some tremolo quirks
                              // chord tick position is wrong
                              //
                              int ticklen2 = chord->tickLen() / 2;
                              int tick = c1->tick() + ticklen2;
                              chord->setTick(tick);
                              }
                        Segment* s = getSegment(chord);
                        s->add(chord);
                        score()->curTick = chord->tick() + chord->tickLen() / 2;
                        }
                  else {
                        Segment* s = getSegment(chord);
                        s->add(chord);
                        score()->curTick = chord->tick() + chord->tickLen();
                        }
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTick(score()->curTick);
                  breath->setTrack(score()->curTrack);
                  breath->read(e);
                  Segment* s = getSegment(SegBreath, breath->tick());
                  s->add(breath);
                  score()->curTick = breath->tick();
                  }
            else if (tag == "Note") {
                  Chord* chord = new Chord(score());
                  chord->setTrack(score()->curTrack);
                  chord->setTick(score()->curTick);   // set default tick position
                  chord->readNote(e, _tuplets);
                  Segment* s = getSegment(chord);
                  s->add(chord);
                  score()->curTick = chord->tick() + chord->tickLen();
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setTrack(score()->curTrack);
                  rest->setTick(score()->curTick);    // set default tick position
                  rest->read(e, _tuplets);
                  Segment* s = getSegment(rest);
                  s->add(rest);
                  int t = rest->tick() + (rest->ticks() <= 0 ? tickLen() : rest->ticks());
                  score()->curTick = t;
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTrack(score()->curTrack);
                  rm->setTick(score()->curTick);    // set default tick position
                  rm->setParent(this);
                  rm->read(e);
                  Segment* s = getSegment(SegChordRest, rm->tick());
                  s->add(rm);
                  score()->curTick = rm->tick() + tickLen();
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTrack(score()->curTrack);
                  clef->setTick(score()->curTick);
                  clef->read(e);
                  Segment* s = getSegment(clef);
                  s->add(clef);
                  score()->curTick = clef->tick();
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTrack(score()->curTrack);
                  ts->setTick(score()->curTick);
                  ts->read(e);
                  Segment* s = getSegment(ts);
                  s->add(ts);
                  score()->curTick = ts->tick();
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTrack(score()->curTrack);
                  ks->setTick(score()->curTick);
                  ks->read(e);
//??                  char newSig = ks->subtype() & 0xff;
//                  ks->setSig(0, newSig);
                  Segment* s = getSegment(ks);
                  s->add(ks);
                  score()->curTick = ks->tick();
                  }
            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTrack(score()->curTrack);
                  dyn->setTick(score()->curTick);
                  dyn->read(e);
                  dyn->resetType(); // for backward compatibility
                  add(dyn);
                  score()->curTick = dyn->tick();
                  }
            else if (tag == "Lyrics") {
                  Lyrics* lyrics = new Lyrics(score());
                  lyrics->setTrack(score()->curTrack);
                  lyrics->setTick(score()->curTick);
                  lyrics->read(e);
                  Segment* segment = tick2segment(lyrics->tick());
                  if (segment == 0) {
                        printf("no segment for lyrics at %d\n",
                           lyrics->tick());
                        }
                  else
                        segment->add(lyrics);
                  score()->curTick = lyrics->tick();
                  }
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->setTrack(score()->curTrack);
                  t->setTick(score()->curTick);
                  t->read(e);
                  score()->curTick = t->tick();

                  int st = t->subtype();
                  if (st == TEXT_TITLE || st == TEXT_SUBTITLE || st == TEXT_COMPOSER
                     || st == TEXT_POET) {
                        if (st == TEXT_TITLE)
                              t->setTextStyle(TEXT_STYLE_TITLE);
                        else if (st == TEXT_SUBTITLE)
                              t->setTextStyle(TEXT_STYLE_SUBTITLE);
                        else if (st == TEXT_COMPOSER)
                              t->setTextStyle(TEXT_STYLE_COMPOSER);
                        else if (st == TEXT_POET)
                              t->setTextStyle(TEXT_STYLE_POET);
                        // for backward compatibility:
                        MeasureBase* measure = score()->first();
                        if (measure->type() != VBOX) {
                              measure = new VBox(score());
                              measure->setTick(0);
                              measure->setNext(score()->first());
                              score()->add(measure);
                              }
                        measure->add(t);
                        }
                  else {
                        if (st == TEXT_MEASURE_NUMBER) {
                              t->setTextStyle(TEXT_STYLE_MEASURE_NUMBER);
                              t->setTick(-1);   // layout to start of measure
                              t->setTrack(-1);
                              }
                        add(t);
                        }
                  }
            else if (tag == "Harmony") {
                  Harmony* h = new Harmony(score());
                  h->setTrack(score()->curTrack);
                  h->setTick(score()->curTick);
                  h->read(e);
                  add(h);
                  score()->curTick = h->tick();
                  }
            else if (tag == "Tempo") {
                  TempoText* t = new TempoText(score());
                  t->setTrack(-1);
                  t->setTick(score()->curTick);
                  t->read(e);
                  add(t);
                  score()->curTick = t->tick();
                  }
            else if (tag == "StaffText") {
                  StaffText* t = new StaffText(score());
                  t->setTrack(score()->curTrack);
                  t->setTick(score()->curTick);
                  t->read(e);
                  add(t);
                  score()->curTick = t->tick();
                  }
            else if (tag == "Symbol") {
                  Symbol* sym = new Symbol(score());
                  sym->setTrack(score()->curTrack);
                  sym->setTick(score()->curTick);
                  sym->read(e);
                  add(sym);
                  score()->curTick = sym->tick();
                  }
            else if (tag == "stretch")
                  _userStretch = val.toDouble();
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "noOffset")
                  _noOffset = val.toInt();
            else if (tag == "irregular")
                  _irregular = true;
            else if (tag == "breakMultiMeasureRest")
                  _breakMultiMeasureRest = true;
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->setTrack(score()->curTrack);
                  tuplet->setTick(score()->curTick);
                  tuplet->setTrack(score()->curTrack);
                  tuplet->setParent(this);
                  tuplet->read(e, _tuplets);
                  add(tuplet);
                  }
            else if (tag == "Marker") {
                  Marker* marker = new Marker(score());
                  marker->setTrack(score()->curTrack);
                  marker->read(e);
                  add(marker);
                  }
            else if (tag == "Jump") {
                  Jump* jump = new Jump(score());
                  jump->setTrack(score()->curTrack);
                  jump->read(e);
                  add(jump);
                  }
            else if (tag == "startRepeat")
                  _repeatFlags |= RepeatStart;
            else if (tag == "endRepeat") {
                  _repeatCount = val.toInt();
                  _repeatFlags |= RepeatEnd;
                  }
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
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
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->setTrack(score()->curTrack);
                        image->read(e);
                        add(image);
                        }
                  }
            else if (tag == "Slur") {           // obsolete
                  Slur* slur = new Slur(score());
                  slur->setTrack(score()->curTrack);
                  slur->read(e);
                  score()->add(slur);
                  }
            else if (tag == "vspacer") {
                  if (staves[idx]->_vspacer == 0) {
                        Spacer* spacer = new Spacer(score());
                        spacer->setTrack(idx * VOICES);
                        add(spacer);
                        }
                  staves[idx]->_vspacer->setSpace(Spatium(val.toDouble()));
                  }
            else if (tag == "visible")
                  staves[idx]->_visible = val.toInt();
            else if (tag == "slashStyle")
                  staves[idx]->_slashStyle = val.toInt();
            else if (tag == "Beam") {
                  Beam* beam = new Beam(score());
                  beam->setTrack(score()->curTrack);
                  beam->read(e);
                  beam->setParent(0);
                  score()->beams().append(beam);
                  }
            else
                  domError(e);
            }
#if 0
      int nstaves = staves.size();
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            MStaff* mstaff = staves[staffIdx];
            if (mstaff->hasVoices)
                  printf("has voices staff %d measure %d\n", staffIdx, _no);
            }
#endif
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
            if (ms->_vspacer)
                  func(data, ms->_vspacer);
            }

      int tracks = nstaves * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!visible(staffIdx))
                        continue;
                  LyricsList* ll = s->lyricsList(staffIdx);
                  foreach(Lyrics* l, *ll) {
                        if (l)
                              func(data, l);
                        }
                  }
            for (int track = 0; track < tracks; ++track) {
                  if (!visible(track/VOICES)) {
                        track += VOICES - 1;
                        continue;
                        }
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  if (e->isChordRest()) {
                        if (e->type() == CHORD)
                              e->scanElements(data, func);
                        else
                              func(data, e);
                        ChordRest* cr = (ChordRest*)e;
                        QList<Articulation*>* al = cr->getArticulations();
                        for (ciArticulation i = al->begin(); i != al->end(); ++i) {
                              Articulation* a = *i;
                              func(data, a);
                              }
                        }
                  else
                        func(data, e);
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
//   createVoice
//    Create a voice on demand by filling the measure
//    with a whole measure rest.
//    Check if there are any chord/rests in track; if
//    not create a whole measure rest
//---------------------------------------------------------

void Measure::createVoice(int track)
      {
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != SegChordRest)
                  continue;
            if (s->element(track) == 0) {
                  Rest* rest = new Rest(score(), tick(), Duration(Duration::V_MEASURE));
                  rest->setTrack(track);
                  rest->setParent(s);
                  score()->undoAddElement(rest);
                  }
            break;
            }
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
                  bl->setSubtype(START_REPEAT);
                  bl->setGenerated(true);
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
                        Segment* seg = getSegment(SegEndBarLine, tick() + tickLen());
                        seg->add(bl);
                        changed = true;
                        bl->layout();
                        }
                  }
            if (bl) {
                  bl->setMag(staff->mag());
                  int et = _multiMeasure > 0 ? _mmEndBarLineType : _endBarLineType;
                  if (bl->subtype() != et) {
                        bl->setSubtype(et);
                        changed = true;
                        }
                  bl->setGenerated(_endBarLineGenerated);
                  bl->setVisible(_endBarLineVisible);
                  bl->setColor(_endBarLineColor);
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

void Measure::setEndBarLineType(int val, bool g, bool visible, QColor color)
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
      foreach(Tuplet* tuplet, _tuplets) {
            int voice    = tuplet->voice();
            int staffIdx = tuplet->staffIdx();
            int idx = dst.indexOf(staffIdx);
            tuplet->setTrack(idx * VOICES + voice);
            }
      }

//---------------------------------------------------------
//   exchangeVoice
//---------------------------------------------------------

void Measure::exchangeVoice(int v1, int v2, int staffIdx1, int staffIdx2)
      {
      for (int staffIdx = staffIdx1; staffIdx < staffIdx2; ++ staffIdx) {
            for (Segment* s = first(SegChordRest); s; s = s->next(SegChordRest)) {
                  int strack = staffIdx * VOICES + v1;
                  int dtrack = staffIdx * VOICES + v2;
                  s->swapElements(strack, dtrack);
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
 this staff
*/

bool Measure::isMeasureRest(int staffIdx)
      {
      int strack = staffIdx * VOICES;
      int etrack = staffIdx * VOICES + VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() == SegChordRest)
                for (int track = strack; track < etrack; ++track) {
                      Element* e = s->element(track);
    				          if(e && e->type() != REST)
    					           return false;
                      }
            }
            return true;
      }

//---------------------------------------------------------
//   userDistance
//---------------------------------------------------------

Spatium Measure::userDistance(int i) const
      {
      return staves[i]->_vspacer ? staves[i]->_vspacer->getSpace() : Spatium(0);
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
//   fraction
//---------------------------------------------------------

Fraction Measure::fraction() const
      {
      return score()->sigmap()->timesig(tick()).fraction();
      }

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
      double stretch;
      double fix;
      Spring(int i, double s, double f) : seg(i), stretch(s), fix(f) {}
      };

typedef std::multimap<double, Spring, std::less<double> > SpringMap;
typedef SpringMap::iterator iSpring;

//---------------------------------------------------------
//   sff
//    compute 1/Force for a given Extend
//---------------------------------------------------------

static double sff(double x, double xMin, SpringMap& springs)
      {
      if (x <= xMin)
            return 0.0;
      iSpring i = springs.begin();
      double c  = i->second.stretch;
      if (c == 0.0)           //DEBUG
            c = 1.1;
      double f = 0.0;
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

void Measure::layoutX(double stretch)
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

      double _spatium           = spatium();
      int tracks                = nstaves * VOICES;
      double clefKeyRightMargin = score()->styleS(ST_clefKeyRightMargin).val() * _spatium;

      double rest[nstaves];    // fixed space needed from previous segment
      memset(rest, 0, nstaves * sizeof(double));
      //--------tick table for segments
      int ticks[segs];
      memset(ticks, 0, segs * sizeof(int));

      double xpos[segs+1];
      int types[segs];
      double width[segs];

      int segmentIdx  = 0;
      double x        = 0.0;
      int minTick     = 100000;
      int ntick       = tick() + tickLen();   // position of next measure

      for (const Segment* s = first(); s; s = s->next(), ++segmentIdx) {
            types[segmentIdx] = s->subtype();
            bool rest2[nstaves+1];
            double segmentWidth    = 0.0;
            double minDistance     = 0.0;
            double stretchDistance = 0.0;

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  Space space;
                  int track  = staffIdx * VOICES;
                  bool found = false;
                  if ((s->subtype() == SegChordRest) || (s->subtype() == SegGrace)) {
                        for (int voice = 0; voice < VOICES; ++voice) {
                              ChordRest* cr = static_cast<ChordRest*>(s->element(track+voice));
                              if (!cr)
                                    continue;
                              found = true;
                              if (s == first() || s->prev()->subtype() == SegStartRepeatBarLine) {
                                    double sp       = score()->styleS(ST_barNoteDistance).val() * _spatium;
                                    minDistance     = sp * .3;
                                    stretchDistance = sp * .7;
                                    }
                              else {
                                    int pt = s->prev()->subtype();
//                                    if (pt == SegKeySig || pt == SegTimeSig || pt == SegClef) {
                                    if (pt == SegKeySig || pt == SegTimeSig) {
                                          minDistance = clefKeyRightMargin;
                                          }
                                    else {
                                          minDistance = score()->styleS(ST_minNoteDistance).val() * _spatium;
                                          if (s->subtype() == SegGrace)
                                                minDistance *= score()->styleD(ST_graceNoteMag);
                                          }
                                    }
                              cr->layout();
                              space.max(cr->space());
                              }

                        double llw = 0.0;
                        double rrw = 0.0;
                        Lyrics* lyrics = 0;
                        foreach(Lyrics* l, *s->lyricsList(staffIdx)) {
                              if (!l)
                                    continue;
                              l->layout();
                              lyrics = l;
                              if (l->endTick() > 0) {
                                    double rw = l->bbox().width();
                                    if (rw > rrw)
                                          rrw = rw;
                                    }
                              else {
                                    double lw = l->bbox().width() * .5;
                                    if (lw > llw)
                                          llw = lw;
                                    if (lw > rrw)
                                          rrw = lw;
                                    }
                              }
                        if (lyrics) {
                              found = true;
                              double y = lyrics->ipos().y() + lyrics->lineHeight()
                                 + point(score()->styleS(ST_lyricsMinBottomDistance));
                              if (y > staves[staffIdx]->distance)
                                 staves[staffIdx]->distance = y;
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
                        double sp = minDistance + rest[staffIdx] + stretchDistance;
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
                        // this happens for tremolo notes
                        printf("layoutX: empty segment: tick %d len %d\n", tick(), tickLen());
                        printf("         nticks==0 segmente %d, segmentIdx: %d, ticks: %d ntick %d\n",
                           size(), segmentIdx-1, s->tick(), ntick
                           );
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        }
                  ticks[segmentIdx] = nticks;
                  }
            else
                  ticks[segmentIdx] = 0;
            }
      double segmentWidth = 0.0;
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
      double stretchList[segs];
      double stretchSum = 0.0;
      stretchList[0]    = 0.0;

      double minimum = xpos[0];
      for (int i = 0; i < segs; ++i) {
            double str = 1.0;
            double d;
            double w = width[i];

            int t = ticks[i];
            if (t) {
                  if (minTick > 0)
                        str += .6 * log2(double(t) / double(minTick));
                  stretchList[i] = str;
                  d = w / str;
                  }
            else {
                  if (ticks[i]) {
                        printf("zero segment %d\n", i);
                        w = 0.0;
                        }
                  stretchList[i] = 0.0;   // dont stretch timeSig and key
                  d = 100000000.0;        // CHECK
                  }
            stretchSum += stretchList[i];
            minimum    += w;
            springs.insert(std::pair<double, Spring>(d, Spring(i, stretchList[i], w)));
            }

      //---------------------------------------------------
      //    distribute stretch to segments
      //---------------------------------------------------

      double force = sff(stretch, minimum, springs);

      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            double stretch = force * i->second.stretch;
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
                                    double eblw = 0.0;
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
                        else if (rest->duration() == Duration::V_MEASURE) {
                              double x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                              double w  = xpos[segs-1] - x1;
                              e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                              }
                        }
                  else if (t == REPEAT_MEASURE) {
                        double x1 = seg == 0 ? 0.0 : xpos[seg] - clefKeyRightMargin;
                        double w  = xpos[segs-1] - x1;
                        e->rxpos() = (w - e->width()) * .5 + x1 - s->x();
                        }
                  else if (t == CHORD) {
                        Chord* chord = static_cast<Chord*>(e);
                        if (chord->glissando())
                              chord->glissando()->layout();
                        chord->layout2();
                        }
                  else if ((t == CLEF) && (s != first())) {
                        double w   = 0.0;
                        if (types[seg+1] != SegChordRest)
                              w = xpos[seg+1] - xpos[seg];
                        double m   = score()->styleS(ST_clefBarlineDistance).val() * _spatium;
                        e->rxpos() = w - e->width() - m;
                        }
                  else {
                        e->setPos(-e->bbox().x(), 0.0);
                        }
                  }
            }
      }

