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

#ifndef __VIEWER_H__
#define __VIEWER_H__

class Score;
class Element;
class Segment;
class TextB;

//---------------------------------------------------------
//   Viewer
//---------------------------------------------------------

class Viewer : public QWidget {
      Q_OBJECT

   public:
      enum State {
         NORMAL, DRAG_OBJ, EDIT, DRAG_EDIT, LASSO, NOTE_ENTRY, MAG
         };
   protected:
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
      TextB* _editText;

      QMatrix _matrix, imatrix;
      int _magIdx;

   public slots:
      void adjustCanvasPosition(Element* el, bool playBack);

   public:
      Viewer(QWidget* parent = 0);
      virtual ~Viewer() {}
      virtual void setScore(Score* s) = 0;
      Score* score() const    { return _score; }
      void setDropRectangle(const QRectF&);
      void setDropTarget(const Element*);
      void setDropAnchor(const QLineF&);
      const QMatrix& matrix() const              { return _matrix; }
      void setEditText(TextB* t)                 { _editText = t;      }
      TextB* editText() const                    { return _editText;   }
      virtual void magCanvas()                   {}
      qreal mag() const;
      int magIdx() const                         { return _magIdx; }
      void setMag(int idx, double mag);
      virtual void setMag(double) = 0;
      qreal xoffset() const;
      qreal yoffset() const;
      void setOffset(qreal x, qreal y);
      QSizeF fsize() const;
      virtual void setCursorOn(bool) {}
      virtual void moveCursor(Segment*, int) {}
      void pageNext();
      void pagePrev();
      void pageTop();
      void pageEnd();
      virtual void cmd(const QAction* a) = 0;
      };

#endif
