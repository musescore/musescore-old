//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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

#include <fenv.h>
#include "page.h"
#include "al/sig.h"
#include "key.h"
#include "clef.h"
#include "score.h"
#include "globals.h"
#include "segment.h"
#include "text.h"
#include "staff.h"
#include "style.h"
#include "timesig.h"
#include "scoreview.h"
#include "chord.h"
#include "note.h"
#include "slur.h"
#include "keysig.h"
#include "barline.h"
#include "repeat.h"
#include "box.h"
#include "system.h"
#include "part.h"
#include "utils.h"
#include "measure.h"
#include "preferences.h"
#include "volta.h"
#include "beam.h"
#include "tuplet.h"

//---------------------------------------------------------
//   rebuildBspTree
//---------------------------------------------------------

void Score::rebuildBspTree()
      {
      QRectF r;
      QList<Element*> el;
      for (MeasureBase* m = first(); m; m = m->next()) {
            if (m->type() == MEASURE && static_cast<const Measure*>(m)->multiMeasure() < 0)
                  continue;
            m->scanElements(&el, collectElements);
            }
      foreach(Page* page, _pages) {
            r |= page->abbox();
            page->scanElements(&el, collectElements);
            }
      foreach (Element* element, _gel) {
            if (element->type() == SLUR)
                  continue;
            if (element->track() != -1) {
                  if (!element->staff()->show())
                        continue;
                  }
            element->scanElements(&el, collectElements);
            }
      foreach(Beam* b, _beams)
            el.append(b);

      int n = el.size();
      bspTree.initialize(r, n);
      for (int i = 0; i < n; ++i)
            bspTree.insert(el.at(i));
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

MeasureBase* Score::first() const
      {
      return _measures.first();
      }

//---------------------------------------------------------
//   last
//---------------------------------------------------------

MeasureBase* Score::last()  const
      {
      return _measures.last();
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

Element* Score::searchNote(int tick, int track) const
      {
      int startTrack = track;
      int endTrack   = startTrack + 1;

      for (const MeasureBase* mb = _measures.first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* measure = (Measure*)mb;
            Element* ipe = 0;
            for (int track = startTrack; track < endTrack; ++track) {
                  for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                        Element* ie  = segment->element(track);
                        if (!ie)
                              continue;
                        if (!ie->isChordRest())
                              continue;
                        if (ie->tick() == tick)
                              return ie;
                        if (ie->tick() >  tick)
                              return ipe ? ipe : ie;
                        ipe = ie;
                        }
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   clefOffset
//---------------------------------------------------------

int Score::clefOffset(int tick, Staff* staff) const
      {
      return clefTable[staff->clefList()->clef(tick)].yOffset;
      }

//---------------------------------------------------------
//   AcEl
//---------------------------------------------------------

struct AcEl {
      Note* note;
      double x;
      };

//---------------------------------------------------------
//   layoutChords
//    only called from layout0
//    - calculate displaced note heads
//---------------------------------------------------------

void Score::layoutChords1(Segment* segment, int staffIdx)
      {
      Staff* staff = Score::staff(staffIdx);

      if (staff->part()->drumset())
            return;

      int startTrack = staffIdx * VOICES;
      int endTrack   = startTrack + VOICES;
      int voices     = 0;
      QList<Note*> notes;
      for (int track = startTrack; track < endTrack; ++track) {
            Element* e = segment->element(track);
            if (!e)
                 continue;
            // ++voices;
            if (e->type() == CHORD) {
                  ++voices;
                  notes.append(static_cast<Chord*>(e)->notes());
                  }
            }
      if (notes.isEmpty())
            return;

      int startIdx, endIdx, incIdx;

      if (notes[0]->chord()->up() || (voices > 1)) {
            startIdx = 0;
            incIdx   = 1;
            endIdx   = notes.size();
            for (int i = 0; i < endIdx-1; ++i) {
                  if ((notes[i]->line() == notes[i+1]->line())
                     && (notes[i]->track() != notes[i+1]->track())
                     && (!notes[i]->chord()->up() && notes[i+1]->chord()->up())
                     ) {
                        Note* n = notes[i];
                        notes[i] = notes[i+1];
                        notes[i+1] = n;
                        }
                  }
            }
      else {
            startIdx = notes.size() - 1;
            incIdx   = -1;
            endIdx   = -1;
            }

      bool moveLeft = false;
      int ll        = 1000;      // line distance to previous note head
      bool isLeft   = notes[startIdx]->chord()->up();
      int move1     = notes[startIdx]->chord()->staffMove();
      bool mirror   = false;
      int lastHead  = -1;

      for (int idx = startIdx; idx != endIdx; idx += incIdx) {
            Note* note   = notes[idx];
            Chord* chord = note->chord();
            int move     = chord->staffMove();
            int line     = note->line();
            int ticks    = chord->tickLen();
            int head     = note->noteHead();      // symbol number or note head

            bool conflict = (qAbs(ll - line) < 2) && (move1 == move);
            bool sameHead = (ll == line) && (head == lastHead);
            if ((chord->up() != isLeft) || conflict)
                  isLeft = !isLeft;
            bool nmirror  = (chord->up() != isLeft) && !sameHead;

            note->setHidden(false);
            chord->rxpos() = 0.0;

            if (conflict && (nmirror == mirror) && idx) {
                  if (sameHead) {
                        Note* pnote = notes[idx-1];
                        if (note->userOff().isNull() && pnote->userOff().isNull()) {
                              if (ticks > pnote->chord()->tickLen()) {
                                    pnote->setHidden(true);
                                    pnote->setAccidentalType(ACC_NONE);
                                    note->setHidden(false);
                                    }
                              else {
                                    note->setAccidentalType(ACC_NONE);
                                    note->setHidden(true);
                                    }
                              }
                        else
                              note->setHidden(false);
                        }
                  else {
                        if ((line > ll) || !chord->up()) {
                              note->chord()->rxpos() = note->headWidth() - note->point(styleS(ST_stemWidth));
                              note->rxpos() = 0.0;
                              }
                        else {
                              notes[idx-incIdx]->chord()->rxpos() = note->headWidth() - note->point(styleS(ST_stemWidth));
                              note->rxpos() = 0.0;
                              }
                        moveLeft = true;
                        }
                  }
            if (note->userMirror() == DH_AUTO) {
                  mirror = nmirror;
                  }
            else {
                  mirror = note->chord()->up();
                  if (note->userMirror() == DH_LEFT)
                        mirror = !mirror;
                  }
            note->setMirror(mirror);
            if (mirror)                   //??
                  moveLeft = true;

            move1    = move;
            ll       = line;
            lastHead = head;
            }

      //---------------------------------------------------
      //    layout accidentals
      //---------------------------------------------------

      QList<AcEl> aclist;

      int nNotes = notes.size();
      for (int i = nNotes-1; i >= 0; --i) {
            Note* note     = notes[i];
            Accidental* ac = note->accidental();
            if (ac) {
                  ac->setMag(note->mag());
                  ac->layout();
                  AcEl acel;
                  acel.note = note;
                  acel.x    = 0.0;
                  aclist.append(acel);
                  }
            }
      int nAcc = aclist.size();
      if (nAcc == 0)
            return;
      double pd  = point(styleS(ST_accidentalDistance));
      double pnd = point(styleS(ST_accidentalNoteDistance));
      //
      // layout top accidental
      //
      Note* note      = aclist[0].note;
      Accidental* acc = note->accidental();
      aclist[0].x     = -pnd * acc->mag() - acc->width() - acc->bbox().x();

      //
      // layout bottom accidental
      //
      if (nAcc > 1) {
            note = aclist[nAcc-1].note;
            acc  = note->accidental();
            int l1 = aclist[0].note->line();
            int l2 = note->line();

            int st1   = aclist[0].note->accidental()->subtype();
            int st2   = acc->subtype();
            int ldiff = st1 == ACC_FLAT ? 4 : 5;

            if (qAbs(l1-l2) > ldiff) {
                  aclist[nAcc-1].x = -pnd * acc->mag() - acc->width() - acc->bbox().x();
                  }
            else {
                  if ((st1 == ACC_FLAT) && (st2 == ACC_FLAT) && (qAbs(l1-l2) > 2))
                        aclist[nAcc-1].x = aclist[0].x - acc->width() * .5;
                  else
                        aclist[nAcc-1].x = aclist[0].x - acc->width();
                  }
            }

      //
      // layout middle accidentals
      //
      if (nAcc > 2) {
            int n = nAcc - 1;
            for (int i = 1; i < n; ++i) {
                  note = aclist[i].note;
                  acc  = note->accidental();
                  int l1 = aclist[i-1].note->line();
                  int l2 = note->line();
                  int l3 = aclist[n].note->line();
                  double x = 0.0;

                  int st1 = aclist[i-1].note->accidental()->subtype();
                  int st2 = acc->subtype();

                  int ldiff = st1 == ACC_FLAT ? 4 : 5;
                  if (qAbs(l1-l2) <= ldiff) {   // overlap accidental above
                        if ((st1 == ACC_FLAT) && (st2 == ACC_FLAT) && (qAbs(l1-l2) > 2))
                              x = aclist[i-1].x + acc->width() * .5;    // undercut flats
                        else
                              x = aclist[i-1].x;
                        }

                  ldiff = acc->subtype() == ACC_FLAT ? 4 : 5;
                  if (qAbs(l2-l3) <= ldiff) {       // overlap accidental below
                        if (aclist[n].x < x)
                              x = aclist[n].x;
                        }
                  if (x == 0.0 || x > acc->width())
                        x = -pnd * acc->mag() - acc->bbox().x();
                  else
                        x -= pd * acc->mag();   // accidental distance
                  aclist[i].x = x - acc->width() - acc->bbox().x();
                  }
            }

      foreach(const AcEl e, aclist) {
            Note* note = e.note;
            double x    = e.x;
            if (moveLeft) {
                  Chord* chord = note->chord();
                  if (((note->mirror() && chord->up()) || (!note->mirror() && !chord->up())))
                        x -= note->headWidth();
                  }
            note->accidental()->setPos(x, 0);
            }
      }

//-------------------------------------------------------------------
//    layoutStage1
//    - compute note head lines and accidentals
//    - mark multi measure rest breaks if in multi measure rest mode
//-------------------------------------------------------------------

void Score::layoutStage1()
      {
      int idx = 0;
      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            ++idx;
            m->setDirty();
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
                  KeySigEvent key = staff(staffIdx)->keymap()->key(m->tick());

                  char tversatz[75];      // list of already set accidentals for this measure
                  initLineList(tversatz, key.accidentalType());

                  m->setBreakMMRest(false);
                  if (styleB(ST_createMultiMeasureRests)) {
                        if ((m->repeatFlags() & RepeatStart) || (m->prevMeasure() && (m->prevMeasure()->repeatFlags() & RepeatEnd)))
                              m->setBreakMMRest(true);
                        else {
                              foreach (Element* e, *m->el()) {
                                    if ((e->type() == TEXT) && (e->subtype() == TEXT_REHEARSAL_MARK))
                                          m->setBreakMMRest(true);
                                    else if (e->type() == TEMPO_TEXT)
                                          m->setBreakMMRest(true);
                                    }
                              if (!m->breakMMRest()) {
                                    // TODO: this is slow!
                                    foreach(const Element* el, _gel) {
                                          if (el->type() == VOLTA) {
                                                const Volta* volta = static_cast<const Volta*>(el);
                                                if (m->tick() == volta->tick2()) {
                                                      m->setBreakMMRest(true);
                                                      break;
                                                      }
                                                }
                                          }
                                    }
                              }
                        }

                  int track = staffIdx * VOICES;

                  for (Segment* segment = m->first(); segment; segment = segment->next()) {
                        Element* e = segment->element(track);

                        if (segment->subtype() == SegKeySig
                           || segment->subtype() == SegStartRepeatBarLine
                           || segment->subtype() == SegTimeSig) {
                              if (e && !e->generated())
                                    m->setBreakMMRest(true);
                              }

                        if ((segment->subtype() == SegChordRest) || (segment->subtype() == SegGrace))
                              m->layoutChords0(segment, staffIdx * VOICES, tversatz);
                        if (e && e->type() == KEYSIG) {
                              KeySigEvent oval = staff(staffIdx)->keymap()->key(e->tick() - 1);
                              static_cast<KeySig*>(e)->setOldSig(oval.accidentalType());
                              }
                        }
                  }
            MeasureBase* mb = m->prev();
            if (mb && mb->type() == MEASURE) {
                  Measure* prev = static_cast<Measure*>(mb);
                  if (prev->endBarLineType() != NORMAL_BAR && prev->endBarLineType() != BROKEN_BAR)
                        m->setBreakMMRest(true);
                  }
            }
      }

//---------------------------------------------------------
//   layoutStage2
//    auto - beamer
//---------------------------------------------------------

void Score::layoutStage2()
      {
      int tracks = nstaves() * VOICES;

      foreach(Beam* beam, _beams)
            beam->clear();
      for (int track = 0; track < tracks; ++track) {
            ChordRest* a1 = 0;      // start of (potential) beam
            Beam* beam = 0;
            Beam* oldBeam = 0;
            Measure* measure = 0;

            BeamMode bm = BEAM_AUTO;
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  Element* e = segment->element(track);
                  if (((segment->subtype() != SegChordRest) && (segment->subtype() != SegGrace))
                     || e == 0 || !e->isChordRest())
                        continue;
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  if (cr->measure() != measure) {
                        if (measure && (bm != BEAM_MID)) {
                              if (beam)
                                    beam->layout1();
                              else if (a1) {
                                    a1->setBeam(0);
                                    a1->layoutStem1();
                                    }
                              }
                        measure = cr->measure();
                        if (bm != BEAM_MID) {
                              a1      = 0;
                              beam    = 0;
                              oldBeam = 0;
                              }
                        }
                  if (segment->subtype() == SegGrace) {
                        Segment* nseg = segment->next();
                        if (nseg && nseg->subtype() == SegGrace && nseg->element(track)) {
                              Beam* b = cr->beam();
                              if (b == 0) {
                                    b = new Beam(this);
                                    b->setTrack(track);
                                    b->setGenerated(true);
                                    add(b);
                                    }
                              b->add(cr);
                              Segment* s = nseg;
                              for (;;) {
                                    nseg = s;
                                    ChordRest* cr = static_cast<ChordRest*>(nseg->element(track));
                                    b->add(cr);
                                    s = nseg->next();
                                    if (!s || (s->subtype() != SegGrace) || !s->element(track))
                                          break;
                                    }
                              b->layout1();
                              segment = nseg;
                              }
                        else {
                              cr->setBeam(0);
                              cr->layoutStem1();
                              }
                        continue;
                        }
                  bm          = cr->beamMode();
                  int len     = cr->duration().ticks();

                  if ((len >= AL::division) || (bm == BEAM_NO)) {
                        if (beam) {
                              beam->layout1();
                              beam = 0;
                              }
                        if (a1) {
                              a1->setBeam(0);
                              a1->layoutStem1();
                              a1 = 0;
                              }
                        cr->setBeam(0);
                        cr->layoutStem1();
                        continue;
                        }
                  bool beamEnd = false;
                  if (beam) {
                        ChordRest* le = beam->elements().back();
                        if (((bm != BEAM_MID) && (le->tuplet() != cr->tuplet())) || (bm == BEAM_BEGIN)) {
                              beamEnd = true;
                              }
                        else if (bm != BEAM_MID) {
                              Fraction f(sigmap()->timesig(cr->tick()).fraction());
                              if (endBeam(f, cr, cr->tick() - measure->tick()))
                                    beamEnd = true;
                              }
                        if (beamEnd) {
                              beam->layout1();
                              beam = 0;
                              a1   = 0;
                              }
                        else {
                              beam->add(cr);
                              cr = 0;

                              // is this the last beam element?
                              if (bm == BEAM_END) {
                                    beam->layout1();
                                    beam = 0;
                                    }
                              }
                        }
                  if (cr && cr->tuplet() && (cr->tuplet()->elements().back() == cr)) {
                        if (beam) {
                              beam->layout1();
                              beam = 0;

                              cr->setBeam(0);
                              cr->layoutStem1();
                              }
                        else if (a1) {
                              beam = a1->beam();
                              if (beam == 0 || (beam == oldBeam)) {
                                    beam = new Beam(this);
                                    beam->setTrack(track);
                                    beam->setGenerated(true);
                                    add(beam);
                                    }
                              oldBeam = beam;
                              beam->add(a1);
                              beam->add(cr);
                              a1 = 0;
                              beam->layout1();
                              beam = 0;
                              }
                        else {
                              cr->setBeam(0);
                              cr->layoutStem1();
                              }
                        }
                  else if (cr) {
                        if (a1 == 0)
                              a1 = cr;
                        else {
                              Fraction f(sigmap()->timesig(cr->tick()).fraction());
                              if (bm != BEAM_MID
                                 &&
                                   (endBeam(f, cr, cr->tick() - measure->tick())
                                   || bm == BEAM_BEGIN
                                   || (a1->segment()->subtype() != cr->segment()->subtype())
                                   )
                                 ) {
                                    a1->setBeam(0);
                                    a1->layoutStem1();      //?
                                    a1 = cr;
                                    }
                              else {
                                    beam = a1->beam();
                                    if (beam == 0 || (beam == oldBeam)) {
                                          beam = new Beam(this);
                                          beam->setGenerated(true);
                                          beam->setTrack(track);
                                          add(beam);
                                          }
                                    oldBeam = beam;
                                    beam->add(a1);
                                    beam->add(cr);
                                    a1 = 0;
                                    }
                              }
                        }
                  }
            if (beam)
                  beam->layout1();
            else if (a1) {
                  a1->setBeam(0);
                  a1->layoutStem1();
                  }
            }
      foreach (Beam* beam, _beams) {
            if (beam->elements().isEmpty()) {
                  remove(beam);
                  delete beam;
                  }
            }
      }

//---------------------------------------------------------
//   layoutStage3
//---------------------------------------------------------

void Score::layoutStage3()
      {
      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  if ((segment->subtype() == SegChordRest) || (segment->subtype() == SegGrace)) {
                        layoutChords1(segment, staffIdx);
                        }
                  }
            }
      }

