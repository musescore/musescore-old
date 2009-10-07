//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2008 Werner Schweer and others
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

#include "globals.h"
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
#include "dynamics.h"
#include "pedal.h"
#include "volta.h"
#include "ottava.h"
#include "textline.h"
#include "trill.h"
#include "hairpin.h"
#include "image.h"
#include "part.h"
#include "editdrumset.h"
#include "editstaff.h"
#include "repeatflag.h"
#include "barline.h"
#include "system.h"
#include "magbox.h"
#include "measure.h"
#include "drumroll.h"

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

      level            = 0;
      dragElement      = 0;
      dragObject       = 0;
      navigator        = 0;
      _score           = 0;
      dragCanvasState  = false;
      draggedCanvas    = false;
      _bgColor         = Qt::darkBlue;
      _fgColor         = Qt::white;
      fgPixmap         = 0;
      bgPixmap         = 0;
      lasso            = new Lasso(_score);

      state            = NORMAL;
      cursor           = 0;
      shadowNote       = 0;
      mousePressed     = false;
      grips            = 0;

      if (debugMode)
            setMouseTracking(true);

      if (preferences.bgUseColor)
            setBackground(preferences.bgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.bgWallpaper);
            setBackground(pm);
            }
      if (preferences.fgUseColor)
            setForeground(preferences.fgColor);
      else {
            QPixmap* pm = new QPixmap(preferences.fgWallpaper);
            if (pm == 0 || pm->isNull())
                  printf("no valid pixmap %s\n", qPrintable(preferences.fgWallpaper));
            setForeground(pm);
            }
      showNavigator(preferences.showNavigator);
      }

//---------------------------------------------------------
//   navigatorVisible
//---------------------------------------------------------

