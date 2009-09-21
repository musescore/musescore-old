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

#include "pianoscene.h"

//---------------------------------------------------------
//   PianoScene
//---------------------------------------------------------

PianoScene::PianoScene(Staff* s, QWidget* parent)
   : QGraphicsScene(parent)
      {
      staff = s;
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoScene::drawBackground(QPainter* p, const QRectF& r)
      {
      p->fillRect(r, QColor(0x71, 0x8d, 0xbe));
//      printf("PianoScene::draw: %f %f   %f %f\n", r.x(), r.y(), r.width(), r.height());
      qreal y1 = r.y();
      qreal y2 = y1 + r.height();
      qreal kh = 13.0;
      qreal x1 = r.x();
      qreal x2 = x1 + r.width();

      int key = floor(y1 / 75);
      qreal y = key * kh;

      for (; key < 75; ++key, y += kh) {
            if (y < y1)
                  continue;
            if (y > y2)
                  break;
            p->setPen(QPen((key % 7) == 5 ? Qt::lightGray : Qt::gray));
            p->drawLine(QLineF(x1, y, x2, y));
            }
      }

