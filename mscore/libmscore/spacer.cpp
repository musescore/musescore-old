//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: spacer.cpp 3442 2010-09-07 09:46:33Z wschweer $
//
//  Copyright (C) 2008 Werner Schweer and others
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

#include "spacer.h"
#include "preferences.h"
#include "score.h"
#include "al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

Spacer::Spacer(Score* score)
   : Element(score)
      {
      _space = Spatium(0);
      }

Spacer::Spacer(const Spacer& s)
   : Element(s)
      {
      _space = s._space;
//      path   = s.path;
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Spacer::draw(Painter* p) const
      {
      if (score()->printing())
            return;
      if (selected())
            p->setPenColor(preferences.selectColor[0]);
      else
            p->setPenColor(preferences.layoutBreakColor);

      p->setNoBrush(true);
      p->setPenWidth(spatium() * 0.4);
//      p->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Spacer::layout()
      {
#if 0
      qreal _spatium = spatium();

      path     = QPainterPath();
      qreal w = _spatium;
      qreal b = w * .5;
      qreal h = _space.val() * _spatium;

      if (subtype() == SPACER_DOWN) {
            path.lineTo(w, 0.0);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.lineTo(0.0, h-b);
            path.moveTo(b, h);
            path.lineTo(w, h-b);
            }
      else if (subtype() == SPACER_UP) {
            path.moveTo(b, 0.0);
            path.lineTo(0.0, b);
            path.moveTo(b, 0.0);
            path.lineTo(w, b);
            path.moveTo(b, 0.0);
            path.lineTo(b, h);
            path.moveTo(0.0, h);
            path.lineTo(w, h);
            }
      qreal lw = spatium() * 0.4;
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
#endif
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Spacer::read(XmlReader* r)
      {
      while (r->readElement()) {
            qreal d;
            if (r->readReal("space", &d))
                  _space = Spatium(d);
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }
