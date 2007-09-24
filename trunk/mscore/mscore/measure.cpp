//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: measure.cpp,v 1.105 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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
#include "sig.h"
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
#include "irregular.h"
#include "layout.h"
#include "viewer.h"
#include "volta.h"
#include "image.h"
#include "hook.h"
#include "beam.h"
#include "pitchspelling.h"
#include "keysig.h"
#include "breath.h"
#include "arpeggio.h"
#include "tremolo.h"
#include "drumset.h"
#include "repeat.h"

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int y2pitch(double y, int clef)
      {
      int l = lrint(y / _spatium * 2.0);
      return line2pitch(l, clef);
      }

//---------------------------------------------------------
//   line2pitch
//    TODO: needs cleanup
//---------------------------------------------------------

int line2pitch(int line, int clef)
      {
      static const int pt[] = { 0, 2, 4, 5, 7, 9, 11, 12 };

      int l = clefTable[clef].pitchOffset - line;
      int octave = 0;
      while (l < 0) {
            l += 7;
            octave++;
            }
      if (l > 74)
            l = 74;
      int pitch = pt[l % 7] + (l / 7 + octave) * 12;
      return pitch;
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::Measure(Score* s)
   : Element(s)
      {
      _first  = 0;
      _last   = 0;
      _size   = 0;

      int n = _score->nstaves();
      for (int staff = 0; staff < n; ++staff) {
            MStaff s;
            staves.push_back(s);
            }

      setTickLen(-1);
      _userStretch = 1.0;     // ::style->measureSpacing;
      _lineBreak   = false;
      _pageBreak   = false;
      _no          = 0;
      _irregular   = false;
      _endRepeat   = 0;
      _ending      = 0;
      _repeatFlags = 0;
      _noOffset    = 0;
      _noText      = new Text(score());
      _noText->setSubtype(TEXT_MEASURE_NUMBER);
      _noText->setParent(this);
      }

//---------------------------------------------------------
//   Measure
//---------------------------------------------------------

Measure::~Measure()
      {
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
      if (Element::tickLen() == -1)
            _duration.setTick(_score->sigmap->ticksMeasure(tick()));
      return Element::tickLen();
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Measure::remove(Segment* el)
      {
      --_size;
      if (el == _first) {
            _first = _first->next();
            if (el == _last)
                  _last = 0;
            return;
            }
      if (el == _last) {
            _last = _last->prev();
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
//   moveAll
//---------------------------------------------------------

void Measure::moveAll(double x, double y)
      {
      move(x, y);
      for (iElement i = _sel.begin(); i != _sel.end(); ++i)
            (*i)->move(x, y);

      int staves = _score->nstaves();
      for (Segment* segment = first(); segment; segment = segment->next()) {
            for (int track = 0; track < staves * VOICES; ++track) {
                  segment->element(track)->move(x, y);
                  LyricsList* ll = segment->lyricsList(track);
                  for (iLyrics il = ll->begin(); il != ll->end(); ++il)
                        (*il)->move(x, y);
                  }
            }
      foreach(Beam* beam, _beamList)
            beam->move(x, y);
      foreach(Tuplet* t, _tuplets)
            t->move(x, y);
      }

//---------------------------------------------------------
//   initLineList
//    preset lines list with accidentals for given key
//---------------------------------------------------------

static void initLineList(char* ll, int key)
      {
      memset(ll, 0, 74);
      for (int octave = 0; octave < 11; ++octave) {
            if (key > 0) {
                  for (int i = 0; i < key; ++i) {
                        int idx = tpc2line(20 + i) + octave * 7;
                        if (idx < 74)
                              ll[idx] = 1;
                        }
                  }
            else {
                  for (int i = 0; i > key; --i) {
                        int idx = tpc2line(12 + i) + octave * 7;
                        if (idx < 74)
                              ll[idx] = -1;
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layoutChord
//---------------------------------------------------------

void Measure::layoutChord(Chord* chord, char* tversatz)
      {
      Drumset* drumset = 0;
      if (chord->staff()->part()->useDrumset())
            drumset = chord->staff()->part()->drumset();
      NoteList* nl     = chord->noteList();
      int tick         = chord->tick();
      int ll           = 1000;      // line distance to previous note head

      int move1      = nl->front()->move();
      Tuplet* tuplet = chord->tuplet();

      QList<Note*> notes;
      for (iNote in = nl->begin(); in != nl->end(); ++in)
            notes.append(in->second);

      bool mirror = false;                 // notehead position relative to stem
      int nNotes  = notes.size();
      for (int i = 0; i < nNotes; ++i) {
            Note* note  = notes[i];
            int pitch   = note->pitch();
            if (drumset) {
                  if (!drumset->isValid(pitch)) {
                        printf("unmapped drum note %d\n", pitch);
                        }
                  else {
                        note->setHeadGroup(drumset->noteHead(pitch));
                        note->setLine(drumset->line(pitch));
                        note->setHead(tuplet ? tuplet->baseLen() : chord->tickLen());
                        continue;
                        }
                  }

            note->setHead(tuplet ? tuplet->baseLen() : chord->tickLen());
            int move = note->move();
            int clef = note->staff()->clef()->clef(tick);

            //
            // compute accidental
            //
            int tpc        = note->tpc();
            int line       = tpc2line(tpc) + (pitch/12) * 7;
            int tpcPitch   = tpc2pitch(tpc);
            if (tpcPitch < 0)
                  line += 7;
            else
                  line -= (tpcPitch/12)*7;

            int accidental = 0;
            if (note->userAccidental())
                  accidental = note->userAccidental();
            else  {
                  int accVal = ((tpc + 1) / 7) - 2;
                  accidental = ACC_NONE;
                  if (accVal != tversatz[line]) {
                        tversatz[line] = accVal;
                        switch(accVal) {
                              case -2: accidental = ACC_FLAT2;  break;
                              case -1: accidental = ACC_FLAT;   break;
                              case  1: accidental = ACC_SHARP;  break;
                              case  2: accidental = ACC_SHARP2; break;
                              case  0: accidental = ACC_NATURAL; break;
                              default: printf("bad accidental\n"); break;
                              }
                        }
                  }

            //
            // calculate the real note line depending on clef
            //
            line = 127 - line - 82 + clefTable[clef].yOffset;
            note->setLine(line);

            if (mirror || (((ll - line) < 2) && move1 == move))
                  mirror = !mirror;

            if ((nNotes >= 3) && (i == (nNotes-1)) && mirror && !notes[i-2]->mirror()) {
                  notes[i-1]->setMirror(true);
                  mirror = false;
                  }
            note->setMirror(mirror);
            note->setAccidentalSubtype(accidental);
            move1 = move;
            ll    = line;
            }

      //---------------------------------------------------
      //    layout accidentals
      //---------------------------------------------------

      int ll2    = -1000;      // line distance to previous accidental
      int ll3    = -1000;
      int accCol = 0;
      for (int i = nNotes-1; i >= 0; --i) {
            Note* note     = notes[i];
            Accidental* ac = note->accidental();
            if (!ac)
                  continue;
            int line    = note->line();
            bool mirror = note->mirror();
            if ((line - ll2) <= 4) {
                  if (accCol == 0 || ((line - ll3) <= 4))
                        ++accCol;
                  else
                        --accCol;
                  }
            if (accCol > 5)
                  accCol = 0;
            double x = - point(score()->style()->prefixNoteDistance);
            if (note->grace())
                  x *= .5;
            x  -= ac->width();
            x  *= (accCol + 1);
            if (mirror)
                  x -= note->headWidth();
            ac->setPos(x, 0);
            ll3 = ll2;
            ll2 = line;
            }
      }

//---------------------------------------------------------
//   layoutNoteHeads
//---------------------------------------------------------

/**
 For \a staff set line & accidental & mirror for notes depending
 on context.
*/

void Measure::layoutNoteHeads(int staff)
      {
      char tversatz[74];      // list of already set accidentals for this measure

      int key  = _score->staff(staff)->keymap()->key(tick());
      initLineList(tversatz, key);

      for (Segment* segment = first(); segment; segment = segment->next()) {
            int startTrack = staff * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (e == 0 || e->type() != CHORD)
                        continue;
                  Chord* chord   = (Chord*)e;
                  layoutChord(chord, tversatz);
                  }
            }
      }

//---------------------------------------------------------
//   Measure::layout
//---------------------------------------------------------

/**
 Layout measure; must fit into  \a width.

 Note: minWidth = width - stretch
*/

void Measure::layout(ScoreLayout* layout, double width)
      {
      if (first() == 0)
            return;

      double _spatium = layout->spatium();
      int nstaves     = _score->nstaves();
      double staffY[nstaves];

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            staffY[staffIdx] = system()->staff(staffIdx)->bbox().y();
            staves[staffIdx].distance = 0.0;
            }

      // height of boundingRect will be set
      // in System::layout2()
      setbbox(QRectF(0, 0, width, system()->height()));

      layoutX(layout, width);

      //---------------------------------------------------
      //   layout Chords/Lyrics/Symbols/BeginRepeatBar
      //---------------------------------------------------

      int tracks = nstaves * VOICES;
      for (Segment* segment = first(); segment; segment = segment->next()) {
            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (e == 0)
                        continue;
                  if (e->isChordRest())
                        e->layout(layout);
                  }
            for (int staff = 0; staff < nstaves; ++staff) {
                  LyricsList* ll = segment->lyricsList(staff);
                  int line = 0;
                  for (iLyrics i = ll->begin(); i != ll->end(); ++i, ++line) {
                        Lyrics* lyrics = *i;
                        if (lyrics == 0)
                              continue;
                        lyrics->layout(layout);
                        // center to middle of notehead:
                        double noteHeadWidth = symbols[quartheadSym].width();
                        double lh = lyrics->lineSpacing();
                        double y  = lh * line + point(score()->style()->lyricsDistance);
                        lyrics->setPos(noteHeadWidth/2 - lyrics->bbox().width() * .5,
                           y + staffY[staff]);

                        // increase staff distance if necessary
                        y += _spatium * 4;
                        if ((staff+1) < nstaves) {
                              if (y > staves[staff].distance) {
                                    staves[staff].distance = y;
                                    }
                              }
                        }
                  }
            }
      if (_noText)
            _noText->layout(layout);
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
            if (s->subtype() != Segment::SegChordRest)
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
            tick2 = tick() + _score->sigmap->ticksMeasure(tick());
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
//---------------------------------------------------------

void Measure::layout2(ScoreLayout* layout)
      {
//      printf("Measure(%p)::layout2(): _sel.size = %d\n", this, _sel.size());

      foreach(Element* element, _sel) {
            int staff = element->staff()->idx();
            double y  = system()->staff(staff)->bbox().y();
            element->layout(layout);

            switch(element->type()) {
                  case IMAGE:
                        element->setPos(QPointF(tick2pos(element->tick()), y));
                        break;
                  case DYNAMIC:
                  case SYMBOL:
                  case TEMPO_TEXT:
                  case TEXT:
                        element->setPos(element->ipos()
                           + QPointF(tick2pos(element->tick()), y));
                        break;
                  case LAYOUT_BREAK:
                        element->setPos(
                           bbox().width() - element->bbox().width() - _spatium,
                           - element->bbox().height() - _spatium);
                        break;
                  case REPEAT:
                        {
                        TextStyle* st = score()->textStyle(TEXT_STYLE_REPEAT);
                        double xo = st->xoff;
                        double yo = st->yoff;
                        if (st->offsetType == OFFSET_SPATIUM) {
                              xo *= _spatium;
                              yo *= _spatium;
                              }
                        switch (element->subtype()) {
                              case RepeatSegno:
                              case RepeatCoda:
                              case RepeatVarcoda:
                              case RepeatCodetta:
                                    xo += (element->width()*.5);
                                    yo += (element->bbox().height()*-.5);
                                    break;
                              case RepeatFine:
                                    yo = (yo*-1)+(bbox().height()*.5)+(element->bbox().height());
                                    xo -= (element->width()*.5);
                              default:
                                    xo -= element->width();
                                    xo += bbox().width();
                                    break;
                              }
                        element->setPos(QPointF(xo, yo));
                        }
                        break;
                  case SLUR:
                        // slur->layout() messes with add()/remove()
                  default:
                        break;
                  }

            }

      //
      //   set measure number
      //
      int pn = _no + _noOffset;
      QString s = QString("%1").arg(pn + 1);

      QString ns;
      if (score()->style()->showMeasureNumber
         && !_irregular
         && (pn || score()->style()->showMeasureNumberOne)) {
            if (score()->style()->measureNumberSystem) {
                  if (system() && !system()->measures().empty() && system()->measures().front() == this)
                        ns = s;
                  }
            else if ((pn % score()->style()->measureNumberInterval) == 0)
                  ns = s;
            }
      _noText->setText(ns);
      _noText->layout(layout);

      int tracks = _score->nstaves() * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int track = 0; track < tracks; ++track) {
                  Element* el = s->element(track);
                  if (el && (el->type() == CHORD)) {
                        Chord* a = (Chord*)el;
                        const NoteList* nl = a->noteList();
                        for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                              Tie* tie = in->second->tieFor();
                              if (tie)
                                    tie->layout(layout);
                              }
                        }
                  }
            }
      foreach(Tuplet* tuplet, _tuplets)
            tuplet->layout(layout);
      }

//---------------------------------------------------------
//   findChord
//---------------------------------------------------------

/**
 Search for chord at position \a tick at \a staff in \a voice.
*/

Chord* Measure::findChord(int tick, int staff, int voice, bool /*grace*/)
      {
      for (Segment* seg = _first; seg; seg = seg->next()) {
            if (seg->tick() > tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(staff * VOICES + voice);
                  if (el && el->type() == CHORD) {
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

Segment* Measure::tick2segment(int tick) const
      {
      for (Segment* s = first(); s; s = s->next()) {
            if ((s->subtype() == Segment::SegChordRest) && (s->tick() == tick))
                  return s;
            }
      return 0;
      }

//---------------------------------------------------------
//   findSegment
//---------------------------------------------------------

Segment* Measure::findSegment(Segment::SegmentType st, int t)
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

Segment* Measure::createSegment(Segment::SegmentType st, int t)
      {
      Segment* newSegment = new Segment(this, t);
      newSegment->setSubtype(st);
      return newSegment;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e)
      {
      Segment::SegmentType st = Segment::segmentType(e->type());
      return getSegment(st, e->tick());
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Segment::SegmentType st, int t)
      {
      Segment* s = findSegment(st, t);
      if (!s) {
            s = createSegment(st, t);
            add(s);
            }
      return s;
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

/**
 Add new Element \a el to Measure.
*/

void Measure::add(Element* el)
      {
      Staff* staffp = el->staff();
      if (el->type() != SEGMENT && staffp == 0) {
            _pel.push_back(el);
            el->setAnchorMeasure(this);
            return;
            }
      el->setParent(this);
      int t = el->tick();
      ElementType type = el->type();

// printf("measure %p: add %s %p, staff %d staves %d\n",
//      this, el->name(), el, staffIdx, staves.size());

      switch (type) {
            case BEAM:
                  _pel.push_back(el);
                  el->setAnchorMeasure(this);
                  break;
            case SEGMENT:
                  {
                  Segment* s;
                  for (s = first(); s && s->tick() < t; s = s->next())
                        ;
                  int st = el->subtype();
                  if (s) {
                        if (st == Segment::SegChordRest) {
                              while (s && s->subtype() != st && s->tick() == t) {
                                    if (s->subtype() == Segment::SegEndBarLine)
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
                              }
                        }
                  insert((Segment*)el, s);
                  }
                  break;
            case TUPLET:
                  _tuplets.append((Tuplet*)el);
                  break;
            case LAYOUT_BREAK:
                  for (iElement i = _sel.begin(); i != _sel.end(); ++i) {
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
                  _sel.push_back(el);
                  break;
            case SLUR:
                  {
                  SlurTie* s = (SlurTie*)el;
                  ElementList* sl = s->elements();
                  for (iElement i = sl->begin(); i != sl->end(); ++i) {
                        _sel.append(*i);
                        }
                  }
                  _sel.append(el);
                  break;

            case VOLTA:
                  switch(el->subtype()) {
                        case PRIMA_VOLTA:
                              _ending = 1; // value changed by DK. 28.08.07
                              break;
                        case SECONDA_VOLTA:
                        case SECONDA_VOLTA2:
                              _ending = 2; // value changed by DK. 28.08.07
                              break;
                        case TERZA_VOLTA:
                              _ending = 3; // value changed by DK. 28.08.07
                              break;
                        };
                  _sel.append(el);
                  break;

            case REPEAT:
                  _repeatFlags |= el->subtype();
                  // fall through

            case OTTAVA:
            case PEDAL:
            case TRILL:
            case HAIRPIN:
            case DYNAMIC:
            case SYMBOL:
            case TEXT:
            case TEMPO_TEXT:
            case IMAGE:
                  _sel.append(el);
                  break;
            case SLUR_SEGMENT:
// printf("measure(%p) add slur segment %p\n", this, el);
                  _sel.append(el);
                  break;

            case BAR_LINE:
                  {
                  Segment* seg = getSegment(Segment::SegEndBarLine, tick() + tickLen());
                  seg->add(el);
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
      switch(el->type()) {
            case SEGMENT:
                  remove((Segment*)el);
                  break;
            case TUPLET:
                  {
                  int idx = _tuplets.indexOf((Tuplet*)el);
                  if (idx == -1) {
                        printf("Measure remove: Tuplet not found\n");
                        return;
                        }
                  _tuplets.removeAt(idx);
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
                  if (!_sel.remove(el))
                        printf("Measure(%p)::remove(%s,%p) not found\n",
                           this, el->name(), el);
                  break;
            case SLUR:
                  {
                  Slur* s = (Slur*)el;
                  ElementList* sl = s->elements();
                  for (iElement i = sl->begin(); i != sl->end(); ++i) {
                        remove(*i);
                        }
                  if (!_sel.remove(s))
                        printf("Measure(%p)::remove(%s,%p) not found\n",
                           this, el->name(), el);
                  }
                  break;

            case VOLTA:
                  _ending = 0;
                  if (!_sel.remove(el))
                        printf("Measure(%p)::remove(%s,%p) not found\n",
                           this, el->name(), el);
                  break;

            case REPEAT:
                  _repeatFlags &= ~el->subtype();
                  // fall through:

            case SLUR_SEGMENT:
            case DYNAMIC:
            case HAIRPIN:
            case TEMPO_TEXT:
            case TEXT:
            case SYMBOL:
            case OTTAVA:
            case PEDAL:
            case TRILL:
            case IMAGE:
                  if (!_sel.remove(el)) {
                        if (!_pel.remove(el))
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

//---------------------------------------------------------
//   moveTicks
//    also adjust endBarLine if measure len has changed
//---------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      foreach(Element* e, _sel) {
            e->setTick(e->tick() + diff);
            if (e->type() == SLUR) {
                  Slur* slur = (Slur*)e;
                  if (slur->tick1() >= tick())
                        slur->setTick1(slur->tick1() + diff);
                  if (slur->tick2() >= tick())
                        slur->setTick2(slur->tick2() + diff);
                  }
            }
      setTick(tick() + diff);
      int staves = _score->nstaves();
      int tracks = staves * VOICES;
      for (Segment* segment = first(); segment; segment = segment->next()) {
            int ltick;
            if ((segment->subtype() == Segment::SegEndBarLine)
               || (segment->subtype() == Segment::SegTimeSigAnnounce))
                  ltick = tick() + tickLen();
            else
                  ltick = segment->tick() + diff;
            segment->setTick(ltick);

            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (e)
                        e->setTick(e->tick() + diff);
                  }
            for (int staff = 0; staff < staves; ++staff) {
                  const LyricsList* ll = segment->lyricsList(staff);
                  for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                        if (*i)
                              (*i)->setTick((*i)->tick() + diff);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   moveY
//---------------------------------------------------------

void Measure::moveY(int staff, double dy)
      {
      for (Segment* segment = first(); segment; segment = segment->next()) {
            for (int track = staff*VOICES; track < staff*VOICES+VOICES; ++track) {
                  Element* e = segment->element(track);
                  if (e)
                        e->move(0, dy);
                  }
            LyricsList* ll = segment->lyricsList(staff);
            foreach(Lyrics* ly, *ll)
                  ly->move(0, dy);
            }
      BarLine* bl = barLine(staff);
      if (bl) {
            if (_score->staff(staff)->isTopSplit()) {
                  Spatium barLineLen = Spatium(8) + score()->style()->staffDistance;
                  bl->setHeight(point(barLineLen));
                  }
            }
//      foreach(Beam* beam, _beamList)
//            beam->move(0, dy);
      foreach(Tuplet* tuplet, _tuplets)
            tuplet->move(0, dy);
      if (_noText)
            _noText->move(0, dy);
      }

//---------------------------------------------------------
//   Space
//---------------------------------------------------------

/**
 Unit of horizontal measure.
*/

class Space {
      double _min;      // minimum width
      double _extra;    // left bearing
      bool _valid;

   public:
      Space()                       { _valid = false; _min = 0.0; _extra = 0.0; }
      bool valid() const            { return _valid; }
      void setValid(bool val)       { _valid = val; }
      double min() const            { return _min; }
      double extra() const          { return _extra; }
      void setExtra(double e)       { _extra = e; }
      void setMin(double m)         { _min = m; }
      void addMin(double m)         { _min += m; }
      void max(const Space& s) {
            if (s._min > _min) {
                  _min = s._min;
                  }
            if (s._extra > _extra)
                  _extra = s._extra;
            }
      void maxMin(Space v) {
            if (_min < v._min) {
                  _min = v._min;
                  }
            }
      };

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

static SpringMap springs;

//---------------------------------------------------------
//   sff
//    compute 1/Force for a given Extend
//---------------------------------------------------------

static double sff(double x, double xMin)
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

//---------------------------------------------------------
//   Measure::layoutX
//---------------------------------------------------------

/**
 Return width of measure, taking into account \a stretch.
*/

void Measure::layoutX(ScoreLayout* layout, double stretch)
      {
      int nstaves = _score->nstaves();
      int tracks  = nstaves * VOICES;

      int segs = size();

      if (nstaves == 0 || segs == 0) {
            _mw = MeasureWidth(1.0, 0.0);
            return;
            }

      Style* style = score()->style();

      //-----------------------------------------------------------------------
      //    remove empty segments
      //-----------------------------------------------------------------------

again:
      for (Segment* s = first(); s; s = s->next()) {
            if (s->isEmpty()) {
//                  printf("Measure::layoutX(): note: remove empty segment %p\n", s);
                  remove(s);
                  goto again;
                  }
            }

      segs = size();
      if (segs == 0) {
            _mw = MeasureWidth(1.0, 0.0);
            return;
            }

      //-----------------------------------------------------------------------
      //    fill array of Spaces for all segments and staves
      //    spaces[0]      - left margin
      //    // spaces[segs+1] - right margin
      //-----------------------------------------------------------------------

      Space spaces[segs+1][nstaves];
      double ypos[tracks];
      double width[segs+1];

      int seg = 1;
      bool notesSeg = first()->subtype() == Segment::SegChordRest;
      bool firstNoteRest = true;
      for (const Segment* s = first(); s; s = s->next(), ++seg) {
            //
            // add extra space between clef/key/timesig and first notes
            //
            double additionalMin   = 0.0;
            double additionalExtra = 0.0;
            if (!notesSeg && s->next() && s->next()->subtype() == Segment::SegChordRest) {
                  additionalMin = point(style->clefKeyRightMargin);
                  notesSeg = true;
                  }
            if (s->subtype() == Segment::SegChordRest) {
                  if (firstNoteRest)
                        firstNoteRest = false;
                  else
                        additionalExtra = point(style->minNoteDistance);
                  }
            else if (s->subtype() == Segment::SegClef)
                  additionalExtra = point(style->clefLeftMargin);
            else if (s->subtype() == Segment::SegTimeSig)
                  additionalExtra = point(style->timesigLeftMargin);
            else if (s->subtype() == Segment::SegKeySig)
                  additionalExtra = point(style->keysigLeftMargin);
            else if (s->subtype() == Segment::SegEndBarLine)
                  additionalExtra = point(style->barNoteDistance);
            else if (s->subtype() == Segment::SegTimeSigAnnounce) {
                  // additionalExtra = point(style->timesigLeftMargin);
                  additionalMin   = point(Spatium(1.0));
                  }

            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  Staff* staff = score()->staff(staffIdx);
                  spaces[seg][staffIdx].setValid(false);
                  if (!staff->show())
                        continue;

                  double min   = 0.0;
                  double extra = 0.0;
                  for (int voice = 0; voice < VOICES; ++voice) {
                        Element* el  = s->element(staffIdx * VOICES + voice);
                        if (!el)
                              continue;
                        spaces[seg][staffIdx].setValid(true);
                        el->layout(layout);
                        double min1, extra1;
                        el->space(min1, extra1);
                        if (min1 > min)
                              min = min1;
                        if (extra1 > extra)
                              extra = extra1;
                        }

                  const LyricsList* ll = s->lyricsList(staffIdx);
                  for (ciLyrics l = ll->begin(); l != ll->end(); ++l) {
                        if (!*l)
                              continue;
                        (*l)->layout(layout);
                        double lw = ((*l)->bbox().width() + _spatium * 0) * .5;
                        if (lw > min)
                              min = lw;
                        if (lw > extra)
                              extra = lw;
                        spaces[seg][staffIdx].setValid(true);
                        }
                  spaces[seg][staffIdx].setMin(min + additionalMin);
                  spaces[seg][staffIdx].setExtra(extra + additionalExtra);
                  }
            }
      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            Spatium min;
            Spatium extra;
            switch(first()->subtype()) {
                  case Segment::SegClef:
                        min = style->clefLeftMargin;
                        break;
                  case Segment::SegKeySig:
                        min = style->keysigLeftMargin;
                        break;
                  case Segment::SegTimeSigAnnounce:
                  case Segment::SegTimeSig:
                        min = style->timesigLeftMargin;
                        break;
                  case Segment::SegChordRest:
                        min = style->barNoteDistance;
                        break;
                  }
            spaces[0][staffIdx].setMin(point(min));
            spaces[0][staffIdx].setExtra(point(extra));
            spaces[0][staffIdx].setValid(true);
            }

      //---------------------------------------------------
      //    move extra space to previous cells
      //---------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
            for (int seg = segs; seg > 0; --seg) {    // seg 0 cannot move any space
                  double extra = spaces[seg][staffIdx].extra();
                  if (extra < 0.00001)
                        continue;
                  // move extra space to previous non empty Segment
                  int tseg;
                  for (tseg = seg-1; tseg >= 0; --tseg) {
                        if (spaces[tseg][staffIdx].valid())
                              break;
                        }
                  if (tseg == 0) {
                        if (spaces[tseg][staffIdx].min() < extra)
                              spaces[tseg][staffIdx].setMin(extra);
                        }
                  else
                        spaces[tseg][staffIdx].addMin(extra);
                  }
            }

      //---------------------------------------------------
      //    populate width[] array
      //---------------------------------------------------

      for (int seg = segs; seg >= 0; --seg) {
            double ww = 0.0;
            for (int staffIdx = 0; staffIdx < nstaves; ++staffIdx) {
                  if (!spaces[seg][staffIdx].valid())
                        continue;
                  double w = spaces[seg][staffIdx].min();
                  for (int nseg = seg+1; nseg < segs+1; ++nseg) {
                        if (spaces[nseg][staffIdx].valid())
                              break;
                        w -= width[nseg];
                        if (w < 0.0)
                              break;
                        }
                  if (w > ww)
                        ww = w;
                  }
            width[seg] = ww;
            }

      //---------------------------------------------------
      //    segments with equal duration should have
      //    equal width
      //---------------------------------------------------

      int ticks[segs+1];
      memset(ticks, 0, (segs + 1) * sizeof(int));

      //--------tick table for segments
      int minTick   = 100000;
      int ntick     = tick() + tickLen();   // position of next measure
      int i         = 1;
      for (Segment* seg = first(); seg; seg = seg->next(), ++i) {
            if (seg->subtype() == Segment::SegChordRest) {
                  Segment* nseg = seg;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->subtype() == Segment::SegChordRest)
                              break;
                        }
                  int nticks;
                  if (nseg)
                        nticks = nseg->tick() - seg->tick();
                  else
                        nticks = ntick - seg->tick();
                  if (nticks == 0) {
                        printf("layoutX: nticks==0 tickLen %d <%s> size %d, idx %d, %d %d  %p %p nseg <%s>\n",
                           tickLen(), seg->name(),
                           size(), i-1, seg->tick(), nseg ? nseg->tick() : 0, seg, nseg,
                           nseg ? nseg->name() : "nil");
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        }
                  ticks[i] = nticks;
                  }
            }

      // compute stretches:

      springs.clear();
      double stretchList[segs + 1];
      double stretchSum   = 0.0;
      stretchList[0]      = 0.0;

      double minimum = width[0];  // + width[segs+1];
      for (int i = 1; i < segs+1; ++i) {
            double str = 1.0;
            double d;
            if (ticks[i] > 0) {
                  if (minTick > 0)
                        str += .6 * log2(double(ticks[i]) / double(minTick));
                  stretchList[i] = str;
                  d = width[i] / str;
                  }
            else {
                  stretchList[i] = 0.0;   // dont stretch timeSig and key
                  d = 100000000.0;        // CHECK
                  }
            stretchSum += stretchList[i];
            minimum += width[i];
            springs.insert(std::pair<double, Spring>(d, Spring(i, stretchList[i], width[i])));
            }

      //---------------------------------------------------
      //    distribute "stretch" to segments
      //---------------------------------------------------

      double force = sff(stretch, minimum);
      for (iSpring i = springs.begin(); i != springs.end(); ++i) {
            double stretch = force * i->second.stretch;
            if (stretch < i->second.fix)
                  stretch = i->second.fix;
            width[i->second.seg] = stretch;
            }

      //-----------------------------------------------------------------------
      //    xpos[segs+2]   - start of next measure (width of current measure)
      //-----------------------------------------------------------------------

      double xpos[segs+2];
      xpos[0] = 0.0;
      for (int seg = 0; seg < segs+1; ++seg)
            xpos[seg+1] = xpos[seg] + width[seg];

      if (stretch == 1.0) {
            // printf("this is pass 1\n");
            _mw = MeasureWidth(xpos[segs + 1], 0.0);
            return;
            }

      //---------------------------------------------------
      //    populate ypos[] array
      //---------------------------------------------------

      for (int staff = 0; staff < nstaves; ++staff)
            ypos[staff] = system()->staff(staff)->bbox().y();

      //---------------------------------------------------
      //    layout individual elements
      //---------------------------------------------------

      seg = 1;
      for (Segment* s = first(); s; s = s->next(), ++seg) {
            s->setPos(xpos[seg], 0.0);
            for (int staff = 0; staff < tracks; ++staff) {
                  Element* e = s->element(staff);
                  if (e == 0)
                        continue;
                  double y = ypos[staff/VOICES];
                  QPointF pos(0.0, y);
                  ElementType t = e->type();
                  if (t == REST) {
                        double y = ypos[staff/VOICES + ((Rest*)e)->move()];
                        //
                        // center symbol if its a whole measure rest
                        //
                        double yoffset = 0.0;
                        if (!_irregular && (e->tickLen() == 0)) {
                              // on pass 2 stretch is the real width of the measure
                              // its assumed that s is the last segment in the measure
                              pos.setX((stretch - s->x() - e->width()) * .5);
                              yoffset = -_spatium;
                              }
                        if (e->voice() == 1)          // TODO: check ??
                              y += -1 * _spatium;
                        else
                              y += 2 * _spatium;
                        e->setPos(pos.x(), y + yoffset);
                        }
                  else if (t == REPEAT_MEASURE) {
                        e->setPos((stretch - s->x() - e->width()) * .5,
                           ypos[staff/VOICES] + _spatium);
                        }
                  else if (t == CHORD) {
                        int move = 0; // TODO ((ChordRest*)e)->translate();
                        double y = ypos[staff/VOICES + move];
                        if (((Chord*)e)->grace()) {
                              double min, extra;
                              ((Chord*)e)->space(min, extra);
                              e->setPos(- min, y);
                              }
                        else {
                              e->setPos(0.0, y);
                              }
                        }
                  else {
                        double xo = spaces[seg][staff/VOICES].extra();
                        if (t == CLEF)
                              e->setPos(-e->bbox().x() - xo + point(style->clefLeftMargin), y);
                        else if (t == TIMESIG)
                              e->setPos(- e->bbox().x() - xo + point(style->timesigLeftMargin), y);
                        else if (t == KEYSIG)
                              e->setPos(- e->bbox().x() - xo + point(style->keysigLeftMargin), y);
                        else  if (s->subtype() == Segment::SegEndBarLine) {
                              e->setPos(0.0, y);
                              }
                        else
                              e->setPos(- e->bbox().x() - xo, y);
                        }
                  }
            }
      _mw = MeasureWidth(xpos[segs + 1], 0.0);
      }

//---------------------------------------------------------
//   setNoText
//---------------------------------------------------------

void Measure::setNoText(const QString& s)
      {
      _noText->setText(s);
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
      }

//---------------------------------------------------------
//   insertStaves
//---------------------------------------------------------

void Measure::insertStaves(int sStaff, int eStaff)
      {
      for (Segment* s = _first; s; s = s->next()) {
            for (int staff = sStaff; staff < eStaff; ++staff) {
                  s->insertStaff(staff);
                  }
            }
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
                  if (el && !el->generated()) {
                        _score->undoOp(UndoOp::RemoveElement, el);
                        }
                  }
            }
      _score->undoOp(UndoOp::RemoveStaves, this, sStaff, eStaff);
      removeStaves(sStaff, eStaff);

      for (int i = eStaff - 1; i >= sStaff; --i)
            _score->undoOp(UndoOp::RemoveMStaff, this, *(staves.begin() + i), i);
      staves.erase(staves.begin() + sStaff, staves.begin() + eStaff);

      // BeamList   _beamList;
      // TupletList _tuplets;
      // barLine
      // TODO
      }

//---------------------------------------------------------
//   cmdAddStaves
//---------------------------------------------------------

void Measure::cmdAddStaves(int sStaff, int eStaff)
      {
      _score->undoOp(UndoOp::InsertStaves, this, sStaff, eStaff);
      insertStaves(sStaff, eStaff);

      Segment* ts = findSegment(Segment::SegTimeSig, tick());

      for (int i = sStaff; i < eStaff; ++i) {
            Staff* staff = _score->staff(i);
            MStaff ms;

            staves.insert(i, ms);
            _score->undoOp(UndoOp::InsertMStaff, this, ms, i);

            Rest* rest = new Rest(score(), tick(), 0);
            rest->setStaff(staff);
            Segment* s = findSegment(Segment::SegChordRest, tick());
            rest->setParent(s);
            score()->undoAddElement(rest);

            // replicate time signature
            if (ts) {
                  TimeSig* timesig = new TimeSig(*(TimeSig*)ts->element(0));
                  timesig->setStaff(staff);
                  score()->undoAddElement(timesig);
                  }
            }
      }

//---------------------------------------------------------
//   insertMStaff
//---------------------------------------------------------

void Measure::insertMStaff(MStaff staff, int idx)
      {
      staves.insert(idx, staff);
      }

//---------------------------------------------------------
//   removeMStaff
//---------------------------------------------------------

void Measure::removeMStaff(MStaff /*staff*/, int idx)
      {
      staves.removeAt(idx);
      }

//---------------------------------------------------------
//   insertStaff
//---------------------------------------------------------

void Measure::insertStaff(Staff* staff, int staffIdx)
      {
      insertStaff1(staff, staffIdx);
      Rest* rest = new Rest(score(), tick(), _score->sigmap->ticksMeasure(tick()));
      rest->setStaff(staff);
      }

//---------------------------------------------------------
//   insertStaff1
//---------------------------------------------------------

void Measure::insertStaff1(Staff* /*staff*/, int staffIdx)
      {
      for (Segment* s = _first; s; s = s->next())
            s->insertStaff(staffIdx);

      MStaff ms;
      ms.distance = point(staffIdx == 0 ? score()->style()->systemDistance : score()->style()->staffDistance);
      staves.insert(staves.begin()+staffIdx, ms);
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

bool Measure::acceptDrop(Viewer* viewer, const QPointF& p, int type,
   const QDomElement&) const
      {
      // convert p from canvas to measure relative position and take x and y coordinates
      QPointF mrp = p - pos() - system()->pos() - system()->page()->pos();
      double mrpx = mrp.x();
      double mrpy = mrp.y();

      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1)
            return false;                       // staff not found
      QRectF sb(s->staff(idx)->bbox());
      qreal t = sb.top();    // top of staff
      qreal b = sb.bottom(); // bottom of staff

      // compute rectangle of staff in measure
      QRectF rrr(sb.translated(s->canvasPos()));
      QRectF rr(abbox());
      QRectF r(rr.x(), rrr.y(), rr.width(), rrr.height());

      switch(type) {
            case REPEAT:
                  viewer->setDropRectangle(rr);
                  return true;

            case BRACKET:
            case LAYOUT_BREAK:
            case REPEAT_MEASURE:
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
                        if (seg->subtype() != Segment::SegChordRest)
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
                  for (Segment* seg = _first; seg; seg = seg->next())
                        if (seg->subtype() == Segment::SegChordRest)
                              return (mrpx < seg->pos().x());
                  // fall through if no chordrest segment found

            default:
                  return false;
            }
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

/**
 Handle a dropped element at position \a pos of given element \a type and \a subtype.
*/

Element* Measure::drop(const QPointF& p, const QPointF& /*offset*/, int type, const QDomElement& e)
      {
      // determine staff
      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1)
            return 0;
      Staff* staff = score()->staff(idx);

      // convert p from canvas to measure relative position and take x coordinate
      QPointF mrp = p - pos() - system()->pos() - system()->page()->pos();

      switch(ElementType(type)) {
            case REPEAT:
                  {
                  Repeat* repeat = new Repeat(score());
                  repeat->read(e);
                  repeat->setParent(this);
                  repeat->setStaff(score()->staff(0));
                  score()->cmdAdd(repeat);
                  }
                  break;
            case BRACKET:
                  {
                  Bracket* bracket = new Bracket(score());
                  bracket->read(e);
                  int subtype = bracket->subtype();
                  delete bracket;
                  staff->addBracket(BracketItem(subtype, 1));
                  }
                  break;
            case CLEF:
                  {
                  Clef* clef = new Clef(score());
                  clef->read(e);
                  staff->changeClef(tick(), clef->subtype());
                  delete clef;
                  }
                  break;
            case KEYSIG:
                  {
                  KeySig* ks = new KeySig(score());
                  ks->read(e);
                  char newSig = ks->subtype() & 0xff;
                  if (newSig < -7 || newSig > 7) {
                        printf("illegal keysig %d\n", newSig);
                        abort();
                        }
                  staff->changeKeySig(tick(), newSig);
                  delete ks;
                  }
                  break;
            case TIMESIG:
                  {
                  TimeSig* ts = new TimeSig(score());
                  ts->read(e);
                  score()->changeTimeSig(tick(), ts->subtype());
                  delete ts;
                  }
                  break;
            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(e);
                  if ((lb->subtype() == LAYOUT_BREAK_PAGE && _pageBreak)
                     || (lb->subtype() == LAYOUT_BREAK_LINE && _lineBreak)) {
                        //
                        // if break already set
                        //
                        delete lb;
                        break;
                        }
                  lb->setStaff(staff);
                  lb->setParent(this);
                  score()->cmdAdd(lb);
                  return lb;
                  }

            case BAR_LINE:
                  {
                  BarLine* bl = new BarLine(score());
                  bl->read(e);
                  switch(bl->subtype()) {
                        case END_BAR:
                        case NORMAL_BAR:
                        case DOUBLE_BAR:
                        case BROKEN_BAR:
                              {
                              score()->undoChangeRepeatFlags(this, _repeatFlags & ~RepeatEnd);
                              if (next())
                                    score()->undoChangeRepeatFlags(next(), next()->repeatFlags() & ~RepeatStart);
                              score()->undoChangeEndBarLine(this, bl->subtype());
                              BarLine* bl = endBarLine();
                              if (bl)
                                    bl->setGenerated(false);
                              }
                              break;
                        case START_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatStart);
                              break;
                        case END_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatEnd);
                              if (next())
                                    score()->undoChangeRepeatFlags(next(), next()->repeatFlags() & ~RepeatStart);
                              break;
                        case END_START_REPEAT:
                              score()->undoChangeRepeatFlags(this, _repeatFlags | RepeatEnd);
                              if (next())
                                    score()->undoChangeRepeatFlags(next(), next()->repeatFlags() | RepeatStart);
                              break;
                        }
                  delete bl;
                  }
                  break;

            case REPEAT_MEASURE:
                  {
                  //
                  // see also cmdDeleteSelection()
                  //
                  _score->select(0, 0, 0);
                  bool rmFlag = false;
                  for (Segment* s = first(); s; s = s->next()) {
                        if (s->subtype() == Segment::SegEndBarLine
                           || s->subtype() == Segment::SegTimeSigAnnounce
                           || s->subtype() == Segment::SegStartRepeatBarLine)
                              continue;
                        if (s->subtype() == Segment::SegChordRest)
                              rmFlag = true;
                        if (rmFlag) {
                              int strack = idx * VOICES;
                              int etrack = strack + VOICES;
                              for (int track = strack; track < etrack; ++track) {
                                    Element* el = s->element(track);
                                    if (el)
                                          _score->undoRemoveElement(el);
                                    }
                              }
                        if (s->isEmpty()) {
                              _score->undoRemoveElement(s);
                              }
                        }
                  //
                  // add repeat measure
                  //

                  Segment* seg = findSegment(Segment::SegChordRest, tick());
                  if (seg == 0) {
                        seg = createSegment(Segment::SegChordRest, tick());
                        _score->undoAddElement(seg);
                        }
                  RepeatMeasure* rm = new RepeatMeasure(_score);
                  rm->setTick(tick());
                  rm->setStaff(staff);
                  rm->setParent(seg);
                  _score->undoAddElement(rm);
                  foreach(Element* el, _sel) {
                        if (el->type() == SLUR && el->staffIdx() == idx)
                              _score->undoRemoveElement(el);
                        }
                  _score->select(rm, 0, 0);
                  }
                  break;

            default:
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
      QAction* a = popup->addAction(QT_TR_NOOP("set irregular..."));
      a->setData("irregular");
      return true;
      }

//---------------------------------------------------------
//   propertyAction
//---------------------------------------------------------

void Measure::propertyAction(const QString& s)
      {
      if (s == "irregular") {
            IrregularMeasureDialog im(this);
            if (!im.exec())
                  return;

            setIrregular(im.isIrregular());     // TODO: shall we make this undoable?
            score()->setDirty();
            score()->select(0, 0, 0);
            SigList* sl = score()->sigmap;

            SigEvent oev = sl->timesig(tick());
            SigEvent newEvent = im.sig();
            if (oev == newEvent)
                  return;

            int oldLen = oev.ticks;
            int newLen = newEvent.ticks;

            SigEvent oldEvent;
            iSigEvent i = sl->find(tick());
            if (i != sl->end())
                  oldEvent = i->second;

//            printf("1.change sig at %d %s -> %s\n", tick(),
//               qPrintable(oldEvent.print()),
//               qPrintable(newEvent.print()));
            score()->undoChangeSig(tick(), oldEvent, newEvent);

            //
            // change back to nominal values if there is
            // not already another TimeSig
            //
            i = sl->find(tick() + oldLen);
            if (i == sl->end()) {
//                  printf("2.add sig at %d %s\n", tick() + newLen,
//                     qPrintable(oev.print()));
                  score()->undoChangeSig(tick() + newLen, SigEvent(), oev);
                  }
            adjustToLen(oldLen, newLen);
            score()->fixTicks();
            score()->select(this, 0, 0);
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
            Staff* staffp = score()->staff(staffIdx);
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
                              rest = (Rest*)e;
                              }
                        else if (e && e->type() == CHORD)
                              ++chords;
                        }
                  }
            // printf("rests = %d\n", rests);
            if (rests == 1 && chords == 0) {
                  rest->setTickLen(0);    // whole measure rest
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
                                    if (e && (e->type() == CHORD || e->type() == REST)) {
                                          n += e->tickLen();
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
                              Segment* seg = findSegment(Segment::SegChordRest, rtick);
                              if (seg == 0) {
                                    seg = createSegment(Segment::SegChordRest, rtick);
                                    score()->undoAddElement(seg);
                                    }
                              rest = new Rest(score(), rtick, n);
                              rest->setStaff(staffp);
                              rest->setVoice(voice);
                              seg->add(rest);
                              }
                        }
                  }
            }
      score()->undoChangeMeasureLen(this, nl);
      if (diff < 0) {
            //
            //  CHECK: do not remove all slurs
            //
            foreach(Element* e, _sel) {
                  if (e->type() == SLUR)
                        score()->undoRemoveElement(e);
                  }
            score()->cmdRemoveTime(tick() + nl, -diff);
            }
      else
            score()->undoInsertTime(tick() + ol, diff);
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Measure::write(Xml& xml, int no, int staff) const
      {
      if (xml.curTick != tick())
            xml.stag(QString("Measure number=\"%1\" tick=\"%2\"").arg(no).arg(tick()));
      else
            xml.stag(QString("Measure number=\"%1\"").arg(no));
      xml.curTick = tick();

      if (staff == 0) {
            for (ciElement ie = _pel.begin(); ie != _pel.end(); ++ie)
                  (*ie)->write(xml);
            if (_repeatFlags & RepeatStart)
                  xml.tagE("startRepeat");
            if (_repeatFlags & RepeatEnd)
                  xml.tag("endRepeat", _endRepeat);
            if (_ending)
                  xml.tag("ending", _ending-1); //changed by DK. 28.08.07
            if (_irregular)
                  xml.tagE("irregular");
            if (_userStretch != 1.0)
                  xml.tag("stretch", _userStretch);
            int id = 0;
            foreach(Tuplet* tuplet, _tuplets)
                  tuplet->write(xml, id++);
            }

      for (ciElement i = _sel.begin(); i != _sel.end(); ++i) {
            if ((*i)->staff() == _score->staff(staff) && (*i)->type() != SLUR_SEGMENT)
                  (*i)->write(xml);
            }

      for (int track = staff * VOICES; track < staff * VOICES + VOICES; ++track) {
            for (Segment* segment = first(); segment; segment = segment->next()) {
                  Element* e = segment->element(track);
                  if (e && !e->generated()) {
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

      for (ciElement ie = _pel.begin(); ie != _pel.end(); ++ie)
            (*ie)->write(xml);
      if (_repeatFlags & RepeatStart)
            xml.tagE("startRepeat");
      if (_repeatFlags & RepeatEnd)
            xml.tag("endRepeat", _endRepeat);
      xml.tag("ending", _ending-1); //changed by DK. 28.08.07
      if (_irregular)
            xml.tagE("irregular");
      xml.tag("stretch", _userStretch);

      for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
            xml.stag("Staff");
            for (ciElement i = _sel.begin(); i != _sel.end(); ++i) {
                  if ((*i)->staff() == _score->staff(staffIdx) && (*i)->type() != SLUR_SEGMENT)
                        (*i)->write(xml);
                  }
            int id = 0;
            foreach(Tuplet* tuplet, _tuplets) {
                  if (staffIdx == tuplet->staffIdx())
                        tuplet->write(xml, id++);
                  }
            for (int track = staffIdx * VOICES; track < staffIdx * VOICES + VOICES; ++track) {
                  for (Segment* segment = first(); segment; segment = segment->next()) {
                        Element* e = segment->element(track);
                        if (e && !e->generated()) {
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
            MStaff s;
            s.distance = point(n == 0 ? score()->style()->systemDistance : score()->style()->staffDistance);
            s.userDistance = 0.0;
            staves.push_back(s);
            }

      int tck = e.attribute("tick", "-1").toInt();
      if (tck >= 0) {
            tck = score()->fileDivision(tck);
            setTick(tck);
            }
      Staff* staff     = _score->staff(idx);
      score()->curTick = tick();

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setParent(this);
                  barLine->read(e);
                  setEndBarLineType(barLine->subtype(), false);
                  delete barLine;
                  barLine = endBarLine();
                  if (barLine)
                        barLine->setGenerated(false);
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTick(score()->curTick);   // set default tick position
                  chord->setParent(this);             // only for reading tuplets
                  chord->setStaff(staff);
                  chord->read(e, idx);
                  Segment* s = getSegment(chord);
                  s->add(chord);
                  score()->curTick = chord->tick() + chord->tickLen();
                  }
            else if (tag == "Breath") {
                  Breath* breath = new Breath(score());
                  breath->setTick(score()->curTick);
                  breath->setStaff(staff);
                  breath->read(e);
                  Segment* s = getSegment(Segment::SegBreath, breath->tick());
                  s->add(breath);
                  score()->curTick = breath->tick();
                  }
            else if (tag == "Note") {
                  Chord* chord = new Chord(score());
                  chord->setTick(score()->curTick);   // set default tick position
                  chord->setParent(this);       // only for reading tuplets
                  chord->setStaff(staff);
                  chord->readNote(e, idx);
                  Segment* s = getSegment(chord);
                  s->add(chord);
                  score()->curTick = chord->tick() + chord->tickLen();
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setTick(score()->curTick);    // set default tick position
                  rest->setParent(this);        // only for reading tuplets
                  rest->setStaff(staff);
                  rest->read(e);
                  Segment* s = getSegment(rest);
                  s->add(rest);
                  score()->curTick = rest->tick() + rest->tickLen();
                  }
            else if (tag == "RepeatMeasure") {
                  RepeatMeasure* rm = new RepeatMeasure(score());
                  rm->setTick(score()->curTick);    // set default tick position
                  rm->setParent(this);
                  rm->setStaff(staff);
                  rm->read(e);
                  Segment* s = getSegment(Segment::SegChordRest, rm->tick());
                  s->add(rm);
                  score()->curTick = rm->tick() + rm->tickLen();
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTick(score()->curTick);
                  clef->setStaff(staff);
                  clef->read(e);
                  Segment* s = getSegment(clef);
                  s->add(clef);
                  score()->curTick = clef->tick();
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTick(score()->curTick);
                  ts->setStaff(staff);
                  ts->read(e);
                  Segment* s = getSegment(ts);
                  s->add(ts);
                  score()->curTick = ts->tick();
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTick(score()->curTick);
                  ks->setStaff(staff);
                  ks->read(e);
                  int oldSig = staff->keymap()->key(score()->curTick - 1);
                  ks->setSig(oldSig, ks->subtype() & 0xff);
                  Segment* s = getSegment(ks);
                  s->add(ks);
                  score()->curTick = ks->tick();
                  }
            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTick(score()->curTick);
                  dyn->setStaff(staff);
                  dyn->read(e);
                  add(dyn);
                  score()->curTick = dyn->tick();
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(score());
                  slur->setTick(score()->curTick);
                  slur->setStaff(staff);
                  slur->setParent(this);
                  slur->read(_score, e);
                  add(slur);
                  score()->curTick = slur->tick();
                  }
            else if (tag == "HairPin") {
                  Hairpin* hairpin = new Hairpin(score());
                  hairpin->setTick(score()->curTick);
                  hairpin->setStaff(staff);
                  hairpin->read(e);
                  add(hairpin);
                  score()->curTick = hairpin->tick();
                  }
            else if (tag == "Lyrics") {
                  Lyrics* lyrics = new Lyrics(score());
                  lyrics->setTick(score()->curTick);
                  lyrics->setStaff(staff);
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
                  t->setTick(score()->curTick);
                  t->read(e);
                  if (t->anchor() != ANCHOR_PAGE) {
                        score()->curTick = t->tick();
                        t->setStaff(staff);
                        }
                  add(t);
                  }

            //-------------------------------------obsolete:
            else if (tag == "work-title") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_TITLE);
                  t->read(e);
                  add(t);
                  }
            else if (tag == "creator-composer") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_COMPOSER);
                  t->read(e);
                  add(t);
                  }
            else if (tag == "work-number") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_SUBTITLE);
                  t->read(e);
                  add(t);
                  }
            //-------------------------------------
            else if (tag == "Tempo") {
                  TempoText* t = new TempoText(score());
                  t->setTick(score()->curTick);
                  t->setStaff(staff);
                  t->read(e);
                  add(t);
                  score()->curTick = t->tick();
                  }
            else if (tag == "Symbol") {
                  Symbol* sym = new Symbol(score());
                  sym->setTick(score()->curTick);
                  sym->setStaff(staff);
                  sym->read(e);
                  add(sym);
                  score()->curTick = sym->tick();
                  }
            else if (tag == "Ottava") {
                  Ottava* ottava = new Ottava(score());
                  ottava->setTick(score()->curTick);
                  ottava->setStaff(staff);
                  ottava->read(e);
                  add(ottava);
                  score()->curTick = ottava->tick();
                  }
            else if (tag == "Volta") {
                  Volta* volta = new Volta(score());
                  volta->setTick(score()->curTick);
                  volta->setStaff(staff);
                  volta->read(e);
                  add(volta);
                  score()->curTick = volta->tick();
                  }
            else if (tag == "Trill") {
                  Trill* trill = new Trill(score());
                  trill->setTick(score()->curTick);
                  trill->setStaff(staff);
                  trill->read(e);
                  add(trill);
                  }
            else if (tag == "Pedal") {
                  Pedal* pedal = new Pedal(score());
                  pedal->setTick(score()->curTick);
                  pedal->setStaff(staff);
                  pedal->read(e);
                  add(pedal);
                  score()->curTick = pedal->tick();
                  }
            else if (tag == "stretch")
                  _userStretch = val.toDouble();
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->setStaff(staff);
                  lb->read(e);
                  add(lb);
                  }
            else if (tag == "irregular")
                  _irregular = true;
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->read(e);
                  tuplet->setStaff(staff);
                  add(tuplet);
                  }
            else if (tag == "Repeat") {
                  Repeat* repeat = new Repeat(score());
                  repeat->setStaff(staff);
                  repeat->read(e);
                  add(repeat);
                  }
            else if (tag == "startRepeat")
                  _repeatFlags |= RepeatStart;
            else if (tag == "endRepeat") {
                  _endRepeat = val.toInt();
                  _repeatFlags |= RepeatEnd;
                  }
            else if (tag == "ending")
                  _ending = val.toInt()+1; //changed by DK. 28.08.07
            else if (tag == "Image") {
                  // look ahead for image type
                  QString path;
                  QDomElement ee = e.firstChildElement("path");
                  if (!ee.isNull())
                        path = ee.text();
                  Image* image = 0;
                  if (path.endsWith(".svg"))
                        image = new SvgImage(score());
                  else if (path.endsWith(".jpg")
                     || path.endsWith(".png")
                     || path.endsWith(".xpm")
                        ) {
                        image = new RasterImage(score());
                        }
                  else {
                        printf("unknown image format <%s>\n", path.toLatin1().data());
                        }
                  if (image) {
                        image->read(e);
                        add(image);
                        }
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Measure::read(QDomElement e)
      {
      int curTickPos = e.attribute("tick", "0").toInt();
      setTick(curTickPos);
      int staffIdx = 0;
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "Staff") {
                  read(e, staffIdx);
                  ++staffIdx;
                  }
            else if (tag == "startRepeat")
                  _repeatFlags |= RepeatStart;
            else if (tag == "endRepeat") {
                  _endRepeat = val.toInt();
                  _repeatFlags |= RepeatEnd;
                  }
            else if (tag == "ending")
                  _ending = val.toInt()+1; // changed by DK. 28.08.07
            else if (tag == "irregular")
                  _irregular = true;
            else if (tag == "stretch")
                  _userStretch = val.toDouble();
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->read(e);
                  if (t->anchor() != ANCHOR_PAGE) {
                        t->setTick(curTickPos);
                        t->setStaff(0);
                        }
                  add(t);
                  }
            else
                  domError(e);
            }
      }

