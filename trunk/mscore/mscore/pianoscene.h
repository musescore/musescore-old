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
class Note;

//---------------------------------------------------------
//   PianoItem
//---------------------------------------------------------

class PianoItem : public QGraphicsRectItem {

   public:
      PianoItem(Note*);
      };

//---------------------------------------------------------
//   PianoView
//---------------------------------------------------------

class PianoView : public QGraphicsView {
      Q_OBJECT

      Staff* staff;
      AL::Pos pos;
      AL::Pos* _locator;
      QGraphicsLineItem* locatorLines[3];
      int ticks;
      AL::TType _timeType;
      int magStep;

      virtual void drawBackground(QPainter* painter, const QRectF& rect);

      int y2pitch(int y) const;
      AL::Pos pix2pos(int x) const;
      int pos2pix(const AL::Pos& p) const;

   protected:
      virtual void wheelEvent(QWheelEvent* event);
      virtual void mouseMoveEvent(QMouseEvent* event);
      virtual void leaveEvent(QEvent*);

   signals:
      void magChanged(double, double);
      void xposChanged(int);
      void pitchChanged(int);
      void posChanged(const AL::Pos&);

   public slots:
      void moveLocator(int);

   public:
      PianoView();
      void setStaff(Staff*, AL::Pos* locator);
      void ensureVisible(int tick);
      };


#endif
