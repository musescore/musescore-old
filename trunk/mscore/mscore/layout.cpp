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
#include "barline.h"
#include "repeat.h"
#include "box.h"
#include "system.h"

// #define OPTIMIZE_LAYOUT

//---------------------------------------------------------
//   intmaxlog
//---------------------------------------------------------

static inline int intmaxlog(int n)
      {
      return (n > 0 ? qMax(int(::ceil(::log(double(n))/::log(double(2)))), 5) : 0);
      }

//---------------------------------------------------------
//   first
//---------------------------------------------------------

MeasureBase* ScoreLayout::first() const
      {
      return _score->measures()->first();
      }

//---------------------------------------------------------
//   last
//---------------------------------------------------------

MeasureBase* ScoreLayout::last()  const
      {
      return _score->measures()->last();
      }

//---------------------------------------------------------
//   ScoreLayout
//---------------------------------------------------------

ScoreLayout::ScoreLayout(Score* score)
   : Element(score)
      {
      _spatium     = ::_spatium;
      _pageFormat  = new PageFormat;
      _paintDevice = 0;

      //DEBUG:
      _spatium = ::_spatium;
      _systems.clear();
      _pages.clear();
      _needLayout = false;
      _pageFormat = new PageFormat;
      _paintDevice = _score->canvas();
      startLayout = 0;
      }

ScoreLayout::~ScoreLayout()
      {
      if (_pageFormat)
            delete _pageFormat;
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

#ifdef OPTIMIZE_LAYOUT
      if (startLayout) {
            doReLayout();
            startLayout = 0;
            return;
            }
#endif

      if (first() == 0) {
            // score is empty
            foreach(Page* page, _pages)
                  delete page;
            _pages.clear();

            Page* page = addPage();
            page->layout(this);
            page->setNo(0);
            page->setPos(0.0, 0.0);

            //---------------------------------------------------
            //    clear bspTree
            //---------------------------------------------------

            QRectF r = page->abbox();
            bspTree.initialize(r, 0);
            return;
            }

      for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
            for (MeasureBase* mb = first(); mb; mb = mb->next()) {
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;
                  m->layout0(staffIdx);
                  }
            }

      //-----------------------------------------
      //    pass I:  process pages
      //-----------------------------------------

      curMeasure = first();
      curSystem  = 0;
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
      //   pass II:  place ties & slurs & hairpins & beams
      //---------------------------------------------------

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            m->layout2(this);
            }

      foreach(Element* el, *score()->gel())
            el->layout(this);

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

      //---------------------------------------------------
      //    rebuild bspTree
      //---------------------------------------------------

      QRectF r;
      QList<const Element*> el;
      for (const MeasureBase* m = first(); m; m = m->next())
            m->collectElements(el);
      foreach(const Page* page, _pages) {
            r |= page->abbox();
            page->collectElements(el);
            }
      foreach (const Element* element, *score()->gel()) {
            if (element->track() != -1) {
                  if (!element->staff()->show())
                        continue;
                  }
            element->collectElements(el);
            }

      int depth = intmaxlog(el.size());
      bspTree.initialize(r, depth);
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            bspTree.insert(e);
            }
      }

//---------------------------------------------------------
//   processSystemHeader
//    add generated timesig keysig and clef
//---------------------------------------------------------

