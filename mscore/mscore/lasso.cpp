//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: element.h 3424 2010-08-28 14:44:18Z wschweer $
//
//  Copyright (C) 2002-2010 Werner Schweer and others
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

#include "lasso.h"
#include "preferences.h"

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
   : Element(s)
      {
      setVisible(false);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Lasso::draw(QPainter& p, ScoreView*) const
      {
      p.setBrush(QColor(0, 0, 50, 50));
      QPen pen(QColor(preferences.selectColor[0]));
      // always 2 pixel width
      qreal w = 2.0 / p.matrix().m11();
      pen.setWidthF(w);
      p.setPen(pen);
      p.drawRect(bbox());
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(int curGrip, const QPointF& delta)
      {
      switch(curGrip) {
            case 0:
                  _bbox.setTopLeft(_bbox.topLeft() + delta);
                  break;
            case 1:
                  _bbox.setTopRight(_bbox.topRight() + delta);
                  break;
            case 2:
                  _bbox.setBottomRight(_bbox.bottomRight() + delta);
                  break;
            case 3:
                  _bbox.setBottomLeft(_bbox.bottomLeft() + delta);
                  break;
            case 4:
                  _bbox.setTop(_bbox.top() + delta.y());
                  break;
            case 5:
                  _bbox.setRight(_bbox.right() + delta.x());
                  break;
            case 6:
                  _bbox.setBottom(_bbox.bottom() + delta.y());
                  break;
            case 7:
                  _bbox.setLeft(_bbox.left() + delta.x());
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Lasso::updateGrips(int* n, QRectF* r) const
      {
      *n = 8;
      QRectF b(abbox());
      r[0].translate(b.topLeft());
      r[1].translate(b.topRight());
      r[2].translate(b.bottomRight());
      r[3].translate(b.bottomLeft());
      r[4].translate(b.x() + b.width() * .5, b.top());
      r[5].translate(b.right(), b.y() + b.height() * .5);
      r[6].translate(b.x() + b.width()*.5, b.bottom());
      r[7].translate(b.left(), b.y() + b.height() * .5);
      }
