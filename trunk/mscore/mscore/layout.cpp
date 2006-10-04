//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: layout.cpp,v 1.61 2006/09/08 19:37:08 lvinken Exp $
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

#include "page.h"
#include "sig.h"
#include "key.h"
#include "clef.h"
#include "score.h"
#include "globals.h"
#include "segment.h"
#include "textelement.h"
#include "staff.h"
#include "style.h"
#include "layout.h"
#include "timesig.h"

//---------------------------------------------------------
//   ScoreLayout
//---------------------------------------------------------

ScoreLayout::ScoreLayout()
      {
      _spatium    = ::_spatium;
      _pageFormat = 0;
      _systems    = new SystemList;
      _pages      = new PageList;
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
      if (_pageFormat)
            delete _pageFormat;
      _pageFormat = new PageFormat;
      }

//---------------------------------------------------------
//   searchNote
//    search for note or rest before or at tick position tick
//    in staff
//---------------------------------------------------------

Element* Score::searchNote(int tick, int staff) const
      {
      for (const Measure* measure = _layout->first(); measure; measure = measure->next()) {
            Element* ipe = 0;
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                  // LVIFIX: check: shouldn't segment->element()'s parameter be a track ?
                  Element* ie  = segment->element(staff);
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
      return 0;
      }

//---------------------------------------------------------
//   clefOffset
//---------------------------------------------------------

int Score::clefOffset(int tick, int staffIdx) const
      {
      return clefTable[staff(staffIdx)->clef()->clef(tick)].yOffset;
      }

//---------------------------------------------------------
//   layout
//    place all elements in staves into page structure
//---------------------------------------------------------

void ScoreLayout::doLayout()
      {
      ::_spatium = _spatium;
      _needLayout = false;

      Measure* im = (Measure*)(_measures.first());
      iPage    ip = _pages->begin();
      iSystem  is = _systems->begin();

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
                  page = *ip;
                  ++ip;
                  }
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
            m->layout2();
            }

      //---------------------------------------------------
      //    remove uneccesary pages
      //---------------------------------------------------

      for (iPage i = ip; i != _pages->end(); ++i) {
//            refresh |= (*i)->abbox();
            delete *i;
            }
      _pages->erase(ip, _pages->end());
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
            bool hasTimesig = false;
            bool hasKeysig  = false;
            bool hasClef    = false;
            int strack      = i * VOICES;
            int etrack      = strack + VOICES;

            for (int track = strack; track < etrack; ++track) {
                  for (Segment* seg = m->first(); seg; seg = seg->next()) {
                        if (seg->segmentType() == Segment::SegChordRest)
                              break;
                        Element* el = seg->element(track);
                        if (!el)
                              continue;
                        switch (el->type()) {
                              case TIMESIG:
                                    hasTimesig = true;
                                    break;
                              case KEYSIG:
                                    hasKeysig = true;
                                    break;
                              case CLEF:
                                    hasClef = true;
                                    ((Clef*)el)->setSmall(false);
                                    break;
                              default:
                                    break;
                              }
                        }
                  }
            if (tick == 0 && !hasTimesig) {
                  int z, n;
                  _score->sigmap->timesig(tick, z, n);
                  TimeSig* ts = new TimeSig(_score);
                  ts->setStaff(staff);
                  ts->setTick(tick);
                  ts->setSig(z, n);
                  ts->setGenerated(true);
                  m->add(ts);
                  }
            if (!hasKeysig) {
                  int clef = staff->clef()->clef(tick);
                  int clefOffset = clefTable[clef].yOffset;
                  int idx = _score->keymap->key(tick);
                  if (idx) {
                        KeySig* ks = new KeySig(_score, idx, clefOffset);
                        ks->setStaff(staff);
                        ks->setTick(tick);
                        ks->setGenerated(true);
                        m->add(ks);
                        }
                  }
            if (!hasClef) {
                  int idx = staff->clef()->clef(tick);
                  Clef* cs = new Clef(_score, idx);
                  cs->setStaff(staff);
                  cs->setTick(tick);
                  cs->setGenerated(true);
                  m->add(cs);
                  }
            }
      }

//---------------------------------------------------------
//   clearGenerated
//    remove generated elements form measure
//---------------------------------------------------------

void ScoreLayout::clearGenerated(Measure* m)
      {
      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            std::vector<Element*>* el = seg->elist();
            for (std::vector<Element*>::iterator i = el->begin(); i != el->end(); ++i) {
                  Element* el = *i;
                  if (el && el->generated())
                        *i = 0;
                  }
            }
      }

//---------------------------------------------------------
//   addGenerated
//---------------------------------------------------------

void ScoreLayout::addGenerated(Measure*)
      {
/*      for (Segment* seg = m->first(); seg; seg = seg->next()) {
            int tick = seg->tick();

            std::vector<Element*>* el = seg->elist();
            for (std::vector<Element*>::iterator i = el->begin(); i != el->end(); ++i) {
                  Element* el = *i;
                  if (el && el->generated())
                        *i = 0;
                  }
            }
*/
      }

//---------------------------------------------------------
//   addMeasure
//---------------------------------------------------------

