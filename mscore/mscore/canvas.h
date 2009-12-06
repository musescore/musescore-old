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

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

class Canvas : public Viewer {
      Q_OBJECT

      QFocusFrame* focusFrame;
      Navigator* navigator;
      int level;

      State state;
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

      //============================================

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect&, QPainter&);

      void canvasPopup(const QPoint&);
      void objectPopup(const QPoint&, Element*);
      void measurePopup(const QPoint&, Measure*);

      void saveChord(Xml&);

      virtual void resizeEvent(QResizeEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      void mouseMoveEvent1(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual bool event(QEvent*);
      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragLeaveEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);
      virtual void keyPressEvent(QKeyEvent*);
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
      void zoom(int step, const QPoint& pos);

   private slots:
      void setState(Viewer::State);
      void moveCursor();

   public slots:
      void setViewRect(const QRectF&);
      void dataChanged(const QRectF&);
      bool startEdit(Element*, int startGrip);

   public:
      Canvas(QWidget* parent = 0);
      ~Canvas();

      virtual void moveCursor(Segment*, int staffIdx);
      virtual void setCursorOn(bool);
      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);

      Page* addPage();

      void modifyElement(Element* obj);

      void clearScore();

      State getState() const { return state; }
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
      void magCanvas();
      virtual void cmd(const QAction* a);
      };

extern int searchStaff(const Element* element);

#endif

