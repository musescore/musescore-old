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

#include "libmscore/scoreproxy.h"
#include "scoreview.h"

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QWidget* parent)
   : QWidget(parent)
      {
      }

//---------------------------------------------------------
//   loadFile
//---------------------------------------------------------

void ScoreView::loadFile(const QString& s)
      {
      score = new ScoreProxy;
      if (!score->read(s)) {
            printf("score read failed\n");
            exit(1);
            }
      _matrix = QTransform();
      _matrix.scale(2.0, 2.0);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ScoreView::paintEvent(QPaintEvent* e)
      {
      QPainter painter(this);
      painter.setRenderHint(QPainter::Antialiasing, true);
      painter.setRenderHint(QPainter::TextAntialiasing, true);
      painter.setWorldTransform(_matrix, false);
      score->expose(&painter, 0, 0, 0, e->rect().width(), e->rect().height());
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ScoreView::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            zoom(event->delta() / 120, event->pos());
            return;
            }
      int dx = 0;
      int dy = 0;
      if (event->modifiers() & Qt::ShiftModifier || event->orientation() == Qt::Horizontal) {
            //
            //    scroll horizontal
            //
            int n = width() / 10;
            if (n < 2)
                  n = 2;
            dx = event->delta() * n / 120;
            }
      else {
            //
            //    scroll vertical
            //
            int n = height() / 10;
            if (n < 2)
                  n = 2;
            dy = event->delta() * n / 120;
            }
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ScoreView::mousePressEvent(QMouseEvent* event)
      {
      startDrag = event->pos();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ScoreView::mouseMoveEvent(QMouseEvent* event)
      {
      int dx = event->pos().x() - startDrag.x();
      int dy = event->pos().y() - startDrag.y();
      startDrag = event->pos();
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }

 //---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void ScoreView::zoom(int step, const QPoint& pos)
      {
      QPointF p1 = imatrix.map(QPointF(pos));
      //
      //    magnify
      //
      qreal mag = _matrix.m11();
      qreal omag = mag;
      if (step > 0) {
            for (int i = 0; i < step; ++i) {
                  mag *= 1.1;
                  }
            }
      else {
            for (int i = 0; i < -step; ++i) {
                  mag /= 1.1;
                  }
            }
      if (mag > 16.0)
            mag = 16.0;
      else if (mag < 0.05)
            mag = 0.05;

      qreal deltamag = mag / omag;
      _matrix.setMatrix(mag, _matrix.m12(), _matrix.m13(), _matrix.m21(),
         mag, _matrix.m23(), _matrix.dx()*deltamag, _matrix.dy()+deltamag, _matrix.m33());
      imatrix = _matrix.inverted();

      QPointF p2 = imatrix.map(QPointF(pos));
      QPointF p3 = p2 - p1;
      int dx     = lrint(p3.x() * mag);
      int dy     = lrint(p3.y() * mag);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }


