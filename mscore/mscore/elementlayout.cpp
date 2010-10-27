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
#include "xml.h"
#include "element.h"
#include "text.h"

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
      double h = 0.0;
      double w = 0.0;
      if (e->parent()) {
            o += QPointF(_reloff.x() * e->parent()->width() * 0.01, _reloff.y() * e->parent()->height() * 0.01);
//            h = e->parent()->height();
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
//   writeProperties
//---------------------------------------------------------

void ElementLayout::writeProperties(Xml& xml) const
      {
      if (_align & ALIGN_HCENTER)
            xml.tag("halign", "center");
      else if (_align & ALIGN_RIGHT)
            xml.tag("halign", "right");
      else
            xml.tag("halign", "left");
      if (_align & ALIGN_BOTTOM)
            xml.tag("valign", "bottom");
      else if (_align & ALIGN_VCENTER)
            xml.tag("valign", "center");
      else if (_align & ALIGN_BASELINE)
            xml.tag("valign", "baseline");
      else
            xml.tag("valign", "top");

      if (_xoff != 0.0 || _yoff != 0.0) {
            if (offsetType() == OFFSET_ABS) {
                  xml.tag("xoffset", xoff() * INCH);
                  xml.tag("yoffset", yoff() * INCH);
                  }
            else {
                  xml.tag("xoffset", xoff());
                  xml.tag("yoffset", yoff());
                  }
            }
      if (_reloff.x() != 0.0)
            xml.tag("rxoffset", _reloff.x());
      if (_reloff.y() != 0.0)
            xml.tag("ryoffset", _reloff.y());

      const char* p = 0;
      switch(_offsetType) {
            case OFFSET_SPATIUM: p = "spatium"; break;
            case OFFSET_ABS:     p = "absolute"; break;
            }
      xml.tag("offsetType", p);
      }

//---------------------------------------------------------
//   readProperties
//---------------------------------------------------------

bool ElementLayout::readProperties(QDomElement e)
      {
      QString tag(e.tagName());
      QString val(e.text());
      int i = val.toInt();

      if (tag == "halign") {
            _align &= ~(ALIGN_HCENTER | ALIGN_RIGHT);
            if (val == "center")
                  _align |= ALIGN_HCENTER;
            else if (val == "right")
                  _align |= ALIGN_RIGHT;
            else if (val == "left")
                  ;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "valign") {
            _align &= ~(ALIGN_VCENTER | ALIGN_BOTTOM | ALIGN_BASELINE);
            if (val == "center")
                  _align |= ALIGN_VCENTER;
            else if (val == "bottom")
                  _align |= ALIGN_BOTTOM;
            else if (val == "baseline")
                  _align |= ALIGN_BASELINE;
            else if (val == "top")
                  ;
            else
                  printf("Text::readProperties: unknown alignment: <%s>\n", qPrintable(val));
            }
      else if (tag == "xoffset")
            setXoff(val.toDouble());
      else if (tag == "yoffset")
            setYoff(val.toDouble());
      else if (tag == "rxoffset")
            setRxoff(val.toDouble());
      else if (tag == "ryoffset")
            setRyoff(val.toDouble());
      else if (tag == "offsetType")
            setOffsetType((OffsetType)i);
      else
            return false;
      return true;
      }