double Page::addMeasure(Measure* m, double y)
      {
      //---------------------------------------------------
      //    collect page elements from measure
      //---------------------------------------------------

      ElementList sel = *(m->pel());
      m->pel()->clear();
      bool textFound = false;
      for (iElement ie = sel.begin(); ie != sel.end(); ++ie) {
            Element* el = *ie;
            add(el);

            el->layout();
            if (el->type() == TEXT) {
                  int style = ((TextElement*)el)->style();
                  if (style == TEXT_STYLE_TITLE
                     || style == TEXT_STYLE_SUBTITLE
                     || style == TEXT_STYLE_COMPOSER
                     || style == TEXT_STYLE_POET
                     || style == TEXT_STYLE_TRANSLATOR) {
                        if (el->pos().y() > y)
                              y = el->pos().y();
                        textFound = true;
                        }
                  }
            }
      if (textFound)
            y += point(::style->staffUpperBorder);
      return y;
      }

//---------------------------------------------------------
//   layoutPage
//    return true, if next page must be relayouted
//---------------------------------------------------------

bool ScoreLayout::layoutPage(Page* page, Measure*& im, iSystem& is)
      {
      page->layout();

      // usable width of page:
      qreal w  = _pageFormat->width() - page->lm() - page->rm();
      qreal x  = page->lm();
      qreal ey = page->height() - page->bm() - point(::style->staffLowerBorder);

      page->systems()->clear();
      page->pel()->clear();
      qreal y = page->addMeasure(im, page->tm());

      int systemNo = 0;
      while (im) {
            System* system = layoutSystem(im, is, x, y, w);
            system->setParent(page);

            qreal h = system->bbox().height() + point(::style->systemDistance);
            if (y + h >= ey) {  // system does not fit on page
                  // rollback
                  im = system->measures()->front();
                  --is;
                  break;
                  }
            page->appendSystem(system);

            //  move system vertically to final position:

            double systemDistance;
            if (systemNo == 1)
                  systemDistance = point(style->staffUpperBorder);
            else
                  systemDistance = point(style->systemDistance);
            system->move(0.0, systemDistance);
            y += h;

            if (is != systems()->end())
                  ++is;
            if (system->pageBreak())
                  break;
            }

      //-----------------------------------------------------------------------
      // if remaining y space on page is greater (pageHeight*pageFillLimit)
      // then insert space between staffs to fill page
      //-----------------------------------------------------------------------

      double restHeight = ey - y;
      double ph = page->height()
            - point(::style->staffLowerBorder + ::style->staffUpperBorder);

      if (restHeight > (ph * ::style->pageFillLimit))
            return true;

      SystemList* sl   = page->systems();
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

System* ScoreLayout::layoutSystem(Measure*& im, iSystem& is, qreal x, qreal y, qreal w)
      {
      // get next system:
      System* system;
      if (is == _systems->end()) {
            system = new System(_score);
            _systems->push_back(system);
            is = _systems->end();
            --is;
            }
      else {
            system = *is;
            system->clear();   // remove measures from system
            }

      for (int i = system->staves()->size(); i < _score->nstaves(); ++i)
            system->insertStaff(_score->staff(i), i);

      double systemOffset = system->layout(QPointF(x, y), w);

      //-------------------------------------------------------
      //    Round I
      //    find out how many measures fit on current line
      //-------------------------------------------------------

      int nm              = 0;
      double systemWidth  = w - systemOffset;
      double minWidth     = 0;
      std::vector<double> mwList;
      double uStretch = 0.0;

      bool pageBreak = false;
      for (Measure* m = im; m; m = m->next()) {
            pageBreak = m->pageBreak();
//            if (nm && m->lineBreak())
//                  break;

            clearGenerated(m);
            m->setSystem(system);   // needed by m->layout()
            if (m == im)
                  processSystemHeader(m);  // add generated clef+keysig+timesig
            else
                  addGenerated(m);  //DEBUG
            MeasureWidth mw = m->layoutX(1.0);
            double ww = mw.stretchable;

            mwList.push_back(ww);
            double stretch = m->userStretch() * ::style->measureSpacing;
            ww *= stretch;
            if (ww < point(::style->minMeasureWidth))
                  ww = point(::style->minMeasureWidth);

            if (minWidth + ww > systemWidth) {
                  // minimum is one measure
                  if (nm == 0) {
                        minWidth  += ww;
                        nm = 1;
                        printf("warning: system too small (%f+%f) > %f\n",
                           minWidth, ww, systemWidth);
                        // bad things are happening here
                        }
                  break;
                  }
            ++nm;
            minWidth  += ww;
            uStretch  += stretch;
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

      minWidth = 0.0;
      double totalWeight = 0.0;
      Measure* itt = im;
      for (int i = 0; itt && (i < nm); ++i, itt = itt->next()) {
            minWidth    += mwList[i];
            totalWeight += itt->tickLen() * itt->userStretch();
            }

      double rest = layoutDebug ? 0.0 : systemWidth - minWidth;
      QPointF pos(systemOffset, 0);

      itt = im;
      for (int i = 0; itt && (i < nm); ++i, itt = itt->next()) {
            system->measures()->push_back(itt);
            itt->setPos(pos);
            double weight = itt->tickLen() * itt->userStretch();
            double ww     = mwList[i] + rest * weight / totalWeight;
            itt->layout(ww);
            pos.rx() += ww;
            }

      system->layout2();      // layout staff distances

      im = itt;
      if (is != systems()->end())
            ++is;
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
      _pages->update();
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
//   ScoreLayout
//---------------------------------------------------------

ScoreLayout::ScoreLayout(const ScoreLayout& l)
      {
      _score      = l._score;
      _spatium    = l._spatium;
      _pageFormat = l._pageFormat;
      _measures   = l._measures;
      _pages      = new PageList(*(l._pages));
      _systems    = new SystemList(*(l._systems));
      _needLayout = l._needLayout;
      }


