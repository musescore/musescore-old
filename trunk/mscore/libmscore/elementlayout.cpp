//=============================================================================
//  MuseScore
//  Music Composition & Notation
//  $Id:$
//
//  Copyright (C) 2010-2011 Werner Schweer
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License version 2
//  as published by the Free Software Foundation and appearing in
//  the file LICENCE.GPL
//=============================================================================

#include "elementlayout.h"
#include "xml.h"
#include "element.h"
#include "text.h"
#include "mscore.h"

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
            double pw, ph;
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
            double x(_xoff);
            double y(_yoff);
            if (offsetType() == OFFSET_ABS) {
                  x *= INCH;
                  y *= INCH;
                  }
            xml.tag("xoffset", x);
            xml.tag("yoffset", y);
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
//      int i = val.toInt();

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
      else if (tag == "xoffset") {
            double xo = val.toDouble();
            if (offsetType() == OFFSET_ABS)
                  xo /= INCH;
            setXoff(xo);
            }
      else if (tag == "yoffset") {
            double yo = val.toDouble();
            if (offsetType() == OFFSET_ABS)
                  yo /= INCH;
            setYoff(yo);
            }
      else if (tag == "rxoffset")
            setRxoff(val.toDouble());
      else if (tag == "ryoffset")
            setRyoff(val.toDouble());
      else if (tag == "offsetType") {
            OffsetType ot = OFFSET_ABS;
            if (val == "spatium" || val == "1")
                  ot = OFFSET_SPATIUM;
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


