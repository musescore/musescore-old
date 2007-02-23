//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: measure.cpp,v 1.105 2006/04/12 14:58:10 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
#include "painter.h"
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
//---------------------------------------------------------

int line2pitch(int line, int clef)
      {
      static const int pt[] = { 0, 2, 4, 5, 7, 9, 11, 12 };

      int l = clefTable[clef].pitchOffset - line;
      if (l < 0)
            l = 0;
      if (l > 74)
            l = 74;
      int pitch = pt[l % 7] + (l / 7) * 12;
      return pitch;
      }

//
// This table contails the line position of notes for one
// octave and all key signatures.
// This determines what accidentals are used by default
// to show a note of a given pitch.
//
// TODO: fix this
//

static char table1[15][12] = {
      { 0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6, 7 },  // ces
      { 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 7 },  // ges
      { 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 7 },  // des
      { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 7 },  // as
      { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6 },  // es
      { 0, 1, 1, 2, 2, 3, 3, 4, 5, 5, 6, 6 },  // Bb
      { 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6 },  // F

//      c  #c d #d  e  f #f  g #g  a  b  #b    key signature
      { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 },  // C

      { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 },  // G
      { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6 },  // D
      { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6 },  // A
      { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6 },  // E
      { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6 },  // B
      { 0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6 },  // fis
      { 0, 0, 0, 1, 2, 2, 3, 3, 4, 4, 5, 6 }   // cis
      };

//
//    fixed accidentals
//
static int tab3[15][8] = {
      //c  d  e  f  g  a  b  c
      //------------------------
      { 2, 2, 2, 2, 2, 2, 2, 2 },    // ces
      { 2, 2, 2, 0, 2, 2, 2, 2 },    // ges
      { 0, 2, 2, 0, 2, 2, 2, 0 },    // des
      { 0, 2, 2, 0, 0, 2, 2, 0 },    // as
      { 0, 0, 2, 0, 0, 2, 2, 0 },    // es
      { 0, 0, 2, 0, 0, 0, 2, 0 },    // B
      { 0, 0, 0, 0, 0, 0, 2, 0 },    // F

      { 0, 0, 0, 0, 0, 0, 0, 0 },    // C

      { 0, 0, 0, 1, 0, 0, 0, 0 },    // G
      { 1, 0, 0, 1, 0, 0, 0, 1 },    // D
      { 1, 0, 0, 1, 1, 0, 0, 1 },    // A
      { 1, 1, 0, 1, 1, 0, 0, 1 },    // E
      { 1, 1, 0, 1, 1, 1, 0, 1 },    // H
      { 1, 1, 1, 1, 1, 1, 0, 1 },    // fis
      { 1, 1, 1, 1, 1, 1, 1, 1 }     // cis
      };

//---------------------------------------------------------
//    pitch2y
//    y  = 0 = top staff line
//
//    key: -7 - +7  (C==0)
//    userPrefix = -1   no user defined prefix
//    return line and prefix
//---------------------------------------------------------

static int pitch2y(int pitch, int userPrefix, int clef, int key, int& prefix, const char* tversatz)
      {
      int clefOffset = clefTable[clef].yOffset;
      bool natural = false;
      int l1;
      int mtone  = pitch % 12;
      int octave = pitch / 12;
      octave = pitch / 12;
      natural = false;
      if (userPrefix == ACC_NATURAL) {
            userPrefix = ACC_NONE;
            natural = true;
            }

      l1 = table1[key+7][mtone];

      while (l1 < 0) {
            l1 += 7;
            octave--;
            mtone += 12;
            }
      static char table2[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };

      int l2     = 45 - l1 - (octave * 7) + clefOffset;
      int offset = mtone - table2[l1];
      switch (offset) {
            case -2:   prefix = 4; break;
            case -1:   prefix = 2; break;
            case 0:    prefix = 0; break;
            case 1:    prefix = 1; break;
            case 2:    prefix = 3; break;
            default:
                  printf("pitch2y: internal error: bad val %d\n", offset);
                  abort();
            }
      int cprefix = tversatz[l1 + octave * 7];
      if (cprefix == 0)
            cprefix = tab3[key+7][l1];
      if (cprefix == 5)
            cprefix = 0;
      if (cprefix) {
            if (prefix == cprefix)
                  prefix = 0;             // keine Versetzung
            else if (prefix == 0) {
                  prefix = 5;
                  }
            }
      if (natural)
            prefix = 5;
      return l2;
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
//            s.distance = point(staff == 0 ? style->systemDistance : style->staffDistance);
//            s.userDistance = 0.0;
            staves.push_back(s);
            }

      setTickLen(-1);
      _userStretch = 1.0;     // ::style->measureSpacing;
      _lineBreak   = false;
      _pageBreak   = false;
      _no          = 0;
      _irregular   = false;
      _startRepeat = false;
      _endRepeat   = 0;
      _ending      = 0;
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
// printf("remove Segment %p  _first %p _last %p prev %p next %p\n",
//   this, _first, _last, el->prev(), el->next());

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
      for (iMStaff ib = staves.begin(); ib != staves.end(); ++ib) {
            if (ib->endBarLine)
                  ib->endBarLine->move(x, y);
            }
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
      for (ciBeam i = _beamList.begin(); i != _beamList.end(); ++i)
            (*i)->move(x, y);
      foreach(Tuplet* t, _tuplets)
            t->move(x, y);
      }

//---------------------------------------------------------
//   findSelectableElement
//---------------------------------------------------------

/**
 If found, return selectable Element in this Measure near System relative point \a p.

 May return any Element in the Measure including measure relative elements (stored in _sel),
 but excluding page relative elements (stored in _pel) and the Measure itself.
*/

