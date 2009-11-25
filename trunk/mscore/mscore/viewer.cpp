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

/**
 \file
 Implementation of class Viewer.
*/

#include "viewer.h"
#include "score.h"
#include "element.h"
#include "magbox.h"
#include "page.h"
#include "note.h"
#include "chord.h"
#include "rest.h"
#include "segment.h"
#include "lyrics.h"
#include "harmony.h"
#include "measure.h"
#include "system.h"

//---------------------------------------------------------
//   Viewer
//---------------------------------------------------------

Viewer::Viewer(QWidget* parent)
   : QWidget(parent)
      {
      _score     = 0;
      dropTarget = 0;
      _editText  = 0;
      _matrix    = QMatrix(PDPI/DPI, 0.0, 0.0, PDPI/DPI, 0.0, 0.0);
      imatrix    = _matrix.inverted();
      _magIdx    = MAG_100;
      };

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void Viewer::setDropTarget(const Element* el)
      {
      if (dropTarget != el) {
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  _score->addRefresh(dropTarget->abbox());
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  _score->addRefresh(dropTarget->abbox());
                  }
            }
      if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      }

//---------------------------------------------------------
//   setDropRectangle
//---------------------------------------------------------

void Viewer::setDropRectangle(const QRectF& r)
      {
      if (dropRectangle.isValid())
            _score->addRefresh(dropRectangle);
      dropRectangle = r;
      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->abbox());
            dropTarget = 0;
            }
      else if (!dropAnchor.isNull()) {
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            _score->addRefresh(r.normalized());
            dropAnchor = QLineF();
            }
      _score->addRefresh(r);
      }

//---------------------------------------------------------
//   setDropAnchor
//---------------------------------------------------------

