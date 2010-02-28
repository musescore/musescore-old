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
#include "omrview.h"
#include "omr.h"
#include "page.h"

//---------------------------------------------------------
//   OmrView
//---------------------------------------------------------

OmrView::OmrView(QWidget* parent)
   : QWidget(parent)
      {
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setMouseTracking(true);

      _omr   = 0;
      double m = .25;
      _matrix = QTransform(m, 0.0, 0.0, m, 0.0, 0.0);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   nextPage
//---------------------------------------------------------

void OmrView::nextPage()
      {
      gotoPage(curPage + 2);
      }

//---------------------------------------------------------
//   previousPage
//---------------------------------------------------------

void OmrView::previousPage()
      {
      gotoPage(curPage);
      }

//---------------------------------------------------------
//   setOmr
//---------------------------------------------------------

void OmrView::setOmr(Omr* s)
      {
      delete _omr;
      _omr = s;
      curPage = -1;
      gotoPage(1);
      }

//---------------------------------------------------------
//   gotoPage
//    page number n is counting from 1
//---------------------------------------------------------

void OmrView::gotoPage(int n)
      {
      if (n < 1)
            n = 1;
      if (n > _omr->numPages()) {
            n = _omr->numPages();
            }
      if ((curPage + 1) == n)
            return;
      curPage    = n - 1;
      Page* page = _omr->page(curPage);
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

void OmrView::paintEvent(QPaintEvent* event)
      {
      if (_omr == 0)
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
      Page* page = _omr->page(curPage);

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

void OmrView::mousePressEvent(QMouseEvent* e)
      {
      startDrag = e->pos();
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void OmrView::mouseMoveEvent(QMouseEvent* e)
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

void OmrView::setMag(double nmag)
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

void OmrView::zoom(int step, const QPoint& pos)
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

void OmrView::wheelEvent(QWheelEvent* event)
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

//---------------------------------------------------------
//   setScale
//---------------------------------------------------------

void OmrView::setScale(double v)
      {
      double spatium = _omr->spatium();
      setMag(v/spatium);
      update();
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void OmrView::setOffset(double x, double y)
      {
      double spatium = _omr->spatium() * _matrix.m11();
      double nx = x*spatium + (_omr->page(curPage)->width() * _matrix.m11() * curPage);
      double ny = y*spatium;
      double ox = _matrix.dx();
      double oy = _matrix.dy();
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), nx, ny, _matrix.m33());
      imatrix = _matrix.inverted();

      scroll(ox-nx, oy-ny, QRect(0, 0, width(), height()));
      update();
      }

