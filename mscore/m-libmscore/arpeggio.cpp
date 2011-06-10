//=============================================================================
//  MuseScore
//  Linux Music Score Editor
//  $Id: arpeggio.cpp 3555 2010-10-06 11:15:52Z wschweer $
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
#include "staff.h"
#include "part.h"
#include "segment.h"
#include "m-al/xml.h"
#include "painter.h"

//---------------------------------------------------------
//   Arpeggio
//---------------------------------------------------------

Arpeggio::Arpeggio(Score* s)
  : Element(s)
      {
      setFlags(ELEMENT_MOVABLE | ELEMENT_SELECTABLE);
      setHeight(spatium() * 4);      // for use in palettes
      _span = 1;
      }

//---------------------------------------------------------
//   setHeight
//---------------------------------------------------------

void Arpeggio::setHeight(qreal h)
      {
      _height = h;
      }

//---------------------------------------------------------
//   read
//---------------------------------------------------------

void Arpeggio::read(XmlReader* reader)
      {
      qreal r;
      while (reader->readElement()) {
            if (reader->readReal("userLen1", &r))
                  _userLen1 = Spatium(r);
            else if (reader->readReal("userLen2", &r))
                  _userLen2 = Spatium(r);
            else if (reader->readInt("span", &_span))
                  ;
            else if (!Element::readProperties(reader))
                  reader->unknown();
            }
      }

//---------------------------------------------------------
//   bbox
//---------------------------------------------------------

QRectF Arpeggio::bbox() const
      {
      qreal _spatium = spatium();
      qreal y1 = -_userLen1.val() * _spatium;
      qreal y2 = _height + _userLen2.val() * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
            case ARP_UP:
            case ARP_DOWN:
            default:
                  return QRectF(0.0, y1, symbols[score()->symIdx()][arpeggioSym].width(magS()), y2-y1);
            case ARP_BRACKET:
                  {
                  qreal lw = score()->styleS(ST_ArpeggioLineWidth).val() * _spatium;
                  qreal w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  return QRectF(-lw * .5, y1 - lw * .5, w + lw, y2 - y1 + lw);
                  }
            }
      }

//---------------------------------------------------------
//   draw
//---------------------------------------------------------

void Arpeggio::draw(Painter* p) const
      {
      qreal _spatium = spatium();

      qreal y1 = _spatium - _userLen1.val() * _spatium;
      qreal y2 = _height + (_userLen2.val() + .5) * _spatium;
      switch (subtype()) {
            case ARP_NORMAL:
                  for (qreal y = y1; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_UP:
                  symbols[score()->symIdx()][arpeggioarrowupSym].draw(p, 1.0, 0.0, y1);
                  for (qreal y = y1 + _spatium; y < y2; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  break;
            case ARP_DOWN:
                  {
                  qreal y = y1;
                  for (; y < y2 - _spatium; y += _spatium)
                        symbols[score()->symIdx()][arpeggioSym].draw(p, 1.0, 0.0, y);
                  symbols[score()->symIdx()][arpeggioarrowdownSym].draw(p, 1.0, 0.0, y);
                  }
                  break;
            case ARP_BRACKET:
                  {
                  y1 = - _userLen1.val() * _spatium;
                  y2 = _height + _userLen2.val() * _spatium;
                  p->save();

                  p->setLineCap(Qt::RoundCap);
                  p->setPenWidth(score()->styleS(ST_ArpeggioLineWidth).val() * _spatium);

                  qreal w = score()->styleS(ST_ArpeggioHookLen).val() * _spatium;
                  p->drawLine(0.0, y1, 0.0, y2);
                  p->drawLine(0.0, y1, w, y1);
                  p->drawLine(0.0, y2, w, y2);

                  p->restore();
                  }
                  break;
            }
      }

