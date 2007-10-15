//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: canvas.cpp,v 1.80 2006/09/15 09:34:57 wschweer Exp $
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
#include "rest.h"
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
#include "image.h"
#include "globals.h"
#include "part.h"
#include "editdrumset.h"
#include "editstaff.h"

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

Canvas::Canvas(QWidget* parent)
   : QFrame(parent)
      {
      setAcceptDrops(true);
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setAutoFillBackground(true);

      dragElement      = 0;
      navigator        = 0;
      _score           = 0;
      dragCanvasState  = false;
      _bgColor         = Qt::darkBlue;
      _fgColor         = Qt::white;
      fgPixmap         = 0;
      bgPixmap         = 0;
      lasso            = new Lasso(_score);

      setXoffset(30);
      setYoffset(30);

      state            = NORMAL;
      cursor           = 0;
      shadowNote       = 0;
      cursorTimer      = new QTimer(this);
      buttonState      = 0;
      keyState         = 0;
      mousePressed     = false;

      connect(cursorTimer, SIGNAL(timeout()), SLOT(cursorBlink()));
      if (preferences.cursorBlink)
            cursorTimer->start(500);

      if (debugMode)
            setMouseTracking(true);
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Canvas::setScore(Score* s, ScoreLayout* l)
      {
      _score = s;
      if (cursor == 0) {
            cursor = new Cursor(_score, this);
            shadowNote = new ShadowNote(_score);
            cursor->setVisible(false);
            shadowNote->setVisible(false);
            }
      else {
            cursor->setScore(_score);
            shadowNote->setScore(_score);
            }
      Viewer::setScore(s);
      _layout = l;
      if (navigator) {
            navigator->setScore(_score);
            updateNavigator(false);
            }
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
//    cursor timer slot
//---------------------------------------------------------

void Canvas::cursorBlink()
      {
      if (!preferences.cursorBlink) {
            cursor->noBlinking();
            cursorTimer->stop();
            }
      cursor->blink();
      }

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

Canvas::~Canvas()
      {
      delete lasso;
      if (cursor)
            delete cursor;
      if (shadowNote)
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
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addSeparator();
      a->setText(obj->name());
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();

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
      else if (cmd == "edit") {
            if (startEdit(obj))
                  return;
            }
      else
            obj->propertyAction(cmd);
      _score->endCmd();
      }

//---------------------------------------------------------
//   measurePopup
//---------------------------------------------------------

void Canvas::measurePopup(const QPoint& gpos, Measure* obj)
      {
      Staff* staff = 0;
      int pitch;
      Segment* seg;
      QPointF offset;
      int tick = 0;
      _score->pos2measure(startMove, &tick, &staff, &pitch, &seg, &offset);
      if (staff == 0) {
            printf("Canvas::measurePopup: staff == 0 !\n");
            return;
            }

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addSeparator();
      a->setText(tr("Staff"));
      a = popup->addAction(tr("Edit Drumset..."));
      a->setData("edit-drumset");
      a->setEnabled(staff->part()->drumset() != 0);
      a = popup->addAction(tr("Properties"));
      a->setData("staff-properties");

      a = popup->addSeparator();
      a->setText(tr("Measure"));
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();

      if (obj->genPropertyMenu(popup))
            popup->addSeparator();
      a = popup->addAction(tr("Properties"));
      a->setData("measure-properties");
      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "measure-properties")
            mscore->showElementContext(obj);
      else if (cmd == "invisible")
            _score->toggleInvisible(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
      else if (cmd == "edit") {
            if (startEdit(obj))
                  return;
            }
      else if (cmd == "edit-drumset") {
            EditDrumset drumsetEdit(staff->part()->drumset(), this);
            drumsetEdit.exec();
            }
      else if (cmd == "staff-properties") {
            EditStaff staffEdit(staff, this);
            staffEdit.exec();
            }
      else
            obj->propertyAction(cmd);
      _score->setLayoutAll(true);
      _score->endCmd();
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
      bool b3 = ev->button() == Qt::RightButton;

      if (state == MAG) {
            if (b1)
                  mscore->incMag();
            else if (b3)
                  mscore->decMag();
            return;
            }

      keyState    = ev->modifiers();
      buttonState = ev->button();
      startMove   = imatrix.map(QPointF(ev->pos()));

      Element* element = elementAt(startMove);

      _score->setDragObject(element);

      if (seq && mscore->playEnabled() && element && element->type() == NOTE) {
            Note* note = (Note*)element;
            seq->startNote(note->staff()->midiChannel(), note->pitch(), 60);
            }

      //-----------------------------------------
      //  context menus
      //-----------------------------------------

      if (b3) {
            if (element) {
                  ElementType type = element->type();
                  _score->dragStaff = 0;
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
                  _score->setDragObject(0);
                  seq->stopNotes();       // stop now because we dont get a mouseRelease event
                  if (type == MEASURE) {
                        measurePopup(ev->globalPos(), (Measure*)element);
                        }
                  else {
                        objectPopup(ev->globalPos(), element);
                        }
                  }
            else {
                  canvasPopup(ev->globalPos());
                  }
            return;
            }

      if (state != EDIT)
            _score->startCmd();
      switch (state) {
            case NORMAL:
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
                  else {
                        _score->select(0, 0, 0);
                        // shift+drag selects "lasso mode"
                        if (!(keyState & Qt::ShiftModifier)) {
                              dragCanvasState = true;
                              setCursor(Qt::SizeAllCursor);
                              }
                        update();
                        }
                  break;

            case NOTE_ENTRY:
                  if (keyState & Qt::ControlModifier) {
                        dragCanvasState = true;
                        setCursor(Qt::SizeAllCursor);
                        }
                  else
                        _score->putNote(startMove, keyState & Qt::ShiftModifier);
                  break;

            case EDIT:
                  if (ev->button() == Qt::MidButton) {
                        // clipboard paste
                        _score->editObject->mousePress(startMove, ev);
                        break;
                        }
                  for (int i = 0; i < grips; ++i) {
                        if (grip[i].contains(startMove)) {
                              curGrip = i;
                              setState(DRAG_EDIT);
                              break;
                              }
                        }
                  if (state == DRAG_EDIT)
                        break;
                  else if (_score->editObject->mousePress(startMove, ev))
                        update();
                  else
                        setState(NORMAL);
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
                  _score->endCmd();
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
            QString mimeType = _score->sel->mimeType();
            if (!mimeType.isEmpty()) {
                  QDrag* drag = new QDrag(this);
                  drag->setPixmap(QPixmap());
                  QMimeData* mimeData = new QMimeData;
                  mimeData->setData(mimeType, _score->sel->mimeData());
                  drag->setMimeData(mimeData);
                  _score->endCmd();
                  drag->start(Qt::CopyAction);
                  return;
                  }
            }
      mouseMoveEvent1(ev);
      if (dragCanvasState)
           return;
      if (state == LASSO || state == DRAG_EDIT || state == NOTE_ENTRY)
            _score->setLayoutAll(false);  // DEBUG
      _score->end();
      }

//---------------------------------------------------------
//   mouseMoveEvent1
//---------------------------------------------------------

void Canvas::mouseMoveEvent1(QMouseEvent* ev)
      {
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

      QPointF p     = imatrix.map(QPointF(ev->pos()));
      QPointF delta = p - startMove;

      switch (state) {
            case NORMAL:
                  if (buttonState == 0)    // debug
                        return;
                  if (sqrt(pow(delta.x(),2)+pow(delta.y(),2)) * _matrix.m11() <= 2.0)
                        return;
                  {
                  Element* de = _score->dragObject();
                  if (de && keyState == Qt::ShiftModifier) {
                        if (de->type() == MEASURE) {
                              QString mimeType = _score->sel->mimeType();
                              if (!mimeType.isEmpty()) {
                                    QDrag* drag = new QDrag(this);
                                    QMimeData* mimeData = new QMimeData;
                                    mimeData->setData(mimeType, score()->sel->mimeData());
                                    drag->setMimeData(mimeData);
                                    _score->endCmd();
                                    drag->start(Qt::CopyAction);
                                    break;
                                    }
                              }
                        else {
                              QDrag* drag = new QDrag(this);
                              QMimeData* mimeData = new QMimeData;
                              QPointF rpos(startMove - de->abbox().topLeft());
                              mimeData->setData(mimeSymbolFormat, de->mimeData(rpos));
                              drag->setMimeData(mimeData);
                              _score->endCmd();
                              drag->start(Qt::CopyAction);
                              break;
                              }
                        }
                  if (de && de->isMovable()) {
                        QPointF o;
                        if (_score->sel->state() == SEL_STAFF || _score->sel->state() == SEL_SYSTEM) {
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
                  _lassoRect = lasso->abbox().normalized();
                  r = _matrix.mapRect(_lassoRect);
                  QSize sz(r.size().toSize());
                  QString s("%1 x %2");
                  mscore->statusBar()->showMessage(
                     s.arg(sz.width()).arg(sz.height()), 3000);
                  }
                  _score->addRefresh(lasso->abbox());
                  lassoSelect();
                  break;

            case EDIT:
                  break;

            case DRAG_EDIT:
                  {
                  Element* e = _score->editObject;
                  score()->addRefresh(e->abbox());
                  e->editDrag(curGrip, startMove, delta);
                  updateGrips();
                  startMove += delta;
                  }
                  break;

            case DRAG_STAFF:
//                  _score->dragSystem->setDistance(_score->dragStaff, delta.y());
//                  _score->layout1();   // DEBUG: does not work
                  break;

            case DRAG_OBJ:
                  _score->drag(delta);
                  break;

            case NOTE_ENTRY:
                  _score->addRefresh(shadowNote->abbox());
                  setShadowNote(p);
                  _score->addRefresh(shadowNote->abbox());
                  break;

            case MAG:
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Canvas::updateGrips()
      {
      Element* e = _score->editObject;
      if (e == 0)
            return;

      qreal w = 8.0 / _matrix.m11();
      qreal h = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < 4; ++i)
            grip[i] = r;
      e->updateGrips(&grips, grip);
      for (int i = 0; i < 4; ++i)
            score()->addRefresh(grip[i]);
      QPointF anchor = e->gripAnchor(curGrip);
      if (!anchor.isNull())
            setDropAnchor(QLineF(anchor, grip[curGrip].center()));
      else
            setDropTarget(0); // this also resets dropAnchor
      score()->addRefresh(e->abbox());
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Canvas::mouseReleaseEvent(QMouseEvent* ev)
      {
      if (dragCanvasState) {
            dragCanvasState = false;
            setState(state);        // reset cursor pixmap
            mousePressed = false;
            _score->endCmd();
            return;
            }
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
            _score->endCmd();
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Canvas::mouseReleaseEvent1(QMouseEvent* /*ev*/)
      {
      buttonState = 0;
      switch (state) {
            case DRAG_EDIT:
                  _score->addRefresh(_score->editObject->abbox());
                  _score->editObject->endEditDrag();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  _score->addRefresh(_score->editObject->abbox());
                  setState(EDIT);
                  _score->setLayoutAll(true);
                  _score->end();
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
                  _score->setLayoutAll(true);
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
            setDropTarget(0);
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
      if (element->startEdit(startMove)) {
            setFocus();
            _score->startEdit(element);
            setState(EDIT);
            qreal w = 8.0 / _matrix.m11();
            qreal h = 8.0 / _matrix.m22();
            QRectF r(-w*.5, -h*.5, w, h);
            for (int i = 0; i < 4; ++i)
                  grip[i] = r;
            _score->editObject->updateGrips(&grips, grip);
            curGrip = grips-1;

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
//---------------------------------------------------------

void Canvas::moveCursor()
      {
      int track = _score->inputTrack();
      if (track == -1) {
            track = 0;
            }
      int staff = track / VOICES;
      cursor->setVoice(track % VOICES);
      cursor->setTick(_score->inputPos());
      cursor->setStaff(_score->staff(staff));

      Segment* segment = _score->tick2segment(cursor->tick());
      if (segment) {
//            _score->adjustCanvasPosition(segment);
            System* system = segment->measure()->system();
            double x = segment->canvasPos().x();
            double y = system->bboxStaff(staff).y() + system->canvasPos().y();
            _score->addRefresh(cursor->abbox());
            cursor->setPos(x - _spatium, y - _spatium);
            _score->addRefresh(cursor->abbox());
            cursor->setHeight(6.0 * _spatium);
            return;
            }
      // _score->appendMeasures(1);
      printf("cursor position not found for tick %d, append new measure\n", cursor->tick());
      }

void Canvas::moveCursor(Segment* segment)
      {
      System* system = segment->measure()->system();
      double x = segment->canvasPos().x();

      double y = system->bboxStaff(0).y() + system->canvasPos().y();
      double y2 = system->bboxStaff(_score->nstaves()-1).y() + system->canvasPos().y();
      cursor->setHeight(y2 - y + 6.0 * _spatium);

      qreal d = _spatium * .5;

      _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2.0 * d, 2.0 * d));
      cursor->setPos(x - _spatium, y - _spatium);
      _score->addRefresh(cursor->abbox());
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void Canvas::setCursorOn(bool val)
      {
      if (cursor)
            cursor->setOn(val);
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
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);

      QRegion region;
      if (_score->needLayout()) {
            _score->doLayout();
            if (navigator)
                  navigator->layoutChanged();
            if (_score->noteEntryMode())
                  moveCursor();
            if (state == EDIT || state == DRAG_EDIT)
                  updateGrips();
            region = QRegion(0, 0, width(), height());
            }
      else
            region = ev->region();

      QVector<QRect> vector = region.rects();
      foreach(QRect r, vector)
            paint(r, p);

      p.setMatrix(_matrix);
      p.setClipping(false);

      cursor->draw(p);
      lasso->draw(p);
      shadowNote->draw(p);
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / p.worldMatrix().m11(), Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }
      if (dragElement) {
            p.translate(dragElement->canvasPos());
            p.setPen(Qt::black);
            dragElement->draw(p);
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Canvas::paint(const QRect& rr, QPainter& p)
      {
      p.save();
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
            r1 -= _matrix.mapRect(page->abbox()).toRect();
            }
      p.setClipRect(fr);

      QList<Element*> ell = _layout->items(fr);
      drawElements(p, ell);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      if (state == EDIT || state == DRAG_EDIT) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  p.setBrush(i == curGrip ? QBrush(Qt::blue) : Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
            }
      if (_score->sel->state() == SEL_STAFF || _score->sel->state() == SEL_SYSTEM) {
            int sstart = _score->sel->tickStart;
            int send   = _score->sel->tickEnd;

            Measure* sm = _score->tick2measure(sstart);
            Measure* em = _score->tick2measure(send);
            if (em->tick() != send) {       // hack for last measure
                  em = 0;
                  }

            p.setBrush(Qt::NoBrush);

            if (_score->sel->state() == SEL_SYSTEM) {
                  QPen pen(QColor(Qt::blue));
                  pen.setWidthF(3.0 / p.matrix().m11());
                  pen.setStyle(Qt::DotLine);
                  p.setPen(pen);
                  for (Measure* m = sm; m && (m != em);) {
                        double x1 = m->abbox().x();
                        double x2 = x1 + m->abbox().width();
                        double y1 = m->abbox().y() - _spatium;
                        double y2 = y1 + m->abbox().height() + 2 * _spatium;

                        // is this measure start of selection?
                        if (m == sm) {
                              x1 -= _spatium;
                              p.drawLine(QLineF(x1, y1, x1, y2));
                              }
                        m = m->next();
                        // is this measure end of selection?
                        if (m == em) {
                              x2 += _spatium;
                              p.drawLine(QLineF(x2, y1, x2, y2));
                              }
                        p.drawLine(QLineF(x1, y1, x2, y1));
                        p.drawLine(QLineF(x1, y2, x2, y2));
                        }
                  }
            else {
                  QPen pen(QColor(Qt::blue));
                  pen.setWidthF(2.0 / p.matrix().m11());
                  pen.setStyle(Qt::SolidLine);
                  p.setPen(pen);
                  for (Measure* m = sm; m && (m != em);) {
                        QRectF bb     = m->abbox();
                        double x1     = bb.x();
                        double x2     = x1 + bb.width();
                        SysStaff* ss1 = m->system()->staff(_score->sel->staffStart);
                        double y1     = ss1->bbox().y() - _spatium + bb.y();
                        SysStaff* ss2 = m->system()->staff(_score->sel->staffEnd-1);
                        double y2     = ss2->bbox().y() + ss2->bbox().height() + _spatium + bb.y();

                        // is this measure start of selection?
                        if (m == sm) {
                              x1 -= _spatium;
                              p.drawLine(QLineF(x1, y1, x1, y2));
                              }
                        m = m->next();
                        // is this measure end of selection?
                        if (m == em) {
                              x2 += _spatium;
                              p.drawLine(QLineF(x2, y1, x2, y2));
                              }
                        p.drawLine(QLineF(x1, y1, x2, y1));
                        p.drawLine(QLineF(x1, y2, x2, y2));
                        }
                  }
            }

      p.setMatrixEnabled(false);
      if (!r1.isEmpty()) {
            p.setClipRegion(r1);  // only background
            if (bgPixmap == 0 || bgPixmap->isNull())
                  p.fillRect(rr, _bgColor);
            else
                  p.drawTiledPixmap(rr, *bgPixmap, rr.topLeft()-QPoint(lrint(xoffset()), lrint(yoffset())));
            }
      p.restore();
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
//   dragTimeAnchorElement
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
//   dragMeasureAnchorElement
//---------------------------------------------------------

bool Canvas::dragMeasureAnchorElement(const QPointF& pos)
      {
      int tick;
      Measure* m = _score->pos2measure3(pos, &tick);
      if (m) {
            QPointF anchor;
            if (m->tick() == tick)
                  anchor = m->abbox().topLeft();
            else
                  anchor = m->abbox().topRight();
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
//   dragEnterEvent
//---------------------------------------------------------

void Canvas::dragEnterEvent(QDragEnterEvent* event)
      {
      if (dragElement) {
            delete dragElement;
            dragElement = 0;
            }

      const QMimeData* data = event->mimeData();

      if (data->hasFormat(mimeSymbolListFormat)
         || data->hasFormat(mimeStaffListFormat)
         || data->hasFormat(mimeMeasureListFormat)) {
            event->acceptProposedAction();
            return;
            }

      else if (data->hasFormat(mimeSymbolFormat)) {
            event->acceptProposedAction();

            QByteArray a = data->data(mimeSymbolFormat);
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(a, &err, &line, &column)) {
                  printf("error reading drag data at %d/%d: %s\n<%s>\n",
                     line, column, err.toLatin1().data(), a.data());
                  return;
                  }
            docName = "--";
            QDomElement e = doc.documentElement();

            int type = Element::readType(e, &dragOffset);
            Element* el = 0;
            switch(type) {
                  case VOLTA:
                  case OTTAVA:
                  case TRILL:
                  case PEDAL:
                  case HAIRPIN:
                        el = Element::create(type, score());
                        ((SLine*)el)->setLen(_spatium * 7);
                        break;
                  case SYMBOL:
                        el = new Symbol(score());
                        break;
                  case IMAGE:
                        {
                        // look ahead for image type
                        QString path;
                        for (QDomElement ee = e.firstChildElement(); !ee.isNull(); ee = ee.nextSiblingElement()) {
                              QString tag(ee.tagName());
                              if (tag == "path") {
                                    path = ee.text();
                                    break;
                                    }
                              }
                        Image* image = 0;
                        if (path.endsWith(".svg"))
                              image = new SvgImage(score());
                        else if (path.endsWith(".jpg")
                           || path.endsWith(".png")
                           || path.endsWith(".xpm")
                              )
                              image = new RasterImage(score());
                        else {
                              printf("unknown image format <%s>\n", path.toLatin1().data());
                              }
                        el = image;
                        }
                        break;
                  case KEYSIG:
                  case CLEF:
                  case TIMESIG:
                  case BREATH:
                  case ATTRIBUTE:
                  case ACCIDENTAL:
                  case DYNAMIC:
                  case TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case REPEAT:
                  case REPEAT_MEASURE:
                        el = Element::create(type, score());
                        break;
                  case BAR_LINE:
                  case ARPEGGIO:
                  case BRACKET:
                        el = Element::create(type, score());
                        el->setHeight(_spatium * 5);
                        break;
                  default:
                        printf("dragEnter %s\n", Element::name(type));
                        break;
                  }
            if (el) {
                  dragElement = el;
                  dragElement->setParent(0);
                  dragElement->read(e);
                  dragElement->layout(score()->mainLayout());
                  }
            }

      else if (data->hasUrls()) {
            QList<QUrl>ul = data->urls();
            QUrl u = ul.front();
            if (debugMode)
                  printf("drag Url: %s\n", u.toString().toLatin1().data());
            printf("scheme <%s> path <%s>\n", u.scheme().toLatin1().data(),
               u.path().toLatin1().data());
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  if (fi.suffix() == "svg"
                     || fi.suffix() == "jpg"
                     || fi.suffix() == "png"
                     || fi.suffix() == "xpm"
                     ) {
                        event->acceptProposedAction();
                        }
                  }
            }
      else {
            if (debugMode) {
                  printf("dragEnterEvent: formats %d:\n", data->hasFormat(mimeSymbolFormat));
                  foreach(QString s, data->formats())
                        printf("   <%s>\n", s.toLatin1().data());
                  }
            }
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Canvas::dragMoveEvent(QDragMoveEvent* event)
      {
      // we always accept the drop action
      // to get a "drop" Event:

      event->acceptProposedAction();

      _score->start();
      _score->setLayoutAll(false);

      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));

      if (dragElement) {
            switch(dragElement->type()) {
                  case VOLTA:
                        dragMeasureAnchorElement(pos);
                        break;
                  case PEDAL:
                  case DYNAMIC:
                  case OTTAVA:
                  case TRILL:
                  case HAIRPIN:
                        dragTimeAnchorElement(pos);
                        break;
                  case SYMBOL:
                        {
                        Symbol* s = (Symbol*)dragElement;
                        if (s->anchor() == ANCHOR_STAFF) {
                              dragTimeAnchorElement(pos);
                              }
                        }
                        break;
                  case KEYSIG:
                  case CLEF:
                  case TIMESIG:
                  case BAR_LINE:
                  case ARPEGGIO:
                  case BREATH:
                  case BRACKET:
                  case ATTRIBUTE:
                  case ACCIDENTAL:
                  case TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case REPEAT:
                  case REPEAT_MEASURE:
                        {
                        Element* el = elementAt(pos);
                        if (el) {
                              if (el->acceptDrop(this, pos, dragElement->type(), dragElement->subtype())) {
                                    event->acceptProposedAction();
                                    break;
                                    }
                              if (debugMode)
                                    printf("ignore drop of %s\n", dragElement->name());
                              }
                        setDropTarget(0);
                        }
                        break;
                  case IMAGE:
                  default:
                        break;
                  }
            score()->addRefresh(dragElement->abbox());
            dragElement->setPos(pos - dragOffset);
            score()->addRefresh(dragElement->abbox());
            _score->end();
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  if (fi.suffix() != "svg"
                     && fi.suffix() != "jpg"
                     && fi.suffix() != "png"
                     && fi.suffix() != "xpm"
                     )
                        return;
                  //
                  // special drop target Note
                  //
                  Element* el = elementAt(pos);
                  if (el && (el->type() == NOTE || el->type() == REST))
                        setDropTarget(el);
                  else
                        setDropTarget(0);
                  }
            _score->end();
            return;
            }
      const QMimeData* md = event->mimeData();
      QByteArray data;
      int etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else if (md->hasFormat(mimeMeasureListFormat)) {
            etype = MEASURE_LIST;
            data = md->data(mimeMeasureListFormat);
            }
      else {
            _score->end();
            return;
            }
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != MEASURE) {
            _score->end();
            return;
            }
      else if (etype == ELEMENT_LIST) {
            printf("accept drop element list\n");
            }
      else if (etype == STAFF_LIST || etype == MEASURE_LIST) {
//TODO            el->acceptDrop(this, pos, etype, e);
            }
      _score->end();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Canvas::dropEvent(QDropEvent* event)
      {
      QPointF pos(imatrix.map(QPointF(event->pos())));

      if (dragElement) {
            _score->startCmd();
            _score->addRefresh(dragElement->abbox());
            switch(dragElement->type()) {
                  case VOLTA:
                  case OTTAVA:
                  case TRILL:
                  case PEDAL:
                  case DYNAMIC:
                  case HAIRPIN:
                        score()->cmdAdd1(dragElement, pos, dragOffset);
                        event->acceptProposedAction();
                        break;
                  case SYMBOL:
                        {
                        Symbol* s = (Symbol*) dragElement;
                        score()->cmdAddBSymbol(s, pos, dragOffset);
                        event->acceptProposedAction();
                        }
                        break;
                  case IMAGE:
                        score()->cmdAddBSymbol((Image*)dragElement, pos, dragOffset);
                        event->acceptProposedAction();
                        break;
                  case KEYSIG:
                  case CLEF:
                  case TIMESIG:
                  case BAR_LINE:
                  case ARPEGGIO:
                  case BREATH:
                  case BRACKET:
                  case ATTRIBUTE:
                  case ACCIDENTAL:
                  case TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case REPEAT:
                  case REPEAT_MEASURE:
                        {
                        Element* el = elementAt(pos);
                        if (el) {
                              _score->addRefresh(el->abbox());
                              _score->addRefresh(dragElement->abbox());
                              Element* dropElement = el->drop(pos, dragOffset, dragElement);
                              _score->addRefresh(el->abbox());
                              if (dropElement) {
                                    _score->select(dropElement, 0, 0);
                                    _score->addRefresh(dropElement->abbox());
                                    }
                              event->acceptProposedAction();
                              }
                        else {
                              printf("cannot drop here\n");
                              delete dragElement;
                              }
                        }
                        break;
                  default:
                        delete dragElement;
                        break;
                  }
            score()->endCmd();
            dragElement = 0;
            setDropTarget(0); // this also resets dropRectangle and dropAnchor
            setState(NORMAL);
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = 0;
                  if (fi.suffix() == "svg")
                        s = new SvgImage(score());
                  else if (fi.suffix() == "jpg"
                     || fi.suffix() == "png"
                     || fi.suffix() == "xpm"
                        )
                        s = new RasterImage(score());
                  else
                        return;
                  _score->startCmd();
                  s->setPath(u.path());
                  s->setAnchor(ANCHOR_PAGE);
                  Element* el = elementAt(pos);
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        s->setAnchor(ANCHOR_STAFF);
                        s->setStaff(el->staff());
                        if (el->type() == NOTE) {
                              Note* note = (Note*)el;
                              s->setTick(note->chord()->tick());
                              s->setParent(note->chord()->segment()->measure());
                              }
                        else  {
                              Rest* rest = (Rest*)el;
                              s->setTick(rest->tick());
                              s->setParent(rest->segment()->measure());
                              }
                        score()->undoAddElement(s);
                        }
                  else
                        score()->cmdAddBSymbol(s, pos, dragOffset);

                  event->acceptProposedAction();
                  score()->endCmd();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  setState(NORMAL);
                  return;
                  }
            setState(NORMAL);
            return;
            }

      const QMimeData* md = event->mimeData();
      QByteArray data;
      int etype;
      if (md->hasFormat(mimeSymbolListFormat)) {
            etype = ELEMENT_LIST;
            data = md->data(mimeSymbolListFormat);
            }
      else if (md->hasFormat(mimeStaffListFormat)) {
            etype = STAFF_LIST;
            data = md->data(mimeStaffListFormat);
            }
      else if (md->hasFormat(mimeMeasureListFormat)) {
            etype = MEASURE_LIST;
            data = md->data(mimeMeasureListFormat);
            }
      else {
            printf("cannot drop this object: unknown mime type\n");
            _score->end();
            return;
            }

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading drag data\n");
            return;
            }
      docName = "--";
      QDomElement e = doc.documentElement();
      Element* el = elementAt(pos);
      if (el == 0 || el->type() != MEASURE) {
            setDropTarget(0);
            setState(NORMAL);
            return;
            }
      Measure* measure = (Measure*) el;

      if (etype == ELEMENT_LIST) {
            printf("drop element list\n");
            }
      else if (etype == MEASURE_LIST || etype == STAFF_LIST) {
            _score->startCmd();
            System* s = measure->system();
            int idx = s->y2staff(pos.y());
            if (idx != -1)
                  score()->pasteStaff(e, measure, idx);
            event->acceptProposedAction();
            _score->endCmd();
            }
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      setState(NORMAL);
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void Canvas::dragLeaveEvent(QDragLeaveEvent*)
      {
      if (dragElement) {
            _score->start();
            _score->setLayoutAll(false);
            _score->addRefresh(dragElement->abbox());
            delete dragElement;
            dragElement = 0;
            _score->end();
            }
      setDropTarget(0);
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
      if ((dy < 0 || dx > 0) && navigator->isVisible()) {
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

      // if p hit more than one element, give back
      // the element after the selected one;

      int n = el.size();
      for (int i = 0; i < n; ++i) {
            if (el[i]->selected() && (i < (n-1))) {
                  return el.at(i + 1);
                  }
            }
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

            if (!e->visible()) {
                  if (score()->printing() || !score()->showInvisible())
                        continue;
                  }
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
                  p.setPen(QPen(Qt::red, 0, Qt::SolidLine));
                  qreal w = e->bbox().width() / 4.0;
                  qreal h = e->bbox().height() / 4.0;
                  qreal x = e->bbox().x();
                  qreal y = e->bbox().y();
                  p.drawLine(QLineF(x-w, y-h, x+w, y+h));
                  p.drawLine(QLineF(x+w, y-h, x-w, y+h));
                  }
            p.restore();
            }
      }

//---------------------------------------------------------
//   paintLasso
//---------------------------------------------------------

void Canvas::paintLasso(QPainter& p)
      {
      QRectF r = _matrix.mapRect(lassoRect());
      double x = r.x();
      double y = r.y();

      QMatrix omatrix(_matrix);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), _matrix.dx()-x, _matrix.dy()-y);
      imatrix = _matrix.inverted();

      p.setMatrix(_matrix);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      QList<Element*> el = _layout->items(QRectF(0.0, 0.0, 100000.0, 1000000.0));
      drawElements(p, el);
      cursor->draw(p);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / p.worldMatrix().m11(), Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }
      if (state == EDIT || state == DRAG_EDIT) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::blue);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  p.setBrush(i == curGrip ? QBrush(Qt::blue) : Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
            }
      _matrix = omatrix;
      imatrix = _matrix.inverted();
      p.end();
      }


