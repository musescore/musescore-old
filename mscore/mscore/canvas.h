//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2009 Werner Schweer and others
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

#ifndef __SCANVAS_H__
#define __SCANVAS_H__

#include "viewer.h"

class Rest;
class Element;
class Transformation;
class Page;
class Xml;
class Note;
class Lasso;
class ShadowNote;
class Navigator;
class Cursor;
class ElementList;
class Segment;
class Measure;
class System;

//---------------------------------------------------------
//   CommandEvent
//---------------------------------------------------------

struct CommandEvent : public QEvent
      {
      QString value;
      CommandEvent(const QString& c)
         : QEvent(QEvent::Type(QEvent::User+1)), value(c) {}
      };

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

class Canvas : public Viewer {
      Q_OBJECT

      enum States { NORMAL, DRAG, DRAG_OBJECT, EDIT, DRAG_EDIT, LASSO,
            NOTE_ENTRY, MAG, PLAY, STATES
            };

      QStateMachine* sm;
      QState* states[STATES];

      QFocusFrame* focusFrame;
      Navigator* navigator;
      int level;

      bool dragCanvasState;
      bool draggedCanvas;
      Element* dragElement;   // current moved drag&drop element
      Element* dragObject;    // current canvas element

      QPointF dragOffset;
      bool mousePressed;

      // editing mode
      int curGrip;
      QRectF grip[4];         // edit "grips"
      int grips;              // number of used grips

      QPointF startMove;

      //--input state:
      Cursor* cursor;
      ShadowNote* shadowNote;

      Lasso* lasso;           ///< temporarily drawn lasso selection
      QRectF _lassoRect;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;

      Element* origEditObject;
      Element* editObject;          ///< Valid in edit mode
      QPointF _startDragPosition;
      int textUndoLevel;
      System* dragSystem;           ///< Valid if DRAG_STAFF.
      int dragStaff;
      //============================================

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual void resizeEvent(QResizeEvent*);
//      virtual void mousePressEvent(QMouseEvent*);
//      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
//      void mouseMoveEvent1(QMouseEvent*);
//      virtual void mouseReleaseEvent(QMouseEvent*);
//      virtual bool event(QEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void focusInEvent(QFocusEvent*);
      virtual void focusOutEvent(QFocusEvent*);

      void contextItem(Element*);

      void lassoSelect();
      Note* searchTieNote(Note* note);

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,const QList<const Element*>& el);
      bool dragTimeAnchorElement(const QPointF& pos);
      void dragSymbol(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      bool dragAboveMeasure(const QPointF& pos);
      bool dragAboveSystem(const QPointF& pos);
      void updateGrips();
      const QList<const Element*> elementsAt(const QPointF&);
      void lyricsTab(bool back, bool end);
      void lyricsReturn();
      void lyricsEndEdit();
      void lyricsUpDown(bool up, bool end);
      void lyricsMinus();
      void lyricsUnderscore();
      void harmonyEndEdit();
      void chordTab(bool back);
      void cmdAddPitch(int note, bool addFlag);
      void cmdAddChordName();
      void cmdAddText(int style);

   private slots:
      void moveCursor();
      void textUndoLevelAdded();
      void enterState();
      void exitState();

   public slots:
      void setViewRect(const QRectF&);
      void dataChanged(const QRectF&);

      void startEdit();
      void endEdit();
      void endStartEdit() { endEdit(); startEdit(); }

      void startDrag();
      void endDrag();

      void endDragEdit();

      void startNoteEntry();
      void endNoteEntry();

      void endLasso();
      void deselectAll();

   public:
      Canvas(QWidget* parent = 0);
      ~Canvas();

      void startEdit(Element*, int startGrip);
      virtual void startEdit(Element*);

      virtual void moveCursor(Segment*, int staffIdx);
      virtual void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();

      void modifyElement(Element* obj);

      void clearScore();

//      State getState() const { return state; }
      virtual void setScore(Score* s);

      virtual void setMag(qreal m);
      void showNavigator(bool visible);
      void redraw(const QRectF& r);
      void updateNavigator(bool layoutChanged) const;
      Element* elementAt(const QPointF& pp);
      Element* elementNear(const QPointF& pp);
      QRectF lassoRect() const { return _lassoRect; }
      void setLassoRect(const QRectF& r) { _lassoRect = r; }
      void paintLasso(QPainter& p, double mag);
      bool navigatorVisible() const;
      virtual void cmd(const QAction* a);

      void drag(const QPointF&);
      void endUndoRedo();
      void zoom(int step, const QPoint& pos);
      void contextPopup(QMouseEvent* ev);
      void setOrigEditObject(Element* e) { origEditObject = e; }
      void editKey(QKeyEvent*);
      Element* getDragObject() const { return dragObject; }
      void dragCanvas(QMouseEvent* ev);
      void dragNoteEntry(QMouseEvent* ev);
      void noteEntryButton(QMouseEvent* ev);
      void doDragElement(QMouseEvent* ev);
      void doDragLasso(QMouseEvent* ev);
      void doDragEdit(QMouseEvent* ev);
      void select(QMouseEvent*);
      void mousePress(QMouseEvent* ev);
      bool testElementDragTransition(QMouseEvent* ev) const;
      bool editElementDragTransition(QMouseEvent* ev);
      bool editCanvasDragTransition(QMouseEvent* e);
      void cmdAddSlur();
      void cmdAddSlur(Note* firstNote, Note* lastNote);
      bool noteEntryMode() const;
      void editInputTransition(QInputMethodEvent* ie);
      void onEditPasteTransition(QMouseEvent* ev);
      };

extern int searchStaff(const Element* element);

#endif

