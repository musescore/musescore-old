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
#include "staff.h"

//---------------------------------------------------------
//   PianoScene
//---------------------------------------------------------

PianoScene::PianoScene(Staff* s, QWidget* parent)
   : QGraphicsScene(parent)
      {
      staff = s;
      _score = staff->score();
      _timeType = TICKS;
      metronomeRulerMag = 0;
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

Pos PianoScene::pix2pos(int x) const
      {
      if (x < 0)
            x = 0;
      return Pos(_score, x, _timeType);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int PianoScene::pos2pix(const Pos& p) const
      {
      return lrint(p.time(_timeType));
      }

//---------------------------------------------------------
//   drawBackground
//---------------------------------------------------------

void PianoScene::drawBackground(QPainter* p, const QRectF& r)
      {
      p->fillRect(r, QColor(0x71, 0x8d, 0xbe));

// printf("draw %f %f  %f %f\n", r.x(), r.y(), r.width(), r.height());
      //
      // draw horizontal grid lines
      //
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

      //
      // draw vertical grid lines
      //
      static const int mag[7] = {
            1, 1, 2, 5, 10, 20, 50
            };

      Pos pos1 = pix2pos(x1);
      Pos pos2 = pix2pos(x2);

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      int bar1, bar2, beat, tick;
      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      int n = mag[metronomeRulerMag];

      bar1 = (bar1 / n) * n;           // round down
      if (bar1 && n >= 2)
            bar1 -= 1;
      bar2 = ((bar2 + n - 1) / n) * n; // round up

      for (int bar = bar1; bar <= bar2;) {
            Pos stick(_score, bar, 0, 0);
            if (metronomeRulerMag) {
                  int x = pos2pix(stick);
                  if (x > 0) {
                        p->setPen(Qt::lightGray);
                        p->drawLine(x, y1, x, y2);
                        }
                  else {
                        p->setPen(Qt::black);
                        p->drawLine(x, y1, x, y1);
                        }
                  }
            else {
                  int z = stick.timesig().nominator;
                  for (int beat = 0; beat < z; beat++) {
                        Pos xx(_score, bar, beat, 0);
                        int xp = pos2pix(xx);
                        if (xp < 0)
                              continue;
                        if (xp > 0) {
                              p->setPen(beat == 0 ? Qt::lightGray : Qt::gray);
                              p->drawLine(xp, y1, xp, y2);
                              }
                        else {
                              p->setPen(Qt::black);
                              p->drawLine(xp, y1, xp, y2);
                              }
                        }
                  }
            if (bar == 0 && n >= 2)
                  bar += (n-1);
            else
                  bar += n;
            }
      }

