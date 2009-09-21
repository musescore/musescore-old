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

#include "ruler.h"

static const int MAP_OFFSET = 0;

//---------------------------------------------------------
//   Ruler
//---------------------------------------------------------

Ruler::Ruler(Score* s, QWidget* parent)
   : QWidget(parent), _score(s), _cursor(s, 480*3)
      {
      _showCursor = false;
      metronomeRulerMag = 0;
      _xpos = 0;
      _xmag = 0.1;
      _timeType = TICKS;
      _font2.setPixelSize(14);
      _font2.setBold(true);
      _font1.setPixelSize(10);
      }

//---------------------------------------------------------
//   setXmag
//---------------------------------------------------------

void Ruler::setXmag(double val)
      {
      _xmag = val;
      update();
      }

//---------------------------------------------------------
//   setXpos
//---------------------------------------------------------

void Ruler::setXpos(int val)
      {
      _xpos = val;
      update();
      }

//---------------------------------------------------------
//   pix2pos
//---------------------------------------------------------

Pos Ruler::pix2pos(int x) const
      {
      int val = lrint((x + _xpos - MAP_OFFSET)/_xmag);
      if (val < 0)
            val = 0;
      return Pos(_score, val, _timeType);
      }

//---------------------------------------------------------
//   pos2pix
//---------------------------------------------------------

int Ruler::pos2pix(const Pos& p) const
      {
      return lrint(p.time(_timeType) * _xmag) + MAP_OFFSET - _xpos;
      }

//---------------------------------------------------------
//   paintEvent
//---------------------------------------------------------

void Ruler::paintEvent(QPaintEvent* e)
      {
      QPainter p(this);
      const QRect& r = e->rect();

      static const int mag[7] = {
            1, 1, 2, 5, 10, 20, 50
            };

      int x  = r.x();
      int w  = r.width();
      int y  = rulerHeight - 16;
      int h  = 14;
      int y1 = r.y();
      int rh = r.height();
      if (y1 < rulerHeight) {
            rh -= rulerHeight - y1;
            y1 = rulerHeight;
            }
      int y2 = y1 + rh;

      if (x < (MAP_OFFSET - _xpos))
            x = MAP_OFFSET - _xpos;

      Pos pos1 = pix2pos(x);
      Pos pos2 = pix2pos(x+w);

      //---------------------------------------------------
      //    draw raster
      //---------------------------------------------------

      int bar1, bar2, beat, tick;

      pos1.mbt(&bar1, &beat, &tick);
      pos2.mbt(&bar2, &beat, &tick);

      int n = mag[metronomeRulerMag];

      bar1 = (bar1 / n) * n;        // round down
      if (bar1 && n >= 2)
            bar1 -= 1;
      bar2 = ((bar2 + n - 1) / n) * n; // round up

      for (int bar = bar1; bar <= bar2;) {
            Pos stick(_score, bar, 0, 0);
            if (metronomeRulerMag) {
                  p.setFont(_font2);
                  int x = pos2pix(stick);
                  QString s;
                  s.setNum(bar + 1);

                  p.setPen(Qt::black);
                  p.drawLine(x, y, x, y + h);
                  QRect r = QRect(x+2, y, 1000, h);
                  p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, s);
                  p.setPen(Qt::lightGray);
                  if (x > 0)
                        p.drawLine(x, y1, x, y2);
                  }
            else {
                  SigEvent sig = stick.timesig();
                  int z = sig.nominator;
                  for (int beat = 0; beat < z; beat++) {
                        Pos xx(_score, bar, beat, 0);
                        int xp = pos2pix(xx);
                        if (xp < 0)
                              continue;
                        QString s;
                        QRect r(xp+2, y + 1, 1000, h);
                        int y3;
                        int num;
                        if (beat == 0) {
                              num = bar + 1;
                              y3  = y + 2;
                              p.setFont(_font2);
                              }
                        else {
                              num = beat + 1;
                              y3  = y + 8;
                              p.setFont(_font1);
                              r.moveTop(r.top() + 1);
                              }
                        s.setNum(num);
                        p.setPen(Qt::black);
                        p.drawLine(xp, y3, xp, y+h);
                        p.drawText(r, Qt::AlignLeft | Qt::AlignVCenter, s);
                        p.setPen(beat == 0 ? Qt::lightGray : Qt::gray);
                        if (xp > 0)
                              p.drawLine(xp, y1, xp, y2);
                        }
                  }
            if (bar == 0 && n >= 2)
                  bar += (n-1);
            else
                  bar += n;
            }
      //
      //  draw mouse cursor marker
      //
      p.setPen(Qt::black);
      if (_showCursor) {
            int xp = pos2pix(_cursor);
            if (xp >= x && xp < x+w)
                  p.drawLine(xp, 0, xp, rulerHeight-1);
            }
      }

//---------------------------------------------------------
//   mousePressEvent
//---------------------------------------------------------

void Ruler::mousePressEvent(QMouseEvent*)
      {
      }

//---------------------------------------------------------
//   mouseReleaseEvent
//---------------------------------------------------------

void Ruler::mouseReleaseEvent(QMouseEvent*)
      {
      }

