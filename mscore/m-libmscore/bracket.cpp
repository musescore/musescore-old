//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: bracket.cpp 3537 2010-10-01 10:52:51Z wschweer $
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

#include <math.h>

#include "bracket.h"
#include "style.h"
#include "preferences.h"
#include "utils.h"
#include "staff.h"
#include "score.h"
#include "system.h"
#include "sym.h"
#include "m-al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   Bracket
//---------------------------------------------------------

Bracket::Bracket(Score* s)
   : Element(s)
      {
      h2       = 3.5 * spatium();
      _span    = 1;
      _column   = 0;
      yoff     = 0.0;
      setGenerated(true);     // brackets are not saved
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Bracket::setHeight(qreal h)
      {
      h2 = h * .5;
      }

//---------------------------------------------------------
//   width
//---------------------------------------------------------

qreal Bracket::width() const
      {
      qreal w;
      if (subtype() == BRACKET_AKKOLADE)
            w = point(score()->styleS(ST_akkoladeWidth));
      else
            w = point(score()->styleS(ST_bracketWidth) + score()->styleS(ST_bracketDistance));
      return w;
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Bracket::layout()
      {
      //qreal _spatium = spatium();
      path = PainterPath();
      if (h2 == 0.0)
            return;

      qreal h = h2 + yoff * .5;

      if (subtype() == BRACKET_AKKOLADE) {
            qreal w         = point(score()->styleS(ST_akkoladeWidth));
            const qreal X1 =  2.0 * w;
            const qreal X2 = -0.7096 * w;
            const qreal X3 = -1.234 * w;
            const qreal X4 =  1.734 * w;

            path.moveTo(0, h);
            path.cubicTo(X1,  h + h * .3359, X2,  h + h * .5089, w, 2 * h);
            path.cubicTo(X3,  h + h * .5025, X4,  h + h * .2413, 0, h);
            path.cubicTo(X1,  h - h * .3359, X2,  h - h * .5089, w, 0);
            path.cubicTo(X3,  h - h * .5025, X4,  h - h * .2413, 0, h);
            setbbox(QRectF(0, 0, w, 2 * h));
            }
      else if (subtype() == BRACKET_NORMAL) {
#if 0
            qreal w = point(score()->styleS(ST_bracketWidth));

            QChar up   = symbols[score()->symIdx()][brackettipsRightUp].code();
            QChar down = symbols[score()->symIdx()][brackettipsRightDown].code();

            Font ff(symbols[score()->symIdx()][brackettipsRightUp].font());
            Font f(ff.family());
            f.setPixelSize(lrint(2.0 * _spatium));

            qreal o   = _spatium * .17;
            qreal slw = point(score()->styleS(ST_staffLineWidth));

            path.setFillRule(Qt::WindingFill);

            path.addText(QPointF(0.0, -o), f,          QString(up));
            path.addText(QPointF(0.0, h * 2.0 + o), f, QString(down));
            path.addRect(0.0, -slw * .5, w, h * 2.0 + slw);
#endif
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Bracket::draw(Painter* p) const
      {
//      p->setBrush(p->pen().color());
      p->setPenWidth(spatium() * .15);
      p->drawPath(path);
      }

//---------------------------------------------------------
//   Bracket::read
//---------------------------------------------------------

void Bracket::read(XmlReader* r)
      {
      while (r->readAttribute()) {
            if (r->tag() == "type") {
                  QString t = r->stringValue();
                  if (t == "Normal")
                        setSubtype(BRACKET_NORMAL);
                  else if (t == "Akkolade")
                        setSubtype(BRACKET_AKKOLADE);
                  }
            }
      while (r->readElement()) {
            if (r->readInt("level", &_column))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

