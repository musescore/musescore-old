//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: canvas.cpp,v 1.80 2006/09/15 09:34:57 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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
 Implementation of most part of class Canvas.
*/

#include "canvas.h"
#include "score.h"
#include "preferences.h"
#include "utils.h"
#include "segment.h"
#include "mscore.h"
#include "seq.h"
#include "staff.h"
#include "navigator.h"
#include "chord.h"
#include "page.h"
#include "xml.h"
#include "text.h"
#include "note.h"
#include "layout.h"
#include "dynamics.h"
#include "pedal.h"
#include "volta.h"
#include "ottava.h"
#include "trill.h"
#include "hairpin.h"

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

Canvas::Canvas(QWidget* parent)
   : QWidget(parent)
      {
      setAcceptDrops(true);
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);

      navigator        = 0;
      _score           = 0;
      dragCanvasState  = false;
      _bgColor         = Qt::darkBlue;
      _fgColor         = Qt::white;
      fgPixmap         = 0;
      bgPixmap         = 0;
      cursorIsBlinking = preferences.cursorBlink;
      lasso            = new Lasso(_score);

      setXoffset(30);
      setYoffset(30);

      state            = NORMAL;
      cursor           = new Cursor(_score, 6);
      shadowNote       = new ShadowNote(_score);
      cursorTimer      = new QTimer(this);
      buttonState      = 0;
      keyState         = 0;
      mousePressed     = false;

      connect(cursorTimer, SIGNAL(timeout()), SLOT(cursorBlink()));
      cursorTimer->start(500);
      cursor->setVisible(false);
      shadowNote->setVisible(false);

      if (debugMode)
            setMouseTracking(true);
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool Canvas::event(QEvent* ev)
      {
      if (ev->type() == QEvent::KeyPress) {
            QKeyEvent* ke = (QKeyEvent*) ev;
            if ((state == EDIT) && (ke->key() == Qt::Key_Tab)) {
                  keyPressEvent(ke);
                  return true;
                  }
            }
      return QWidget::event(ev);
      }

//---------------------------------------------------------
//   cursorBlink
//---------------------------------------------------------

void Canvas::cursorBlink()
      {
      bool update = false;
      if (preferences.cursorBlink != cursorIsBlinking) {
            cursorIsBlinking = preferences.cursorBlink;
            if (!cursorIsBlinking) {
                  cursor->noBlinking();
                  update = true;
                  }
            }
      if (cursorIsBlinking) {
            cursor->blink();
            update = true;
            }
      if (update)
            redraw(cursor->abbox());
      }

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

Canvas::~Canvas()
      {
      delete lasso;
      delete cursor;
      delete shadowNote;
      }

//---------------------------------------------------------
//   cavasPopup
//---------------------------------------------------------

void Canvas::canvasPopup(const QPoint& pos)
      {
      QMenu* popup = mscore->genCreateMenu();
      popup->popup(pos);
      setState(NORMAL);
      }

//---------------------------------------------------------
//   objectPopup
//    the menu can be extended by Elements with
//      genPropertyMenu()/propertyAction() methods
//---------------------------------------------------------