void ScoreLayout::processSystemHeader(Measure* m, bool isFirstSystem)
      {
      int tick = m->tick();
      int nstaves = _score->nstaves();

      for (int i = 0; i < nstaves; ++i) {
            Staff* staff   = _score->staff(i);
            if (!staff->show())
                  continue;

            KeySig* hasKeysig = 0;
            Clef*   hasClef   = 0;
            int strack        = i * VOICES;

            // we assume that keysigs and clefs are only in the first
            // track of a segment

            int keyIdx = staff->keymap()->key(tick) & 0xff;

            for (Segment* seg = m->first(); seg; seg = seg->next()) {
                  // search only up to the first ChordRest
                  if (seg->subtype() == Segment::SegChordRest)
                        break;
                  Element* el = seg->element(strack);
                  if (!el)
                        continue;
                  switch (el->type()) {
                        case KEYSIG:
                              hasKeysig = (KeySig*)el;
                              hasKeysig->setSubtype(keyIdx);
                              hasKeysig->setMag(staff->mag());
                              break;
                        case CLEF:
                              hasClef = (Clef*)el;
                              hasClef->setMag(staff->mag());
                              break;
                        default:
                              break;
                        }
                  }
            bool needKeysig = keyIdx && (isFirstSystem || _score->style()->genKeysig);
            if (needKeysig && !hasKeysig) {
                  //
                  // create missing key signature
                  //
                  KeySig* ks = new KeySig(_score);
                  ks->setTrack(i * VOICES);
                  ks->setTick(tick);
                  ks->setGenerated(true);
                  ks->setSubtype(keyIdx);
                  ks->setMag(staff->mag());
                  Segment* seg = m->getSegment(ks);
                  seg->add(ks);
                  }
            else if (!needKeysig && hasKeysig) {
                  int track = hasKeysig->track();
                  Segment* seg = hasKeysig->segment();
                  seg->setElement(track, 0);    // TODO: delete element
                  }
            bool needClef = isFirstSystem || _score->style()->genClef;
            if (needClef && !hasClef) {
                  //
                  // create missing clef
                  //
                  int idx = staff->clef()->clef(tick);
                  Clef* cs = new Clef(_score, idx);
                  cs->setTrack(i * VOICES);
                  cs->setTick(tick);
                  cs->setGenerated(true);
                  cs->setMag(staff->mag());
                  Segment* s = m->getSegment(cs);
                  s->add(cs);
                  }
            else if (!needClef && hasClef) {
                  int track = hasClef->track();
                  Segment* seg = hasClef->segment();
                  seg->setElement(track, 0);    // TODO: delete element
                  }
            }
      }

//---------------------------------------------------------
//   getNextSystem
//---------------------------------------------------------

System* ScoreLayout::getNextSystem(bool isFirstSystem, bool isVbox)
      {
      System* system;
      if (curSystem >= _systems.size()) {
            system = new System(_score);
            _systems.append(system);
            }
      else {
            system = _systems[curSystem];
            system->clear();   // remove measures from system
            }
      system->setFirstSystem(isFirstSystem);
      system->setVbox(isVbox);
      if (!isVbox) {
            int nstaves = _score->nstaves();
            for (int i = system->staves()->size(); i < nstaves; ++i)
                  system->insertStaff(i);
            for (int i = nstaves; i < system->staves()->size(); ++i)
                  system->removeStaff(nstaves);
            }
      return system;
      }

//---------------------------------------------------------
//   getCurPage
//---------------------------------------------------------

void ScoreLayout::getCurPage()
      {
      Page* page = curPage >= _pages.size() ? addPage() : _pages[curPage];
      page->setNo(curPage);
      page->layout(this);
      double x = (curPage == 0) ? 0.0 : _pages[curPage - 1]->pos().x()
         + page->width() + ((curPage & 1) ? 1.0 : 50.0);
      page->setPos(x, 0.0);
      }

//---------------------------------------------------------
//   layoutPage
//    return true, if next page must be relayouted
//---------------------------------------------------------

bool ScoreLayout::layoutPage()
      {
      Page* page = _pages[curPage];

      // usable width of page:
      qreal w  = page->loWidth() - page->lm() - page->rm();
      qreal x  = page->lm();
      qreal ey = page->loHeight() - page->bm() - point(score()->style()->staffLowerBorder);

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
                  VBox* vbox = (VBox*) curMeasure;
                  vbox->setParent(system);
                  vbox->layout(this);
                  double bh = vbox->height();

                  // put at least one system on page
                  if (((y + bh) > ey) && !firstSystemOnPage)
                        break;

                  system->setPos(QPointF(x, y));
                  system->setHeight(vbox->height());

                  system->measures().push_back(vbox);
                  page->appendSystem(system);

                  curMeasure = curMeasure->next();
                  ++curSystem;
                  y += bh + score()->style()->boxSystemDistance.point();
                  if (y > ey)
                        break;
                  }
            else {
                  if (firstSystemOnPage) {
                        y += point(score()->style()->staffUpperBorder);
                        }
                  int cs            = curSystem;
                  MeasureBase* cm   = curMeasure;
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
                        system->setYpos(y);
                        }
                  firstSystem = false;
                  firstSystemOnPage = false;
                  y += h;
                  if (sl.back()->pageBreak())
                        break;
                  }
            ++rows;
            }

      //-----------------------------------------------------------------------
      // if remaining y space on page is greater (pageHeight*pageFillLimit)
      // then insert space between staffs to fill page
      //-----------------------------------------------------------------------

      double restHeight = ey - y; //  + systemDistance;
      double ph = page->height()
            - point(score()->style()->staffLowerBorder + score()->style()->staffUpperBorder);

      if (restHeight > (ph * (1.0 - score()->style()->pageFillLimit)))
            return true;
