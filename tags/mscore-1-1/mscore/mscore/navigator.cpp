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

#include "navigator.h"
#include "mscore.h"
#include "scoreview.h"
#include "score.h"
#include "page.h"
#include "preferences.h"

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

Navigator::Navigator(QWidget* parent)
  : QFrame(parent)
      {
      setAttribute(Qt::WA_NoBackground);

      setFrameStyle(QFrame::Box | QFrame::Raised);
      setLineWidth(2);
      _score = 0;
      _cv    = 0;
      moving = false;
      redraw = false;
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Navigator::resizeEvent(QResizeEvent* ev)
      {
      pm = QPixmap(ev->size());
      redraw = true;
      if (_score) {
            qreal m = (height()-10.0) / (_score->pageFormat()->height() * DPI);
            matrix.setMatrix(m, matrix.m12(), matrix.m13(), matrix.m21(), m,
            matrix.m23(), matrix.m31(), matrix.m32(), matrix.m33());
            }
      pm.fill(Qt::gray);
      }

//---------------------------------------------------------
//   showNavigator
//---------------------------------------------------------

void MuseScore::showNavigator(bool visible)
      {
      if (navigator == 0) {
            navigator = new Navigator(this);
            navigator->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
            connect(navigator, SIGNAL(viewRectMoved(const QRectF&)), SLOT(setViewRect(const QRectF&)));
            }
      navigator->setShown(visible);
      navigator->setScore(cv);
      navigator->updateViewRect();
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
            if (_score)
                  disconnect(_score, SIGNAL(layoutChanged()), this, SLOT(updateLayout()));
            }
      _cv = QPointer<ScoreView>(v);
      if (v) {
            _score  = v->score();
            connect(this, SIGNAL(viewRectMoved(const QRectF&)), v, SLOT(setViewRect(const QRectF&)));
            connect(_cv,  SIGNAL(viewRectChanged()), this, SLOT(updateViewRect()));
            if (_score)
                  connect(_score,  SIGNAL(layoutChanged()), this, SLOT(updateLayout()));
            updateLayout();
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
      qreal m = (height()-10.0) / (_score->pageFormat()->height() * DPI);
      matrix.setMatrix(m, matrix.m12(), matrix.m13(), matrix.m21(), m,
         matrix.m23(), matrix.m31(), matrix.m32(), matrix.m33());

      QRectF r(0.0, 0.0, _cv->width(), _cv->height());
      setViewRect(_cv->toLogical(r));
      }

//---------------------------------------------------------
//   updateLayout
//---------------------------------------------------------

void Navigator::updateLayout()
      {
      redraw = true;
      update();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Navigator::paintEvent(QPaintEvent* ev)
      {
      QPainter p;
      QRect r(ev->rect());
      if (redraw) {
            if (_cv) {
                  redraw = false;
                  qreal m = (height()-10.0) / (_score->pageFormat()->height() * DPI);
                  matrix.setMatrix(m, matrix.m12(), matrix.m13(), matrix.m21(), m,
                     matrix.m23(), matrix.m31(), matrix.m32(), matrix.m33());

                  QRectF r(0.0, 0.0, _cv->width(), _cv->height());
                  viewRect = matrix.mapRect(_cv->toLogical(r)).toRect();

                  QColor _fgColor(Qt::white);
                  QColor _bgColor(Qt::gray);
                  int dx = lrint(matrix.m11());
                  int dy = lrint(matrix.m22());

                  r.setRect(0, 0, width(), height());
                  QRect rr(r.x()-dx, r.y()-dy, r.width()+2*dx, r.height()+2*dy);

                  p.begin(&pm);
                  p.setRenderHint(QPainter::Antialiasing, false);

                  p.fillRect(rr, _fgColor);
                  p.setTransform(matrix);
                  QRegion r1(rr);
                  foreach(const Page* page, _score->pages()) {
                        QRectF pbbox(page->abbox());
                        r1 -= matrix.mapRect(pbbox).toRect();
                        p.translate(page->pos());
                        page->draw(p);
                        p.translate(-page->pos());
                        }

                  QRectF fr = matrix.inverted().mapRect(QRectF(rr));
                  QList<const Element*> ell = _score->items(fr);
                  qStableSort(ell.begin(), ell.end(), elementLessThan);
                  for (int i = 0; i < ell.size(); ++i) {
                        const Element* e = ell.at(i);
                        e->itemDiscovered = 0;

                        if (!(e->visible() || _score->showInvisible()))
                              continue;

                        QPointF ap(e->canvasPos());
                        p.translate(ap);
                        p.setPen(QPen(e->color()));
                        e->draw(p);
                        p.translate(-ap);
                        }

                  p.setMatrixEnabled(false);
                  p.setClipRegion(r1);
                  p.fillRect(rr, _bgColor);
                  p.end();
                  }
            }
      p.begin(this);
      p.drawPixmap(r.topLeft(), pm, r);
      QPen pen(Qt::blue, 2.0);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      if (_cv)
            p.drawRect(viewRect);
      p.end();
      QFrame::paintEvent(ev);
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

      if (viewRect.x() < 0) {
            double dx = -viewRect.x() / matrix.m11();
            viewRect.moveLeft(0);
            if (matrix.dx() < 5.0)
                  matrix.translate(dx, 0.0);
            redraw = true;
            }
      if (viewRect.right() > width()) {
            double dx = (rect().right() - viewRect.right()) / matrix.m11();
            dx *= 5.0;
            viewRect.moveRight(width());
            matrix.translate(dx, 0.0);
            redraw = true;
            }
      if (viewRect.y() < 0)
            viewRect.moveTop(0);
      else if (viewRect.bottom() > height())
            viewRect.moveBottom(height());

      emit viewRectMoved(matrix.inverted().mapRect(viewRect));
      update();
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Navigator::setViewRect(const QRectF& _viewRect)
      {
      viewRect = matrix.mapRect(_viewRect).toRect();
      update();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Navigator::mouseReleaseEvent(QMouseEvent*)
      {
      moving = false;
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void Navigator::layoutChanged()
      {
      redraw = true;
      update();
      }

