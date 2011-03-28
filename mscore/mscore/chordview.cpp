//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer and others
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

#include "chordview.h"
#include "staff.h"
#include "piano.h"
#include "measure.h"
#include "chord.h"
#include "score.h"
#include "note.h"
#include "slur.h"
#include "segment.h"
#include "noteevent.h"

static const int MAP_OFFSET = 20;

//---------------------------------------------------------
//   pitch2y
//---------------------------------------------------------

static int pitch2y(int pitch)
      {
      return keyHeight * (128 + pitch);
      }

//---------------------------------------------------------
//   ChordItem
//---------------------------------------------------------

ChordItem::ChordItem(Note* n, NoteEvent* e)
   : QGraphicsRectItem(), note(n), event(e)
      {
      setFlags(flags() | QGraphicsItem::ItemIsSelectable);
      int pitch = e->pitch();
      int len   = e->len();
      setRect(0, 0, len, keyHeight/2);
      setBrush(QBrush());
      setSelected(n->selected());
      setData(0, QVariant::fromValue<void*>(n));

      setPos(e->ontime() + MAP_OFFSET, pitch2y(pitch) + keyHeight / 4);
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ChordItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
      {
      int len = event->len();
      int x1  = event->ontime();
      int x2  = x1 + len;
      painter->setPen(pen());
      painter->setBrush(isSelected() ? Qt::yellow : Qt::blue);
      painter->drawRect(x1, 0.0, len, keyHeight / 2);
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

AL::Pos ChordView::pix2pos(int x) const
      {
      x -= MAP_OFFSET;
      if (x < 0)
            x = 0;
      return AL::Pos(staff->score()->tempomap(), staff->score()->sigmap(), x, _timeType);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int ChordView::pos2pix(const AL::Pos& p) const
      {
      return p.time(_timeType) + MAP_OFFSET;
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void ChordView::drawBackground(QPainter* p, const QRectF& r)
      {
      if (staff == 0)
            return;
      Score* _score = staff->score();

      QRectF r1;
      r1.setCoords(-1000000.0, 0.0, MAP_OFFSET, 1000000.0);
      QRectF r2;
      r2.setCoords(ticks + MAP_OFFSET, 0.0, 1000000.0, 1000000.0);
      QColor bg(0x71, 0x8d, 0xbe);

      p->fillRect(r, bg);
      if (r.intersects(r1))
            p->fillRect(r.intersected(r1), bg.darker(150));
      if (r.intersects(r2))
            p->fillRect(r.intersected(r2), bg.darker(150));

      //
      // draw horizontal grid lines
      //
      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      int key = floor(y1 / keyHeight);
      qreal y = key * keyHeight;

      for (; key < 256; ++key, y += keyHeight) {
            if (y < y1)
                  continue;
            if (y > y2)
                  break;
            p->setPen(QPen((key % 6) == 5 ? Qt::lightGray : Qt::gray));
            p->drawLine(QLineF(x1, y, x2, y));
            }

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      for (int x = 0; x < 1000; x += 50) {
            if (x % 200) {
                  p->setPen(Qt::lightGray);
                  p->drawLine(x, y1, x, y2);
                  }
            else {
                  p->setPen(Qt::black);
                  p->drawLine(x, y1, x, y2);
                  }
            }
      }

//---------------------------------------------------------
//   ChordView
//---------------------------------------------------------

ChordView::ChordView()
   : QGraphicsView()
      {
      setScene(new QGraphicsScene);
      setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
      setResizeAnchor(QGraphicsView::AnchorUnderMouse);
      setMouseTracking(true);
      setRubberBandSelectionMode(Qt::IntersectsItemBoundingRect);
      setDragMode(QGraphicsView::RubberBandDrag);
      _timeType = AL::TICKS;
      magStep   = 2;
      staff     = 0;
      chord     = 0;
      }

//---------------------------------------------------------
//   setChord
//---------------------------------------------------------

void ChordView::setChord(Chord* c, AL::Pos* l)
      {
      static const QColor lcColors[3] = { Qt::red, Qt::blue, Qt::blue };

      chord    = c;
      staff    = chord->staff();
      _locator = l;
      Score* score = chord->score();
      pos.setContext(score->tempomap(), score->sigmap());

      scene()->blockSignals(true);

      scene()->clear();
      for (int i = 0; i < 3; ++i) {
            locatorLines[i] = new QGraphicsLineItem(QLineF(0.0, 0.0, 0.0, keyHeight * 256.0 * 5));
            QPen pen(lcColors[i]);
            pen.setWidth(2);
            locatorLines[i]->setPen(pen);
            locatorLines[i]->setZValue(1000+i);       // set stacking order
            locatorLines[i]->setFlag(QGraphicsItem::ItemIgnoresTransformations, true);
            scene()->addItem(locatorLines[i]);
            }

      foreach(Note* note, c->notes()) {
            if (!note->playEvents().isEmpty()) {
                  foreach(NoteEvent* e, note->playEvents())
                        scene()->addItem(new ChordItem(note, e));
                  }
            else {
                  scene()->addItem(new ChordItem(note, new NoteEvent));
                  }
            }

      scene()->blockSignals(false);

      Measure* lm = staff->score()->lastMeasure();
      ticks       = 1000;
      scene()->setSceneRect(0.0, 0.0, double(ticks + MAP_OFFSET * 2), keyHeight * 256);

      for (int i = 0; i < 3; ++i)
            moveLocator(i);
      //
      // move to something interesting
      //
      QList<QGraphicsItem*> items = scene()->selectedItems();
      QRectF boundingRect;
      foreach(QGraphicsItem* item, items) {
            Note* note = static_cast<Note*>(item->data(0).value<void*>());
            if (note)
                  boundingRect |= item->mapToScene(item->boundingRect()).boundingRect();
            }
      centerOn(boundingRect.center());
      }

//---------------------------------------------------------
//   moveLocator
//---------------------------------------------------------

void ChordView::moveLocator(int i)
      {
      if (_locator[i].valid()) {
            locatorLines[i]->setVisible(true);
            qreal x = qreal(pos2pix(_locator[i]));
            locatorLines[i]->setPos(QPointF(x, 0.0));
            }
      else
            locatorLines[i]->setVisible(false);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ChordView::wheelEvent(QWheelEvent* event)
      {
      int step    = event->delta() / 120;
      double xmag = transform().m11();
      double ymag = transform().m22();

      if (event->modifiers() == Qt::ControlModifier) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i) {
                        if (xmag > 10.0)
                              break;
                        scale(1.1, 1.0);
                        xmag *= 1.1;
                        }
                  }
            else {
                  for (int i = 0; i < -step; ++i) {
                        if (xmag < 0.001)
                              break;
                        scale(.9, 1.0);
                        xmag *= .9;
                        }
                  }
            emit magChanged(xmag, ymag);

            int tpix  = 1000 * xmag;
            magStep = -5;
            if (tpix <= 4000)
                  magStep = -4;
            if (tpix <= 2000)
                  magStep = -3;
            if (tpix <= 1000)
                  magStep = -2;
            if (tpix <= 500)
                  magStep = -1;
            if (tpix <= 128)
                  magStep = 0;
            if (tpix <= 64)
                  magStep = 1;
            if (tpix <= 32)
                  magStep = 2;
            if (tpix <= 16)
                  magStep = 3;
            if (tpix <= 8)
                  magStep = 4;
            if (tpix <= 4)
                  magStep = 5;
            if (tpix <= 2)
                  magStep = 6;

            //
            // if xpos <= 0, then the scene is centered
            // there is no scroll bar anymore sending
            // change signals, so we have to do it here:
            //
            double xpos = -(mapFromScene(QPointF()).x());
            if (xpos <= 0)
                  emit xposChanged(xpos);
            }
      else if (event->modifiers() == Qt::ShiftModifier) {
            QWheelEvent we(event->pos(), event->delta(), event->buttons(), 0, Qt::Horizontal);
            QGraphicsView::wheelEvent(&we);
            }
      else if (event->modifiers() == 0) {
            QGraphicsView::wheelEvent(event);
            }
      else if (event->modifiers() == (Qt::ShiftModifier | Qt::ControlModifier)) {
            if (step > 0) {
                  for (int i = 0; i < step; ++i) {
                        if (ymag > 3.0)
                              break;
                        scale(1.0, 1.1);
                        ymag *= 1.1;
                        }
                  }
            else {
                  for (int i = 0; i < -step; ++i) {
                        if (ymag < 0.4)
                              break;
                        scale(1.0, .9);
                        ymag *= .9;
                        }
                  }
            emit magChanged(xmag, ymag);
            }
      }

//---------------------------------------------------------
//   y2pitch
//---------------------------------------------------------

int ChordView::y2pitch(int y) const
      {
      return (y / keyHeight) - 128;
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void ChordView::mouseMoveEvent(QMouseEvent* event)
      {
      QPointF p(mapToScene(event->pos()));
      int pitch = y2pitch(int(p.y()));
      emit pitchChanged(pitch);
      int tick = int(p.x()) - MAP_OFFSET;
      if (tick < 0) {
            tick = 0;
            pos.setTick(tick);
            pos.setInvalid();
            }
      else
            pos.setTick(tick);
      emit posChanged(pos);
      QGraphicsView::mouseMoveEvent(event);
      }

//---------------------------------------------------------
//   leaveEvent
//---------------------------------------------------------

void ChordView::leaveEvent(QEvent* event)
      {
      emit pitchChanged(-1);
      pos.setInvalid();
      emit posChanged(pos);
      QGraphicsView::leaveEvent(event);
      }

//---------------------------------------------------------
//   ensureVisible
//---------------------------------------------------------

void ChordView::ensureVisible(int tick)
      {
      tick += MAP_OFFSET;
      QPointF pt = mapToScene(0, height() / 2);
      QGraphicsView::ensureVisible(qreal(tick), pt.y(), 240.0, 1.0);
      }