//---------------------------------------------------------
//   collectElements
//---------------------------------------------------------

void Measure::collectElements(QList<Element*>& el)
      {
      el.append(this);     // to make measure clickable
      int staves = score()->nstaves();
      int tracks = staves * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                  if (!score()->staff(staffIdx)->show())
                        continue;
                  LyricsList* ll = s->lyricsList(staffIdx);
                  foreach(Lyrics* l, *ll) {
                        if (l)
                              el.append(l);
                        }
                  }
            for (int track = 0; track < tracks; ++track) {
                  Staff* staff = score()->staff(track / VOICES);
                  if (!staff->show())
                        continue;
                  Element* e = s->element(track);
                  if (e == 0)
                        continue;
                  if (e->isChordRest()) {
                        if (e->type() == CHORD) {
                              Chord* chord = (Chord*)e;
                              if (chord->hook())
                                    el.append(chord->hook());
                              if (chord->stem())
                                    el.append(chord->stem());
                              if (chord->arpeggio())
                                    el.append(chord->arpeggio());
                              if (chord->tremolo())
                                    el.append(chord->tremolo());

                              foreach(LedgerLine* h, *chord->ledgerLines())
                                    el.append(h);

                              const NoteList* nl = chord->noteList();
                              for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                                    Note* note = in->second;
                                    el.append(note);
                                    if (note->tieFor())
                                          el.append(note->tieFor());
                                    foreach(Text* f, note->fingering())
                                          el.append(f);
                                    if (note->accidental())
                                          el.append(note->accidental());
                                    }
                              }
                        else
                              el.append(e);
                        ChordRest* cr = (ChordRest*)e;
                        QList<NoteAttribute*>* al = cr->getAttributes();
                        for (ciAttribute i = al->begin(); i != al->end(); ++i) {
                              NoteAttribute* a = *i;
                              el.append(a);
                              }
                        }
                  else
                        el.append(e);
                  }
            }
      foreach(Element* e, _sel) {
            if (!e->staff()->show())
                  continue;
            switch(e->type()) {
                  case HAIRPIN:
                  case OTTAVA:
                  case PEDAL:
                  case TRILL:
                        ((SLine*)e)->collectElements(el);
                        break;
                  default:
                        el.append(e);
                        break;
                  }
            }
      foreach(Element* e, _pel) {
            el.append(e);
            }
      foreach(Beam* b, _beamList) {
            if (b->staff()->show())
                  el.append(b);
            }
      for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
            BarLine* b = barLine(staffIdx);
            // the system barline has no staff()
            if (b && (!b->staff() || b->staff()->show()))
                  el.append(b);
            }
      foreach(Tuplet* tuplet, _tuplets) {
            if (tuplet->staff()->show())
                  el.append(tuplet);
            }

      if (noText())
            el.append(noText());
      }

