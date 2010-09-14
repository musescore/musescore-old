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
#include "scoreview.h"

//---------------------------------------------------------
//   Lasso
//---------------------------------------------------------

Lasso::Lasso(Score* s)
   : Element(s)
      {
      setVisible(false);
      view = 0;
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
      p.drawRect(_rect);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Lasso::editDrag(int curGrip, const QPointF& delta)
      {
      switch(curGrip) {
            case 0:
                  _rect.setTopLeft(_rect.topLeft() + delta);
                  break;
            case 1:
                  _rect.setTopRight(_rect.topRight() + delta);
                  break;
            case 2:
                  _rect.setBottomRight(_rect.bottomRight() + delta);
                  break;
            case 3:
                  _rect.setBottomLeft(_rect.bottomLeft() + delta);
                  break;
            case 4:
                  _rect.setTop(_rect.top() + delta.y());
                  break;
            case 5:
                  _rect.setRight(_rect.right() + delta.x());
                  break;
            case 6:
                  _rect.setBottom(_rect.bottom() + delta.y());
                  break;
            case 7:
                  _rect.setLeft(_rect.left() + delta.x());
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Lasso::updateGrips(int* n, QRectF* r) const
      {
      *n = 8;
      r[0].translate(_rect.topLeft());
      r[1].translate(_rect.topRight());
      r[2].translate(_rect.bottomRight());
      r[3].translate(_rect.bottomLeft());
      r[4].translate(_rect.x() + _rect.width() * .5, _rect.top());
      r[5].translate(_rect.right(), _rect.y() + _rect.height() * .5);
      r[6].translate(_rect.x() + _rect.width()*.5, _rect.bottom());
      r[7].translate(_rect.left(), _rect.y() + _rect.height() * .5);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Lasso::bbox() const
      {
      QRectF bb(_rect);
      if (view) {
            double dx = 1.5 / view->matrix().m11();
            double dy = 1.5 / view->matrix().m22();
            for (int i = 0; i < view->gripCount(); ++i)
                  bb |= view->getGrip(i).adjusted(-dx, -dy, dx, dy);
            }
      return bb;
      }

//---------------------------------------------------------
//   startEdit
//---------------------------------------------------------

void Lasso::startEdit(ScoreView* sv, const QPointF&)
      {
      view = sv;
      }

//---------------------------------------------------------
//   endEdit
//---------------------------------------------------------

void Lasso::endEdit()
      {
      view = 0;
      }

