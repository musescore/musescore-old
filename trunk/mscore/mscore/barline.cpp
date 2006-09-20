//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: barline.cpp,v 1.3 2006/04/12 14:58:10 wschweer Exp $
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

#include "xml.h"
#include "barline.h"
#include "preferences.h"
#include "painter.h"
#include "style.h"
#include "utils.h"
#include "score.h"

//---------------------------------------------------------
//   BarLine
//---------------------------------------------------------

BarLine::BarLine(Score* s)
   : Element(s)
      {
      setSubtype(NORMAL_BAR);
      setWidth(point(style->barWidth));
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void BarLine::draw1(Painter& p) const
      {
      qreal lw    = point(style->barWidth);
      qreal h     = height();

      QColor color(selected() ? preferences.selectColor[0] : Qt::black);

      bool split = height() > (_spatium * 4.01);

      QPen pen(color);
      pen.setWidthF(lw);
      pen.setCapStyle(Qt::FlatCap);
      p.setPen(pen);

      switch(subtype()) {
            case BROKEN_BAR:
            case NORMAL_BAR:
                  p.drawLine(QLineF(lw*.5, 0.0, lw*.5, h));
                  break;

            case END_BAR:
                  {
                  qreal lw2 = point(style->endBarWidth);

                  p.drawLine(QLineF(lw*.5, 0.0, lw*.5, h));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  qreal x = point(style->endBarDistance) + lw2*.5 + lw;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  }
                  break;

            case DOUBLE_BAR:
                  {
                  qreal lw2 = point(style->doubleBarWidth);
                  qreal d   = point(style->doubleBarDistance);

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  qreal x = lw2/2;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  x += d + lw2;
                  p.drawLine(QLineF(x, 0.0, x, h));
                  }
                  break;

            case START_REPEAT:
                  {
                  qreal lw2  = point(style->endBarWidth);
                  qreal lw22 = point(style->endBarWidth) / 2.0;
                  qreal d1   = point(style->endBarDistance);
                  qreal dotw = dotSym.width();
                  qreal x2   =  dotw/2;
                  qreal x1   =  dotw + d1 + lw/2;
                  qreal x0   =  dotw + d1 + lw + d1 + lw22;

                  dotSym.draw(p, x0, 1.5 * _spatium);
                  dotSym.draw(p, x0, 2.5 * _spatium);
                  if (split) {
                        dotSym.draw(p, x0, h - 1.5 * _spatium);
                        dotSym.draw(p, x0, h - 2.5 * _spatium);
                        }

                  p.drawLine(QLineF(x1, 0.0, x1, h));
                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, 0.0, x2, h));
                  }
                  break;

            case END_REPEAT:
                  {
                  qreal lw2  = point(style->endBarWidth);
                  qreal lw22 = point(style->endBarWidth) / 2.0;
                  qreal d1   = point(style->endBarDistance);

                  qreal dotw = dotSym.width();
                  qreal x0   =  dotw/2;
                  qreal x1   =  dotw + d1 + lw/2;
                  qreal x2   =  dotw + d1 + lw + d1 + lw22;

                  dotSym.draw(p, x0, 1.5 * _spatium);
                  dotSym.draw(p, x0, 2.5 * _spatium);
                  if (split) {
                        dotSym.draw(p, x0, h - 1.5 * _spatium);
                        dotSym.draw(p, x0, h - 2.5 * _spatium);
                        }

                  p.drawLine(QLineF(x1, 0.0, x1, h));

                  pen.setWidthF(lw2);
                  p.setPen(pen);
                  p.drawLine(QLineF(x2, 0.0, x2, h));
                  }
                  break;

            case INVISIBLE_BAR:
                  break;
            }
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void BarLine::write(Xml& xml) const
      {
      if (subtype() == NORMAL_BAR)
            xml.stag("BarLine");
      else
            xml.stag("BarLine type=\"%d\"", subtype());
      Element::writeProperties(xml);
      xml.etag("BarLine");
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void BarLine::read(QDomNode node)
      {
      QDomElement e = node.toElement();
      setSubtype(e.attribute("type", "0").toInt());

      for (node = node.firstChild(); !node.isNull(); node = node.nextSibling()) {
            if (Element::readProperties(node))
                  ;
            else
                  domError(node);
            }
      }

//---------------------------------------------------------
//   dump
//---------------------------------------------------------

void BarLine::dump() const
      {
      Element::dump();
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void BarLine::setSubtype(int t)
      {
      Element::setSubtype(t);
      Spatium w = style->barWidth;
      qreal dw;

      switch(subtype()) {
            case DOUBLE_BAR:
                  w  = style->doubleBarWidth * 2 + style->doubleBarDistance;
                  dw = point(w);
                  break;
            case START_REPEAT:
            case END_REPEAT:
                  w  += style->endBarWidth + 2 * style->endBarDistance;
                  dw = point(w) + dotSym.width();
                  break;
            case END_BAR:
                  w += style->endBarWidth + style->endBarDistance;
                  dw = point(w);
                  break;
            case BROKEN_BAR:
            case NORMAL_BAR:
            case INVISIBLE_BAR:
                  dw = point(w);
                  break;
            default:
            	dw = 0.0;
            	break;
            }
      setWidth(dw);
      }

//---------------------------------------------------------
//   acceptDrop
//---------------------------------------------------------

bool BarLine::acceptDrop(int type, int) const
      {
      return type == BAR_LINE;
      }

//---------------------------------------------------------
//   drop
//---------------------------------------------------------

void BarLine::drop(const QPointF& /*pos*/, int type, int st)
      {
      if (type != BAR_LINE)
            return;
      if (subtype() == st)
            return;
      score()->cmdRemove(this);

      BarLine* bl = new BarLine(score());
      bl->setSubtype(st);
      bl->setParent(parent());
      bl->setStaff(staff());

      if (subtype() == START_REPEAT) {
            Measure* m = (Measure*)(parent()->parent());
            Measure* pm = m->system()->prevMeasure(m);
            if (pm)
                  bl->setParent(pm);
            }
      if (st == START_REPEAT){
            Measure* m  = (Measure*)(parent());
            Measure* nm = m->system()->nextMeasure(m);
            if (nm)
                  bl->setParent(nm);
            }
      score()->cmdAdd(bl);
      }

