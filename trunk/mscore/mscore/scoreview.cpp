//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2011 Werner Schweer and others
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
#include "libmscore/score.h"
#include "preferences.h"
#include "libmscore/utils.h"
#include "libmscore/segment.h"
#include "musescore.h"
#include "seq.h"
#include "libmscore/staff.h"
#include "libmscore/chord.h"
#include "libmscore/rest.h"
#include "libmscore/page.h"
#include "libmscore/xml.h"
#include "libmscore/text.h"
#include "libmscore/note.h"
#include "libmscore/dynamic.h"
#include "libmscore/pedal.h"
#include "libmscore/volta.h"
#include "libmscore/ottava.h"
#include "libmscore/textline.h"
#include "libmscore/trill.h"
#include "libmscore/hairpin.h"
#include "libmscore/image.h"
#include "libmscore/part.h"
#include "editdrumset.h"
#include "editstaff.h"
#include "splitstaff.h"
#include "libmscore/barline.h"
#include "libmscore/system.h"
#include "magbox.h"
#include "libmscore/measure.h"
#include "drumroll.h"
#include "libmscore/lyrics.h"
#include "textpalette.h"
#include "libmscore/undo.h"
#include "libmscore/slur.h"
#include "libmscore/harmony.h"
#include "libmscore/navigate.h"
#include "libmscore/tablature.h"
#include "libmscore/shadownote.h"
#include "libmscore/sym.h"
#include "libmscore/lasso.h"
#include "libmscore/box.h"
#include "texttools.h"
#include "libmscore/clef.h"
#include "scoretab.h"
#include "painterqt.h"
#include "measureproperties.h"
#include "libmscore/pitchspelling.h"

#include "libmscore/articulation.h"
#include "libmscore/tuplet.h"
#include "libmscore/stafftext.h"
#include "libmscore/keysig.h"
#include "libmscore/timesig.h"
#include "libmscore/spanner.h"

#include "navigator.h"

static const QEvent::Type CloneDrag = QEvent::Type(QEvent::User + 1);
extern TextPalette* textPalette;

//---------------------------------------------------------
//   CloneEvent
//---------------------------------------------------------

class CloneEvent : public QEvent {
      Element* _element;

   public:
      CloneEvent(Element* e) : QEvent(CloneDrag) { _element = e->clone(); }
      ~CloneEvent() { delete _element; }
      Element* element() const { return _element; }
      };

//---------------------------------------------------------
//   stateNames
//---------------------------------------------------------

static const char* stateNames[] = {
      "Normal", "Drag", "DragObject", "Edit", "DragEdit",
      "Lasso",  "NoteEntry", "Mag", "Play", "Search", "EntryPlay",
      "Fotomode"
      };

//---------------------------------------------------------
//   MagTransition
//---------------------------------------------------------

class MagTransition1 : public QEventTransition
      {
   protected:
      virtual bool eventTest(QEvent* e) {
            if (!QEventTransition::eventTest(e))
                  return false;
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

class ContextTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e) {
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(e)->event());
            canvas->contextPopup(me);
            }
   public:
      ContextTransition(ScoreView* c)
         : QEventTransition(c, QEvent::ContextMenu), canvas(c) {}
      };

//---------------------------------------------------------
//   EditTransition
//---------------------------------------------------------

class EditTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event) || canvas->getOrigEditObject())
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            QPointF p = canvas->toLogical(me->pos());
            Element* e = canvas->elementNear(p);
            if (e)
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
//   SeekTransition
//---------------------------------------------------------

class SeekTransition : public QMouseEventTransition
      {
      ScoreView* canvas;
      ChordRest* cr;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            QPointF p = canvas->toLogical(me->pos());
            Element* e = canvas->elementNear(p);
            if (e && (e->type() == NOTE || e->type() == REST)) {
                  if (e->type() == NOTE)
                        e = e->parent();
                  cr = static_cast<ChordRest*>(e);
                  return true;
                  }
            return false;
            }
      virtual void onTransition(QEvent*) {
            seq->seek(cr->tick());
            canvas->setCursorVisible(true);
            }
   public:
      SeekTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
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
            return !canvas->mousePress(me);
            }
   public:
      ScoreViewLassoTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
// ElementDragTransition
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
            // canvas->mousePress(me);
            return canvas->testElementDragTransition(me);
            }
   public:
      ElementDragTransition(ScoreView* c, QState* target)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditDragEditTransition
//---------------------------------------------------------

class EditDragEditTransition : public QMouseEventTransition
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
      EditDragEditTransition(ScoreView* c, QState* target)
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
//   EditDragTransition
//---------------------------------------------------------

class EditDragTransition : public QMouseEventTransition
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
      EditDragTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   EditSelectTransition
//---------------------------------------------------------

class EditSelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QMouseEvent* me = static_cast<QMouseEvent*>(static_cast<QStateMachine::WrappedEvent*>(event)->event());
            return canvas->editSelectTransition(me);
            }
   public:
      EditSelectTransition(ScoreView* c, QState* target)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {
            setTargetState(target);
            }
      };

//---------------------------------------------------------
//   SelectTransition
//---------------------------------------------------------

class SelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->mousePress(me);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->select(me);
            }
   public:
      SelectTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c) {}
      };

//---------------------------------------------------------
//   DeSelectTransition
//---------------------------------------------------------

class DeSelectTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event) {
            if (!QMouseEventTransition::eventTest(event))
                  return false;
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            return canvas->mousePress(me);
            }
      virtual void onTransition(QEvent* e) {
            QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
            QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
            canvas->select(me);
            }
   public:
      DeSelectTransition(ScoreView* c)
         : QMouseEventTransition(c, QEvent::MouseButtonRelease, Qt::LeftButton), canvas(c) {}
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
//   ScoreViewDragTransition
//---------------------------------------------------------

bool ScoreViewDragTransition::eventTest(QEvent* event)
      {
      if (!QMouseEventTransition::eventTest(event))
            return false;
      QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(event);
      QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
      if (me->modifiers() & Qt::ShiftModifier)
            return false;
      return !canvas->mousePress(me);
      }

ScoreViewDragTransition::ScoreViewDragTransition(ScoreView* c, QState* target)
   : QMouseEventTransition(c, QEvent::MouseButtonPress, Qt::LeftButton), canvas(c)
      {
      setTargetState(target);
      }

//---------------------------------------------------------
//   onTransition
//---------------------------------------------------------