void Viewer::setDropAnchor(const QLineF& l)
      {
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->abbox());
            dropTarget = 0;
            }
      if (dropRectangle.isValid()) {
            _score->addRefresh(dropRectangle);
            dropRectangle = QRectF();
            }
      dropAnchor = l;
      if (!dropAnchor.isNull()) {
            qreal w = 2 / _matrix.m11();
            QRectF r;
            r.setTopLeft(dropAnchor.p1());
            r.setBottomRight(dropAnchor.p2());
            r = r.normalized();
            r.adjust(-w, -w, 2*w, 2*w);
            _score->addRefresh(r);
            }
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Viewer::mag() const
      {
      return _matrix.m11() *  DPI/PDPI;
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void Viewer::setOffset(qreal x, qreal y)
      {
      double m = PDPI / DPI;
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), x * m, y * m);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal Viewer::xoffset() const
      {
      return _matrix.dx() * DPI / PDPI;
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal Viewer::yoffset() const
      {
      return _matrix.dy() * DPI / PDPI;
      }

//---------------------------------------------------------
//   setMagIdx
//---------------------------------------------------------

void Viewer::setMag(int idx, double mag)
      {
      _magIdx = idx;
      setMag(mag);
      update();
      // TODO? updateNavigator(false);
      }

//---------------------------------------------------------
//   fsize
//---------------------------------------------------------

QSizeF Viewer::fsize() const
      {
      QSize s = size();
      return QSizeF(s.width() * imatrix.m11(), s.height() * imatrix.m22());
      }

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void Viewer::pageNext()
      {
      if (score()->pages().empty())
            return;

      Page* page = score()->pages().back();
      qreal x    = xoffset() - (page->width() + 25.0) * mag();
      qreal lx   = 10.0 - page->canvasPos().x() * mag();
      if (x < lx)
            x = lx;
      setOffset(x, 10.0);
//      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   pagePrev
//---------------------------------------------------------

void Viewer::pagePrev()
      {
      if (score()->pages().empty())
            return;
      Page* page = score()->pages().back();
      qreal x = xoffset() +( page->width() + 25.0) * mag();
      if (x > 10.0)
            x = 10.0;
      setOffset(x, 10.0);
//      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   pageTop
//---------------------------------------------------------

void Viewer::pageTop()
      {
      setOffset(10.0, 10.0);
//      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   pageEnd
//---------------------------------------------------------

void Viewer::pageEnd()
      {
      if (score()->pages().empty())
            return;
      Page* lastPage = score()->pages().back();
      QPointF p(lastPage->canvasPos());
      setOffset(25.0 - p.x() * mag(), 25.0);
//      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void Viewer::adjustCanvasPosition(Element* el, bool playBack)
      {
      Measure* m;
      if (el->type() == NOTE)
            m = static_cast<Note*>(el)->chord()->segment()->measure();
      else if (el->type() == REST)
            m = static_cast<Rest*>(el)->segment()->measure();
      else if (el->type() == CHORD)
            m = static_cast<Chord*>(el)->segment()->measure();
      else if (el->type() == SEGMENT)
            m = static_cast<Segment*>(el)->measure();
      else if (el->type() == LYRICS)
            m = static_cast<Lyrics*>(el)->measure();
      else if (el->type() == HARMONY)
            m = static_cast<Harmony*>(el)->measure();
      else if (el->type() == MEASURE)
            m = static_cast<Measure*>(el);
      else
            return;

      System* sys = m->system();

      QPointF p(el->canvasPos());
      QRectF r(imatrix.mapRect(geometry()));
      QRectF mRect(m->abbox());
      QRectF sysRect(sys->abbox());

      double _spatium = score()->spatium();
      const qreal BORDER_X = _spatium * 3;
      const qreal BORDER_Y = _spatium * 3;

      // only try to track measure if not during playback
      if (!playBack)
            sysRect = mRect;
      qreal top = sysRect.top() - BORDER_Y;
      qreal bottom = sysRect.bottom() + BORDER_Y;
      qreal left = mRect.left() - BORDER_X;
      qreal right = mRect.right() + BORDER_X;

      QRectF showRect(left, top, right - left, bottom - top);

      // canvas is not as wide as measure, track note instead
      if (r.width() < showRect.width()) {
            showRect.setX(p.x());
            showRect.setWidth(el->width());
            }

      // canvas is not as tall as system
      if (r.height() < showRect.height()) {
            if (sys->staves()->size() == 1 || !playBack) {
                  // track note if single staff
                  showRect.setY(p.y());
                  showRect.setHeight(el->height());
                  }
            else {
                  // let user control height
//                   showRect.setY(r.y());
//                   showRect.setHeight(1);
                  }
            }

      if (r.contains(showRect))
            return;

//       qDebug() << "showRect" << showRect << "\tcanvas" << r;

      qreal x   = - xoffset() / mag();
      qreal y   = - yoffset() / mag();

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left()) {
//             qDebug() << "left < r.left";
            x = showRect.left() - BORDER_X;
            }
      else if (showRect.left() > r.right()) {
//             qDebug() << "left > r.right";
            x = showRect.right() - width() / mag() + BORDER_X;
            }
      else if (r.width() >= showRect.width() && showRect.right() > r.right()) {
//             qDebug() << "r.width >= width && right > r.right";
            x = showRect.left() - BORDER_X;
            }
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom()) {
//             qDebug() << "top < r.top";
            y = showRect.top() - BORDER_Y;
            }
      else if (showRect.top() > r.bottom()) {
//             qDebug() << "top > r.bottom";
            y = showRect.bottom() - height() / mag() + BORDER_Y;
            }
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom()) {
//             qDebug() << "r.height >= height && bottom > r.bottom";
            y = showRect.top() - BORDER_Y;
            }

      // align to page borders if extends beyond
      Page* page = sys->page();
      if (x < page->x() || r.width() >= page->width())
            x = page->x();
      else if (r.width() < page->width() && r.width() + x > page->width() + page->x())
            x = (page->width() + page->x()) - r.width();
      if (y < page->y() || r.height() >= page->height())
            y = page->y();
      else if (r.height() < page->height() && r.height() + y > page->height())
            y = (page->height() + page->y()) - r.height();

      // hack: don't update if we haven't changed the offset
      if (oldX == x && oldY == y)
            return;

      setOffset(-x * mag(), -y * mag());
//      updateNavigator(false);
      update();
      }
