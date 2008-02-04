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
#include "textline.h"
#include "trill.h"
#include "hairpin.h"
#include "image.h"
#include "globals.h"
#include "part.h"
#include "editdrumset.h"
#include "editstaff.h"
#include "repeatflag.h"
#include "barline.h"

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
      a->setText(obj->userName());
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();
      obj->genPropertyMenu(popup);
      popup->addSeparator();
      a = popup->addAction(tr("Object Inspector"));
      a->setData("list");
      a = popup->exec(pos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "list")
            mscore->showElementContext(obj);
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
      int staffIdx = -1;
      int pitch;
      Segment* seg;
      QPointF offset;
      int tick = 0;

      _score->pos2measure(startMove, &tick, &staffIdx, &pitch, &seg, &offset);
      if (staffIdx == -1) {
            printf("Canvas::measurePopup: staffIdx == -1!\n");
            return;
            }

      Staff* staff = _score->staff(staffIdx);

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addSeparator();
      a->setText(tr("Staff"));
      a = popup->addAction(tr("Edit Drumset..."));
      a->setData("edit-drumset");
      a->setEnabled(staff->part()->drumset() != 0);
      a = popup->addAction(tr("Properties..."));
      a->setData("staff-properties");

      a = popup->addSeparator();
      a->setText(tr("Measure"));
      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();

      if (obj->genPropertyMenu(popup))
            popup->addSeparator();

      a = popup->addAction(tr("Object Inspector"));
      a->setData("list");

      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "list")
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

      Qt::KeyboardModifiers keyState = ev->modifiers();
      Qt::MouseButtons buttonState = ev->button();
      startMove   = imatrix.map(QPointF(ev->pos()));

      Element* element = elementAt(startMove);

      _score->setDragObject(element);

      if (mscore->playEnabled() && element && element->type() == NOTE) {
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

      if (state != EDIT && state != NOTE_ENTRY_EDIT)
            _score->startCmd();
      switch (state) {
            case NORMAL:
                  //-----------------------------------------
                  //  select operation
                  //-----------------------------------------

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

            case NOTE_ENTRY_EDIT:
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
      if (state == EDIT || state == NOTE_ENTRY_EDIT)
            return;

      Element* element = _score->dragObject();

      if (element) {
            _score->startCmd();

            if (!startEdit(element)) {
                  _score->endCmd();
                  }
            }
      else
            mousePressEvent(ev);
      }

//---------------------------------------------------------
//   mouseMoveEvent
//---------------------------------------------------------

void Canvas::mouseMoveEvent(QMouseEvent* ev)
      {
      if (QApplication::mouseButtons() == Qt::MidButton) {
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
      if (state == LASSO || state == NOTE_ENTRY)
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
                  if (QApplication::mouseButtons() == 0)    // debug
                        return;
                  if (sqrt(pow(delta.x(),2)+pow(delta.y(),2)) * _matrix.m11() <= 2.0)
                        return;
                  {
                  Element* de = _score->dragObject();
                  if (de && (QApplication::keyboardModifiers() == Qt::ShiftModifier)) {
                        // drag selection
                        QString mimeType = _score->sel->mimeType();
                        if (!mimeType.isEmpty()) {
                              QDrag* drag = new QDrag(this);
                              QMimeData* mimeData = new QMimeData;
                              mimeData->setData(mimeType, score()->sel->mimeData());
                              drag->setMimeData(mimeData);
                              _score->endCmd();
                              drag->start(Qt::CopyAction);
                              }
                        break;
                        }
                  if (de && de->isMovable()) {
                        QPointF o;
                        if (_score->sel->state() != SEL_STAFF && _score->sel->state() != SEL_SYSTEM) {
                              _score->startDrag();
                              o = QPointF(de->userOff() * _spatium);
                              setState(DRAG_OBJ);
                              }
                        startMove -= o;
                        break;
                        }
                  if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
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
            case NOTE_ENTRY_EDIT:
            case STATES:      // dummy
                  break;


            case DRAG_EDIT:
                  {
                  _score->setLayoutAll(false);
                  Element* e = _score->editObject;
                  score()->addRefresh(e->abbox());
                  e->editDrag(curGrip, startMove, delta);
                  updateGrips();
                  startMove += delta;
                  }
                  break;

            case DRAG_OBJ:
                  {
                  _score->drag(delta);
                  Element* e = _score->getSelectedElement();
                  if (e) {
                        QLineF anchor = e->dragAnchor();
                        if (!anchor.isNull())
                              setDropAnchor(anchor);
                        else
                              setDropTarget(0); // this also resets dropAnchor
                        }
                  }
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
            setCursor(QCursor(Qt::ArrowCursor));
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
      if (state == EDIT || state == NOTE_ENTRY_EDIT)
            return;
      if (state == MAG) {
            if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
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
      switch (state) {
            case DRAG_EDIT:
                  _score->addRefresh(_score->editObject->abbox());
                  _score->editObject->endEditDrag();
                  updateGrips();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  _score->addRefresh(_score->editObject->abbox());
                  setState(EDIT);
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
                  setDropTarget(0); // this also resets dropAnchor
                  break;

            case NOTE_ENTRY:
                  break;

            case NORMAL:
                  if (!_score->dragObject())
                        _score->select(0, 0, 0);      // deselect all
                  break;

            default:
                  setState(NORMAL);
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

void Canvas::setState(State action)
      {
      static const char* stateNames[] = {
         "NORMAL", "DRAG_OBJ", "EDIT", "DRAG_EDIT", "LASSO",
         "NOTE_ENTRY", "MAG", "NOTE_ENTRY_EDIT"
         };
      int stateTable[STATES][STATES-1] = {
//action          NORMAL, DRAG_OBJ, EDIT, DRAG_EDIT, LASSO, NOTE_ENTRY, MAG
/*NORMAL         */ {  1,       99,    8,         0,     6,          2,   4 },
/*DRAG_OBJ       */ { 99,        1,    0,         0,     0,          0,   0 },
/*EDIT           */ {  9,        0,    1,        99,     0,          0,   0 },
/*DRAG_EDIT      */ { 99,        0,   99,         1,     0,          0,   0 },
/*LASSO          */ {  7,        0,    0,         0,     1,          0,   0 },
/*NOTE_ENTRY     */ {  3,        0,   10,         0,     0,          1,   0 },
/*MAG            */ {  5,        0,    0,         0,     0,          0,   1 },
/*NOTE_ENTRY_EDIT*/ { 11,        0,    0,         0,     0,          0,   0 }
      };

      if (debugMode)
            printf("switch from %s to %s\n", stateNames[state], stateNames[action]);

      switch(stateTable[state][action]) {
            default:
            case 0:
                  printf("illegal state switch from %s to %s\n",
                     stateNames[state], stateNames[action]);
                  break;
            case 1:
                  break;
            case 2:     // NORMAL     - NOTE_ENTRY
                  setCursor(QCursor(Qt::UpArrowCursor));
                  setMouseTracking(true);
                  shadowNote->setVisible(true);
                  setCursorOn(true);
                  state = action;
                  break;
            case 5:     // MAG        - NORMAL
                  setCursor(QCursor(Qt::ArrowCursor));
                  state = action;
                  break;
            case 3:     // NOTE_ENTRY - NORMAL
                  setCursor(QCursor(Qt::ArrowCursor));
                  setMouseTracking(false);
                  shadowNote->setVisible(false);
                  setCursorOn(false);
                  state = action;
                  break;
            case 4:     // NORMAL - MAG
                  setCursor(QCursor(Qt::SizeAllCursor));
                  state = action;
                  break;
            case 6:     // NORMAL - LASSO
                  lasso->setVisible(true);
                  state = action;
                  break;
            case 7:     // LASSO - NORMAL
                  lasso->setVisible(false);
                  state = action;
                  break;
            case 8:     // NORMAL - EDIT
                  state = action;
                  mscore->setState(STATE_EDIT);
                  break;
            case 9:     // EDIT - NORMAL
                  state = action;
                  setDropTarget(0);
                  _score->endEdit();
                  break;
            case 10:    // NOTE_ENTRY - EDIT
                  setCursor(QCursor(Qt::ArrowCursor));
                  setMouseTracking(false);
                  shadowNote->setVisible(false);
                  setCursorOn(false);
                  state = NOTE_ENTRY_EDIT;
                  mscore->setState(STATE_EDIT);
                  break;
            case 11:    // EDIT_NOTE_ENTRY - NORMAL
                  setDropTarget(0);
                  _score->endEdit();
                  setCursor(QCursor(Qt::UpArrowCursor));
                  setMouseTracking(true);
                  shadowNote->setVisible(true);
                  setCursorOn(true);
                  _score->setNoteEntry(true, false);
                  if (_score->noteEntryMode())
                        state = NOTE_ENTRY;
                  else {
                        printf("leave note entry\n");
                        state = NORMAL;
                        }
                  break;
            case 99:
                  state = action;
                  break;
            }
      _score->addRefresh(shadowNote->abbox());
      _score->addRefresh(cursor->abbox());
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
      if (track == -1)
            track = 0;

      if (track == cursor->track() && cursor->tick() == _score->inputPos())
            return;

      cursor->setTrack(track);
      cursor->setTick(_score->inputPos());

      Segment* segment = _score->tick2segment(cursor->tick());
      if (segment) {
            System* system = segment->measure()->system();
            double x = segment->canvasPos().x();
            double y = system->bboxStaff(cursor->staffIdx()).y() + system->canvasPos().y();
            _score->addRefresh(cursor->abbox());
            cursor->setPos(x - _spatium, y - _spatium);
            _score->addRefresh(cursor->abbox());
            cursor->setHeight(6.0 * _spatium);
            return;
            }
      // _score->appendMeasures(1);
      // printf("cursor position not found for tick %d, append new measure\n", cursor->tick());
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
      int staffIdx;
      Segment* seg;

      Measure* m = _score->pos2measure2(p, &tick, &staffIdx, &line, &seg);
      if (m == 0)
            return;

      System* s = m->system();
      shadowNote->setLine(line);

      double y = seg->canvasPos().y() + s->staff(staffIdx)->y();
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
            if (state == EDIT || state == DRAG_EDIT || state == NOTE_ENTRY_EDIT)
                  updateGrips();
            region = QRegion(0, 0, width(), height());
            }
      else
            region = ev->region();

      if (score()->noteEntryMode())
            moveCursor();

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
      foreach (const Page* page, _layout->pages())
            r1 -= _matrix.mapRect(page->abbox()).toRect();
      p.setClipRect(fr);

      QList<const Element*> ell = _layout->items(fr);
      drawElements(p, ell);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      if (state == EDIT || state == DRAG_EDIT || state == NOTE_ENTRY_EDIT) {
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
                  pen.setWidthF(2.0 / p.matrix().m11());
                  pen.setStyle(Qt::DotLine);
                  p.setPen(pen);
                  for (MeasureBase* m = sm; m && (m != em); m = m->next()) {
                        double x1 = m->abbox().x();
                        double x2 = x1 + m->abbox().width();
                        double y1 = m->abbox().y() - _spatium;
                        double y2 = y1 + m->abbox().height() + 2 * _spatium;

                        // is this measure start of selection?
                        if (m == sm) {
                              x1 -= _spatium;
                              p.drawLine(QLineF(x1, y1, x1, y2));
                              }
                        // is this measure end of selection?
                        if (((m->tick() + m->tickLen()) >= send) || (m->next() == em)) {
                              x2 += _spatium;
                              p.drawLine(QLineF(x2, y1, x2, y2));
                              p.drawLine(QLineF(x1, y1, x2, y1));
                              p.drawLine(QLineF(x1, y2, x2, y2));
                              break;
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
                  for (MeasureBase* m = sm; m && (m != em); m = m->next()) {
                        QRectF bb     = m->abbox();
                        double x1     = bb.x();
                        double x2     = x1 + bb.width();
                        SysStaff* ss1 = m->system()->staff(_score->sel->staffStart);
                        double y1     = ss1->y() - _spatium + bb.y();
                        SysStaff* ss2 = m->system()->staff(_score->sel->staffEnd-1);
                        double y2     = ss2->y() + ss2->bbox().height() + _spatium + bb.y();

                        // is this measure start of selection?
                        if (m == sm) {
                              x1 -= _spatium;
                              p.drawLine(QLineF(x1, y1, x1, y2));
                              }
                        // is this measure end of selection?
                        if (((m->tick() + m->tickLen()) >= send) || (m->next() == em)) {
                              x2 += _spatium;
                              p.drawLine(QLineF(x2, y1, x2, y2));
                              p.drawLine(QLineF(x1, y1, x2, y1));
                              p.drawLine(QLineF(x1, y2, x2, y2));
                              break;
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
      int staffIdx = -1;
      Segment* seg;
      QPointF offset;
      int tick;
      MeasureBase* mb = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, &offset);
      if (mb && mb->type() == MEASURE) {
            Measure* m = (Measure*)mb;
            System* s = m->system();
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
      int staffIdx = -1;
      Segment* seg;
      QPointF offset;
      int tick;
      MeasureBase* mb = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, &offset);
      if (mb && mb->type() == MEASURE) {
            Measure* m = (Measure*)mb;
            System* s = m->system();

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
      int staffIdx = -1;
      Segment* seg;
      QPointF offset;
      int tick;
      MeasureBase* m = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, &offset);
      if (m && m->type() == MEASURE) {
            System* s = m->system();
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

// printf("DRAG<%s>\n", a.data());
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
                  case TEXTLINE:
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
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case NOTE:
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
                  case TEXTLINE:
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
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                        {
                        Element* el = elementAt(pos);
                        if (el) {
                              if (el->acceptDrop(this, pos, dragElement->type(), dragElement->subtype())) {
                                    event->acceptProposedAction();
                                    break;
                                    }
                              if (debugMode)
                                    printf("ignore drag of %s\n", dragElement->name());
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
                  case TEXTLINE:
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
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case NOTE:
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
                  s->setAnchor(ANCHOR_PARENT);
                  Element* el = elementAt(pos);
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        s->setAnchor(ANCHOR_STAFF);
                        s->setStaffIdx(el->staffIdx());
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
      if (!e1->selectable())
            return false;
      return e1->type() < e2->type();
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* Canvas::elementAt(const QPointF& p)
      {
      QList<const Element*> el = _layout->items(p);
      if (el.empty())
            return 0;
      qSort(el.begin(), el.end(), elementLower);

#if 0
      printf("elementAt\n");
      foreach(const Element* e, el)
            printf("  %s %d\n", e->name(), e->selected());
#endif
      return const_cast<Element*>(el.at(0));
      }

//---------------------------------------------------------
//   selectedElementAt
//---------------------------------------------------------

Element* Canvas::selectedElementAt(const QPointF& p)
      {
      QList<const Element*> el = _layout->items(p);
      if (el.empty())
            return 0;
      foreach(const Element* e, el) {
            if (e->selected())
                  return const_cast<Element*>(e);
            }
      return 0;
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void Canvas::drawElements(QPainter& p,const QList<const Element*>& el)
      {
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            e->itemDiscovered = 0;

            if (!e->visible()) {
                  if (score()->printing() || !score()->showInvisible())
                        continue;
                  }
            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(p);
// if (e->type() == TEXT && e->subtype() == TEXT_INSTRUMENT_LONG)
//   printf("text %p <%s> %f %f\n",
//      e, qPrintable(((Text*)e)->getText()), e->canvasPos().x(), e->canvasPos().y());

//            if (debugMode && e->selected()) {
            if (false) {
                  //
                  //  draw bounding box rectangle for all
                  //  selected Elements
                  //
                  p.setBrush(Qt::NoBrush);
                  p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
                  p.drawPath(e->shape());

                  p.setPen(QPen(Qt::red, 0, Qt::SolidLine));
                  p.drawRect(e->bbox());

                  p.setPen(QPen(Qt::red, 0, Qt::SolidLine));
                  qreal w = 5.0 / p.matrix().m11();
                  qreal h = w;
                  qreal x = 0; // e->bbox().x();
                  qreal y = 0; // e->bbox().y();
                  p.drawLine(QLineF(x-w, y-h, x+w, y+h));
                  p.drawLine(QLineF(x+w, y-h, x-w, y+h));
                  if (e->parent()) {
                        p.restore();
                        p.setPen(QPen(Qt::green, 0, Qt::SolidLine));
                        p.drawRect(e->parent()->abbox());
                        continue;
                        }
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

      QList<const Element*> el = _layout->items(QRectF(0.0, 0.0, 100000.0, 1000000.0));
      drawElements(p, el);
      cursor->draw(p);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / p.worldMatrix().m11(), Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }
      if (state == EDIT || state == DRAG_EDIT || state == NOTE_ENTRY_EDIT) {
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