void DragTransition::onTransition(QEvent* e)
      {
      QStateMachine::WrappedEvent* we = static_cast<QStateMachine::WrappedEvent*>(e);
      QMouseEvent* me = static_cast<QMouseEvent*>(we->event());
      canvas->dragScoreView(me);
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::ScoreView(QWidget* parent)
   : QWidget(parent)
      {
      setAcceptDrops(true);
      setAttribute(Qt::WA_OpaquePaintEvent);
      setAttribute(Qt::WA_NoSystemBackground);
      setFocusPolicy(Qt::StrongFocus);
      setAttribute(Qt::WA_InputMethodEnabled);
      setAttribute(Qt::WA_KeyCompression);
      setAttribute(Qt::WA_StaticContents);
      setAutoFillBackground(false);

      _score      = 0;
      dropTarget  = 0;
      _editText   = 0;

      setContextMenuPolicy(Qt::DefaultContextMenu);

      double mag  = preferences.mag;
      _matrix     = QTransform(mag, 0.0, 0.0, mag, 0.0, 0.0);
      imatrix     = _matrix.inverted();
      _magIdx     = preferences.mag == 1.0 ? MAG_100 : MAG_FREE;
      focusFrame  = 0;
      dragElement = 0;
      curElement  = 0;
      _bgColor    = Qt::darkBlue;
      _fgColor    = Qt::white;
      fgPixmap    = 0;
      bgPixmap    = 0;
      lasso       = new Lasso(_score);
      _foto       = new Lasso(_score);

      _cursor     = new Cursor;
      shadowNote  = 0;
      grips       = 0;
      origEditObject   = 0;
      editObject  = 0;
      addSelect   = false;

      //---setup state machine-------------------------------------------------
      sm          = new QStateMachine(this);
      QState* stateActive = new QState();
      for (int i = 0; i < STATES; ++i) {
            states[i] = new QState(stateActive);
            states[i]->setObjectName(stateNames[i]);
            connect(states[i], SIGNAL(entered()), SLOT(enterState()));
            connect(states[i], SIGNAL(exited()), SLOT(exitState()));
            }

      if (converterMode)      // HACK
            return;

      connect(stateActive, SIGNAL(entered()), SLOT(enterState()));
      connect(stateActive, SIGNAL(exited()), SLOT(exitState()));

      CommandTransition* ct;
      QEventTransition* evt;
      QState* s;

      //----------------------------------
      //    setup normal state
      //----------------------------------

      s = states[NORMAL];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new ContextTransition(this));                          // context menu
      EditTransition* et = new EditTransition(this, states[EDIT]);            // ->edit
      connect(et, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(et);
      s->addTransition(new SelectTransition(this));                           // select
      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
//      s->addTransition(new DeSelectTransition(this));                         // deselect
//      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
      s->addTransition(new ScoreViewDragTransition(this, states[DRAG]));      // ->stateDrag
      s->addTransition(new ScoreViewLassoTransition(this, states[LASSO]));    // ->stateLasso
      s->addTransition(new ElementDragTransition(this, states[DRAG_OBJECT])); // ->stateDragObject
      s->addTransition(new CommandTransition("note-input", states[NOTE_ENTRY])); // ->noteEntry
      ct = new CommandTransition("escape", s);                                // escape
      connect(ct, SIGNAL(triggered()), SLOT(deselectAll()));
      s->addTransition(ct);
      ct = new CommandTransition("edit", states[EDIT]);                       // ->edit harmony/slur/lyrics
      connect(ct, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(ct);
      s->addTransition(new CommandTransition("mag", states[MAG]));            // ->mag
      s->addTransition(new CommandTransition("play", states[PLAY]));          // ->play
      s->addTransition(new CommandTransition("find", states[SEARCH]));        // ->search
      s->addTransition(new CommandTransition("fotomode", states[FOTOMODE]));  // ->fotomode
      ct = new CommandTransition("paste", 0);                                 // paste
      connect(ct, SIGNAL(triggered()), SLOT(normalPaste()));
      s->addTransition(ct);
      ct = new CommandTransition("copy", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(normalCopy()));
      s->addTransition(ct);
      ct = new CommandTransition("cut", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(normalCut()));
      s->addTransition(ct);

      //----------------------------------
      //    setup mag state
      //----------------------------------

      s = states[MAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      s->addTransition(new MagTransition1(this, states[NORMAL]));
      s->addTransition(new MagTransition2(this));

      //----------------------------------
      // setup drag element state
      //----------------------------------

      s = states[DRAG_OBJECT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      QEventTransition* cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      s->addTransition(new DragElementTransition(this));
      connect(s, SIGNAL(entered()), SLOT(startDrag()));
      connect(s, SIGNAL(exited()), SLOT(endDrag()));

      //----------------------------------
      //    setup edit state
      //----------------------------------

      s = states[EDIT];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));

      ct = new CommandTransition("escape", states[NORMAL]);                   // ->NORMAL
      connect(ct, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(ct);

      s->addTransition(new EditKeyTransition(this));                          // key events

      et = new EditTransition(this, s);                                       // ->EDIT
      connect(et, SIGNAL(triggered()), SLOT(endStartEdit()));
      s->addTransition(et);

      s->addTransition(new EditDragEditTransition(this, states[DRAG_EDIT]));  // ->DRAG_EDIT

      evt = new EditDragTransition(this, states[DRAG]);                       // ->DRAG
      connect(evt, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(evt);

      evt = new EditSelectTransition(this, states[NORMAL]);                   // ->NORMAL
      connect(evt, SIGNAL(triggered()), SLOT(endEdit()));
      s->addTransition(evt);

      s->addTransition(new EditInputTransition(this));                        // compose text

      s->addTransition(new EditPasteTransition(this));                        // paste text

      ct = new CommandTransition("copy", 0);                                  // copy
      connect(ct, SIGNAL(triggered()), SLOT(editCopy()));
      s->addTransition(ct);

      ct = new CommandTransition("paste", 0);                                 // paste
      connect(ct, SIGNAL(triggered()), SLOT(editPaste()));
      s->addTransition(ct);

      //----------------------------------
      //    setup drag edit state
      //----------------------------------

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

      //----------------------setup note entry state
      s = states[NOTE_ENTRY];
      s->assignProperty(this, "cursor", QCursor(Qt::UpArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));      // ->normal
      s->addTransition(new CommandTransition("note-input", states[NORMAL]));  // ->normal
      connect(s, SIGNAL(entered()), mscore, SLOT(setNoteEntryState()));
      connect(s, SIGNAL(entered()), SLOT(startNoteEntry()));
      connect(s, SIGNAL(exited()), SLOT(endNoteEntry()));
      s->addTransition(new NoteEntryDragTransition(this));                    // mouse drag
      s->addTransition(new NoteEntryButtonTransition(this));                  // mouse button
      s->addTransition(new CommandTransition("play", states[ENTRY_PLAY]));    // ->entryPlay

      // setup normal drag canvas state
      s = states[DRAG];
      s->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(states[NORMAL]);
      s->addTransition(cl);
      connect(s, SIGNAL(entered()), SLOT(deselectAll()));
      s->addTransition(new DragTransition(this));

      //----------------------setup play state
      s = states[PLAY];
      // s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("play", states[NORMAL]));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));
      s->addTransition(new SeekTransition(this, states[PLAY]));
      QSignalTransition* st = new QSignalTransition(seq, SIGNAL(stopped()));
      st->setTargetState(states[NORMAL]);
      s->addTransition(st);
      connect(s, SIGNAL(entered()), mscore, SLOT(setPlayState()));
      connect(s, SIGNAL(entered()), seq, SLOT(start()));
      connect(s, SIGNAL(exited()),  seq, SLOT(stop()));

      QState* s1 = new QState(s);
      s1->setObjectName("play-normal");
      connect(s1, SIGNAL(entered()), SLOT(enterState()));
      connect(s1, SIGNAL(exited()), SLOT(exitState()));
      QState* s2 = new QState(s);
      connect(s2, SIGNAL(entered()), SLOT(enterState()));
      connect(s2, SIGNAL(exited()), SLOT(exitState()));
      s2->setObjectName("play-drag");

      s1->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s1->addTransition(new ScoreViewDragTransition(this, s2));      // ->stateDrag

      // drag during play state
      s2->assignProperty(this, "cursor", QCursor(Qt::SizeAllCursor));
      cl = new QEventTransition(this, QEvent::MouseButtonRelease);
      cl->setTargetState(s1);
      s2->addTransition(cl);
      connect(s2, SIGNAL(entered()), SLOT(deselectAll()));
      s2->addTransition(new DragTransition(this));

      s->setInitialState(s1);
      s->addTransition(new ScoreViewDragTransition(this, s2));

      //----------------------setup search state
      s = states[SEARCH];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("escape", states[NORMAL]));
      s->addTransition(new CommandTransition("find", states[NORMAL]));
      connect(s, SIGNAL(entered()), mscore, SLOT(setSearchState()));

      // setup editPlay state
      s = states[ENTRY_PLAY];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new CommandTransition("play", states[NOTE_ENTRY]));
      s->addTransition(new CommandTransition("escape", states[NOTE_ENTRY]));
      st = new QSignalTransition(seq, SIGNAL(stopped()));
      st->setTargetState(states[NOTE_ENTRY]);
      s->addTransition(st);
      connect(s, SIGNAL(entered()), mscore, SLOT(setPlayState()));
      connect(s, SIGNAL(entered()), seq, SLOT(start()));
      connect(s, SIGNAL(exited()),  seq, SLOT(stop()));

      setupFotoMode();

      sm->addState(stateActive);
      stateActive->setInitialState(states[NORMAL]);
      sm->setInitialState(stateActive);

      sm->start();


      grabGesture(Qt::PinchGesture);

      //-----------------------------------------------------------------------

      if (debugMode)
            setMouseTracking(true);

      if (preferences.bgUseColor)
            setBackground(MScore::bgColor);
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
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(Score* s)
      {
      if (_score)
            _score->removeViewer(this);
      _score = s;
      _score->addViewer(this);

      if (shadowNote == 0) {
            shadowNote = new ShadowNote(_score);
            shadowNote->setVisible(false);
            }
      else
            shadowNote->setScore(_score);
      lasso->setScore(s);
      _foto->setScore(s);
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::~ScoreView()
      {
      if (_score)
            _score->removeViewer(this);
      delete lasso;
      delete _foto;
      delete _cursor;
      delete bgPixmap;
      delete fgPixmap;
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
      QAction* a = popup->addSeparator();
      a->setText(obj->userName());

      popup->addAction(getAction("cut"));
      popup->addAction(getAction("copy"));
      popup->addAction(getAction("paste"));

      QMenu* selMenu = popup->addMenu(tr("Select"));
      selMenu->addAction(getAction("select-similar"));
      selMenu->addAction(getAction("select-similar-staff"));
      a = selMenu->addAction(tr("More..."));
      a->setData("select-dialog");
      popup->addSeparator();
      a = getAction("edit-element");
      popup->addAction(a);
      a->setEnabled(obj->isEditable());

      createElementPropertyMenu(obj, popup);

      popup->addSeparator();
      a = popup->addAction(tr("Object Debugger"));
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
            mscore->selectSimilar(obj, false);
      else if (cmd == "select-similar-staff")
            mscore->selectSimilar(obj, true);
      else if (cmd == "select-dialog")
            mscore->selectElementDialog(obj);
      else {
            _score->startCmd();
            elementPropertyAction(cmd, obj);
            _score->endCmd();
            mscore->endCmd();
            }
      }

//---------------------------------------------------------
//   measurePopup
//---------------------------------------------------------

void ScoreView::measurePopup(const QPoint& gpos, Measure* obj)
      {
      int staffIdx;
      int pitch;
      Segment* seg;
      QPointF offset;

      if (!_score->pos2measure(startMove, &staffIdx, &pitch, &seg, &offset))
            return;
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
      a->setEnabled(staff->part()->instr()->drumset() != 0);

      if (staff->part()->instr()->drumset()) {
            a = popup->addAction(tr("Drumroll Editor..."));
            a->setData("drumroll");
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
      popup->addAction(getAction("delete"));
      popup->addAction(getAction("insert-measure"));
      popup->addSeparator();

      popup->addAction(tr("Measure Properties..."))->setData("props");
      popup->addSeparator();

      popup->addAction(tr("Object Debugger"))->setData("list");

      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste" || cmd == "insert-measure"
         || cmd == "select-similar"
         || cmd == "delete") {
            // these actions are already activated
            return;
            }
      _score->startCmd();
      if (cmd == "list")
            mscore->showElementContext(obj);
      else if (cmd == "color")
            _score->colorItem(obj);
      else if (cmd == "edit") {
            if (obj->isEditable()) {
                  startEdit(obj);
                  return;
                  }
            }
      else if (cmd == "edit-drumset") {
            EditDrumset drumsetEdit(staff->part()->instr()->drumset(), this);
            drumsetEdit.exec();
            }
      else if (cmd == "drumroll") {
            _score->endCmd();
            mscore->editInDrumroll(staff);
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
      else if (cmd == "props") {
            MeasureProperties im(obj);
            im.exec();
            }
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
      }

//---------------------------------------------------------
//   updateGrips
//    if (curGrip == -1) then initialize to grips-1
//---------------------------------------------------------

void ScoreView::updateGrips()
      {
      if (editObject == 0)
            return;

      double dx = 1.5 / _matrix.m11();
      double dy = 1.5 / _matrix.m22();

      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));

      qreal w   = 8.0 / _matrix.m11();
      qreal h   = 8.0 / _matrix.m22();
      QRectF r(-w*.5, -h*.5, w, h);
      for (int i = 0; i < MAX_GRIPS; ++i)
            grip[i] = r;

      editObject->updateGrips(&grips, grip);

      // updateGrips returns grips in page coordinates,
      // transform to view coordinates:

      Element* page = editObject;
      while (page->parent())
            page = page->parent();
      QPointF pageOffset(page->pos());
      for (int i = 0; i < grips; ++i) {
            grip[i].translate(pageOffset);
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));
            }

      if (curGrip == -1)
            curGrip = grips-1;

#if 0
      double x, y;
      if (grips) {
            x = grip[curGrip].center().x() - editObject->gripAnchor(curGrip).x();
            y = grip[curGrip].center().y() - editObject->gripAnchor(curGrip).y();
            }
      else {
            x = editObject->userOff().x();
            y = editObject->userOff().y();
            }
      double _spatium = score()->spatium();
      mscore->setEditX(x / _spatium);
      mscore->setEditY(y / _spatium);
#endif

      QPointF anchor = editObject->gripAnchor(curGrip);
      if (!anchor.isNull())
            setDropAnchor(QLineF(anchor + pageOffset, grip[curGrip].center()));
      else
            setDropTarget(0); // this also resets dropAnchor
      score()->addRefresh(editObject->canvasBoundingRect());
      }

//---------------------------------------------------------
//   setEditPos
//---------------------------------------------------------

void ScoreView::setEditPos(const QPointF& pt)
      {
      editObject->setGrip(curGrip, pt);
      updateGrips();
      _score->end();
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
      update(_matrix.mapRect(r).toRect());  // generate paint event
      }

//---------------------------------------------------------
//   updateAll
//---------------------------------------------------------

void ScoreView::updateAll()
      {
      update();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* e)
      {
      if (e->type() == TBOX)
            e = static_cast<TBox*>(e)->getText();
      origEditObject = e;
      sm->postEvent(new CommandEvent("edit"));
      _score->end();
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit(Element* element, int startGrip)
      {
      origEditObject = element;
      startEdit();
      if (startGrip == -1)
            curGrip = grips-1;
      else if (startGrip >= 0)
            curGrip = startGrip;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void ScoreView::startEdit()
      {
      mscore->setEditState();
      score()->setLayoutAll(false);
      curElement  = 0;
      setFocus();
      if (!score()->undo()->active())
            score()->startCmd();
      if (origEditObject->isText()) {
            editObject = origEditObject;
            editObject->startEdit(this, startMove);
            Text* t = static_cast<Text*>(editObject);
            _editText = t;
            mscore->textTools()->setText(t);
            mscore->textTools()->updateTools();
            mscore->textTools()->show();
            textUndoLevel = 0;
            connect(t->doc(), SIGNAL(undoCommandAdded()), SLOT(textUndoLevelAdded()));
            }
      else if (origEditObject->isSegment()) {
            SpannerSegment* ss = (SpannerSegment*)origEditObject;
            Spanner* spanner   = ss->spanner();
            Spanner* clone     = static_cast<Spanner*>(spanner->clone());
            clone->setLinks(spanner->links());
            int idx            = spanner->spannerSegments().indexOf(ss);
            editObject         = clone->spannerSegments()[idx];
            editObject->startEdit(this, startMove);
            _score->undoChangeElement(spanner, clone);
            }
      else {
            foreach(Element* e, origEditObject->linkList()) {
                  Element* ce = e->clone();
                  if (e == origEditObject) {
                        editObject = ce;
                        editObject->setSelected(true);
                        }
                  _score->undoChangeElement(e, ce);
                  }
            editObject->startEdit(this, startMove);
            }
      curGrip = -1;
      updateGrips();
      score()->end();
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
      setDropTarget(0);
      setEditText(0);
      if (!editObject) {
            origEditObject = 0;
	      return;
            }

      _score->addRefresh(editObject->canvasBoundingRect());
      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i]);

      editObject->endEdit();
      if (editObject->isText()) {
            if (textPalette) {
                  textPalette->hide();
                  mscore->textTools()->kbAction()->setChecked(false);
                  }
            mscore->textTools()->hide();

            Text* t = static_cast<Text*>(editObject);
            if (textUndoLevel)
                  _score->undo()->push(new EditText(t, textUndoLevel));
            disconnect(t->doc(), SIGNAL(undoCommandAdded()), this, SLOT(textUndoLevelAdded()));
            }
      else if (editObject->isSegment()) {
            Spanner* spanner  = static_cast<SpannerSegment*>(editObject)->spanner();
            Spanner* original = static_cast<SpannerSegment*>(origEditObject)->spanner();

            if (!spanner->isEdited(original)) {
                  UndoStack* undo = _score->undo();
                  undo->current()->unwind();
                  _score->select(editObject);
                  _score->addRefresh(editObject->canvasBoundingRect());
                  _score->addRefresh(origEditObject->canvasBoundingRect());
                  _score->deselect(editObject);
                  _score->select(origEditObject);
                  delete spanner;
                  origEditObject = 0;
                  editObject = 0;
                  grips = 0;
                  _score->endCmd();
                  mscore->endCmd();
                  return;
                  }

            // handle linked elements
            LinkedElements* le = original->links();
            Element* se = spanner->startElement();
            Element* ee = spanner->endElement();
            if (le && (se != original->startElement() || ee != original->endElement())) {
                  //
                  // apply anchor changes
                  // to linked elements
                  //
                  foreach(Element* e, *le) {
                        if (e == spanner)
                              continue;
                        Spanner* lspanner = static_cast<Spanner*>(e);
                        Element* lse = 0;
                        Element* lee = 0;
                        Score* sc = lspanner->score();

                        if (se->type() == NOTE || se->type() == CHORD) {
                              foreach(Element* e, *se->links()) {
                                    if (e->score() == sc && e->staffIdx() == se->staffIdx()) {
                                          lse = e;
                                          break;
                                          }
                                    }
                              foreach(Element* e, *ee->links()) {
                                    if (e->score() == sc && e->staffIdx() == ee->staffIdx()) {
                                          lee = e;
                                          break;
                                          }
                                    }
                              }
                        else if (se->type() == SEGMENT) {
                              int tick   = static_cast<Segment*>(se)->tick();
                              Measure* m = sc->tick2measure(tick);
                              lse        = m->findSegment(SegChordRest, tick);

                              int tick2  = static_cast<Segment*>(ee)->tick();
                              m          = sc->tick2measure(tick2);
                              lee        = m->findSegment(SegChordRest, tick2);
                              }
                        else if (se->type() == MEASURE) {
                              Measure* measure = static_cast<Measure*>(se);
                              int tick         = measure->tick();
                              lse              = sc->tick2measure(tick);

                              measure          = static_cast<Measure*>(ee);
                              tick             = measure->tick();
                              lee              = sc->tick2measure(tick);
                              }
                        Q_ASSERT(lse && lee);
                        score()->undo()->push(new ChangeSpannerAnchor(lspanner, lse, lee));
                        }
                  }
            }

      _score->addRefresh(editObject->canvasBoundingRect());

      int tp = editObject->type();
      if (tp == LYRICS)
            lyricsEndEdit();
      else if (tp == HARMONY)
            harmonyEndEdit();
      _score->endCmd();
      mscore->endCmd();
      if (dragElement && (dragElement != editObject)) {
            curElement = dragElement;
            _score->select(curElement);
            _score->end();
            }
      _score->deselect(origEditObject);
      _score->select(editObject);
      editObject     = 0;
      origEditObject = 0;
      grips          = 0;
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void ScoreView::moveCursor()
      {
      const InputState& is = _score->inputState();
      int track = is.track() == -1 ? 0 : is.track();
      Segment* segment = is.segment();
      if (segment)
            moveCursor(segment, track);
      }

//---------------------------------------------------------
//   moveCursor
//    move cursor during playback
//---------------------------------------------------------

void ScoreView::moveCursor(int tick)
      {
      Measure* measure = score()->tick2measure(tick);
      if (measure == 0)
            return;

      qreal x;
      Segment* s;
      for (s = measure->first(SegChordRest); s;) {
            int t1 = s->tick();
            int x1 = s->canvasPos().x();
            qreal x2;
            int t2;
            Segment* ns = s->next(SegChordRest);
            if (ns) {
                  t2 = ns->tick();
                  x2 = ns->canvasPos().x();
                  }
            else {
                  t2 = measure->endTick();
                  x2 = measure->canvasPos().x() + measure->width();
                  }
            if (tick >= t1 && tick < t2) {
                  int   dt = t2 - t1;
                  qreal dx = x2 - x1;
                  x = x1 + dx * (tick-t1) / dt;
                  break;
                  }
            s = ns;
            }
      if (s == 0)
            return;

      QColor c(MScore::selectColor[0]);
      c.setAlpha(50);
      _cursor->setColor(c);
      _cursor->setTick(tick);

      System* system = measure->system();
      if (system == 0)
            return;
      double y        = system->staffY(0) + system->page()->pos().y();
      double _spatium = score()->spatium();

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      qreal mag = _spatium / (DPI * SPATIUM20);
      double w  = _spatium * 2.0 + symbols[score()->symIdx()][quartheadSym].width(mag);
      double h  = 10 * _spatium;
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
      x -= _spatium;
      y -= 3 * _spatium;

      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
      if (mscore->panDuringPlayback())
            adjustCanvasPosition(measure, true);
      }

void ScoreView::moveCursor(Segment* segment, int track)
      {
      int voice = track == -1 ? 0 : track % VOICES;
      QColor c(MScore::selectColor[voice]);
      c.setAlpha(50);
      _cursor->setColor(c);
      _cursor->setTick(segment->tick());

      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            printf("zero SYSTEM\n");
            return;
            }
      int staffIdx    = track == -1 ? 0 : track / VOICES;
      double x        = segment->canvasPos().x();
      double y        = system->staffY(staffIdx) + system->page()->pos().y();
      double _spatium = score()->spatium();

      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));

      double h;
      qreal mag = _spatium / (DPI * SPATIUM20);
      double w  = _spatium * 2.0 + symbols[score()->symIdx()][quartheadSym].width(mag);

      if (track == -1) {
            h  = 6 * _spatium;
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
            x -= _spatium;
            y -= _spatium;
            }
      else {
            Staff* staff    = _score->staff(staffIdx);
            double lineDist = staff->useTablature() ? 1.5 * _spatium : _spatium;
            int lines       = staff->lines();
            h               = (lines - 1) * lineDist + 4 * _spatium;
            x              -= _spatium;
            y              -= 2.0 * _spatium;
            }
      _cursor->setRect(QRectF(x, y, w, h));
      update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
      }

//---------------------------------------------------------
//   cursorTick
//---------------------------------------------------------

