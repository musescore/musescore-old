//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: canvas.h,v 1.35 2006/09/15 09:34:57 wschweer Exp $
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

#ifndef __SCANVAS_H__
#define __SCANVAS_H__

// #include "page.h"
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

//---------------------------------------------------------
//   Canvas
//---------------------------------------------------------

class Canvas : public QWidget, public Viewer {
      Q_OBJECT

   public:
      enum State { NORMAL, DRAG_OBJ, DRAG_STAFF,
         EDIT, DRAG_EDIT, LASSO, NOTE_ENTRY, MAG };

   private:
      Navigator* navigator;
      Score* _score;
      int keyState;
      int buttonState;
      State state;
      bool dragCanvasState;
      bool mousePressed;

      bool cursorIsBlinking;

      QMatrix matrix, imatrix;

      QPointF startMove;

      //--input state:
      Cursor* cursor;
      ShadowNote* shadowNote;
      QTimer* cursorTimer;    // blink timer

      Element* dropTarget;
      int propertyStaff;

      Lasso* lasso;           // temporarily drawn lasso selection

      QColor _bgColor;
      QColor _fgColor;
      QPixmap* bgPixmap;
      QPixmap* fgPixmap;

      //============================================

      virtual void paintEvent(QPaintEvent*);
      void paint(const QRect& r);
      virtual void updateAll(Score*) { update(); }

      void canvasPopup(const QPoint&);
      void objectPopup(const QPoint&, Element*);

      void saveChord(Xml&);

      virtual void resizeEvent(QResizeEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      void mouseMoveEvent1(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);
      void mouseReleaseEvent1(QMouseEvent*);
      virtual void mouseDoubleClickEvent(QMouseEvent*);
      virtual bool event(QEvent*);

      virtual void dragEnterEvent(QDragEnterEvent*);
      virtual void dragLeaveEvent(QDragEnterEvent*);
      virtual void dragMoveEvent(QDragMoveEvent*);
      virtual void dropEvent(QDropEvent*);

      void contextItem(Element*);

      QRegion lassoSelect();
      Note* searchTieNote(Note* note);

      void setShadowNote(const QPointF&);

   private slots:
      void cursorBlink();

   public slots:
      virtual void keyPressEvent(QKeyEvent*);
      void cmdCut();
      void cmdCopy();
      void cmdPaste();
      void magCanvas();

      void resetStaffOffsets();
      void setViewRect(const QRectF&);

   public:
      Canvas(QWidget* parent = 0);
      ~Canvas();

      void setBackground(QPixmap*);
      void setBackground(const QColor&);
      void setForeground(QPixmap*);
      void setForeground(const QColor&);
      void setChanged(double mag);

      Page* addPage();

      void modifyElement(Element* obj);

      virtual QRectF moveCursor();
      void clearScore();

      virtual void dataChanged(Score* cp, const QRectF&);
      void setState(State);
      State getState() const { return state; }
      bool startEdit(Element*);
      void setScore(Score* s);
      Score* score() const    { return _score; }

      qreal mag() const;
      qreal xoffset() const   { return matrix.dx();  }
      qreal yoffset() const   { return matrix.dy();  }
      void setMag(qreal m);
      void setXoffset(qreal x)  {
            matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
               matrix.m22(), x, matrix.dy());
            imatrix = matrix.inverted();
            }
      void setYoffset(qreal y)  {
            matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
               matrix.m22(), matrix.dx(), y);
            imatrix = matrix.inverted();
            }
      void setOffset(qreal x, qreal y) {
            matrix.setMatrix(matrix.m11(), matrix.m12(), matrix.m21(),
               matrix.m22(), x, y);
            imatrix = matrix.inverted();
            }
      qreal xMag() const { return matrix.m11(); }
      qreal yMag() const { return matrix.m22(); }

      QRectF vGeometry() const {
            return imatrix.mapRect(geometry());
            }

      QSizeF fsize() const;
      void showNavigator(bool visible);
      void redraw(const QRectF& r);
      void updateNavigator(bool layoutChanged) const;
      };

extern int searchStaff(const Element* element);

#endif

