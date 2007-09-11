//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: layout.cpp,v 1.61 2006/09/08 19:37:08 lvinken Exp $
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

#include "page.h"
#include "sig.h"
#include "key.h"
#include "clef.h"
#include "score.h"
#include "globals.h"
#include "segment.h"
#include "text.h"
#include "staff.h"
#include "style.h"
#include "layout.h"
#include "timesig.h"
#include "canvas.h"
#include "chord.h"
#include "note.h"
#include "slur.h"
#include "keysig.h"

//---------------------------------------------------------
//   intmaxlog
//---------------------------------------------------------

static inline int intmaxlog(int n)
      {
      return (n > 0 ? qMax(int(::ceil(::log(double(n))/::log(double(2)))), 5) : 0);
      }

//---------------------------------------------------------
//   ScoreLayout
//---------------------------------------------------------

ScoreLayout::ScoreLayout()
      {
      _spatium     = ::_spatium;
      _pageFormat  = new PageFormat;
      _systems     = new QList<System*>;
      _pages       = new PageList;
      _paintDevice = 0;
      }

ScoreLayout::~ScoreLayout()
      {
      if (_pageFormat)
            delete _pageFormat;
      if (_systems)
            delete _systems;
      if (_pages)
            delete _pages;
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreLayout::setScore(Score* s)
      {
      _score = s;
      _spatium = ::_spatium;
      _systems->clear();
      _pages->clear();
      _needLayout = false;
      _pageFormat = new PageFormat;
      _paintDevice = _score->canvas();
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

Element* Score::searchNote(int tick, int track) const
      {
      int startTrack, endTrack;
//      if (voice == -1) {
//            startTrack = track;
//            endTrack   = startTrack + VOICES;
//            }
//      else {
            startTrack = track;
            endTrack   = startTrack + 1;
//            }

      for (const Measure* measure = _layout->first(); measure; measure = measure->next()) {
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
      return clefTable[staff->clef()->clef(tick)].yOffset;
      }

//---------------------------------------------------------
//   layout
//    - measures are akkumulated into systems
//    - systems are akkumulated into pages
//   already existent systems and pages are reused
//---------------------------------------------------------

void ScoreLayout::doLayout()
      {
      ::_spatium = _spatium;        // needed for preview
      _needLayout = false;

      int n = _score->nstaves();
      for (int staffIdx = 0; staffIdx < n; ++staffIdx) {
            for (Element* m = _measures.first(); m; m = m->next())
                  ((Measure*)m)->layoutNoteHeads(staffIdx);
            }

      Measure* im = (Measure*)(_measures.first());
      iPage    ip = _pages->begin();
      iSystem  is = _systems->begin();

      if (im == 0) {
            foreach(Page* page, *_pages)
                  delete page;
            _pages->clear();

            Page* page = addPage();
            _pages->update();

            //---------------------------------------------------
            //    clear bspTree
            //---------------------------------------------------

            QRectF r = page->abbox();
            el.clear();
            int depth = intmaxlog(el.size());
            bspTree.initialize(r, depth);
            return;
            }

      //-----------------------------------------
      //    pass I:  process pages
      //-----------------------------------------

      while (im) {
            Page* page;
            if (ip == _pages->end()) {
                  page = addPage();
                  ip = _pages->end();
                  }
            else {
                  page = *ip++;
                  }
            _pages->update();

            Measure* om = im;
            layoutPage(page, im, is);
            if (im == om) {
                  printf("empty page?\n");
                  break;
                  }
            }

      //---------------------------------------------------
      //   pass II:  place ties & slurs & hairpins
      //---------------------------------------------------

      for (Measure* m = first(); m; m = m->next()) {
            m->layoutBeams(this);
            }
      for (Measure* m = first(); m; m = m->next()) {
            m->layout2(this);
            }

      //---------------------------------------------------
      //    remove remaining pages and systems
      //---------------------------------------------------

      for (iPage i = ip; i != _pages->end(); ++i)
            delete *i;
      _pages->erase(ip, _pages->end());

      for (iSystem i = is; i != _systems->end(); ++i)
            delete *i;
      _systems->erase(is, _systems->end());

      //---------------------------------------------------
      //    rebuild bspTree
      //---------------------------------------------------

      QRectF r;
      el.clear();
      for(Measure* m = first(); m; m = m->next())
            m->collectElements(el);
      for (iPage ip = _pages->begin(); ip != _pages->end(); ++ip) {
            Page* page = *ip;
            r |= page->abbox();
            page->collectElements(el);
            }
      int depth = intmaxlog(el.size());
      bspTree.initialize(r, depth);
      for (int i = 0; i < el.size(); ++i) {
            Element* e = el.at(i);
            bspTree.insert(e);
            }
      }

//---------------------------------------------------------
//   processSystemHeader
//    add generated timesig keysig and clef
//---------------------------------------------------------

void ScoreLayout::processSystemHeader(Measure* m)
      {
      int tick = m->tick();
      int nstaves = _score->nstaves();

      for (int i = 0; i < nstaves; ++i) {
            Staff* staff   = _score->staff(i);
            if (!staff->show())
                  continue;

            bool hasKeysig  = false;
            bool hasClef    = false;
            int strack      = i * VOICES;

            // we assume that keysigs and clefs are only in the first
            // track of a segment

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  // search only up to the first ChordRest
                  if (seg->subtype() == Segment::SegChordRest)
                        break;
                  Element* el = seg->element(strack);
                  if (!el)
                        continue;
                  switch (el->type()) {
                        case KEYSIG:
                              {
                              hasKeysig = true;
                              KeySig* ks = (KeySig*)el;
                              // no natural accidentals
                              ks->setSubtype(ks->subtype() & 0xff);
                              }
                              break;
                        case CLEF:
                              hasClef = true;
                              ((Clef*)el)->setSmall(false);
                              break;
                        default:
                              break;
                        }
                  }
            if (!hasKeysig) {
                  int idx = staff->keymap()->key(tick);
                  if (idx) {
                        KeySig* ks = new KeySig(_score);
                        ks->setStaff(staff);
                        ks->setTick(tick);
                        ks->setGenerated(true);
                        ks->setSubtype(idx & 0xff);
                        Segment* seg = m->getSegment(ks);
                        seg->add(ks);
                        }
                  }
            if (!hasClef) {
                  int idx = staff->clef()->clef(tick);
                  Clef* cs = new Clef(_score, idx);
                  cs->setStaff(staff);
                  cs->setTick(tick);
                  cs->setGenerated(true);
                  Segment* s = m->getSegment(cs);
                  s->add(cs);
                  }
            }
      }

//---------------------------------------------------------
//   layoutPage
//    return true, if next page must be relayouted
//---------------------------------------------------------

bool ScoreLayout::layoutPage(Page* page, Measure*& im, iSystem& is)
      {
      page->layout(this);

      // usable width of page:
      qreal w  = page->loWidth() - page->lm() - page->rm();
      qreal x  = page->lm();
      qreal ey = page->loHeight() - page->bm() - point(score()->style()->staffLowerBorder);

      page->systems()->clear();
      page->pel().clear();
      qreal y = page->addMeasure(this, im, page->tm());

      int systemNo = 0;
      while (im) {
            // get next system:
            System* system;
            if (is == _systems->end()) {
                  system = new System(_score);
                  _systems->push_back(system);
                  is = _systems->end();
                  }
            else {
                  system = *is++;
                  system->clear();   // remove measures from system
                  }

            layoutSystem(im, system, x, y, w);
            system->setParent(page);

            qreal h = system->bbox().height() + point(score()->style()->systemDistance);
            if (y + h >= ey) {  // system does not fit on page
                  // rollback
                  im = system->measures().front();
                  --is;
                  break;
                  }
            page->appendSystem(system);

            //  move system vertically to final position:

            double systemDistance;
            if (systemNo == 1)
                  systemDistance = point(score()->style()->staffUpperBorder);
            else
                  systemDistance = point(score()->style()->systemDistance);
            system->move(0.0, systemDistance);
            y += h;
            if (system->pageBreak())
                  break;
            }

      //-----------------------------------------------------------------------
      // if remaining y space on page is greater (pageHeight*pageFillLimit)
      // then insert space between staffs to fill page
      //-----------------------------------------------------------------------

      double restHeight = ey - y;
      double ph = page->height()
            - point(score()->style()->staffLowerBorder + score()->style()->staffUpperBorder);

      if (restHeight > (ph * score()->style()->pageFillLimit))
            return true;

      QList<System*>* sl   = page->systems();
      int systems      = sl->size();
      double extraDist = restHeight / (systems-1);
      y                = 0;
      for (iSystem i = sl->begin(); i != sl->end(); ++i) {
            (*i)->move(0, y);
            y += extraDist;
            }
      return true;
      }

//---------------------------------------------------------
//   layoutSystem
//---------------------------------------------------------

System* ScoreLayout::layoutSystem(Measure*& im, System* system, qreal x, qreal y, qreal w)
      {
      int nstaves = _score->nstaves();
      for (int i = system->staves()->size(); i < nstaves; ++i)
            system->insertStaff(_score->staff(i), i);

      double systemOffset = system->layout(this, QPointF(x, y), w);

      //-------------------------------------------------------
      //    Round I
      //    find out how many measures fit in system
      //-------------------------------------------------------

      int nm              = 0;            // number of collected measures
      double systemWidth  = w - systemOffset;
      double minWidth     = 0;
      double uStretch     = 0.0;

      bool pageBreak = false;

      for (Measure* m = im; m; m = m->next()) {
            pageBreak = m->pageBreak();

            m->setSystem(system);   // needed by m->layout()
            if (m == im) {
                  //
                  // special handling for first measure in a system:
                  // add generated clef and key signature
                  //
                  processSystemHeader(m);
                  }
            //
            // remove generated elements
            //    assume: generated elements only live in voice 0
            //    TODO: check if removed elements can be deleted
            //
            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  for (int staffIdx = 0;  staffIdx < nstaves; ++staffIdx) {
                        int track = staffIdx * VOICES;
                        Element* el = seg->element(track);
                        if (el && el->generated()) {
                              if ((m != im) || (seg->subtype() == Segment::SegTimeSigAnnounce))
                                    seg->setElement(track, 0);
                              }
                        }
                  }

            if (m != im) {
                  //
                  // if this is not the first measure in a system
                  // switch all clefs to small size
                  //
                  int nstaves = _score->nstaves();
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->subtype() != Segment::SegClef)
                              continue;
                        for (int i = 0; i < nstaves; ++i) {
                              int strack = i * VOICES;
                              Element* el = seg->element(strack);
                              if (el && el->type() == CLEF) {
                                    ((Clef*)el)->setSmall(true);
                                    }
                              }
                        break;
                        }
                  }

            m->layoutX(this, 1.0);

            double ww      = m->layoutWidth().stretchable;
            double stretch = m->userStretch() * score()->style()->measureSpacing;

            ww *= stretch;
            if (ww < point(score()->style()->minMeasureWidth))
                  ww = point(score()->style()->minMeasureWidth);

            if (minWidth + ww > systemWidth) {
                  // minimum is one measure
                  if (nm == 0) {
                        minWidth = systemWidth;
                        uStretch = stretch;
                        nm       = 1;
                        printf("warning: system too small (%f+%f) > %f\n",
                           minWidth, ww, systemWidth);
                        // bad things are happening here
                        }
                  break;
                  }
            ++nm;
            minWidth += ww;
            uStretch += stretch;
            if (pageBreak || m->lineBreak())
                  break;
            }
      system->setPageBreak(pageBreak);


      //-------------------------------------------------------
      //    Round II
      //    stretch measures
      //    "nm" measures fit on this line of score
      //    "minWidth"   is the minimum width they use
      //    uStretch is the accumulated userStretch
      //-------------------------------------------------------

      Measure* itt = im;
      for (int i = 0; i < nm; ++i, itt = itt->next())
            system->measures().push_back(itt);
      im = itt;

      //
      //    add cautionary time signatures if needed
      //
      Measure* lm = system->measures().back();
      SigEvent sig1 = _score->sigmap->timesig(lm->tick() + lm->tickLen() - 1);
      SigEvent sig2 = _score->sigmap->timesig(lm->tick() + lm->tickLen());
      if (!(sig1 == sig2)) {
            int tick    = lm->tick() + lm->tickLen();
            Segment* s  = lm->getSegment(Segment::SegTimeSigAnnounce, tick);
            int nstaves = score()->nstaves();
            for (int staff = 0; staff < nstaves; ++staff) {
                  if (s->element(staff * VOICES) == 0) {
                        TimeSig* ts = new TimeSig(score(), sig2.denominator, sig2.nominator);
                        ts->setStaff(score()->staff(staff));
                        ts->setGenerated(true);
                        s->add(ts);
                        }
                  }
            }

      minWidth           = 0.0;
      double totalWeight = 0.0;

      foreach(Measure* m, system->measures()) {
            minWidth    += m->layoutWidth().stretchable;
            totalWeight += m->tickLen() * m->userStretch();
            }

      double rest = layoutDebug ? 0.0 : systemWidth - minWidth;
      QPointF pos(systemOffset, 0);

      foreach(Measure* m, system->measures()) {
            m->setPos(pos);
            double weight = m->tickLen() * m->userStretch();
            double ww     = m->layoutWidth().stretchable + rest * weight / totalWeight;
            m->layout(this, ww);
            pos.rx() += ww;
            }

      system->layout2(this);      // layout staves vertical
      return system;
      }