Element* Measure::findSelectableElement(QPointF p) const
      {
      p -= pos();
      for (Segment* segment = first(); segment; segment = segment->next()) {
            QPointF pp  = p - segment->pos();   // segment relative
            int staves = _score->nstaves();
            for (int staff = 0; staff < staves; ++staff) {
                  const LyricsList* ll = segment->lyricsList(staff);
                  for (ciLyrics i = ll->begin(); i != ll->end(); ++i) {
                        if ((*i) && (*i)->contains(pp)) {
                              return *i;
                              }
                        }
                  }
            int tracks = staves * VOICES;
            for (int t = 0; t < tracks; ++t) {
                  Element* e = segment->element(t);
                  if (e == 0)
                        continue;
                  if (e->type() == CHORD) {
                        Element* se = ((ChordRest*)e)->findSelectableElement(pp);
                        if (se)
                              return se;
                        }
                  else if (e->type() == REST) {
                        Element* se = ((ChordRest*)e)->findSelectableElement(pp);
                        if (se)
                              return se;
                        if (e->contains(pp))
                              return e;
                        }
                  else {
                        if (e->contains(pp))
                              return e;
                        }
                  }
            }
      for (ciElement i = _sel.begin(); i != _sel.end(); ++i) {
            if ((*i)->contains(p))
                  return *i;
            }
      for (ciMStaff is = staves.begin(); is != staves.end(); ++is) {
            if (is->endBarLine && is->endBarLine->contains(p))
                  return is->endBarLine;
            }
      if (_noText->contains(p))
            return _noText;
      foreach (Tuplet* tuplet, _tuplets) {
            if (tuplet->number() && tuplet->number()->contains(p))
                  return tuplet->number();
            if (tuplet->contains(p))
                  return tuplet;
            }
      return 0;
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

      if (staff == 0) {
            for (ciElement ie = _pel.begin(); ie != _pel.end(); ++ie)
                  (*ie)->write(xml);
            if (_startRepeat)
                  xml.tag("startRepeat", _startRepeat);
            if (_endRepeat)
                  xml.tag("endRepeat", _endRepeat);
            if (_ending)
                  xml.tag("ending", _ending);
            if (_irregular)
                  xml.tagE("irregular");
            if (_userStretch != 1.0)
                  xml.tag("stretch", _userStretch);
            }

      const MStaff* ms = &staves[staff];
      if (ms->endBarLine)
            ms->endBarLine->write(xml);
      for (ciElement i = _sel.begin(); i != _sel.end(); ++i) {
            if ((*i)->staff() == _score->staff(staff) && (*i)->type() != SLUR_SEGMENT)
                  (*i)->write(xml);
            }

      int id = 0;
      foreach(Tuplet* tuplet, _tuplets) {
            if (staff == tuplet->staffIdx())
                  tuplet->write(xml, id++);
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
      xml.etag("Measure");
      }

//---------------------------------------------------------
//   setEndBarLine
//---------------------------------------------------------

void Measure::setEndBarLine(BarLine* barLine)
      {
      barLine->setParent(this);
      staves[barLine->staffIdx()].endBarLine = barLine;
      }

//---------------------------------------------------------
//   Measure::read
//---------------------------------------------------------

/**
 Read Measure.
*/

void Measure::read(QDomNode node, int idx)
      {
      for (int n = staves.size(); n <= idx; ++n) {
            MStaff s;
            s.distance = point(n == 0 ? style->systemDistance : style->staffDistance);
            s.userDistance = 0.0;
            staves.push_back(s);
            }

      QDomElement e = node.toElement();
      int tck = e.attribute("tick", "-1").toInt();
      if (tck >= 0) {
            tck = score()->fileDivision(tck);
            setTick(tck);
            }
      Staff* staff = _score->staff(idx);
      int curTickPos = tick();

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
            QString tag(e.tagName());
            QString val(e.text());

            if (tag == "BarLine") {
                  BarLine* barLine = new BarLine(score());
                  barLine->setParent(this);
                  barLine->setStaff(staff);
                  barLine->read(node);
                  if (barLine->subtype() == START_REPEAT) {
                        barLine->setTick(tick());
                        Segment* s = getSegment(barLine);
                        s->add(barLine);
                        }
                  else
                        staves[idx].endBarLine = barLine;
                  }
            else if (tag == "Chord") {
                  Chord* chord = new Chord(score());
                  chord->setTick(curTickPos);   // set default tick position
                  chord->setParent(this);       // only for reading tuplets
                  chord->setStaff(staff);
                  chord->read(node, idx);
                  Segment* s = getSegment(chord);
                  s->add(chord);
                  curTickPos = chord->tick() + chord->tickLen();
                  }
            else if (tag == "Rest") {
                  Rest* rest = new Rest(score());
                  rest->setTick(curTickPos);    // set default tick position
                  rest->setParent(this);        // only for reading tuplets
                  rest->setStaff(staff);
                  rest->read(node);
                  Segment* s = getSegment(rest);
                  s->add(rest);
                  curTickPos = rest->tick() + rest->tickLen();
                  }
            else if (tag == "Clef") {
                  Clef* clef = new Clef(score());
                  clef->setTick(curTickPos);
                  clef->setStaff(staff);
                  clef->read(node);
                  Segment* s = getSegment(clef);
                  s->add(clef);
                  }
            else if (tag == "TimeSig") {
                  TimeSig* ts = new TimeSig(score());
                  ts->setTick(curTickPos);
                  ts->setStaff(staff);
                  ts->read(node);
                  Segment* s = getSegment(ts);
                  s->add(ts);
                  }
            else if (tag == "KeySig") {
                  KeySig* ks = new KeySig(score());
                  ks->setTick(curTickPos);
                  ks->setStaff(staff);
                  ks->read(node);
                  ks->setSubtype(ks->subtype());
                  Segment* s = getSegment(ks);
                  s->add(ks);
                  }
            else if (tag == "Dynamic") {
                  Dynamic* dyn = new Dynamic(score());
                  dyn->setTick(curTickPos);
                  dyn->setStaff(staff);
                  dyn->read(node);
                  add(dyn);
                  }
            else if (tag == "Slur") {
                  Slur* slur = new Slur(score());
                  slur->setTick(curTickPos);
                  slur->setStaff(staff);
                  slur->setParent(this);
                  slur->read(_score, node);
                  add(slur);
                  }
            else if (tag == "HairPin") {
                  Hairpin* hairpin = new Hairpin(score());
                  hairpin->setTick(curTickPos);
                  hairpin->setStaff(staff);
                  hairpin->read(node);
                  add(hairpin);
                  }
            else if (tag == "Lyrics") {
                  Lyrics* lyrics = new Lyrics(score());
                  lyrics->setTick(curTickPos);
                  lyrics->setStaff(staff);
                  lyrics->read(node);
                  Segment* segment = tick2segment(lyrics->tick());
                  if (segment == 0) {
                        printf("no segment for lyrics at %d\n",
                           lyrics->tick());
                        }
                  else
                        segment->add(lyrics);
                  }
            else if (tag == "Text") {
                  Text* t = new Text(score());
                  t->read(node);
                  if (t->anchor() != ANCHOR_PAGE) {
                        t->setTick(curTickPos);
                        t->setStaff(staff);
                        }
                  add(t);
                  }
            //-------------------------------------obsolete:
            else if (tag == "work-title") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_TITLE);
                  t->read(node);
                  add(t);
                  }
            else if (tag == "creator-composer") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_COMPOSER);
                  t->read(node);
                  add(t);
                  }
            else if (tag == "work-number") {
                  Text* t = new Text(score());
                  t->setSubtype(TEXT_SUBTITLE);
                  t->read(node);
                  add(t);
                  }
            //-------------------------------------
            else if (tag == "Tempo") {
                  TempoText* t = new TempoText(score());
                  t->setTick(curTickPos);
                  t->setStaff(staff);
                  t->read(node);
                  add(t);
                  }
            else if (tag == "Symbol") {
                  Symbol* sym = new Symbol(score());
                  sym->setTick(curTickPos);
                  sym->setStaff(staff);
                  sym->read(node);
                  add(sym);
                  }
            else if (tag == "Ottava") {
                  Ottava* ottava = new Ottava(score());
                  ottava->setTick(curTickPos);
                  ottava->setStaff(staff);
                  ottava->read(node);
                  add(ottava);
                  }
            else if (tag == "Volta") {
                  Volta* volta = new Volta(score());
                  volta->setTick(curTickPos);
                  volta->setStaff(staff);
                  volta->read(node);
                  add(volta);
                  }
            else if (tag == "Trill") {
                  Trill* trill = new Trill(score());
                  trill->setTick(curTickPos);
                  trill->setStaff(staff);
                  trill->read(node);
                  add(trill);
                  }
            else if (tag == "Pedal") {
                  Pedal* pedal = new Pedal(score());
                  pedal->setTick(curTickPos);
                  pedal->setStaff(staff);
                  pedal->read(node);
                  add(pedal);
                  }
            else if (tag == "stretch")
                  _userStretch = val.toDouble();
            else if (tag == "LayoutBreak") {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->setStaff(staff);
                  lb->read(node);
                  add(lb);
                  }
            else if (tag == "irregular")
                  _irregular = true;
            else if (tag == "Tuplet") {
                  Tuplet* tuplet = new Tuplet(score());
                  tuplet->read(node);
                  tuplet->setStaff(staff);
                  _tuplets.append(tuplet);
                  }
            else if (tag == "startRepeat")
                  _startRepeat = val.toInt();
            else if (tag == "endRepeat")
                  _endRepeat = val.toInt();
            else if (tag == "ending")
                  _ending = val.toInt();
            else
                  domError(node);
            }
