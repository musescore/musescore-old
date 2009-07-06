//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id$
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
      setHeight(spatium() * 4);      // for use in palettes
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(double h)
      {
      _height = h;
      }

//---------------------------------------------------------
//   write
//---------------------------------------------------------

void Arpeggio::write(Xml& xml) const
      {
      xml.stag("Arpeggio");
      Element::writeProperties(xml);
      if (_userLen1.val() != 0.0)
            xml.tag("userLen1", _userLen1);
      if (_userLen2.val() != 0.0)
            xml.tag("userLen2", _userLen2);
      xml.etag();
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(QDomElement e)
      {
      for (e = e.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
            QString tag(e.tagName());
            if (tag == "userLen1")
                  _userLen1 = Spatium(e.text().toDouble());
            else if (tag == "userLen2")
                  _userLen2 = Spatium(e.text().toDouble());
            else if (!Element::readProperties(e))
                  domError(e);
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Arpeggio::bbox() const
      {
      return QRectF(0.0, 0.0, symbols[arpeggioSym].width(magS()), _height);
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(QPainter& p) const
      {
      double _spatium = spatium();

      double y1 = _spatium - _userLen1.val() * _spatium;
      double y2 = _height + _spatium * .5 + _userLen2.val() * _spatium;
      switch(subtype()) {
            case 0:
                  for (double y = y1; y < y2; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case 1:
                  symbols[arpeggioarrowupSym].draw(p, 1.0, 0.0, y1);
                  for (double y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case 2:
                  {
                  double y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  symbols[arpeggioarrowdownSym].draw(p, 1.0, 0.0, y);
                  }
                  break;
            }
      }

//---------------------------------------------------------
//   updateGrips
//---------------------------------------------------------

void Arpeggio::updateGrips(int* grips, QRectF* grip) const
      {
      double _spatium = spatium();
      *grips   = 2;
      QPointF p1(0.0, -_userLen1.val() * _spatium);
      QPointF p2(0.0, _height + _userLen2.val() * _spatium);
      grip[0].translate(canvasPos() + p1);
      grip[1].translate(canvasPos() + p2);
      }

//---------------------------------------------------------
//   editDrag
//---------------------------------------------------------

void Arpeggio::editDrag(int n, const QPointF& delta)
      {
      Spatium d(delta.y() / spatium());
      if (n == 0)
            _userLen1 -= d;
      else if (n == 1)
            _userLen2 += d;
      }
