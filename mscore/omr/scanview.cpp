//=============================================================================
//  MusE Reader
//  Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#include "globals.h"
#include "scanview.h"
#include "scan.h"
#include "page.h"

//---------------------------------------------------------
//   ScanView
//---------------------------------------------------------

ScanView::ScanView(QWidget* parent)
   : QWidget(parent)
      {
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setMouseTracking(true);

      _scan   = 0;
      double m = .25;
      _matrix = QTransform(m, 0.0, 0.0, m, 0.0, 0.0);
      imatrix = _matrix.inverted();

      QAction* a = new QAction(tr("move to next page"), this);
      a->setShortcut(QKeySequence::MoveToNextPage);
      connect(a, SIGNAL(triggered()), SLOT(nextPage()));
      addAction(a);

      a = new QAction(tr("move to previous page"), this);
      a->setShortcut(QKeySequence::MoveToPreviousPage);
      connect(a, SIGNAL(triggered()), SLOT(previousPage()));
      addAction(a);
      }

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void ScanView::nextPage()
      {
      gotoPage(curPage + 2);
      }

//---------------------------------------------------------
//   previousPage
//---------------------------------------------------------

void ScanView::previousPage()
      {
      gotoPage(curPage);
      }

//---------------------------------------------------------
//   setScan
//---------------------------------------------------------

void ScanView::setScan(Scan* s)
      {
      delete _scan;
      _scan = s;
      curPage = -1;
      gotoPage(1);
      }

//---------------------------------------------------------
//   gotoPage
//    page number n is counting from 1
//---------------------------------------------------------

void ScanView::gotoPage(int n)
      {
      if (n < 1)
            n = 1;
      if (n > _scan->numPages()) {
            n = _scan->numPages();
            }
      if ((curPage + 1) == n)
            return;
      curPage    = n - 1;
      Page* page = _scan->page(curPage);
      const QImage& i = page->image();
      int w = i.width();
      int h = i.height();
      //
      // image size is limited in opengl renderer, so split image
      // into four tiles:
      //
      //    0  1
      //    3  2
      //
      pm[0] = QPixmap::fromImage(i.copy(0,   0,       w/2,     h/2));
      pm[1] = QPixmap::fromImage(i.copy(w/2, 0,   w - w/2,     h/2));
      pm[2] = QPixmap::fromImage(i.copy(w/2, h/2,   w - w/2, h - h/2));
      pm[3] = QPixmap::fromImage(i.copy(0,   h/2,       w/2, h - h/2));

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), 0.0, 0.0, _matrix.m33());
      imatrix = _matrix.inverted();

      update();
      emit pageNumberChanged(curPage + 1);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ScanView::paintEvent(QPaintEvent* event)
      {
      if (_scan == 0)
            return;

      QPainter p(this);
      p.setTransform(_matrix);
      p.setRenderHint(QPainter::SmoothPixmapTransform, true);
      p.setRenderHint(QPainter::HighQualityAntialiasing, true);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      p.setRenderHint(QPainter::NonCosmeticDefaultPen, true);

      const QVector<QRect>& vector = event->region().rects();
      foreach(const QRect& r, vector) {
            QRectF rr = imatrix.mapRect(QRectF(r));
            QRectF pr(pm[0].rect());
            if (rr.intersects(pr))
                  p.drawPixmap(0, 0, pm[0]);
            if (rr.intersects(pr.translated(pr.width(), 0.0)))
                  p.drawPixmap(pr.width(), 0.0, pm[1]);
            if (rr.intersects(pr.translated(pr.width(), pr.height())))
                  p.drawPixmap(pr.width(), pr.height(), pm[2]);
            if (rr.intersects(pr.translated(0.0, pr.height())))
                  p.drawPixmap(0.0, pr.height(), pm[3]);
            }
      Page* page = _scan->page(curPage);

      if (debugMode == 1) {
            p.setPen(QPen(QColor(255, 0, 0, 80), 1.0));
            foreach(QLine l, page->sl())
                  p.drawLine(QLineF(l.x1()+.5, l.y1()+.5, l.x2()+.5, l.y2()+.5));
            }
      foreach(const QRect r, page->slices())
            p.fillRect(r, QBrush(QColor(0, 100, 100, 50)));

      p.setPen(QPen(QColor(255, 0, 0), 3.0));
      foreach(const QRect r, page->notes())
            p.drawRect(r);

      foreach(const QRectF& r, page->r())       // staves
            p.fillRect(r, QBrush(QColor(0, 0, 100, 50)));
      p.setPen(QPen(Qt::blue));
      if (debugMode == 1) {
            foreach(const QLineF& l, page->bl()) {
                  p.drawLine(l);
                  }
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void ScanView::mousePressEvent(QMouseEvent* e)
      {
      startDrag = e->pos();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ScanView::mouseMoveEvent(QMouseEvent* e)
      {
      if (QApplication::mouseButtons()) {
            QPoint delta = e->pos() - startDrag;
            int dx       = delta.x();
            int dy       = delta.y();
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
               _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
            imatrix = _matrix.inverted();

            scroll(dx, dy, QRect(0, 0, width(), height()));
            startDrag = e->pos();
            }
      QPoint pt = imatrix.map(QPointF(e->pos())).toPoint();
      emit xPosChanged(pt.x());
      emit yPosChanged(pt.y());
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void ScanView::setMag(double nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m13(), _matrix.m21(),
         nmag, _matrix.m23(), _matrix.dx()*deltamag, _matrix.dy()*deltamag, _matrix.m33());
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void ScanView::zoom(int step, const QPoint& pos)
      {
      QPointF p1 = imatrix.map(QPointF(pos));
      double _scale = mag();
      if (step > 0) {
            for (int i = 0; i < step; ++i)
                   _scale *= 1.1;
            }
      else {
            for (int i = 0; i < -step; ++i)
                  _scale /= 1.1;
            }
      if (_scale > 16.0)
            _scale = 16.0;
      else if (_scale < 0.05)
            _scale = 0.05;
      setMag(_scale);

      QPointF p2 = imatrix.map(QPointF(pos));
      QPointF p3 = p2 - p1;
      int dx     = lrint(p3.x() * _scale);
      int dy     = lrint(p3.y() * _scale);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ScanView::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            QApplication::sendPostedEvents(this, 0);
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
      }