//---------------------------------------------------------
//   layout
//    - measures are akkumulated into systems
//    - systems are akkumulated into pages
//   already existent systems and pages are reused
//---------------------------------------------------------

void Score::doLayout()
      {
      _needLayout = false;

#if 0 // DEBUG
      if (startLayout) {
            startLayout->setDirty();
            if (doReLayout()) {
                  startLayout = 0;
                  return;
                  }
            }
#endif
      if (_staves.isEmpty() || first() == 0) {
            // score is empty
            foreach(Page* page, _pages)
                  delete page;
            _pages.clear();

            Page* page = addPage();
            page->layout();
            page->setNo(0);
            page->setPos(0.0, 0.0);

            QRectF r = page->abbox();
            bspTree.initialize(r, 0);     // clear bspTree
            return;
            }

      layoutStage1();   // compute note head lines and accidentals
      layoutStage2();   // beam notes, finally decide if chord is up/down
      layoutStage3();   // compute note head horizontal positions

      //-----------------------------------------------------------------------
      //    layout measures into systems and pages
      //-----------------------------------------------------------------------

      curMeasure  = first();
      curSystem   = 0;
      firstSystem = true;
      for (curPage = 0; curMeasure; curPage++) {
            getCurPage();
            MeasureBase* om = curMeasure;
            if (!layoutPage())
                  break;
            if (curMeasure == om) {
                  printf("empty page?\n");
                  break;
                  }
            }

      //---------------------------------------------------
      //   place ties & slurs & hairpins & beams
      //---------------------------------------------------

      foreach(Beam* beam, _beams)
            beam->layout();

      int tracks = nstaves() * VOICES;
      for (int track = 0; track < tracks; ++track) {
            for (Segment* segment = firstSegment(); segment; segment = segment->next1()) {
                  Element* e = segment->element(track);
                  if (e && e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if (cr->beam())
                              continue;
                        cr->layoutStem();
                        }
                  }
            }


      for (Measure* m = firstMeasure(); m; m = m->nextMeasure()) {
            //if (m->multiMeasure() >= 0)
                  m->layout2();
            }

      foreach (Element* el, _gel) {
            if (el)
                  el->layout();
            }

      //---------------------------------------------------
      //    remove remaining pages and systems
      //---------------------------------------------------

      int n = _pages.size() - curPage;
      for (int i = 0; i < n; ++i) {
            Page* page = _pages.takeLast();
            delete page;
            }
      n = _systems.size() - curSystem;
      for (int i = 0; i < n; ++i) {
            System* system = _systems.takeLast();
            delete system;
            }
      rebuildBspTree();
      emit layoutChanged();
      }

//---------------------------------------------------------
//   processSystemHeader
//    add generated timesig keysig and clef
//---------------------------------------------------------

void Score::processSystemHeader(Measure* m, bool isFirstSystem)
      {
      int tick = m->tick();
      int i = 0;
      foreach(Staff* staff, _staves) {
            if (!m->system()->staff(i)->show()) {
                  ++i;
                  continue;
                  }

            KeySig* hasKeysig = 0;
            Clef*   hasClef   = 0;
            int strack        = i * VOICES;

            // we assume that keysigs and clefs are only in the first
            // track of a segment

            const KeySigEvent& keyIdx = staff->keymap()->key(tick);

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  // search only up to the first ChordRest
                  if (seg->subtype() == SegChordRest)
                        break;
                  Element* el = seg->element(strack);
                  if (!el)
                        continue;
                  switch (el->type()) {
                        case KEYSIG:
                              hasKeysig = static_cast<KeySig*>(el);
                              hasKeysig->changeType(keyIdx);
                              hasKeysig->setMag(staff->mag());
                              break;
                        case CLEF:
                              hasClef = static_cast<Clef*>(el);
                              hasClef->setMag(staff->mag());
                              hasClef->setSmall(false);
                              break;
                        case TIMESIG:
                              el->setMag(staff->mag());
                              break;
                        default:
                              break;
                        }
                  }
            bool needKeysig = keyIdx.isValid() && (isFirstSystem || styleB(ST_genKeysig));
            if (needKeysig && !hasKeysig) {
                  //
                  // create missing key signature
                  //
                  KeySig* ks = keySigFactory(keyIdx);
                  ks->setTrack(i * VOICES);
                  ks->setTick(tick);
                  ks->setGenerated(true);
                  ks->setMag(staff->mag());
                  Segment* seg = m->getSegment(ks);
                  seg->add(ks);
                  m->setDirty();
                  }
            else if (!needKeysig && hasKeysig) {
                  int track = hasKeysig->track();
                  Segment* seg = hasKeysig->segment();
                  seg->setElement(track, 0);    // TODO: delete element
                  m->setDirty();
                  }
            bool needClef = isFirstSystem || styleB(ST_genClef);
            if (needClef) {
                  int idx = staff->clefList()->clef(tick);
                  if (!hasClef) {
                        //
                        // create missing clef
                        //
                        hasClef = new Clef(this);
                        hasClef->setTrack(i * VOICES);
                        hasClef->setTick(tick);
                        hasClef->setGenerated(true);
                        hasClef->setSmall(false);
                        hasClef->setMag(staff->mag());
                        Segment* s = m->getSegment(hasClef);
                        s->add(hasClef);
                        m->setDirty();
                        }
                  hasClef->setSubtype(idx);
                  }
            else {
                  if (hasClef) {
                        int track = hasClef->track();
                        Segment* seg = hasClef->segment();
                        seg->setElement(track, 0);    // TODO: delete element
                        m->setDirty();
                        }
                  }
            ++i;
            }
      }

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* Score::getNextSystem(bool isFirstSystem, bool isVbox)
      {
      System* system;
      if (curSystem >= _systems.size()) {
            system = new System(this);
            _systems.append(system);
            }
      else {
            system = _systems[curSystem];
            system->clear();   // remove measures from system
            }
      system->setFirstSystem(isFirstSystem);
      system->setVbox(isVbox);
      if (!isVbox) {
            int nstaves = Score::nstaves();
            for (int i = system->staves()->size(); i < nstaves; ++i)
                  system->insertStaff(i);
            int dn = system->staves()->size() - nstaves;
            for (int i = 0; i < dn; ++i)
                  system->removeStaff(system->staves()->size()-1);
            }
      return system;
      }