bool Canvas::navigatorVisible() const
      {
      return navigator && navigator->isVisible();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void Canvas::setScore(Score* s)
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
            if (debugMode) {
                  printf("key key:%x modifiers:%x text:<%s>\n", ke->key(),
                     int(ke->modifiers()), qPrintable(ke->text()));
                  }
            int k = ke->key();
            if ((state == EDIT) && ((k == Qt::Key_Tab) || (k == Qt::Key_Backtab))) {
                  keyPressEvent(ke);
                  return true;
                  }
            }
      if (ev->type() == QEvent::InputMethod) {
            QInputMethodEvent* ie = static_cast<QInputMethodEvent*>(ev);
            if (state != EDIT && state != DRAG_EDIT)
                  return false;
            Element* e = _score->editObject;
            if (e->edit(this, curGrip, 0, 0, ie->commitString())) {
                  updateGrips();
                  _score->end();
                  return true;
                  }
            }
      return QWidget::event(ev);
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
      setState(NORMAL);
      _score->setLayoutAll(true);
      _score->end();
      popup->popup(pos);
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
                  obj->score()->select(obj, SELECT_SINGLE, 0);
            }

      QMenu* popup = new QMenu(this);
      popup->setSeparatorsCollapsible(false);

      QAction* a = popup->addAction(obj->userName());
      a->setEnabled(false);

      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));
      popup->addSeparator();
      QMenu* selMenu = popup->addMenu(tr("Select"));
      selMenu->addAction(getAction("select-similar"));
      selMenu->addAction(getAction("select-similar-staff"));
      a = selMenu->addAction(tr("More..."));
      a->setData("select-dialog");
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
      else if (cmd == "select-similar")
            score()->selectSimilar(obj, false);
      else if (cmd == "select-similar-staff")
            score()->selectSimilar(obj, true);
      else if (cmd == "select-dialog")
            score()->selectElementDialog(obj);
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

      if (enableExperimental) {
            if (staff->part()->drumset()) {
                  a = popup->addAction(tr("Drumroll Editor..."));
                  a->setData("drumroll");
                  }
            else {
                  a = popup->addAction(tr("Pianoroll Editor..."));
                  a->setData("pianoroll");
                  }
            }

      a = popup->addAction(tr("Staff Properties..."));
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
      else if (cmd == "drumroll") {
            DrumrollEditor drumrollEditor(staff);
            drumrollEditor.exec();
            }
      else if (cmd == "pianoroll") {
            _score->endCmd();
            mscore->editInPianoroll(staff);
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
      int idx = score()->magIdx();
      if (idx == MAG_PAGE_WIDTH || idx == MAG_PAGE || idx == MAG_DBL_PAGE) {
            double m = mscore->getMag(this);
            setMag(m);
            }

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
            navigator->setScore(_score);  // adapt to new paper size
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

      if (ev->buttons() != ev->button()) {
#ifdef Q_WS_MAC
			//allow control click
			if (! (b3 && ev->buttons() == (Qt::LeftButton))){
#endif
			if (ev->buttons() == (Qt::LeftButton | Qt::RightButton)) {
                  ++level;
                  mouseReleaseEvent(ev);
                  b1 = true;
                  b3 = false;
                  }
            else
                  return;
            }
#ifdef Q_WS_MAC
			}
#endif

      if (state == MAG) {
            zoom(b1 ? 2 : -1, ev->pos());
            return;
            }

      Qt::KeyboardModifiers keyState = ev->modifiers();
      Qt::MouseButtons buttonState = ev->button();
      startMove   = imatrix.map(QPointF(ev->pos()));

      dragObject = elementNear(startMove);

      if (mscore->playEnabled() && dragObject && dragObject->type() == NOTE) {
            Note* note = static_cast<Note*>(dragObject);
            Part* part = note->staff()->part();
            int pitch  = note->ppitch();
            Instrument* i = part->instrument();
            seq->startNote(i->channel[note->subchannel()], pitch, 60, 1000, note->tuning());
            }

      //-----------------------------------------
      //  context menus
      //-----------------------------------------

      if (b3) {
            if (dragObject) {
                  ElementType type = dragObject->type();
                  _score->dragStaff = 0;
                  if (type == MEASURE) {
                        _score->dragSystem = (System*)(dragObject->parent());
                        _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                        }
                  // As findSelectableElement may return a measure
                  // when clicked "a little bit" above or below it, getStaff
                  // may not find the staff and return -1, which would cause
                  // select() to crash
                  if (_score->dragStaff >= 0) {
                        SelectType st = SELECT_SINGLE;
                        if (keyState == Qt::NoModifier)
                              st = SELECT_SINGLE;
                        else if (keyState & Qt::ShiftModifier)
                              st = SELECT_RANGE;
                        else if (keyState & Qt::ControlModifier)
                              st = SELECT_ADD;
                        _score->select(dragObject, st, _score->dragStaff);
                        }
                  seq->stopNotes();       // stop now because we dont get a mouseRelease event
                  if (type == MEASURE) {
                        measurePopup(ev->globalPos(), (Measure*)dragObject);
                        }
                  else {
                        objectPopup(ev->globalPos(), dragObject);
                        }
                  dragObject = 0;
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

                  if (dragObject) {
                        ElementType type = dragObject->type();
                        _score->dragStaff = 0;
                        if (type == MEASURE) {
                              _score->dragSystem = (System*)(dragObject->parent());
                              _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                              }
                        // As findSelectableElement may return a measure
                        // when clicked "a little bit" above or below it, getStaff
                        // may not find the staff and return -1, which would cause
                        // select() to crash
                        if (_score->dragStaff >= 0) {
                              SelectType st = SELECT_SINGLE;
                              if (keyState == Qt::NoModifier)
                                    st = SELECT_SINGLE;
                              else if (keyState & Qt::ShiftModifier)
                                    st = SELECT_RANGE;
                              else if (keyState & Qt::ControlModifier)
                                    st = SELECT_ADD;
                              _score->select(dragObject, st, _score->dragStaff);
                              }
                        else
                              dragObject = 0;
                        }
                  else {
                        // shift+drag selects "lasso mode"
                        if (!(keyState & Qt::ShiftModifier)) {
                              dragCanvasState = true;
                              draggedCanvas = false;
                              setCursor(Qt::SizeAllCursor);
                              }
                        update();
                        }
                  _score->setLayoutAll(false);
                  _score->end();    // update
                  break;

            case NOTE_ENTRY:
                  if (keyState & Qt::ControlModifier || ev->button() == Qt::MidButton) {
                        dragCanvasState = true;
                        // don't deselect when dragging in note entry mode
                        draggedCanvas = true;
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
                  else {
                        if (dragObject) {
                              ElementType type = dragObject->type();
                              _score->dragStaff = 0;
                              if (type == MEASURE) {
                                    _score->dragSystem = (System*)(dragObject->parent());
                                    _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                                    }
                              // As findSelectableElement may return a measure
                              // when clicked "a little bit" above or below it, getStaff
                              // may not find the staff and return -1, which would cause
                              // select() to crash
                              if (_score->dragStaff >= 0) {
                                    SelectType st = SELECT_SINGLE;
                                    if (keyState == Qt::NoModifier)
                                          st = SELECT_SINGLE;
                                    else if (keyState & Qt::ShiftModifier)
                                          st = SELECT_RANGE;
                                    else if (keyState & Qt::ControlModifier)
                                          st = SELECT_ADD;
                                    _score->select(dragObject, st, _score->dragStaff);
                                    }
                              else
                                    dragObject = 0;
                              }
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

      if (dragObject) {
            _score->startCmd();
            _score->setLayoutAll(false);
            if (!startEdit(dragObject)) {
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
      if (QApplication::mouseButtons() == Qt::MidButton && dragObject) {
            QString mimeType = _score->selection()->mimeType();
            if (!mimeType.isEmpty()) {
                  QDrag* drag = new QDrag(this);
                  drag->setPixmap(QPixmap());
                  QMimeData* mimeData = new QMimeData;
                  mimeData->setData(mimeType, _score->selection()->mimeData());
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
            if ((dx > 0 || dy < 0) && navigatorVisible()) {
	            QRect r(navigator->geometry());
            	r.translate(dx, dy);
            	update(r);
                  }
            updateNavigator(false);
            if (!draggedCanvas)
                  draggedCanvas = true;
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
                  if (dragObject && (QApplication::keyboardModifiers() == Qt::ShiftModifier)) {
                        // drag selection
                        QString mimeType = _score->selection()->mimeType();
                        if (!mimeType.isEmpty()) {
                              QDrag* drag = new QDrag(this);
                              QMimeData* mimeData = new QMimeData;
                              mimeData->setData(mimeType, score()->selection()->mimeData());
                              drag->setMimeData(mimeData);
                              _score->endCmd();
                              //
                              //  also set into the clipboard
                              //
                              QMimeData* clip = new QMimeData();
                              clip->setData(mimeType, score()->selection()->mimeData());
                              QApplication::clipboard()->setMimeData(clip);
                              drag->start(Qt::CopyAction);
                              }
                        break;
                        }
                  if (dragObject && dragObject->isMovable()) {
                        QPointF o;
                        if (_score->selection()->state() != SEL_STAFF && _score->selection()->state() != SEL_SYSTEM) {
                              _score->startDrag(dragObject);
                              o = QPointF(dragObject->userOff());
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
                  _score->lassoSelect(lasso->abbox());
                  break;

            case EDIT:
                  break;


            case DRAG_EDIT:
                  {
                  _score->setLayoutAll(false);
                  Element* e = _score->editObject;
                  score()->addRefresh(e->abbox());
                  e->editDrag(curGrip, delta);
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

      double dx = 1.5 / _matrix.m11();
      double dy = 1.5 / _matrix.m22();

      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));

      qreal w   = 8.0 / _matrix.m11();
      qreal h   = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < 4; ++i)
            grip[i] = r;

      e->updateGrips(&grips, grip);

      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));

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
      if (ev->button() == Qt::RightButton && ev->buttons() == Qt::LeftButton) {
            _score->endCmd();
            return;
            }

      if (ev->buttons() == 0)
            level = 0;
      if (dragCanvasState) {
            dragCanvasState = false;
            setCursor(QCursor(Qt::ArrowCursor));
            mousePressed = false;
            if (!draggedCanvas)
                  _score->select(0, SELECT_SINGLE, 0);
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
            if (QApplication::keyboardModifiers() & Qt::ShiftModifier)
                  return;
            setState(NORMAL);
            return;
            }
      switch (state) {
            case DRAG_EDIT:
                  setState(EDIT);
                  break;

            case LASSO:
                  setState(NORMAL);
                  _score->addRefresh(lasso->abbox().adjusted(-2, -2, 2, 2));
                  lasso->setbbox(QRectF());
                  _score->lassoSelectEnd(lasso->abbox());
                  break;

            case DRAG_OBJ:
                  setState(NORMAL);
                  break;

            case NOTE_ENTRY:
                  break;

            case NORMAL:
                  if (!dragObject)
                        _score->select(0, SELECT_SINGLE, 0);      // deselect all
                  break;

            default:
                  setState(NORMAL);
                  break;
            }

      // here we can be in state EDIT again
      if (state != EDIT)
            _score->endCmd();
      else
            _score->end();
      }

//---------------------------------------------------------
//   setBackground
//---------------------------------------------------------

void Canvas::setBackground(QPixmap* pm)
      {
      delete bgPixmap;
      bgPixmap = pm;
      update();
      }

void Canvas::setBackground(const QColor& color)
      {
      delete bgPixmap;
      bgPixmap = 0;
      _bgColor = color;
      update();
      }

//---------------------------------------------------------
//   setForeground
//---------------------------------------------------------

void Canvas::setForeground(QPixmap* pm)
      {
      delete fgPixmap;
      fgPixmap = pm;
      update();
      }

void Canvas::setForeground(const QColor& color)
      {
      delete fgPixmap;
      fgPixmap = 0;
      _fgColor = color;
      update();
      }

//---------------------------------------------------------
//   setState
//---------------------------------------------------------

void Canvas::setState(State action)
      {
      static const char* stateNames[] = {
            "NORMAL", "DRAG_OBJ", "EDIT", "DRAG_EDIT", "LASSO", "NOTE_ENTRY", "MAG"
            };
      int stateTable[7][7] = {
//action      NORMAL, DRAG_OBJ, EDIT, DRAG_EDIT, LASSO, NOTE_ENTRY, MAG
/*NORMAL     */ {  1,       99,    8,         0,     6,          2,   4 },
/*DRAG_OBJ   */ { 12,        1,    0,         0,     0,          0,   0 },
/*EDIT       */ {  9,        0,    1,        99,     0,          0,   0 },
/*DRAG_EDIT  */ { 11,        0,   13,         1,     0,          0,   0 },
/*LASSO      */ {  7,        0,    0,         0,     1,          0,   0 },
/*NOTE_ENTRY */ {  3,        0,   10,         0,     0,          1,   0 },
/*MAG        */ {  5,        0,    0,         0,     0,          0,   1 },
      };

      if (debugMode)
            printf("Canvas::setState: switch from %s to %s\n", stateNames[state], stateNames[action]);

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
            case 5:     // MAG        - NORMAL
                  setCursor(QCursor(Qt::ArrowCursor));
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
                  score()->setState(STATE_EDIT);
                  break;
            case  9:     // EDIT      - NORMAL
            case 11:     // DRAG_EDIT - NORMAL
                  state = action;
                  setDropTarget(0);
                  _score->endEdit();
                  setEditText(0);
                  break;
            case 10:    // NOTE_ENTRY - EDIT
                  state = action;
                  setCursor(QCursor(Qt::ArrowCursor));
                  setMouseTracking(false);
                  shadowNote->setVisible(false);
                  setCursorOn(false);
                  break;
            case 12:    // DRAG_OBJ - NORMAL
                  _score->endDrag();
                  setDropTarget(0); // this also resets dropAnchor
                  state = NORMAL;
                  break;
            case 13:    // DRAG_EDIT - EDIT
                  _score->addRefresh(_score->editObject->abbox());
                  _score->editObject->endEditDrag();
                  updateGrips();
                  setDropTarget(0); // this also resets dropRectangle and dropAnchor
                  _score->addRefresh(_score->editObject->abbox());
                  state = EDIT;
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
//   startEdit
//---------------------------------------------------------

bool Canvas::startEdit(Element* element, int startGrip)
      {
      dragObject = 0;
      if (element->startEdit(this, startMove)) {
            setFocus();
            _score->startEdit(element);
            setState(EDIT);
            qreal w = 8.0 / _matrix.m11();
            qreal h = 8.0 / _matrix.m22();
            QRectF r(-w*.5, -h*.5, w, h);
            for (int i = 0; i < 4; ++i)
                  grip[i] = r;
            _score->editObject->updateGrips(&grips, grip);
            if (startGrip == -1)
                  curGrip = grips-1;
            else if (startGrip >= 0)
                  curGrip = startGrip;
            // startGrip == -2  -> do not change curGrip

            // update();         // DEBUG
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

      double d = cursor->spatium() * .5;
      if (track == cursor->track() && cursor->tick() == _score->inputPos()) {
            _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2*d, 2*d));
            return;
            }

      cursor->setTrack(track);
      cursor->setTick(_score->inputPos());

      Segment* segment = _score->tick2segment(cursor->tick());
      if (segment) {
            moveCursor(segment, track / VOICES);
            return;
            }
// printf("cursor position not found for tick %d, append new measure\n", cursor->tick());
      }

void Canvas::moveCursor(Segment* segment, int staffIdx)
      {
      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            printf("zero SYSTEM\n");
            return;
            }
      cursor->setSegment(segment);
      int idx   = staffIdx == -1 ? 0 : staffIdx;
      double x  = segment->canvasPos().x();
      double y  = system->bboxStaff(idx).y() + system->canvasPos().y();
      double y2 = system->bboxStaff(_score->nstaves()-1).y() + system->canvasPos().y();

      double _spatium = cursor->spatium();
      double d = _spatium * .5;
      _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2*d, 2*d));
      cursor->setPos(x - _spatium, y - _spatium);
      if (staffIdx == -1)
            cursor->setHeight(y2 - y + 6.0 * _spatium);
      else
            cursor->setHeight(6.0 * _spatium);
      _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2*d, 2*d));
      cursor->setTick(segment->tick());
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
      Position pos;
      if (!score()->getPosition(&pos, p, score()->inputState().voice())) {
            shadowNote->setVisible(false);
            return;
            }

      shadowNote->setVisible(true);
      Staff* staff      = score()->staff(pos.staffIdx);
      shadowNote->setMag(staff->mag());
      Instrument* instr = staff->part()->instrument();
      int notehead      = 0;
      int line          = pos.line;

      if (instr->useDrumset) {
            Drumset* ds  = instr->drumset;
            int pitch    = score()->inputState().drumNote;
            if (pitch >= 0 && ds->isValid(pitch)) {
                  line     = ds->line(pitch);
                  notehead = ds->noteHead(pitch);
                  }
            }
      shadowNote->setLine(line);
      shadowNote->setHeadGroup(notehead);
      shadowNote->setPos(pos.pos);
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
//   paintElement
//---------------------------------------------------------

static void paintElement(void* data, Element* e)
      {
      QPainter* p = static_cast<QPainter*>(data);
      p->save();
      p->translate(e->canvasPos());
      e->draw(*p);
      p->restore();
      }

//---------------------------------------------------------
//   paintEvent
//    Note: desktop background and paper background are not
//    scaled
//---------------------------------------------------------

void Canvas::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      QRegion region;
      if (_score->needLayout()) {

//            unsigned long long a = cycles();
// printf("paint event layout\n");
            _score->doLayout();
//            unsigned long long b = (cycles() - a) / 1000000LL;
//            printf("layout %lld\n", b);

            if (navigator)
                  navigator->layoutChanged();
            if (state == EDIT || state == DRAG_EDIT)
                  updateGrips();
            region = QRegion(0, 0, width(), height());
            }
      else
            region = ev->region();

      const QVector<QRect>& vector = region.rects();
      foreach(const QRect& r, vector)
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
            p.setPen(preferences.defaultColor);
            dragElement->scanElements(&p, paintElement);
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void Canvas::paint(const QRect& rr, QPainter& p)
      {
// printf("paint %d %d -- %d %d\n", rr.x(), rr.y(), rr.width(), rr.height());
      p.save();
      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(rr, _fgColor);
      else {
            p.drawTiledPixmap(rr, *fgPixmap, rr.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }

      p.setMatrix(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(rr));

      QRegion r1(rr);
      foreach (const Page* page, _score->pages())
            r1 -= _matrix.mapRect(page->abbox()).toAlignedRect();
//      p.setClipRect(fr);

      QList<const Element*> ell = _score->items(fr);
      drawElements(p, ell);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      if (_editText) {
            QRectF r = _editText->abbox();
            qreal w = 6.0 / matrix().m11();   // 6 pixel border
            r.adjust(-w, -w, w, w);
            w = 2.0 / matrix().m11();   // 2 pixel pen size
            p.setPen(QPen(QBrush(Qt::blue), w));
            p.setBrush(QBrush(Qt::NoBrush));
            p.drawRect(r);
            }

      if (state == EDIT || state == DRAG_EDIT) {
            qreal lw = 2.0/p.matrix().m11();
            // QPen pen(Qt::blue);
            QPen pen(preferences.defaultColor);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  p.setBrush(i == curGrip ? QBrush(Qt::blue) : Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
            }
      Selection* sel = _score->selection();

      if (sel->state() == SEL_STAFF || sel->state() == SEL_SYSTEM) {
            Segment* ss = sel->startSegment();
            Segment* es = sel->endSegment();

            p.setBrush(Qt::NoBrush);

            QPen pen(QColor(Qt::blue));
            pen.setWidthF(2.0 / p.matrix().m11());

            if (sel->state() == SEL_SYSTEM){
                  pen.setStyle(Qt::DotLine);
				  #ifdef Q_WS_MAC
				  //TODO: remove if qt fix. This is a workaround for a qt bug on mac apparenlty
				  //For dotline the spaces are not proportional to the line width except for custom dash
				  QVector<qreal> dashes;
				  qreal space = 2;
				  dashes << 2 << space << 2;
				  pen.setDashPattern(dashes);
				  #endif
				  }
            else
                  pen.setStyle(Qt::SolidLine);

            p.setPen(pen);
            double _spatium = score()->spatium();
            double x2      = ss->canvasPos().x() - _spatium;
            int staffStart = sel->staffStart;
            int staffEnd   = sel->staffEnd;

            System* system2 = ss->measure()->system();
            QPointF pt      = ss->canvasPos();
            double y        = pt.y();
            SysStaff* ss1   = system2->staff(staffStart);
            SysStaff* ss2   = system2->staff(staffEnd - 1);
            double y1       = ss1->y() - 2 * _spatium + y;
            double y2       = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;

            // drag vertical start line
            p.drawLine(QLineF(x2, y1, x2, y2));

            System* system1 = system2;
            double x1;

            for (Segment* s = ss; s && (s != es);) {
                  Segment* ns = s->nextCR();
                  system1  = system2;
                  system2  = s->measure()->system();
                  pt       = s->canvasPos();
                  x1  = x2;
                  x2  = pt.x() + _spatium * 2;

                  // HACK for whole measure rest:
                  if (ns == 0 || ns == es) {    // last segment?
                        Element* e = s->element(staffStart * VOICES);
                        if (e && e->type() == REST && static_cast<Rest*>(e)->duration().type() == Duration::V_MEASURE)
                              x2 = s->measure()->abbox().right() - _spatium;
                        }

                  if (system2 != system1)
                        x1  = x2 - 2 * _spatium;
                  y   = pt.y();
                  ss1 = system2->staff(staffStart);
                  ss2 = system2->staff(staffEnd - 1);
                  y1  = ss1->y() - 2 * _spatium + y;
                  y2  = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;
                  p.drawLine(QLineF(x1, y1, x2, y1));
                  p.drawLine(QLineF(x1, y2, x2, y2));
                  s = ns;
                  }
            //
            // draw vertical end line
            //
            p.drawLine(QLineF(x2, y1, x2, y2));
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

      if ((dx > 0 || dy < 0) && navigatorVisible()) {
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
      int tick;
      MeasureBase* mb = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, 0);
      if (mb && mb->type() == MEASURE) {
            Measure* m = (Measure*)mb;
            System* s  = m->system();
            QRectF sb(s->staff(staffIdx)->bbox());
            sb.translate(s->pos() + s->page()->pos());
            QPointF anchor(seg->abbox().x(), sb.topLeft().y());
            setDropAnchor(QLineF(pos, anchor));
            dragElement->setTrack(staffIdx * VOICES);
            dragElement->setTick(tick);
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
      double _spatium = score()->spatium();
      delete dragElement;
      dragElement = 0;

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

            if (debugMode)
                  printf("Canvas::dragEnterEvent: <%s>\n", a.data());

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

            dragOffset = QPoint();
            ElementType type = Element::readType(e, &dragOffset);

            Element* el = 0;
            switch(type) {
                  case SLUR:
                        el = Element::create(type, gscore);
                        break;
                  case VOLTA:
                  case OTTAVA:
                  case TRILL:
                  case PEDAL:
                  case HAIRPIN:
                  case TEXTLINE:
                        el = Element::create(type, gscore);
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
                        QString lp(path.toLower());

                        if (lp.endsWith(".svg"))
                              image = new SvgImage(score());
                        else if (lp.endsWith(".jpg")
                           || lp.endsWith(".png")
                           || lp.endsWith(".gif")
                           || lp.endsWith(".xpm")
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
                  case GLISSANDO:
                  case ARTICULATION:
                  case ACCIDENTAL:
                  case DYNAMIC:
                  case TEXT:
                  case TEMPO_TEXT:
                  case STAFF_TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case NOTE:
                  case SYMBOL:
                  case CHORD:
                  case SPACER:
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
                  dragElement->layout();
                  }
            }

      else if (data->hasUrls()) {
            QList<QUrl>ul = data->urls();
            foreach(const QUrl& u, ul) {
                  if (debugMode)
                        printf("drag Url: %s\n", qPrintable(u.toString()));
                  if (u.scheme() == "file") {
                        QFileInfo fi(u.path());
                        QString suffix = fi.suffix().toLower();
                        if (suffix == "svg"
                           || suffix == "jpg"
                           || suffix == "png"
                           || suffix == "gif"
                           || suffix == "xpm"
                           ) {
                              event->acceptProposedAction();
                              break;
                              }
                        }
                  }
            }
      else {
            QString s = QString("unknown drop format: formats %1:\n").arg(data->hasFormat(mimeSymbolFormat));
            foreach(QString ss, data->formats())
                  s += (QString("   <%1>\n").arg(ss));
            QMessageBox::warning(0,
               "Drop:", s, QString::null, "Quit", QString::null, 0, 1);
            }
      }

//---------------------------------------------------------
//   dragSymbol
//    drag SYMBOL and IMAGE elements
//---------------------------------------------------------

void Canvas::dragSymbol(const QPointF& pos)
      {
      const QList<const Element*> el = elementsAt(pos);
      const Element* e = el.isEmpty() ? 0 : el[0];
      if (e && (e->type() == NOTE || e->type() == SYMBOL || e->type() == IMAGE)) {
            if (e->acceptDrop(this, pos, dragElement->type(), dragElement->subtype()))
                  return;
            else {
                  setDropTarget(0);
                  return;
                  }
            }
      dragTimeAnchorElement(pos);
      }

//---------------------------------------------------------
//   dragMoveEvent
//---------------------------------------------------------

void Canvas::dragMoveEvent(QDragMoveEvent* event)
      {
      // we always accept the drop action
      // to get a "drop" Event:

      event->acceptProposedAction();

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
                  case IMAGE:
                  case SYMBOL:
                        dragSymbol(pos);
                        break;
                  case KEYSIG:
                  case CLEF:
                  case TIMESIG:
                  case BAR_LINE:
                  case ARPEGGIO:
                  case BREATH:
                  case GLISSANDO:
                  case BRACKET:
                  case ARTICULATION:
                  case ACCIDENTAL:
                  case TEXT:
                  case TEMPO_TEXT:
                  case STAFF_TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case CHORD:
                  case SPACER:
                  case SLUR:
                        {
                        Element* el = elementAt(pos);
                        if (el && el->acceptDrop(this, pos, dragElement->type(), dragElement->subtype()))
                              break;
                        setDropTarget(0);
                        }
                        break;
                  default:
                        break;
                  }
            score()->addRefresh(dragElement->abbox());
            QRectF r(dragElement->abbox());
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
                  QString suffix(fi.suffix().toLower());
                  if (suffix != "svg"
                     && suffix != "jpg"
                     && suffix != "png"
                     && suffix != "gif"
                     && suffix != "xpm"
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
            dragElement->setScore(_score);
            _score->addRefresh(dragElement->abbox());
            switch(dragElement->type()) {
                  case VOLTA:
                  case OTTAVA:
                  case TRILL:
                  case PEDAL:
                  case DYNAMIC:
                  case HAIRPIN:
                  case TEXTLINE:
                        dragElement->setScore(score());
                        score()->cmdAdd1(dragElement, pos, dragOffset);
                        event->acceptProposedAction();
                        break;
                  case SYMBOL:
                  case IMAGE:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0) {
                              int staffIdx = -1;
                              Segment* seg;
                              int tick;
                              el = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, 0);
                              if (el == 0) {
                                    printf("cannot drop here\n");
                                    delete dragElement;
                                    break;
                                    }
                             }
                        _score->addRefresh(el->abbox());
                        _score->addRefresh(dragElement->abbox());
                        Element* dropElement = el->drop(pos, dragOffset, dragElement);
                        _score->addRefresh(el->abbox());
                        if (dropElement) {
                              _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->abbox());
                              }
                        }
                        event->acceptProposedAction();
                        break;
                  case KEYSIG:
                  case CLEF:
                  case TIMESIG:
                  case BAR_LINE:
                  case ARPEGGIO:
                  case BREATH:
                  case GLISSANDO:
                  case BRACKET:
                  case ARTICULATION:
                  case ACCIDENTAL:
                  case TEXT:
                  case TEMPO_TEXT:
                  case STAFF_TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case MARKER:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case NOTE:
                  case CHORD:
                  case SPACER:
                  case SLUR:
                        {
                        Element* el = elementAt(pos);
                        if (!el) {
                              printf("cannot drop here\n");
                              delete dragElement;
                              break;
                              }
                        _score->addRefresh(el->abbox());
                        _score->addRefresh(dragElement->abbox());
                        Element* dropElement = el->drop(pos, dragOffset, dragElement);
                        _score->addRefresh(el->abbox());
                        if (dropElement) {
                              if (!_score->noteEntryMode())
                                    _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->abbox());
                              }
                        event->acceptProposedAction();
                        }
                        break;
                  default:
                        delete dragElement;
                        break;
                  }
            score()->endCmd();
            dragElement = 0;
            setDropTarget(0); // this also resets dropRectangle and dropAnchor
//DEBUG            setState(NORMAL);
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = 0;
                  QString suffix = fi.suffix().toLower();
                  if (suffix == "svg")
                        s = new SvgImage(score());
                  else if (suffix == "jpg"
                     || suffix == "png"
                     || suffix == "gif"
                     || suffix == "xpm"
                        )
                        s = new RasterImage(score());
                  else
                        return;
                  _score->startCmd();
                  QString str(u.path());
                  if (str.startsWith("/C:/"))    // HACK
                        str = str.mid(1);
                  s->setPath(str);
if (debugMode)
      printf("drop image <%s> <%s>\n", qPrintable(str), qPrintable(s->path()));

                  Element* el = elementAt(pos);
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        s->setTrack(el->track());
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
            QStringList sl = md->formats();
            foreach(QString s, sl)
                  printf("  %s\n", qPrintable(s));
            _score->end();
            return;
            }

// printf("drop <%s>\n", data.data());

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            qWarning("error reading drag data at %d/%d: %s\n   %s\n",
               line, column, qPrintable(err), data.data());
            return;
            }
      docName = "--";
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
            int idx   = s->y2staff(pos.y());
            if (idx != -1) {
                  Segment* seg = measure->first();
                  // assume there is always a ChordRest segment
                  while (seg->subtype() != Segment::SegChordRest)
                        seg = seg->next();
                  score()->pasteStaff(doc.documentElement(), (ChordRest*)(seg->element(idx * VOICES)));
                  }
            event->acceptProposedAction();
            _score->setLayoutAll(true);
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
            _score->setLayoutAll(false);
            _score->addRefresh(dragElement->abbox());
            delete dragElement;
            dragElement = 0;
            _score->end();
            }
      setDropTarget(0);
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void Canvas::zoom(int step, const QPoint& pos)
      {
      QPointF p1 = imatrix.map(QPointF(pos));
      //
      //    magnify
      //
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

      QPointF p2 = imatrix.map(QPointF(pos));
      QPointF p3 = p2 - p1;

      double m = _mag * PDPI/DPI;

      int dx    = lrint(p3.x() * m);
      int dy    = lrint(p3.y() * m);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), _matrix.dx()+dx, _matrix.dy()+dy);
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));

      if ((dx > 0 || dy < 0) && navigatorVisible()) {
	      QRect r(navigator->geometry());
      	r.translate(dx, dy);
      	update(r);
            }
      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void Canvas::wheelEvent(QWheelEvent* event)
      {
      if (event->modifiers() & Qt::ControlModifier) {
            QApplication::sendPostedEvents(this, 0);
            zoom(event->delta() / 120, event->pos());
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
      if ((dy < 0 || dx > 0) && navigatorVisible()) {
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
//   elementsAt
//---------------------------------------------------------

const QList<const Element*> Canvas::elementsAt(const QPointF& p)
      {
      QList<const Element*> el = _score->items(p);
      qSort(el.begin(), el.end(), elementLower);
      return el;
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* Canvas::elementAt(const QPointF& p)
      {
      QList<const Element*> el = elementsAt(p);
      if (el.empty())
            return 0;
#if 0
      printf("elementAt\n");
      foreach(const Element* e, el)
            printf("  %s %d\n", e->name(), e->selected());
#endif
      return const_cast<Element*>(el.at(0));
      }

//---------------------------------------------------------
//   elementNear
//---------------------------------------------------------

Element* Canvas::elementNear(const QPointF& p)
      {
      double w  = (preferences.proximity * .5) / matrix().m11();
      QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

      QList<const Element*> el = _score->items(r);
      QList<const Element*> ll;
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            e->itemDiscovered = 0;
            if (e->selectable() && e->contains(p))
                  ll.append(e);
            }
      int n = ll.size();
      if ((n == 0) || ((n == 1) && (ll[0]->type() == MEASURE))) {
            //
            // if no relevant element hit, look nearby
            //
            for (int i = 0; i < el.size(); ++i) {
                  const Element* e = el.at(i);
                  if (e->abbox().intersects(r) && e->selectable())
                        ll.append(e);
                  }
            }
      if (ll.empty())
            return 0;
      qSort(ll.begin(), ll.end(), elementLower);

#if 0
      printf("elementNear ========= %f\n", w);
      foreach(const Element* e, el)
            printf("  %s %d\n", e->name(), e->selected());
#endif
      Element* e = const_cast<Element*>(ll.at(level % ll.size()));
      return e;
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void Canvas::drawElements(QPainter& p,const QList<const Element*>& el)
      {
      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (score()->printing() || !score()->showInvisible())
                        continue;
                  }
            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(p);

            if (debugMode && e->selected()) {
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
                        if (e->parent()->type() == SEGMENT) {
                              qreal w = 7.0 / p.matrix().m11();
                              QPointF pt = e->parent()->canvasPos();
                              p.setPen(QPen(Qt::blue, 2, Qt::SolidLine));
                              p.drawLine(QLineF(pt.x()-w, pt.y()-h, pt.x()+w, pt.y()+h));
                              p.drawLine(QLineF(pt.x()+w, pt.y()-h, pt.x()-w, pt.y()+h));
                              }
                        continue;
                        }
                  }
            p.restore();
            }
      Element* e = score()->dragObject();
      if (e) {
            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(p);
            p.restore();
            }
      e = score()->editObject;
      if (e) {
            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(p);
            p.restore();
            }
      }

//---------------------------------------------------------
//   paintLasso
//---------------------------------------------------------

void Canvas::paintLasso(QPainter& p, double mag)
      {
      QRectF r = _matrix.mapRect(lassoRect());
      double x = r.x();
      double y = r.y();

      QMatrix omatrix(_matrix);

      double imag = 1.0 / mag;

      _matrix.setMatrix(_matrix.m11() * mag, _matrix.m12(), _matrix.m21(),
         _matrix.m22() * mag, (_matrix.dx()-x) * imag, (_matrix.dy()-y) * imag);
      imatrix = _matrix.inverted();

      p.setMatrix(_matrix);
      p.setRenderHint(QPainter::Antialiasing, true);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      QList<const Element*> el = _score->items(QRectF(0.0, 0.0, 100000.0, 1000000.0));
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

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void Canvas::setMag(double nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;
      nmag *= (PDPI/DPI);

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m21(), nmag,
         _matrix.dx() * deltamag, _matrix.dy() * deltamag);
      imatrix = _matrix.inverted();

      update();
      updateNavigator(false);
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Canvas::mag() const
      {
      return _matrix.m11() *  DPI/PDPI;
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void Canvas::setOffset(qreal x, qreal y)
      {
      double m = PDPI / DPI;
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), x * m, y * m);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal Canvas::xoffset() const
      {
      return _matrix.dx() * DPI / PDPI;
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal Canvas::yoffset() const
      {
      return _matrix.dy() * DPI / PDPI;
      }