//---------------------------------------------------------
//   barLine
//---------------------------------------------------------

BarLine* Measure::barLine(int staff) const
      {
      int track = staff * VOICES;
      for (Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != Segment::SegEndBarLine)
                  continue;
            if (s->element(track))
                  return (BarLine*)(s->element(track));
            }
      return 0;
      }

//---------------------------------------------------------
//   setEndBarLine
//---------------------------------------------------------

void Measure::setEndBarLine(BarLine* ebl)
      {
      BarLine* bl = barLine(ebl->staffIdx());
      if (bl) {
            if (debugMode)
                  printf("Measure::setEndBarLine: already set at %d (%d)\n",
                     bl->tick(), tick() + tickLen());
            return;
            }
      int t = tick() + tickLen();
      Segment* seg = getSegment(Segment::SegEndBarLine, t);
      seg->add(ebl);
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
            if (s->subtype() != Segment::SegChordRest)
                  continue;
            if (s->element(track) == 0) {
                  Rest* rest = new Rest(score(), tick(), 0);
                  rest->setStaff(score()->staff(track / VOICES));
                  rest->setParent(s);
                  rest->setVoice(track % VOICES);
                  score()->undoAddElement(rest);
                  }
            break;
            }
      }

//---------------------------------------------------------
//   setEndBarLineType
//---------------------------------------------------------

void Measure::setEndBarLineType(int type, bool generated)
      {
      QList<Part*>* pl = score()->parts();
      foreach(Part* part, *pl) {
            Staff* staff = part->staff(0);
            int track = staff->idx() * VOICES;
            bool found = false;
            for (Segment* s = first(); s; s = s->next()) {
                  if (s->subtype() != Segment::SegEndBarLine)
                        continue;
                  if (s->element(track)) {
                        BarLine* bar = (BarLine*)(s->element(track));
                        bar->setSubtype(type);
                        bar->setGenerated(generated);
                        found = true;
                        }
                  break;
                  }
            if (!found) {
                  BarLine* bl = new BarLine(score());
                  bl->setStaff(staff);
                  bl->setSubtype(type);
                  bl->setGenerated(generated);
                  Segment* seg = getSegment(Segment::SegEndBarLine, tick() + tickLen());
                  seg->add(bl);
                  }
            }
      }

//---------------------------------------------------------
//   endBarLine
//---------------------------------------------------------

BarLine* Measure::endBarLine() const
      {
      for (const Segment* s = first(); s; s = s->next()) {
            if (s->subtype() != Segment::SegEndBarLine)
                  continue;
            if (s->element(0))
                  return (BarLine*)(s->element(0));
            }
      return 0;
      }

//---------------------------------------------------------
//   endBarLineType
//---------------------------------------------------------

int Measure::endBarLineType() const
      {
      BarLine* bl = endBarLine();
      if (bl)
            return bl->subtype();
      return NORMAL_BAR;
      }

//---------------------------------------------------------
//   setStartRepeatBarLine
//---------------------------------------------------------

void Measure::setStartRepeatBarLine(bool val)
      {
      QList<Part*>* pl = score()->parts();
      foreach(Part* part, *pl) {
            Staff* staff = part->staff(0);
            int track    = staff->idx() * VOICES;
            bool found   = false;
            for (Segment* s = first(); s; s = s->next()) {
                  if (s->subtype() != Segment::SegStartRepeatBarLine)
                        continue;
                  if (s->element(track)) {
                        found = true;
                        if (!val) {
                              delete s->element(track);
                              s->setElement(track, 0);
                              break;
                              }
                        }
                  }
            if (!found && val) {
                  BarLine* bl = new BarLine(score());
                  bl->setStaff(staff);
                  bl->setSubtype(START_REPEAT);
                  bl->setGenerated(true);
                  Segment* seg = getSegment(Segment::SegStartRepeatBarLine, tick());
                  seg->add(bl);
                  }
            }
      }