//---------------------------------------------------------
//   getCurPage
//---------------------------------------------------------

void Score::getCurPage()
      {
      Page* page = curPage >= _pages.size() ? addPage() : _pages[curPage];
      page->setNo(curPage);
      page->layout();
      double x = (curPage == 0) ? 0.0 : _pages[curPage - 1]->pos().x()
         + page->width() + ((curPage & 1) ? 1.0 : 50.0);
      page->setPos(x, 0.0);
      }

//---------------------------------------------------------
//   layoutPage
//    return true, if next page must be relayouted
//---------------------------------------------------------

bool Score::layoutPage()
      {
      Page* page = _pages[curPage];
      const double slb = point(styleS(ST_staffLowerBorder));
      const double sub = point(styleS(ST_staffUpperBorder));

      // usable width of page:
      qreal w  = page->loWidth() - page->lm() - page->rm();
      qreal x  = page->lm();
      qreal ey = page->loHeight() - page->bm() - slb;

      page->clear();
      qreal y = page->tm();

      int rows = 0;
      bool firstSystemOnPage = true;

      while (curMeasure) {
            if (curMeasure->type() == VBOX) {
                  System* system = getNextSystem(false, true);

                  foreach(SysStaff* ss, *system->staves())
                        delete ss;
                  system->staves()->clear();

                  system->setWidth(w);
                  VBox* vbox = static_cast<VBox*>(curMeasure);
                  vbox->setParent(system);
                  vbox->layout();
                  double bh = vbox->height();

                  // put at least one system on page
                  if (((y + bh) > ey) && !firstSystemOnPage)
                        break;

                  system->setPos(x, y);
                  system->setHeight(bh);

                  system->measures().push_back(vbox);
                  page->appendSystem(system);

                  curMeasure = curMeasure->next();
                  ++curSystem;
                  y += bh + point(styleS(ST_frameSystemDistance));
                  if (y > ey) {
                        ++rows;
                        break;
                        }
                  }
            else {
                  if (firstSystemOnPage)
                        y += sub;
                  int cs          = curSystem;
                  MeasureBase* cm = curMeasure;
                  double h;
                  QList<System*> sl = layoutSystemRow(x, y, w, firstSystem, &h);
                  if (sl.isEmpty()) {
                        printf("layoutSystemRow returns zero systems\n");
                        abort();
                        }

                  // a page contains at least one system
                  if (rows && (y + h > ey)) {
                        // system does not fit on page: rollback
                        curMeasure = cm;
                        curSystem  = cs;
                        break;
                        }

                  foreach (System* system, sl) {
                        page->appendSystem(system);
                        system->rypos() = y;
                        }
                  firstSystem       = false;
                  firstSystemOnPage = false;
                  y += h;
                  if (sl.back()->pageBreak()) {
                        ++rows;
                        break;
                        }
                  }
            ++rows;
            }

      //-----------------------------------------------------------------------
      // if remaining y space on page is greater (pageHeight*pageFillLimit)
      // then increase system distance to fill page
      //-----------------------------------------------------------------------

      double restHeight = ey - y;
      double ph         = page->loHeight() - page->bm() - page->tm() - slb - sub;

      if ((rows <= 1) || (restHeight > (ph * (1.0 - styleD(ST_pageFillLimit)))))
            return true;

      double systemDistance = point(styleS(ST_systemDistance));
      double extraDist      = (restHeight + systemDistance) / (rows - 1);
      y = 0;
      int n = page->systems()->size();
      for (int i = 0; i < n;) {
            System* system = page->systems()->at(i);
            double yy = system->pos().y();
            system->move(0, y);
            ++i;
            if (i >= n)
                  break;
            System* nsystem = page->systems()->at(i);
            if (nsystem->pos().y() != yy)
                  y += extraDist;               // next system row
            }
      return true;
      }

