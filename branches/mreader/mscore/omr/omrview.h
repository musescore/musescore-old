//=============================================================================
//  MusE Reader
//  Linux Music Score Reader
//  $Id$
//
//  Copyright (C) 2010 Werner Schweer
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

#ifndef __SCAN_VIEW_H__
#define __SCAN_VIEW_H__

class Omr;

//---------------------------------------------------------
//   OmrView
//---------------------------------------------------------

class OmrView : public QWidget {
      Q_OBJECT
      Omr* _omr;
      QPixmap pm[4];    // tiled because of max size restrictions
      QPoint startDrag;

      QTransform _matrix, imatrix;
      int curPage;

      void zoom(int step, const QPoint& pos);

      virtual void mousePressEvent(QMouseEvent*);
      virtual void mouseMoveEvent(QMouseEvent*);
      virtual void wheelEvent(QWheelEvent*);
      virtual void paintEvent(QPaintEvent*);

      qreal mag() const { return _matrix.m11(); }
      void setMag(double mag);

   public slots:
      void gotoPage(int);
      void setScale(double);
      void setOffset(double, double);
      void nextPage();
      void previousPage();

   signals:
      void pageNumberChanged(int);
      void xPosChanged(int);
      void yPosChanged(int);

   public:
      OmrView(QWidget* parent = 0);
      void setOmr(Omr*);
      Omr* omr() const { return _omr; }
      };


#endif

