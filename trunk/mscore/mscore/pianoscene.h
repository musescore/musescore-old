//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2009 Werner Schweer and others
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

#ifndef __PIANOSCENE_H__
#define __PIANOSCENE_H__

#include "al/pos.h"

class Staff;
class Score;

//---------------------------------------------------------
//   PianoScene
//---------------------------------------------------------

class PianoScene : public QGraphicsScene {
      Q_OBJECT

      Staff* staff;
      Score* _score;
      int magStep;
      AL::TType _timeType;
      int ticks;

      virtual void drawBackground(QPainter* painter, const QRectF& rect);
      AL::Pos pix2pos(int x) const;
      int pos2pix(const AL::Pos& p) const;

   public:
      PianoScene(Staff* staff, QWidget* parent = 0);
      void setMag(double);
      };

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

class PianoView : public QGraphicsView {
      Q_OBJECT

   protected:
      void wheelEvent(QWheelEvent* event);

   signals:
      void magChanged(double, double);
      void xposChanged(int);

   public:
      PianoView(Staff*);
      };


#endif
