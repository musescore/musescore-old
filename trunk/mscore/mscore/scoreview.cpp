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
 Implementation of most part of class ScoreView.
*/

#include "globals.h"
#include "scoreview.h"
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
#include "splitstaff.h"
#include "barline.h"
#include "system.h"
#include "magbox.h"
#include "measure.h"
#include "drumroll.h"
#include "lyrics.h"
#include "textpalette.h"
#include "undo.h"
#include "slur.h"
#include "harmony.h"

//---------------------------------------------------------
//   stateNames
//---------------------------------------------------------

static const char* stateNames[] = {
      "Normal", "Drag", "DragObject", "Edit", "DragEdit",
      "Lasso",  "NoteEntry", "Mag", "Play", "Search"
      };

//---------------------------------------------------------
//   CommandTransition
//---------------------------------------------------------

class CommandTransition : public QAbstractTransition
      {
      QString val;

   protected:
      virtual bool eventTest(QEvent* e);
      virtual void onTransition(QEvent*) {}

   public:
      CommandTransition(const QString& cmd, QState* target) : val(cmd) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   MagTransition
//---------------------------------------------------------

class MagTransition1 : public QEventTransition
      {
   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e)) {
                  printf("MagTransition1: wrong event\n");
                  return false;
                  }
            printf("MagTransition1: event %d\n", !(QApplication::keyboardModifiers() & Qt::ShiftModifier));
            return !(QApplication::keyboardModifiers() & Qt::ShiftModifier);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            bool b1 = me->button() == Qt::LeftButton;
            ScoreView* c = static_cast<ScoreView*>(eventSource());
            c->zoom(b1 ? 2 : -1, me->pos());
            }
   public:
      MagTransition1(QObject* obj, QState* target)
         : QEventTransition(obj, QEvent::MouseButtonPress) {
            setTargetState(target);
            }
      };

class MagTransition2 : public QEventTransition
      {
   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e)) {
                  printf("MagTransition2: wrong event\n");
                  return false;
                  }
            printf("MagTransition2: event %d\n", int (QApplication::keyboardModifiers() & Qt::ShiftModifier));
            return bool(QApplication::keyboardModifiers() & Qt::ShiftModifier);
            }
      virtual void onTransition(QEvent* e) {
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            bool b1 = me->button() == Qt::LeftButton;
            ScoreView* c = static_cast<ScoreView*>(eventSource());
            c->zoom(b1 ? 2 : -1, me->pos());
            }
   public:
      MagTransition2(QObject* obj)
         : QEventTransition(obj, QEvent::MouseButtonPress) {}
      };

//---------------------------------------------------------
//   ContextTransition
//---------------------------------------------------------

class ContextTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            canvas->contextPopup(me);
            }
   public:
      ContextTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::RightButton), canvas(c) {}
      };

//---------------------------------------------------------
//   EditTransition
//---------------------------------------------------------

class EditTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            QPointF p = canvas->toLogical(me->pos());
            Element* e = canvas->elementNear(p);
            canvas->setOrigEditObject(e);
            return e && e->isEditable();
            }
   public:
      EditTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonDblClick, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditKeyTransition
//---------------------------------------------------------

class EditKeyTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QKeyEvent* ke = static_cast<QKeyEvent*>(we->event());
            canvas->editKey(ke);
            }
   public:
      EditKeyTransition(ScoreView* c)
         : QEventTransition(c, QEvent::KeyPress), canvas(c) {}
      };

//---------------------------------------------------------
//   ScoreViewDragTransition
//---------------------------------------------------------

class ScoreViewDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            if (me->modifiers() & Qt::ShiftModifier)
                  return false;
            canvas->mousePress(me);
            return canvas->getDragObject() == 0;
            }
   public:
      ScoreViewDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   ScoreViewLassoTransition
//---------------------------------------------------------

class ScoreViewLassoTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            if (!(me->modifiers() & Qt::ShiftModifier))
                  return false;
            canvas->mousePress(me);
            return canvas->getDragObject() == 0;
            }
   public:
      ScoreViewLassoTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   ElementDragTransition
//---------------------------------------------------------

class ElementDragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->mousePress(me);
            return canvas->testElementDragTransition(me);
            }
   public:
      ElementDragTransition(ScoreView* c, QState* target)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditElementDragTransition
//---------------------------------------------------------

class EditElementDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->editElementDragTransition(me);
            }
   public:
      EditElementDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditPasteTransition
//---------------------------------------------------------

class EditPasteTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->onEditPasteTransition(me);
            }
   public:
      EditPasteTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::MidButton), canvas(c) {
            }
      };

//---------------------------------------------------------
//   EditInputTransition
//---------------------------------------------------------

class EditInputTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            canvas->editInputTransition(static_cast<QInputMethodEvent*>(we->event()));
            return true;
            }
   public:
      EditInputTransition(ScoreView* c)
         : QEventTransition(c, QEvent::InputMethod), canvas(c) {}
      };

//---------------------------------------------------------
//   EditScoreViewDragTransition
//---------------------------------------------------------

class EditScoreViewDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            return canvas->editScoreViewDragTransition(me);
            }
   public:
      EditScoreViewDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   ScoreViewSelectTransition
//---------------------------------------------------------

class ScoreViewSelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->mousePress(me);
            return canvas->getDragObject() != 0;
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->select(me);
            }
   public:
      ScoreViewSelectTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {}
      };