void Canvas::objectPopup(const QPoint& pos, Element* obj)
      {
      // show tuplet properties if number is clicked:
      if (obj->type() == TEXT && obj->subtype() == TEXT_TUPLET) {
            obj = obj->parent();
            if (!obj->selected())
                  obj->score()->select(obj, 0, 0);
            }

      QMenu* popup = new QMenu(this);

      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();

      QAction* a;
      if (obj->visible())
            a = popup->addAction(tr("Set Invisible"));
      else
            a = popup->addAction(tr("Set Visible"));
      a->setData("invisible");
      a = popup->addAction(tr("Color..."));
      a->setData("color");
      popup->addSeparator();
      if (obj->genPropertyMenu(popup))
            popup->addSeparator();
      a = popup->addAction(tr("Properties"));
      a->setData("props");
      a = popup->exec(pos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "props")
            mscore->showElementContext(obj);
      else if (cmd == "invisible")
            _score->toggleInvisible(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
      else
            obj->propertyAction(cmd);
      _score->endCmd(true);
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void Canvas::resizeEvent(QResizeEvent*)
      {
      if (navigator) {
            navigator->move(0, height() - navigator->height());
            updateNavigator(false);
            }
      }

//---------------------------------------------------------
//   updateNavigator
//---------------------------------------------------------

void Canvas::updateNavigator(bool layoutChanged) const
      {
      if (navigator) {
            if (layoutChanged)
                  navigator->layoutChanged();
            QRectF r(0.0, 0.0, width(), height());
            navigator->setViewRect(imatrix.mapRect(r));
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Canvas::mousePressEvent(QMouseEvent* ev)
      {
      mousePressed = true;

      bool b1 = ev->button() == Qt::LeftButton;
      bool b2 = ev->button() == Qt::MidButton;
      bool b3 = ev->button() == Qt::RightButton;

      keyState = ev->modifiers();
      if (state == MAG) {
            if (b1)
                  mscore->incMag();
            else if (b3)
                  mscore->decMag();
            return;
            }

      buttonState = ev->button();
      startMove   = imatrix.map(QPointF(ev->pos()));

      Element* element = elementAt(startMove);
      _score->setDragObject(element);

      if (seq && mscore->playEnabled() && element && element->type() == NOTE) {
            Note* note = (Note*)element;
            Staff* staff = note->staff();
            seq->startNote(staff->midiChannel(), note->pitch(), 60);
            }

      //-----------------------------------------
      //  context menus
      //-----------------------------------------

      if (b3) {
            if (element) {
//                  if (!element->selected() && _score->sel->state != SEL_STAFF && _score->sel->state != SEL_SYSTEM) {
                        ElementType type = element->type();
                        _score->dragStaff = 0;  // WS
                        if (type == MEASURE) {
                              _score->dragSystem = (System*)(element->parent());
                              _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                              }
                        // As findSelectableElement may return a measure
                        // when clicked "a little bit" above or below it, getStaff
                        // may not find the staff and return -1, which would cause
                        // select() to crash
                        if (_score->dragStaff >= 0)
                              _score->select(element, keyState, _score->dragStaff);
//                        else
                              _score->setDragObject(0);
//                        }
                  seq->stopNotes(); // stop now because we dont get a mouseRelease event
                  objectPopup(ev->globalPos(), element);
                  }
            else {
                  canvasPopup(ev->globalPos());
                  }
            return;
            }

      if (state != EDIT)
            _score->startCmd();
      switch (state) {
            case NOTE_ENTRY:
                  if (!(keyState & Qt::AltModifier))  // Alt+move = drag canvas
                        _score->putNote(startMove, keyState & Qt::ShiftModifier);
                  break;

            case NORMAL:
                  //-----------------------------------------
                  //  drag operation
                  //-----------------------------------------

                  if (b2 && (_score->sel->state == SEL_SINGLE)) {
#if 0
                        QDrag* drag = new QDrag(this);
                        QMimeData* mimeData = new QMimeData;
                        Element* el = symbols[currentSymbol];

                        mimeData->setData("application/mscore/symbol", el->mimeData());
                        drag->setMimeData(mimeData);

                        /*Qt::DropAction dropAction =*/ drag->start(Qt::CopyAction);
#endif
                        break;
                        }

                  //-----------------------------------------
                  //  select operation
                  //-----------------------------------------

                  if (element) {
                        ElementType type = element->type();
                        _score->dragStaff = 0;  // WS
                        if (type == MEASURE) {
                              _score->dragSystem = (System*)(element->parent());
                              _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                              }
                        // As findSelectableElement may return a measure
                        // when clicked "a little bit" above or below it, getStaff
                        // may not find the staff and return -1, which would cause
                        // select() to crash
                        if (_score->dragStaff >= 0)
                              _score->select(element, keyState, _score->dragStaff);
                        else
                              _score->setDragObject(0);
                        }
                  break;

            case EDIT:
                  if (_score->editObject->startEditDrag(this, startMove - _score->editObject->canvasPos())) {
                        setState(DRAG_EDIT);
                        update();
                        }
                  else if (_score->editObject->mousePress(startMove))
                        update();
                  else {
                        setState(NORMAL);
                        }
                  break;

            default:
                  break;
            }
      }

//---------------------------------------------------------
//   mouseDoubleClickEvent
//---------------------------------------------------------

void Canvas::mouseDoubleClickEvent(QMouseEvent* ev)
      {
      if (state == EDIT)
            return;
      Element* element = _score->dragObject();
      if (element) {
            _score->startCmd();
            if (!startEdit(element))
                  _score->endCmd(true);
            }
      else
            mousePressEvent(ev);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Canvas::mouseMoveEvent(QMouseEvent* ev)
      {
      if (buttonState == Qt::MidButton) {
            const char* mimeType = _score->sel->mimeType();
            if (mimeType) {
                  QDrag* drag = new QDrag(this);
                  QMimeData* mimeData = new QMimeData;
                  mimeData->setData(mimeType, _score->sel->mimeData());
                  drag->setMimeData(mimeData);
                  _score->endCmd(true);
                  drag->start(Qt::CopyAction);
                  return;
                  }
            }
      mouseMoveEvent1(ev);
      if (dragCanvasState)
           ;
//      else if (state == LASSO || state == DRAG_EDIT)
      else if (state == LASSO)
            _score->end1();
      else
            _score->endCmd(false);      // update display but dont end undo
      }

//---------------------------------------------------------
//   mouseMoveEvent1
//---------------------------------------------------------

void Canvas::mouseMoveEvent1(QMouseEvent* ev)
      {
      QPointF p = imatrix.map(QPointF(ev->pos()));
      if (state == NOTE_ENTRY) {
            _score->addRefresh(shadowNote->abbox());
            setShadowNote(p);
            _score->addRefresh(shadowNote->abbox());
            }

      if (!_score->dragObject() && !(keyState & Qt::ShiftModifier) && buttonState) {
            dragCanvasState = true;
            setCursor(Qt::SizeAllCursor);
            }

      QPointF delta = p - startMove;

      if (dragCanvasState) {
            QPoint d = ev->pos() - _matrix.map(startMove).toPoint();
            int dx   = d.x();
            int dy   = d.y();
            QApplication::sendPostedEvents(this, 0);

            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
               _matrix.m22(), _matrix.dx()+dx, _matrix.dy()+dy);
            imatrix = _matrix.inverted();
            scroll(dx, dy, QRect(0, 0, width(), height()));

            //
            // this is necessary at least for qt4.1:
            //
            if ((dx > 0 || dy < 0) && navigator->isVisible()) {
	            QRect r(navigator->geometry());
            	r.translate(dx, dy);
            	update(r);
                  }

            updateNavigator(false);
            return;
            }
      switch (state) {
            case NORMAL:
                  if (buttonState == 0)    // debug
                        return;
                  if (sqrt(pow(delta.x(),2)+pow(delta.y(),2)) * _matrix.m11() <= 2.0)
                        return;
                  {
                  Element* de = _score->dragObject();
                  if (de && keyState == Qt::ShiftModifier) {
                        QDrag* drag = new QDrag(this);
                        QMimeData* mimeData = new QMimeData;
                        QPointF rpos(startMove - de->abbox().topLeft());
// printf("drag %s: diff %f %f\n", de->name(), rpos.x(), rpos.y());
                        mimeData->setData("application/mscore/symbol", de->mimeData(rpos));
                        drag->setMimeData(mimeData);
                        _score->endCmd(true);
                        drag->start(Qt::CopyAction);
                        break;
                        }
                  if (de && de->isMovable()) {
                        QPointF o;
                        if (_score->sel->state == SEL_STAFF || _score->sel->state == SEL_SYSTEM) {
                              double s(_score->dragSystem->distance(_score->dragStaff));
                              o = QPointF(0.0, mag() * s);
                              setState(DRAG_STAFF);
                              }
                        else {
                              _score->startDrag();
                              o = QPointF(de->userOff() * _spatium);
                              setState(DRAG_OBJ);
                              }
                        startMove -= o;
                        break;
                        }
                  if (keyState & Qt::ShiftModifier)
                        setState(LASSO);
                  else {
                        dragCanvasState = true;
                        setCursor(QCursor(Qt::SizeAllCursor));
                        return;
                        }
                  }
                  break;
            case LASSO:
                  _score->addRefresh(lasso->abbox());
                  {
                  QRectF r;
                  r.setCoords(startMove.x(), startMove.y(), p.x(), p.y());
                  lasso->setbbox(r);
                  }
                  _score->addRefresh(lasso->abbox());
                  lassoSelect();
                  break;

            case EDIT:
                  break;

            case DRAG_EDIT:
                  _score->dragEdit(this, &startMove, delta);
                  startMove += delta;
                  break;

            case DRAG_STAFF:
//                  _score->dragSystem->setDistance(_score->dragStaff, delta.y());
//                  _score->layout1();   // DEBUG: does not work
                  break;

            case DRAG_OBJ:
                  _score->drag(delta);
                  break;

            case NOTE_ENTRY:
            case MAG:
                  break;
            }
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Canvas::mouseReleaseEvent(QMouseEvent* ev)
      {
      if (!mousePressed) {
            //
            // this happens if a pulldown menu is pressed and the
            // mouse release happens on the canvas
            //
            if (debugMode)
                  printf("...spurious mouse release\n");
            return;
            }
      mousePressed = false;

      seq->stopNotes();
      if (state == EDIT)
            return;
      if (state == MAG) {
            if (keyState & Qt::ShiftModifier)
                  return;
            setState(NORMAL);
            return;
            }
      mouseReleaseEvent1(ev);
      // here we can be in state EDIT again
      if (state != EDIT)
            _score->endCmd(true);
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Canvas::mouseReleaseEvent1(QMouseEvent* /*ev*/)
      {
      buttonState = 0;
      if (dragCanvasState) {
            dragCanvasState = false;
            setState(state);        // reset cursor pixmap
            return;
            }

      switch (state) {
            case DRAG_EDIT:
                  _score->addRefresh(_score->editObject->abbox());
                  _score->editObject->endEditDrag();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  _score->addRefresh(_score->editObject->abbox());
                  setState(EDIT);
                  _score->endCmd(false);
                  break;

            case LASSO:
                  setState(NORMAL);
                  _score->addRefresh(lasso->abbox().adjusted(-2, -2, 2, 2));
                  lasso->setbbox(QRectF());
                  break;

            case DRAG_OBJ:
                  setState(NORMAL);
                  _score->endDrag();
                  break;

            case DRAG_STAFF:
                  _score->layout();
                  setState(NORMAL);
                  break;

            default:
                  setState(NORMAL);

            case NOTE_ENTRY:
                  break;

            case NORMAL:
                  if (!_score->dragObject())
                        _score->select(0, 0, 0);      // deselect all
                  break;
            }
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Canvas::setMag(double nmag)
      {
      qreal m = mag();
      if (nmag == m)
            return;
      double deltamag = nmag / m;;
      setXoffset(xoffset() * deltamag);
      setYoffset(yoffset() * deltamag);

      m = nmag;
      _matrix.setMatrix(m, _matrix.m12(), _matrix.m21(),
         m * qreal(appDpiY)/qreal(appDpiX), _matrix.dx(), _matrix.dy());
      imatrix = _matrix.inverted();

      update();
      updateNavigator(false);
      }

//---------------------------------------------------------
//   setBackground
//---------------------------------------------------------

void Canvas::setBackground(QPixmap* pm)
      {
      if (bgPixmap)
            delete bgPixmap;
      bgPixmap = pm;
      update();
      }

void Canvas::setBackground(const QColor& color)
      {
      if (bgPixmap) {
            delete bgPixmap;
            bgPixmap = 0;
            }
      _bgColor = color;
      update();
      }

//---------------------------------------------------------
//   setForeground
//---------------------------------------------------------

void Canvas::setForeground(QPixmap* pm)
      {
      if (fgPixmap)
            delete fgPixmap;
      fgPixmap = pm;
      update();
      }

void Canvas::setForeground(const QColor& color)
      {
      if (fgPixmap) {
            delete fgPixmap;
            fgPixmap = 0;
            }
      _fgColor = color;
      update();
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Canvas::setState(State s)
      {
      switch(s) {
            case NOTE_ENTRY:
                  setCursor(QCursor(Qt::UpArrowCursor));
                  break;
            case MAG:
                  setCursor(QCursor(Qt::SizeAllCursor));
                  break;
            case EDIT:
                  mscore->setState(STATE_EDIT);

            case NORMAL:
            case DRAG_OBJ:
            case DRAG_STAFF:
            case DRAG_EDIT:
            case LASSO:
                  setCursor(QCursor(Qt::ArrowCursor));
                  break;
            }
      if (shadowNote->visible() != (s == NOTE_ENTRY)) {
            shadowNote->setVisible(s == NOTE_ENTRY);
            _score->addRefresh(shadowNote->abbox());
            _score->addRefresh(cursor->abbox());
            }
      setMouseTracking(s == NOTE_ENTRY);
      if (state == LASSO || s == LASSO)
            lasso->setVisible(s == LASSO);
      if ((state == EDIT) && (s != EDIT) && (s != DRAG_EDIT)) {
            _score->endEdit();
            }
      state = s;
      }

//---------------------------------------------------------
//   cmdCut
//---------------------------------------------------------

void Canvas::cmdCut()
      {
      }

//---------------------------------------------------------
//   cmdCopy
//---------------------------------------------------------

void Canvas::cmdCopy()
      {
      }

//---------------------------------------------------------
//   cmdPaste
//---------------------------------------------------------

void Canvas::cmdPaste()
      {
      }

//---------------------------------------------------------
//   magCanvas
//---------------------------------------------------------

void Canvas::magCanvas()
      {
      setState(MAG);
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void Canvas::dataChanged(const QRectF& r)
      {
      redraw(r);
      }

//---------------------------------------------------------
//   redraw
//---------------------------------------------------------

void Canvas::redraw(const QRectF& fr)
      {
      update(_matrix.mapRect(fr).toRect());  // generate paint event
      }

//---------------------------------------------------------
//   resetStaffOffsets
//---------------------------------------------------------

void Canvas::resetStaffOffsets()
      {
/*      for (iSystem i = _score->systems->begin(); i != _score->systems->end(); ++i) {
            for (int staff = 0; staff < _score->staves(); ++staff)
                  (*i)->staff(staff)->setUserOff(0.0);
            }
      _score->layout();
      redraw();
*/
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

bool Canvas::startEdit(Element* element)
      {
      if (element->startEdit(_matrix, startMove)) {
            setFocus();
            _score->startEdit(element);
            setState(EDIT);
            update();         // DEBUG
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   clearScore
//---------------------------------------------------------

void Canvas::clearScore()
      {
      cursor->setVisible(false);
      shadowNote->setVisible(false);
      update();
//TODO      padState.pitch = 64;
      setState(NORMAL);
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor and set visible
//    if (tick == -1) hide cursor
//---------------------------------------------------------

QRectF Canvas::moveCursor()
      {
      QRectF refresh;

      cursor->setVoice(cis->voice);
      int staff = cis->staff;
      int tick  = cis->pos;
      if (cursor->isOn()) {
            refresh |= cursor->abbox();
            }
      if (tick == -1) {
            cursor->setOn(false);
            return refresh;
            }
      if (staff == -1) {
            printf("cannot set Cursor to staff -1\n");
            return refresh;
            }
      cursor->setOn(true);
      cursor->setTick(tick);

      Measure* measure = _score->tick2measure(tick);
      if (measure) {
            Segment* segment;
            for (segment = measure->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != Segment::SegChordRest)
                        continue;
                  if (segment->tick() == tick)
                        break;
                  }
            if (segment) {
                  _score->adjustCanvasPosition(segment);
                  System* system = measure->system();
                  double x = segment->x() + measure->canvasPos().x();
                  double y = system->bboxStaff(staff).y() + system->canvasPos().y();
                  refresh |= cursor->abbox();
                  cursor->setPos(x - _spatium, y - _spatium);
                  refresh |= cursor->abbox();
                  return refresh;
                  }
            }
      printf("cursor position not found for tick %d\n", tick);
      return refresh;
      }

//---------------------------------------------------------
//   setShadowNote
//---------------------------------------------------------

void Canvas::setShadowNote(const QPointF& p)
      {
      int tick, line;
      Staff* staff;
      Segment* seg;

      Measure* m = _score->pos2measure2(p, &tick, &staff, &line, &seg);
      if (m == 0)
            return;

      System* s = m->system();
      shadowNote->setLine(line);

      double y = seg->canvasPos().y() + s->staff(staff->idx())->bbox().y();
      y += line * _spatium * .5;

      shadowNote->setPos(seg->canvasPos().x(), y);
//      shadowNote->layout();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Canvas::mag() const
      {
      return _matrix.m11();
      }

//---------------------------------------------------------
//   fsize
//---------------------------------------------------------

QSizeF Canvas::fsize() const
      {
      QSize s = size();
      return QSizeF(s.width() * imatrix.m11(), s.height() * imatrix.m22());
      }

//---------------------------------------------------------
//   paintEvent
//    Note: desktop background and paper background are not
//    scaled
//---------------------------------------------------------

static inline unsigned long long cycles()
      {
      unsigned long long rv;
      __asm__ __volatile__("rdtsc" : "=A" (rv));
      return rv;
      }

void Canvas::paintEvent(QPaintEvent* ev)
      {
// QRect r = ev->rect();
// printf("Canvas::paintEvent() %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());

// unsigned long long now = cycles();
      QRect rr;
      if (_score->needLayout()) {
// printf("paintEvent: do Layout\n");
            _score->doLayout();
            if (navigator)
                  navigator->layoutChanged();
            rr.setRect(0, 0, width(), height());  // does not work because paintEvent
                                                  // is clipped?
            paint(rr);
            }
      else {
            int dx = lrint(_matrix.m11());
            int dy = lrint(_matrix.m22());

            const QRegion& region = ev->region();
            QVector<QRect> vector = region.rects();
            foreach(QRect r, vector) {
                  // refresh a little more:
                  rr = r.adjusted(-dx, -dy, 2 * dx, 2 * dy);
                  paint(rr);
                  }
            }
//      printf("redraw cycles %lld\n", (cycles() - now) / 1000000LL);
      }

void Canvas::paint(const QRect& rr)
      {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);

      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(rr, _fgColor);
      else {
            p.drawTiledPixmap(rr, *fgPixmap, rr.topLeft()-QPoint(lrint(xoffset()), lrint(yoffset())));
            }

      p.setMatrix(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(rr));

      QRegion r1(rr);
      for (iPage ip = _layout->pages()->begin(); ip != _layout->pages()->end(); ++ip) {
            Page* page = *ip;
            page->draw(p);
            r1 -= _matrix.mapRect(page->abbox()).toRect();
            }
      p.setClipRect(fr);

      QList<Element*> ell = _layout->items(fr);
      drawElements(p, ell);

      lasso->draw(p);
      cursor->draw(p);
      shadowNote->draw(p);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0, Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }

      p.setMatrixEnabled(false);
      if (!r1.isEmpty()) {
            p.setClipRegion(r1);  // only background
            if (bgPixmap == 0 || bgPixmap->isNull())
                  p.fillRect(rr, _bgColor);
            else
                  p.drawTiledPixmap(rr, *bgPixmap, rr.topLeft()-QPoint(lrint(xoffset()), lrint(yoffset())));
            }
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Canvas::setScore(Score* s, ScoreLayout* l)
      {
      _score = s;
      Viewer::setScore(s);
      _layout = l;
      if (navigator) {
            navigator->setScore(_score);
            updateNavigator(false);
            }
      }

//---------------------------------------------------------
//   setViewRect
//---------------------------------------------------------

void Canvas::setViewRect(const QRectF& r)
      {
      QRectF rr = _matrix.mapRect(r);
      QPoint d = rr.topLeft().toPoint();
      int dx   = -d.x();
      int dy   = -d.y();
      QApplication::sendPostedEvents(this, 0);
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), _matrix.dx()+dx, _matrix.dy()+dy);
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
	//
      // this is necessary at least for qt4.1:
      //
      if ((dx > 0 || dy < 0) && navigator->isVisible()) {
		QRect r(navigator->geometry());
            r.translate(dx, dy);
            update(r);
            }
      }

//---------------------------------------------------------
//   dragMovePageElement
//---------------------------------------------------------

bool Canvas::dragTimeAnchorElement(const QPointF& pos)
      {
      Staff* staff = 0;
      Segment* seg;
      QPointF offset;
      int tick;
      Measure* m = _score->pos2measure(pos, &tick, &staff, 0, &seg, &offset);
      if (m) {
            System* s = m->system();
            int staffIdx = staff->idx();
            QRectF sb(s->staff(staffIdx)->bbox());
            sb.translate(s->pos() + s->page()->pos());
            QPointF anchor(seg->abbox().x(), sb.topLeft().y());
            setDropAnchor(QLineF(pos, anchor));
            return true;
            }
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragAboveMeasure
//---------------------------------------------------------

bool Canvas::dragAboveMeasure(const QPointF& pos)
      {
      Staff* staff = 0;
      Segment* seg;
      QPointF offset;
      int tick;
      Measure* m = _score->pos2measure(pos, &tick, &staff, 0, &seg, &offset);
      if (m) {
            System* s = m->system();
            int staffIdx = staff->idx();

            // compute rectangle of staff in measure
            QRectF rrr(s->staff(staffIdx)->bbox().translated(s->canvasPos()));
            QRectF rr(m->abbox());
            QRectF r(rr.x(), rrr.y()-rrr.height(), rr.width(), rrr.height());

            setDropRectangle(r);
            return true;
            }
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragAboveSystem
//---------------------------------------------------------

bool Canvas::dragAboveSystem(const QPointF& pos)
      {
      Staff* staff = 0;
      Segment* seg;
      QPointF offset;
      int tick;
      Measure* m = _score->pos2measure(pos, &tick, &staff, 0, &seg, &offset);
      if (m) {
            System* s = m->system();
            int staffIdx = staff->idx();
            if (staffIdx) {
                  setDropTarget(0);
                  return false;
                  }
            // compute rectangle of staff in measure
            QRectF rrr(s->staff(staffIdx)->bbox().translated(s->canvasPos()));
            QRectF rr(m->abbox());
            QRectF r(rr.x(), rrr.y()-rrr.height(), rr.width(), rrr.height());

            setDropRectangle(r);
            return true;
            }
      setDropTarget(0);
      return false;
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Canvas::dragMoveEvent(QDragMoveEvent* event)
      {
      if (!event->mimeData()->hasFormat("application/mscore/symbol"))
            return;

      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));

      QByteArray data(event->mimeData()->data("application/mscore/symbol"));
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading drag data at %d/%d: %s\n<%s>\n",
               line, column, err.toLatin1().data(), data.data());
                  return;
            }
      QDomNode node = doc.documentElement();
      QPointF dragOffset;
      int type = Element::readType(node, &dragOffset);
      switch(type) {
            case VOLTA:
                  if (dragAboveSystem(pos))
                        event->acceptProposedAction();
                  break;
            case PEDAL:
            case DYNAMIC:
            case OTTAVA:
            case TRILL:
            case HAIRPIN:
                  if (dragTimeAnchorElement(pos))
                        event->acceptProposedAction();
                  break;
            case SYMBOL:
                  {
                  Symbol* s = new Symbol(_score);
                  s->read(node);
                  if (s->anchor() == ANCHOR_TICK) {
                        if (dragTimeAnchorElement(pos))
                              event->acceptProposedAction();
                        }
                  delete s;
                  }
                  break;

            default:
                  {
                  Element* el = elementAt(pos);
                  if (el) {
                        bool accept = el->acceptDrop(this, pos, type, node);
                        if (accept)
                              event->acceptProposedAction();
                        else {
                              if (debugMode)
                                    printf("ignore drop of %s\n", elementNames[type]);
                              event->ignore();
                              setDropTarget(0);
                              }
                        }
                  else {
                        event->ignore();
                        setDropTarget(0);
                        }
                  }
                  break;
            }
      _score->end();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Canvas::dropEvent(QDropEvent* event)
      {
      if (!event->mimeData()->hasFormat("application/mscore/symbol")) {
            printf("cannot drop this object: unknown mime type\n");
            return;
            }
      QPointF pos(imatrix.map(QPointF(event->pos())));

      QByteArray data(event->mimeData()->data("application/mscore/symbol"));
      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading drag data\n");
            return;
            }
      QDomNode node = doc.documentElement();
      QPointF dragOffset;
      int type = Element::readType(node, &dragOffset);

      switch(type) {
            case VOLTA:
                  {
                  Volta* volta = new Volta(score());
                  volta->read(node);
                  score()->cmdAdd(volta, pos, dragOffset);
                  event->acceptProposedAction();
                  }
                  break;
            case OTTAVA:
                  {
                  Ottava* ottava = new Ottava(score());
                  ottava->read(node);
                  score()->cmdAdd(ottava, pos, dragOffset);
                  event->acceptProposedAction();
                  }
                  break;
            case TRILL:
                  {
                  Trill* trill = new Trill(score());
                  score()->cmdAdd(trill, pos, dragOffset);
                  event->acceptProposedAction();
                  }
                  break;
            case SYMBOL:
                  {
                  _score->startCmd();
                  Symbol* s = new Symbol(score());
                  s->read(node);
                  score()->cmdAddSymbol(s, pos, dragOffset);
                  event->acceptProposedAction();
                  score()->endCmd(true);
                  }
                  break;
            case PEDAL:
                  {
                  Pedal* pedal = new Pedal(score());
                  pedal->read(node);
                  score()->cmdAdd(pedal, pos, dragOffset);
                  event->acceptProposedAction();
                  }
                  break;
            case HAIRPIN:
                  {
                  Hairpin* hairpin = new Hairpin(score());
                  hairpin->read(node);
                  score()->cmdAdd(hairpin, pos, dragOffset);
                  }
                  break;
            case DYNAMIC:
                  {
                  Dynamic* dynamic = new Dynamic(score());
                  dynamic->read(node);
                  _score->cmdAdd(dynamic, pos, dragOffset);
                  event->acceptProposedAction();
                  }
                  break;
            default:
                  {
                  Element* el = elementAt(pos);
                  if (el) {
                        _score->startCmd();
                        _score->addRefresh(el->abbox());
                        Element* dropElement = el->drop(pos, dragOffset, type, node);
                        _score->addRefresh(el->abbox());
                        if (dropElement) {
                              _score->select(dropElement, 0, 0);
                              _score->addRefresh(dropElement->abbox());
                              }
                        event->acceptProposedAction();
                        _score->endCmd(true);
                        }
                  else
                        printf("cannot drop here\n");
                  }
                  break;
            }
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      setState(NORMAL);
      }

//---------------------------------------------------------
//   dragEnterEvent
//---------------------------------------------------------

void Canvas::dragEnterEvent(QDragEnterEvent* event)
      {
      if (event->mimeData()->hasFormat("application/mscore/symbol"))
            event->acceptProposedAction();
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void Canvas::dragLeaveEvent(QDragEnterEvent*)
      {
      if (!dropTarget)
            return;
      dropTarget->setDropTarget(false);
      _score->addRefresh(dropTarget->abbox());
      _score->end();
      dropTarget = 0;
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void Canvas::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            QApplication::sendPostedEvents(this, 0);
            QPointF p1 = imatrix.map(QPointF(event->pos()));
            //
            //    magnify
            //
            int step = event->delta() / 120;
            qreal _mag = mag();

            if (step > 0) {
                  for (int i = 0; i < step; ++i)
                        _mag *= 1.1;
                  }
            else {
                  for (int i = 0; i < -step; ++i)
                        _mag *= 0.9;
                  }
            if (_mag > 16.0)
                  _mag = 16.0;
            else if (_mag < 0.05)
                  _mag = 0.05;

            mscore->setMag(_mag);

            QPointF p2 = imatrix.map(QPointF(event->pos()));
            QPointF p3 = p2 - p1;
            int dx    = lrint(p3.x() * _mag);
            int dy    = lrint(p3.y() * _mag);

            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
               _matrix.m22(), _matrix.dx()+dx, _matrix.dy()+dy);
            imatrix = _matrix.inverted();
            scroll(dx, dy, QRect(0, 0, width(), height()));

            if ((dx > 0 || dy < 0) && navigator->isVisible()) {
	            QRect r(navigator->geometry());
            	r.translate(dx, dy);
            	update(r);
                  }
            updateNavigator(false);
            update();
            return;
            }
      int dx = 0;
      int dy = 0;
      if (event->modifiers() & Qt::ShiftModifier) {
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

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), _matrix.dx() + dx, _matrix.dy() + dy);
      imatrix = _matrix.inverted();

      scroll(dx, dy, QRect(0, 0, width(), height()));

      //
      // this is necessary at least for qt4.1:
      //
      if ((dy < 0) && navigator->isVisible()) {
		QRect r(navigator->geometry());
		r.translate(dx, dy);
		update(r);
            }
	updateNavigator(false);
      }

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      return e1->type() < e2->type();
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* Canvas::elementAt(const QPointF& p)
      {
      QList<Element*> el = _layout->items(p);
      if (el.empty())
            return 0;
      qSort(el.begin(), el.end(), elementLower);
/*      printf("elementAt: %f %f\n", p.x(), p.y());
      foreach(Element* e, el)
            printf("  %s\n", e->name());
      */
      return el.at(0);
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void Canvas::drawElements(QPainter& p,const QList<Element*>& el)
      {
      for (int i = 0; i < el.size(); ++i) {
            Element* e = el.at(i);
            e->itemDiscovered = 0;

            if (!(e->visible() || score()->showInvisible()))
                  continue;

            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->color()));
            e->draw(p);
            if (debugMode && e->selected()) {
                  //
                  //  draw bounding box rectangle for all
                  //  selected Elements
                  //
                  p.setBrush(Qt::NoBrush);
                  p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
                  // p.drawRect(e->bbox());
                  p.drawPath(e->shape());

                  Element* seg = e->parent();
                  if (e->type() == NOTE)
                        seg = seg->parent();
                  p.setPen(QPen(Qt::red, 0, Qt::SolidLine));
                  p.drawRect(seg->abbox());
                  }
            p.restore();
            }
      }

