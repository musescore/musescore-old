//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009-2010 Werner Schweer and others
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

#include "scoretab.h"
#include "scoreview.h"
#include "scoreview.h"
#include "score.h"
#include "magbox.h"
#ifdef OMR
#include "omr/omr.h"
#include "omr/omrview.h"
#endif
#include "excerpt.h"

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

ScoreTab::ScoreTab(QList<Score*>* sl, QWidget* parent)
   : QWidget(parent)
      {
      scoreList = sl;
      QVBoxLayout* layout = new QVBoxLayout;
      setLayout(layout);
      layout->setSpacing(0);
      layout->setMargin(2);

      tab = new QTabBar;
      tab->setExpanding(false);
      tab->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab->setFocusPolicy(Qt::StrongFocus);

      tab2 = new QTabBar;
      tab2->setExpanding(false);
      tab2->setSelectionBehaviorOnRemove(QTabBar::SelectRightTab);
      tab2->setFocusPolicy(Qt::StrongFocus);
      tab2->setVisible(false);
      tab2->setTabsClosable(false);

      stack = new QStackedLayout;
      layout->addWidget(tab);
      layout->addWidget(tab2);
      layout->addLayout(stack);
      tab->setTabsClosable(true);

      foreach(Score* s, *sl)
            insertTab(s);

      connect(tab, SIGNAL(currentChanged(int)), this, SLOT(setCurrent(int)));
      connect(tab2, SIGNAL(currentChanged(int)), this, SLOT(setExcerpt(int)));
      connect(tab, SIGNAL(tabCloseRequested(int)), this, SIGNAL(tabCloseRequested(int)));
      }

//---------------------------------------------------------
//   view
//---------------------------------------------------------

ScoreView* ScoreTab::view(int n) const
      {
      QSplitter* s = viewSplitter(n);
      if (s)
            return static_cast<ScoreView*>(s->widget(0));
      return 0;
      }

//---------------------------------------------------------
//   viewSplitter
//---------------------------------------------------------

QSplitter* ScoreTab::viewSplitter(int n) const
      {
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(n).value<void*>());
      if (tsv == 0) {
            printf("ScoreTab::viewSplitter %d is zero\n", n);
            return 0;
            }
      Score* score = tsv->score;
      if (tsv->part) {
            QList<Excerpt*>* excerpts = score->excerpts();
            if (excerpts && !excerpts->isEmpty())
                  score = excerpts->at(tsv->part - 1)->score();
            }

      for (int i = 0; i < stack->count(); ++i) {
            QSplitter* sp = static_cast<QSplitter*>(stack->widget(i));
            ScoreView* v = static_cast<ScoreView*>(sp->widget(0));
            if (v->score() == score)
                  return sp;
            }
      return 0;
      }

//---------------------------------------------------------
//   setCurrent
//---------------------------------------------------------

void ScoreTab::setCurrent(int n)
      {
      if (n == -1) {
            emit currentScoreViewChanged(0);
            return;
            }
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(n).value<void*>());
      QSplitter* vs = viewSplitter(n);

      ScoreView* v;
      if (!vs) {
            Score* score = scoreList->value(n);
            vs = new QSplitter;
            v = new ScoreView;
            tab2->blockSignals(true);
            tab2->setCurrentIndex(0);
            tab2->blockSignals(false);
            vs->addWidget(v);
            v->setScore(scoreList->value(n));
            stack->addWidget(vs);

#ifdef OMR
            if (score->showOmr()) {
                  Omr* omr = score->omr();
                  OmrView* sv = omr->newOmrView(v);
                  vs->addWidget(sv);
                  connect(v, SIGNAL(scaleChanged(double)), sv, SLOT(setScale(double)));
                  connect(v, SIGNAL(offsetChanged(double,double)), sv, SLOT(setOffset(double,double)));
                  connect(v, SIGNAL(nextPage()), sv, SLOT(nextPage()));
                  connect(v, SIGNAL(previousPage()), sv, SLOT(previousPage()));
                  const QTransform _matrix = v->matrix();
                  double _spatium = score->spatium();
                  double scale = _matrix.m11() * _spatium;
                  sv->setScale(scale);
                  sv->setOffset(_matrix.dx(), _matrix.dy());
                  QList<int> sizes;
                  sizes << 100 << 100;
                  vs->setSizes(sizes);
                  }
#endif
            }
      else
            v = static_cast<ScoreView*>(vs->widget(0));
      stack->setCurrentWidget(vs);
      if (v) {
            Score* score = v->score();
            if (score->parentScore())
                  score = score->parentScore();
            QList<Excerpt*>* excerpts = score->excerpts();
            if (v && excerpts && !excerpts->isEmpty()) {
                  int n = tab2->count();
                  tab2->blockSignals(true);
                  for (int i = 0; i < n; ++i)
                        tab2->removeTab(0);
                  tab2->addTab(score->name());
                  foreach(Excerpt* excerpt, *excerpts)
                        tab2->addTab(excerpt->score()->name());
                  tab2->setVisible(true);
                  tab2->setCurrentIndex(tsv->part);
                  tab2->blockSignals(false);
                  }
            else
                  tab2->setVisible(false);
            }
      else
            tab2->setVisible(false);
      emit currentScoreViewChanged(v);
      }

