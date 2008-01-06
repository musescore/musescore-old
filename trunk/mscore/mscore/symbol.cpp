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

#include "symbol.h"
#include "sym.h"
#include "xml.h"
#include "system.h"
#include "staff.h"
#include "measure.h"
#include "layout.h"

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(Score* s)
   : BSymbol(s)
      {
      setAnchor(ANCHOR_PARENT);
      _sym = 0;
      }

//---------------------------------------------------------
//   setSym
//---------------------------------------------------------

void Symbol::setSym(int s)
      {
      _sym  = s;
      }

//---------------------------------------------------------
//   setAbove
//---------------------------------------------------------

void Symbol::setAbove(bool val)
      {
      setYoff(val ? -2.0 : 7.0);
      }

//---------------------------------------------------------
//   layout
//    height() and width() should return sensible
//    values when calling this method
//---------------------------------------------------------

void Symbol::layout(ScoreLayout* layout)
      {
      QPointF o(QPointF(_xoff, _yoff));
      if (_offsetType == OFFSET_SPATIUM)
            o *= layout->spatium();
      else
            o *= DPI;
      if (parent())
            o += QPointF(_rxoff * parent()->width() * 0.01, _ryoff * parent()->height() * 0.01);
      double h = height();
      double w = width();
      QPointF p;
      if (_align & ALIGN_BOTTOM)
            p.setY(-h);
      else if (_align & ALIGN_VCENTER)
            p.setY(-(h * .5));
      else if (_align & ALIGN_BASELINE)
            p.setY(0.0);
      if (_align & ALIGN_RIGHT)
            p.setX(-w);
      else if (_align & ALIGN_HCENTER)
            p.setX(-(w * .5));
      setPos(p + o);
      }

//---------------------------------------------------------
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(QPainter& p) const
      {
      symbols[_sym].draw(p, mag());
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Symbol::bbox() const
      {
      return symbols[_sym].bbox(mag());
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag("Symbol");
      xml.tag("name", symbols[_sym].name());
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   Symbol::read
//---------------------------------------------------------

void Symbol::read(QDomElement e)
      {
      QPointF pos;
      int s = -1;

      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            QString val(e.text());
            if (tag == "name") {
                  for (int i = 0; i < symbols.size(); ++i) {
                        if (val == symbols[i].name()) {
                              s = i;
                              break;
                              }
                        }
                  if (s == -1) {
                        printf("unknown symbol <%s>, symbols %d\n",
                           val.toLocal8Bit().data(), symbols.size());
                        s = 0;
                        }
                  }
            else if (!Element::readProperties(e))
                  domError(e);
            }
      if (s == -1) {
            printf("unknown symbol\n");
            s = 0;
            }
      setPos(pos);
      setSym(s);
      }

