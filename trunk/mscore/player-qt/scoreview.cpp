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
//   PlaybackCursor
//---------------------------------------------------------

PlaybackCursor::PlaybackCursor(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      setFlag(QGraphicsItem::ItemHasNoContents, false);
      setCacheMode(QGraphicsItem::ItemCoordinateCache);
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void PlaybackCursor::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      QColor c(0,0,255,50);
      painter->fillRect(QRect(0, 0, width(), height()), c);
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      setFlag(QGraphicsItem::ItemHasNoContents, false);
      setCacheMode(QGraphicsItem::ItemCoordinateCache);
      playbackCursor = new PlaybackCursor(this);
      score = 0;
      seq->setView(this);
      grabGesture(Qt::SwipeGesture);
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
      score->setPrinting(true);     // render only printable elements

      seq->setScore(score);
      Page* page = score->pages()[_currentPage];
      QRectF pr(page->abbox());
      qreal m1 = parentWidth()  / pr.width();
      qreal m2 = parentHeight() / pr.height();
      mag = qMax(m1, m2);
      _boundingRect = QRectF(0.0, 0.0, pr.width() * mag, pr.height() * mag);

      setWidth(pr.width() * mag);
      setHeight(pr.height() * mag);
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

      if (pm.isNull())
            return;

      QPainter p(&pm);
      PainterQt pqt(&p, this);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.drawTiledPixmap(QRect(0, 0, width(), height()), QPixmap(":/mobile/images/paper.png"), QPoint(0,0));

      p.scale(mag, mag);

      Page* page = score->pages()[_currentPage];
      QList<const Element*> el;
      foreach(System* s, *page->systems()) {
            foreach(MeasureBase* m, s->measures())
                  m->scanElements(&el, collectElements, false);
            }
      page->scanElements(&el, collectElements, false);
//      qStableSort(el.begin(), el.end(), elementLessThan);

      foreach(const Element* e, el) {
            p.save();
            p.translate(e->pagePos());
            p.setPen(QPen(e->curColor()));
            e->draw(&pqt);
            p.restore();
            }
      p.end();

      painter->drawPixmap(0, 0, pm);
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

//---------------------------------------------------------
//   play
//---------------------------------------------------------

void ScoreView::play()
      {
      seq->startStop();
      }

//---------------------------------------------------------
//   rewind
//---------------------------------------------------------

void ScoreView::rewind()
      {
      seq->seek(0);
      moveCursor(0);
      }

//---------------------------------------------------------
//   setTempo
//---------------------------------------------------------

void ScoreView::setTempo(qreal val)
      {
      seq->setRelTempo(val);
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void ScoreView::moveCursor(int tick)
      {
      Measure* measure = score->tick2measure(tick);
      if (measure == 0)
            return;

      qreal x;
      Segment* s;
      for (s = measure->first(SegChordRest); s;) {
            int t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            int t2;
            Segment* ns = s->next(SegChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  x2 = measure->canvasPos().x() + measure->width();
                  }
            if (tick >= t1 && tick < t2) {
                  int   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1) / dt;
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      QColor c(MScore::selectColor[0]);
      c.setAlpha(50);
      playbackCursor->setColor(c);
      playbackCursor->setTick(tick);

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffY(0) + system->page()->pos().y();
      double _spatium = score->spatium();

      qreal xmag = _spatium / (DPI * SPATIUM20);
      double w   = _spatium * 2.0 + symbols[score->symIdx()][quartheadSym].width(xmag);
      double h   = 8 * _spatium;
      //
      // set cursor height for whole system
      //
      double y2 = 0.0;
      for (int i = 0; i < score->nstaves(); ++i) {
            SysStaff* ss = system->staff(i);
            if (!ss->show())
                  continue;
            y2 = ss->y();
            }
      h += y2;
      x -= _spatium;
      y -= 2* _spatium;

      playbackCursor->setPos(x * mag, y * mag);
      playbackCursor->setWidth(w * mag);
      playbackCursor->setHeight(h * mag);
      }

//---------------------------------------------------------
//   sceneEvent
//---------------------------------------------------------

bool ScoreView::sceneEvent(QEvent* event)
      {
      if (event->type() == QEvent::Gesture) {
            printf("gesture\n");
            QGestureEvent* ge = static_cast<QGestureEvent*>(event);
            QGesture* g = ge->gesture(Qt::SwipeGesture);
            if (g && g->gestureType() == Qt::SwipeGesture) {
                  QSwipeGesture* sg = static_cast<QSwipeGesture*>(g);
                  QSwipeGesture::SwipeDirection d = sg->horizontalDirection();
                  if (d == QSwipeGesture::Left)
                        printf("  swipe left\n");
                  else if (d == QSwipeGesture::Right)
                        printf("  swipe right\n");
                  }
            return true;
            }
      return QGraphicsItem::sceneEvent(event);
      }