//      layoutNoteHeads(idx);
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
// printf("Measure::layoutNoteHeads(this=%p staff=%d)\n", this, staff);
      char tversatz[75];
      memset(tversatz, 0, sizeof(tversatz));
      for (Segment* segment = first(); segment; segment = segment->next()) {
            int startTrack = staff * VOICES;
            int endTrack   = startTrack + VOICES;
            for (int track = startTrack; track < endTrack; ++track) {
                  Element* e = segment->element(track);
                  if (e == 0 || e->type() != CHORD)
                        continue;
                  Chord* chord   = (Chord*)e;
                  NoteList* nl   = chord->noteList();
                  bool mirror    = false;   // start with head left from stem
                  int tick       = chord->tick();
                  int ll         = -1000;    // make sure top head is not mirrored
                  int move1      = nl->front()->move();
                  Tuplet* tuplet = chord->tuplet();
                  // printf("tick=%d key=%d\n", tick, _score->keymap->key(tick));

                  for (riNote in = nl->rbegin(); in != nl->rend(); ++in) {
                        Note* note  = in->second;
                        if (tuplet)
                              note->setHead(tuplet->baseLen());
                        else
                              note->setHead(chord->tickLen());
                        int pitch   = note->pitch();
                        int move    = note->move();
                        Staff* staffp = _score->staff(staff+move);
                        int clef    = staffp->clef()->clef(tick);
                        int userAcc = note->userAccidental();
                        int key     = _score->staff(staff)->keymap()->key(tick);

                        int prefix;
                        int line = note->line();
                        if (userAcc != -1) {
                              int np = line2pitch(line, clef);
                              int offset = pitch - np;
                              switch (offset) {
                                    case -2:   prefix = 4; break;
                                    case -1:   prefix = 2; break;
                                    case 0:    prefix = 0; break;
                                    case 1:    prefix = 1; break;
                                    case 2:    prefix = 3; break;
                                    default:
                                          printf("line2pitch: error2: tick %d bad offset %d(%d-%d), line %d clef %d\n",
                                             tick, offset, pitch, np, line, clef);
                                          //abort();
                                          break;
                                    }
                              }
                        else {
                              line = ::pitch2y(pitch, userAcc, clef, key, prefix, tversatz);
                              note->setLine(line);
                              }
                        int offset = clefTable[clef].yOffset;
                        if (prefix) {
                              int l1       = 45 - line + offset;
                              tversatz[l1] = prefix;
                              }
                        if (mirror || (((line - ll) < 2) && move1 == move)) {
                              mirror = !mirror;
                              }
                        move1 = move;
                        note->setMirror(mirror);
                        note->setAccidental(userAcc == -1 ? prefix : userAcc);
                        ll = line;
//                        printf("tick=%d key=%d pitch=%d line=%d ua=%d pref=%d\n",
//                                tick, key, pitch, line, userAcc, prefix);
                        }
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

void Measure::layout(double width)
      {
      if (first() == 0)
            return;

      int n = _score->nstaves();
      for (int i = 0; i < n; ++i)
            layoutNoteHeads(i);

      double staffY[n];
      for (int i = 0; i < n; ++i) {
            staffY[i] = system()->staff(i)->bbox().y();
            }
      setbbox(QRectF(0, 0, width, system()->height()));

      layoutX(width);

      //---------------------------------------------------
      //    layout bars
      //---------------------------------------------------

      PartList* pl = _score->parts();
      double x  = width;
      int staff = 0;
      Spatium barLineLen(4);
      barLineLen += ::style->staffLineWidth;
      for (iPart ip = pl->begin(); ip != pl->end(); ++ip) {
            Part* p = *ip;
            BarLine* barLine = staves[staff].endBarLine;
            if (barLine) {
                  double y1 = staffY[staff];
                  double y2 = staffY[staff + p->nstaves() - 1] + point(barLineLen);
                  barLine->setHeight(y2 - y1);
                  barLine->setPos(x - barLine->width(), y1 - point(::style->staffLineWidth) * .5);
                  }
            staff += p->nstaves();
            }

#if 0
      for (iMStaff is = staves.begin(); is != staves.end(); ++is, ++staff) {
            double y = staffY[staff];
            BarLine* barLine = is->endBarLine;
            if (barLine) {
                  bool split = _score->staff(staff)->isTopSplit();
                  Spatium barLineLen(4);
                  barLineLen += ::style->staffLineWidth;
                  if (split)
                        barLineLen += spatium(staffY[staff+1] - y);
                  barLine->setHeight(point(barLineLen));
                  barLine->setPos(x - barLine->width(), y - point(::style->staffLineWidth) * .5);
                  }
            }
#endif

      //---------------------------------------------------
      //   layout Chords/Lyrics/Symbols/BeginRepeatBar
      //---------------------------------------------------

      int st = _score->nstaves();
      int tracks = st * VOICES;
      for (Segment* segment = first(); segment; segment = segment->next()) {
            for (int track = 0; track < tracks; ++track) {
                  Element* e = segment->element(track);
                  if (e == 0)
                        continue;
                  if (e->isChordRest())
                        e->layout();
                  else if (e->type() == BAR_LINE) {
                        //
                        // must be begin repeat bar
                        // TODO: _no_ can also be a dotted bar in the middle
                        //       of the measure
                        //
                        BarLine* barLine = (BarLine*)e;
                        int staff = track / VOICES;
                        double y  = staffY[staff];
                        bool split = _score->staff(staff)->isTopSplit();
                        Spatium barLineLen(4);
                        barLineLen += ::style->staffLineWidth;
                        if (split)
                              barLineLen += spatium(staffY[staff+1] - y);
                        barLine->setHeight(point(barLineLen));
                        barLine->setPos(barLine->pos().x(), y - point(::style->staffLineWidth) * .5);
                        }
                  }
            for (int staff = 0; staff < st; ++staff) {
                  LyricsList* ll = segment->lyricsList(staff);
                  int line = 0;
                  for (iLyrics i = ll->begin(); i != ll->end(); ++i, ++line) {
                        Lyrics* lyrics = *i;
                        if (lyrics == 0)
                              continue;
                        lyrics->layout();
                        // center to middle of notehead:
                        double noteHeadWidth = symbols[quartheadSym].width();
                        double lh = lyrics->lineSpacing();
                        double y  = lh * line + 6 * _spatium;
                        lyrics->setPos(noteHeadWidth/2 - lyrics->bbox().width() * .5, y);

                        // increase staff distance if necessary
                        y += _spatium * 4;
                        if ((staff+1) < st) {
                              if (y > staves[staff].distance) {
                                    staves[staff].distance = y;
                                    }
                              }
                        }
                  }
            }

      //---------------------------------------------------
      //    layout beams and tuplets
      //---------------------------------------------------

      layoutBeams();
      foreach(Tuplet* tuplet, _tuplets)
            tuplet->layout();

#if 0  // adjust topDistance, bottomDistance
      for (iBeam ib = _beamList.begin(); ib != _beamList.end(); ++ib) {
            Beam* b = *ib;

            double y1 = b->bbox().y();
            double y2 = y1 + b->bbox().height();
            }
#endif
      if (_noText)
            _noText->layout();
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
            if (s->segmentType() != Segment::SegChordRest)
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

void Measure::layout2()
      {
//      printf("Measure(%p)::layout2(): _sel.size = %d\n", this, _sel.size());

      foreach(Element* pel, _sel) {
            int staff = _score->staff(pel->staff());

            // double y  = staff * point(Spatium(4) + style->staffDistance);
            double y = system()->staff(staff)->bbox().y();

            switch(pel->type()) {
                  case VOLTA:
                  case OTTAVA:
                        pel->layout();
                        break;
                  case DYNAMIC:
                  case SYMBOL:
                  case TEXT:
                  case TEMPO_TEXT:
                        {
                        pel->layout();
                        double x = tick2pos(pel->tick());
                        pel->setPos(x, y);
                        }
                        break;
                  case LAYOUT_BREAK:
                        pel->setPos(
                           bbox().width() - pel->bbox().width() - _spatium,
                           - pel->bbox().height() - _spatium);
                        break;
                  case SLUR:
                        // slur->layout() messes with add()/remove()
                  default:
                        pel->layout();
                        break;
                  }

            }

      //
      //   set measure number
      //
      int pn = _no + _noOffset;
      QString s = QString("%1").arg(pn + 1);

      QString ns;
      if (::style->showMeasureNumber
         && !_irregular
         && (pn || ::style->showMeasureNumberOne)) {
            if (::style->measureNumberSystem) {
                  if (system()->measures() && system()->measures()->front() == this)
                        ns = s;
                  }
            else if ((pn % ::style->measureNumberInterval) == 0)
                  ns = s;
            }
      _noText->setText(ns);

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
                                    tie->layout();
                              }
                        }
                  }
            }
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