//---------------------------------------------------------
//   skipEmptyMeasures
//    search for empty measures; return number if empty
//    measures in sequence
//---------------------------------------------------------

Measure* Score::skipEmptyMeasures(Measure* m, System* system)
      {
      Measure* sm = m;
      int n       = 0;
      while (m->isEmpty()) {
            MeasureBase* mb = m->next();
            if (m->breakMultiMeasureRest() && n)
                  break;
            ++n;
            if (!mb || (mb->type() != MEASURE))
                  break;
            m = static_cast<Measure*>(mb);
            }
      m = sm;
      if (n >= styleI(ST_minEmptyMeasures)) {
            m->setMultiMeasure(n);  // first measure is presented as multi measure rest
            m->setSystem(system);
            for (int i = 1; i < n; ++i) {
                  m = static_cast<Measure*>(m->next());
                  m->setMultiMeasure(-1);
                  m->setSystem(system);
                  }
            }
      else
            m->setMultiMeasure(0);
      return m;
      }

//---------------------------------------------------------
//   layoutSystem1
//    return true on line break
//---------------------------------------------------------

bool Score::layoutSystem1(double& minWidth, double w, bool isFirstSystem)
      {
      System* system = getNextSystem(isFirstSystem, false);

      double xo = 0;
      if (curMeasure->type() == HBOX)
            xo = point(static_cast<Box*>(curMeasure)->boxWidth());

      system->setInstrumentNames();
      system->layout(xo);

      minWidth            = system->leftMargin();
      double systemWidth  = w;

      bool continueFlag   = false;

      int nstaves = Score::nstaves();
      bool isFirstMeasure = true;

      for (; curMeasure;) {
            MeasureBase* nextMeasure;
            if (curMeasure->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  if (styleB(ST_createMultiMeasureRests)) {
                        nextMeasure = skipEmptyMeasures(m, system)->next();
                        }
                  else {
                        m->setMultiMeasure(0);
                        nextMeasure = curMeasure->next();
                        }
                  }
            else
                  nextMeasure = curMeasure->next();

            System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);
            double ww      = 0.0;
            double stretch = 0.0;

            if (curMeasure->type() == HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure) {
                        // try to put another system on current row
                        // if not a line break
                        continueFlag = !curMeasure->lineBreak();
                        }
                  }
            else if (curMeasure->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  if (isFirstMeasure)
                        processSystemHeader(m, isFirstSystem);

                  //
                  // remove generated elements
                  //    assume: generated elements are only living in voice 0
                  //    TODO: check if removed elements can be deleted
                  //
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->subtype() == SegEndBarLine)
                              continue;
                        for (int staffIdx = 0;  staffIdx < nstaves; ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Element* el = seg->element(track);
                              if (el == 0)
                                    continue;
                              if (el->generated()) {
                                    if (!isFirstMeasure || (seg->subtype() == SegTimeSigAnnounce))
                                          seg->setElement(track, 0);
                                    }
                              double staffMag = staff(staffIdx)->mag();
                              if (el->type() == CLEF) {
                                    Clef* clef = static_cast<Clef*>(el);
                                    clef->setSmall(!isFirstMeasure || (seg != m->first()));
                                    clef->setMag(staffMag);
                                    }
                              else if (el->type() == KEYSIG || el->type() == TIMESIG)
                                    el->setMag(staffMag);
                              }
                        }

                  m->createEndBarLines();

                  m->layoutX(1.0);
                  ww      = m->layoutWidth().stretchable;
                  stretch = m->userStretch() * styleD(ST_measureSpacing);

                  ww *= stretch;
                  if (ww < point(styleS(ST_minMeasureWidth)))
                        ww = point(styleS(ST_minMeasureWidth));
                  isFirstMeasure = false;
                  }

            // collect at least one measure
            if ((minWidth + ww > systemWidth) && !system->measures().isEmpty()) {
                  curMeasure->setSystem(oldSystem);
                  break;
                  }

            minWidth += ww;
            system->measures().append(curMeasure);
            int n = styleI(ST_FixMeasureNumbers);
            if ((n && system->measures().size() >= n)
               || continueFlag || curMeasure->pageBreak()
               || curMeasure->lineBreak()
               || (curMeasure->next() && curMeasure->next()->type() == VBOX)) {
                  system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = nextMeasure;
                  break;
                  }
            else
                  curMeasure = nextMeasure;
            }

      //
      //    hide empty staves
      //
      bool showChanged = false;
      int staves = system->staves()->size();
      int staffIdx = 0;
      foreach (Part* p, _parts) {
            int nstaves   = p->nstaves();
            bool hidePart = false;

            if (styleB(ST_hideEmptyStaves) && (staves > 1)) {
                  hidePart = true;
                  for (int i = staffIdx; i < staffIdx + nstaves; ++i) {
                        foreach(MeasureBase* m, system->measures()) {
                              if (m->type() != MEASURE)
                                    continue;
                              if (!((Measure*)m)->isMeasureRest(i)) {
                                    hidePart = false;
                                    break;
                                    }
                              }
                        }
                  }

            for (int i = staffIdx; i < staffIdx + nstaves; ++i) {
                  SysStaff* s  = system->staff(i);
                  Staff* staff = Score::staff(i);
                  bool oldShow = s->show();
                  s->setShow(hidePart ? false : staff->show());
                  if (oldShow != s->show()) {
                        showChanged = true;
                        }
                  }
            staffIdx += nstaves;
            }