//---------------------------------------------------------
//   DragTransition
//---------------------------------------------------------

class DragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->dragScoreView(me);
            }
   public:
      DragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   NoteEntryDragTransition
//---------------------------------------------------------

class NoteEntryDragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->dragNoteEntry(me);
            }
   public:
      NoteEntryDragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   NoteEntryButtonTransition
//---------------------------------------------------------

class NoteEntryButtonTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->noteEntryButton(me);
            }
   public:
      NoteEntryButtonTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseButtonPress), canvas(c) {}
      };

//---------------------------------------------------------
//   DragElementTransition
//---------------------------------------------------------

class DragElementTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragElement(me);
            }
   public:
      DragElementTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   DragEditTransition
//---------------------------------------------------------

class DragEditTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragEdit(me);
            }
   public:
      DragEditTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   DragLassoTransition
//---------------------------------------------------------

class DragLassoTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->doDragLasso(me);
            }
   public:
      DragLassoTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

//---------------------------------------------------------
//   eventTest
//---------------------------------------------------------

bool CommandTransition::eventTest(QEvent* e)
      {
      if (e->type() != QEvent::Type(QEvent::User+1))
            return false;
      CommandEvent* ce = static_cast<CommandEvent*>(e);
      return ce->value == val;
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QWidget* parent)
   : QWidget(parent)
      {
      _score     = 0;
      dropTarget = 0;
      _editText  = 0;
      _matrix    = QMatrix(PDPI/DPI, 0.0, 0.0, PDPI/DPI, 0.0, 0.0);
      imatrix    = _matrix.inverted();
      _magIdx    = MAG_100;

      //---setup state machine-------------------------------------------------
      sm          = new QStateMachine(this);
//      sm->setGlobalRestorePolicy(QStateMachine::RestoreProperties);

      QState* stateActive = new QState();
      for (int i = 0; i < STATES; ++i) {
            states[i] = new QState(stateActive);
            states[i]->setObjectName(stateNames[i]);
            connect(states[i], SIGNAL(entered()), SLOT(enterState()));
            connect(states[i], SIGNAL(exited()), SLOT(exitState()));
            }
      connect(stateActive, SIGNAL(entered()), SLOT(enterState()));
      connect(stateActive, SIGNAL(exited()), SLOT(exitState()));

      CommandTransition* ct;
      QState* s;

      // setup normal state
      s = states[NORMAL];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new ContextTransition(this));                          // context menu
      EditTransition* et = new EditTransition(this, states[EDIT]);            // ->edit
      connect(et, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(et);
      s->addTransition(new ScoreViewSelectTransition(this));                  // select
      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
      s->addTransition(new ScoreViewDragTransition(this, states[DRAG]));      // ->stateDrag
      s->addTransition(new ScoreViewLassoTransition(this, states[LASSO]));    // ->stateLasso
      s->addTransition(new ElementDragTransition(this, states[DRAG_OBJECT])); // ->stateDragObject
      ct = new CommandTransition("note-input", states[NOTE_ENTRY]);           // ->noteEntry
      s->addTransition(ct);
      ct = new CommandTransition("escape", s);                                // escape
      connect(ct, SIGNAL(triggered()), SLOT(deselectAll()));
      s->addTransition(ct);
      ct = new CommandTransition("edit", states[EDIT]);                       // ->edit harmony/slur/lyrics
      connect(ct, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(ct);
      s->addTransition(new CommandTransition("mag", states[MAG]));
      s->addTransition(new CommandTransition("play", states[PLAY]));
      s->addTransition(new CommandTransition("find", states[SEARCH]));

      // setup mag state
      s = states[MAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      s->addTransition(new MagTransition1(this, states[NORMAL]));
      s->addTransition(new MagTransition2(this));

      // setup drag element state
      s = states[DRAG_OBJECT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      QEventTransition* cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      s->addTransition(new DragElementTransition(this));
      connect(s, SIGNAL(entered()), SLOT(startDrag()));
      connect(s, SIGNAL(exited()), SLOT(endDrag()));

      //----- setup edit state
      s = states[EDIT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      connect(s, SIGNAL(entered()), mscore, SLOT(setEditState()));
      ct = new CommandTransition("escape", states[NORMAL]);                   // ->normal
      connect(ct, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(ct);
      s->addTransition(new EditKeyTransition(this));                          // key events
      et = new EditTransition(this, s);                                       // ->edit
      connect(et, SIGNAL(triggered()), SLOT(endStartEdit()));
      s->addTransition(et);
      s->addTransition(new EditElementDragTransition(this, states[DRAG_EDIT]));  // ->editDrag
      EditScoreViewDragTransition* ent = new EditScoreViewDragTransition(this, states[DRAG]); // ->drag
      connect(ent, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(ent);
      s->addTransition(new EditInputTransition(this));                        // compose text
      s->addTransition(new EditPasteTransition(this));                        // paste text
      ct = new CommandTransition("copy", 0);                   // copy
      connect(ct, SIGNAL(triggered()), SLOT(editCopy()));
      s->addTransition(ct);
      ct = new CommandTransition("paste", 0);                   // paste
      connect(ct, SIGNAL(triggered()), SLOT(editPaste()));
      s->addTransition(ct);

      // setup drag edit state
      s = states[DRAG_EDIT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[EDIT]);
      s->addTransition(cl);
      s->addTransition(new DragEditTransition(this));
      connect(s, SIGNAL(exited()), SLOT(endDragEdit()));

      // setup lasso state
      s = states[LASSO];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);            // ->normal
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      s->addTransition(new class DragLassoTransition(this));                  // drag
      connect(s, SIGNAL(exited()), SLOT(endLasso()));

      // setup note entry state
      s = states[NOTE_ENTRY];
      s->assignProperty(this, "cursor", QCursor(Qt::UpArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));      // ->normal
      s->addTransition(new CommandTransition("note-input", states[NORMAL]));  // ->normal
      connect(s, SIGNAL(entered()), mscore, SLOT(setNoteEntryState()));
      connect(s, SIGNAL(entered()), SLOT(startNoteEntry()));
      connect(s, SIGNAL(exited()), SLOT(endNoteEntry()));
      s->addTransition(new NoteEntryDragTransition(this));                          // mouse drag
      s->addTransition(new NoteEntryButtonTransition(this));                        // mouse button

      // setup normal drag canvas state
      s = states[DRAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      connect(s, SIGNAL(entered()), SLOT(deselectAll()));
      s->addTransition(new DragTransition(this));

      // setup play state
      s = states[PLAY];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("play", states[NORMAL]));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));
      QSignalTransition* st = new QSignalTransition(seq, SIGNAL(stopped()));
      st->setTargetState(states[NORMAL]);
      s->addTransition(st);
      connect(s, SIGNAL(entered()), mscore, SLOT(setPlayState()));
      connect(s, SIGNAL(entered()), seq, SLOT(start()));
      connect(s, SIGNAL(exited()), seq, SLOT(stop()));

      // setup search state
      s = states[SEARCH];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));
      s->addTransition(new CommandTransition("find", states[NORMAL]));
      connect(s, SIGNAL(entered()), mscore, SLOT(setSearchState()));


      sm->addState(stateActive);
      stateActive->setInitialState(states[NORMAL]);
      sm->setInitialState(stateActive);


      sm->start();
      //-----------------------------------------------------------------------

      setAcceptDrops(true);
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setAutoFillBackground(true);

      focusFrame = 0;

      level            = 0;
      dragElement      = 0;
      dragObject       = 0;
      navigator        = 0;
      _score           = 0;
      _bgColor         = Qt::darkBlue;
      _fgColor         = Qt::white;
      fgPixmap         = 0;
      bgPixmap         = 0;
      lasso            = new Lasso(_score);

      cursor           = 0;
      shadowNote       = 0;
      mousePressed     = false;
      grips            = 0;
      origEditObject   = 0;
      editObject       = 0;

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

bool ScoreView::navigatorVisible() const
      {
      return navigator && navigator->isVisible();
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(Score* s)
      {
// printf("ScoreView(%p)::setScore: %p -> %p\n", this, _score, s);
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
      connect(s, SIGNAL(updateAll()),     SLOT(update()));
      connect(s, SIGNAL(dataChanged(const QRectF&)), SLOT(dataChanged(const QRectF&)));
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::~ScoreView()
      {
      delete lasso;
      delete cursor;
      delete shadowNote;
      }

//---------------------------------------------------------
//   objectPopup
//    the menu can be extended by Elements with
//      genPropertyMenu()/propertyAction() methods
//---------------------------------------------------------

void ScoreView::objectPopup(const QPoint& pos, Element* obj)
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
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "edit") {
            if (obj->isEditable()) {
                  startEdit(obj);
                  return;
                  }
            }
      else if (cmd == "select-similar")
            score()->selectSimilar(obj, false);
      else if (cmd == "select-similar-staff")
            score()->selectSimilar(obj, true);
      else if (cmd == "select-dialog")
            score()->selectElementDialog(obj);
      else {
            _score->startCmd();
            obj->propertyAction(this, cmd);
            _score->endCmd();
            }
      }

//---------------------------------------------------------
//   measurePopup
//---------------------------------------------------------

void ScoreView::measurePopup(const QPoint& gpos, Measure* obj)
      {
      int staffIdx = -1;
      int pitch;
      Segment* seg;
      QPointF offset;
      int tick = 0;

      _score->pos2measure(startMove, &tick, &staffIdx, &pitch, &seg, &offset);
      if (staffIdx == -1) {
            printf("ScoreView::measurePopup: staffIdx == -1!\n");
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

      if (staff->part()->drumset()) {
            if (enableExperimental) {
                  a = popup->addAction(tr("Drumroll Editor..."));
                  a->setData("drumroll");
                  }
            }
      else {
            a = popup->addAction(tr("Pianoroll Editor..."));
            a->setData("pianoroll");
            }

      a = popup->addAction(tr("Staff Properties..."));
      a->setData("staff-properties");
      a = popup->addAction(tr("Split Staff..."));
      a->setData("staff-split");

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
            if (obj->isEditable()) {
                  startEdit(obj);
                  return;
                  }
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
            EditStaff editStaff(staff, this);
            editStaff.exec();
            }
      else if (cmd == "staff-split") {
            SplitStaff splitStaff(this);
            if (splitStaff.exec())
                  _score->splitStaff(staffIdx, splitStaff.getSplitPoint());
            }
      else
            obj->propertyAction(this, cmd);
      _score->setLayoutAll(true);
      _score->endCmd();
      }

//---------------------------------------------------------
//   resizeEvent
//---------------------------------------------------------

void ScoreView::resizeEvent(QResizeEvent* /*ev*/)
      {
      if (_magIdx == MAG_PAGE_WIDTH || _magIdx == MAG_PAGE || _magIdx == MAG_DBL_PAGE) {
            double m = mscore->getMag(this);
            setMag(m);
            }

      update();
      if (navigator) {
            navigator->move(0, height() - navigator->height());
            updateNavigator(false);
            }
      }

//---------------------------------------------------------
//   updateNavigator
//---------------------------------------------------------

void ScoreView::updateNavigator(bool layoutChanged) const
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
//   updateGrips
//---------------------------------------------------------

void ScoreView::updateGrips()
      {
      Element* e = editObject;
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
//   setBackground
//---------------------------------------------------------

void ScoreView::setBackground(QPixmap* pm)
      {
      delete bgPixmap;
      bgPixmap = pm;
      update();
      }

void ScoreView::setBackground(const QColor& color)
      {
      delete bgPixmap;
      bgPixmap = 0;
      _bgColor = color;
      update();
      }

//---------------------------------------------------------
//   setForeground
//---------------------------------------------------------

void ScoreView::setForeground(QPixmap* pm)
      {
      delete fgPixmap;
      fgPixmap = pm;
      update();
      }

void ScoreView::setForeground(const QColor& color)
      {
      delete fgPixmap;
      fgPixmap = 0;
      _fgColor = color;
      update();
      }

//---------------------------------------------------------
//   dataChanged
//---------------------------------------------------------

void ScoreView::dataChanged(const QRectF& r)
      {
      redraw(r);
      }

//---------------------------------------------------------
//   redraw
//---------------------------------------------------------

void ScoreView::redraw(const QRectF& fr)
      {
      update(_matrix.mapRect(fr).toRect());  // generate paint event
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* element, int startGrip)
      {
      origEditObject = element;
      startEdit();
      editObject->updateGrips(&grips, grip);
      if (startGrip == -1)
            curGrip = grips-1;
      else if (startGrip >= 0)
            curGrip = startGrip;
      // startGrip == -2  -> do not change curGrip
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit()
      {
printf("ScoreView::startEdit: %s\n", origEditObject->name());
      score()->startCmd();
      score()->setLayoutAll(false);
      dragObject = 0;
      origEditObject->startEdit(this, startMove);
      setFocus();
      if (origEditObject->isTextB()) {
            editObject = origEditObject;
            TextB* t = static_cast<TextB*>(editObject);
            _editText = t;
            mscore->textTools()->setText(t);
            mscore->textTools()->setCharFormat(t->getCursor()->charFormat());
            mscore->textTools()->setBlockFormat(t->getCursor()->blockFormat());
            textUndoLevel = 0;
            connect(t->doc(), SIGNAL(undoCommandAdded()), this, SLOT(textUndoLevelAdded()));
            }
      else {
            editObject = origEditObject->clone();
            editObject->setSelected(true);
            origEditObject->resetMode();
            _score->undoChangeElement(origEditObject, editObject);
            _score->select(editObject, SELECT_SINGLE, 0);
            _score->removeBsp(origEditObject);
            }
      _score->setLayoutAll(true);
      _score->end();

      qreal w = 8.0 / _matrix.m11();
      qreal h = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < 4; ++i)
            grip[i] = r;
      editObject->updateGrips(&grips, grip);
      curGrip = grips-1;
      score()->end();
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void ScoreView::moveCursor()
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

void ScoreView::moveCursor(Segment* segment, int staffIdx)
      {
      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            printf("zero SYSTEM\n");
            return;
            }
      cursor->setSegment(segment);
      int idx         = staffIdx == -1 ? 0 : staffIdx;
      double systemY  = system->canvasPos().y();
      double x        = segment->canvasPos().x();
      double y        = system->staffY(idx) + systemY;
      double _spatium = cursor->spatium();
      double d        = _spatium * .5;

      _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2*d, 2*d));
      cursor->setPos(x - _spatium, y - _spatium);
      double h = 6.0 * _spatium;
      if (staffIdx == -1) {
            //
            // set cursor height for whole system
            //
            double y2 = 0.0;
            for (int i = 0; i < _score->nstaves(); ++i) {
                  SysStaff* ss = system->staff(i);
                  if (!ss->show())
                        continue;
                  y2 = ss->y();
                  }
            h += y2;
            }
      cursor->setHeight(h);
      _score->addRefresh(cursor->abbox().adjusted(-d, -d, 2*d, 2*d));
      cursor->setTick(segment->tick());
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void ScoreView::setCursorOn(bool val)
      {
      if (cursor)
            cursor->setOn(val);
      }

//---------------------------------------------------------
//   setShadowNote
//---------------------------------------------------------

void ScoreView::setShadowNote(const QPointF& p)
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
      int noteheadGroup = 0;
      int line          = pos.line;
      int noteHead      = score()->inputState().duration.headType();


      if (instr->useDrumset) {
            Drumset* ds  = instr->drumset;
            int pitch    = score()->inputState().drumNote;
            if (pitch >= 0 && ds->isValid(pitch)) {
                  line     = ds->line(pitch);
                  noteheadGroup = ds->noteHead(pitch);
                  }
            }
      shadowNote->setLine(line);
      shadowNote->setHeadGroup(noteheadGroup);
      shadowNote->setHead(noteHead);
      shadowNote->setPos(pos.pos);
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

void ScoreView::paintEvent(QPaintEvent* ev)
      {
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      p.setRenderHint(QPainter::TextAntialiasing, true);

      QRegion region;
      if (_score->needLayout()) {
            _score->doLayout();
            if (navigator)
                  navigator->layoutChanged();
            if (grips)
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

void ScoreView::paint(const QRect& rr, QPainter& p)
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

      if (grips) {
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
			if(!ss || !es)
				return;
            p.setBrush(Qt::NoBrush);

            QPen pen(QColor(Qt::blue));
            pen.setWidthF(2.0 / p.matrix().m11());

            if (sel->state() == SEL_SYSTEM) {
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

void ScoreView::setViewRect(const QRectF& r)
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

bool ScoreView::dragTimeAnchorElement(const QPointF& pos)
      {
      int staffIdx = -1;
      Segment* seg;
      int tick;
      MeasureBase* mb = _score->pos2measure(pos, &tick, &staffIdx, 0, &seg, 0);
      if (mb && mb->type() == MEASURE) {
            Measure* m = static_cast<Measure*>(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QPointF anchor(seg->abbox().x(), y);
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

bool ScoreView::dragMeasureAnchorElement(const QPointF& pos)
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

bool ScoreView::dragAboveMeasure(const QPointF& pos)
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

bool ScoreView::dragAboveSystem(const QPointF& pos)
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

void ScoreView::dragEnterEvent(QDragEnterEvent* event)
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
                  printf("ScoreView::dragEnterEvent: <%s>\n", a.data());

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

void ScoreView::dragSymbol(const QPointF& pos)
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

void ScoreView::dragMoveEvent(QDragMoveEvent* event)
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

void ScoreView::dropEvent(QDropEvent* event)
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
                  return;
                  }
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
      }

//---------------------------------------------------------
//   dragLeaveEvent
//---------------------------------------------------------

void ScoreView::dragLeaveEvent(QDragLeaveEvent*)
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

void ScoreView::zoom(int step, const QPoint& pos)
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
      setMag(_mag);
      _magIdx = MAG_FREE;

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
            }
      update();
      updateNavigator(false);
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ScoreView::wheelEvent(QWheelEvent* event)
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

const QList<const Element*> ScoreView::elementsAt(const QPointF& p)
      {
      QList<const Element*> el = _score->items(p);
      qSort(el.begin(), el.end(), elementLower);
      return el;
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* ScoreView::elementAt(const QPointF& p)
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

Element* ScoreView::elementNear(const QPointF& p)
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

void ScoreView::drawElements(QPainter& p,const QList<const Element*>& el)
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
      if (dragObject) {
            p.save();
            p.translate(dragObject->canvasPos());
            p.setPen(QPen(dragObject->curColor()));
            dragObject->draw(p);
            p.restore();
            }
      if (editObject) {
            p.save();
            p.translate(editObject->canvasPos());
            p.setPen(QPen(editObject->curColor()));
            editObject->draw(p);
            p.restore();
            }
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void ScoreView::setMag(double nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;
      nmag *= (PDPI/DPI);

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m21(), nmag,
         _matrix.dx() * deltamag, _matrix.dy() * deltamag);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   focusInEvent
//---------------------------------------------------------

void ScoreView::focusInEvent(QFocusEvent* event)
      {
      mscore->setCurrentScoreView(static_cast<ScoreView*>(this));

      if (mscore->splitScreen()) {
            if (!focusFrame) {
                  focusFrame = new QFocusFrame;
                  QPalette p(focusFrame->palette());
                  p.setColor(QPalette::WindowText, Qt::blue);
                  focusFrame->setPalette(p);
                  }
            focusFrame->setWidget(static_cast<QWidget*>(this));
            }
      QWidget::focusInEvent(event);
      }

//---------------------------------------------------------
//   focusOutEvent
//---------------------------------------------------------

void ScoreView::focusOutEvent(QFocusEvent* event)
      {
      if (focusFrame)
            focusFrame->setWidget(0);
      QWidget::focusOutEvent(event);
      }

//---------------------------------------------------------
//   setFocusRect
//---------------------------------------------------------

void ScoreView::setFocusRect()
      {
      if (mscore->splitScreen()) {
            if (!focusFrame) {
                  focusFrame = new QFocusFrame;
                  QPalette p(focusFrame->palette());
                  p.setColor(QPalette::WindowText, Qt::blue);
                  focusFrame->setPalette(p);
                  }
            focusFrame->setWidget(static_cast<QWidget*>(this));
            focusFrame->show();
            }
      else {
            if (focusFrame)
                  focusFrame->setWidget(0);
            }
      }

//---------------------------------------------------------
//   editCopy
//---------------------------------------------------------

void ScoreView::editCopy()
      {
      _score->cmdCopy();
      }

//---------------------------------------------------------
//   editPaste
//---------------------------------------------------------

void ScoreView::editPaste()
      {
      if (editObject->isTextB())
            static_cast<TextB*>(editObject)->paste();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void ScoreView::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
//      if (debugMode)
            printf("ScoreView::cmd <%s>\n", qPrintable(cmd));

      if (cmd == "escape")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "note-input" || cmd == "copy" || cmd == "paste")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "lyrics") {
            _score->startCmd();
            Lyrics* lyrics = _score->addLyrics();
            if (lyrics) {
                  _score->setLayoutAll(true);
                  startEdit(lyrics);
                  return;     // no endCmd()
                  }
            _score->endCmd();
            }
      else if (cmd == "mag")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "add-slur")
            cmdAddSlur();
      else if (cmd == "note-c")
            cmdAddPitch(0, false);
      else if (cmd == "note-d")
            cmdAddPitch(1, false);
      else if (cmd == "note-e")
            cmdAddPitch(2, false);
      else if (cmd == "note-f")
            cmdAddPitch(3, false);
      else if (cmd == "note-g")
            cmdAddPitch(4, false);
      else if (cmd == "note-a")
            cmdAddPitch(5, false);
      else if (cmd == "note-b")
            cmdAddPitch(6, false);
      else if (cmd == "chord-c")
            cmdAddPitch(0, true);
      else if (cmd == "chord-d")
            cmdAddPitch(1, true);
      else if (cmd == "chord-e")
            cmdAddPitch(2, true);
      else if (cmd == "chord-f")
            cmdAddPitch(3, true);
      else if (cmd == "chord-g")
            cmdAddPitch(4, true);
      else if (cmd == "chord-a")
            cmdAddPitch(5, true);
      else if (cmd == "chord-b")
            cmdAddPitch(6, true);
      else if (cmd == "chord-text")
            cmdAddChordName();
      else if (cmd == "title-text")
            cmdAddText(TEXT_TITLE);
      else if (cmd == "subtitle-text")
            cmdAddText(TEXT_SUBTITLE);
      else if (cmd == "composer-text")
            cmdAddText(TEXT_COMPOSER);
      else if (cmd == "poet-text")
            cmdAddText(TEXT_POET);
      else if (cmd == "copyright-text")
            cmdAddText(TEXT_COPYRIGHT);
      else if (cmd == "system-text")
            cmdAddText(TEXT_SYSTEM);
      else if (cmd == "staff-text")
            cmdAddText(TEXT_STAFF);
      else if (cmd == "rehearsalmark-text")
            cmdAddText(TEXT_REHEARSAL_MARK);
      else if (cmd == "edit-element") {
            Element* e = _score->selection()->element();
            if (e) {
                  _score->setLayoutAll(false);
                  startEdit(e);
                  }
            }
      else if (cmd == "play") {
            if (seq->canStart())
                  sm->postEvent(new CommandEvent(cmd));
            }
      else if (cmd == "find")
            sm->postEvent(new CommandEvent(cmd));
      else
            _score->cmd(a);
      _score->processMidiInput();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* e)
      {
      origEditObject = e;
      sm->postEvent(new CommandEvent("edit"));
      _score->end();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
printf("ScoreView::endEdit\n");

      setDropTarget(0);
      setEditText(0);
      _score->addRefresh(editObject->bbox());
      editObject->endEdit();
      _score->addRefresh(editObject->bbox());

      if (editObject->isTextB()) {
            TextB* t = static_cast<TextB*>(editObject);
            // if (t->doc()->isUndoAvailable()) {
            if (textUndoLevel)
                  _score->undo()->push(new EditText(t, textUndoLevel));
            disconnect(t->doc(), SIGNAL(undoCommandAdded()), this, SLOT(textUndoLevelAdded()));
            }

      int tp = editObject->type();

      if (tp == LYRICS)
            lyricsEndEdit();
      else if (tp == HARMONY)
            harmonyEndEdit();
      _score->setLayoutAll(true);
      _score->endCmd();
      editObject = 0;
      grips = 0;
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
      {
printf("startDrag\n");
      startMove -= dragObject->userOff();
      _score->startCmd();
      _startDragPosition = dragObject->userOff();
      QList<Element*> el;
      dragObject->scanElements(&el, collectElements);
      foreach(Element* e, el)
            _score->removeBsp(e);
      _score->end();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

void ScoreView::drag(const QPointF& delta)
      {
      foreach(Element* e, *_score->selection()->elements())
            _score->addRefresh(e->drag(delta));
      _score->end();
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void ScoreView::endDrag()
      {
      dragObject->endDrag();
      QPointF npos = dragObject->userOff();
      dragObject->setUserOff(_startDragPosition);
      _score->undoMove(dragObject, npos);
      _score->setLayoutAll(true);
      dragObject = 0;
      setDropTarget(0); // this also resets dropAnchor
      _score->endCmd();
      }

//---------------------------------------------------------
//   textUndoLevelAdded
//---------------------------------------------------------

void ScoreView::textUndoLevelAdded()
      {
      ++textUndoLevel;
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void ScoreView::startNoteEntry()
      {
      _score->inputState()._segment = 0;
      Note* note  = 0;
      Element* el = _score->selection()->activeCR() ? _score->selection()->activeCR() : _score->selection()->element();
      if (el == 0 || (el->type() != CHORD && el->type() != REST && el->type() != NOTE)) {
            int track = _score->inputState().track == -1 ? 0 : _score->inputState().track;
            el = static_cast<ChordRest*>(_score->searchNote(0, track));
            if (el == 0) {
printf("no note or rest selected 1\n");
                  return;
                  }
            }
      if (el->type() == CHORD) {
            Chord* c = static_cast<Chord*>(el);
            note = c->selectedNote();
            if (note == 0)
                  note = c->upNote();
            el = note;
            }

      _score->select(el, SELECT_SINGLE, 0);
      _score->inputState().noteEntryMode = true;
      moveCursor();
      _score->inputState().rest = false;
      getAction("pad-rest")->setChecked(false);
      setMouseTracking(true);
      shadowNote->setVisible(true);
      setCursorOn(true);
      dragObject = 0;
      //
      // TODO: check for valid duration
      //
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   endNoteEntry
//---------------------------------------------------------

void ScoreView::endNoteEntry()
      {
      _score->inputState()._segment = 0;
      _score->inputState().noteEntryMode = false;
      if (_score->inputState().slur) {
            QList<SlurSegment*>* el = _score->inputState().slur->slurSegments();
            if (!el->isEmpty())
                  el->front()->setSelected(false);
            static_cast<ChordRest*>(_score->inputState().slur->endElement())->addSlurBack(_score->inputState().slur);
            _score->inputState().slur = 0;
            }
      moveCursor();
      setMouseTracking(false);
      shadowNote->setVisible(false);
      setCursorOn(false);
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   enterState
//    for debugging
//---------------------------------------------------------

void ScoreView::enterState()
      {
      printf("enterState <%s>\n", qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   exitState
//    for debugging
//---------------------------------------------------------

void ScoreView::exitState()
      {
      printf("exitState <%s>\n", qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   contextPopup
//---------------------------------------------------------

void ScoreView::contextPopup(QMouseEvent* ev)
      {
      QPoint gp = ev->globalPos();
      startMove = toLogical(ev->pos());
      Element* dragObject = elementNear(startMove);
      if (dragObject) {
            ElementType type = dragObject->type();
            dragStaff = 0;
            if (type == MEASURE) {
                  dragSystem = (System*)(dragObject->parent());
                  dragStaff  = getStaff(dragSystem, startMove);
                  }
            seq->stopNotes();       // stop now because we dont get a mouseRelease event
            if (type == MEASURE)
                  measurePopup(gp, static_cast<Measure*>(dragObject));
            else
                  objectPopup(gp, dragObject);
            dragObject = 0;
            }
      else {
            QMenu* popup = mscore->genCreateMenu();
            _score->setLayoutAll(true);
            _score->end();
            popup->popup(gp);
            }
      }

//---------------------------------------------------------
//   dragScoreView
//---------------------------------------------------------

void ScoreView::dragScoreView(QMouseEvent* ev)
      {
      QPoint d = ev->pos() - _matrix.map(startMove).toPoint();
      int dx   = d.x();
      int dy   = d.y();

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
      if (!draggedScoreView)
            draggedScoreView = true;
      }

//---------------------------------------------------------
//   dragNoteEntry
//    mouse move event in note entry mode
//---------------------------------------------------------

void ScoreView::dragNoteEntry(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->addRefresh(shadowNote->abbox());
      setShadowNote(p);
      _score->addRefresh(shadowNote->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   noteEntryButton
//    mouse button press in note entry mode
//---------------------------------------------------------

void ScoreView::noteEntryButton(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->startCmd();
      _score->putNote(p, ev->modifiers() & Qt::ShiftModifier);
      _score->endCmd();
      }

//---------------------------------------------------------
//   doDragElement
//---------------------------------------------------------

void ScoreView::doDragElement(QMouseEvent* ev)
      {
      QPointF delta = toLogical(ev->pos()) - startMove;
      drag(delta);
      Element* e = _score->getSelectedElement();
      if (e) {
            QLineF anchor = e->dragAnchor();
            if (!anchor.isNull())
                  setDropAnchor(anchor);
            else
                  setDropTarget(0); // this also resets dropAnchor
            }
      }

//---------------------------------------------------------
//   select
//---------------------------------------------------------

void ScoreView::select(QMouseEvent* ev)
      {
printf("select\n");
      Qt::KeyboardModifiers keyState = ev->modifiers();
      ElementType type = dragObject->type();
      dragStaff = 0;
      if (type == MEASURE) {
            dragSystem = (System*)(dragObject->parent());
            dragStaff  = getStaff(dragSystem, startMove);
            }
      // As findSelectableElement may return a measure
      // when clicked "a little bit" above or below it, getStaff
      // may not find the staff and return -1, which would cause
      // select() to crash
      if (dragStaff >= 0) {
            SelectType st = SELECT_SINGLE;
            if (keyState == Qt::NoModifier)
                  st = SELECT_SINGLE;
            else if (keyState & Qt::ShiftModifier)
                  st = SELECT_RANGE;
            else if (keyState & Qt::ControlModifier)
                  st = SELECT_ADD;
            _score->select(dragObject, st, dragStaff);
            }
      else
            dragObject = 0;
      _score->setLayoutAll(false);
      _score->end();    // update
      }

//---------------------------------------------------------
//   mousePress
//---------------------------------------------------------

void ScoreView::mousePress(QMouseEvent* ev)
      {
      startMove   = imatrix.map(QPointF(ev->pos()));
      dragObject  = elementNear(startMove);
      }

//---------------------------------------------------------
//   testElementDragTransition
//---------------------------------------------------------

bool ScoreView::testElementDragTransition(QMouseEvent* ev) const
      {
      if (dragObject == 0 || !dragObject->isMovable())
            return false;
      if (!(QApplication::mouseButtons() == Qt::LeftButton))
            return false;
      QPointF delta = toLogical(ev->pos()) - startMove;
      return sqrt(pow(delta.x(),2)+pow(delta.y(),2)) * _matrix.m11() <= 2.0;
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editObject->abbox());
      editObject->endEditDrag();
      updateGrips();
      setDropTarget(0); // this also resets dropRectangle and dropAnchor
      _score->addRefresh(editObject->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   doDragEdit
//---------------------------------------------------------

void ScoreView::doDragEdit(QMouseEvent* ev)
      {
      QPointF p     = toLogical(ev->pos());
      QPointF delta = p - startMove;
      _score->setLayoutAll(false);
      score()->addRefresh(editObject->abbox());
      if (editObject->isTextB()) {
            TextB* text = static_cast<TextB*>(editObject);
            text->dragTo(p);
            }
      else {
            editObject->editDrag(curGrip, delta);
            updateGrips();
            startMove = p;
            }
      _score->end();
      }

//---------------------------------------------------------
//   editElementDragTransition
//---------------------------------------------------------

bool ScoreView::editElementDragTransition(QMouseEvent* ev)
      {
      startMove = imatrix.map(QPointF(ev->pos()));
      Element* e = elementNear(startMove);
      if ((e == editObject) && (editObject->isTextB())) {
            if (editObject->mousePress(startMove, ev)) {
                  _score->addRefresh(editObject->abbox());
                  _score->end();
                  }
            return true;
            }
      int i;
      for (i = 0; i < grips; ++i) {
            if (grip[i].contains(startMove)) {
                  curGrip = i;
                  updateGrips();
                  score()->end();
                  break;
                  }
            }
      QPointF delta = toLogical(ev->pos()) - startMove;
      return (i != grips) && (sqrt(pow(delta.x(),2)+pow(delta.y(),2)) * _matrix.m11() <= 2.0);
      }

//---------------------------------------------------------
//   onEditPasteTransition
//---------------------------------------------------------

void ScoreView::onEditPasteTransition(QMouseEvent* ev)
      {
      startMove = imatrix.map(QPointF(ev->pos()));
      Element* e = elementNear(startMove);
      if ((e == editObject)) {
            if (editObject->mousePress(startMove, ev)) {
                  _score->addRefresh(editObject->abbox());
                  _score->end();
                  }
            }
      }

//---------------------------------------------------------
//   editScoreViewDragTransition
//    Check for mouse click outside of editObject.
//---------------------------------------------------------

bool ScoreView::editScoreViewDragTransition(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      Element* e = elementNear(p);
      if (e != editObject) {
            startMove  = p;
            dragObject = e;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   doDragLasso
//---------------------------------------------------------

void ScoreView::doDragLasso(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->addRefresh(lasso->abbox());
      QRectF r;
      r.setCoords(startMove.x(), startMove.y(), p.x(), p.y());
      lasso->setbbox(r);
      _lassoRect = lasso->abbox().normalized();
      r = _matrix.mapRect(_lassoRect);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);
      _score->addRefresh(lasso->abbox());
      _score->lassoSelect(lasso->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   endLasso
//---------------------------------------------------------

void ScoreView::endLasso()
      {
      _score->addRefresh(lasso->abbox().adjusted(-2, -2, 2, 2));
      lasso->setbbox(QRectF());
      _score->lassoSelectEnd(lasso->abbox());
      _score->end();
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void ScoreView::deselectAll()
      {
      _score->select(0, SELECT_SINGLE, 0);
      _score->end();
      }

//---------------------------------------------------------
//   noteEntryMode
//---------------------------------------------------------

bool ScoreView::noteEntryMode() const
      {
      return sm->configuration().contains(states[NOTE_ENTRY]);
      }

//---------------------------------------------------------
//   editInputTransition
//---------------------------------------------------------

void ScoreView::editInputTransition(QInputMethodEvent* ie)
      {
      if (editObject->edit(this, curGrip, 0, 0, ie->commitString())) {
            updateGrips();
            _score->end();
            }
      }

//---------------------------------------------------------
//   setDropTarget
//---------------------------------------------------------

void ScoreView::setDropTarget(const Element* el)
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

void ScoreView::setDropRectangle(const QRectF& r)
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

void ScoreView::setDropAnchor(const QLineF& l)
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

qreal ScoreView::mag() const
      {
      return _matrix.m11() *  DPI/PDPI;
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void ScoreView::setOffset(qreal x, qreal y)
      {
      double m = PDPI / DPI;
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
         _matrix.m22(), x * m, y * m);
      imatrix = _matrix.inverted();
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal ScoreView::xoffset() const
      {
      return _matrix.dx() * DPI / PDPI;
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal ScoreView::yoffset() const
      {
      return _matrix.dy() * DPI / PDPI;
      }

//---------------------------------------------------------
//   setMagIdx
//---------------------------------------------------------

void ScoreView::setMag(int idx, double mag)
      {
      _magIdx = idx;
      setMag(mag);
      update();
      // TODO? updateNavigator(false);
      }

//---------------------------------------------------------
//   fsize
//---------------------------------------------------------

QSizeF ScoreView::fsize() const
      {
      QSize s = size();
      return QSizeF(s.width() * imatrix.m11(), s.height() * imatrix.m22());
      }

//---------------------------------------------------------
//   pageNext
//---------------------------------------------------------

void ScoreView::pageNext()
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

void ScoreView::pagePrev()
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

void ScoreView::pageTop()
      {
      setOffset(10.0, 10.0);
//      updateNavigator(false);
      update();
      }

//---------------------------------------------------------
//   pageEnd
//---------------------------------------------------------

void ScoreView::pageEnd()
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

void ScoreView::adjustCanvasPosition(Element* el, bool playBack)
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

