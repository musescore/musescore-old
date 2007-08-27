//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id:$
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

#include "repeat.h"
#include "layout.h"

//---------------------------------------------------------
//   RepeatMeasure
//---------------------------------------------------------

RepeatMeasure::RepeatMeasure(Score* score)
   : Element(score)
      {
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void RepeatMeasure::draw(QPainter& p)
      {
      p.setBrush(p.pen().color());
      p.drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void RepeatMeasure::layout(ScoreLayout* layout)
      {
      double sp  = layout->spatium();

      double w   = sp * 2.0;
      double h   = sp * 2.0;
      double lw  = sp * .30;  // line width
      double r   = sp * .15;  // dot radius

      path       = QPainterPath();

      path.moveTo(w - lw, 0.0);
      path.lineTo(w,  0.0);
      path.lineTo(lw,  h);
      path.lineTo(0.0, h);
      path.closeSubpath();
      path.addEllipse(QRectF(w * .25 - r, h * .25 - r, r * 2.0, r * 2.0 ));
      path.addEllipse(QRectF(w * .75 - r, h * .75 - r, r * 2.0, r * 2.0 ));

      setbbox(path.boundingRect());
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void RepeatMeasure::write(Xml& xml) const
      {
      xml.stag(name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void RepeatMeasure::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }


