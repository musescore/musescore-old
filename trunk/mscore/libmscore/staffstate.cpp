//=============================================================================
//  MusE Score
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

#include "staffstate.h"
#include "preferences.h"
#include "score.h"
#include "segment.h"
#include "staff.h"
#include "part.h"
#include "seq.h"
#include "al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   StaffState
//---------------------------------------------------------

StaffState::StaffState(Score* score)
   : Element(score)
      {
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void StaffState::read(XmlReader* r)
      {
      while (r->readElement()) {
            if (r->tag() == "Instrument")
                  _instrument.read(r);
            else if (!Element::readProperties(r))
                  r->unknown();
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void StaffState::draw(Painter* p) const
      {
      if (score()->printing())
            return;
      if (selected())
            p->setPenColor(preferences.selectColor[0]);
      else
            p->setPenColor(preferences.layoutBreakColor);

      p->setLineCap(Qt::RoundCap);
      p->setLineJoin(Qt::RoundJoin);
      p->setPenWidth(lw);
//      p->setBrush(Qt::NoBrush);
//      p->drawPath(path);
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void StaffState::layout()
      {
#if 0
      qreal _spatium = spatium();
      path      = QPainterPath();
      lw        = _spatium * 0.3;
      qreal h  = _spatium * 4;
      qreal w  = _spatium * 2.5;
//      qreal w1 = w * .6;

      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  path.moveTo(w * .5, h - _spatium * .5);
                  path.lineTo(w * .5, _spatium * 2);
                  path.moveTo(w * .5, _spatium * .8);
                  path.lineTo(w * .5, _spatium * 1.0);
                  break;

            case STAFF_STATE_TYPE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_VISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            case STAFF_STATE_INVISIBLE:
                  path.lineTo(w, 0.0);
                  path.lineTo(w, h);
                  path.lineTo(0.0, h);
                  path.lineTo(0.0, 0.0);
                  break;

            default:
                  printf("unknown layout break symbol\n");
                  break;
            }
      QRectF bb(0, 0, w, h);
      bb.adjust(-lw, -lw, lw, lw);
      setbbox(bb);
      setPos(0.0, _spatium * -6.0);
#endif
      }

//---------------------------------------------------------
//   setSubtype
//---------------------------------------------------------

void StaffState::setSubtype(const QString& s)
      {
      if (s == "instrument")
            setSubtype(STAFF_STATE_INSTRUMENT);
      else if (s == "type")
            setSubtype(STAFF_STATE_TYPE);
      else if (s == "visible")
            setSubtype(STAFF_STATE_VISIBLE);
      else if (s == "invisible")
            setSubtype(STAFF_STATE_INVISIBLE);
      }

//---------------------------------------------------------
//   subtypeName
//---------------------------------------------------------

const QString StaffState::subtypeName() const
      {
      switch(subtype()) {
            case STAFF_STATE_INSTRUMENT:
                  return "instrument";
            case STAFF_STATE_TYPE:
                  return "type";
            case STAFF_STATE_VISIBLE:
                  return "visible";
            case STAFF_STATE_INVISIBLE:
                  return "invisible";
            default:
                  return "??";
            }
      }

