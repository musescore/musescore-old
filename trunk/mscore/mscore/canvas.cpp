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
#include "painter.h"
#include "navigator.h"
#include "chord.h"
#include "page.h"
#include "note.h"
#include "xml.h"

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

Canvas::Canvas(QWidget* parent)
   : QWidget(parent)
      {
      setAcceptDrops(true);
//      setAttribute(Qt::WA_OpaquePaintEvent);
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
      dropTarget       = 0;

      setXoffset(30);
      setYoffset(30);

      state            = NORMAL;
      cursor           = new Cursor(_score, 6);
      shadowNote       = new ShadowNote(_score);
      cursorTimer      = new QTimer(this);
      buttonState      = 0;
      keyState         = 0;

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
//   readType
//---------------------------------------------------------

static int readType(QDomNode& node)
      {
      int type = 0;
      for (; !node.isNull(); node = node.nextSibling()) {
            QDomElement e = node.toElement();
            if (e.isNull())
                  continue;
                  //
                  // DEBUG:
                  // check names; remove non needed elements
                  //
                  if (e.tagName() == "Dynamic")
                        type = DYNAMIC;
                  else if (e.tagName() == "Symbol")
                        type = SYMBOL;
                  else if (e.tagName() == "Text")
                        type = TEXT;
                  else if (e.tagName() == "Staff")
                        type = STAFF;
                  else if (e.tagName() == "Slur")
                        type = SLUR_SEGMENT;
                  else if (e.tagName() == "Note")
                        type = NOTE;
                  else if (e.tagName() == "BarLine")
                        type = BAR_LINE;
                  else if (e.tagName() == "Stem")
                        type = STEM;
                  else if (e.tagName() == "Bracket")
                        type = BRACKET;
                  else if (e.tagName() == "Accidental")
                        type = ACCIDENTAL;
                  else if (e.tagName() == "Clef")
                        type = CLEF;
                  else if (e.tagName() == "KeySig")
                        type = KEYSIG;
                  else if (e.tagName() == "TimeSig")
                        type = TIMESIG;
                  else if (e.tagName() == "Chord")
                        type = CHORD;
                  else if (e.tagName() == "Rest")
                        type = REST;
                  else if (e.tagName() == "Tie")
                        type = TIE;
                  else if (e.tagName() == "Slur")
                        type = SLUR;
                  else if (e.tagName() == "Measure")
                        type = MEASURE;
                  else if (e.tagName() == "Attribute")
                        type = ATTRIBUTE;
                  else if (e.tagName() == "Page")
                        type = PAGE;
                  else if (e.tagName() == "Beam")
                        type = BEAM;
                  else if (e.tagName() == "Hook")
                        type = HOOK;
                  else if (e.tagName() == "Lyric")
                        type = LYRICS;
                  else if (e.tagName() == "Instrument1")
                        type = INSTRUMENT_NAME1;
                  else if (e.tagName() == "Instrument2")
                        type = INSTRUMENT_NAME2;
                  else if (e.tagName() == "System")
                        type = SYSTEM;
                  else if (e.tagName() == "HairPin")
                        type = HAIRPIN;
                  else if (e.tagName() == "Tuplet")
                        type = TUPLET;
                  else if (e.tagName() == "VSpacer")
                        type = VSPACER;
                  else if (e.tagName() == "Segment")
                        type = SEGMENT;
                  else if (e.tagName() == "TempoText")
                        type = TEMPO_TEXT;
                  else if (e.tagName() == "Volta")
                        type = VOLTA;
                  else if (e.tagName() == "Ottava")
                        type = OTTAVA;
                  else if (e.tagName() == "Pedal")
                        type = PEDAL;
                  else if (e.tagName() == "Trill")
                        type = TRILL;
                  else if (e.tagName() == "LayoutBreak")
                        type = LAYOUT_BREAK;
                  else if (e.tagName() == "HelpLine")
                        type = HELP_LINE;
                  else
                        domError(node);
                  break;
                  }
      return type;
      }

//---------------------------------------------------------
//   cavasPopup
//---------------------------------------------------------

void Canvas::canvasPopup(const QPoint& pos)
      {
      QMenu* popup = mscore->genCreateMenu();
      popup->popup(pos);
      }

//---------------------------------------------------------
//   objectPopup
//---------------------------------------------------------

void Canvas::objectPopup(const QPoint& pos, Element* obj)
      {
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
      a = popup->addAction(tr("Context"));
      a->setData("context");
      a = popup->exec(pos);
      if (a == 0)
            return;
      _score->startCmd();
      QString cmd(a->data().toString());
      if (cmd == "cut") {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData("application/mscore/symbol", obj->mimeData());
            QApplication::clipboard()->setMimeData(mimeData);
            _score->deleteItem(obj);
            }
      else if (cmd == "copy") {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData("application/mscore/symbol", obj->mimeData());
            QApplication::clipboard()->setMimeData(mimeData);
            }
      else if (cmd == "paste") {
            const QMimeData* ms = QApplication::clipboard()->mimeData();
            if (ms && ms->hasFormat("application/mscore/symbol")) {
                  QByteArray data(ms->data("application/mscore/symbol"));
                  QDomDocument doc;
                  int line, column;
                  QString err;
                  if (!doc.setContent(data, &err, &line, &column)) {
                        printf("error reading paste data\n");
                        return;
                        }
                  QDomNode node = doc.documentElement();
                  int type      = readType(node);
                  _score->addRefresh(obj->abbox());   // layout() ?!
                  obj->drop(pos, type, node);
                  _score->addRefresh(obj->abbox());
                  }
            }
      else if (cmd == "context")
            mscore->showElementContext(obj);
      else if (cmd == "invisible")
            _score->toggleInvisible(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
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
      bool b1 = ev->button() == Qt::LeftButton;
      bool b2 = ev->button() == Qt::MidButton;
      bool b3 = ev->button() == Qt::RightButton;

      keyState = ev->modifiers();
      if (state == MAG) {
            if (b1)
                  incMag();
            else if (b3)
                  decMag();
            return;
            }

      buttonState = ev->button();
      startMove   = imatrix.map(QPointF(ev->pos()));

      Element* element = _score->findSelectableElement(startMove);

      //
      if (state == EDIT && element == score()->editElement())
            return;

      _score->setDragObject(element);

      if (seq && mscore->playEnabled() && _score->dragObject() && _score->dragObject()->type() == NOTE) {
            Note* note = (Note*)(_score->dragObject());
            Staff* staff = note->staff();
            seq->startNote(staff->midiChannel(), note->pitch(), 60);
            }

      //-----------------------------------------
      //  context menus
      //-----------------------------------------

      if (b3) {
            if (_score->dragObject()) {
                  _score->select(_score->dragObject(), 0, 0);
                  seq->stopNotes(); // stop now because we dont get a mouseRelease event
                  objectPopup(ev->globalPos(), _score->dragObject());
                  }
            else
                  canvasPopup(ev->globalPos());
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
                  //  paste operation
                  //-----------------------------------------

                  if (b2 && (_score->sel->state == SEL_SINGLE)) {
                        _score->paste(_score->sel->element(), startMove);
                        break;
                        }

                  //-----------------------------------------
                  //  select operation
                  //-----------------------------------------

                  if (_score->dragObject()) {
                        ElementType type = _score->dragObject()->type();
                        _score->dragStaff = 0;  // WS
                        if (type == MEASURE) {
                              _score->dragSystem = (System*)(_score->dragObject()->parent());
                              _score->dragStaff  = getStaff(_score->dragSystem, startMove);
                              }
                        // As findSelectableElement may return a measure
                        // when clicked "a little bit" above or below it, getStaff
                        // may not find the staff and return -1, which would cause
                        // select() to crash
                        if (_score->dragStaff >= 0)
                              _score->select(_score->dragObject(), keyState, _score->dragStaff);
                        else
                              _score->setDragObject(0);
                        }
                  break;

            case EDIT:
                  if (_score->editObject->startEditDrag(startMove - _score->editObject->aref()))
                        setState(DRAG_EDIT);
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
            if (element->type() == NOTE) {
                  keyState = ev->modifiers();
                  _score->setDragObject(((Note*)element)->chord());
                  _score->select(_score->dragObject(), keyState, _score->dragStaff);
                  }
            else {
                  startEdit(element);
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
      mouseMoveEvent1(ev);
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
            QPoint d = ev->pos() - matrix.map(startMove).toPoint();
            int dx   = d.x();
            int dy   = d.y();
            QApplication::sendPostedEvents(this, 0);

            matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
               matrix.m22(), matrix.dx()+dx, matrix.dy()+dy);
            imatrix = matrix.inverted();
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
                  if (sqrt(pow(delta.x(),2)+pow(delta.y(),2))*matrix.m11() <= 2.0)
                        return;
                  if (_score->dragObject() && _score->dragObject()->isMovable()) {
                        QPointF o;
                        if (_score->sel->state == SEL_STAFF || _score->sel->state == SEL_SYSTEM) {
                              double s(_score->dragSystem->distance(_score->dragStaff));
                              o = QPointF(0.0, mag() * s);
                              setState(DRAG_STAFF);
                              }
                        else {
                              _score->startDrag();
                              o = QPointF(_score->dragObject()->userOff() * _spatium);
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
                  _score->dragEdit(matrix, &startMove, delta);
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
#if 0
      if (state == EDIT)
            _score->end();
      else
#endif
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
                  _score->addRefresh(_score->editObject->abbox());
                  setState(EDIT);
                  break;

            case LASSO:
                  setState(NORMAL);
                  _score->addRefresh(lasso->abbox());
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
      matrix.setMatrix(m, matrix.m12(), matrix.m21(),
         m * qreal(appDpiY)/qreal(appDpiX), matrix.dx(), matrix.dy());
      imatrix = matrix.inverted();

      update();
      updateNavigator(false);
      emit magChanged();
      }

//---------------------------------------------------------
//   incMag
//---------------------------------------------------------

void Canvas::incMag()
      {
      qreal _mag = mag() * 1.7;
      if (_mag > 16.0)
            _mag = 16.0;
      setMag(_mag);
      emit magChanged();
      }

//---------------------------------------------------------
//   decMag
//---------------------------------------------------------

void Canvas::decMag()
      {
      qreal nmag = mag() / 1.7;
      if (nmag < 0.05)
            nmag = 0.05;
      setMag(nmag);
      emit magChanged();
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
            case NORMAL:
            case EDIT:
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
      if (state == EDIT && s != EDIT && s != DRAG_EDIT) {
            endEdit();
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

void Canvas::dataChanged(Score*, const QRectF& r)
      {
      redraw(r);
      }

//---------------------------------------------------------
//   redraw
//---------------------------------------------------------

void Canvas::redraw(const QRectF& fr)
      {
      update(matrix.mapRect(fr).toRect());  // generate paint event
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
      if (element->startEdit(matrix)) {
            setFocus();
            _score->startEdit(element);
            setState(EDIT);
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Canvas::endEdit()
      {
      _score->endEdit();
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
                  if (segment->segmentType() != Segment::SegChordRest)
                        continue;
                  if (segment->tick() == tick)
                        break;
                  }
            if (segment) {
                  _score->adjustCanvasPosition(segment);
                  System* system = measure->system();
                  double x = segment->x() + measure->aref().x();
                  double y = system->bboxStaff(staff).y() + system->aref().y();
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

      double y = seg->apos().y() + s->staff(staff->idx())->bbox().y();
      y += line * _spatium * .5;

      shadowNote->setPos(seg->apos().x(), y);
      shadowNote->layout();
      }

//---------------------------------------------------------
//   mag
//---------------------------------------------------------

qreal Canvas::mag() const
      {
      return matrix.m11();
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
// printf("Canvas::paintEvent() %d %d %d %d\n", r.x(), r.y(), r.width(), r.height());
// unsigned long long now = cycles();
      QRect rr;
      if (_score->needLayout()) {
// printf("   doLayout()\n");
            _score->doLayout();
            if (navigator)
                  navigator->layoutChanged();
            rr.setRect(0, 0, width(), height());  // does not work because paintEvent
                                                  // is clipped?
            paint(rr);
            }
      else {
            int dx = lrint(matrix.m11());
            int dy = lrint(matrix.m22());

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
      Painter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);

      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(rr, _fgColor);
      else {
            p.drawTiledPixmap(rr, *fgPixmap, rr.topLeft()-QPoint(lrint(xoffset()), lrint(yoffset())));
            }

      p.setMatrix(matrix);
      QRectF fr = imatrix.mapRect(QRectF(rr));
      p.setClipRect(fr);

      QRegion r1(rr);

      for (iPage ip = _score->pages()->begin(); ip != _score->pages()->end(); ++ip) {
            Page* page = *ip;
            QRectF pbbox(page->abbox());
            r1 -= matrix.mapRect(pbbox).toRect();
            if (fr.intersects(pbbox)) {
                  p.translate(page->pos());
                  page->draw(p);
                  p.translate(-page->pos());
                  }
            }

      p.setClipRect(fr);
      lasso->draw(p);
      cursor->draw(p);
      shadowNote->draw(p);
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

void Canvas::setScore(Score* s)
      {
      _score = s;
      if (_score->needLayout())
            _score->doLayout();
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
      QRectF rr = matrix.mapRect(r);
      QPoint d = rr.topLeft().toPoint();
      int dx   = -d.x();
      int dy   = -d.y();
      QApplication::sendPostedEvents(this, 0);
      matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
         matrix.m22(), matrix.dx()+dx, matrix.dy()+dy);
      imatrix = matrix.inverted();
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
//   dragMoveEvent
//---------------------------------------------------------

void Canvas::dragMoveEvent(QDragMoveEvent* event)
      {
      if (!event->mimeData()->hasFormat("application/mscore/symbol"))
            return;
      // convert window to canvas position
      QPointF pos(imatrix.map(QPointF(event->pos())));
      Element* el = _score->findSelectableElement(pos);
      if (el) {
            QByteArray data(event->mimeData()->data("application/mscore/symbol"));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading drag data\n");
                  return;
                  }

            QDomNode node = doc.documentElement();
            int type      = readType(node);

            bool val = el->acceptDrop(pos, type, node);
            if (val)
                  event->accept();
            else {
                  if (debugMode)
                        printf("ignore drop of %s\n", el->name());
                  event->ignore();
                  }
            // DEBUG:
            if (dropTarget != el) {
                  if (dropTarget) {
                        dropTarget->setDropTarget(false);
                        _score->addRefresh(dropTarget->abbox());
                        dropTarget = 0;
                        }
                  if (type == ATTRIBUTE && el->type() == NOTE) {
                        dropTarget = el;
                        dropTarget->setDropTarget(true);
                        _score->addRefresh(dropTarget->abbox());
                        }
                  }
            }
      else {
            event->ignore();
            if (dropTarget) {
                  dropTarget->setDropTarget(false);
                  _score->addRefresh(dropTarget->abbox());
                  dropTarget = 0;
                  }
            }
      _score->end();
      }

//---------------------------------------------------------
//   dropEvent
//---------------------------------------------------------

void Canvas::dropEvent(QDropEvent* event)
      {
      if (!event->mimeData()->hasFormat("application/mscore/symbol"))
            return;
      QPointF pos(imatrix.map(QPointF(event->pos())));
      Element* el = _score->findSelectableElement(pos);
      if (el) {
            QByteArray data(event->mimeData()->data("application/mscore/symbol"));
            QDomDocument doc;
            int line, column;
            QString err;
            if (!doc.setContent(data, &err, &line, &column)) {
                  printf("error reading drag data\n");
                  return;
                  }
            QDomNode node = doc.documentElement();
            int type      = readType(node);

            _score->startCmd();
            _score->addRefresh(el->abbox());
            el->drop(pos, type, node);
            _score->addRefresh(el->abbox());
            }
      if (dropTarget) {
            dropTarget->setDropTarget(false);
            _score->addRefresh(dropTarget->abbox());
            dropTarget = 0;
            }
      event->acceptProposedAction();
      _score->endCmd(true);
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
      int n = height() / 20;
      if (n < 2)
            n = 2;
      int dy = event->delta() * n / 120;
      matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
	   matrix.m22(), matrix.dx(), matrix.dy() + dy);
	imatrix = matrix.inverted();

	scroll(0, dy, QRect(0, 0, width(), height()));

	//
      // this is necessary at least for qt4.1:
      //
      if ((dy < 0) && navigator->isVisible()) {
		QRect r(navigator->geometry());
		r.translate(0, dy);
		update(r);
            }
	updateNavigator(false);
      }

