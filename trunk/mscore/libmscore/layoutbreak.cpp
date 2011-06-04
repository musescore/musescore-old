//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: layoutbreak.cpp 3592 2010-10-18 17:24:18Z wschweer $
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

#include "layoutbreak.h"
#include "preferences.h"
#include "score.h"
#include "al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   LayoutBreak
//---------------------------------------------------------

LayoutBreak::LayoutBreak(Score* score)
   : Element(score)
      {
      _pause = score->styleD(ST_SectionPause);
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void LayoutBreak::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (r->readReal("pause", &_pause))
                  ;
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void LayoutBreak::draw(Painter* p) const
      {
      if (score()->printing())
            return;
      if (selected())
            p->setPenColor(preferences.selectColor[0]);
      else
            p->setPenColor(preferences.layoutBreakColor);

      p->setLineCap(Qt::RoundCap);
      p->setLineJoin(Qt::RoundJoin);
//TODO      p->setBrush(Qt::NoBrush);
//      p->drawPath(path);
      p->setPenWidth(lw);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void LayoutBreak::layout()
      {
#if 0
      qreal _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
      qreal w1 = w * .6;

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

            case LAYOUT_BREAK_SECTION:
                  path.lineTo(w, 0.0);
                  path.lineTo(w,  h);
                  path.lineTo(0.0,  h);
                  path.moveTo(w-_spatium * .8,  0.0);
                  path.lineTo(w-_spatium * .8,  h);
                  break;

            default:
                  printf("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);

      if (parent()) {
            setPos(-_spatium - w + parent()->width(), -2 * _spatium - h);
            }
#endif
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void LayoutBreak::setSubtype(const QString& s)
      {
      if (s == "line")
            setSubtype(LAYOUT_BREAK_LINE);
      else if (s == "page")
            setSubtype(LAYOUT_BREAK_PAGE);
      else
            setSubtype(LAYOUT_BREAK_SECTION);
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
            case LAYOUT_BREAK_SECTION:
                  return "section";
            default:
                  return "??";
            }
      }