int ScoreView::cursorTick() const
      {
      return _cursor->tick();
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void ScoreView::setCursorOn(bool val)
      {
      if (_cursor && (_cursor->visible() != val)) {
            _cursor->setVisible(val);
            update(_matrix.mapRect(_cursor->rect()).toRect().adjusted(-1,-1,1,1));
            }
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
      const Instrument* instr = staff->part()->instr();
      int noteheadGroup = 0;
      int line          = pos.line;
      int noteHead      = score()->inputState().duration().headType();

      if (instr->useDrumset()) {
            Drumset* ds  = instr->drumset();
            int pitch    = score()->inputState().drumNote();
            if (pitch >= 0 && ds->isValid(pitch)) {
                  line     = ds->line(pitch);
                  noteheadGroup = ds->noteHead(pitch);
                  }
            }
      shadowNote->setLine(line);
      Sym* s;
      if (score()->inputState().rest) {
            int yo;
            int id = Rest::getSymbol(score()->inputState().duration().type(), 0,
               staff->lines(), &yo);
            s = &symbols[0][id];
            }
      else
            s = noteHeadSym(true, noteheadGroup, noteHead);
      shadowNote->setSym(s);
      shadowNote->setPos(pos.pos);
      }

//---------------------------------------------------------
//   paintElement
//---------------------------------------------------------

#if 0
static void paintElement(void* data, Element* e)
      {
      PainterQt* p = static_cast<PainterQt*>(data);
      p->painter()->save();
      p->painter()->setPen(QPen(e->curColor()));
      p->painter()->translate(e->pagePos());
      e->draw(p);
      p->painter()->restore();
      }
#endif

//---------------------------------------------------------
//   paintEvent
//    Note: desktop background and paper background are not
//    scaled
//---------------------------------------------------------

void ScoreView::paintEvent(QPaintEvent* ev)
      {
      if (!_score)
            return;
      QPainter p(this);
      p.setRenderHint(QPainter::Antialiasing, preferences.antialiasedDrawing);
      p.setRenderHint(QPainter::TextAntialiasing, true);
      PainterQt vp(&p, this);

#if 0
      // for whatever reason this does not work
      foreach(const QRect& r, ev->region().rects())
            paint(r.adjusted(-2, -2, 2, 2), p); // adjust for antialiased drawing
#else
      paint(ev->rect(), p);
#endif

      p.setTransform(_matrix);
      p.setClipping(false);

      if (_cursor && _cursor->visible())
            p.fillRect(_cursor->rect(), _cursor->color());

      lasso->draw(&vp);
      if (fotoMode())
            _foto->draw(&vp);
      shadowNote->draw(&vp);
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / p.worldMatrix().m11(), Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }
//      if (dragElement)
//            dragElement->scanElements(&vp, paintElement, false);

      if (grips) {
            qreal lw = 2.0/p.matrix().m11();
            QPen pen(Qt::gray);
            pen.setWidthF(lw);
            if (grips >= 4) {
                  p.setPen(pen);
                  QPolygonF polygon(grips+1);
                  for (int i = 0; i < grips; ++i)
                        polygon[i] = QPointF(grip[i].center());
                  polygon[grips] = QPointF(grip[0].center());
                  p.drawPolyline(polygon);
                  }
            pen.setColor(MScore::defaultColor);
            p.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  p.setBrush(((i == curGrip) && hasFocus()) ? QBrush(Qt::blue) : Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
            }
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void ScoreView::drawBackground(QPainter& p, QRectF r)
      {
      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(r, _fgColor);
      else {
            p.drawTiledPixmap(r, *fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }
      }

//---------------------------------------------------------
//   paintPageBorder
//---------------------------------------------------------

void ScoreView::paintPageBorder(QPainter& p, Page* page)
      {
      QRectF r(page->bbox());
      qreal x1 = r.x();
      qreal y1 = r.y();
      qreal x2 = x1 + r.width();
      qreal y2 = y1 + r.height();

      const QColor c1("#befbbefbbefb");
      const QColor c2("#79e779e779e7");
      int h1, h2, s1, s2, v1, v2;
      int bw = 6;
      c2.getHsv(&h1, &s1, &v1);
      c1.getHsv(&h2, &s2, &v2);

      if (page->no() & 1) {
            int bbw = bw/2-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw/2; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(QPen(c));
                  p.drawLine(QLineF(x1+i, y1, x1+i, y2));
                  }
            c1.getHsv(&h1, &s1, &v1);
            c2.getHsv(&h2, &s2, &v2);

            p.fillRect(x2-bw, y1, bw, bw, MScore::bgColor);
            p.fillRect(x2-bw, y2-bw, bw, bw, MScore::bgColor);

            bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(QPen(c));
                  p.drawLine(QLineF(x2-bw+i, y1+i+1, x2-bw+i, y2-i-1));
                  }
            }
      else {
            c2.getHsv(&h1, &s1, &v1);
            c1.getHsv(&h2, &s2, &v2);

            p.fillRect(x1, y1, bw, bw, MScore::bgColor);
            p.fillRect(x1, y2-bw, bw, bw, MScore::bgColor);
            int bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(QColor(c));
                  p.drawLine(QLineF(x1+i, y1+(bw-i), x1+i, y2-(bw-i)-1));
                  }
            c1.getHsv(&h1, &s1, &v1);
            c2.getHsv(&h2, &s2, &v2);

            bw/=2;
            bbw = bw-1;
            bbw = bbw >= 1 ? bbw : 1;
            for (int i = 0; i < bw; ++i) {
                  QColor c;
                  c.setHsv(h1+((h2-h1)*i)/bbw,
                     s1+((s2-s1)*i)/bbw,
                     v1+((v2-v1)*i)/bbw);
                  p.setPen(QColor(c));
                  p.drawLine(QLineF(x2-bw+i, y1, x2-bw+i, y2));
                  }
            }
      if (_score->showPageborders()) {
            // show page margins
            p.setPen(QColor(0, 0, 255, 50));
            QRectF f(page->canvasBoundingRect());
            f.adjust(page->lm(), page->tm(), -page->rm(), -page->bm());
            p.drawRect(f);
            }
      }

//---------------------------------------------------------
//   paint
//---------------------------------------------------------

void ScoreView::paint(const QRect& r, QPainter& p)
      {
      p.save();
      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(r, _fgColor);
      else {
            p.drawTiledPixmap(r, *fgPixmap, r.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }

      p.setTransform(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(r));

      QRegion r1(r);
      foreach (Page* page, _score->pages()) {
            if (!score()->printing())
                  paintPageBorder(p, page);
            QRectF pr(page->abbox().translated(page->pos()));
            if (pr.right() < fr.left())
                  continue;
            if (pr.left() > fr.right())
                  break;
            QList<const Element*> ell = page->items(fr.translated(-page->pos()));
            qStableSort(ell.begin(), ell.end(), elementLessThan);
            p.save();
            p.translate(page->pos());
            drawElements(p, ell);
            p.restore();
            r1 -= _matrix.mapRect(pr).toAlignedRect();
            }

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      //
      // frame text in edit mode, except for text in a text frame
      //
      if (_editText && !(_editText->parent() && _editText->parent()->type() == TBOX)) {
            Element* e = _editText;
            while (e->parent())
                  e = e->parent();
            QRectF r = _editText->pageRectangle().translated(e->pos()); // abbox();
            p.setPen(QPen(QBrush(Qt::blue), 2.0 / matrix().m11()));  // 2 pixel pen size
            p.setBrush(QBrush(Qt::NoBrush));
            p.drawRect(r);
            }
      const Selection& sel = _score->selection();
      if (sel.state() == SEL_RANGE) {
            Segment* ss = sel.startSegment();
            Segment* es = sel.endSegment();
            if (!ss)
                  return;
            p.setBrush(Qt::NoBrush);

            QPen pen;
            pen.setColor(Qt::blue);
            pen.setWidthF(2.0 / p.matrix().m11());

            pen.setStyle(Qt::SolidLine);

            p.setPen(pen);
            double _spatium = score()->spatium();
            double x2      = ss->pagePos().x() - _spatium;
            int staffStart = sel.staffStart();
            int staffEnd   = sel.staffEnd();

            System* system2 = ss->measure()->system();
            QPointF pt      = ss->pagePos();
            double y        = pt.y();
            SysStaff* ss1   = system2->staff(staffStart);
            SysStaff* ss2   = system2->staff(staffEnd - 1);
            double y1       = ss1->y() - 2 * _spatium + y;
            double y2       = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;

            // drag vertical start line
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));

            System* system1 = system2;
            double x1;

            for (Segment* s = ss; s && (s != es);) {
                  Segment* ns = s->next1();
                  system1  = system2;
                  system2  = s->measure()->system();
                  pt       = s->pagePos();
                  x1  = x2;
                  x2  = pt.x() + _spatium * 2;

                  // HACK for whole measure rest:
                  if (ns == 0 || ns == es) {    // last segment?
                        Element* e = s->element(staffStart * VOICES);
                        if (e && e->type() == REST && static_cast<Rest*>(e)->durationType().type() == Duration::V_MEASURE)
                              x2 = s->measure()->abbox().right() - _spatium;
                        }

                  if (system2 != system1)
                        x1  = x2 - 2 * _spatium;
                  y   = pt.y();
                  ss1 = system2->staff(staffStart);
                  ss2 = system2->staff(staffEnd - 1);
                  y1  = ss1->y() - 2 * _spatium + y;
                  y2  = ss2->y() + ss2->bbox().height() + 2 * _spatium + y;
                  p.drawLine(QLineF(x1, y1, x2, y1).translated(system2->page()->pos()));
                  p.drawLine(QLineF(x1, y2, x2, y2).translated(system2->page()->pos()));
                  s = ns;
                  }
            //
            // draw vertical end line
            //
            p.drawLine(QLineF(x2, y1, x2, y2).translated(system2->page()->pos()));
            }

      p.setMatrixEnabled(false);
      if (!r1.isEmpty()) {
            p.setClipRegion(r1);  // only background
            if (bgPixmap == 0 || bgPixmap->isNull())
                  p.fillRect(r, _bgColor);
            else
                  p.drawTiledPixmap(r, *bgPixmap, r.topLeft()-QPoint(lrint(xoffset()), lrint(yoffset())));
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
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      }

//---------------------------------------------------------
//   dragTimeAnchorElement
//    pos is in canvas coordinates
//---------------------------------------------------------

bool ScoreView::dragTimeAnchorElement(const QPointF& pos)
      {
      int staffIdx;
      Segment* seg;
      MeasureBase* mb = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
      if (mb && mb->type() == MEASURE) {
            Measure* m = static_cast<Measure*>(mb);
            System* s  = m->system();
            qreal y    = s->staff(staffIdx)->y() + s->pos().y() + s->page()->pos().y();
            QPointF anchor(seg->canvasBoundingRect().x(), y);
            setDropAnchor(QLineF(pos, anchor));
            dragElement->setTrack(staffIdx * VOICES);
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
      Measure* m = _score->searchMeasure(pos);
      if (m) {
            QRectF b(m->canvasBoundingRect());

            QPointF anchor;
            if (pos.x() < (b.x() + b.width() * .5))
                  anchor = m->canvasBoundingRect().topLeft();
            else
                  anchor = m->canvasBoundingRect().topRight();
            setDropAnchor(QLineF(pos, anchor));
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
      if (debugMode)
            printf("dragEnterEvent\n");
      double _spatium = score()->spatium();
      dragElement = 0;

      const QMimeData* data = event->mimeData();

      if (data->hasFormat(mimeSymbolListFormat)
         || data->hasFormat(mimeStaffListFormat)) {
            event->acceptProposedAction();
            return;
            }

      if (data->hasFormat(mimeSymbolFormat)) {
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
            if (type == IMAGE) {
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

#ifdef SVG_IMAGES
                  if (lp.endsWith(".svg"))
                        image = new SvgImage(score());
                  else
#endif
                        if (lp.endsWith(".jpg")
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
            else {
                  el = Element::create(type, score());
                  if (type == BAR_LINE || type == ARPEGGIO || type == BRACKET)
                        el->setHeight(_spatium * 5);
                  else if (el->isText()) {
                        Text* text = static_cast<Text*>(el);
                        QString sname = text->styleName();
                        if (!text->styled() && !sname.isEmpty()) {
                              // check if we can transform text into styled text
                              Style* s = score()->style();
                              TextStyleType st = s->textStyleType(sname);
                              if (st != TEXT_STYLE_INVALID)
                                    text->setTextStyle(st);
                              }
                        }
                  }
            if (el) {
                  dragElement = el;
                  dragElement->setParent(0);
                  dragElement->read(e);
                  dragElement->layout();
                  }
            return;
            }

      if (data->hasUrls()) {
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
            return;
            }
      QStringList formats = data->formats();
      printf("unknown drop format: formats:\n");
      foreach(const QString& s, formats)
            printf("  <%s>\n", qPrintable(s));
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
            if (e->acceptDrop(this, pos, dragElement->type(), dragElement->subtype())) {
                  setDropTarget(e);
                  return;
                  }
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
                  case FRET_DIAGRAM:
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
                  case CHORDLINE:
                  case BEND:
                  case ACCIDENTAL:
                  case TEXT:
                  case FINGERING:
                  case TEMPO_TEXT:
                  case STAFF_TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case MARKER:
                  case STAFF_STATE:
                  case INSTRUMENT_CHANGE:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case CHORD:
                  case SPACER:
                  case SLUR:
                  case ACCIDENTAL_BRACKET:
                  case HARMONY:
                        {
                        QList<const Element*> el = elementsAt(pos);
                        bool found = false;
                        foreach(const Element* e, el) {
                              if (e->acceptDrop(this, pos, dragElement->type(), dragElement->subtype())) {
                                    if (e->type() != MEASURE)
                                          setDropTarget(const_cast<Element*>(e));
                                    found = true;
                                    break;
                                    }
                              }
                        if (!found)
                              setDropTarget(0);
                        }
                        break;
                  default:
                        break;
                  }
            score()->addRefresh(dragElement->canvasBoundingRect());
            dragElement->setPos(pos);
            score()->addRefresh(dragElement->canvasBoundingRect());
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

      DropData dropData;
      dropData.view       = this;
      dropData.pos        = pos;
      dropData.dragOffset = dragOffset;
      dropData.element    = dragElement;
      dropData.modifiers  = event->keyboardModifiers();

      if (dragElement) {
            _score->startCmd();
            dragElement->setScore(_score);      // CHECK: should already be ok
            _score->addRefresh(dragElement->canvasBoundingRect());
            switch(dragElement->type()) {
                  case VOLTA:
                  case OTTAVA:
                  case TRILL:
                  case PEDAL:
                  case HAIRPIN:
                  case TEXTLINE:
                        {
                        dragElement->setScore(score());
                        Spanner* spanner = static_cast<Spanner*>(dragElement);
                        score()->cmdAddSpanner(spanner, pos, dragOffset);
                        event->acceptProposedAction();
                        }
                        break;
                  case SYMBOL:
                  case IMAGE:
                  case DYNAMIC:
                  case FRET_DIAGRAM:
                  case HARMONY:
                        {
                        Element* el = elementAt(pos);
                        if (el == 0) {
                              int staffIdx;
                              Segment* seg;
                              el = _score->pos2measure(pos, &staffIdx, 0, &seg, 0);
                              if (el == 0) {
                                    printf("cannot drop here\n");
                                    delete dragElement;
                                    dragElement = 0;
                                    break;
                                    }
                             }
                        _score->addRefresh(el->canvasBoundingRect());
                        _score->addRefresh(dragElement->canvasBoundingRect());

                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->canvasBoundingRect());
                        if (dropElement) {
                              _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->canvasBoundingRect());
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
                  case CHORDLINE:
                  case BEND:
                  case ACCIDENTAL:
                  case TEXT:
                  case FINGERING:
                  case TEMPO_TEXT:
                  case STAFF_TEXT:
                  case NOTEHEAD:
                  case TREMOLO:
                  case LAYOUT_BREAK:
                  case MARKER:
                  case STAFF_STATE:
                  case INSTRUMENT_CHANGE:
                  case JUMP:
                  case REPEAT_MEASURE:
                  case ICON:
                  case NOTE:
                  case CHORD:
                  case SPACER:
                  case SLUR:
                  case ACCIDENTAL_BRACKET:
                        {
                        Element* el = 0;
                        foreach(const Element* e, elementsAt(pos)) {
                              if (e->acceptDrop(this, pos, dragElement->type(), dragElement->subtype())) {
                                    el = const_cast<Element*>(e);
                                    break;
                                    }
                              }
                        if (!el) {
                              printf("cannot drop here\n");
                              delete dragElement;
                              break;
                              }
                        _score->addRefresh(el->canvasBoundingRect());
                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->canvasBoundingRect());
                        if (dropElement) {
                              if (!_score->noteEntryMode())
                                    _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->canvasBoundingRect());

                              Qt::KeyboardModifiers keyState = event->keyboardModifiers();
                              if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
                                    //
                                    // continue drag with a cloned element
                                    //
//??                                    CloneEvent* ce = new CloneEvent(dropElement);
//                                    QCoreApplication::postEvent(this, ce);
                                    }
                              }
                        event->acceptProposedAction();
                        }
                        break;
                  default:
                        delete dragElement;
                        break;
                  }
            dragElement = 0;
            setDropTarget(0); // this also resets dropRectangle and dropAnchor
            score()->endCmd();
            mscore->endCmd();
            return;
            }

      if (event->mimeData()->hasUrls()) {
            QList<QUrl>ul = event->mimeData()->urls();
            QUrl u = ul.front();
            if (u.scheme() == "file") {
                  QFileInfo fi(u.path());
                  Image* s = 0;
                  QString suffix = fi.suffix().toLower();
#ifdef SVG_IMAGES
                  if (suffix == "svg")
                        s = new SvgImage(score());
                  else
#endif
                        if (suffix == "jpg"
                     || suffix == "png"
                     || suffix == "gif"
                     || suffix == "xpm"
                        )
                        s = new RasterImage(score());
                  else
                        return;
                  _score->startCmd();
                  QString str(u.toLocalFile());
                  s->setPath(str);
if (debugMode)
      printf("drop image <%s> <%s>\n", qPrintable(str), qPrintable(s->path()));

                  Element* el = elementAt(pos);
                  if (el && (el->type() == NOTE || el->type() == REST)) {
                        s->setTrack(el->track());
                        if (el->type() == NOTE) {
                              Note* note = (Note*)el;
                              // s->setTick(note->chord()->tick());
                              s->setParent(note->chord()->segment()->measure());
                              }
                        else  {
                              Rest* rest = (Rest*)el;
                              // s->setTick(rest->tick());
                              s->setParent(rest->segment()->measure());
                              }
                        score()->undoAddElement(s);
                        }
                  else
                        score()->cmdAddBSymbol(s, pos, dragOffset);

                  event->acceptProposedAction();
                  score()->endCmd();
                  mscore->endCmd();
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
                  while (seg->subtype() != SegChordRest)
                        seg = seg->next();
                  score()->pasteStaff(doc.documentElement(), (ChordRest*)(seg->element(idx * VOICES)));
                  }
            event->acceptProposedAction();
            _score->setLayoutAll(true);
            _score->endCmd();
            mscore->endCmd();
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
            _score->addRefresh(dragElement->canvasBoundingRect());
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
      qreal _mag = mag();

      if (step > 0) {
            for (int i = 0; i < step; ++i) {
                   _mag *= 1.1;
                  }
            }
      else {
            for (int i = 0; i < -step; ++i) {
                  _mag /= 1.1;
                  }
            }

      zoom(_mag, QPointF(pos));
      }

//---------------------------------------------------------
//   zoom
//---------------------------------------------------------

void ScoreView::zoom(qreal _mag, const QPointF& pos)
      {

      QPointF p1 = imatrix.map(pos);

      if (_mag > 16.0)
            _mag = 16.0;
      else if (_mag < 0.05)
            _mag = 0.05;

      mscore->setMag(_mag);
      setMag(_mag);
      _magIdx = MAG_FREE;

      QPointF p2 = imatrix.map(pos);
      QPointF p3 = p2 - p1;

      double m = _mag;

      int dx    = lrint(p3.x() * m);
      int dy    = lrint(p3.y() * m);

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit viewRectChanged();
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      update();
      }

//---------------------------------------------------------
//   gestureEvent
//---------------------------------------------------------

bool ScoreView::gestureEvent(QGestureEvent *event)
      {
      if (QGesture *gesture = event->gesture(Qt::PinchGesture)) {
            // Zoom in/out when receiving a pinch gesture
            QPinchGesture *pinch = static_cast<QPinchGesture *>(gesture);

            static qreal magStart = 1.0;

            if (pinch->state() == Qt::GestureStarted) {
                  magStart = mag();
                  }
            if (pinch->changeFlags() & QPinchGesture::ScaleFactorChanged) {
                  qreal value = pinch->property("scaleFactor").toReal();
                  zoom(magStart*value, pinch->startCenterPoint());
                  }
            }
      return true;
      }

//---------------------------------------------------------
//   wheelEvent
//---------------------------------------------------------

void ScoreView::wheelEvent(QWheelEvent* event)
      {
      static int deltaSum = 0;
      deltaSum += event->delta();
      int n = deltaSum / 120;
      deltaSum %= 120;

      if (event->buttons() & Qt::RightButton) {
            bool up = n > 0;
            if (!up)
                  n = -n;
            score()->startCmd();
            for (int i = 0; i < n; ++i)
                  score()->upDown(up, UP_DOWN_CHROMATIC);
            score()->endCmd();
            return;
            }
      if (event->modifiers() & Qt::ControlModifier) {
            QApplication::sendPostedEvents(this, 0);
            zoom(n, event->pos());
            return;
            }
      int dx = 0;
      int dy = 0;
      if (event->modifiers() & Qt::ShiftModifier || event->orientation() == Qt::Horizontal) {
            //
            //    scroll horizontal
            //
            dx = n * qMax(2, width() / 10);
            }
      else {
            //
            //    scroll vertical
            //
            dy = n * qMax(2, height() / 10);
            }

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();

      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit viewRectChanged();
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      }

//---------------------------------------------------------
//   elementLower
//---------------------------------------------------------

static bool elementLower(const Element* e1, const Element* e2)
      {
      if (!e1->selectable())
            return false;
      return e1->z() < e2->z();
      }

//---------------------------------------------------------
//   point2page
//---------------------------------------------------------

Page* ScoreView::point2page(const QPointF& p)
      {
      foreach(Page* page, score()->pages()) {
            if (page->bbox().translated(page->pos()).contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   elementsAt
//    p is in canvas coordinates
//---------------------------------------------------------

const QList<const Element*> ScoreView::elementsAt(const QPointF& p)
      {
      QList<const Element*> el;

      Page* page = point2page(p);
      if (page) {
            el = page->items(p - page->pos());
            qSort(el.begin(), el.end(), elementLower);
            }
      return el;
      }

//---------------------------------------------------------
//   elementAt
//---------------------------------------------------------

Element* ScoreView::elementAt(const QPointF& p)
      {
      QList<const Element*> el = elementsAt(p);
#if 0
      printf("elementAt\n");
      foreach(const Element* e, el)
            printf("  %s %d\n", e->name(), e->selected());
#endif
      const Element* e = el.value(0);
      if (e && (e->type() == PAGE))
            e = el.value(1);
      return const_cast<Element*>(e);
      }

//---------------------------------------------------------
//   elementNear
//---------------------------------------------------------

Element* ScoreView::elementNear(QPointF p)
      {
      Page* page = point2page(p);
      if (!page) {
            // printf("  no page\n");
            return 0;
            }

      p -= page->pos();
      double w  = (preferences.proximity * .5) / matrix().m11();
      QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

      QList<const Element*> el = page->items(r);
      QList<const Element*> ll;
      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->selectable() || e->type() == PAGE)
                  continue;
            if (e->contains(p))
                  ll.append(e);
            }
      int n = ll.size();
      if ((n == 0) || ((n == 1) && (ll[0]->type() == MEASURE))) {
            //
            // if no relevant element hit, look nearby
            //
            foreach(const Element* e, el) {
                  if (e->type() == PAGE || !e->selectable())
                        continue;
                  if (e->intersects(r))
                        ll.append(e);
                  }
            }
      if (ll.empty()) {
            // printf("  nothing found\n");
            return 0;
            }
      qSort(ll.begin(), ll.end(), elementLower);

#if 0
      printf("elementNear\n");
      foreach(const Element* e, ll)
            printf("  %s selected %d z %d\n", e->name(), e->selected(), e->z());
#endif
      Element* e = const_cast<Element*>(ll.at(0));
      return e;
      }

//---------------------------------------------------------
//   drawDebugInfo
//---------------------------------------------------------

static void drawDebugInfo(QPainter& p, const Element* e)
      {
      //
      //  draw bounding box rectangle for all
      //  selected Elements
      //
      p.setBrush(Qt::NoBrush);

      // p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
      // p.drawPath(e->shape());

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
            p.save();
            // we are now in page coordinate system
            const Element* ee = e->parent();
            if (e->type() == NOTE)
                  ee = static_cast<const Note*>(e)->chord()->segment();
            else if (e->type() == CLEF)
                  ee = static_cast<const Clef*>(e)->segment();

            p.setPen(QPen(Qt::green, 0, Qt::SolidLine));
            p.drawRect(ee->pageBoundingRect());

            if (ee->type() == SEGMENT) {
                  QPointF pt = ee->pagePos();
                  p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
                  p.drawLine(QLineF(pt.x()-w, pt.y()-h, pt.x()+w, pt.y()+h));
                  p.drawLine(QLineF(pt.x()+w, pt.y()-h, pt.x()-w, pt.y()+h));
                  }
            }
      }

//---------------------------------------------------------
//   drawElements
//---------------------------------------------------------

void ScoreView::drawElements(QPainter& p, const QList<const Element*>& el)
      {
      PainterQt painter(&p, this);

      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (score()->printing() || !score()->showInvisible())
                        continue;
                  }
            p.save();
            QPointF pos(e->pagePos());
            p.translate(pos);
            p.setPen(QPen(e->curColor()));
            e->draw(&painter);

            if (debugMode && e->selected())
                  drawDebugInfo(p, e);
            p.restore();
            }
      }