return true;

      double dist = restHeight / (rows - 1);
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
                  y += dist;                    // next system row
            }
      return true;
      }

//---------------------------------------------------------
//   layoutSystem1
//---------------------------------------------------------

bool ScoreLayout::layoutSystem1(double& minWidth, double w, bool isFirstSystem)
      {
      System* system = getNextSystem(isFirstSystem, false);

      system->layout(this);
      minWidth            = system->leftMargin();
      double systemWidth  = w;

      double uStretch     = 1.0;
      bool continueFlag   = false;

      int nstaves = _score->nstaves();
      bool isFirstMeasure = true;

      for (; curMeasure; curMeasure = curMeasure->next()) {
            curMeasure->setSystem(system);
            double ww      = 0.0;
            double stretch = 0.0;

            if (curMeasure->type() == HBOX) {
                  ww = ((Box*)curMeasure)->boxWidth().point();
                  if (!isFirstMeasure)
                        continueFlag = true;    //try to put another system on current row
                  }
            else if (curMeasure->type() == MEASURE) {
                  Measure* m = (Measure*)curMeasure;
                  if (isFirstMeasure)
                        processSystemHeader((Measure*)curMeasure, isFirstSystem);

                  //
                  // remove generated elements
                  //    assume: generated elements are only living in voice 0
                  //    TODO: check if removed elements can be deleted
                  //
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->subtype() == Segment::SegEndBarLine)
                              continue;
                        for (int staffIdx = 0;  staffIdx < nstaves; ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Element* el = seg->element(track);
                              if (el) {
                                    if (el->generated()) {
                                          if (!isFirstMeasure || (seg->subtype() == Segment::SegTimeSigAnnounce))
                                                seg->setElement(track, 0);
                                          }
                                    if (!isFirstMeasure && el->type() == CLEF)
                                          el->setMag(el->staff()->mag() * score()->style()->smallClefMag);
                                    }
                              }
                        }

                  m->createEndBarLines();
                  m->layoutBeams1(this);  // find hooks

                  m->layoutX(this, 1.0);
                  ww      = m->layoutWidth().stretchable;
                  stretch = m->userStretch() * score()->style()->measureSpacing;

                  ww *= stretch;
                  if (ww < point(score()->style()->minMeasureWidth))
                        ww = point(score()->style()->minMeasureWidth);
                  isFirstMeasure = false;
                  }

            // collect at least one measure
            if ((minWidth + ww > systemWidth) && !system->measures().isEmpty())
                  break;

            minWidth += ww;
            uStretch += stretch;
            system->measures().append(curMeasure);
            if (continueFlag || curMeasure->pageBreak() || curMeasure->lineBreak() || (curMeasure->next() && curMeasure->next()->type() == VBOX)) {
                  system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = curMeasure->next();
                  break;
                  }
            }
      return continueFlag;
      }

//---------------------------------------------------------
//   layoutSystemRow
//    x, y  position of row on page
//---------------------------------------------------------

