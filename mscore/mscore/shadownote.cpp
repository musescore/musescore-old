//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
//
//  Copyright (C) 2010 Werner Schweer and others
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

#include "shadownote.h"
#include "score.h"
#include "drumset.h"
#include "preferences.h"
#include "sym.h"

//---------------------------------------------------------
//   ShadowNote
//---------------------------------------------------------

ShadowNote::ShadowNote(Score* s)
   : Element(s)
      {
      _line = 1000;
      sym   = 0;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void ShadowNote::draw(QPainter& p, ScoreView*) const
      {
      if (!visible() || sym == 0)
            return;

      QPointF ap(canvasPos());
      QRect r(abbox().toRect());

      p.translate(ap);
      qreal lw = point(score()->styleS(ST_ledgerLineWidth));
      InputState ps = score()->inputState();
      int voice;
      if (ps.drumNote() != -1 && ps.drumset())
            voice = ps.drumset()->voice(ps.drumNote());
      else
            voice = ps.voice();

      QPen pen(preferences.selectColor[voice].light(140));  // was 160
      pen.setWidthF(lw);
      p.setPen(pen);

      sym->draw(p, magS());

      double ms = spatium();

      double x1 = sym->width(magS())*.5 - ms;
      double x2 = x1 + 2 * ms;

      ms *= .5;
      if (_line < 100 && _line > -100) {
            for (int i = -2; i >= _line; i -= 2) {
                  double y = ms * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            for (int i = 10; i <= _line; i += 2) {
                  double y = ms * (i - _line);
                  p.drawLine(QLineF(x1, y, x2, y));
                  }
            }
      p.translate(-ap);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF ShadowNote::bbox() const
      {
      if (sym == 0)
            return QRectF();
      QRectF b = sym->bbox(magS());
      double _spatium = spatium();
      double x  = b.width()/2 - _spatium;
      double lw = point(score()->styleS(ST_ledgerLineWidth));

      if (_line < 100 && _line > -100) {
            QRectF r(0, -lw/2.0, 2 * _spatium, lw);
            for (int i = -2; i >= _line; i -= 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            for (int i = 10; i <= _line; i += 2)
                  b |= r.translated(QPointF(x, _spatium * .5 * (i - _line)));
            }
      return b;
      }