#if 0 // DEBUG: endless recursion can happen if number of measures change
      // relayout if stave's show status has changed
      if (showChanged) {
            minWidth = 0;
            curMeasure = firstMeasure;
            bool val = layoutSystem1(minWidth, w, isFirstSystem);
            return val;
            }
#endif
      return continueFlag && curMeasure;
      }

//---------------------------------------------------------
//   layoutSystemRow
//    x, y  position of row on page
//---------------------------------------------------------

QList<System*> Score::layoutSystemRow(qreal x, qreal y, qreal rowWidth,
   bool isFirstSystem, double* h)
      {
      bool raggedRight = layoutDebug;

      *h = 0.0;
      QList<System*> sl;

      double ww = rowWidth;
      double minWidth;
      for (bool a = true; a;) {
            a = layoutSystem1(minWidth, ww, isFirstSystem);
            sl.append(_systems[curSystem]);
            ++curSystem;
            ww -= minWidth;
            }

      //
      // dont stretch last system row, if minWidth is <= lastSystemFillLimit
      //
      if (curMeasure == 0 && ((minWidth / rowWidth) <= styleD(ST_lastSystemFillLimit)))
            raggedRight = true;

      //-------------------------------------------------------
      //    Round II
      //    stretch measures
      //    "nm" measures fit on this line of score
      //-------------------------------------------------------

      bool needRelayout = false;

      foreach(System* system, sl) {
            //
            //    add cautionary time/key signatures if needed
            //

            if (system->measures().isEmpty()) {
                  printf("system %p is empty\n", system);
                  abort();
                  }
            MeasureBase* lm = system->measures().back();
            while (lm && lm->type() != MEASURE)
                  lm = lm->prev();
            Measure* m = static_cast<Measure*>(lm);
            bool hasCourtesyKeysig = false;

            if (m) {
                  int tick            = lm->tick() + lm->tickLen();
                  AL::SigEvent sig1   = _sigmap->timesig(tick - 1);
                  AL::SigEvent sig2   = _sigmap->timesig(tick);
                  if (styleB(ST_genCourtesyTimesig) && !sig1.nominalEqual(sig2)) {
                        Segment* s  = m->getSegment(SegTimeSigAnnounce, tick);
                        int nstaves = Score::nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              if (s->element(track) == 0) {
                                    TimeSig* ts = new TimeSig(this, sig2.fraction().denominator(),
                                       sig2.fraction().numerator());
                                    Measure* nm = m->nextMeasure();
                                    if(nm){
                                        Segment* tss = nm->findSegment(SegTimeSig, tick);
                                        if(tss){
                                            TimeSig* nts = (TimeSig*)tss->element(0);
                                            if(nts)
                                                ts->setSubtype(nts->subtype());
                                            }
                                        }
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    ts->setMag(ts->staff()->mag());
                                    s->add(ts);
                                    needRelayout = true;
                                    }
                              }
                        }
                  if (styleB(ST_genCourtesyKeysig)) {
                        int n = _staves.size();
                        for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
                              Staff* staff = _staves[staffIdx];
                              KeySigEvent key1 = staff->key(tick - 1);
                              KeySigEvent key2 = staff->key(tick);
                              if (key1 != key2) {
                                    hasCourtesyKeysig = true;
                                    Segment* s  = m->getSegment(SegKeySigAnnounce, tick);
                                    int track = staffIdx * VOICES;
                                    if (!s->element(track)) {
                                          KeySig* ks = new KeySig(this);
                                          ks->setSig(key1.accidentalType(), key2.accidentalType());
                                          ks->setTrack(track);
                                          ks->setGenerated(true);
                                          ks->setMag(staff->mag());
                                          s->add(ks);
                                          needRelayout = true;
                                          }
                                    // change bar line to double bar line
                                    m->setEndBarLineType(DOUBLE_BAR, true);
                                    }
                              }
                        }
                  }

            const QList<MeasureBase*>& ml = system->measures();
            int n                         = ml.size();
            while (n > 0) {
                  if (ml[n-1]->type() == MEASURE)
                        break;
                  --n;
                  }

            //
            //    compute repeat bar lines
            //
            bool firstMeasure = true;
            MeasureBase* lmb = ml.back();
            if (lmb->type() == MEASURE) {
                  if (static_cast<Measure*>(lmb)->multiMeasure() > 0) {
                        for (;;lmb = lmb->next()) {
                              if (lmb->next() == 0)
                                    break;
                              if ((lmb->next()->type() == MEASURE) && ((Measure*)(lmb->next()))->multiMeasure() >= 0)
                                    break;
                              }
                        }
                  }
            for (MeasureBase* mb = ml.front(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE) {
                        if (mb == lmb)
                              break;
                        continue;
                        }
                  Measure* m = static_cast<Measure*>(mb);
                  // first measure repeat?
                  bool fmr = firstMeasure && (m->repeatFlags() & RepeatStart);

                  if (mb == ml.back()) {       // last measure in system?
                        //
                        // if last bar has a courtesy key signature,
                        // create a double bar line as end bar line
                        //
                        int bl = hasCourtesyKeysig ? DOUBLE_BAR : NORMAL_BAR;

                        if (m->repeatFlags() & RepeatEnd)
                              m->setEndBarLineType(END_REPEAT, true);
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(bl, true);
                        needRelayout |= m->setStartRepeatBarLine(fmr);
                        }
                  else {
                        MeasureBase* mb = m->next();
                        while (mb && mb->type() != MEASURE && (mb != ml.back()))
                              mb = mb->next();

                        Measure* nm = 0;
                        if (mb && mb->type() == MEASURE)
                              nm = static_cast<Measure*>(mb);

                        needRelayout |= m->setStartRepeatBarLine(fmr);
                        if (m->repeatFlags() & RepeatEnd) {
                              if (nm && (nm->repeatFlags() & RepeatStart))
                                    m->setEndBarLineType(END_START_REPEAT, true);
                              else
                                    m->setEndBarLineType(END_REPEAT, true);
                              }
                        else if (nm && (nm->repeatFlags() & RepeatStart))
                              m->setEndBarLineType(START_REPEAT, true);
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(NORMAL_BAR, true);
                        }
                  needRelayout |= m->createEndBarLines();
                  firstMeasure = false;
                  if (mb == lmb)
                        break;
                  }

            foreach (MeasureBase* mb, ml) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = static_cast<Measure*>(mb);
                  int nn = m->multiMeasure() - 1;
                  if (nn > 0) {
                        // skip to last rest measure of multi measure rest
                        Measure* mm = m;
                        for (int k = 0; k < nn; ++k)
                              mm = mm->nextMeasure();
                        if (mm) {
                              m->setMmEndBarLineType(mm->endBarLineType());
                              needRelayout |= m->createEndBarLines();
                              }
                        }
                  }
            }


      minWidth           = 0.0;
      double totalWeight = 0.0;

      foreach(System* system, sl) {
            foreach (MeasureBase* mb, system->measures()) {
                  if (mb->type() == HBOX)
                        minWidth += point(((Box*)mb)->boxWidth());
                  else {
                        Measure* m = (Measure*)mb;
                        if (needRelayout)
                              m->layoutX(1.0);
                        minWidth    += m->layoutWidth().stretchable;
                        totalWeight += m->tickLen() * m->userStretch();
                        }
                  }
            minWidth += system->leftMargin();
            }

      double rest = (raggedRight ? 0.0 : rowWidth - minWidth) / totalWeight;
      double xx   = 0.0;

      foreach(System* system, sl) {
            QPointF pos;

            bool firstMeasure = true;
            foreach(MeasureBase* mb, system->measures()) {
                  double ww = 0.0;
                  if (mb->type() == MEASURE) {
                        if (firstMeasure) {
                              pos.rx() += system->leftMargin();
                              firstMeasure = false;
                              }
                        mb->setPos(pos);
                        Measure* m    = static_cast<Measure*>(mb);
                        if (styleB(ST_FixMeasureWidth)) {
                              ww = rowWidth / system->measures().size();
                              }
                        else {
                              double weight = m->tickLen() * m->userStretch();
                              ww            = m->layoutWidth().stretchable + rest * weight;
                              }
                        m->layout(ww);
                        }
                  else if (mb->type() == HBOX) {
                        mb->setPos(pos);
                        ww = point(static_cast<Box*>(mb)->boxWidth());
                        mb->layout();
                        }
                  pos.rx() += ww;
                  }
            system->setPos(xx + x, y);
            double w = pos.x();
            system->setWidth(w);
            system->layout2();
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() == HBOX)
                        mb->setHeight(system->height());
                  }
            xx += w;
            double hh = system->height() + point(system->staves()->back()->distance());
            if (hh > *h)
                  *h = hh;
            }
      return sl;
      }

