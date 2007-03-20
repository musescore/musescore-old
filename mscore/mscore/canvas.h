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

#include "viewer.h"
#include "bsp.h"
#include "element.h"

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
      ScoreLayout* _layout;

      int keyState;
      int buttonState;
      State state;
      bool dragCanvasState;
      bool mousePressed;

      bool cursorIsBlinking;

      QPointF startMove;

      //--input state:
      Cursor* cursor;
      ShadowNote* shadowNote;
      QTimer* cursorTimer;    // blink timer

      Lasso* lasso;           ///< temporarily drawn lasso selection

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

      void lassoSelect();
      Note* searchTieNote(Note* note);

      void setShadowNote(const QPointF&);
      void drawElements(QPainter& p,const QList<Element*>& el);
      bool dragTimeAnchorElement(const QPointF& pos);
      bool dragAboveMeasure(const QPointF& pos);
      bool dragAboveSystem(const QPointF& pos);

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

      virtual void dataChanged(const QRectF&);
      void setState(State);
      State getState() const { return state; }
      bool startEdit(Element*);
      void setScore(Score* s, ScoreLayout*);
      Score* score() const    { return _score; }

      qreal mag() const;
      qreal xoffset() const   { return _matrix.dx();  }
      qreal yoffset() const   { return _matrix.dy();  }
      void setMag(qreal m);
      void setXoffset(qreal x)  {
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
               _matrix.m22(), x, _matrix.dy());
            imatrix = _matrix.inverted();
            }
      void setYoffset(qreal y)  {
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
               _matrix.m22(), _matrix.dx(), y);
            imatrix = _matrix.inverted();
            }
      void setOffset(qreal x, qreal y) {
            _matrix.setMatrix(_matrix.m11(), _matrix.m12(), _matrix.m21(),
               _matrix.m22(), x, y);
            imatrix = _matrix.inverted();
            }
      qreal xMag() const { return _matrix.m11(); }
      qreal yMag() const { return _matrix.m22(); }

      QRectF vGeometry() const {
            return imatrix.mapRect(geometry());
            }

      QSizeF fsize() const;
      void showNavigator(bool visible);
      void redraw(const QRectF& r);
      void updateNavigator(bool layoutChanged) const;
      Element* elementAt(const QPointF& pp);
      };

extern int searchStaff(const Element* element);

#endif

