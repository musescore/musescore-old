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
#include "sig.h"
#include "key.h"
#include "clef.h"
#include "score.h"
#include "globals.h"
#include "segment.h"
#include "text.h"
#include "staff.h"
#include "style.h"
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
#include "part.h"
#include "utils.h"
#include "measure.h"
#include "preferences.h"

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
            if (element->track() != -1) {
                  if (element->staffIdx() < 0 || element->staffIdx() >= nstaves()) {
                        printf("element %s bad staffIdx %d(track:%d) >= staves(%d)\n",
                           element->name(), element->staffIdx(), element->track(), nstaves());
                        continue;
                        }
                  if (!element->staff()->show())
                        continue;
                  }
            element->scanElements(&el, collectElements);
            }

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
//   layout
//    - measures are akkumulated into systems
//    - systems are akkumulated into pages
//   already existent systems and pages are reused
//---------------------------------------------------------

void Score::doLayout()
      {
// printf("doLayout============\n");
      _needLayout = false;

      if (startLayout) {
            startLayout->setDirty();
            if (doReLayout()) {
                  startLayout = 0;
                  return;
                  }
            }
      if (first() == 0) {
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

      //-----------------------------------------------------------------------
      //    pass 0:  layout chords
      //             set line & accidental & mirror for notes depending
      //             on context
      //-----------------------------------------------------------------------

      for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx) {
            for (MeasureBase* mb = first(); mb; mb = mb->next()) {
                  mb->setDirty();
                  if (mb->type() == MEASURE) {
                        Measure* m = static_cast<Measure*>(mb);
#if 1 // CS1
                        if (staffIdx == 0)
                              m->layoutBeams1();  // find hooks / stem direction
#endif
                        m->layout0(staffIdx);
                        }
                  }
            }

      //-----------------------------------------
      //    pass 1:  process pages
      //-----------------------------------------

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
      //   pass 2:  place ties & slurs & hairpins & beams
      //---------------------------------------------------

      for (MeasureBase* mb = first(); mb; mb = mb->next()) {
            if (mb->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(mb);
                  //if (m->multiMeasure() >= 0)
                        m->layout2();
                  }
            }

      foreach(Element* el, _gel)
            el->layout();

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
                              hasKeysig = static_cast<KeySig*>(el);
                              hasKeysig->setSubtype(keyIdx);
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
            bool needKeysig = keyIdx && (isFirstSystem || styleB(ST_genKeysig));
            if (needKeysig && !hasKeysig) {
                  //
                  // create missing key signature
                  //
                  KeySig* ks = new KeySig(this);
                  ks->setTrack(i * VOICES);
                  ks->setTick(tick);
                  ks->setGenerated(true);
                  ks->setSubtype(keyIdx);
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

      double nettoHeight = 0.0;
      while (curMeasure) {
            if (curMeasure->type() == VBOX) {
                  System* system = getNextSystem(false, true);

                  foreach(SysStaff* ss, *system->staves())
                        delete ss;
                  system->staves()->clear();

                  system->setWidth(w);
                  VBox* vbox = (VBox*) curMeasure;
                  vbox->setParent(system);
                  vbox->layout();
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
                  y += bh + point(styleS(ST_frameSystemDistance));
                  nettoHeight += bh;
                  if (y > ey)
                        break;
                  nettoHeight += point(styleS(ST_frameSystemDistance));
                  }
            else {
                  if (firstSystemOnPage) {
                        y += sub;
                        }
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
                        system->setYpos(y);
                        }
                  firstSystem       = false;
                  firstSystemOnPage = false;
                  y += h;
                  nettoHeight += h;
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

      double restHeight = ey - y; //  + systemDistance;

      double ph = page->loHeight() - page->bm() - page->tm() - slb - sub;

      if (restHeight > (ph * (1.0 - styleD(ST_pageFillLimit))))
            return true;

      double systemDistance = point(styleS(ST_systemDistance));
      double extraDist = (rows > 1) ? ((ph - nettoHeight + systemDistance) / (rows - 1.0)) : 0.0;
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
            for (int i = 0; i < (n-1); ++i) {
                  m->setMultiMeasure(-1);
                  m->setSystem(system);
                  m = static_cast<Measure*>(m->next());
                  }
            m->setMultiMeasure(n);  // last measure is presented as multi measure rest
            }
      else
            m->setMultiMeasure(0);
      return m;
      }

//---------------------------------------------------------
//   layoutSystem1
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
//      MeasureBase* firstMeasure = curMeasure;

      for (; curMeasure;) {
            if (curMeasure->type() == MEASURE) {
                  Measure* m = static_cast<Measure*>(curMeasure);
                  if (styleB(ST_createMultiMeasureRests))
                        curMeasure = skipEmptyMeasures(m, system);
                  else
                        m->setMultiMeasure(0);
                  }

            System* oldSystem = curMeasure->system();
            curMeasure->setSystem(system);
            double ww      = 0.0;
            double stretch = 0.0;

            if (curMeasure->type() == HBOX) {
                  ww = point(static_cast<Box*>(curMeasure)->boxWidth());
                  if (!isFirstMeasure)
                        continueFlag = true;    //try to put another system on current row
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
                        if (seg->subtype() == Segment::SegEndBarLine)
                              continue;
                        for (int staffIdx = 0;  staffIdx < nstaves; ++staffIdx) {
                              int track = staffIdx * VOICES;
                              Element* el = seg->element(track);
                              if (el == 0)
                                    continue;
                              if (el->generated()) {
                                    if (!isFirstMeasure || (seg->subtype() == Segment::SegTimeSigAnnounce))
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
//TODO                              double staffMag = staff(staffIdx)->mag();
//                              el->setMag(staffMag);
                              }
                        }

                  m->createEndBarLines();
                  m->layoutBeams1();  // find hooks

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
            if (continueFlag || curMeasure->pageBreak() || curMeasure->lineBreak() || (curMeasure->next() && curMeasure->next()->type() == VBOX)) {
                  system->setPageBreak(curMeasure->pageBreak());
                  curMeasure = curMeasure->next();
                  break;
                  }
            else
                  curMeasure = curMeasure->next();
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
            //    add cautionary time signatures if needed
            //

            if (system->measures().isEmpty()) {
                  printf("system %p is empty\n", system);
                  abort();
                  }
            MeasureBase* lm = system->measures().back();
            int tick        = lm->tick() + lm->tickLen();
            SigEvent sig1   = sigmap->timesig(tick - 1);
            SigEvent sig2   = sigmap->timesig(tick);

            if (styleB(ST_genCourtesyTimesig) && !sig1.nominalEqual(sig2)) {
                  while (lm && lm->type() != MEASURE)
                        lm = lm->prev();
                  if (lm) {
                        Measure* m = (Measure*)lm;
                        Segment* s  = m->getSegment(Segment::SegTimeSigAnnounce, tick);
                        int nstaves = Score::nstaves();
                        for (int track = 0; track < nstaves * VOICES; track += VOICES) {
                              if (s->element(track) == 0) {
                                    TimeSig* ts = new TimeSig(this, sig2.denominator, sig2.nominator);
                                    ts->setTrack(track);
                                    ts->setGenerated(true);
                                    ts->setMag(ts->staff()->mag());
                                    s->add(ts);
                                    needRelayout = true;
                                    }
                              }
                        }
                  }

            const QList<MeasureBase*>& ml = system->measures();
            int n                         = ml.size();

            //
            //    compute repeat bar lines
            //
            bool firstMeasure = true;
            for (int i = 0; i < n; ++i) {
                  MeasureBase* mb = ml[i];
                  if (mb->type() != MEASURE)
                        continue;
                  Measure* m = (Measure*)mb;

                  // first measure repeat?
                  bool fmr = firstMeasure && (m->repeatFlags() & RepeatStart);

                  if (i == (n-1)) {       // last measure in system?
                        if (m->repeatFlags() & RepeatEnd)
                              m->setEndBarLineType(END_REPEAT, true);
                        else if (m->endBarLineGenerated())
                              m->setEndBarLineType(NORMAL_BAR, true);
                        needRelayout |= m->setStartRepeatBarLine(fmr);
                        }
                  else {
                        MeasureBase* mb = m->next();
                        while (mb && mb->type() != MEASURE && (mb != ml[n-1]))
                              mb = mb->next();
                        Measure* nm = 0;
                        if (mb->type() == MEASURE)
                              nm = (Measure*)mb;

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
//   add
//---------------------------------------------------------

void Score::add(Element* el)
      {
      if (el->type() == MEASURE || el->type() == HBOX || el->type() == VBOX) {
            measures()->add((MeasureBase*)el);
            }
      else {
            if (el->check())
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
      else {
            if (!_gel.removeOne(el))
                  printf("Score::remove(): element not found\n");
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
      if (startLayout->type() == MEASURE) {
            for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                  static_cast<Measure*>(startLayout)->layout0(staffIdx);
            }
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
            if (m->type() == HBOX) {
                  ww = point(static_cast<Box*>(m)->boxWidth());
                  }
            else if (m->type() == MEASURE) {
                  Measure* measure = static_cast<Measure*>(m);
                  for (int staffIdx = 0; staffIdx < nstaves(); ++staffIdx)
                        measure->layout0(staffIdx);
                  measure->layoutBeams1();
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

//---------------------------------------------------------
//   clear
//---------------------------------------------------------

#if 0
void Score::clear()
      {
      _pages.clear();
      _systems.clear();
      }
#endif
