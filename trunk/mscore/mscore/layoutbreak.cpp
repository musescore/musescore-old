//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.cpp,v 1.1 2006/03/27 14:16:24 wschweer Exp $
//
//  Copyright (C) 2002-2006 Werner Schweer (ws@seh.de)
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

#include "layoutbreak.h"
#include "painter.h"
#include "preferences.h"
#include "score.h"
#include "canvas.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      layout();
      }

//---------------------------------------------------------
//   draw1
//---------------------------------------------------------

void LayoutBreak::draw1(Painter& p)
      {
      QPen pen;
      if (selected())
            pen.setColor(preferences.selectColor[0]);
      else
            pen.setColor(preferences.layoutBreakColor);

      qreal lw = _spatium * 0.3;
      pen.setWidthF(lw);
      p.setPen(pen);
      p.setBrush(Qt::NoBrush);
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LayoutBreak::layout()
      {
      path = QPainterPath();

      double h = _spatium * 4;
      double w = _spatium * 2.5;
      double w1 = w * .6;

      switch(subtype()) {
            case LAYOUT_BREAK_LINE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);

                  path.moveTo(w * .8, w * .7);
                  path.lineTo(w * .8, w);
                  path.lineTo(w * .2, w);

                  path.moveTo(w * .4, w * .8);
                  path.lineTo(w * .2, w);
                  path.lineTo(w * .4, w * 1.2);
                  break;

            case LAYOUT_BREAK_PAGE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h-w1);
                  path.lineTo(w1, h-w1);
                  path.lineTo(w1, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w, h-w1);
                  path.lineTo(w1, h);
                  break;
            default:
                  printf("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      qreal lw = _spatium * 0.3;
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void LayoutBreak::setSubtype(const QString& s)
      {
      if (s == "line")
            setSubtype(LAYOUT_BREAK_LINE);
      else
            setSubtype(LAYOUT_BREAK_PAGE);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString LayoutBreak::subtypeName() const
      {
      switch(subtype()) {
            case LAYOUT_BREAK_LINE:
                  return "line";
            case LAYOUT_BREAK_PAGE:
                  return "page";
            default:
                  return "??";
            }
      }