ChordRest* Measure::findChordRest(int tick, Staff* staff, int voice, bool /*grace*/)
      {
      int staffIdx = _score->staves()->indexOf(staff);
      for (Segment* seg = _first; seg; seg = seg->next()) {
            if (seg->tick() > tick)
                  return 0;
            if (seg->tick() == tick) {
                  Element* el = seg->element(staffIdx * VOICES + voice);
                  if (el && (el->type() == CHORD || el->type() == REST)) {
                        return (Chord*)el;
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
            if ((s->segmentType() == Segment::SegChordRest) && (s->tick() == tick))
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
            if (ss->segmentType() == st)
                  return ss;
            }
      return 0;
      }

//---------------------------------------------------------
//   createSegment
//---------------------------------------------------------

Segment* Measure::createSegment(Segment::SegmentType st, int t)
      {
      Segment* newSegment = new Segment(this, t);
      newSegment->setSegmentType(st);
      return newSegment;
      }

//---------------------------------------------------------
//   getSegment
//---------------------------------------------------------

Segment* Measure::getSegment(Element* e)
      {
      Segment::SegmentType st = Segment::segmentType(e->type());
      Segment* s = findSegment(st, e->tick());
      if (!s) {
            s = createSegment(st, e->tick());
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
            el->setAnchor(this);
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
                  el->setAnchor(this);
                  break;
            case SEGMENT:
                  {
                  Segment* s;
                  for (s = first(); s && s->tick() < t; s = s->next())
                        ;
                  Segment::SegmentType st = ((Segment*)el)->segmentType();
                  if (s) {
                        if (st == Segment::SegChordRest) {
                              while (s && s->segmentType() != Segment::SegChordRest && s->tick() == t)
                                    s = s->next();
                              }
                        else {
                              while (s && s->segmentType() <= st) {
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
                              printf("layout break already set\n");
                              delete el;
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
                              _ending = 0;
                              break;
                        case SECONDA_VOLTA:
                        case SECONDA_VOLTA2:
                              _ending = 1;
                              break;
                        case TERZA_VOLTA:
                              _ending = 2;
                              break;
                        };
                  _sel.append(el);
                  break;

            case BAR_LINE:
                  staves[el->staffIdx()].endBarLine = (BarLine*)el;
                  break;
            case OTTAVA:
            case PEDAL:
            case TRILL:
            case HAIRPIN:
            case DYNAMIC:
            case SYMBOL:
            case TEXT:
            case TEMPO_TEXT:
                  _sel.append(el);
                  break;
            case SLUR_SEGMENT:
// printf("measure(%p) add slur segment %p\n", this, el);
                  _sel.append(el);
                  break;
            default:
                  printf("Measure::add(%s) not impl.\n", el->name());
                  abort();
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
      int staff = _score->staff(el->staff());

// printf("measure %p: remove el %s %p, staff %d\n", this, el->name(), el, staff);

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
            case SLUR_SEGMENT:
// printf("measure(%p) remove slur segment %p\n", this, el);
            case DYNAMIC:
            case HAIRPIN:
            case TEMPO_TEXT:
            case TEXT:
            case SYMBOL:
            case OTTAVA:
            case PEDAL:
            case TRILL:
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

            case BAR_LINE:
                  if (el->subtype() != START_REPEAT) {
                        if (staves[staff].endBarLine == el) {
                              staves[staff].endBarLine = 0;
                              if (el->subtype() != END_REPEAT)
                                    _endRepeat = 0;
                              }
                        break;
                        }
                  printf("Measure::remove: BarLine %p not found\n", el);
                  break;

            default:
                  printf("Measure::remove %s: not impl.\n", el->name());
                  break;
            }
      }

//---------------------------------------------------------
//   moveTicks
//---------------------------------------------------------

void Measure::moveTicks(int diff)
      {
      setTick(tick() + diff);
      for (iMStaff is = staves.begin(); is != staves.end(); ++is) {
            if (is->endBarLine)
                  is->endBarLine->setTick(is->endBarLine->tick() + diff);
            }
      for (ciElement ii = _sel.begin(); ii != _sel.end(); ++ii)
            (*ii)->setTick((*ii)->tick() + diff);
      int staves = _score->nstaves();
      int tracks = staves * VOICES;
      for (Segment* segment = first(); segment; segment = segment->next()) {
            segment->setTick(segment->tick() + diff);
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
//   draw
//---------------------------------------------------------

void Measure::draw(Painter& p)
      {
      p.translate(pos());

      // printf("measure %p ::draw\n", this);
      for (Segment* segment = first(); segment; segment = segment->next())
            segment->draw(p);
      for (ciBeam i = _beamList.begin(); i != _beamList.end(); ++i)
            (*i)->draw(p);
      foreach(Tuplet* tuplet, _tuplets)
            tuplet->draw(p);
      int staffIdx = 0;
      for (ciMStaff is = staves.begin(); is != staves.end(); ++is, ++staffIdx) {
            if (score()->part(staffIdx)->show() && is->endBarLine)
                  is->endBarLine->draw(p);
            }
      for (ciElement i = _sel.begin(); i != _sel.end(); ++i)
            (*i)->draw(p);

      if (_noText)
            _noText->draw(p);

      //-------------------------------
      // draw selection:
      //-------------------------------

      if (_score->sel->state != SEL_STAFF && _score->sel->state != SEL_SYSTEM) {
            p.translate(-pos());
            return;
            }
      int sstart = _score->sel->tickStart;
      int send   = _score->sel->tickEnd;
      int mstart = tick();
      int mend   = tick() + tickLen();

      if (send <= mstart || sstart >= mend) {
            p.translate(-pos());
            return;
            }

      p.setBrush(Qt::NoBrush);

      double x1 = bbox().x();
      double x2 = x1 + bbox().width();
      if (_score->sel->state == SEL_SYSTEM) {
            p.setPen(QPen(QColor(Qt::blue), 4, Qt::DotLine));
            double y1 = bbox().y() - _spatium;
            double y2 = y1 + bbox().height() + 2 * _spatium;

            // is this measure start of selection?
            if (sstart >= mstart && sstart < mend) {
                  x1 -= _spatium;
                  p.drawLine(QLineF(x1, y1, x1, y2));
                  }
            // is this measure end of selection?
            if (send > mstart && send <= mend) {
                  x2 += _spatium;
                  p.drawLine(QLineF(x2, y1, x2, y2));
                  }
            p.drawLine(QLineF(x1, y1, x2, y1));
            p.drawLine(QLineF(x1, y2, x2, y2));
            }
      else {
            QPen pen(QColor(Qt::blue));
            pen.setWidthF(2.0 / p.matrix().m11());
            pen.setStyle(Qt::SolidLine);
            p.setPen(pen);
            double y1 = system()->staff(_score->sel->staffStart)->bbox().y() - _spatium;
            double y2 = system()->staff(_score->sel->staffEnd-1)->bbox().y()
                        + system()->staff(_score->sel->staffEnd-1)->bbox().height()
                        + _spatium;

            // is this measure start of selection?
            if (sstart >= mstart && sstart < mend) {
                  x1 -= _spatium;
                  p.drawLine(QLineF(x1, y1, x1, y2));
                  }
            // is this measure end of selection?
            if (send > mstart && send <= mend) {
                  x2 += _spatium;
                  p.drawLine(QLineF(x2, y1, x2, y2));
                  }
            p.drawLine(QLineF(x1, y1, x2, y1));
            p.drawLine(QLineF(x1, y2, x2, y2));
            }
      p.translate(-pos());
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
            }
      BarLine* barLine = staves[staff].endBarLine;
      if (barLine) {
            barLine->move(0, dy);
            if (_score->staff(staff)->isTopSplit()) {
                  Spatium barLineLen = Spatium(8) + ::style->staffDistance;
                  barLine->setHeight(point(barLineLen));
                  }
            }
      _noText->move(0, dy);
      }

//---------------------------------------------------------
//   addBeam
//---------------------------------------------------------

void Measure::addBeam(Beam* b)
      {
      b->setParent(this);
      _beamList.push_back(b);
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

MeasureWidth Measure::layoutX(double stretch)
      {
      int nstaves = _score->nstaves();
      int tracks  = nstaves * VOICES;

      int segs   = size();
      if (nstaves == 0 || segs == 0)
            return MeasureWidth(1.0, 0.0);

      //-----------------------------------------------------------------------
      //    remove empty segments
      //-----------------------------------------------------------------------
/**/

// LVIFIX: removing empty segments prevents redo of actions on elements stored in segments
// (e.g. CLEF, CHORD, REST, TIMESIG).
// Cause: redo restores the pointer from the segment to the element, but not does not
// undo segment removal from the measure
//
// WS: this has now changed; undo/redo should create/delete segments; needs
// further testing
// removal at this place should not be necessary any more

again:
      for (Segment* s = first(); s; s = s->next()) {
            bool empty = true;
            for (int track = 0; track < tracks; ++track) {
                  if (s->element(track)) {
                        empty = false;
                        break;
                        }
                  }
            if (empty) {
// printf("Measure::layoutX(): internal error: remove empty segment %p\n", s);
                  remove(s);
                  goto again;
                  }
            }
/**/
      //-----------------------------------------------------------------------
      //    fill array of Spaces for all segments and staves
      //    spaces[0]      - left margin
      //    spaces[segs+1] - right margin
      //    xpos[segs+2]   - start of next measure (width of current measure)
      //-----------------------------------------------------------------------

      Space spaces[segs+2][tracks + nstaves]; // dont forget lyrics
      double xpos[segs+3], ypos[tracks], width[segs+2];

      int seg = 1;
      bool notesSeg = first()->segmentType() == Segment::SegChordRest;
      bool firstNoteRest = true;
      for (const Segment* s = first(); s; s = s->next(), ++seg) {
            //
            // add extra space between clef/key/timesig and first notes
            //
            double additionalMin = 0.0;
            double additionalExtra = 0.0;
            if (!notesSeg && s->next() && s->next()->segmentType() == Segment::SegChordRest) {
                  additionalMin = point(::style->clefKeyRightMargin);
                  notesSeg = true;
                  }
            if (s->segmentType() == Segment::SegChordRest) {
                  if (firstNoteRest) {
                        firstNoteRest = false;
                        }
                  else
                        additionalExtra = point(::style->minNoteDistance);
                  }
            else if (s->segmentType() == Segment::SegClef)
                  additionalExtra = point(::style->clefLeftMargin);
            else if (s->segmentType() == Segment::SegTimeSig)
                  additionalExtra = point(::style->timesigLeftMargin);
            else if (s->segmentType() == Segment::SegKeySig)
                  additionalExtra = point(::style->keysigLeftMargin);

            for (int track = 0; track < tracks; ++track) {
                  const Element* el = s->element(track);
                  Staff* staff = score()->staff(track/VOICES);
                  bool valid = el && staff->show();
                  spaces[seg][track].setValid(valid);
                  if (valid) {
                        double min, extra;
                        el->space(min, extra);
                        spaces[seg][track].setMin(min + additionalMin);
                        spaces[seg][track].setExtra(extra + additionalExtra);
                        }
                  }
            for (int i = 0; i < nstaves; ++i) {
                  const LyricsList* ll = s->lyricsList(i);
                  double min = 0;
                  double extra = 0;
                  for (ciLyrics l = ll->begin(); l != ll->end(); ++l) {
                        if (!*l)
                              continue;
                        double lw = ((*l)->bbox().width() + _spatium * 0) / 2.0;
                        if (lw > min)
                              min = lw;
                        if (lw > extra)
                              extra = lw;
                        }
                  spaces[seg][tracks+i].setMin(min);
                  spaces[seg][tracks+i].setExtra(extra);
                  spaces[seg][tracks+i].setValid(!ll->empty());
                  }
            }
      for (int track = 0; track < tracks; ++track) {
            Spatium lMargin;
            switch(first()->segmentType()) {
                  case Segment::SegClef:
                        lMargin = ::style->clefLeftMargin;
                        break;
                  case Segment::SegKeySig:
                        lMargin = ::style->keysigLeftMargin;
                        break;
                  case Segment::SegTimeSig:
                        lMargin = ::style->timesigLeftMargin;
                        break;
                  case Segment::SegChordRest:
                        lMargin = ::style->barNoteDistance;
                        break;
                  case Segment::SegBarLine:
                        lMargin = Spatium(0);
                        break;
                  }
            spaces[0][track].setMin(point(lMargin));
            spaces[0][track].setExtra(0.0);
            spaces[0][track].setValid(true);
            spaces[segs+1][track].setExtra(point(::style->noteBarDistance));
            double w;
            BarLine* barLine = staves[track/VOICES].endBarLine;
            if (barLine)
                  w = barLine->width();
            else
                  w = 0.0;
            spaces[segs+1][track].setMin(w);
            spaces[segs+1][track].setValid(true);
            }
      for (int track = tracks; track < tracks+nstaves; ++track) {
            spaces[0][track].setValid(false);
            spaces[segs+1][track].setValid(false);
            }

#ifdef DEBUG
      printf("1======== ");
      for (int track = 0; track < tracks+nstaves; ++track) {
            for (int seg = 0; seg < segs+2; ++seg) {
                  if (spaces[seg][track].valid())
                        printf("%3.1f+%3.1f ", spaces[seg][track].min(), spaces[seg][track].extra());
                  else
                        printf("     -     ");
                  }
            printf("          \n");
            }
#endif

      //---------------------------------------------------
      //    move extra space to previous cells
      //---------------------------------------------------

      for (int staff = 0; staff < tracks+nstaves; ++staff) {
            for (int seg = segs+1; seg > 0; --seg) {    // seg 0 kann keinen Platz mehr verschieben
                  double extra = spaces[seg][staff].extra();
                  if (extra < 0.00001)
                        continue;
                  // extra Platz auf vorheriges nichtleeres Segment verschieben
                  int tseg;
                  for (tseg = seg-1; tseg >= 0; --tseg) {
                        if (spaces[tseg][staff].valid())
                              break;
                        }
                  if (tseg == 0) {
                        if (spaces[tseg][staff].min() < extra)
                              spaces[tseg][staff].setMin(extra);
                        }
                  else
                        spaces[tseg][staff].addMin(extra);
                  }
            }
#ifdef DEBUG
      printf("2======== ");
      for (int staff = 0; staff < tracks+nstaves; ++staff) {
            for (int seg = 0; seg < segs+2; ++seg) {
                  if (spaces[seg][staff].valid())
                        printf("%3.1f+%3.1f ", spaces[seg][staff].min(), spaces[seg][staff].extra());
                  else
                        printf("     -     ");
                  }
            printf("          \n");
            }
#endif
      //---------------------------------------------------
      //    populate width[] array
      //---------------------------------------------------

      for (int seg = segs+1; seg >= 0; --seg) {
            double ww = 0.0;
            for (int staff = 0; staff < tracks+nstaves; ++staff) {
                  if (spaces[seg][staff].valid()) {
                        double w = spaces[seg][staff].min();
                        for (int nseg = seg+1; nseg < segs+2; ++nseg) {
                              if (spaces[nseg][staff].valid())
                                    break;
                              w -= width[nseg];
                              if (w < 0.0)
                                    break;
                              }
                        if (w > ww)
                              ww = w;
                        }
                  }
            width[seg] = ww;
            }

#ifdef DEBUG
      printf("3width:===");
      for (int i = 0; i < segs+2; ++i)
            printf("%3.1f   ", width[i]);
      printf("\n");
#endif
      //---------------------------------------------------
      //    segments with equal duration should have
      //    equal width
      //---------------------------------------------------

      int ticks[segs+2];
      memset(ticks, 0, (segs+2) * sizeof(int));

      //--------tick table for segments
      int minTick   = 100000;
      int ntick     = tick() + tickLen();   // position of next measure
      int i         = 1;
      for (Segment* seg = first(); seg; seg = seg->next(), ++i) {
            if (seg->segmentType() == Segment::SegChordRest) {
                  Segment* nseg = seg;
                  for (;;) {
                        nseg = nseg->next();
                        if (nseg == 0 || nseg->segmentType() == Segment::SegChordRest)
                              break;
                        }
                  int nticks = (nseg ? nseg->tick() : ntick) - seg->tick();
                  if (nticks == 0) {
                        printf("layoutX: nticks==0 size %d, idx %d, %d %d  %p %p types %d %d\n",
                           size(), i-1, seg->tick(), nseg ? nseg->tick() : 0, seg, nseg,
                           seg->type(), nseg ? nseg->type() : 0);
                        }
                  else {
                        if (nticks < minTick)
                              minTick = nticks;
                        }
                  ticks[i] = nticks;
                  }
            }
#ifdef DEBUG
      printf("ticks:    ");
      for (int i = 0; i < segs+2; ++i)
            printf("%d ", ticks[i]);
      printf("\n");
#endif

      // compute stretches:

      springs.clear();
      double stretchList[segs+2];
      double stretchSum   = 0.0;
      stretchList[0]      = 0.0;
      stretchList[segs+1] = 0.0;
      double minimum      = width[0] + width[segs+1];
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
#ifdef DEBUG
      printf("stretch:    ");
      for (int i = 0; i < segs+2; ++i)
            printf("%3.1f ", stretchList[i]);
      printf("\n");
#endif

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

      xpos[0] = 0.0;
      for (int seg = 0; seg < segs+2; ++seg)
            xpos[seg+1] = xpos[seg] + width[seg];

#ifdef DEBUG
      printf("a.stretch ");
      for (int seg = 0; seg < segs+2; ++seg)
            printf("%3.1f(%3.1f) ", xpos[seg], width[seg]);
      printf("\n");
#endif

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
                  double y = ypos[staff/VOICES];
                  QPointF pos(0.0, y);
                  Element* e = s->element(staff);
                  if (e == 0)
                        continue;
                  ElementType t = e->type();
                  if (t == REST) {
                        double y = ypos[staff/VOICES + ((Rest*)e)->move()];
                        int len = e->tickLen();
                        //
                        // center symbol if its a whole rest
                        //
                        double yoffset = 0.0;   // hack for whole rest symbol
                        if (!_irregular && len == _score->sigmap->ticksMeasure(e->tick())) {
                              // center whole rest
                              int idx = 0;
                              for (Segment* s = _first; s; s = s->next(), ++idx) {
                                    if (s->segmentType() == Segment::SegChordRest)
                                          break;
                                    }
                              double o = xpos[idx+1];
                              double w = xpos[segs+2] - o;
                              pos.setX(w/2.0 - e->width()/2.0);
                              yoffset = -_spatium;
                              }
                        if (e->voice() == 1)
                              y += -1 * _spatium;
                        else
                              y += 2 * _spatium;
                        e->setPos(pos.x(), y + yoffset);
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
                        double xo = spaces[seg][staff].extra();
                        if (t == CLEF)
                              e->setPos(-e->bbox().x() - xo + point(::style->clefLeftMargin), y);
                        else if (t == TIMESIG)
                              e->setPos(- e->bbox().x() - xo + point(::style->timesigLeftMargin), y);
                        else if (t == KEYSIG)
                              e->setPos(- e->bbox().x() - xo + point(::style->keysigLeftMargin), y);
                        else
                              e->setPos(- e->bbox().x() - xo, y);
                        }
                  }
            }
      return MeasureWidth(xpos[segs+2], 0.0);
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
//                  _score->undoOp(UndoOp::RemoveSegStaff, s, staff);
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
//                  _score->undoOp(UndoOp::InsertSegStaff, s, staff);
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

      for (int i = sStaff; i < eStaff; ++i) {
            BarLine* barLine = 0;
            if (i == sStaff) {
                  barLine = new BarLine(score());
                  barLine->setParent(this);
                  }
            MStaff ms;

            ms.endBarLine = barLine;
            staves.insert(i, ms);
            _score->undoOp(UndoOp::InsertMStaff, this, ms, i);

            int tickLen = _score->sigmap->ticksMeasure(tick());
            Rest* rest = new Rest(score(), tick(), tickLen);
            Staff* staff = _score->staff(i);
            rest->setStaff(staff);
            Segment* s = findSegment(Segment::SegChordRest, tick());
            rest->setParent(s);
            _score->undoOp(UndoOp::AddElement, rest);
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

void Measure::insertStaff1(Staff* staff, int staffIdx)
      {
      for (Segment* s = _first; s; s = s->next())
            s->insertStaff(staffIdx);

      BarLine* barLine = 0;
      if (staff->isTop()) {
            barLine = new BarLine(score());
            barLine->setParent(this);
            }

      MStaff ms;
      ms.endBarLine = barLine;
      ms.distance = point(staffIdx == 0 ? style->systemDistance : style->staffDistance);
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

bool Measure::acceptDrop(const QPointF& p, int type, const QDomNode&) const
      {
      // convert p from canvas to measure relative position and take x and y coordinates
      QPointF mrp = p - pos() - system()->pos() - system()->page()->pos();
      double mrpx = mrp.x();
      double mrpy = mrp.y();

      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1)
            return false;                       // staff not found
      qreal t = s->staff(idx)->bbox().top();    // top of staff
      qreal b = s->staff(idx)->bbox().bottom(); // bottom of staff

      switch(type) {
            case VOLTA:
            case OTTAVA:
            case TRILL:
                  // accept drop only above staff
                  if (mrpy < t)
                        return true;
                  return false;
            case PEDAL:
            case DYNAMIC:
                  // accept drop only below staff
                  if (mrpy > b)
                        return true;
                  return false;
            case BRACKET:
            case LAYOUT_BREAK:
                  return true;
            case BAR_LINE:
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  return true;
            case HAIRPIN:
                  // accept drop only above or below staff
                  if (mrpy < t || mrpy > b)
                        return true;
                  return false;
            case CLEF:
                  {
                  // accept drop only inside staff
                  if (mrpy < t || mrpy > b)
                        return false;
                  // search segment list backwards for segchordrest
                  for (Segment* seg = _last; seg; seg = seg->prev()) {
                        if (seg->segmentType() != Segment::SegChordRest)
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
                  for (Segment* seg = _first; seg; seg = seg->next())
                        if (seg->segmentType() == Segment::SegChordRest)
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

void Measure::drop(const QPointF& p, int type, const QDomNode& node)
      {
      // determine staff
      System* s = system();
      int idx = s->y2staff(p.y());
      if (idx == -1)
            return;
//      SysStaff* ss = s->staff(idx);
      Staff* staff = score()->staff(idx);

      // convert p from canvas to measure relative position and take x coordinate
      QPointF mrp = p - pos() - system()->pos() - system()->page()->pos();
//      double mrpx = mrp.x();

      switch(ElementType(type)) {
            case BRACKET:
                  {
                  Bracket* bracket = new Bracket(0);
                  bracket->read(node);
                  int subtype = bracket->subtype();
                  delete bracket;
                  staff->addBracket(BracketItem(subtype, 1));
                  score()->layout();
                  }
                  break;
            case CLEF:
                  {
                  Clef* clef = new Clef(0);
                  clef->read(node);
                  staff->changeClef(tick(), clef->subtype());
                  delete clef;
                  }
                  break;
            case KEYSIG:
                  {
                  KeySig* ks = new KeySig(0);
                  ks->read(node);
                  staff->changeKeySig(tick(), ks->subtype());
                  delete ks;
                  }
                  break;
            case TIMESIG:
                  {
                  TimeSig* ts = new TimeSig(0);
                  ts->read(node);
                  score()->changeTimeSig(tick(), ts->subtype());
                  delete ts;
                  }
                  break;
            case VOLTA:
                  {
                  Volta* volta = new Volta(score());
                  volta->read(node);
                  volta->setStaff(staff);
                  volta->setParent(this);
                  score()->cmdAdd(volta);
                  }
                  break;
            case OTTAVA:
                  {
                  Ottava* ottava = new Ottava(score());
                  ottava->read(node);
                  ottava->setStaff(staff);
                  ottava->setTick1(tick());
                  ottava->setTick2(tick() + tickLen());
                  ottava->setParent(this);
                  score()->cmdAdd(ottava);
                  }
                  break;
            case HAIRPIN:
                  {
                  Hairpin* hairpin = new Hairpin(score());
                  hairpin->read(node);
                  hairpin->setStaff(staff);
                  hairpin->setTick1(tick());
                  hairpin->setTick2(tick() + tickLen());
                  hairpin->setParent(this);
                  score()->cmdAdd(hairpin);
                  }
                  break;
            case DYNAMIC:
                  {
                  Dynamic* dynamic = new Dynamic(score());
                  dynamic->read(node);
                  dynamic->setSelected(false);
                  Dynamic* nd = new Dynamic(score());
                  nd->setSubtype(dynamic->subtype());
                  if (dynamic->subtype() == 0)
                        nd->setText(dynamic->getText());
                  score()->addDynamic(nd, p);
                  }
                  break;
            case TRILL:
                  {
                  Trill* trill = new Trill(score());
                  trill->setStaff(staff);
                  trill->setTick1(tick());
                  int lt = _last->tick();
                  trill->setTick2(lt);
                  trill->setParent(this);
                  score()->cmdAdd(trill);
                  }
                  break;
            case PEDAL:
                  {
                  Pedal* pedal = new Pedal(score());
                  pedal->setStaff(staff);
                  pedal->setTick1(tick());
                  pedal->setTick2(tick() + tickLen());
                  pedal->setParent(this);
                  score()->cmdAdd(pedal);
                  }
                  break;
            case LAYOUT_BREAK:
                  {
                  LayoutBreak* lb = new LayoutBreak(score());
                  lb->read(node);
                  lb->setStaff(staff);
                  lb->setParent(this);
                  score()->cmdAdd(lb);
                  }
                  break;

            case BAR_LINE:
                  {
                  BarLine* barLine = new BarLine(score());
                  barLine->read(node);
                  int subtype = barLine->subtype();
                  delete barLine;

                  for (int i = 0; i < staves.size(); ++i) {
                        MStaff& s = staves[i];
                        Staff* staff = score()->staff(i);
                        if (!staff->isTop())
                              continue;
                        if (subtype == START_REPEAT) {
                              for (Segment* s = _first; s != _last; s = s->next()) {
                                    if (s->segmentType() == Segment::SegBarLine) {
                                          Element* e = s->element(i * VOICES);
                                          if (e && e->type() == BAR_LINE && e->subtype() == START_REPEAT) {
                                                // BarLine already there,
                                                // do nothing
                                                return;
                                                }
                                          }
                                    }
                              Segment::SegmentType st = Segment::SegBarLine;
                              Segment* seg = findSegment(st, tick());
                              if (seg == 0) {
                                    seg = createSegment(st, tick());
                                    score()->undoOp(UndoOp::AddElement, seg);
                                    }
                              BarLine* bl = new BarLine(score());
                              bl->setSubtype(subtype);
                              bl->setStaff(staff);
                              bl->setParent(seg);
                              score()->cmdAdd(bl);
                              bl = 0;
                              Measure* m = system()->prevMeasure(this);
                              if (m) {
                                    MStaffList* sl = m->staffList();
                                    MStaff& ms = (*sl)[i];
                                    if (ms.endBarLine)
                                          score()->cmdRemove(ms.endBarLine);
                                    }
                              }
                        else {
                              //
                              // if next measure is a start repeat, look for a
                              // start repeat barline and remove it
                              //
                              Measure* m = system()->nextMeasure(this);
                              if (m && m->startRepeat()) {
                                    for (Segment* s = m->first(); s != m->last(); s = s->next()) {
                                          if (s->segmentType() == Segment::SegBarLine) {
                                                Element* e = s->element(i * VOICES);
                                                if (e && e->type() == BAR_LINE && e->subtype() == START_REPEAT) {
                                                      score()->cmdRemove(e);
                                                      m->cmdRemoveEmptySegment(s);
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              if (s.endBarLine)
                                    score()->cmdRemove(s.endBarLine);
                              BarLine* bl = new BarLine(score());
                              bl->setSubtype(subtype);
                              bl->setStaff(staff);
                              bl->setParent(this);
                              score()->cmdAdd(bl);
#if 0
                              for (Segment* s = _first; s != _last; s = s->next()) {
                                    if (s->type() == Segment::SegBarLine) {
                                          Element* e = s->element(i * VOICES);
                                          if (e && e->type() == BAR_LINE) {
                                                if (e->subtype() == START_REPEAT) {
                                                      }
                                                score()->cmdRemove(e);
                                                break;
                                                }
                                          }
                                    }
#endif
                              }
                        }
                  }
                  break;

            default:
                  break;
            }
      }

//---------------------------------------------------------
//   cmdRemoveEmptySegment
//---------------------------------------------------------

void Measure::cmdRemoveEmptySegment(Segment* s)
      {
      int tracks = _score->nstaves() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            if (s->element(track))
                  return;
            }
      _score->undoOp(UndoOp::RemoveElement, s);
      remove(s);
      }

//---------------------------------------------------------
//   genPropertyMenu
//---------------------------------------------------------

bool Measure::genPropertyMenu(QMenu* popup) const
      {
      QAction* a = popup->addAction(QT_TR_NOOP("set irregular"));
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
            SigList* sl = score()->sigmap;
            int t = tick();
            SigEvent oev = sl->timesig(t);
            SigEvent nev = im.sig();
            if (oev == nev)
                  return;
            int oldLen = oev.ticks;
            int newLen = nev.ticks;

            SigEvent ev1;
            iSigEvent i = sl->find(t);
            if (i != sl->end())
                  ev1 = i->second;
            score()->undoChangeSig(t, ev1, nev);
            i = sl->find(t + newLen);
            if (i != sl->end())
                  ev1 = i->second;
            else
                  ev1 = SigEvent();
            score()->undoChangeSig(t + newLen, ev1, oev);
            adjustToLen(oldLen, newLen);
            score()->layout();
            }
      }

//---------------------------------------------------------
//   adjustToLen
//    the measure len has changed, adjust elements to
//    new len
//---------------------------------------------------------

void Measure::adjustToLen(int ol, int nl)
      {
      //
      // Plan B: remove all elements and replace with
      //         measure rest
      //

      setTickLen(nl);
      Segment* crs = 0;
      int staves = score()->nstaves();

      for (Segment* s = first(); s;) {
            if (s->segmentType() == Segment::SegChordRest) {
                  crs = s;
                  Segment* ns = s->next();
                  if (ns)
                        remove(s);
                  else
                        crs = s;
                  s = ns;
                  }
            else
                  s = s->next();
            }
      if (crs) {
            int tracks = staves * VOICES;
            for (int i = 0; i < tracks; ++i) {
                  Element* el = crs->element(i);
                  if (el)
                        crs->setElement(i, 0);
                  }
            for (int i = 0; i < staves; ++i) {
                  Rest* rest = new Rest(score(), crs->tick(), nl);
                  rest->setStaff(score()->staff(i));
                  rest->setTickLen(nl);
                  crs->add(rest);
                  }
            }
      score()->adjustTime(tick() + tickLen(), next());
      }

