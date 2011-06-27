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

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QDeclarativeItem* parent)
   : QDeclarativeItem(parent)
      {
      setFlag(QGraphicsItem::ItemHasNoContents, false);
      setSmooth(true);
      score = 0;
      loadFile("test1.mscx");
      }

//---------------------------------------------------------
//   loadFile
//---------------------------------------------------------

void ScoreView::loadFile(const QString& name)
      {
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

//      _matrix = QTransform();
//      _matrix.scale(2.0, 2.0);
//      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void ScoreView::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      PainterQt p(painter, this);

      QRectF fr(boundingRect());
      painter->setClipRect(fr);
      painter->setClipping(true);
      painter->setRenderHint(QPainter::Antialiasing, true);
      painter->setRenderHint(QPainter::TextAntialiasing, true);

      painter->translate(_offset);
      fr.translate(-_offset);

      foreach(Page* page, score->pages()) {
            QRectF pr(page->abbox());
            if (pr.right() < fr.left())
                  continue;
            if (pr.left() > fr.right())
                  break;
            QList<const Element*> ell = page->items(fr);
            qStableSort(ell.begin(), ell.end(), elementLessThan);

            foreach(const Element* e, ell) {
                  e->itemDiscovered = 0;
                  painter->save();
                  painter->translate(e->canvasPos());
                  painter->setPen(QPen(e->curColor()));
                  e->draw(&p);
                  painter->restore();
                  }
            }
      }

#if 0
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
#endif

//---------------------------------------------------------
//   scroll
//---------------------------------------------------------

void ScoreView::drag(qreal x, qreal y)
      {
      qreal dx = x - _startDrag.x();
      qreal dy = y - _startDrag.y();
      startDrag(x, y);
      _offset += QPointF(dx, dy);
      scroll(dx, dy, QRect(0, 0, width(), height()));
      update();
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag(qreal x, qreal y)
      {
      _startDrag.setX(x);
      _startDrag.setY(y);
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void ScoreView::zoom(int step, const QPoint& pos)
      {
#if 0
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
#endif
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

void ScoreView::setScore(Score*)
      {
      }

void ScoreView::removeScore()
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
      QTransform t;
      return t; // _matrix;
      }

void ScoreView::setDropRectangle(const QRectF&)
      {
      }

void ScoreView::cmdAddSlur(Note*, Note*)
      {
      }

void ScoreView::startEdit()
      {
      }

void ScoreView::startEdit(Element*, int)
      {
      }

Element* ScoreView::elementNear(const QPointF&)
      {
      return 0;
      }