//---------------------------------------------------------
//   setMag
//---------------------------------------------------------

void ScoreView::setMag(qreal nmag)
      {
      qreal m = mag();

      if (nmag == m)
            return;
      double deltamag = nmag / m;

      _matrix.setMatrix(nmag, _matrix.m12(), _matrix.m13(), _matrix.m21(),
         nmag, _matrix.m23(), _matrix.dx()*deltamag, _matrix.dy()*deltamag, _matrix.m33());
      imatrix = _matrix.inverted();
      emit scaleChanged(nmag * score()->spatium());
      if (grips) {
            qreal w = 8.0 / nmag;
            qreal h = 8.0 / nmag;
            QRectF r(-w*.5, -h*.5, w, h);
            for (int i = 0; i < grips; ++i) {
                  QPointF p(grip[i].center());
                  grip[i] = r.translated(p);
                  }
            }
      }

//---------------------------------------------------------
//   setMagIdx
//---------------------------------------------------------

void ScoreView::setMag(int idx, double mag)
      {
      _magIdx = idx;
      setMag(mag);
      update();
      }

//---------------------------------------------------------
//   focusInEvent
//---------------------------------------------------------

void ScoreView::focusInEvent(QFocusEvent* event)
      {
      if(this != mscore->currentScoreView())
         mscore->setCurrentScoreView(this);

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
      if (editObject && editObject->isText()) {
            //
            // store selection as plain text
            //
            Text* text = static_cast<Text*>(editObject);
            QTextCursor* tcursor = text->getCursor();
            if (tcursor && tcursor->hasSelection())
                  QApplication::clipboard()->setText(tcursor->selectedText(), QClipboard::Clipboard);
            }
      }

//---------------------------------------------------------
//   normalCopy
//---------------------------------------------------------

void ScoreView::normalCopy()
      {
      if (!_score->selection().canCopy()) {
            printf("cannot copy selection: intersects a tuplet\n");
            return;
            }
      QString mimeType = _score->selection().mimeType();
      if (!mimeType.isEmpty()) {
            QMimeData* mimeData = new QMimeData;
            mimeData->setData(mimeType, _score->selection().mimeData());
            if (debugMode)
                  printf("cmd copy: <%s>\n", mimeData->data(mimeType).data());
            QApplication::clipboard()->setMimeData(mimeData);
            }
      }

//---------------------------------------------------------
//   normalCut
//---------------------------------------------------------

void ScoreView::normalCut()
      {
      if (!_score->selection().canCopy()) {
            printf("cannot copy selection: intersects a tuplet\n");
            return;
            }
      _score->startCmd();
      normalCopy();
      _score->cmdDeleteSelection();
      _score->endCmd();
      }

//---------------------------------------------------------
//   editPaste
//---------------------------------------------------------

void ScoreView::editPaste()
      {
      if (editObject->isText()) {
            static_cast<Text*>(editObject)->paste();
#if defined(Q_WS_MAC) || defined(__MINGW32__)
            QClipboard::Mode mode = QClipboard::Clipboard;
#else
            QClipboard::Mode mode = QClipboard::Selection;
#endif
            QString txt = QApplication::clipboard()->text(mode);
            if (editObject->type() == LYRICS && !txt.isEmpty())
                  lyricsTab(false, false, true);
            }
      }

//---------------------------------------------------------
//   normalPaste
//---------------------------------------------------------