//---------------------------------------------------------
//   updateExcerpts
//    number of excerpts in score changed
//---------------------------------------------------------

void ScoreTab::updateExcerpts()
      {
      int idx = currentIndex();
      if (idx == -1)
            return;
      ScoreView* v = view(idx);
      Score* score = v->score();
      QList<Excerpt*>* excerpts = score->excerpts();
      if (v && excerpts && !excerpts->isEmpty()) {
printf("ScoreTab::updateExcerpts()\n");
            int n = tab2->count();
            tab2->blockSignals(true);
            for (int i = 0; i < n; ++i)
                  tab2->removeTab(0);
            tab2->addTab(score->name());
            foreach(Excerpt* excerpt, *excerpts)
                  tab2->addTab(excerpt->score()->name());
            tab2->setVisible(true);
            tab2->blockSignals(false);
            }
      else
            tab2->setVisible(false);
      }

//---------------------------------------------------------
//   setExcerpt
//---------------------------------------------------------

void ScoreTab::setExcerpt(int n)
      {
printf("ScoreTab::setExcerpt %d\n", n);
      if (n == -1)
            return;
      int idx           = tab->currentIndex();
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(idx).value<void*>());
      if (tsv == 0)
            return;
      tsv->part         = n;
      QSplitter* vs     = viewSplitter(idx);
      ScoreView* v;
      Score* score = tsv->score;
      if (n) {
            QList<Excerpt*>* excerpts = score->excerpts();
printf("  excerpts(%p) %d\n", score, excerpts->size());
            if (!excerpts->isEmpty()) {
                  score = excerpts->at(n - 1)->score();
                  }
            }
      if (!vs) {
            vs = new QSplitter;
            v = new ScoreView;
            vs->addWidget(v);
            v->setScore(score);
            stack->addWidget(vs);
            }
      else
            v = static_cast<ScoreView*>(vs->widget(0));
      stack->setCurrentWidget(vs);
      emit currentScoreViewChanged(v);
      }

//---------------------------------------------------------
//   insertTab
//---------------------------------------------------------

void ScoreTab::insertTab(Score* s)
      {
      int idx = scoreList->indexOf(s);
      tab->blockSignals(true);
      tab->insertTab(idx, s->name());
      tab->setTabData(idx, QVariant::fromValue<void*>(new TabScoreView(s)));
      tab->blockSignals(false);
      }

//---------------------------------------------------------
//   setTabText
//---------------------------------------------------------

void ScoreTab::setTabText(int idx, const QString& s)
      {
      tab->setTabText(idx, s);
      }

//---------------------------------------------------------
//   currentIndex
//---------------------------------------------------------

int ScoreTab::currentIndex() const
      {
      return tab->currentIndex();
      }

//---------------------------------------------------------
//   setCurrentIndex
//---------------------------------------------------------

void ScoreTab::setCurrentIndex(int idx)
      {
      if (tab->currentIndex() == idx)
            setCurrent(idx);
      else
            tab->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void ScoreTab::removeTab(int idx)
      {
      TabScoreView* tsv = static_cast<TabScoreView*>(tab->tabData(idx).value<void*>());
      Score* score = tsv->score;

      for (int i = 0; i < stack->count(); ++i) {
            ScoreView* v = static_cast<ScoreView*>(stack->widget(i));
            if (v->score() == score) {
                  stack->takeAt(i);
                  delete v;
                  break;
                  }
            }

      int cidx = currentIndex();
      tab->removeTab(idx);
      if (cidx > idx)
            cidx -= 1;
      setCurrentIndex(cidx);
      }

//---------------------------------------------------------
//   initScoreView
//---------------------------------------------------------

void ScoreTab::initScoreView(int idx, double mag, int magIdx, double xoffset, double yoffset)
      {
      ScoreView* v = view(idx);
      if (!v)  {
            v = new ScoreView;
            v->setScore(scoreList->value(idx));
            stack->addWidget(v);
            }
      v->setMag(magIdx, mag);
      v->setOffset(xoffset, yoffset);
      }