QList<System*> ScoreLayout::layoutSystemRow(qreal x, qreal y, qreal rowWidth,
   bool isFirstSystem, double* h)
      {
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
      if (curMeasure == 0 && ((minWidth / rowWidth) <= score()->style()->lastSystemFillLimit))
            rowWidth = minWidth;

      //-------------------------------------------------------
      //    Round II
      //    stretch measures
      //    "nm" measures fit on this line of score
      //    "minWidth"   is the minimum width they use
      //    uStretch is the accumulated userStretch
      //-------------------------------------------------------

      bool needRelayout = false;


      foreach(System* system, sl) {
            //
            //    add cautionary time signatures if needed
            //

            MeasureBase* lm = system->measures().back();
            int tick        = lm->tick() + lm->tickLen();
            SigEvent sig1   = _score->sigmap->timesig(tick - 1);
            SigEvent sig2   = _score->sigmap->timesig(tick);

            if (!(sig1 == sig2)) {
                  while (lm && lm->type() != MEASURE)
                        lm = lm->prev();
                  if (lm) {
                        Measure* m = (Measure*)lm;
                        Segment* s  = m->getSegment(Segment::SegTimeSigAnnounce, tick);
                        int nstaves = score()->nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              if (s->element(track) == 0) {
                                    TimeSig* ts = new TimeSig(score(), sig2.denominator, sig2.nominator);
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    s->add(ts);
                                    needRelayout = true;
                                    }
                              }
                        }
                  }

            //
            //    compute repeat bar lines
            //
            const QList<MeasureBase*>& ml = system->measures();
            int n                     = ml.size();
            for (int i = 0; i < n; ++i) {
                  MeasureBase* mb = ml[i];
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  if (i == (n-1)) {       // last measure in system?
                        if (m->repeatFlags() & RepeatEnd)
                              m->setEndBarLineType(END_REPEAT, true);
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(NORMAL_BAR, true);
                        }
                  else {
                        MeasureBase* mb = m->next();
                        while (mb && mb->type() != MEASURE && (mb != ml[n-1]))
                              mb = mb->next();
                        Measure* nm = 0;
                        if (mb->type() == MEASURE)
                              nm = (Measure*)mb;

                        needRelayout |= m->setStartRepeatBarLine((i == 0) && (m->repeatFlags() & RepeatStart));
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
                  }
            }

      minWidth           = 0.0;
      double totalWeight = 0.0;

      foreach(System* system, sl) {
            foreach (MeasureBase* mb, system->measures()) {
                  if (mb->type() == HBOX)
                        minWidth += ((Box*)mb)->boxWidth().point();
                  else {
                        Measure* m = (Measure*)mb;
                        if (needRelayout)
                              m->layoutX(this, 1.0);
                        minWidth    += m->layoutWidth().stretchable;
                        totalWeight += m->tickLen() * m->userStretch();
                        }
                  }
            minWidth += system->leftMargin();
            }

      double rest = layoutDebug ? 0.0 : rowWidth - minWidth;
      double xx = 0.0;
      foreach(System* system, sl) {
            QPointF pos(system->leftMargin(), 0);

            foreach(MeasureBase* mb, system->measures()) {
                  mb->setPos(pos);
                  double ww = 0.0;
                  if (mb->type() == MEASURE) {
                        Measure* m    = (Measure*)mb;
                        double weight = m->tickLen() * m->userStretch();
                        ww            = m->layoutWidth().stretchable + rest * weight / totalWeight;
                        m->layout(this, ww);
                        }
                  else if (mb->type() == HBOX) {
                        ww = ((Box*)mb)->boxWidth().point();
                        mb->layout(this);
                        }
                  pos.rx() += ww;
                  }
            system->setPos(xx + x, y);
            double w = pos.x();
            system->setWidth(w);
            system->layout2(this);
            foreach(MeasureBase* mb, system->measures()) {
                  if (mb->type() == HBOX)
                        mb->setHeight(system->height());
                  }
            xx += w;
            double hh = system->height() + system->staves()->back()->distance();
            if (hh > *h)
                  *h = hh;
            }
      return sl;
      }

//---------------------------------------------------------
//   addPage
//---------------------------------------------------------

