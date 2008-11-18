//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigator.cpp,v 1.12 2006/03/02 17:08:37 wschweer Exp $
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
#include "canvas.h"
#include "score.h"
#include "layout.h"
#include "page.h"
#include "preferences.h"

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

Navigator::Navigator(QWidget* parent)
  : QFrame(parent), pm(200,100)
      {
      setAttribute(Qt::WA_NoBackground);
      setFixedSize(200, 100);

      setFrameStyle(QFrame::Box | QFrame::Raised);
      setLineWidth(2);
      _score = 0;
      moving = false;
      redraw = false;
      }

//---------------------------------------------------------
//   showNavigator
//---------------------------------------------------------

void MuseScore::showNavigator(bool visible)
      {
      if (cs)
            cs->canvas()->showNavigator(visible);
      navigatorId->setChecked(visible);
      }

void Canvas::showNavigator(bool visible)
      {
      if (navigator == 0) {
            navigator = new Navigator(this);
            navigator->move(0, height() - navigator->height());
            if (_score) {
                  navigator->setScore(_score);
                  QRectF r(0.0, 0.0, width(), height());
                  navigator->setViewRect(imatrix.mapRect(r));
                  }
            connect(navigator, SIGNAL(viewRectMoved(const QRectF&)), SLOT(setViewRect(const QRectF&)));
            }
      navigator->setShown(visible);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Navigator::setScore(Score* s)
      {
      redraw  = true;
      _score  = s;
      qreal m = (height()-10.0) / (_score->pageFormat()->height() * DPI);
      matrix.setMatrix(m, matrix.m12(), matrix.m21(), m, 5, 5);
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Navigator::paintEvent(QPaintEvent* ev)
      {
      QPainter p;
      QRect r(ev->rect());
      if (redraw) {
            redraw = false;
            QColor _fgColor(Qt::white);
            QColor _bgColor(Qt::gray);
            int dx = lrint(matrix.m11());
            int dy = lrint(matrix.m22());

            r.setRect(0, 0, width(), height());
            QRect rr(r.x()-dx, r.y()-dy, r.width()+2*dx, r.height()+2*dy);

            p.begin(&pm);
            p.setRenderHint(QPainter::Antialiasing, true);

            p.fillRect(rr, _fgColor);
            if (_score->layout()->pages().empty())
                  return;
            p.setMatrix(matrix);
            QRegion r1(rr);
            foreach(const Page* page, _score->layout()->pages()) {
                  QRectF pbbox(page->abbox());
                  r1 -= matrix.mapRect(pbbox).toRect();
                  p.translate(page->pos());
                  page->draw(p);
                  p.translate(-page->pos());
                  }

            QRectF fr = matrix.inverted().mapRect(QRectF(rr));
            QList<const Element*> ell = _score->layout()->items(fr);

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

      p.begin(this);
      p.drawPixmap(r.topLeft(), pm, r);
      QPen pen(Qt::blue, 2.0);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawRect(viewRect);
      p.end();
      QFrame::paintEvent(ev);
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Navigator::mousePressEvent(QMouseEvent* ev)
      {
      startMove = ev->pos();
      if (viewRect.contains(startMove))
            moving = true;
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Navigator::mouseMoveEvent(QMouseEvent* ev)
      {
      if (moving) {
            QPoint delta = ev->pos() - startMove;
            viewRect.translate(delta);
            startMove = ev->pos();

            setViewRect(matrix.inverted().mapRect(viewRect));

            // viewRect is now within bounds
            emit viewRectMoved(matrix.inverted().mapRect(viewRect));
            }
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Navigator::setViewRect(const QRectF& _viewRect)
      {
      viewRect  = matrix.mapRect(_viewRect).toRect();
      if (viewRect.x() < 0) {
            double dx = -viewRect.x() / matrix.m11();
            viewRect.moveLeft(0);
            if (matrix.dx() < 5.0)
                  matrix.translate(dx, 0.0);
            redraw = true;
            }
      if (viewRect.right() > width()) {
            double dx = (rect().right() - viewRect.right()) / matrix.m11();
            viewRect.moveRight(width());
            matrix.translate(dx, 0.0);
            redraw = true;
            }
      if (viewRect.y() < 0)
            viewRect.moveTop(0);
      else if (viewRect.bottom() > height())
            viewRect.moveBottom(height());
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

