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
#include "chord.h"
#include "note.h"
#include "score.h"

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
            xml.sTag("userLen1", _userLen1);
      if (_userLen2.val() != 0.0)
            xml.sTag("userLen2", _userLen2);
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
      double _spatium = spatium();
      double y1 = -_userLen1.val() * _spatium;
      double y2 = _height + _userLen2.val() * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
            case ARP_UP:
            case ARP_DOWN:
            default:
                  return QRectF(0.0, y1, symbols[arpeggioSym].width(magS()), y2-y1);
            case ARP_BRACKET:
                  {
                  double lw = score()->styleS(ST_ArpeggioLineWidth).val() * _spatium;
                  double w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  return QRectF(-lw * .5, y1 - lw * .5, w + lw, y2 - y1 + lw);
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(QPainter& p) const
      {
      double _spatium = spatium();

      double y1 = _spatium - _userLen1.val() * _spatium;
      double y2 = _height + (_userLen2.val() + .5) * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
                  for (double y = y1; y < y2; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_UP:
                  symbols[arpeggioarrowupSym].draw(p, 1.0, 0.0, y1);
                  for (double y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_DOWN:
                  {
                  double y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[arpeggioSym].draw(p, 1.0, 0.0, y);
                  symbols[arpeggioarrowdownSym].draw(p, 1.0, 0.0, y);
                  }
                  break;
            case ARP_BRACKET:
                  {
                  y1 = - _userLen1.val() * _spatium;
                  y2 = _height + _userLen2.val() * _spatium;
                  p.save();

                  QPen pen(p.pen());
                  pen.setWidthF(score()->styleS(ST_ArpeggioLineWidth).val() * _spatium);
                  pen.setCapStyle(Qt::RoundCap);
                  p.setPen(pen);

                  double w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  p.drawLine(QLineF(0.0, y1, 0.0, y2));
                  p.drawLine(QLineF(0.0, y1, w, y1));
                  p.drawLine(QLineF(0.0, y2, w, y2));

                  p.restore();
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

//---------------------------------------------------------
//   dragAnchor
//---------------------------------------------------------

QLineF Arpeggio::dragAnchor() const
      {
      Chord* chord = static_cast<Chord*>(parent());
      if (chord)
            return QLineF(canvasPos(), chord->upNote()->canvasPos());
      return QLineF();
      }

