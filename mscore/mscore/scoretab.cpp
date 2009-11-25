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
#include "viewer.h"
#include "canvas.h"
#include "score.h"
#include "magbox.h"

//---------------------------------------------------------
//   ScoreTab
//---------------------------------------------------------

ScoreTab::ScoreTab(QList<Score*>* sl, QWidget* parent)
   : QWidget(parent)
      {
      scoreList = sl;
      QVBoxLayout* layout = new QVBoxLayout;
      setLayout(layout);
      tab = new QTabBar;
      stack = new QStackedLayout;
      layout->addWidget(tab);
      layout->addLayout(stack);
      tab->setTabsClosable(true);
      connect(tab, SIGNAL(currentChanged(int)), this, SLOT(setCurrent(int)));
      connect(tab, SIGNAL(tabCloseRequested(int)), this, SIGNAL(tabCloseRequested(int)));
      }

//---------------------------------------------------------
//   viewer
//---------------------------------------------------------

Viewer* ScoreTab::viewer(int n) const
      {
      Score* score = scoreList->value(n);
      for (int i = 0; i < stack->count(); ++i) {
            Viewer* viewer = static_cast<Viewer*>(stack->widget(i));
            if (viewer->score() == score)
                  return viewer;
            }
      return 0;
      }

//---------------------------------------------------------
//   setCurrent
//---------------------------------------------------------

void ScoreTab::setCurrent(int n)
      {
      if (n == -1) {
            emit currentViewerChanged(0);
            return;
            }
      Viewer* v = viewer(n);
      if (!v)  {
            v = new Canvas;
            v->setScore(scoreList->value(n));
            stack->addWidget(v);
            }
      stack->setCurrentWidget(v);
      emit currentViewerChanged(v);
      }

//---------------------------------------------------------
//   insertTab
//---------------------------------------------------------

void ScoreTab::insertTab(int idx, const QString& s)
      {
      tab->insertTab(idx, s);
      }

//---------------------------------------------------------
//   addTab
//---------------------------------------------------------

void ScoreTab::addTab(const QString& s)
      {
      tab->addTab(s);
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
      tab->setCurrentIndex(idx);
      }

//---------------------------------------------------------
//   removeTab
//---------------------------------------------------------

void ScoreTab::removeTab(int idx)
      {
      tab->removeTab(idx);
      Score* score = scoreList->value(idx);
      for (int i = 0; i < stack->count(); ++i) {
            Viewer* viewer = static_cast<Viewer*>(stack->widget(i));
            if (viewer->score() == score) {
                  delete viewer;
                  stack->takeAt(i);
                  return;
                  }
            }
      }

//---------------------------------------------------------
//   initViewer
//---------------------------------------------------------

void ScoreTab::initViewer(int idx, double mag, int magIdx, double xoffset, double yoffset)
      {
      Viewer* v = viewer(idx);
      if (!v)  {
            v = new Canvas;
            v->setScore(scoreList->value(idx));
            stack->addWidget(v);
            }
      v->setMag(magIdx, mag);
      v->setOffset(xoffset, yoffset);
      }


