//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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
#include "omr/omr.h"
#include "omr/scanview.h"

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
      stack = new QStackedLayout;
      layout->addWidget(tab);
      layout->addLayout(stack);
      tab->setTabsClosable(true);
      connect(tab, SIGNAL(currentChanged(int)), this, SLOT(setCurrent(int)));
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

QSplitter* ScoreTab::viewSplitter(int n) const
      {
      Score* score = scoreList->value(n);
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
      QSplitter* vs = viewSplitter(n);
      ScoreView* v;
      if (!vs)  {
            Score* score = scoreList->value(n);
            vs = new QSplitter;
            v = new ScoreView;
            vs->addWidget(v);
            v->setScore(scoreList->value(n));
            stack->addWidget(vs);
            if (score->showOmr()) {
                  Omr* omr = score->omr();
                  ScanView* sv = omr->newScanView();
                  vs->addWidget(sv);
                  connect(v, SIGNAL(scaleChanged(double)), sv, SLOT(setScale(double)));
                  connect(v, SIGNAL(offsetChanged(double,double)), sv, SLOT(setOffset(double,double)));
                  connect(v, SIGNAL(nextPage()), sv, SLOT(nextPage()));
                  connect(v, SIGNAL(previousPage()), sv, SLOT(previousPage()));
                  const QTransform _matrix = v->matrix();
                  double _spatium = score->spatium();
                  double scale = _matrix.m11() * _spatium;
                  sv->setScale(scale);
                  double dx = _matrix.dx() / scale;
                  double dy = _matrix.dy() / scale;
                  sv->setOffset(dx, dy);
                  QList<int> sizes;
                  sizes << 100 << 100;
                  vs->setSizes(sizes);
                  }
            }
      else
            v = static_cast<ScoreView*>(vs->widget(0));
      stack->setCurrentWidget(vs);
      emit currentScoreViewChanged(v);
      }

//---------------------------------------------------------
//   insertTab
//---------------------------------------------------------

void ScoreTab::insertTab(int idx, const QString& s)
      {
      tab->insertTab(idx, s);
      tab->setTabData(idx, QVariant::fromValue<void*>(scoreList->value(idx)));
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
      Score* score = static_cast<Score*>(tab->tabData(idx).value<void*>());
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

