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
#include "score.h"
#include "preferences.h"
#include "utils.h"
#include "segment.h"
#include "mscore.h"
#include "seq.h"
#include "staff.h"
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
#include "navigate.h"
#include "tablature.h"
#include "shadownote.h"
#include "sym.h"
#include "lasso.h"
#include "box.h"
#include "cursor.h"
#include "texttools.h"
#include "clef.h"
#include "scoretab.h"
#include "painter.h"


static const QEvent::Type CloneDrag = QEvent::Type(QEvent::User + 1);

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

      _cursor     = 0;
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
      QState* s;

      // setup normal state
      s = states[NORMAL];
      s->assignProperty(this, "cursor", QCursor(Qt::ArrowCursor));
      s->addTransition(new ContextTransition(this));                          // context menu
      EditTransition* et = new EditTransition(this, states[EDIT]);            // ->edit
      connect(et, SIGNAL(triggered()), SLOT(startEdit()));
      s->addTransition(et);
      s->addTransition(new SelectTransition(this));                           // select
      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
      s->addTransition(new DeSelectTransition(this));                         // deselect
      connect(s, SIGNAL(entered()), mscore, SLOT(setNormalState()));
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
//      connect(s, SIGNAL(entered()), mscore, SLOT(setEditState()));
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
      //-----------------------------------------------------------------------

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
      }

//---------------------------------------------------------
//   setScore
//---------------------------------------------------------

void ScoreView::setScore(Score* s)
      {
      _score = s;
      if (_cursor == 0) {
            _cursor = new Cursor(_score);
            shadowNote = new ShadowNote(_score);
            _cursor->setVisible(false);
            shadowNote->setVisible(false);
            }
      else {
            _cursor->setScore(_score);
            shadowNote->setScore(_score);
            }
      cursorSegment = 0;
      cursorTrack   = -2;

      lasso->setScore(s);
      _foto->setScore(s);
      connect(s, SIGNAL(updateAll()),                SLOT(updateAll()));
      connect(s, SIGNAL(dataChanged(const QRectF&)), SLOT(dataChanged(const QRectF&)));
      connect(s, SIGNAL(inputCursorChanged()),       SLOT(moveCursor()));
      }

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

