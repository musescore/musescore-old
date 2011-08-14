//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#include <stdio.h>
#include <math.h>
#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QWheelEvent>

#include "scoreview.h"
#include "painterqt.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "libmscore/staff.h"
#include "libmscore/measure.h"
#include "libmscore/segment.h"
#include "libmscore/keysig.h"
#include "libmscore/system.h"

#include "seq.h"

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      setFlag(QGraphicsItem::ItemHasNoContents, false);
      setCacheMode(QGraphicsItem::DeviceCoordinateCache);
      setSmooth(true);
      setCacheMode(QGraphicsItem::ItemCoordinateCache);
      score = 0;
      }

//---------------------------------------------------------
//   loadFile
//---------------------------------------------------------

void ScoreView::setScore(const QString& name)
      {
      if (seq->isPlaying())
            seq->stop();
      _currentPage = 0;
      delete score;
      score = new Score(MScore::defaultStyle());
      score->setName(name);
      QString cs  = score->fileInfo()->suffix();
      QString csl = cs.toLower();

      if (csl == "mscz") {
            if (!score->loadCompressedMsc(name)) {
                  delete score;
                  return;
                  }
            }
      else if (csl == "mscx") {
            if (!score->loadMsc(name)) {
                  delete score;
                  return;
                  }
            }
      int staffIdx = 0;
      foreach(Staff* st, score->staves()) {
            if (st->updateKeymap())
                  st->keymap()->clear();
            int track = staffIdx * VOICES;
            KeySig* key1 = 0;
            for (Measure* m = score->firstMeasure(); m; m = m->nextMeasure()) {
                  for (Segment* s = m->first(); s; s = s->next()) {
                        if (!s->element(track))
                              continue;
                        Element* e = s->element(track);
                        if (e->generated())
                              continue;
                        if ((s->subtype() == SegKeySig) && st->updateKeymap()) {
                              KeySig* ks = static_cast<KeySig*>(e);
                              int naturals = key1 ? key1->keySigEvent().accidentalType() : 0;
                              ks->setOldSig(naturals);
                              st->setKey(s->tick(), ks->keySigEvent());
                              key1 = ks;
                              }
                        }
                  if (m->sectionBreak())
                        key1 = 0;
                  }
            st->setUpdateKeymap(false);
            ++staffIdx;
            }
      score->updateNotes();
      score->doLayout();

      seq->setScore(score);
      setWidth(boundingRect().width());
      setHeight(boundingRect().height());
      update();
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      if (!score)
            return;
      QPixmap pm(width(), height());

      QPainter p(&pm);
      PainterQt pqt(&p, this);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.drawTiledPixmap(QRect(0, 0, width(), height()), QPixmap(":/mobile/images/paper.png"), QPoint(0,0));

      Page* page = score->pages()[_currentPage];
      QList<const Element*> el;
      foreach(System* s, *page->systems()) {
            foreach(MeasureBase* m, s->measures())
                  m->scanElements(&el, collectElements, false);
            }
      page->scanElements(&el, collectElements, false);
      qStableSort(el.begin(), el.end(), elementLessThan);

      foreach(const Element* e, el) {
            p.save();
            p.translate(e->pagePos());
            p.setPen(QPen(e->curColor()));
            e->draw(&pqt);
            p.restore();
            }
      painter->drawPixmap(0, 0, pm);
      }

//---------------------------------------------------------
//   boundingRect
//---------------------------------------------------------

QRectF ScoreView::boundingRect() const
      {
      if (score == 0)
            return QRectF();
      Page* page = score->pages()[_currentPage];
      QRectF pr(page->abbox());
      return QRectF(0.0, 0.0, pr.width(), pr.height());
      }

//---------------------------------------------------------
//   setCurrentPage
//---------------------------------------------------------

void ScoreView::setCurrentPage(int n)
      {
      if (score == 0)
            return;
      if (n < 0)
            n = 0;
      int nn = score->pages().size();
      if (nn == 0)
            return;
      if (n >= nn)
            n = nn - 1;
      _currentPage = n;
      update();
      }

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void ScoreView::nextPage()
      {
      setCurrentPage(_currentPage + 1);
      }

//---------------------------------------------------------
//   prevPage
//---------------------------------------------------------

void ScoreView::prevPage()
      {
      setCurrentPage(_currentPage - 1);
      }

void ScoreView::dataChanged(const QRectF&)
      {
      update();
      }

void ScoreView::updateAll()
      {
      update();
      }

void ScoreView::moveCursor()
      {
      }

void ScoreView::adjustCanvasPosition(const Element*, bool)
      {
      }

void ScoreView::changeEditElement(Element*)
      {
      }

int ScoreView::gripCount() const
      {
      return 0;
      }

const QRectF& ScoreView::getGrip(int) const
      {
      static const QRectF a;
      return a;
      }

const QTransform& ScoreView::matrix() const
      {
      static const QTransform t;
      return t; // _matrix;
      }

void ScoreView::setDropRectangle(const QRectF&)
      {
      }

void ScoreView::cmdAddSlur(Note*, Note*)
      {
      }

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void ScoreView::play()
      {
      seq->startStop();
      }