//---------------------------------------------------------
//   addPage
//---------------------------------------------------------

Page* ScoreLayout::addPage()
      {
      Page* page = new Page(this);
      page->setNo(_pages->size());
      _pages->push_back(page);
      return page;
      }

//---------------------------------------------------------
//   setPageFormat
//---------------------------------------------------------

void ScoreLayout::setPageFormat(const PageFormat& pf)
      {
      *_pageFormat = pf;
      }

//---------------------------------------------------------
//   push_back
//---------------------------------------------------------

void ElemList::push_back(Element* e)
      {
      ++_size;
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

void ElemList::push_front(Element* e)
      {
      ++_size;
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
//   insert
//    insert e before el
//---------------------------------------------------------

void ElemList::insert(Element* e, Element* el)
      {
      if (el == 0) {
            push_back(e);
            return;
            }
      if (el == _first) {
            push_front(e);
            return;
            }
      ++_size;
      e->setNext(el);
      e->setPrev(el->prev());
      el->prev()->setNext(e);
      el->setPrev(e);
      }

//---------------------------------------------------------
//   erase
//---------------------------------------------------------

void ElemList::erase(Element* el)
      {
      --_size;
      if (el->prev())
            el->prev()->setNext(el->next());
      else {
            _first = el->next();
            }
      if (el->next())
            el->next()->setPrev(el->prev());
      else
            _last = el->prev();
      }

//---------------------------------------------------------
//   setInstrumentNames
//---------------------------------------------------------

void ScoreLayout::setInstrumentNames()
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
            const NoteList* nl = ((Chord*)element)->noteList();
            for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                  if (in->second->pitch() == pitch)
                        return in->second;
                  }
            }
      return 0;
      }

