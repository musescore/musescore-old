//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: navigator.h,v 1.3 2006/03/02 17:08:37 wschweer Exp $
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__

class Score;

//---------------------------------------------------------
//   Navigator
//---------------------------------------------------------

class Navigator : public QFrame {
      Q_OBJECT

      Score* _score;
      QRectF viewRect;
      QPointF startMove;
      bool moving;
      QPixmap pm;
      bool redraw;
      QMatrix matrix;

      virtual void paintEvent(QPaintEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void mouseReleaseEvent(QMouseEvent*);

   signals:
      void viewRectMoved(const QRectF& r);

   public:
      Navigator(QWidget* parent);
      void setScore(Score*);
      void setViewRect(const QRectF& r);
      void layoutChanged();
      };

#endif