ScoreView::~ScoreView()
      {
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

      if (obj->genPropertyMenu(popup))
            popup->addSeparator();

      a = popup->addAction(tr("Object Inspector"));
      a->setData("list");

      a = popup->exec(gpos);
      if (a == 0)
            return;
      QString cmd(a->data().toString());
      if (cmd == "cut" || cmd =="copy" || cmd == "paste" || cmd == "insert-measure"
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

      for (int i = 0; i < grips; ++i)
            score()->addRefresh(grip[i].adjusted(-dx, -dy, dx, dy));

      if (curGrip == -1)
            curGrip = grips-1;

      QPointF pt(editObject->getGrip(curGrip));
      mscore->setEditPos(pt);
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
            setDropAnchor(QLineF(anchor, grip[curGrip].center()));
      else
            setDropTarget(0); // this also resets dropAnchor
      score()->addRefresh(editObject->abbox());
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
      if (!score()->undo()->active())
            score()->startCmd();
      score()->setLayoutAll(false);
      curElement  = 0;
      setFocus();
      if (origEditObject->isText()) {
            editObject = origEditObject;
            editObject->startEdit(this, startMove);
            Text* t = static_cast<Text*>(editObject);
            _editText = t;
            mscore->textTools()->setText(t);
            mscore->textTools()->updateTools();
            textUndoLevel = 0;
            connect(t->doc(), SIGNAL(undoCommandAdded()), SLOT(textUndoLevelAdded()));
            }
      else if (origEditObject->isSegment()) {
            origEditObject->resetMode();
            SpannerSegment* ols = (SpannerSegment*)origEditObject;
            Spanner* ohp        = ols->spanner();

            LinkedElements* links = ohp->links();
            if (links) {
                  foreach(Element* e, *links) {
                        Spanner* sp = static_cast<Spanner*>(e);
                        Spanner* csp = static_cast<Spanner*>(sp->clone());
                        if (sp == ohp) {
                              int idx = sp->spannerSegments().indexOf(ols);
                              editObject = csp->spannerSegments().at(idx);
                              }
                        _score->undoChangeElement(sp, csp);
                        }
                  }
            else {
                  Spanner* hp         = (Spanner*)ohp->clone();
                  int idx             = ohp->spannerSegments().indexOf(ols);
                  editObject          = hp->spannerSegments().at(idx);
                  hp->setSelected(true);
                  _score->undoChangeElement(ohp, hp);
                  }
            editObject->startEdit(this, startMove);
            }
      else {
            LinkedElements* links = origEditObject->links();
            if (links) {
                  foreach(Element* e, *links) {
                        Element* ce = e->clone();
                        if (e == origEditObject) {
                              editObject = ce;
                              editObject->setSelected(true);
                              }
                        _score->undoChangeElement(e, ce);
                        }
                  }
            else {
                  editObject = origEditObject->clone();
                  editObject->setSelected(true);
                  _score->undoChangeElement(origEditObject, editObject);
                  }
            editObject->startEdit(this, startMove);
            }
      curGrip = -1;
      updateGrips();
      _score->setLayoutAll(true);
      score()->end();
      }

//---------------------------------------------------------
//   moveCursor
//---------------------------------------------------------

void ScoreView::moveCursor()
      {
      const InputState& is = _score->inputState();
      int track = is.track();
      if (track == -1)
            track = 0;

      _cursor->setTrack(track);
      Segment* segment = is.segment();
      if (segment)
            moveCursor(segment, track);
      }

void ScoreView::moveCursor(Segment* segment, int track)
      {
      if (cursorSegment == segment && cursorTrack == track)
            return;
      cursorTrack = track;
      cursorSegment = segment;

      int staffIdx = (track == -1) ? -1 : (track / VOICES);

      System* system = segment->measure()->system();
      if (system == 0) {
            // a new measure was appended but no layout took place
            printf("zero SYSTEM\n");
            return;
            }
      int idx         = staffIdx == -1 ? 0 : staffIdx;
      double x        = segment->canvasPos().x();
      double y        = system->staffY(idx);
      double _spatium = _cursor->spatium();

      update(_matrix.mapRect(_cursor->abbox()).toRect().adjusted(-1,-1,2,2));

      double h;
      double w;
      if (staffIdx == -1) {
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
            // w = _spatium * 2.0;
            w  = _spatium * 2.0 + symbols[score()->symIdx()][quartheadSym].width(_cursor->magS());
            x -= _spatium;
            y -= _spatium;
            }
      else {
            Staff* staff    = _score->staff(staffIdx);
            double lineDist = staff->useTablature() ? 1.5 * _spatium : _spatium;
            int lines       = staff->lines();
            h               = (lines - 1) * lineDist + 4 * _spatium;
            w               = _spatium * 2.0 + symbols[score()->symIdx()][quartheadSym].width(_cursor->magS());
            x              -= _spatium;
            y              -= 2.0 * _spatium;
            }
      _cursor->setPos(x, y);
      QRectF r(0.0, 0.0, w, h);
      _cursor->setbbox(r);
      update(_matrix.mapRect(_cursor->abbox()).toRect().adjusted(-1,-1,2,2));
      }

//---------------------------------------------------------
//   cursorTick
//---------------------------------------------------------

int ScoreView::cursorTick() const
      {
      return cursorSegment ? cursorSegment->tick() : 0;
      }

//---------------------------------------------------------
//   setCursorOn
//---------------------------------------------------------

void ScoreView::setCursorOn(bool val)
      {
      if (_cursor && (_cursor->visible() != val)) {
            _cursor->setVisible(val);
            cursorSegment = 0;
            cursorTrack   = -1;
            update(_matrix.mapRect(_cursor->abbox()).toRect().adjusted(-1,-1,2,2));
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

static void paintElement(void* data, Element* e)
      {
      Painter* p = static_cast<Painter*>(data);
      p->painter()->save();
      p->painter()->setPen(QPen(e->curColor()));
      p->painter()->translate(e->canvasPos());
      e->draw(p);
      p->painter()->restore();
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
      Painter vp(&p, this);

      QRegion region = ev->region();

      const QVector<QRect>& vector = region.rects();
      foreach(const QRect& r, vector)
            paint(r, p);

      p.setTransform(_matrix);
      p.setClipping(false);

      _cursor->draw(&vp);
      lasso->draw(&vp);
      if (fotoMode())
            _foto->draw(&vp);
      shadowNote->draw(&vp);
      if (!dropAnchor.isNull()) {
            QPen pen(QBrush(QColor(80, 0, 0)), 2.0 / p.worldMatrix().m11(), Qt::DotLine);
            p.setPen(pen);
            p.drawLine(dropAnchor);
            }
      if (dragElement)
            dragElement->scanElements(&vp, paintElement);

      if (grips) {
            qreal lw = 2.0/p.matrix().m11();
            // QPen pen(Qt::blue);
            QPen pen(preferences.defaultColor);
            pen.setWidthF(lw);
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
//   paint
//---------------------------------------------------------

void ScoreView::paint(const QRect& rr, QPainter& p)
      {
      p.save();
      if (fgPixmap == 0 || fgPixmap->isNull())
            p.fillRect(rr, _fgColor);
      else {
            p.drawTiledPixmap(rr, *fgPixmap, rr.topLeft()
               - QPoint(lrint(_matrix.dx()), lrint(_matrix.dy())));
            }

      p.setTransform(_matrix);
      QRectF fr = imatrix.mapRect(QRectF(rr));

      QRegion r1(rr);
      foreach (Page* page, _score->pages()) {
            QRectF pr(page->abbox());
            if (pr.right() < fr.left())
                  continue;
            if (pr.left() > fr.right())
                  break;
            if (debugMode) {
                  //
                  // show page margins
                  //
//                  QRectF bpr = pr.adjusted(page->lm(), page->tm(), -page->rm(), -page->bm());
                  p.setPen(QPen(Qt::gray));
            //      p.drawRect(bpr);
                  p.drawRect(fr);
                  }
            QList<const Element*> ell = page->items(fr);
            qStableSort(ell.begin(), ell.end(), elementLessThan);
            drawElements(p, ell);
            r1 -= _matrix.mapRect(pr).toAlignedRect();
            }
//      p.setClipRect(fr);

      if (dropRectangle.isValid())
            p.fillRect(dropRectangle, QColor(80, 0, 0, 80));

      //
      // frame text in edit mode, except for text in a text frame
      //
      if (_editText && !(_editText->parent() && _editText->parent()->type() == TBOX)) {
            QRectF r = _editText->pageRectangle(); // abbox();
//            qreal w = 6.0 / matrix().m11();   // 6 pixel border
//            r.adjust(-w, -w, w, w);
            p.setPen(QPen(QBrush(Qt::blue), 2.0 / matrix().m11()));  // 2 pixel pen size
            p.setBrush(QBrush(Qt::NoBrush));
            p.drawRect(r);
            }
#if 0
      if (grips) {
            qreal lw = 2.0/p.matrix().m11();
            // QPen pen(Qt::blue);
            QPen pen(preferences.defaultColor);
            pen.setWidthF(lw);
            p.setPen(pen);
            for (int i = 0; i < grips; ++i) {
                  p.setBrush(((i == curGrip) && hasFocus()) ? QBrush(Qt::blue) : Qt::NoBrush);
                  p.drawRect(grip[i]);
                  }
            }
#endif
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
            double x2      = ss->canvasPos().x() - _spatium;
            int staffStart = sel.staffStart();
            int staffEnd   = sel.staffEnd();

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
                  Segment* ns = s->next1();
                  system1  = system2;
                  system2  = s->measure()->system();
                  pt       = s->canvasPos();
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
      _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m13(), _matrix.m21(),
         _matrix.m22(), _matrix.m23(), _matrix.dx()+dx, _matrix.dy()+dy, _matrix.m33());
      imatrix = _matrix.inverted();
      scroll(dx, dy, QRect(0, 0, width(), height()));
      emit offsetChanged(_matrix.dx(), _matrix.dy());
      }

//---------------------------------------------------------
//   dragTimeAnchorElement
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
            QPointF anchor(seg->abbox().x(), y);
            setDropAnchor(QLineF(pos, anchor));
            dragElement->setTrack(staffIdx * VOICES);
//TODO1            if (dragElement->type() == SYMBOL)
//                  static_cast<Symbol*>(dragElement)->setTick(tick);
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
            QRectF b(m->abbox());

            QPointF anchor;
            if (pos.x() < (b.x() + b.width() * .5))
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
//   dragEnterEvent
//---------------------------------------------------------

void ScoreView::dragEnterEvent(QDragEnterEvent* event)
      {
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
            score()->addRefresh(dragElement->abbox());
//            dragElement->setPos(pos - dragOffset);
            dragElement->setPos(pos);
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
            _score->addRefresh(dragElement->abbox());
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
                        _score->addRefresh(el->abbox());
                        _score->addRefresh(dragElement->abbox());

                        Element* dropElement = el->drop(dropData);
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
                        _score->addRefresh(el->abbox());
                        Element* dropElement = el->drop(dropData);
                        _score->addRefresh(el->abbox());
                        if (dropElement) {
                              if (!_score->noteEntryMode())
                                    _score->select(dropElement, SELECT_SINGLE, 0);
                              _score->addRefresh(dropElement->abbox());

                              Qt::KeyboardModifiers keyState = event->keyboardModifiers();
                              if (keyState == (Qt::ShiftModifier | Qt::ControlModifier)) {
                                    //
                                    // continue drag with a cloned element
                                    //
                                    CloneEvent* ce = new CloneEvent(dropElement);
                                    QCoreApplication::postEvent(this, ce);
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
            for (int i = 0; i < step; ++i) {
                   _mag *= 1.1;
                  }
            }
      else {
            for (int i = 0; i < -step; ++i) {
                  _mag /= 1.1;
                  }
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
//   wheelEvent
//---------------------------------------------------------

void ScoreView::wheelEvent(QWheelEvent* event)
      {
      if (event->buttons() & Qt::RightButton) {
            int n = event->delta() / 120;
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
            if (page->abbox().contains(p))
                  return page;
            }
      return 0;
      }

//---------------------------------------------------------
//   elementsAt
//---------------------------------------------------------

const QList<const Element*> ScoreView::elementsAt(const QPointF& p)
      {
      QList<const Element*> el;

      Page* page = point2page(p);
      if (page) {
            el = page->items(p);
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

Element* ScoreView::elementNear(const QPointF& p)
      {
      double w  = (preferences.proximity * .5) / matrix().m11();
      QRectF r(p.x() - w, p.y() - w, 3.0 * w, 3.0 * w);

      Page* page = point2page(p);
      if (!page) {
            // printf("  no page\n");
            return 0;
            }

      QList<const Element*> el = page->items(r);
      QList<const Element*> ll;
      for (int i = 0; i < el.size(); ++i) {
            const Element* e = el.at(i);
            e->itemDiscovered = 0;
            if (e->selectable() && (e->type() != PAGE) && e->contains(p))
                  ll.append(e);
            }
      int n = ll.size();
      if ((n == 0) || ((n == 1) && (ll[0]->type() == MEASURE))) {
            //
            // if no relevant element hit, look nearby
            //
            for (int i = 0; i < el.size(); ++i) {
                  const Element* e = el.at(i);
                  if ((e->type() != PAGE) && e->selectable() && e->intersects(r))
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
//   drawElements
//---------------------------------------------------------

void ScoreView::drawElements(QPainter& p,const QList<const Element*>& el)
      {
      Painter painter(&p, this);

      foreach(const Element* e, el) {
            e->itemDiscovered = 0;
            if (!e->visible()) {
                  if (score()->printing() || !score()->showInvisible())
                        continue;
                  }
            p.save();
            p.translate(e->canvasPos());
            p.setPen(QPen(e->curColor()));
            e->draw(&painter);

            if (debugMode && e->selected()) {
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
                        const Element* ee = e->parent();
                        if (e->type() == NOTE)
                              ee = static_cast<const Note*>(e)->chord()->segment();
                        else if (e->type() == CLEF)
                              ee = static_cast<const Clef*>(e)->segment();

                        p.setPen(QPen(Qt::green, 0, Qt::SolidLine));
                        p.drawRect(ee->abbox());
#if 1
                        if (ee->type() == SEGMENT) {
                              qreal w    = 7.0 / p.matrix().m11();
                              QPointF pt = ee->canvasPos();
                              p.setPen(QPen(Qt::blue, 0, Qt::SolidLine));
                              p.drawLine(QLineF(pt.x()-w, pt.y()-h, pt.x()+w, pt.y()+h));
                              p.drawLine(QLineF(pt.x()+w, pt.y()-h, pt.x()-w, pt.y()+h));
                              }
#endif
                        continue;
                        }
                  }
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
                  QPointF pt(preferences.nudgeStep * el->spatium(), 0.0);
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
      else if (cmd == "reset-positions") {
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
            _score->cmdSplitMeasure();
            }
      else if (cmd == "join-measure") {
            _score->cmdJoinMeasure();
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
//   endEdit
//---------------------------------------------------------

void ScoreView::endEdit()
      {
      setDropTarget(0);
      setEditText(0);
      if (!editObject)
	      return;

      _score->addRefresh(editObject->bbox());
      editObject->endEdit();
      _score->addRefresh(editObject->bbox());

      if (editObject->isText()) {
            Text* t = static_cast<Text*>(editObject);
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
      grips      = 0;
      }

//---------------------------------------------------------
//   startDrag
//---------------------------------------------------------

void ScoreView::startDrag()
      {
      dragElement = curElement;
      startMove -= dragElement->userOff();
      _score->startCmd();

      foreach(Element* e, _score->selection().elements())
            e->setStartDragPosition(e->userOff());
      QList<Element*> el;
      dragElement->scanElements(&el, collectElements);
//TODO2      foreach(Element* e, el)
//            _score->removeBsp(e);
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
      foreach(Element* e, _score->selection().elements())
            _score->addRefresh(e->drag(pt));
      _score->end();
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
      _score->inputState().setSegment(0);
      Note* note  = 0;
      Element* el = _score->selection().activeCR() ? _score->selection().activeCR() : _score->selection().element();
      if (el == 0 || (el->type() != CHORD && el->type() != REST && el->type() != NOTE)) {
            int track = _score->inputState().track() == -1 ? 0 : _score->inputState().track();
            el = static_cast<ChordRest*>(_score->searchNote(0, track));
            if (el == 0) {
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
      Duration d(_score->inputState().duration());
      if (!d.isValid() || d.isZero() || d.type() == Duration::V_MEASURE)
            _score->inputState().setDuration(Duration(Duration::V_QUARTER));

      _score->select(el, SELECT_SINGLE, 0);
      _score->inputState().noteEntryMode = true;
      mscore->updateInputState(_score);
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
      _score->setLayoutAll(false);
      _score->setUpdateAll(true);   //DEBUG
      _score->end();    // update
      }

//---------------------------------------------------------
//   mousePress
//    return true if element is clicked
//---------------------------------------------------------

bool ScoreView::mousePress(QMouseEvent* ev)
      {
      startMove  = imatrix.map(QPointF(ev->pos()));
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
      return sqrt(pow(delta.x(),2) + pow(delta.y(),2)) > 2;
      }

//---------------------------------------------------------
//   endDragEdit
//---------------------------------------------------------

void ScoreView::endDragEdit()
      {
      _score->addRefresh(editObject->abbox());
      editObject->endEditDrag();
      updateGrips();
      // setDropTarget(0); // this also resets dropRectangle and dropAnchor
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
      startMove = imatrix.map(QPointF(ev->pos()));
      Element* e = elementNear(startMove);
      if (e && (e == editObject) && (editObject->isText())) {
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
      if (e == editObject) {
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
      _score->addRefresh(lasso->abbox());
      QRectF r;
      r.setCoords(startMove.x(), startMove.y(), p.x(), p.y());
      lasso->setRect(r.normalized());
      QRectF _lassoRect(lasso->rect());
      r = _matrix.mapRect(_lassoRect);
      QSize sz(r.size().toSize());
      mscore->statusBar()->showMessage(QString("%1 x %2").arg(sz.width()).arg(sz.height()), 3000);
      _score->addRefresh(lasso->abbox());
      _score->lassoSelect(lasso->rect());
      _score->end();
      }

//---------------------------------------------------------
//   endLasso
//---------------------------------------------------------

void ScoreView::endLasso()
      {
      _score->addRefresh(lasso->abbox());
      lasso->setRect(QRectF());
      _score->lassoSelectEnd();
      _score->end();
      }

//---------------------------------------------------------
//   deselectAll
//---------------------------------------------------------

void ScoreView::deselectAll()
      {
      _score->deselectAll();
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
      qreal lx   = 10.0 - page->canvasPos().x() * mag();
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
      Page* page = score()->pages().back();
      qreal x = xoffset() +( page->width() + 25.0) * mag();
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
      QPointF p(lastPage->canvasPos());
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

      qreal x   = - xoffset() / mag();
      qreal y   = - yoffset() / mag();

      qreal oldX = x, oldY = y;

      if (showRect.left() < r.left())
            x = showRect.left() - BORDER_X;
      else if (showRect.left() > r.right())
            x = showRect.right() - width() / mag() + BORDER_X;
      else if (r.width() >= showRect.width() && showRect.right() > r.right())
            x = showRect.left() - BORDER_X;
      if (showRect.top() < r.top() && showRect.bottom() < r.bottom())
            y = showRect.top() - BORDER_Y;
      else if (showRect.top() > r.bottom())
            y = showRect.bottom() - height() / mag() + BORDER_Y;
      else if (r.height() >= showRect.height() && showRect.bottom() > r.bottom())
            y = showRect.top() - BORDER_Y;

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
      return QWidget::event(event);
      }

//---------------------------------------------------------
//   startUndoRedo
//---------------------------------------------------------

void ScoreView::startUndoRedo()
      {
      // exit edit mode
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
      _score->setLayoutAll(true);
      mscore->updateInputState(_score);
      _score->end();
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
      if (!e->isMovable())
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

