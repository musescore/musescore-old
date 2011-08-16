//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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

#include "navigator.h"
#include "musescore.h"
#include "scoreview.h"
#include "libmscore/score.h"
#include "libmscore/page.h"
#include "preferences.h"
#include "painterqt.h"
#include "libmscore/mscore.h"
#include "libmscore/system.h"
#include "libmscore/measurebase.h"

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

Navigator::Navigator(QScrollArea* sa, QWidget* parent)
  : QWidget(parent)
      {
      setAttribute(Qt::WA_NoBackground);
      _score         = 0;
      scrollArea     = sa;
      _cv            = 0;
      moving         = false;
      recreatePixmap = false;
      viewRect       = QRect();
      cachedWidth    = -1;
      connect(&watcher, SIGNAL(finished()), SLOT(pmFinished()));
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Navigator::resizeEvent(QResizeEvent* ev)
      {
      if (ev->size().height() == ev->oldSize().height())
            return;
      if (!isVisible())
            return;
      if (_cv) {
            qreal m = height() / (_score->pageFormat()->height() * DPI);
            matrix.setMatrix(m, matrix.m12(), matrix.m13(), matrix.m21(), m,
               matrix.m23(), matrix.m31(), matrix.m32(), matrix.m33());

            Page* lp = _score->pages().back();
            int w    = int ((lp->x() + lp->width()) * matrix.m11());
            if (w != cachedWidth) {
                  cachedWidth = w;
                  setFixedWidth(w);
                  QScrollArea* sa = mscore->navigatorScrollArea();
                  if (!sa->horizontalScrollBar()->isVisible() && (w > sa->width())) {
                        // we will get another resize event with bc. a scrollbar will
                        // be added
                        return;
                        }
                  updateViewRect();
                  layoutChanged();
                  }

            }
      else
            recreatePixmap = true;
      }

//---------------------------------------------------------
//   showNavigator
//---------------------------------------------------------

void MuseScore::showNavigator(bool visible)
      {
      Navigator* n = static_cast<Navigator*>(_navigator->widget());
      if (n == 0 && visible) {
            n = new Navigator(_navigator, this);
            n->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
            _navigator->setWidget(n);
            _navigator->setWidgetResizable(true);
            // connect(n, SIGNAL(viewRectMoved(const QRectF&)), SLOT(setViewRect(const QRectF&)));
            n->setScore(cv);
            n->updateViewRect();
            }
      _navigator->setShown(visible);
      getAction("toggle-navigator")->setChecked(visible);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Navigator::setScore(ScoreView* v)
      {
      if (_cv) {
            disconnect(this, SIGNAL(viewRectMoved(const QRectF&)), _cv, SLOT(setViewRect(const QRectF&)));
            disconnect(_cv, SIGNAL(viewRectChanged()), this, SLOT(updateViewRect()));
            }
      _cv = QPointer<ScoreView>(v);
      if (v) {
            _score  = v->score();
            connect(this, SIGNAL(viewRectMoved(const QRectF&)), v, SLOT(setViewRect(const QRectF&)));
            connect(_cv,  SIGNAL(viewRectChanged()), this, SLOT(updateViewRect()));
            updateViewRect();
            layoutChanged();
            }
      else {
            _score = 0;
            pm.fill(Qt::gray);
            update();
            }
      }

//---------------------------------------------------------
//   updateViewRect
//---------------------------------------------------------

void Navigator::updateViewRect()
      {
      if (_score == 0) {
            setViewRect(QRect());
            return;
            }
      QRectF r(0.0, 0.0, _cv->width(), _cv->height());
      setViewRect(_cv->toLogical(r));
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Navigator::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      QRect r(ev->rect());
      if (pm.isNull()) {
            p.fillRect(r, QColor(Qt::darkGray));
            return;
            }
      p.drawPixmap(r.topLeft(), pm, r);
      QPen pen(Qt::blue, 2.0);
      p.setPen(pen);
      p.setBrush(QColor(0, 0, 255, 40));
      if (_cv && !recreatePixmap) {
            p.drawRect(viewRect);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Navigator::mousePressEvent(QMouseEvent* ev)
      {
      if (_cv == 0)
            return;
      startMove = ev->pos();
      if (!viewRect.contains(startMove)) {
            QPointF p = matrix.inverted().map(QPointF(ev->pos()));
            QRectF r(_cv->toLogical(QRectF(0.0, 0.0, _cv->width(), _cv->height())));
            double dx = p.x() - (r.x() + (r.width() * .5));
            r.translate(dx, 0.0);
            setViewRect(r);
            emit viewRectMoved(matrix.inverted().mapRect(viewRect));
            update();
            }
      moving = true;
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Navigator::mouseMoveEvent(QMouseEvent* ev)
      {
      if (!moving)
            return;
      QPoint delta = ev->pos() - startMove;
      viewRect.translate(delta);
      startMove = ev->pos();

      if (viewRect.x() <= 0 && viewRect.width() < width())
            viewRect.moveLeft(0);
      else if (viewRect.right() > width() && viewRect.width() < width())
            viewRect.moveRight(width());
      if (viewRect.height() == height())
            viewRect.moveTop(0);
      else if (viewRect.height() < height()) {
            if (viewRect.y() < 0)
                  viewRect.moveTop(0);
            else if (viewRect.bottom() > height())
                  viewRect.moveBottom(height());
            }
      else {
            if (viewRect.bottom() < (height()))
                  viewRect.moveBottom(height());
            else if (viewRect.top() > 0)
                  viewRect.moveTop(0);
            }

      emit viewRectMoved(matrix.inverted().mapRect(viewRect));
      int x = delta.x() > 0 ? viewRect.x() + viewRect.width() : viewRect.x();
      scrollArea->ensureVisible(x, height()/2, 0, 0);
      update();
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Navigator::setViewRect(const QRectF& _viewRect)
      {
      viewRect = matrix.mapRect(_viewRect).toRect();
      scrollArea->ensureVisible(viewRect.x(), 0);
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Navigator::mouseReleaseEvent(QMouseEvent*)
      {
      moving = false;
//      scrollArea->ensureVisible(viewRect.x(), 0);
      }

//---------------------------------------------------------
//   paintElement
//---------------------------------------------------------

static void paintElement(void* data, Element* e)
      {
      PainterQt* p = static_cast<PainterQt*>(data);
      p->painter()->save();
      p->painter()->setPen(QPen(e->curColor()));
      p->painter()->translate(e->pagePos());
      e->draw(p);
      p->painter()->restore();
      }

//---------------------------------------------------------
//   createPixmap
//---------------------------------------------------------

QImage Navigator::createPixmap()
      {
      QMutexLocker locker(_score->mutex());

      QImage pixmap = QImage(size(), QImage::Format_ARGB32_Premultiplied);
      QPainter p(&pixmap);

      QColor _fgColor(Qt::white);
      QColor _bgColor(Qt::darkGray);

      PainterQt painter(&p, 0);
      p.setRenderHint(QPainter::Antialiasing, false);

      QRect r(QPoint(), size());
      p.fillRect(r, _bgColor);
      p.setTransform(matrix);

      foreach(Page* page, _score->pages()) {
            p.save();
            p.translate(page->pos());
            p.fillRect(page->bbox(), _fgColor);
            foreach(System* s, *page->systems()) {
                  foreach(MeasureBase* m, s->measures())
                        m->scanElements(&painter, paintElement, false);
                  }
            page->scanElements(&painter, paintElement, false);

            p.setFont(QFont("FreeSans", 400));  // !!
            p.setPen(QColor(0, 0, 255, 50));
            p.drawText(page->bbox(), Qt::AlignCenter, QString("%1").arg(page->no()+1));
            p.restore();
            }
      return pixmap;
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void Navigator::layoutChanged()
      {
      pm.fill(Qt::darkGray);
      if (_cv == 0 || _score->pages().isEmpty() || !_score->mutex()->tryLock()) {
            recreatePixmap = true;
            update();
            return;
            }
      _score->mutex()->unlock();
      Page* lp = _score->pages().back();
      int w    = int ((lp->x() + lp->width()) * matrix.m11());
      if (w != cachedWidth) {
            cachedWidth = w;
            setFixedWidth(w);
            }
      if (w == 0)
            return;
      updatePixmap = QtConcurrent::run(this, &Navigator::createPixmap);
      watcher.setFuture(updatePixmap);
      update();
      }

//---------------------------------------------------------
//   pmFinished
//---------------------------------------------------------

void Navigator::pmFinished()
      {
      if (recreatePixmap) {
            recreatePixmap = false;
            layoutChanged();
            }
      else {
            pm = QPixmap::fromImage(updatePixmap.result());
            update();
            }
      }

