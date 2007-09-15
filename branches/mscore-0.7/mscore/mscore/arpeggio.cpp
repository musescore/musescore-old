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

#include "arpeggio.h"
#include "sym.h"

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Score* s)
  : Element(s)
      {
      }

//---------------------------------------------------------
//   layout
//---------------------------------------------------------

void Arpeggio::layout(ScoreLayout*)
      {
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Arpeggio::write(Xml& xml) const
      {
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(double h)
      {
      _height = h;
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Arpeggio::bbox() const
      {
      QRectF b = symbols[arpeggioSym].bbox();
//      printf("====%f %f %f %f\n", b.x(), b.y(), b.height(), b.width());
      return QRectF(0.0, 0.0, symbols[arpeggioSym].width(), _height);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(QPainter& p)
      {
      double y;

      double h1, h2, h3;

      h1 = h2 = h3 = _spatium;

      switch(subtype()) {
            case 0:
                  for (y = h1; y < _height+h1; y += h1)
                        symbols[arpeggioSym].draw(p, 0.0, y);
                  break;
            case 1:
                  symbols[arpeggioarrowupSym].draw(p, 0.0, h2);
                  for (y = h2 + h1; y < _height + h1; y += h1)
                        symbols[arpeggioSym].draw(p, 0.0, y);
                  break;
            case 2:
                  for (y = h1; y < _height + h1 - h3; y += h1)
                        symbols[arpeggioSym].draw(p, 0.0, y);
                  symbols[arpeggioarrowdownSym].draw(p, 0.0, y);
                  break;
            }
      }