Page* ScoreLayout::addPage()
      {
      Page* page = new Page(this);
      page->setNo(_pages.size());
      _pages.push_back(page);
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
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
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
//   searchHiddenNotes
//---------------------------------------------------------

void ScoreLayout::searchHiddenNotes()
      {
      int staves = _score->nstaves();
      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            for (Segment* s = m->first(); s; s = s->next()) {
                  if (s->subtype() != Segment::SegChordRest)
                        continue;
                  for (int staffIdx = 0; staffIdx < staves; ++staffIdx) {
                        for (int voice = 0; voice < VOICES; ++voice) {
                              Element* el = s->element(staffIdx * VOICES + voice);
                              if (el == 0 || el->type() != CHORD)
                                    continue;
                              NoteList* nl = ((Chord*)el)->noteList();
                              for (iNote in = nl->begin(); in != nl->end(); ++in) {
                                    Note* note = in->second;
                                    for (int v2 = voice; v2 < VOICES; ++v2) {
                                          Element* e = s->element(staffIdx * VOICES + v2);
                                          if (e == 0 || e->type() != CHORD)
                                                continue;
                                          NoteList* nl2 = ((Chord*)e)->noteList();
                                          for (iNote in2 = nl2->begin(); in2 != nl2->end(); ++in2) {
                                                Note* note2 = in2->second;
                                                if (note2->hidden())
                                                      continue;
                                                if (note2->pitch() == note->pitch()) {
                                                      if (e->tickLen() > el->tickLen())
                                                            note->setHidden(true);
                                                      }
                                                }
                                          }
                                    }
                              }
                        }
                  }
            }
      }

//---------------------------------------------------------
//   add
//---------------------------------------------------------

void ScoreLayout::add(Element* el)
      {
      if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX) {
            _score->measures()->add((MeasureBase*)el);
            }
      else {
            el->setParent(this);
            score()->gel()->append(el);
            }
      }

//---------------------------------------------------------
//   remove
//---------------------------------------------------------

void ScoreLayout::remove(Element* el)
      {
      if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX) {
            _score->measures()->remove((MeasureBase*)el);
            }
      else {
            int idx = score()->gel()->indexOf(el);
            if (idx == -1)
                  printf("ScoreLayout::remove(): element not found\n");
            else
                  score()->gel()->removeAt(idx);
            }
      }

//---------------------------------------------------------
//   change
//---------------------------------------------------------

void ScoreLayout::change(Element* o, Element* n)
      {
      if (n->type() == MEASURE || n->type() == HBOX || n->type() == VBOX)
            _score->measures()->change((MeasureBase*)o, (MeasureBase*)n);
      else {
            remove(o);
            add(n);
            }
      }

//---------------------------------------------------------
//   reLayout
//---------------------------------------------------------

void ScoreLayout::reLayout(Measure* m)
      {
      _needLayout = true;
      startLayout = m;
      }

//---------------------------------------------------------
//   doReLayout
//---------------------------------------------------------

void ScoreLayout::doReLayout()
      {
      if (startLayout->type() == MEASURE) {
            for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx)
                  ((Measure*)startLayout)->layout0(staffIdx);
            }

      // collect row of systems
      System* curSystem   = startLayout->system();
      Page* page  = curSystem->page();
      QList<System*>* psl = page->systems();

      int i = 0;
      for(; i < psl->size(); ++i) {
            if (psl->at(i) == curSystem) {
                  printf("curSystem %d\n", i);
                  break;
                  }
            }
      QList<System*> sl;
      double y = curSystem->y();
      while (i) {
            if (psl->at(i-1)->y() != y)
                  break;
            --i;
            }
      for (; i < psl->size(); ++i) {
            if (psl->at(i)->y() != y)
                  break;
            sl.append(psl->at(i));
            }

#if 0
      //-----------------------------------------
      //    pass I:  process pages
      //-----------------------------------------

      curSystem   = startLayout->system();
      curMeasure  = curSystem->measures()->front();
      firstSystem = false;

      //---------------------------------------------------
      //   pass II:  place ties & slurs & hairpins & beams
      //---------------------------------------------------

      foreach(MeasureBase* mb, curSystem->measures()) {
            if (mb->type() != MEASURE)
                  continue;
            Measure* m = (Measure*)mb;
            m->layout2(this);
            }

      foreach(Element* el, *score()->gel())
            el->layout(this);

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
#endif

      //---------------------------------------------------
      //    rebuild bspTree
      //---------------------------------------------------

      QRectF r;
      QList<const Element*> el;
      for (const MeasureBase* m = first(); m; m = m->next())
            m->collectElements(el);
      foreach(const Page* page, _pages) {
            r |= page->abbox();
            page->collectElements(el);
            }
      foreach (const Element* element, *score()->gel()) {
            if (element->track() != -1) {
                  if (!element->staff()->show())
                        continue;
                  }
            element->collectElements(el);
            }

      int depth = intmaxlog(el.size());
      bspTree.initialize(r, depth);
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            bspTree.insert(e);
            }
      }

