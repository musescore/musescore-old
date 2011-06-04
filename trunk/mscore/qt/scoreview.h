//=============================================================================
//  MuseScore
//  Music Score Editor/Player
//  $Id:$
//
//  Copyright (C) 2011 Werner Schweer
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

#ifndef __SCOREVIEW_H__
#define __SCOREVIEW_H__

#include <QtGui/QWidget>

class ScoreProxy;
class QString;
class QWheelEvent;

//---------------------------------------------------------
//   ScoreView
//---------------------------------------------------------

class ScoreView : public QWidget {
      ScoreProxy* score;
      QTransform _matrix, imatrix;
      QPoint startDrag;

      virtual void paintEvent(QPaintEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void mousePressEvent(QMouseEvent*);
      void zoom(int step, const QPoint& pos);

   public:
      ScoreView(QWidget* parent = 0);
      void loadFile(const QString&);
      };


#endif

