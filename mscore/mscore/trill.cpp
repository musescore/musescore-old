//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
//
//  Copyright (C) 2002-2007 Werner Schweer and others
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

#include "trill.h"
#include "style.h"
#include "system.h"
#include "measure.h"
#include "xml.h"
#include "utils.h"
#include "sym.h"
#include "score.h"

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void TrillSegment::draw(QPainter& p, ScoreView*) const
      {
      double mags = magS();
      QRectF b1 = symbols[trillSym].bbox(mags);
      QRectF b2 = symbols[trillelementSym].bbox(mags);
      qreal w2  = symbols[trillelementSym].width(mags);
      int n     = lrint((pos2().x() - (b1.width() - b2.x())) / w2);

      symbols[trillSym].draw(p, mags, -b1.x(), 0);
      symbols[trillelementSym].draw(p, mags, b1.width() - b2.x(), b2.y(), n);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF TrillSegment::bbox() const
      {
      QRectF rr(symbols[trillSym].bbox(magS()));
      QRectF r(0.0, rr.y(), pos2().x(), rr.height());
      return r;
      }

//---------------------------------------------------------
//   Trill
//---------------------------------------------------------

Trill::Trill(Score* s)
  : SLine(s)
      {
      setLen(spatium() * 7);   // for use in palettes
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Trill::layout()
      {
      SLine::layout();
      qreal y = -2.0 * spatium();
      setPos(ipos().x(), y);
      }

//---------------------------------------------------------
//   createLineSegment
//---------------------------------------------------------

LineSegment* Trill::createLineSegment()
      {
      TrillSegment* seg = new TrillSegment(score());
      seg->setTrack(track());
      return seg;
      }