//---------------------------------------------------------
//   addPage
//---------------------------------------------------------

Page* Score::addPage()
      {
      Page* page = new Page(this);
      page->setNo(_pages.size());
      _pages.push_back(page);
      return page;
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

void Score::setPageFormat(const PageFormat& pf)
      {
      *_pageFormat = pf;
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void Score::setInstrumentNames()
      {
      for (iSystem is = systems()->begin(); is != systems()->end(); ++is)
            (*is)->setInstrumentNames();
      }

//---------------------------------------------------------
//   searchTieNote
//---------------------------------------------------------

static Note* searchTieNote(Note* note, Segment* segment, int track)
      {
      int pitch = note->pitch();

      while ((segment = segment->next1())) {
            Element* element = segment->element(track);
            if (element == 0 || element->type() != CHORD)
                  continue;
            Note* n = static_cast<Chord*>(element)->findNote(pitch);
            if (n)
                  return n;
            }
      return 0;
      }

//---------------------------------------------------------
//   connectTies
//---------------------------------------------------------

/**
 Rebuild tie connections.
*/

void Score::connectTies()
      {
      int tracks = nstaves() * VOICES;
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            for (Segment* s = m->first(); s; s = s->next()) {
                  for (int i = 0; i < tracks; ++i) {
                        Element* el = s->element(i);
                        if (el == 0 || el->type() != CHORD)
                              continue;
                        foreach(Note* n, static_cast<Chord*>(el)->notes()) {
                              Tie* tie = n->tieFor();
                              if (!tie)
                                    continue;
                              Note* nnote = searchTieNote(n, s, i);
                              if (nnote == 0) {
                                    printf("next note at %d(measure %d) voice %d for tie not found; delete tie\n",
                                       n->chord()->tick(),
                                       m->no(),
                                       n->chord()->voice()
                                       );
                                    n->setTieFor(0);
                                    delete tie;
                                    }
                              else {
                                    tie->setEndNote(nnote);
                                    nnote->setTieBack(tie);
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void Score::add(Element* el)
      {
      if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX) {
            measures()->add((MeasureBase*)el);
            }
      else {
            if (el->type() == BEAM) {
                  Beam* b = static_cast<Beam*>(el);
                  _beams.append(b);
                  foreach(ChordRest* cr, b->elements())
                        cr->setBeam(b);
                  }
            else if (el->check())
                  _gel.append(el);
            else {
                  printf("remove invalid element <%s>\n", el->name());
                  delete el;
                  }
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void Score::remove(Element* el)
      {
      if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX) {
            measures()->remove(static_cast<MeasureBase*>(el));
            }
      else if (el->type() == BEAM) {
            Beam* b = static_cast<Beam*>(el);
            if (_beams.removeOne(b)) {
                  foreach(ChordRest* cr, b->elements())
                        cr->setBeam(0);
                  }
            else
                  printf("Score::remove(): cannot find Beam\n");
            }
      else {
            if (!_gel.removeOne(el))
                  printf("Score::remove(): element %s not found\n", el->name());
            }
      }

//---------------------------------------------------------
//   reLayout
//---------------------------------------------------------

void Score::reLayout(Measure* m)
      {
      _needLayout = true;
      startLayout = m;
      }

//---------------------------------------------------------
//   doReLayout
//    return true, if relayout was successful; if false
//    a full layout must be done starting at "startLayout"
//---------------------------------------------------------

bool Score::doReLayout()
      {
#if 0
      if (startLayout->type() == MEASURE)
            static_cast<Measure*>(startLayout)->layout0();
#endif
      System* system  = startLayout->system();
      qreal sysWidth  = system->width();
      double minWidth = system->leftMargin();

      //
      //  check if measures still fit in system
      //
      MeasureBase* m = 0;
      foreach(m, system->measures()) {
            double ww;
            if (m->type() == HBOX)
                  ww = point(static_cast<Box*>(m)->boxWidth());
            else {            // MEASURE
                  Measure* measure = static_cast<Measure*>(m);
//TODOXX                  measure->layout0();
//TODO            measure->layoutBeams1();

                  measure->layoutX(1.0);
                  ww      = measure->layoutWidth().stretchable;
                  double stretch = measure->userStretch() * styleD(ST_measureSpacing);

                  ww *= stretch;
                  if (ww < point(styleS(ST_minMeasureWidth)))
                        ww = point(styleS(ST_minMeasureWidth));
                  }
            minWidth += ww;
            }
      if (minWidth > sysWidth)            // measure does not fit: do full layout
            return false;

      //
      // check if another measure will fit into system
      //
      m = m->next();
      if (m && m->subtype() == MEASURE) {
            Measure* measure = static_cast<Measure*>(m);
            measure->layoutX(1.0);
            double ww      = measure->layoutWidth().stretchable;
            double stretch = measure->userStretch() * styleD(ST_measureSpacing);

            ww *= stretch;
            if (ww < point(styleS(ST_minMeasureWidth)))
                  ww = point(styleS(ST_minMeasureWidth));
            if ((minWidth + ww) <= sysWidth)    // if another measure fits, do full layout
                  return false;
            }
      //
      // stretch measures
      //
      minWidth    = system->leftMargin();
      double totalWeight = 0.0;
      foreach (MeasureBase* mb, system->measures()) {
            if (mb->type() == HBOX)
                  minWidth += point(static_cast<Box*>(mb)->boxWidth());
            else {
                  Measure* m   = static_cast<Measure*>(mb);
                  minWidth    += m->layoutWidth().stretchable;
                  totalWeight += m->tickLen() * m->userStretch();
                  }
            }

      double rest = (sysWidth - minWidth) / totalWeight;

      bool firstMeasure = true;
      QPointF pos;
      foreach(MeasureBase* mb, system->measures()) {
            double ww = 0.0;
            if (mb->type() == MEASURE) {
                  if (firstMeasure) {
                        pos.rx() += system->leftMargin();
                        firstMeasure = false;
                        }
                  mb->setPos(pos);
                  Measure* m    = static_cast<Measure*>(mb);
                  double weight = m->tickLen() * m->userStretch();
                  ww            = m->layoutWidth().stretchable + rest * weight;
                  m->layout(ww);
                  }
            else if (mb->type() == HBOX) {
                  mb->setPos(pos);
                  ww = point(static_cast<Box*>(mb)->boxWidth());
                  mb->layout();
                  }
            pos.rx() += ww;
            }

      foreach(MeasureBase* mb, system->measures()) {
            if (mb->type() == MEASURE)
                  static_cast<Measure*>(mb)->layout2();
            }

      rebuildBspTree();
      return true;
      }

