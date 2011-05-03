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

#include "globals.h"
#include "durationtype.h"

class ChordRest;
class Rest;
class Element;
class Page;
class Xml;
class Note;
class Lasso;
class ShadowNote;
class Cursor;
class Segment;
class Measure;
class System;
class Score;
class ScoreView;
class Text;
class MeasureBase;

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
//   ScoreViewDragTransition
//---------------------------------------------------------

class ScoreViewDragTransition : public QMouseEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual bool eventTest(QEvent* event);

   public:
      ScoreViewDragTransition(ScoreView* c, QState* target);
      };

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
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QWidget {
      Q_OBJECT

      enum States { NORMAL, DRAG, DRAG_OBJECT, EDIT, DRAG_EDIT, LASSO,
            NOTE_ENTRY, MAG, PLAY, SEARCH, ENTRY_PLAY, FOTOMODE,
            STATES
            };
      static const int MAX_GRIPS = 8;

      Score* _score;

      // the next elements are used during dragMove to give some visual
      // feedback:
      //    dropTarget:       if valid, the element is drawn in a different color
      //                      to mark it as a valid drop target
      //    dropRectangle:    if valid, the rectangle is filled with a
      //                      color to visualize a valid drop area
      //    dropAnchor:       if valid the line is drawn from the current
      //                      cursor position to the current anchor point
      // Note:
      //    only one of the elements is active during drag

      const Element* dropTarget;    ///< current drop target during dragMove
      QRectF dropRectangle;         ///< current drop rectangle during dragMove
      QLineF dropAnchor;            ///< line to current anchor point during dragMove

      // in text edit mode text is framed
      Text* _editText;

      QTransform _matrix, imatrix;
      int _magIdx;

      QStateMachine* sm;
      QState* states[STATES];
      bool addSelect;

      QFocusFrame* focusFrame;

      Element* dragElement;   // valid in state DRAG_OBJECT

      Element* curElement;    // current item at mouse press
      QPointF startMove;      // position of last mouse press
      QPoint  startMoveI;

      QPointF dragOffset;

      // editing mode
      int curGrip;
      QRectF grip[8];         // edit "grips"
      int grips;              // number of used grips
      Element* origEditObject;
      Element* editObject;         ///< Valid in edit mode
      int textUndoLevel;

      //--input state:
      Cursor* _cursor;
      Segment* cursorSegment;
      int cursorTrack;
      ShadowNote* shadowNote;

      Lasso* lasso;           ///< temporarily drawn lasso selection
      Lasso* _foto;

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);
      void paint1(bool printMode, const QRectF&, QPainter&);

      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual bool event(QEvent* event);
      virtual void resizeEvent(QResizeEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void focusInEvent(QFocusEvent*);
      virtual void focusOutEvent(QFocusEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);

      void contextItem(Element*);

      void lassoSelect();
      Note* searchTieNote(Note* note);

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,const QList<const Element*>& el);
      bool dragTimeAnchorElement(const QPointF& pos);
      void dragSymbol(const QPointF& pos);
      bool dragMeasureAnchorElement(const QPointF& pos);
      void updateGrips();
      const QList<const Element*> elementsAt(const QPointF&);
      void lyricsTab(bool back, bool end, bool moveOnly);
      void lyricsReturn();
      void lyricsEndEdit();
      void lyricsUpDown(bool up, bool end);
      void lyricsMinus();
      void lyricsUnderscore();
      void harmonyEndEdit();
      void chordTab(bool back);
      void cmdInsertNote(int note);
      void cmdAddPitch(int note, bool addFlag);
      void cmdAddPitch1(int, bool);
      void cmdAddChordName();
      void cmdAddText(int style);
      void cmdEnterRest(const Duration&);
      void cmdEnterRest();
      void cmdTuplet(int n, ChordRest*);
      void cmdTuplet(int);
      void cmdRepeatSelection();
      void cmdChangeEnharmonic(bool);

      Page* point2page(const QPointF&);
      void setupFotoMode();

      void insertMeasures(int, ElementType);
      void appendMeasures(int, ElementType);
      MeasureBase* appendMeasure(ElementType);
      void cmdInsertMeasure(ElementType);
      MeasureBase* insertMeasure(ElementType, int tick);

   private slots:
      void textUndoLevelAdded();
      void enterState();
      void exitState();
      void startFotomode();
      void stopFotomode();
      void startFotoDrag();
      void endFotoDrag();
      void endFotoDragEdit();

   public slots:
      void moveCursor();
      void setViewRect(const QRectF&);
      void dataChanged(const QRectF&);
      void updateAll();

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
      void adjustCanvasPosition(const Element* el, bool playBack);

      void editCopy();
      void editPaste();

      void normalCut();
      void normalCopy();
      void normalPaste();

      void cloneElement(Element* e);
      void doFotoDragEdit(QMouseEvent* ev);

   signals:
      void viewRectChanged();
      void scaleChanged(double);
      void offsetChanged(double, double);

   public:
      ScoreView(QWidget* parent = 0);
      ~ScoreView();

      void startEdit(Element*, int startGrip);
      void startEdit(Element*);

      void moveCursor(Segment*, int staffIdx);
      int cursorTick() const;
      void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();
      void modifyElement(Element* obj);
      void setScore(Score* s);

      void setMag(qreal m);
      Element* elementAt(const QPointF& pp);
      Element* elementNear(const QPointF& pp);
      bool navigatorVisible() const;
      void cmd(const QAction* a);

      void drag(const QPointF&);
      void startUndoRedo();
      void endUndoRedo();
      void zoom(int step, const QPoint& pos);
      void contextPopup(QMouseEvent* ev);
      void setOrigEditObject(Element* e) { origEditObject = e; }
      void editKey(QKeyEvent*);
      void dragScoreView(QMouseEvent* ev);
      void dragNoteEntry(QMouseEvent* ev);
      void noteEntryButton(QMouseEvent* ev);
      void doDragElement(QMouseEvent* ev);
      void doDragLasso(QMouseEvent* ev);
      void doDragFoto(QMouseEvent* ev);
      void doDragEdit(QMouseEvent* ev);
      void select(QMouseEvent*);
      bool mousePress(QMouseEvent* ev);
      bool testElementDragTransition(QMouseEvent* ev) const;
      bool editElementDragTransition(QMouseEvent* ev);
      bool fotoEditElementDragTransition(QMouseEvent* ev);
      bool editScoreViewDragTransition(QMouseEvent* e);
      void cmdAddSlur();
      void cmdAddSlur(Note* firstNote, Note* lastNote);
      bool noteEntryMode() const;
      bool editMode() const;
      bool fotoMode() const;

      void editInputTransition(QInputMethodEvent* ie);
      void onEditPasteTransition(QMouseEvent* ev);

      Score* score() const                      { return _score; }
      void setDropRectangle(const QRectF&);
      void setDropTarget(const Element*);
      void setDropAnchor(const QLineF&);
      const QTransform& matrix() const           { return _matrix; }
      void setEditText(Text* t)                 { _editText = t;      }
      Text* editText() const                    { return _editText;   }
      qreal mag() const;
      int magIdx() const                         { return _magIdx; }
      void setMag(int idx, double mag);
      qreal xoffset() const;
      qreal yoffset() const;
      void setOffset(qreal x, qreal y);
      QSizeF fsize() const;
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      QPointF toLogical(const QPoint& p) const { return imatrix.map(QPointF(p)); }
      QRectF  toLogical(const QRectF& r) const { return imatrix.mapRect(r);      }
      QRect toPhysical(const QRectF& r) const { return _matrix.mapRect(r).toRect(); }

      void search(const QString& s);
      void search(int i);
      void postCmd(const char* cmd)   { sm->postEvent(new CommandEvent(cmd));  }
      void setFocusRect();
      Element* getDragElement() const { return dragElement; }
      void changeVoice(int voice);
      void drawBackground(QPainter& p, QRectF r);
      bool fotoScoreViewDragTest(QMouseEvent*);
      bool fotoScoreViewDragRectTest(QMouseEvent*);
      void doDragFotoRect(QMouseEvent*);
      void fotoContextPopup(QMouseEvent*);
      bool fotoRectHit(const QPoint& p);
      void paintRect(bool printMode, QPainter& p, const QRectF& r, double mag);
      bool saveFotoAs(bool printMode, const QRectF&);
      void fotoDragDrop(QMouseEvent*);
      const QRectF& getGrip(int n) const { return grip[n]; }
      int gripCount() const { return grips; }              // number of used grips
      void changeEditElement(Element*);

      void cmdAppendMeasures(int, ElementType);
      void cmdInsertMeasures(int, ElementType);

      ScoreState mscoreState() const;
      void setCursorVisible(bool v);
      void showOmr(bool flag);
      Element* getCurElement() const { return curElement; }   // current item at mouse press
      void midiNoteReceived(int pitch, bool);
      };

//---------------------------------------------------------
//   DragTransition
//---------------------------------------------------------

class DragTransition : public QEventTransition
      {
      ScoreView* canvas;

   protected:
      virtual void onTransition(QEvent* e);

   public:
      DragTransition(ScoreView* c)
         : QEventTransition(c, QEvent::MouseMove), canvas(c) {}
      };

extern int searchStaff(const Element* element);

#endif

