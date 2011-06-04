//=============================================================================
//  MusE Score
//  Linux Music Score Editor
//  $Id: text.h 3555 2010-10-06 11:15:52Z wschweer $
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

#include "elementlayout.h"
#include "element.h"
#include "text.h"
#include "al/xml.h"

//---------------------------------------------------------
//   ElementLayout
//---------------------------------------------------------

ElementLayout::ElementLayout()
      {
      _align      = ALIGN_LEFT | ALIGN_BASELINE;
      _xoff       = 0.0;
      _yoff       = 0.0;
      _offsetType = OFFSET_SPATIUM;
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void ElementLayout::layout(Element* e) const
      {
      QPointF o(_xoff, _yoff);
      if (_offsetType == OFFSET_SPATIUM)
            o *= e->spatium();
      else
            o *= DPI;
      qreal h = 0.0;
      qreal w = 0.0;
      if (e->parent()) {
            qreal pw, ph;
            if ((e->type() == MARKER || e->type() == JUMP) && e->parent()->parent()) {
                  pw = e->parent()->parent()->width();      // measure width
                  ph = e->parent()->parent()->height();
                  }
            else {
                  pw = e->parent()->width();
                  ph = e->parent()->height();
                  }
            o += QPointF(_reloff.x() * pw * 0.01, _reloff.y() * ph * 0.01);
            }
      bool frameText = e->type() == TEXT && static_cast<Text*>(e)->layoutToParentWidth() && e->parent();
      QPointF p;
      if (frameText)
            h = e->parent()->height();
      else
            w = e->width();
      if (_align & ALIGN_BOTTOM)
            p.setY(h - e->height());
      else if (_align & ALIGN_VCENTER)
            p.setY((h - e->height()) * .5);
      else if (_align & ALIGN_BASELINE)
            p.setY(-e->baseLine());
      if (!frameText) {
            if (_align & ALIGN_RIGHT)
                  p.setX(-w);
            else if (_align & ALIGN_HCENTER)
                  p.setX(-(w * .5));
            }
      e->setPos(p + o);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ElementLayout::readProperties(XmlReader* r)
      {
      MString8 tag = r->tag();
      QString val;
      int i;
      qreal d;

      if (r->readString("halign", &val)) {
            _align &= ~(ALIGN_HCENTER | ALIGN_RIGHT);
            if (val == "center")
                  _align |= ALIGN_HCENTER;
            else if (val == "right")
                  _align |= ALIGN_RIGHT;
            else if (val == "left") {
                  ;
                  }
            }
      else if (r->readString("valign", &val)) {
            _align &= ~(ALIGN_VCENTER | ALIGN_BOTTOM | ALIGN_BASELINE);
            if (val == "center")
                  _align |= ALIGN_VCENTER;
            else if (val == "bottom")
                  _align |= ALIGN_BOTTOM;
            else if (val == "baseline")
                  _align |= ALIGN_BASELINE;
            else if (val == "top") {
                  ;
                  }
            }
      else if (r->readReal("xoffset", &d)) {
            if (offsetType() == OFFSET_ABS)
                  d /= INCH;
            setXoff(d);
            }
      else if (r->readReal("yoffset", &d)) {
            if (offsetType() == OFFSET_ABS)
                  d /= INCH;
            setYoff(d);
            }
      else if (r->readReal("rxoffset", &d))
            setRxoff(d);
      else if (r->readReal("ryoffset", &d))
            setRyoff(val.toDouble());
      else if (r->readInt("offsetType", &i)) {
            OffsetType ot = (OffsetType)i;
            if (ot != offsetType()) {
                  setOffsetType(ot);
                  if (ot == OFFSET_ABS) {
                        _xoff /= INCH;
                        _yoff /= INCH;
                        }
                  else {
                        _xoff *= INCH;
                        _yoff *= INCH;
                        }
                  }
            }
      else
            return false;
      return true;
      }


