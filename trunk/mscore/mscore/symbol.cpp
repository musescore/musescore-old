//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: element.cpp,v 1.79 2006/04/12 14:58:10 wschweer Exp $
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

#include "symbol.h"
#include "sym.h"
#include "xml.h"

//---------------------------------------------------------
//   Symbol
//---------------------------------------------------------

Symbol::Symbol(Score* s)
   : BSymbol(s)
      {
      setAnchor(ANCHOR_PAGE);
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
//   Symbol::draw
//---------------------------------------------------------

void Symbol::draw(QPainter& p)
      {
      symbols[_sym].draw(p);
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Symbol::bbox() const
      {
      return symbols[_sym].bbox();
      }

//---------------------------------------------------------
//   Symbol::write
//---------------------------------------------------------

void Symbol::write(Xml& xml) const
      {
      xml.stag("Symbol");
      xml.tag("name", symbols[_sym].name());
//      xml.tag("x", pos().x());
//      xml.tag("y", pos().y());
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
            else if (Element::readProperties(e))
                  ;
//            else if (tag == "x")
//                  pos.setX(val.toDouble());
//            else if (tag == "y")
//                  pos.setY(val.toDouble());
            else
                  domError(e);
            }
      if (s == -1) {
            printf("unknown symbol\n");
            s = 0;
            }
      setPos(pos);
      setSym(s);
      }