void ScoreView::normalPaste()
      {
      _score->startCmd();
      _score->cmdPaste(this);
      _score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   cmd
//---------------------------------------------------------

void ScoreView::cmd(const QAction* a)
      {
      QString cmd(a ? a->data().toString() : "");
      if (debugMode)
            printf("ScoreView::cmd <%s>\n", qPrintable(cmd));

      if (cmd == "escape")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "note-input" || cmd == "copy" || cmd == "paste"
         || cmd == "cut" || cmd == "fotomode") {
            sm->postEvent(new CommandEvent(cmd));
            }
      else if (cmd == "lyrics") {
            Lyrics* lyrics = _score->addLyrics();
            if (lyrics) {
                  _score->setLayoutAll(true);
                  startEdit(lyrics);
                  return;     // no endCmd()
                  }
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
      else if (cmd == "insert-c")
            cmdInsertNote(0);
      else if (cmd == "insert-d")
            cmdInsertNote(1);
      else if (cmd == "insert-e")
            cmdInsertNote(2);
      else if (cmd == "insert-f")
            cmdInsertNote(3);
      else if (cmd == "insert-g")
            cmdInsertNote(4);
      else if (cmd == "insert-a")
            cmdInsertNote(5);
      else if (cmd == "insert-b")
            cmdInsertNote(6);
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
      else if (cmd == "system-text")
            cmdAddText(TEXT_SYSTEM);
      else if (cmd == "staff-text")
            cmdAddText(TEXT_STAFF);
      else if (cmd == "rehearsalmark-text")
            cmdAddText(TEXT_REHEARSAL_MARK);
      else if (cmd == "edit-element") {
            Element* e = _score->selection().element();
            if (e) {
                  _score->setLayoutAll(false);
                  startEdit(e);
                  }
            }
      else if (cmd == "play") {
            if (seq->canStart())
                  sm->postEvent(new CommandEvent(cmd));
            else
                  getAction("play")->setChecked(false);
            }
      else if (cmd == "find")
            sm->postEvent(new CommandEvent(cmd));
      else if (cmd == "page-prev")
            pagePrev();
      else if (cmd == "page-next")
            pageNext();
      else if (cmd == "page-top")
            pageTop();
      else if (cmd == "page-end")
            pageEnd();
      else if (cmd == "select-next-chord"
         || cmd == "select-prev-chord"
         || cmd == "select-next-measure"
         || cmd == "select-prev-measure"
         || cmd == "select-begin-line"
         || cmd == "select-end-line"
         || cmd == "select-begin-score"
         || cmd == "select-end-score"
         || cmd == "select-staff-above"
         || cmd == "select-staff-below") {
            Element* el = _score->selectMove(cmd);
            if (el)
                  adjustCanvasPosition(el, false);
            updateAll();
            }
      else if (cmd == "next-chord"
         || cmd == "prev-chord"
         || cmd == "next-measure"
         || cmd == "prev-measure") {
            Element* el = score()->selection().element();
            if (el && (el->type() == FINGERING)) {
                  score()->startCmd();
                  QPointF pt(MScore::nudgeStep * el->spatium(), 0.0);
                  if (cmd == "prev-chord")
                        score()->undoMove(el, el->userOff() - pt);
                  else if (cmd == "next-chord")
                        score()->undoMove(el, el->userOff() + pt);
                  score()->endCmd();
                  }
            else {
                  Element* el = _score->move(cmd);
                  if (el)
                        adjustCanvasPosition(el, false);
                  updateAll();
                  }
            }
      else if (cmd == "rest")
            cmdEnterRest();
      else if (cmd == "rest-1")
            cmdEnterRest(Duration(Duration::V_WHOLE));
      else if (cmd == "rest-2")
            cmdEnterRest(Duration(Duration::V_HALF));
      else if (cmd == "rest-4")
            cmdEnterRest(Duration(Duration::V_QUARTER));
      else if (cmd == "rest-8")
            cmdEnterRest(Duration(Duration::V_EIGHT));
      else if (cmd.startsWith("interval")) {
            int n = cmd.mid(8).toInt();
            QList<Note*> nl = _score->selection().noteList();
            if (!nl.isEmpty()) {
                  if (!noteEntryMode())
                        sm->postEvent(new CommandEvent("note-input"));
                  _score->cmdAddInterval(n, nl);
                  }
            }
      else if (cmd == "tie")
            _score->cmdAddTie();
      else if (cmd == "duplet")
            cmdTuplet(2);
      else if (cmd == "triplet")
            cmdTuplet(3);
      else if (cmd == "quadruplet")
            cmdTuplet(4);
      else if (cmd == "quintuplet")
            cmdTuplet(5);
      else if (cmd == "sextuplet")
            cmdTuplet(6);
      else if (cmd == "septuplet")
            cmdTuplet(7);
      else if (cmd == "octuplet")
            cmdTuplet(8);
      else if (cmd == "nonuplet")
            cmdTuplet(9);
      else if (cmd == "repeat-sel")
            cmdRepeatSelection();
      else if (cmd == "voice-1")
            changeVoice(0);
      else if (cmd == "voice-2")
            changeVoice(1);
      else if (cmd == "voice-3")
            changeVoice(2);
      else if (cmd == "voice-4")
            changeVoice(3);
      else if (cmd == "enh-up")
            cmdChangeEnharmonic(true);
      else if (cmd == "enh-down")
            cmdChangeEnharmonic(false);
      else if (cmd == "revision") {
            Score* s = _score;
            if (s->parentScore())
                  s = s->parentScore();
            s->createRevision();
            }
      else if (cmd == "append-measure")
            cmdAppendMeasures(1, MEASURE);
      else if (cmd == "insert-measure")
	      cmdInsertMeasures(1, MEASURE);
      else if (cmd == "insert-hbox")
	      cmdInsertMeasures(1, HBOX);
      else if (cmd == "insert-vbox")
	      cmdInsertMeasures(1, VBOX);
      else if (cmd == "append-hbox") {
	      MeasureBase* mb = appendMeasure(HBOX);
            _score->select(mb, SELECT_SINGLE, 0);
            }
      else if (cmd == "append-vbox") {
	      MeasureBase* mb = appendMeasure(VBOX);
            _score->select(mb, SELECT_SINGLE, 0);
            }
      else if (cmd == "insert-textframe")
            cmdInsertMeasure(TBOX);
      else if (cmd == "append-textframe") {
            MeasureBase* mb = appendMeasure(TBOX);
            if (mb) {
                  TBox* tf = static_cast<TBox*>(mb);
                  Text* text = 0;
                  foreach(Element* e, *tf->el()) {
                        if (e->type() == TEXT) {
                              text = static_cast<Text*>(e);
                              break;
                              }
                        }
                  if (text) {
                        _score->select(text, SELECT_SINGLE, 0);
                        startEdit(text);
                        }
                  }
            }
      else if (cmd == "insert-fretframe")
            cmdInsertMeasure(FBOX);
      else if (cmd == "move-left") {
            Element* e = _score->getSelectedElement();
            if (e && (e->type() == NOTE || e->type() == REST)) {
                  if (e->type() == NOTE)
                        e = e->parent();
                  ChordRest* cr1 = static_cast<ChordRest*>(e);
                  ChordRest* cr2 = prevChordRest(cr1);
                  if (cr2) {
                        _score->startCmd();
                        _score->undoSwapCR(cr1, cr2);
                        _score->endCmd();
                        }
                  }
            }
      else if (cmd == "move-right") {
            Element* e = _score->getSelectedElement();
            if (e && (e->type() == NOTE || e->type() == REST)) {
                  if (e->type() == NOTE)
                        e = e->parent();
                  ChordRest* cr1 = static_cast<ChordRest*>(e);
                  ChordRest* cr2 = nextChordRest(cr1);
                  if (cr2) {
                        _score->startCmd();
                        _score->undoSwapCR(cr1, cr2);
                        _score->endCmd();
                        }
                  }
            }
      else if (cmd == "reset") {
            if (editMode()) {
                  editObject->toDefault();
                  updateGrips();
                  _score->end();
                  }
            else {
                  _score->startCmd();
                  foreach(Element* e, _score->selection().elements())
                        e->toDefault();
                  _score->endCmd();
                  }
            _score->setLayoutAll(true);
            }
      else if (cmd == "show-omr") {
            if (_score->omr())
                  showOmr(!_score->showOmr());
            }
      else if (cmd == "split-measure") {
            Element* e = _score->selection().element();
            if (!(e && (e->type() == NOTE || e->type() == REST))) {
                  QMessageBox::warning(0, "MuseScore",
                     tr("No chord/rest selected:\n"
                     "please select a chord/rest and try again"));
                  }
            else {
                  if (e->type() == NOTE)
                        e = static_cast<Note*>(e)->chord();
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  _score->cmdSplitMeasure(cr);
                  }
            }
      else if (cmd == "join-measure") {
            Measure* m1 = _score->selection().startSegment()->measure();
            Measure* m2 = _score->selection().endSegment()->measure();
            if (_score->selection().state() != SEL_RANGE || (m1 == m2)) {
                  QMessageBox::warning(0, "MuseScore",
                     tr("No measures selected:\n"
                     "please select range of measures to join and try again"));
                  }
            else {
                  _score->cmdJoinMeasure(m1, m2);
                  }
            }
      else if (cmd == "delete") {
            // no delete in edit mode except for slurs/ties
            if (editMode() && editObject->type() == SLUR_SEGMENT) {
                  sm->postEvent(new CommandEvent("escape"));   // leave edit mode
                  qApp->processEvents();
                  _score->startCmd();     // start new command
                  }
            _score->cmdDeleteSelection();
            }
      else
            _score->cmd(a);
      _score->processMidiInput();
      }

//---------------------------------------------------------
//   showOmr
//---------------------------------------------------------

void ScoreView::showOmr(bool flag)
      {
      _score->setShowOmr(flag);
      ScoreTab* t = mscore->getTab1();
      if (t->view() != this)
            t = mscore->getTab2();
      if (t->view() == this)
            t->setCurrent(t->currentIndex());
      else
            printf("view not found\n");
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
      {
      dragElement = curElement;
      startMove  -= dragElement->userOff();
      _score->startCmd();

      foreach(Element* e, _score->selection().elements())
            e->setStartDragPosition(e->userOff());
      QList<Element*> el;
      dragElement->scanElements(&el, collectElements);
      _score->end();
      }

//---------------------------------------------------------
//   drag
//---------------------------------------------------------

void ScoreView::drag(const QPointF& delta)
      {
      QPointF pt(delta);
      if (qApp->keyboardModifiers() == Qt::ShiftModifier)
            pt.setX(0.0);
      else if (qApp->keyboardModifiers() == Qt::ControlModifier)
            pt.setY(0.0);
      EditData data;
      data.hRaster = mscore->hRaster();
      data.vRaster = mscore->vRaster();
      data.pos     = pt;
      foreach(Element* e, _score->selection().elements())
            _score->addRefresh(e->drag(data));
      _score->end();
      if (_score->playNote()) {
            Element* e = _score->selection().element();
            if (e) {
                  mscore->play(e);
                  }
            _score->setPlayNote(false);
            }
      }

//---------------------------------------------------------
//   endDrag
//---------------------------------------------------------

void ScoreView::endDrag()
      {
      foreach(Element* e, _score->selection().elements()) {
            e->endDrag();
            QPointF npos = e->userOff();
            e->setUserOff(e->startDragPosition());
            _score->undoMove(e, npos);
            }
      _score->setLayoutAll(true);
      dragElement = 0;
      setDropTarget(0); // this also resets dropAnchor
      _score->endCmd();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   textUndoLevelAdded
//---------------------------------------------------------

void ScoreView::textUndoLevelAdded()
      {
      ++textUndoLevel;
      }

//---------------------------------------------------------
//   enableInputToolbar
//---------------------------------------------------------

static void enableInputToolbar(bool val)
      {
      static const char* actionNames[] = {
            "pad-rest", "pad-dot", "pad-dotdot", "note-longa",
            "note-breve", "pad-note-1", "pad-note-2", "pad-note-4",
            "pad-note-8", "pad-note-16", "pad-note-32", "pad-note-64",
            "pad-note-128",
//            "voice-1", "voice-2", "voice-3", "voice-4",
            "acciaccatura", "appoggiatura", "grace4", "grace16",
            "grace32", "beam-start", "beam-mid", "no-beam", "beam32",
            "auto-beam"
            };
      for (unsigned i = 0; i < sizeof(actionNames)/sizeof(*actionNames); ++i) {
            getAction(actionNames[i])->setEnabled(val);
            }
      }

//---------------------------------------------------------
//   startNoteEntry
//---------------------------------------------------------

void ScoreView::startNoteEntry()
      {
      _score->inputState().setSegment(0);
      Note* note  = 0;
      Element* el = _score->selection().activeCR() ? _score->selection().activeCR() : _score->selection().element();
      if (el == 0 || (el->type() != CHORD && el->type() != REST && el->type() != NOTE)) {
            int track = _score->inputState().track() == -1 ? 0 : _score->inputState().track();
            el = static_cast<ChordRest*>(_score->searchNote(0, track));
            if (el == 0)
                  return;
            }
      if (el->type() == CHORD) {
            Chord* c = static_cast<Chord*>(el);
            note = c->selectedNote();
            if (note == 0)
                  note = c->upNote();
            el = note;
            }
      Duration d(_score->inputState().duration());
      if (!d.isValid() || d.isZero() || d.type() == Duration::V_MEASURE)
            _score->inputState().setDuration(Duration(Duration::V_QUARTER));

      _score->select(el, SELECT_SINGLE, 0);
      _score->setInputState(el);
      bool enable = el && (el->type() == NOTE || el->type() == REST);
      enableInputToolbar(enable);
      if (el == 0)
            mscore->showDrumTools(0, 0);

      _score->inputState().noteEntryMode = true;
      _score->moveCursor();
      setCursorOn(true);
      _score->inputState().rest = false;
      getAction("pad-rest")->setChecked(false);
      setMouseTracking(true);
      shadowNote->setVisible(true);
      dragElement = 0;
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   endNoteEntry
//---------------------------------------------------------

void ScoreView::endNoteEntry()
      {
      _score->inputState().setSegment(0);
      _score->inputState().noteEntryMode = false;
      if (_score->inputState().slur) {
            const QList<SpannerSegment*>& el = _score->inputState().slur->spannerSegments();
            if (!el.isEmpty())
                  el.front()->setSelected(false);
            static_cast<ChordRest*>(_score->inputState().slur->endElement())->addSlurBack(_score->inputState().slur);
            _score->inputState().slur = 0;
            }
      setMouseTracking(false);
      shadowNote->setVisible(false);
      setCursorOn(false);
      _score->setUpdateAll();
      _score->end();
      }

//---------------------------------------------------------
//   contextPopup
//---------------------------------------------------------

void ScoreView::contextPopup(QMouseEvent* ev)
      {
// printf("contextPopup\n");
      QPoint gp = ev->globalPos();
      startMove = toLogical(ev->pos());
      Element* e = elementNear(startMove);
      if (e) {
            if (!e->selected()) {
                  // bool control = (ev->modifiers() & Qt::ControlModifier) ? true : false;
                  // _score->select(e, control ? SELECT_ADD : SELECT_SINGLE, 0);
                  curElement = e;
                  select(ev);
                  }
            ElementType type = e->type();
            seq->stopNotes();       // stop now because we dont get a mouseRelease event
            if (type == MEASURE)
                  measurePopup(gp, static_cast<Measure*>(e));
            else
                  objectPopup(gp, e);
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

      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      emit viewRectChanged();
      }

//---------------------------------------------------------
//   dragNoteEntry
//    mouse move event in note entry mode
//---------------------------------------------------------

void ScoreView::dragNoteEntry(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      _score->addRefresh(shadowNote->canvasBoundingRect());
      setShadowNote(p);
      _score->addRefresh(shadowNote->canvasBoundingRect());
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
      mscore->endCmd();
      ChordRest* cr = _score->inputState().cr();
      if (cr)
            adjustCanvasPosition(cr, false);
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
      Qt::KeyboardModifiers keyState = ev->modifiers();
      if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
            cloneElement(curElement);
            return;
            }

      ElementType type = curElement->type();
      int dragStaff = 0;
      if (type == MEASURE) {
            System* dragSystem = (System*)(curElement->parent());
            dragStaff  = getStaff(dragSystem, startMove);
            }
      if ((ev->type() == QEvent::MouseButtonRelease) && ((!curElement->selected() || addSelect)))
            return;
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
            else if (keyState & Qt::ControlModifier) {
                  if (curElement->selected() && (ev->type() == QEvent::MouseButtonPress)) {
                        // do not deselect on ButtonPress, only on ButtonRelease
                        addSelect = false;
                        return;
                        }
                  addSelect = true;
                  st = SELECT_ADD;
                  }
            _score->select(curElement, st, dragStaff);
            if (curElement && curElement->type() == NOTE) {
                  Note* note = static_cast<Note*>(curElement);
                  int pitch = note->ppitch();
                  mscore->play(note, pitch);
                  }
            }
      else
            curElement = 0;
      _score->setUpdateAll(true);   //DEBUG
      mscore->endCmd();
      // Experimental:
      if (_score->selection().isSingle()) {
            // start edit mode
            Element* e = _score->selection().element();
            if (e->type() == SLUR_SEGMENT)
                  startEdit(e);
            }
      }

//---------------------------------------------------------
//   mousePress
//    return true if element is clicked
//---------------------------------------------------------

bool ScoreView::mousePress(QMouseEvent* ev)
      {
      startMoveI = ev->pos();
      startMove  = imatrix.map(QPointF(startMoveI));
      curElement = elementNear(startMove);

      if (curElement && curElement->type() == MEASURE) {
            System* dragSystem = (System*)(curElement->parent());
            int dragStaff  = getStaff(dragSystem, startMove);
            if (dragStaff < 0)
                  curElement = 0;
            }
      return curElement != 0;
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void ScoreView::mouseReleaseEvent(QMouseEvent* event)
      {
      seq->stopNotes();
      QWidget::mouseReleaseEvent(event);
      }

//---------------------------------------------------------
//   testElementDragTransition
//---------------------------------------------------------

bool ScoreView::testElementDragTransition(QMouseEvent* ev) const
      {
      if (curElement == 0 || !curElement->isMovable() || QApplication::mouseButtons() != Qt::LeftButton)
            return false;
      QPoint delta = ev->pos() - startMoveI;
      return delta.manhattanLength() > 2;
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editObject->canvasBoundingRect());
      editObject->endEditDrag();
      updateGrips();
      // setDropTarget(0); // this also resets dropRectangle and dropAnchor
      _score->addRefresh(editObject->canvasBoundingRect());
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
      score()->addRefresh(editObject->canvasBoundingRect());
      if (editObject->isText()) {
            Text* text = static_cast<Text*>(editObject);
            text->dragTo(p);
            }
      else {
            EditData ed;
            ed.view    = this;
            ed.curGrip = curGrip;
            ed.delta   = delta;
            ed.pos     = p;
            ed.hRaster = false;
            ed.vRaster = false;
            editObject->editDrag(ed);
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
      startMove = toLogical(ev->pos());
      Element* e = elementNear(startMove);
      if (e && (e == editObject) && (editObject->isText())) {
            if (editObject->mousePress(startMove, ev)) {
                  _score->addRefresh(editObject->canvasBoundingRect());
                  _score->end();
                  }
            return true;
            }
      int i;
      qreal a = grip[0].width() * 1.0;
      for (i = 0; i < grips; ++i) {
            if (grip[i].adjusted(-a, -a, a, a).contains(startMove)) {
                  curGrip = i;
                  updateGrips();
                  score()->end();
                  break;
                  }
            }
      return i != grips;
      }

//---------------------------------------------------------
//   onEditPasteTransition
//---------------------------------------------------------

void ScoreView::onEditPasteTransition(QMouseEvent* ev)
      {
      startMove = imatrix.map(QPointF(ev->pos()));
      Element* e = elementNear(startMove);
      if (e == editObject) {
            if (editObject->mousePress(startMove, ev)) {
                  _score->addRefresh(editObject->canvasBoundingRect());
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

      if (e == 0 || e->type() == MEASURE) {
            startMove   = p;
            dragElement = e;
            return true;
            }
      return false;
      }

//---------------------------------------------------------
//   editSelectTransition
//    Check for mouse click outside of editObject.
//---------------------------------------------------------

bool ScoreView::editSelectTransition(QMouseEvent* ev)
      {
      QPointF p = toLogical(ev->pos());
      Element* e = elementNear(p);

      if (e != editObject) {
            startMove   = p;
            dragElement = e;
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
      _score->addRefresh(lasso->canvasBoundingRect());
      QRectF r;
      r.setCoords(startMove.x(), startMove.y(), p.x(), p.y());
      lasso->setRect(r.normalized());
      QRectF _lassoRect(lasso->rect());
      r = _matrix.mapRect(_lassoRect);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);
      _score->addRefresh(lasso->canvasBoundingRect());
      _score->lassoSelect(lasso->rect());
      _score->end();
      }

//---------------------------------------------------------
//   endLasso
//---------------------------------------------------------

void ScoreView::endLasso()
      {
      _score->addRefresh(lasso->canvasBoundingRect());
      lasso->setRect(QRectF());
      _score->lassoSelectEnd();
      _score->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void ScoreView::deselectAll()
      {
      _score->deselectAll();
      _score->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   noteEntryMode
//---------------------------------------------------------

bool ScoreView::noteEntryMode() const
      {
      return sm->configuration().contains(states[NOTE_ENTRY]);
      }

//---------------------------------------------------------
//   editMode
//---------------------------------------------------------

bool ScoreView::editMode() const
      {
      return sm->configuration().contains(states[EDIT]);
      }

//---------------------------------------------------------
//   fotoMode
//---------------------------------------------------------

bool ScoreView::fotoMode() const
      {
      return sm->configuration().contains(states[FOTOMODE]);
      }

//---------------------------------------------------------
//   editInputTransition
//---------------------------------------------------------

void ScoreView::editInputTransition(QInputMethodEvent* ie)
      {
      if (editObject->edit(this, curGrip, 0, 0, ie->commitString())) {
            if (editObject->isText())
                  mscore->textTools()->updateTools();
            updateGrips();
            _score->end();
            mscore->endCmd();
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
                  _score->addRefresh(dropTarget->canvasBoundingRect());
                  dropTarget = 0;
                  }
            dropTarget = el;
            if (dropTarget) {
                  dropTarget->setDropTarget(true);
                  _score->addRefresh(dropTarget->canvasBoundingRect());
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
            _score->addRefresh(dropTarget->canvasBoundingRect());
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
            _score->addRefresh(dropTarget->canvasBoundingRect());
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
      return _matrix.m11();
      }

//---------------------------------------------------------
//   setOffset
//---------------------------------------------------------

void ScoreView::setOffset(qreal x, qreal y)
      {
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), x, y, _matrix.m33());
      imatrix = _matrix.inverted();
      emit viewRectChanged();
      emit offsetChanged(x, y);
      }

//---------------------------------------------------------
//   xoffset
//---------------------------------------------------------

qreal ScoreView::xoffset() const
      {
      return _matrix.dx();
      }

//---------------------------------------------------------
//   yoffset
//---------------------------------------------------------

qreal ScoreView::yoffset() const
      {
      return _matrix.dy();
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
      qreal lx   = 10.0 - page->pos().x() * mag();

      if (x < lx)
            x = lx;
      setOffset(x, 10.0);
      update();
      }

//---------------------------------------------------------
//   pagePrev
//---------------------------------------------------------

void ScoreView::pagePrev()
      {
      if (score()->pages().empty())
            return;
      Page* page = score()->pages().front();
      qreal x    = xoffset() + (page->width() + 25.0) * mag();
      if (x > 10.0)
            x = 10.0;
      setOffset(x, 10.0);
      update();
      }

//---------------------------------------------------------
//   pageTop
//---------------------------------------------------------

void ScoreView::pageTop()
      {
      setOffset(10.0, 10.0);
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
      QPointF p(lastPage->pos());
      setOffset(25.0 - p.x() * mag(), 25.0);
      update();
      }

//---------------------------------------------------------
//   adjustCanvasPosition
//---------------------------------------------------------

void ScoreView::adjustCanvasPosition(const Element* el, bool playBack)
      {
      const Measure* m;
      if (el->type() == NOTE)
            m = static_cast<const Note*>(el)->chord()->measure();
      else if (el->type() == REST)
            m = static_cast<const Rest*>(el)->measure();
      else if (el->type() == CHORD)
            m = static_cast<const Chord*>(el)->measure();
      else if (el->type() == SEGMENT)
            m = static_cast<const Segment*>(el)->measure();
      else if (el->type() == LYRICS)
            m = static_cast<const Lyrics*>(el)->measure();
      else if (el->type() == HARMONY)
            m = static_cast<const Harmony*>(el)->measure();
      else if (el->type() == MEASURE)
            m = static_cast<const Measure*>(el);
      else
            return;

      System* sys = m->system();

      QPointF p(el->canvasPos());
      QRectF r(imatrix.mapRect(geometry()));
      QRectF mRect(m->canvasBoundingRect());
      QRectF sysRect(sys->canvasBoundingRect());

      // only try to track measure if not during playback
      if (!playBack)
            sysRect = mRect;

      double _spatium    = score()->spatium();
      const qreal border = _spatium * 3;
      QRectF showRect = QRectF(mRect.x(), sysRect.y(), mRect.width(), sysRect.height())
                        .adjusted(-border, -border, border, border);

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

      qreal x   = - xoffset() / mag();
      qreal y   = - yoffset() / mag();

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left())
            x = showRect.left() - border;
      else if (showRect.left() > r.right())
            x = showRect.right() - width() / mag() + border;
      else if (r.width() >= showRect.width() && showRect.right() > r.right())
            x = showRect.left() - border;
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom())
            y = showRect.top() - border;
      else if (showRect.top() > r.bottom())
            y = showRect.bottom() - height() / mag() + border;
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom())
            y = showRect.top() - border;

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
      update();
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest()
      {
      cmdEnterRest(_score->inputState().duration());
      }

//---------------------------------------------------------
//   cmdEnterRest
//---------------------------------------------------------

void ScoreView::cmdEnterRest(const Duration& d)
      {
printf("cmdEnterRest %s\n", qPrintable(d.name()));
      if (!noteEntryMode())
            sm->postEvent(new CommandEvent("note-input"));
      _score->cmdEnterRest(d);
#if 0
      expandVoice();
      if (_is.cr() == 0) {
            printf("cannot enter rest here\n");
            return;
            }

      int track = _is.track;
      Segment* seg  = setNoteRest(_is.cr(), track, -1, d.fraction(), 0, AUTO);
      ChordRest* cr = static_cast<ChordRest*>(seg->element(track));
      if (cr)
            nextInputPos(cr, false);
      _is.rest = false;  // continue with normal note entry
#endif
      }

//---------------------------------------------------------
//   mscoreState
//---------------------------------------------------------

ScoreState ScoreView::mscoreState() const
      {
      if (sm->configuration().contains(states[NOTE_ENTRY]))
            return STATE_NOTE_ENTRY;
      else if (sm->configuration().contains(states[EDIT]))
            return STATE_EDIT;
      else if (sm->configuration().contains(states[FOTOMODE]))
            return STATE_FOTO;
      else if (sm->configuration().contains(states[PLAY]))
            return STATE_PLAY;
      else if (sm->configuration().contains(states[SEARCH]))
            return STATE_SEARCH;
      else
            return STATE_NORMAL;
      }

//---------------------------------------------------------
//   enterState
//    for debugging
//---------------------------------------------------------

void ScoreView::enterState()
      {
      if (debugMode)
            printf("%p enterState <%s>\n", this, qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   exitState
//    for debugging
//---------------------------------------------------------

void ScoreView::exitState()
      {
      if (debugMode)
            printf("%p exitState <%s>\n", this, qPrintable(sender()->objectName()));
      }

//---------------------------------------------------------
//   event
//---------------------------------------------------------

bool ScoreView::event(QEvent* event)
      {
      if (event->type() == QEvent::KeyPress && editObject) {
            QKeyEvent* ke = static_cast<QKeyEvent*>(event);
            if (ke->key() == Qt::Key_Tab || ke->key() == Qt::Key_Backtab) {
                  if(editObject->isText()) {
                        return true;
                        }
                  bool rv = true;
                  if (ke->key() == Qt::Key_Tab) {
                        curGrip += 1;
                        if (curGrip >= grips) {
                              curGrip = 0;
                              rv = false;
                              }
                        updateGrips();
                        _score->end();
                        if (curGrip)
                              return true;
                        }
                  else if (ke->key() == Qt::Key_Backtab) {
                        curGrip -= 1;
                        if (curGrip < 0) {
                              curGrip = grips -1;
                              rv = false;
                              }
                        }
                  updateGrips();
                  _score->end();
                  if (rv)
                        return true;
                  }
            }
      else if (event->type() == CloneDrag) {
            Element* e = static_cast<CloneEvent*>(event)->element();
            cloneElement(e);
            }
      else if (event->type() == QEvent::Gesture) {
            return gestureEvent(static_cast<QGestureEvent*>(event));
            }
      return QWidget::event(event);
      }

//---------------------------------------------------------
//   startUndoRedo
//---------------------------------------------------------

void ScoreView::startUndoRedo()
      {
      // exit edit mode
      _score->setLayoutAll(false);
      if (sm->configuration().contains(states[EDIT]))
            sm->postEvent(new CommandEvent("escape"));
      }

//---------------------------------------------------------
//   endUndoRedo
///   Common handling for ending undo or redo
//---------------------------------------------------------

void ScoreView::endUndoRedo()
      {
      if (_score->inputState().segment())
            mscore->setPos(_score->inputState().tick());
      if (_score->noteEntryMode() && !noteEntryMode()) {
            // enter note entry mode
            postCmd("note-input");
            }
      else if (!_score->inputState().noteEntryMode && noteEntryMode()) {
            // leave note entry mode
            postCmd("escape");
            }
      _score->updateSelection();
      mscore->updateInputState(_score);
      if (_score->layoutAll())
            _score->doLayout();           // TODO: does not really work
      _score->end();
      mscore->endCmd();
      }

//---------------------------------------------------------
//   cmdAddSlur
//    'S' typed on keyboard
//---------------------------------------------------------

void ScoreView::cmdAddSlur()
      {
      InputState& is = _score->inputState();
      if (noteEntryMode() && is.slur) {
            const QList<SpannerSegment*>& el = is.slur->spannerSegments();
            if (!el.isEmpty())
                  el.front()->setSelected(false);
            static_cast<ChordRest*>(is.slur->endElement())->addSlurBack(is.slur);
            is.slur = 0;
            return;
            }
      _score->startCmd();
      QList<Note*> nl = _score->selection().noteList();
      Note* firstNote = 0;
      Note* lastNote  = 0;
      foreach(Note* n, nl) {
            if (firstNote == 0 || firstNote->chord()->tick() > n->chord()->tick())
                  firstNote = n;
            if (lastNote == 0 || lastNote->chord()->tick() < n->chord()->tick())
                  lastNote = n;
            }
      if (!firstNote)
            return;
      if (firstNote == lastNote)
            lastNote = 0;
      cmdAddSlur(firstNote, lastNote);
      }

//---------------------------------------------------------
//   addSlur
//---------------------------------------------------------

void ScoreView::cmdAddSlur(Note* firstNote, Note* lastNote)
      {
      ChordRest* cr1 = firstNote->chord();
      ChordRest* cr2 = lastNote ? lastNote->chord() : nextChordRest(cr1);

      if (cr2 == 0)
            cr2 = cr1;

      Slur* slur = new Slur(_score);
      slur->setStartElement(cr1);
      slur->setEndElement(cr2);
      slur->setParent(0);
      _score->undoAddElement(slur);

      _score->endCmd();
      _score->startCmd();

      slur->layout();
      if (cr1 == cr2) {
            SlurSegment* ss = slur->frontSegment();
            ss->setSlurOffset(3, QPointF(3.0, 0.0));
            }
      const QList<SpannerSegment*>& el = slur->spannerSegments();
      if (noteEntryMode()) {
            _score->inputState().slur = slur;
            if (!el.isEmpty())
                  el.front()->setSelected(true);
            else
                  printf("addSlur: no segment\n");
            // set again when leaving slur mode:
            static_cast<ChordRest*>(slur->endElement())->removeSlurBack(slur);
            _score->endCmd();
            }
      else {
            //
            // start slur in edit mode if lastNote is not given
            //
            if (origEditObject && origEditObject->isText()) {
                  _score->endCmd();
                  return;
                  }
            if ((lastNote == 0) && !el.isEmpty()) {
                  origEditObject = el.front();
                  sm->postEvent(new CommandEvent("edit"));  // calls startCmd()
                  }
            else
                  _score->endCmd();
            }
      }

//---------------------------------------------------------
//   cmdChangeEnharmonic
//---------------------------------------------------------

void ScoreView::cmdChangeEnharmonic(bool up)
      {
      _score->startCmd();
      QList<Note*> nl = _score->selection().noteList();
      foreach(Note* n, nl) {
            Staff* staff = n->staff();
            if (staff->part()->instr()->useDrumset())
                  continue;
            if (staff->useTablature()) {
                  int string = n->line() + (up ? 1 : -1);
                  int fret = staff->part()->instr()->tablature()->fret(n->pitch(), string);
                  if (fret != -1) {
                        _score->undoChangePitch(n, n->pitch(), n->tpc(), n->line(), fret, string);
                        }
                  }
            else {
                  static const int tab[36] = {
                        26, 14,  2,  // 60  B#   C   Dbb
                        21, 21,  9,  // 61  C#   C#  Db
                        28, 16,  4,  // 62  C##  D   Ebb
                        23, 23, 11,  // 63  D#   D#  Eb
                        30, 18,  6,  // 64  D##  E   Fb
                        25, 13,  1,  // 65  E#   F   Gbb
                        20, 20,  8,  // 66  F#   F#  Gb
                        27, 15,  3,  // 67  F##  G   Abb
                        22, 22, 10,  // 68  G#   G#  Ab
                        29, 17,  5,  // 69  G##  A   Bbb
                        24, 24, 12,  // 70  A#   A#  Bb
                        31, 19,  7,  // 71  A##  B   Cb
                        };
                  int tpc  = n->tpc();
                  int line = n->line();
                  int i;
                  for (i = 0; i < 36; ++i) {
                        if (tab[i] == tpc) {
                              int k = i % 3;
                              i-= k;
                              if ((k < 2) && tab[i] == tab[i+1])
                                    ++k;
                              if (k < 2) {
                                    ++k;
                                    tpc = tab[i + k];
                                    ++line;
                                    }
                              else {
                                    if (tab[i + 2] != tab[i + 1])
                                          --line;
                                    --k;
                                    if (tab[i + 1] != tab[i])
                                          --line;
                                    --k;
                                    tpc = tab[i];
                                    }
                              break;
                              }
                        }
                  if (i == 36)
                        printf("tpc %d not found\n", tpc);
                  else if (tpc != n->tpc()) {
                        _score->undoChangePitch(n, n->pitch(), tpc, line, n->fret(), n->string());
                        }
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   cloneElement
//---------------------------------------------------------

void ScoreView::cloneElement(Element* e)
      {
      if (!e->isMovable() && e->type() != SPACER && e->type() != VBOX)
            return;
      QDrag* drag = new QDrag(this);
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeSymbolFormat, e->mimeData(QPointF()));
      drag->setMimeData(mimeData);
      drag->setPixmap(QPixmap());
      drag->start(Qt::CopyAction);
      }

//---------------------------------------------------------
//   changeEditElement
//---------------------------------------------------------

void ScoreView::changeEditElement(Element* e)
      {
      int grip = curGrip;
      endEdit();
      startEdit(e, grip);
      }

//---------------------------------------------------------
//   setCursorVisible
//---------------------------------------------------------

void ScoreView::setCursorVisible(bool v)
      {
      _cursor->setVisible(v);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n, ChordRest* cr)
      {
      Fraction f(cr->duration());
      int tick    = cr->tick();
      Tuplet* ot  = cr->tuplet();

      f.reduce();       //measure duration might not be reduced
      Fraction ratio(n, f.numerator());
      Fraction fr(1, f.denominator());
      while (ratio.numerator() >= ratio.denominator()*2) {
            ratio /= 2;
            fr    /= 2;
            }

      Tuplet* tuplet = new Tuplet(_score);
      tuplet->setRatio(ratio);

      //
      // "fr" is the fraction value of one tuple element
      //
      // "tuplet time" is "normal time" / tuplet->ratio()
      //    Example: an 1/8 has 240 midi ticks, in an 1/8 triplet the note
      //             has a tick duration of 240 / (3/2) = 160 ticks
      //

      tuplet->setDuration(f);
      Duration baseLen(fr);
      tuplet->setBaseLen(baseLen);

      tuplet->setTrack(cr->track());
      tuplet->setTick(tick);
      Measure* measure = cr->measure();
      tuplet->setParent(measure);

      if (ot)
            tuplet->setTuplet(ot);
      _score->cmdCreateTuplet(cr, tuplet);

      const QList<DurationElement*>& cl = tuplet->elements();

      int ne = cl.size();
      DurationElement* el = 0;
      if (ne && cl[0]->type() == REST)
            el  = cl[0];
      else if (ne > 1)
            el = cl[1];
      if (el) {
            _score->select(el, SELECT_SINGLE, 0);
            if (!noteEntryMode()) {
                  sm->postEvent(new CommandEvent("note-input"));
                  qApp->processEvents();
                  }
            _score->inputState().setDuration(baseLen);
            mscore->updateInputState(_score);
            }
      }

//---------------------------------------------------------
//   changeVoice
//---------------------------------------------------------

void ScoreView::changeVoice(int voice)
      {
      InputState* is = &score()->inputState();
      int track = (is->track() / VOICES) * VOICES + voice;
      is->setTrack(track);
      //
      // in note entry mode search for a valid input
      // position
      //
      if (!is->noteEntryMode || is->cr()) {
            score()->startCmd();
            QList<Element*> el;
            foreach(Element* e, score()->selection().elements()) {
                  if (e->type() == NOTE) {
                        Note* note = static_cast<Note*>(e);
                        Chord* chord = note->chord();
                        if (chord->voice() != voice) {
                              int notes = note->chord()->notes().size();
                              if (notes > 1) {
                                    //
                                    // TODO: check destination voice content
                                    //
                                    Note* newNote   = new Note(*note);
                                    Chord* newChord = new Chord(score());
                                    newNote->setSelected(false);
                                    el.append(newNote);
                                    int track = chord->staffIdx() * VOICES + voice;
                                    newChord->setTrack(track);
                                    newChord->setDurationType(chord->durationType());
                                    newChord->setDuration(chord->duration());
                                    newChord->setParent(chord->parent());
                                    newChord->add(newNote);
                                    score()->undoRemoveElement(note);
                                    score()->undoAddElement(newChord);
                                    }
                              else if (notes > 1 || (voice && chord->voice())) {
                                    Chord* newChord = new Chord(*chord);
                                    int track = chord->staffIdx() * VOICES + voice;
                                    newChord->setTrack(track);
                                    newChord->setParent(chord->parent());
                                    score()->undoRemoveElement(chord);
                                    score()->undoAddElement(newChord);
                                    }
                              }
                        }
                  }
            score()->selection().clear();
            foreach(Element* e, el)
                  score()->select(e, SELECT_ADD, -1);
            score()->setLayoutAll(true);
            score()->endCmd();
            return;
            }

      is->setSegment(is->segment()->measure()->firstCRSegment());
      score()->setUpdateAll(true);
      score()->end();
      mscore->setPos(is->segment()->tick());
      }

//---------------------------------------------------------
//   harmonyEndEdit
//---------------------------------------------------------

void ScoreView::harmonyEndEdit()
      {
      Harmony* harmony = static_cast<Harmony*>(editObject);
      Harmony* origH   = static_cast<Harmony*>(origEditObject);

      if (harmony->isEmpty() && origH->isEmpty()) {
            Measure* measure = (Measure*)(harmony->parent());
            measure->remove(harmony);
            }
      }

//---------------------------------------------------------
//   lyricsUpDown
//---------------------------------------------------------

void ScoreView::lyricsUpDown(bool up, bool end)
      {
      Lyrics* lyrics   = static_cast<Lyrics*>(editObject);
      int track        = lyrics->track();
      ChordRest* cr    = lyrics->chordRest();
      int verse        = lyrics->no();
      const QList<Lyrics*>* ll = &lyrics->chordRest()->lyricsList();

      if (up) {
            if (verse == 0)
                  return;
            --verse;
            }
      else {
            ++verse;
            if (verse >= ll->size())
                  return;
            }
      endEdit();
      _score->startCmd();
      lyrics = ll->value(verse);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(cr);
            lyrics->setNo(verse);
            _score->undoAddElement(lyrics);
            }

      _score->select(lyrics, SELECT_SINGLE, 0);
      startEdit(lyrics, -1);
      adjustCanvasPosition(lyrics, false);
      if (end)
            ((Lyrics*)editObject)->moveCursorToEnd();
      else
            ((Lyrics*)editObject)->moveCursor(0);

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsTab
//---------------------------------------------------------

void ScoreView::lyricsTab(bool back, bool end, bool moveOnly)
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();

      Segment* nextSegment = segment;
      if (back) {
            // search prev chord
            while ((nextSegment = nextSegment->prev1(SegChordRest | SegGrace))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == CHORD)
                        break;
                  }
            }
      else {
            // search next chord
            while ((nextSegment = nextSegment->next1(SegChordRest | SegGrace))) {
                  Element* el = nextSegment->element(track);
                  if (el &&  el->type() == CHORD)
                        break;
                  }
            }
      if (nextSegment == 0)
            return;

      endEdit();

      // search previous lyric
      Lyrics* oldLyrics = 0;
      if (!back) {
            while (segment) {
                  const QList<Lyrics*>* nll = segment->lyricsList(staffIdx);
                  if (nll) {
                        oldLyrics = nll->value(verse);
                        if (oldLyrics)
                              break;
                        }
                  segment = segment->prev1(SegChordRest | SegGrace);
                  }
            }

      const QList<Lyrics*>* ll = nextSegment->lyricsList(staffIdx);
      if (ll == 0) {
            printf("no next lyrics list: %s\n", nextSegment->element(track)->name());
            return;
            }
      lyrics = ll->value(verse);

      bool newLyrics = false;
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            ChordRest* cr = static_cast<ChordRest*>(nextSegment->element(track));
            lyrics->setParent(cr);
            lyrics->setNo(verse);
            lyrics->setSyllabic(Lyrics::SINGLE);
            newLyrics = true;
            }

      _score->startCmd();

      if (oldLyrics && !moveOnly) {
            switch(lyrics->syllabic()) {
                  case Lyrics::SINGLE:
                  case Lyrics::BEGIN:
                        break;
                  case Lyrics::END:
                        lyrics->setSyllabic(Lyrics::SINGLE);
                        break;
                  case Lyrics::MIDDLE:
                        lyrics->setSyllabic(Lyrics::BEGIN);
                        break;
                  }
            switch(oldLyrics->syllabic()) {
                  case Lyrics::SINGLE:
                  case Lyrics::END:
                        break;
                  case Lyrics::BEGIN:
                        oldLyrics->setSyllabic(Lyrics::SINGLE);
                        break;
                  case Lyrics::MIDDLE:
                        oldLyrics->setSyllabic(Lyrics::END);
                        break;
                  }
            }

      if (newLyrics)
          _score->undoAddElement(lyrics);

      _score->select(lyrics, SELECT_SINGLE, 0);
      startEdit(lyrics, -1);
      adjustCanvasPosition(lyrics, false);
      if (end)
            ((Lyrics*)editObject)->moveCursorToEnd();
      else
            ((Lyrics*)editObject)->moveCursor(0);

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsMinus
//---------------------------------------------------------

void ScoreView::lyricsMinus()
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();

      endEdit();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(SegChordRest | SegGrace))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }
      if (nextSegment == 0) {
            return;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            const QList<Lyrics*>* nll = segment->lyricsList(staffIdx);
            if (!nll) {
                  segment = segment->prev1(SegChordRest | SegGrace);
                  continue;
                  }
            oldLyrics = nll->value(verse);
            if (oldLyrics)
                  break;
            segment = segment->prev1(SegChordRest | SegGrace);
            }

      _score->startCmd();

      const QList<Lyrics*>* ll = nextSegment->lyricsList(staffIdx);
      lyrics         = ll->value(verse);
      bool newLyrics = (lyrics == 0);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment->element(track));
            lyrics->setNo(verse);
            lyrics->setSyllabic(Lyrics::END);
            }

      if(lyrics->syllabic()==Lyrics::BEGIN) {
            lyrics->setSyllabic(Lyrics::MIDDLE);
            }
      else if(lyrics->syllabic()==Lyrics::SINGLE) {
            lyrics->setSyllabic(Lyrics::END);
            }

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::BEGIN:
                  case Lyrics::MIDDLE:
                        break;
                  case Lyrics::SINGLE:
                        oldLyrics->setSyllabic(Lyrics::BEGIN);
                        break;
                  case Lyrics::END:
                        oldLyrics->setSyllabic(Lyrics::MIDDLE);
                        break;
                  }
            }

      if(newLyrics)
          _score->undoAddElement(lyrics);

      _score->select(lyrics, SELECT_SINGLE, 0);
      startEdit(lyrics, -1);
      adjustCanvasPosition(lyrics, false);
      ((Lyrics*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsUnderscore
//---------------------------------------------------------

void ScoreView::lyricsUnderscore()
      {
      Lyrics* lyrics   = static_cast<Lyrics*>(editObject);
      int track        = lyrics->track();
      int staffIdx     = lyrics->staffIdx();
      Segment* segment = lyrics->segment();
      int verse        = lyrics->no();
      int endTick      = segment->tick();

      endEdit();

      // search next chord
      Segment* nextSegment = segment;
      while ((nextSegment = nextSegment->next1(SegChordRest | SegGrace))) {
            Element* el = nextSegment->element(track);
            if (el &&  el->type() == CHORD)
                  break;
            }

      // search previous lyric
      Lyrics* oldLyrics = 0;
      while (segment) {
            const QList<Lyrics*>* nll = segment->lyricsList(staffIdx);
            if (nll) {
                  oldLyrics = nll->value(verse);
                  if (oldLyrics)
                        break;
                  }
            segment = segment->prev1(SegChordRest | SegGrace);
            }

      if (nextSegment == 0) {
            if (oldLyrics) {
                  switch(oldLyrics->syllabic()) {
                        case Lyrics::SINGLE:
                        case Lyrics::END:
                              break;
                        default:
                              oldLyrics->setSyllabic(Lyrics::END);
                              break;
                        }
                  if (oldLyrics->segment()->tick() < endTick)
                        oldLyrics->setTicks(endTick - oldLyrics->segment()->tick());
                  }
            return;
            }
      _score->startCmd();

      const QList<Lyrics*>* ll = nextSegment->lyricsList(staffIdx);
      lyrics         = ll->value(verse);
      bool newLyrics = (lyrics == 0);
      if (!lyrics) {
            lyrics = new Lyrics(_score);
            lyrics->setTrack(track);
            lyrics->setParent(nextSegment->element(track));
            lyrics->setNo(verse);
            }

      lyrics->setSyllabic(Lyrics::SINGLE);

      if (oldLyrics) {
            switch(oldLyrics->syllabic()) {
                  case Lyrics::SINGLE:
                  case Lyrics::END:
                        break;
                  default:
                        oldLyrics->setSyllabic(Lyrics::END);
                        break;
                  }
            if (oldLyrics->segment()->tick() < endTick)
                  oldLyrics->setTicks(endTick - oldLyrics->segment()->tick());
            }
      if (newLyrics)
            _score->undoAddElement(lyrics);

      _score->select(lyrics, SELECT_SINGLE, 0);
      startEdit(lyrics, -1);
      adjustCanvasPosition(lyrics, false);
      ((Lyrics*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsReturn
//---------------------------------------------------------

void ScoreView::lyricsReturn()
      {
      Lyrics* lyrics   = (Lyrics*)editObject;
      Segment* segment = lyrics->segment();

      endEdit();

      _score->startCmd();

      Lyrics* oldLyrics = lyrics;

      lyrics = new Lyrics(_score);
      lyrics->setTrack(oldLyrics->track());
      lyrics->setParent(segment->element(oldLyrics->track()));
      lyrics->setNo(oldLyrics->no() + 1);
      _score->undoAddElement(lyrics);
      _score->select(lyrics, SELECT_SINGLE, 0);
      startEdit(lyrics, -1);
      adjustCanvasPosition(lyrics, false);
      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   lyricsEndEdit
//---------------------------------------------------------

void ScoreView::lyricsEndEdit()
      {
      Lyrics* lyrics = (Lyrics*)editObject;
      Lyrics* origL  = (Lyrics*)origEditObject;
      int endTick    = lyrics->segment()->tick();

      // search previous lyric:
      int verse    = lyrics->no();
      int staffIdx = lyrics->staffIdx();

      // search previous lyric
      Lyrics* oldLyrics = 0;
      Segment* segment  = lyrics->segment();
      while (segment) {
            const QList<Lyrics*>* nll = segment->lyricsList(staffIdx);
            if (nll) {
                  oldLyrics = nll->value(verse);
                  if (oldLyrics)
                        break;
                  }
            segment = segment->prev1(SegChordRest | SegGrace);
            }

      if (lyrics->isEmpty() && origL->isEmpty())
            lyrics->parent()->remove(lyrics);
      else {
            if (oldLyrics && oldLyrics->syllabic() == Lyrics::END) {
                  if (oldLyrics->endTick() >= endTick)
                        oldLyrics->setTicks(0);
                  }
            }
      }

//---------------------------------------------------------
//   modifyElement
//---------------------------------------------------------

void ScoreView::modifyElement(Element* el)
      {
      if (el == 0) {
            printf("modifyElement: el==0\n");
            return;
            }
      Score* cs = el->score();
      if (!cs->selection().isSingle()) {
            printf("modifyElement: cs->selection().state() != SEL_SINGLE\n");
            delete el;
            return;
            }
      Element* e = cs->selection().element();
      Chord* chord;
      if (e->type() == CHORD)
            chord = static_cast<Chord*>(e);
      else if (e->type() == NOTE)
            chord = static_cast<Note*>(e)->chord();
      else {
            printf("modifyElement: no note/Chord selected:\n  ");
            e->dump();
            delete el;
            return;
            }
      switch (el->type()) {
            case ARTICULATION:
                  chord->add(static_cast<Articulation*>(el));
                  break;
            default:
                  printf("modifyElement: %s not ARTICULATION\n", el->name());
                  delete el;
                  return;
            }
      cs->setLayoutAll(true);
      }

//---------------------------------------------------------
//   chordTab
//---------------------------------------------------------

void ScoreView::chordTab(bool back)
      {
      Harmony* cn      = (Harmony*)editObject;
      Segment* segment = cn->segment();
      int track        = cn->track();
      if (segment == 0) {
            printf("chordTab: no segment\n");
            return;
            }

      // search next chord
      if (back)
            segment = segment->prev1(SegChordRest);
      else
            segment = segment->next1(SegChordRest);
      if (segment == 0) {
            printf("no next segment\n");
            return;
            }
      endEdit();
      _score->startCmd();

      // search for next chord name
      cn = 0;
      foreach(Element* e, segment->annotations()) {
            if (e->type() == HARMONY && e->track() == track) {
                  Harmony* h = static_cast<Harmony*>(e);
                  cn = h;
                  break;
                  }
            }

      if (!cn) {
            cn = new Harmony(_score);
            cn->setTrack(track);
            cn->setParent(segment);
            _score->undoAddElement(cn);
            }

      _score->select(cn, SELECT_SINGLE, 0);
      startEdit(cn, -1);
      adjustCanvasPosition(cn, false);
      ((Harmony*)editObject)->moveCursorToEnd();

      _score->setLayoutAll(true);
      }

//---------------------------------------------------------
//   cmdTuplet
//---------------------------------------------------------

void ScoreView::cmdTuplet(int n)
      {
      _score->startCmd();
      if (noteEntryMode()) {
            _score->expandVoice();
            _score->changeCRlen(_score->inputState().cr(), _score->inputState().duration());
            if (_score->inputState().cr())
                  cmdTuplet(n, _score->inputState().cr());
            }
      else {
            QSet<ChordRest*> set;
            foreach(Element* e, _score->selection().elements()) {
                  if (e->type() == NOTE) {
                        Note* note = static_cast<Note*>(e);
                        if(note->noteType() != NOTE_NORMAL) { //no tuplet on grace notes
                              _score->endCmd();
                              return;
                              }
                        e = note->chord();
                        }
                  if (e->isChordRest()) {
                        ChordRest* cr = static_cast<ChordRest*>(e);
                        if(!set.contains(cr)) {
                              cmdTuplet(n, cr);
                              set.insert(cr);
                              }
                        }
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   midiNoteReceived
//---------------------------------------------------------

void ScoreView::midiNoteReceived(int pitch, bool chord)
      {
      MidiInputEvent ev;
      ev.pitch = pitch;
      ev.chord = chord;

printf("midiNoteReceived %d chord %d\n", pitch, chord);
      score()->enqueueMidiEvent(ev);
      if (!score()->undo()->active())
            cmd(0);
      }

//---------------------------------------------------------
//   cmdInsertNote
//---------------------------------------------------------

void ScoreView::cmdInsertNote(int note)
      {
      printf("not implemented: cmdInsertNote %d\n", note);
      }

//---------------------------------------------------------
//   cmdAddPitch
//    c d e f g a b entered:
//       insert note or add note to chord
//---------------------------------------------------------

void ScoreView::cmdAddPitch(int note, bool addFlag)
      {
      InputState& is = _score->inputState();
      if (is.track() == -1)          // invalid state
            return;
      Drumset* ds = is.drumset();
      int pitch;
      if (ds) {
            char note1 = "CDEFGAB"[note];
            pitch = -1;
            for (int i = 0; i < 127; ++i) {
                  if (!ds->isValid(i))
                        continue;
                  if (ds->shortcut(i) && (ds->shortcut(i) == note1)) {
                        pitch = i;
                        break;
                        }
                  }
            if (pitch == -1) {
                  printf("  shortcut %c not defined in drumset\n", note1);
                  return;
                  }
            is.setDrumNote(pitch);
            }
      else {
            KeySigEvent key = _score->staff(is.track() / VOICES)->keymap()->key(is.tick());
            int octave = is.pitch / 12;
            pitch      = pitchKeyAdjust(note, key.accidentalType());
            int delta  = is.pitch - (octave*12 + pitch);
            if (delta > 6)
                  is.pitch = (octave+1)*12 + pitch;
            else if (delta < -6)
                  is.pitch = (octave-1)*12 + pitch;
            else
                  is.pitch = octave*12 + pitch;
            if (is.pitch < 0)
                  is.pitch = 0;
            if (is.pitch > 127)
                  is.pitch = 127;
            pitch = is.pitch;
            }
      cmdAddPitch1(pitch, addFlag);
      }

//---------------------------------------------------------
//   cmdAddChordName
//---------------------------------------------------------

void ScoreView::cmdAddChordName()
      {
      if (!_score->checkHasMeasures())
            return;
      ChordRest* cr = _score->getSelectedChordRest();
      if (!cr)
            return;
      _score->startCmd();
      Harmony* s = new Harmony(_score);
      s->setTrack(cr->track());
      s->setParent(cr->segment());
      _score->undoAddElement(s);

      _score->setLayoutAll(true);

      _score->select(s, SELECT_SINGLE, 0);
      // adjustCanvasPosition(s, false);
      startEdit(s);
      }

//---------------------------------------------------------
//   cmdAddText
//---------------------------------------------------------

void ScoreView::cmdAddText(int subtype)
      {
      if (!_score->checkHasMeasures())
            return;
      Page* page = _score->pages().front();
      const QList<System*>* sl = page->systems();
      const QList<MeasureBase*>& ml = sl->front()->measures();
      Text* s = 0;
      _score->startCmd();
      switch(subtype) {
            case TEXT_TITLE:
            case TEXT_SUBTITLE:
            case TEXT_COMPOSER:
            case TEXT_POET:
                  {
                  MeasureBase* measure = ml.front();
                  if (measure->type() != VBOX) {
                        MeasureBase* mb = new VBox(_score);
                        mb->setTick(0);
                        _score->undoInsertMeasure(mb, measure);
                        measure = mb;
                        }
                  s = new Text(_score);
                  switch(subtype) {
                        case TEXT_TITLE:    s->setTextStyle(TEXT_STYLE_TITLE);    break;
                        case TEXT_SUBTITLE: s->setTextStyle(TEXT_STYLE_SUBTITLE); break;
                        case TEXT_COMPOSER: s->setTextStyle(TEXT_STYLE_COMPOSER); break;
                        case TEXT_POET:     s->setTextStyle(TEXT_STYLE_POET);     break;
                        }
                  s->setSubtype(subtype);
                  s->setParent(measure);
                  }
                  break;

            case TEXT_REHEARSAL_MARK:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new Text(_score);
                  s->setTrack(0);
                  s->setSubtype(subtype);
                  s->setTextStyle(TEXT_STYLE_REHEARSAL_MARK);
                  s->setParent(cr->segment());
                  }
                  break;
            case TEXT_STAFF:
            case TEXT_SYSTEM:
                  {
                  ChordRest* cr = _score->getSelectedChordRest();
                  if (!cr)
                        break;
                  s = new StaffText(_score);
                  if (subtype == TEXT_SYSTEM) {
                        s->setTrack(0);
                        s->setSystemFlag(true);
                        s->setTextStyle(TEXT_STYLE_SYSTEM);
                        s->setSubtype(TEXT_SYSTEM);
                        }
                  else {
                        s->setTrack(cr->track());
                        s->setSystemFlag(false);
                        s->setTextStyle(TEXT_STYLE_STAFF);
                        s->setSubtype(TEXT_STAFF);
                        }
                  s->setSubtype(subtype);
                  s->setParent(cr->segment());
                  }
                  break;
            }

      if (s) {
            _score->undoAddElement(s);
            _score->setLayoutAll(true);
            _score->select(s, SELECT_SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            }
      else
            _score->endCmd();
      }

//---------------------------------------------------------
//   cmdAppendMeasures
///   Append \a n measures.
///
///   Keyboard callback, called from pulldown menu.
//
//    - called from pulldown menu
//---------------------------------------------------------

void ScoreView::cmdAppendMeasures(int n, ElementType type)
      {
      _score->startCmd();
      appendMeasures(n, type);
      _score->endCmd();
      }

//---------------------------------------------------------
//   appendMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::appendMeasure(ElementType type)
      {
      _score->startCmd();
      MeasureBase* mb = _score->appendMeasure(type);
      _score->endCmd();
      return mb;
      }

//---------------------------------------------------------
//   appendMeasures
//---------------------------------------------------------

void ScoreView::appendMeasures(int n, ElementType type)
      {
      if (_score->noStaves()) {
            QMessageBox::warning(0, "MuseScore",
               tr("No staves found:\n"
                  "please use the instruments dialog to\n"
                  "first create some staves"));
            return;
            }
      bool createEndBar = false;
      bool endBarGenerated = false;
      if (type == MEASURE) {
            Measure* lm = _score->lastMeasure();
            if (lm && lm->endBarLineType() == END_BAR) {
                  if (!lm->endBarLineGenerated()) {
                        _score->undoChangeEndBarLineType(lm, NORMAL_BAR);
                        createEndBar = true;
                        // move end Bar to last Measure;
                        }
                  else {
                        createEndBar    = true;
                        endBarGenerated = true;
                        lm->setEndBarLineType(NORMAL_BAR, endBarGenerated);
                        }
                  }
            else if (lm == 0)
                  createEndBar = true;
            }
      for (int i = 0; i < n; ++i)
            _score->appendMeasure(type);
      if (createEndBar) {
            Measure* lm = _score->lastMeasure();
            if (lm)
                  lm->setEndBarLineType(END_BAR, endBarGenerated);
            }
      }

//---------------------------------------------------------
//   checkSelectionStateForInsertMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::checkSelectionStateForInsertMeasure()
      {
	if (_score->selection().state() == SEL_RANGE) {
	      MeasureBase* mb = _score->selection().startSegment()->measure();
            return mb;
            }
      Element* e = _score->selection().element();
      if (e) {
            if (e->type() == VBOX)
                  return static_cast<MeasureBase*>(e);
            }
	QMessageBox::warning(0, "MuseScore",
         tr("No Measure selected:\n" "please select a measure and try again"));
      return 0;
      }

//---------------------------------------------------------
//   cmdInsertMeasures
//---------------------------------------------------------

void ScoreView::cmdInsertMeasures(int n, ElementType type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
	for (int i = 0; i < n; ++i)
            mb = insertMeasure(type, mb);
      _score->select(0, SELECT_SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   cmdInsertMeasure
//---------------------------------------------------------

void ScoreView::cmdInsertMeasure(ElementType type)
      {
      MeasureBase* mb = checkSelectionStateForInsertMeasure();
      if (!mb)
            return;
      _score->startCmd();
      mb = insertMeasure(type, mb);
      if (mb->type() == TBOX) {
            TBox* tbox = static_cast<TBox*>(mb);
            Text* s = tbox->getText();
            _score->select(s, SELECT_SINGLE, 0);
            _score->endCmd();
            startEdit(s);
            return;
            }
      _score->select(0, SELECT_SINGLE, 0);
      _score->endCmd();
      }

//---------------------------------------------------------
//   insertMeasure
//---------------------------------------------------------

MeasureBase* ScoreView::insertMeasure(ElementType type, MeasureBase* measure)
      {
      int tick = measure->tick();
      MeasureBase* mb = static_cast<MeasureBase*>(Element::create(type, _score));
      mb->setTick(tick);

      if (type == MEASURE) {
            Measure* m = static_cast<Measure*>(mb);
            // Fraction f(m->timesig());  TODO
            Fraction f(4,4);
	      int ticks = f.ticks();

            m->setTimesig(f);
            m->setLen(f);
	      for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
    	            Rest* rest = new Rest(_score, Duration(Duration::V_MEASURE));
                  rest->setDuration(m->len());
              	rest->setTrack(staffIdx * VOICES);
                    Segment* s = m->getSegment(SegChordRest, tick);
              	s->add(rest);
	            }
            QList<TimeSig*> tsl;
            QList<KeySig*>  ksl;

            if (tick == 0) {
                  //
                  // remove time and key signatures
                  //
                  for (int staffIdx = 0; staffIdx < _score->nstaves(); ++staffIdx) {
                        for (Segment* s = _score->firstSegment(); s && s->tick() == 0; s = s->next()) {
                              if (s->subtype() == SegKeySig) {
                                    KeySig* e = static_cast<KeySig*>(s->element(staffIdx * VOICES));
                                    if (e) {
                                          if (!e->generated())
                                                ksl.append(static_cast<KeySig*>(e));
                                          _score->undoRemoveElement(e);
                                          if (e->segment()->isEmpty())
                                                _score->undoRemoveElement(e->segment());
                                          }
                                    }
                              else if (s->subtype() == SegTimeSig) {
                                    TimeSig* e = static_cast<TimeSig*>(s->element(staffIdx * VOICES));
                                    if (e) {
                                          if (!e->generated())
                                                tsl.append(static_cast<TimeSig*>(e));
                                          _score->undoRemoveElement(e);
                                          if (e->segment()->isEmpty())
                                                _score->undoRemoveElement(e->segment());
                                          }
                                    }
                              else if (s->subtype() == SegClef) {
                                    Element* e = s->element(staffIdx * VOICES);
                                    if (e) {
                                          _score->undoRemoveElement(e);
                                          if (static_cast<Segment*>(e->parent())->isEmpty())
                                                _score->undoRemoveElement(e->parent());
                                          }
                                    }
                              }
                        }
                  }
            _score->undoInsertMeasure(m, measure);
            _score->undoInsertTime(tick, ticks);

            //
            // if measure is inserted at tick zero,
            // create key and time signature
            //
            foreach(TimeSig* ts, tsl) {
                  TimeSig* nts = new TimeSig(*ts);
                  SegmentType st = SegTimeSig;
                  Segment* s = m->findSegment(st, 0);
                  if (s == 0) {
                        s = new Segment(m, st, 0);
                        _score->undoAddElement(s);
                        }
                  nts->setParent(s);
                  _score->undoAddElement(nts);
                  }
            foreach(KeySig* ks, ksl) {
                  KeySig* nks = new KeySig(*ks);
                  SegmentType st = SegKeySig;
                  Segment* s = m->findSegment(st, 0);
                  if (s == 0) {
                        s = new Segment(m, st, 0);
                        _score->undoAddElement(s);
                        }
                  nks->setParent(s);
                  _score->undoAddElement(nks);
                  }
            }
      else {
            _score->undoInsertMeasure(mb, measure);
            }
      return mb;
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void ScoreView::cmdRepeatSelection()
      {
      const Selection& selection = _score->selection();
      if (selection.isSingle() && noteEntryMode()) {
            const InputState& is = _score->inputState();
            cmdAddPitch1(is.pitch, false);
            return;
            }

      if (selection.state() != SEL_RANGE) {
            printf("wrong selection type\n");
            return;
            }

      QString mimeType = selection.mimeType();
      if (mimeType.isEmpty()) {
            printf("mime type is empty\n");
            return;
            }
      QMimeData* mimeData = new QMimeData;
      mimeData->setData(mimeType, selection.mimeData());
      if (debugMode)
            printf("cmdRepeatSelection: <%s>\n", mimeData->data(mimeType).data());
      QApplication::clipboard()->setMimeData(mimeData);

      QByteArray data(mimeData->data(mimeType));

// printf("repeat <%s>\n", data.data());

      QDomDocument doc;
      int line, column;
      QString err;
      if (!doc.setContent(data, &err, &line, &column)) {
            printf("error reading paste data at line %d column %d: %s\n",
               line, column, qPrintable(err));
            printf("%s\n", data.data());
            return;
            }
      docName = "--";

      int dStaff = selection.staffStart();
      Segment* endSegment = selection.endSegment();
      if (endSegment && endSegment->subtype() != SegChordRest)
            endSegment = endSegment->next(SegChordRest);
      if (endSegment && endSegment->element(dStaff * VOICES)) {
            Element* e = endSegment->element(dStaff * VOICES);
            if (e) {
                  ChordRest* cr = static_cast<ChordRest*>(e);
                  _score->startCmd();
                  _score->pasteStaff(doc.documentElement(), cr);
                  _score->setLayoutAll(true);
                  _score->endCmd();
                  }
            else
                  printf("??? %p <%s>\n", e, e ? e->name() : "");
            }
      else {
            printf("cmdRepeatSelection: endSegment: %p dStaff %d\n", endSegment, dStaff);
            }
      }

//---------------------------------------------------------
//   cmdRepeatSelection
//---------------------------------------------------------

void ScoreView::selectMeasure(int n)
      {
      int i = 0;
      for (Measure* measure = _score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (++i < n)
                  continue;
            _score->selection().setState(SEL_RANGE);
            _score->selection().setStartSegment(measure->first());
            _score->selection().setEndSegment(measure->last());
            _score->selection().setStaffStart(0);
            _score->selection().setStaffEnd(_score->nstaves());
            _score->selection().updateSelectedElements();
            _score->selection().setState(SEL_RANGE);
            _score->addRefresh(measure->canvasBoundingRect());
            adjustCanvasPosition(measure, true);
            _score->setUpdateAll(true);
            _score->end();
            break;
            }
      }

//---------------------------------------------------------
//   search
//---------------------------------------------------------

void ScoreView::search(const QString& s)
      {
      bool ok;

      int n = s.toInt(&ok);
      if (!ok || n <= 0)
            return;
      search(n);
      }

void ScoreView::search(int n)
      {
      int i = 0;
      for (Measure* measure = _score->firstMeasure(); measure; measure = measure->nextMeasure()) {
            if (++i < n)
                  continue;
            adjustCanvasPosition(measure, true);
            int tracks = _score->nstaves() * VOICES;
            for (Segment* segment = measure->first(); segment; segment = segment->next()) {
                  if (segment->subtype() != SegChordRest)
                        continue;
                  int track;
                  for (track = 0; track < tracks; ++track) {
                        ChordRest* cr = static_cast<ChordRest*>(segment->element(track));
                        if (cr) {
                              Element* e;
                              if(cr->type() == CHORD)
                                    e =  static_cast<Chord*>(cr)->upNote();
                              else //REST
                                    e = cr;

                              _score->select(e, SELECT_SINGLE, 0);
                              break;
                              }
                        }
                  if (track != tracks)
                        break;
                  }
            _score->setUpdateAll(true);
            _score->end();
            break;
            }
      }

//---------------------------------------------------------
//   wrongPosition
//---------------------------------------------------------

static void wrongPosition()
      {
      printf("cannot enter notes here (no chord rest at current position)\n");
      }

//---------------------------------------------------------
//   cmdAddPitch1
//---------------------------------------------------------

void ScoreView::cmdAddPitch1(int pitch, bool addFlag)
      {
      InputState& is = _score->inputState();

      if (is.segment() == 0) {
            wrongPosition();
            return;
            }
      _score->startCmd();
      _score->expandVoice();
      if (is.cr() == 0) {
            wrongPosition();
            return;
            }
      if (!noteEntryMode()) {
            sm->postEvent(new CommandEvent("note-input"));
            qApp->processEvents();
            if (is.drumset())
                  is.setDrumNote(pitch);
            }
      if (noteEntryMode()) {
            Note* note = _score->addPitch(pitch, addFlag);
            if (note) {
                  mscore->play(note->chord());
                  adjustCanvasPosition(note, false);
                  }
            }
      else {
            Element* e = _score->selection().element();
            if (e && e->type() == NOTE) {
                  Note* note = static_cast<Note*>(e);
                  Chord* chord = note->chord();
                  int key = _score->staff(chord->staffIdx())->key(chord->segment()->tick()).accidentalType();
                  int newTpc = pitch2tpc(pitch, key);
                  _score->undoChangePitch(note, pitch, newTpc, note->line(), note->fret(), note->string());
                  }
            }
      _score->endCmd();
      }

//---------------------------------------------------------
//   layoutChanged
//---------------------------------------------------------

void ScoreView::layoutChanged()
      {
      if (mscore->navigator())
            mscore->navigator()->layoutChanged();
      }