//---------------------------------------------------------
//   connectTies
//---------------------------------------------------------

/**
 Rebuild tie connections.
*/

void ScoreLayout::connectTies()
      {
      int tracks = _score->nstaves() * VOICES;
      for (Measure* m = first(); m; m = m->next()) {
            for (Segment* s = m->first(); s; s = s->next()) {
                  for (int i = 0; i < tracks; ++i) {
                        Element* el = s->element(i);
                        if (el == 0 || el->type() != CHORD)
                              continue;
                        const NoteList* nl = ((Chord*)el)->noteList();
                        for (ciNote in = nl->begin(); in != nl->end(); ++in) {
                              Tie* tie = in->second->tieFor();
                              if (!tie)
                                    continue;
                              Note* nnote = searchTieNote(in->second, s, i);
                              if (nnote == 0)
                                    printf("next note at %d(measure %d) voice %d for tie not found\n",
                                       in->second->chord()->tick(),
                                       m->no(),
                                       in->second->chord()->voice()
                                       );
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
//   erase
//---------------------------------------------------------

void ScoreLayout::erase(Measure* im)
      {
      _measures.erase(im);
      }

//---------------------------------------------------------
//   insert
//---------------------------------------------------------

void ScoreLayout::insert(Measure* im, Measure* m)
      {
      _measures.insert(im, m);
      }

//---------------------------------------------------------
//   reLayout
//---------------------------------------------------------

void ScoreLayout::reLayout(Measure*)
      {
      printf("reLayout\n");
      }

